#ifndef _ISP_SENSOR_IF_H_
#define _ISP_SENSOR_IF_H_

#include "isp_common.h"
#include "types/type_def.h"

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(4)

#define MIRROR_FLIP_BAYER_FORMATE_SIZE 16  // 对应get_sensor_mirror_flip_bayer_format函数指针返回的数据大小
#define USER_AWB_GAIN_SIZE 48              // 对应get_user_awb_gain函数指针返回的数据大小
#define USER_LTM_CURVE_SIZE 512            // 对应get_user_ltm_curve函数指针返回的数据大小
#define SENSOR_AWB_GAIN_SIZE 12            // 对应get_awb_gain/set_awb_gain函数指针传参的数据大小
#define SENSOR_USER_DEFINED_DATA_SIZE 512  // 对应SENSOR_USER_DEFINED_DATA* data指针返回的数据大小

    typedef struct _Sensor_AE_Default_S {
        unsigned int minIntt;      // 最小曝光时间，单位为行
        unsigned int maxIntt;      // 最大曝光时间(非降帧时)，单位为行
        unsigned int minAGain;     // 最小曝光增益,U.6精度
        unsigned int maxAGain;     // 最大曝光增益,U.6精度
        unsigned int fullLineStd;  // 框架高度(非降帧时)
        unsigned int MinRstLine;   // 计算最大曝光时间时的最小复位行数
    } Sensor_AE_Default_S;

    typedef struct _Sensor_AE_INFO_S {
        unsigned int currIntt;    // 当前曝光时间，单位为行
        unsigned int currAGain;   // 当前曝光增益，u.6精度
        unsigned int currHsf;     // 当前行频
        unsigned int currFrameH;  // 当前框架高度
    } Sensor_AE_INFO_S;

    typedef union {
        unsigned int dw[128];
    } SENSOR_USER_DEFINED_DATA;

    typedef struct {
        FH_UINT8 u8CsiDeviceId;
        FH_UINT8 u8CciDeviceId;
        FH_BOOL bGrpSync;  //是否需要多sensor同步,
    } Sensor_Init_t;

    // module interface
    struct isp_sensor_if {
        FH_CHAR *name; /*!< sensor名称 */

        FH_SINT32(*get_vi_attr)
        (void *obj, ISP_VI_ATTR_S *vi_attr);

        FH_SINT32(*set_flipmirror)
        (void *obj, FH_UINT32 sensor_en_stat);

        FH_SINT32(*get_flipmirror)
        (void *obj, FH_UINT32 *sensor_en_stat);

        FH_SINT32(*set_iris)
        (void *obj, FH_UINT32 iris);

        FH_SINT32(*init)
        (void *obj, Sensor_Init_t *initCfg);

        FH_VOID(*reset)
        (void *obj);

        FH_SINT32(*deinit)
        (void *obj);

        FH_SINT32(*set_sns_fmt)
        (void *obj, int format);

        FH_SINT32(*kick)
        (void *obj);

        FH_SINT32(*set_sns_reg)
        (void *obj, FH_UINT16 addr, FH_UINT16 data);

        FH_SINT32(*set_exposure_ratio)
        (void *obj, FH_UINT32 exposure_ratio);

        FH_SINT32(*get_exposure_ratio)
        (void *obj, FH_UINT32 *exposure_ratio);

        FH_SINT32(*get_sensor_attribute)
        (void *obj, char *name, FH_UINT32 *value);

        FH_SINT32(*set_lane_num_max)
        (void *obj, FH_UINT32 lane_num_max);

        FH_SINT32(*get_sns_reg)
        (void *obj, FH_UINT16 addr, FH_UINT16 *data);

        FH_SINT32(*get_awb_gain)
        (void *obj, FH_UINT32 *awb_gain);

        FH_SINT32(*set_awb_gain)
        (void *obj, FH_UINT32 *awb_gain);

        SENSOR_USER_DEFINED_DATA *data;

        void *para;

        FH_SINT32(*common_if)
        (void *obj, unsigned int cmd, ISP_SENSOR_COMMON_CMD_DATA0 *data0, ISP_SENSOR_COMMON_CMD_DATA1 *data1);

        // new add interface

        FH_SINT32(*get_sns_ae_default)
        (void *obj, Sensor_AE_Default_S *sensAEDefault);

        FH_SINT32(*get_sns_ae_info)
        (void *obj, Sensor_AE_INFO_S *sensAEInfo);

        FH_SINT32(*set_sns_intt)
        (void *obj, FH_UINT32 intt, FH_UINT8 fNo);

        FH_SINT32(*calc_sns_valid_intt)
        (void *obj, FH_UINT32 *u32Intt);

        FH_SINT32(*set_sns_gain)
        (void *obj, FH_UINT32 again, FH_UINT8 fNo);

        FH_SINT32(*calc_sns_valid_gain)
        (void *obj, FH_UINT32 *u32Gain);

        FH_SINT32(*set_sns_frame_height)
        (void *obj, FH_UINT32 frameH);

        FH_UINT32 *(*get_sensor_mirror_flip_bayer_format)(void *obj);

        FH_UINT32 *(*get_user_awb_gain)(void *obj, FH_UINT32 idx);

        FH_UINT32 *(*get_user_ltm_curve)(void *obj, FH_UINT32 idx);

        FH_SINT32(*is_sensor_connect)
        (void *obj, FH_UINT8 deviceId);
    };

    enum COMMON_IF_CMD {
        CMD_INIT = 0,
        CMD_MAP_INTT_GAIN = 1,
        CMD_GET_BLC_VALUE = 2,
        CMD_GET_GPIO_PARAM = 3,
        CMD_GET_REGS_INFO = 4,
        COMMON_IF_CMD_DUMMY = 0xffffffff,
    };

    struct isp_sensor_if *Sensor_Create(void);
    FH_VOID Sensor_Destroy(struct isp_sensor_if *s_if);

#pragma pack()
#ifdef __cplusplus
}
#endif

#endif /* _ISP_SENSOR_H_ */
