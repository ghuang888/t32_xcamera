/*
 *         (C) COPYRIGHT Ingenic Limited
 *              ALL RIGHT RESERVED
 *
 * File        : gekko_ai3dnr.h
 * Authors     : klyu
 * Create Time : 2024-03-02 17:51:21 (CST)
 * Description :
 *
 */

#ifndef __AISP_GEKKO_INFERSDK_INCLUDE_GEKKO_AI3DNR_H__
#define __AISP_GEKKO_INFERSDK_INCLUDE_GEKKO_AI3DNR_H__

#include "type.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

//当前4MP 只支持开启时域滤波
typedef enum GEKKO_API {
    AI3DNR_NET_TF = 0,//时域滤波
    AI3DNR_NET_TSF = 1,//时域和空域滤波
    AI3DNR_NET_BUTT,
} AI3DNR_NET_TYPE;

typedef struct GEKKO_API {
    bool is_hdr_mode;//不必关注
    FPN_TYPE fpn_type;//不必关注
    AI3DNR_NET_TYPE net_type;//2MP 配置为AI3DNR_NET_TSF； 4MP 配置为AI3DNR_NET_TF
    const char *fpn_path;//标定文件路径，目前每台机器都需要标定，且标定文件不通用；后期提供通用方案
    const char *mode_path;//模型路径
} AI3DNRConfig;

typedef struct GEKKO_API {
    uint8_t tfs_lut[BV_LUT_ITEM_NUM];//时域滤波强度
    uint16_t sfs_lut[BV_LUT_ITEM_NUM];//空域滤波强度
} AI3DNRAttr;//强度信息，不建议自己调试

GEKKO_API int gekko_ai3dnr_init(VI_NUM vi_num, AI3DNRConfig *config);//滤波初始化
GEKKO_API int gekko_ai3dnr_deinit(VI_NUM vi_num);
GEKKO_API int gekko_ai3dnr_enable(VI_NUM vi_num);//滤波功能使能
GEKKO_API int gekko_ai3dnr_disable(VI_NUM vi_num);//滤波功能失能
GEKKO_API int gekko_ai3dnr_load_model(VI_NUM vi_num);//load 模型 占用nmem空间
GEKKO_API int gekko_ai3dnr_unload_model(VI_NUM vi_num);//unload 模型，释放出nmem
GEKKO_API int gekko_ai3dnr_set_attr(VI_NUM vi_num, AI3DNRAttr *attr);//配置属性，搭配TF  TSF使用
GEKKO_API int gekko_ai3dnr_get_attr(VI_NUM vi_num, AI3DNRAttr *attr);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __AISP_GEKKO_INFERSDK_INCLUDE_GEKKO_AI3DNR_H__ */
