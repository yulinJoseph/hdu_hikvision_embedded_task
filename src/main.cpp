#include <ctime>
#include <fcntl.h>
#include <iostream>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <thread>
#include <unistd.h>

// FH related
#include "dsp/fh_system_mpi.h"
#include "dsp/fh_venc_mpi.h"
#include "vicap/fh_vicap_mpi.h"
// FH related

#include "libdmc.h"
#include "libdmc_pes.h"
#include "libdmc_record_raw.h"
#include "libpes.h"
#include "sensor.h"
#include "wrapper/config.h"
#include "wrapper/drawer.h"
#include "wrapper/led_ctrl.h"
#include "wrapper/macro.h"
#include "wrapper/venc.h"

#define TRACE_PROC "/proc/driver/trace"

isp_sensor_if sensor_func;

static int g_sig_stop = 0;
static void sample_vlcview_handle_sig(int signo) {
    g_sig_stop = 1;
}

static int g_get_stream_stop = 0;
static int g_get_stream_running = 0;

int isp_set_param(int key, int param) {
    int ret;

    switch (key) {
        case ISP_AE:
            ret = isp_set_ae(param);
            break;
        case ISP_AWB:
            ret = isp_set_awb(param);
            break;
        case ISP_COLOR:
            ret = isp_set_saturation(param);
            break;
        case ISP_BRIGHT:
            ret = isp_set_bright(param);
            break;
        case ISP_NR:
            ret = isp_set_nr(param);
            break;
        case ISP_MF:
            ret = isp_set_mirrorflip(param);
            break;
        default:
            printf("Error: not support the key %d\n", key);
            break;
    }
    if (ret != 0) {
        printf("Error(%d - %x): set isp param failed!\n", ret, ret);
        return -1;
    }

    return ret;
}

int sample_dmc_init(FH_CHAR *dst_ip, FH_UINT32 port, FH_SINT32 max_channel_no) {
    dmc_init();

    if (dst_ip != nullptr && *dst_ip != 0) {
        dmc_pes_subscribe(max_channel_no, dst_ip, port);
    }

    return 0;
}

void *get_stream_proc(void *arg) {
    FH_SINT32 ret, i;
    FH_SINT32 end_flag;
    FH_SINT32 subtype;
    FH_VENC_STREAM stream;
    FH_SINT32 *stop = (FH_SINT32 *)arg;

    while (*stop == 0) {
        WR_PROC_DEV(TRACE_PROC, "timing_GetStream_START");

        ret = FH_VENC_GetStream_Block(FH_STREAM_ALL & (~(FH_STREAM_JPEG)), &stream);
        WR_PROC_DEV(TRACE_PROC, "timing_EncBlkFinish_xxx");

        if (ret != 0) {
            printf("Error(%d - %x): FH_VENC_GetStream_Block(FH_STREAM_ALL & (~(FH_STREAM_JPEG))) failed!\n", ret, ret);
            continue;
        }

        if (stream.stmtype == FH_STREAM_H264) {
            subtype = stream.h264_stream.frame_type == FH_FRAME_I ? DMC_MEDIA_SUBTYPE_IFRAME : DMC_MEDIA_SUBTYPE_PFRAME;
            for (i = 0; i < stream.h264_stream.nalu_cnt; i++) {
                end_flag = (i == (stream.h264_stream.nalu_cnt - 1)) ? 1 : 0;
                dmc_input(stream.chan,
                          DMC_MEDIA_TYPE_H264,
                          subtype,
                          stream.h264_stream.time_stamp,
                          stream.h264_stream.nalu[i].start,
                          stream.h264_stream.nalu[i].length,
                          end_flag);
            }
        } else if (stream.stmtype == FH_STREAM_H265) {
            subtype = stream.h265_stream.frame_type == FH_FRAME_I ? DMC_MEDIA_SUBTYPE_IFRAME : DMC_MEDIA_SUBTYPE_PFRAME;
            for (i = 0; i < stream.h265_stream.nalu_cnt; i++) {
                end_flag = (i == (stream.h265_stream.nalu_cnt - 1)) ? 1 : 0;
                dmc_input(stream.chan,
                          DMC_MEDIA_TYPE_H265,
                          subtype,
                          stream.h265_stream.time_stamp,
                          stream.h265_stream.nalu[i].start,
                          stream.h265_stream.nalu[i].length,
                          end_flag);
            }
        } else if (stream.stmtype == FH_STREAM_MJPEG) {
            dmc_input(stream.chan,
                      DMC_MEDIA_TYPE_MJPEG,
                      0,
                      0,
                      stream.mjpeg_stream.start,
                      stream.mjpeg_stream.length,
                      1);
        }

        ret = FH_VENC_ReleaseStream(&stream);
        if (ret) {
            printf("Error(%d - %x): FH_VENC_ReleaseStream failed for chan(%d)!\n", ret, ret, stream.chan);
        }
        WR_PROC_DEV(TRACE_PROC, "timing_GetStream_END");
    }

    *stop = 0;
    return nullptr;
}

std::string get_cur_time() {
    time_t t = time(nullptr);
    char ch[64] = {0};
    strftime(ch, sizeof(ch) - 1, "%Y-%m-%d %H:%M:%S", localtime(&t));
    return std::string(ch);
}

int main(int argc, char *argv[]) {
    // 设置信号处理函数
    signal(SIGINT, sample_vlcview_handle_sig);
    signal(SIGQUIT, sample_vlcview_handle_sig);
    signal(SIGKILL, sample_vlcview_handle_sig);
    signal(SIGTERM, sample_vlcview_handle_sig);

    auto dst_ip = argc > 1 ? argv[1] : nullptr;
    auto port = argc > 2 ? strtol(argv[2], nullptr, 0) : 1234;

    LedCtrl led{};
    led.on();

    Config config(3840, 2160, 30, 0);
    // 配置视频缓存池
    _IF_bool(config.init_media_cache());
    // 启动系统
    _IF_0(FH_SYS_Init());
    // 初始化 ISP
    _IF_bool(config.init_isp(&sensor_func));
    // 初始化 vpu
    _IF_bool(config.init_vpu());

    // 配置编码
    Venc venc(3840, 2160, 30, 3840, 2160, 0, 0);

    // 暂不做修改
    sample_dmc_init(dst_ip, port, 1);
    std::thread thread_stream(get_stream_proc, &g_get_stream_stop);

    Drawer drawer{};
    _IF_bool(drawer.put_text(0, std::string("hello world"), 0, 0););
    _IF_bool(drawer.put_text(1, std::string("一二三"), 100, 100););
    std::thread thread_timer([&drawer]() {
        while (true) {
            drawer.put_text(2, get_cur_time(), 200, 200);
            sleep(1);
        }
    });

    _IF_bool(drawer.draw_mask(0, 200, 200, 200, 200, 0x00ff0000););
    _IF_bool(drawer.draw_mask(1, 400, 400, 200, 200, 0x00ffff00););

    isp_set_param(ISP_AE, 1);
    isp_set_param(ISP_AWB, 1);
    isp_set_param(ISP_COLOR, 25);
    isp_set_param(ISP_BRIGHT, 125);
    isp_set_param(ISP_NR, 1);
    isp_set_param(ISP_MF, 0);

    sleep(60);
    led.off();
    exit(0);

    return 0;
}
