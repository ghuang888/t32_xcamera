/*
 *         (C) COPYRIGHT Ingenic Limited
 *              ALL RIGHT RESERVED
 *
 * File        : gekko.h
 * Authors     : klyu
 * Create Time : 2024-03-02 16:19:00 (CST)
 * Description :
 *
 */

#ifndef __AISP_GEKKO_INFERSDK_INCLUDE_GEKKO_H__
#define __AISP_GEKKO_INFERSDK_INCLUDE_GEKKO_H__
#include "type.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

GEKKO_API int gekko_init(VI_NUM vi_num, uint32_t nmem_size);//gekko初始化，构建数据通路;nmem_size是gekko和另一个算法比如人车使用nmem的总和
GEKKO_API int gekko_deinit(VI_NUM vi_num);//gekko反初始化，销毁数据通路；关闭gekko时一定要调用，否则影响帧率
GEKKO_API int gekko_start(VI_NUM vi_num);//gekko开始处理
GEKKO_API int gekko_stop(VI_NUM vi_num);//gekko停止处理
GEKKO_API int gekko_set_bv_lut(VI_NUM vi_num, BVLut *bv_lut);//修改bv曝光表
GEKKO_API int gekko_get_bv_lut(VI_NUM vi_num, BVLut *bv_lut);//获取IQ bin中的bv曝光表
GEKKO_API int gekko_set_module_ctrl(VI_NUM vi_num, ModuleCtrl *module_ctrl);//先不必关注
GEKKO_API int gekko_get_module_ctrl(VI_NUM vi_num, ModuleCtrl *module_ctrl);//先不必关注


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __AISP_GEKKO_INFERSDK_INCLUDE_GEKKO_H__ */
