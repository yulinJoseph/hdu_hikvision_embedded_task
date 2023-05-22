#include "config.h"

#include <fstream>
#include <iostream>

// FH related
#include "dsp/fh_system_mpi.h"
#include "dsp/fh_vpu_mpi.h"
#include "isp/isp_api.h"
#include "isp/isp_common.h"
#include "isp/isp_sensor_if.h"
#include "mpp/fh_vb_mpi.h"
#include "mpp/fh_vb_mpipara.h"
#include "sensor.h"
#include "vicap/fh_vicap_mpi.h"
// FH related

#include "wrapper/macro.h"

Config::Config(int w, int h, int fps, int group_id)
    : w(w), h(h), fps(fps), group_id(group_id) {
    std::cout << "hello world" << std::endl;
}

Config::~Config() {
    std::cout << "goodbye world" << std::endl;
}

bool Config::init_media_cache() {
    std::cout << "init media cache" << std::endl;
    VB_CONF_S stVbConf{};
    FH_VB_Exit();

    stVbConf.u32MaxPoolCnt = 4;
    stVbConf.astCommPool[0].u32BlkSize = 3840 * 2160 * 3;
    stVbConf.astCommPool[0].u32BlkCnt = 4;
    stVbConf.astCommPool[1].u32BlkSize = 1920 * 1080 * 3;
    stVbConf.astCommPool[1].u32BlkCnt = 4;
    stVbConf.astCommPool[2].u32BlkSize = 1280 * 720 * 3;
    stVbConf.astCommPool[2].u32BlkCnt = 4;
    stVbConf.astCommPool[3].u32BlkSize = 768 * 448 * 3;
    stVbConf.astCommPool[3].u32BlkCnt = 4;

    _IF_0(FH_VB_SetConf(&stVbConf));
    _IF_0(FH_VB_Init());

    // enc proc
    WR_PROC_DEV("/proc/driver/enc", "allchnstm_0_20000000_40");
    WR_PROC_DEV("/proc/driver/enc", "stm_20000000_40");
    // jpeg proc
    WR_PROC_DEV("/proc/driver/jpeg", "frmsize_1_3000000_3000000");
    WR_PROC_DEV("/proc/driver/jpeg", "jpgstm_12000000_2");
    WR_PROC_DEV("/proc/driver/jpeg", "mjpgstm_12000000_2");

    return true;
}

bool Config::init_isp(isp_sensor_if *sensor_func, int vimod, int vomod) {
    std::cout << "init isp" << std::endl;

    // sensor reset
    isp_sensor_reset();

    ISP_MEM_INIT mem_conf{};
    mem_conf.enOfflineWorkMode = ISP_OFFLINE_MODE_DISABLE;
    mem_conf.enIspOutMode = ISP_OUT_TO_VPU;
    mem_conf.enIspOutFmt = ISP_OUT_TO_DDR_YUV422_8BIT;  // 422 8bit
    mem_conf.stPicConf.u32Width = this->w;
    mem_conf.stPicConf.u32Height = this->h;
    _IF_0(API_ISP_MemInit(0, &mem_conf));

    ISP_VI_ATTR_S vi_conf{};
    vi_conf.u16InputWidth = this->w;
    vi_conf.u16InputHeight = this->h;
    vi_conf.u16PicWidth = this->w;
    vi_conf.u16PicHeight = this->h;
    vi_conf.u16FrameRate = this->fps;
    vi_conf.enBayerType = BAYER_GBRG;
    _IF_0(API_ISP_SetViAttr(0, &vi_conf));

    sensor_func->init = sensor_init_imx415;
    sensor_func->set_sns_fmt = sensor_set_fmt_imx415;
    sensor_func->set_sns_reg = sensor_write_reg;
    sensor_func->get_sns_reg = sensor_read_reg;
    sensor_func->set_exposure_ratio = sensor_set_exposure_ratio_imx415;
    sensor_func->get_exposure_ratio = sensor_get_exposure_ratio_imx415;
    sensor_func->get_sensor_attribute = sensor_get_attribute_imx415;
    sensor_func->set_flipmirror = sensor_set_mirror_flip_imx415;
    sensor_func->get_sns_ae_default = GetAEDefault;
    sensor_func->get_sns_ae_info = GetAEInfo;
    sensor_func->set_sns_gain = SetGain;
    sensor_func->set_sns_intt = SetIntt;
    _IF_0(API_ISP_SensorRegCb(0, 0, sensor_func));

    Sensor_Init_t sensor_conf{
        .u8CsiDeviceId = 0,
        .u8CciDeviceId = 1,
    };
    _IF_0(API_ISP_SensorInit(0, &sensor_conf));

    _IF_0(API_ISP_Init(0));

    FH_VICAP_DEV_ATTR_S vi_dev_conf{};
    vi_dev_conf.enWorkMode = VICAP_WORK_MODE_ONLINE;
    vi_dev_conf.stSize.u16Width = this->w;
    vi_dev_conf.stSize.u16Height = this->h;
    _IF_0(FH_VICAP_InitViDev(0, &vi_dev_conf));

    FH_VICAP_VI_ATTR_S vi_vi_conf{};
    vi_vi_conf.enWorkMode = VICAP_WORK_MODE_ONLINE;
    // attention
    vi_vi_conf.stInSize.u16Width = 3864;
    vi_vi_conf.stInSize.u16Height = 2192;
    vi_vi_conf.stCropSize.bCutEnable = FH_TRUE;
    vi_vi_conf.stCropSize.stRect.u16Width = this->w;
    vi_vi_conf.stCropSize.stRect.u16Height = this->h;
    _IF_0(FH_VICAP_SetViAttr(0, &vi_vi_conf));

    if (vimod == 1) {
        FH_BIND_INFO src, dst;
        src.obj_id = FH_OBJ_VICAP;
        src.dev_id = 0;
        src.chn_id = 0;
        dst.obj_id = FH_OBJ_ISP;
        dst.dev_id = 0;
        dst.chn_id = 0;
        FH_SYS_Bind(src, dst);
    }

    ISP_PARAM_CONFIG isp_param_conf{};
    _IF_0(API_ISP_GetBinAddr(0, &isp_param_conf));

    // auto param_addr = isp_param_conf.u32BinAddr;
    auto param_size = isp_param_conf.u32BinSize;
    // 分配内存
    auto isp_param_buff = new char[param_size];
    std::fstream param_file{};
    param_file.open("/home/imx415_mipi_attr.hex", std::ios::in | std::ios::binary);
    if (!param_file.is_open()) {
        std::cout << "open param file failed" << std::endl;
        std::cout << __FILE__ << ":" << __LINE__ << std::endl;
        delete[] isp_param_buff;
        return false;
    }
    param_file.read(isp_param_buff, param_size);
    param_file.close();
    {
        auto res = API_ISP_LoadIspParam(0, isp_param_buff);
        if (res != 0) {
            std::cout << "API_ISP_LoadIspParam failed, res = " << res << std::endl;
            std::cout << __FILE__ << ":" << __LINE__ << std::endl;
            delete[] isp_param_buff;
            return false;
        }
    }
    delete[] isp_param_buff;

    _IF_0(isp_server_run());

    return true;
}

