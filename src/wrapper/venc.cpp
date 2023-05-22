#include "venc.h"

#include <iostream>

#include "dsp/fh_system_mpi.h"
#include "dsp/fh_venc_mpi.h"
#include "dsp/fh_venc_mpipara.h"
#include "wrapper/macro.h"

Venc::Venc(int in_w, int in_h, int fps, int out_w, int out_h, int group_id, int chan)
    : in_w(in_w), in_h(in_h), fps(fps), out_w(out_w), group_id(group_id), out_h(out_h), chan(chan) {
    bool res = init();
    if (!res) {
        std::cout << "Venc init failed" << std::endl;
        std::cout << __FILE__ << ":" << __LINE__ << std::endl;
        exit(1);
    }
}

Venc::~Venc() {
}

bool Venc::init() {
    FH_VENC_CHN_CAP chn_cap_conf{};
    chn_cap_conf.support_type = FH_NORMAL_H264 | FH_NORMAL_H265;
    chn_cap_conf.max_size.u32Height = in_h;
    chn_cap_conf.max_size.u32Width = in_w;
    _IF_0(FH_VENC_CreateChn(this->chan, &chn_cap_conf));

    FH_VENC_CHN_CONFIG chn_conf;
    chn_conf.chn_attr.enc_type = FH_NORMAL_H264;
    chn_conf.chn_attr.h264_attr.profile = H264_PROFILE_MAIN;
    chn_conf.chn_attr.h264_attr.i_frame_intterval = 50;
    chn_conf.chn_attr.h264_attr.size.u32Width = this->out_w;
    chn_conf.chn_attr.h264_attr.size.u32Height = this->out_h;

    chn_conf.rc_attr.rc_type = FH_RC_H264_CBR;
    chn_conf.rc_attr.h264_cbr.init_qp = 35;
    chn_conf.rc_attr.h264_cbr.bitrate = 5000 * 1000;
    chn_conf.rc_attr.h264_cbr.FrameRate.frame_count = 30;
    chn_conf.rc_attr.h264_cbr.FrameRate.frame_time = 1;
    chn_conf.rc_attr.h264_cbr.maxrate_percent = 200;
    chn_conf.rc_attr.h264_cbr.IFrmMaxBits = 0;
    chn_conf.rc_attr.h264_cbr.IP_QPDelta = 3;
    chn_conf.rc_attr.h264_cbr.I_BitProp = 5;
    chn_conf.rc_attr.h264_cbr.P_BitProp = 1;
    chn_conf.rc_attr.h264_cbr.fluctuate_level = 0;
    _IF_0(FH_VENC_SetChnAttr(this->chan, &chn_conf));

    // 旋转 180 度
    _IF_0(FH_VENC_SetRotate(this->chan, FH_RO_OPS_180));

    FH_BIND_INFO src, dst;
    src.obj_id = FH_OBJ_ISP;
    src.dev_id = 0;
    src.chn_id = 0;
    dst.obj_id = FH_OBJ_VPU_VI;
    dst.dev_id = 0;
    dst.chn_id = 0;
    _IF_0(FH_SYS_Bind(src, dst));
    _IF_0(FH_VENC_StartRecvPic(this->chan));
    src.obj_id = FH_OBJ_VPU_VO;
    src.dev_id = this->group_id;
    src.chn_id = 0;
    dst.obj_id = FH_OBJ_ENC;
    dst.dev_id = 0;
    dst.chn_id = 0;
    // 数据源到数据接收者的绑定接口
    _IF_0(FH_SYS_Bind(src, dst));

    return true;
}