bool Config::init_vpu() {
    std::cout << "init vpu" << std::endl;

    FH_VPU_SET_GRP_INFO grp_conf{};
    grp_conf.vi_max_size.u32Width = this->w;
    grp_conf.vi_max_size.u32Height = this->h;
    grp_conf.ycmean_en = 1;
    grp_conf.ycmean_ds = 16;
    _IF_0(FH_VPSS_CreateGrp(this->group_id, &grp_conf));

    FH_VPU_SIZE size_conf{};
    size_conf.vi_size.u32Width = this->w;
    size_conf.vi_size.u32Height = this->h;
    size_conf.crop_area.crop_en = 0;
    size_conf.crop_area.vpu_crop_area.u32X = 0;
    size_conf.crop_area.vpu_crop_area.u32Y = 0;
    size_conf.crop_area.vpu_crop_area.u32Width = 0;
    size_conf.crop_area.vpu_crop_area.u32Height = 0;
    _IF_0(FH_VPSS_SetViAttr(this->group_id, &size_conf));

    // 使能视频处理模块
    _IF_0(FH_VPSS_Enable(this->group_id, VPU_MODE_ISP));

    FH_VPU_CHN_INFO chn_info{};
    chn_info.bgm_enable = 1;
    chn_info.cpy_enable = 1;
    chn_info.sad_enable = 1;
    chn_info.bgm_ds = 8;
    chn_info.chn_max_size.u32Width = this->w;
    chn_info.chn_max_size.u32Height = this->h;
    chn_info.out_mode = VPU_VOMODE_SCAN;
    chn_info.support_mode = 1 << chn_info.out_mode;
    chn_info.bufnum = 3;
    chn_info.max_stride = 0;
    // 创建通道
    _IF_0(FH_VPSS_CreateChn(this->group_id, 0, &chn_info));

    FH_VPU_CHN_CONFIG chn_conf;
    chn_conf.vpu_chn_size.u32Width = this->w;
    chn_conf.vpu_chn_size.u32Height = this->h;
    chn_conf.crop_area.crop_en = 0;
    chn_conf.crop_area.vpu_crop_area.u32X = 0;
    chn_conf.crop_area.vpu_crop_area.u32Y = 0;
    chn_conf.crop_area.vpu_crop_area.u32Width = 0;
    chn_conf.crop_area.vpu_crop_area.u32Height = 0;
    chn_conf.offset = 0;
    chn_conf.depth = 1;
    chn_conf.stride = 0;
    // 设置视频处理模块通道属性
    _IF_0(FH_VPSS_SetChnAttr(this->group_id, 0, &chn_conf));

    // 设置视频处理模块通道输出组织格式
    _IF_0(FH_VPSS_SetVOMode(this->group_id, 0, VPU_VOMODE_SCAN));

    // 开启通道使能
    _IF_0(FH_VPSS_OpenChn(this->group_id, 0));

    return true;
}
