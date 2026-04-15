/*
 *         (C) COPYRIGHT Ingenic Limited
 *              ALL RIGHT RESERVED
 *
 * File        : type.h
 * Authors     : klyu
 * Create Time : 2024-03-02 16:07:48 (CST)
 * Description :
 *
 */

#ifndef __AISP_GEKKO_INFERSDK_INCLUDE_TYPE_H__
#define __AISP_GEKKO_INFERSDK_INCLUDE_TYPE_H__

#include <stdint.h>

#if defined _WIN32 || defined __CYGWIN__
#ifdef BUILDING_DLL
#ifdef __GNUC__
#define DLL_EXPORTS __attribute__((dllexport))
#else
#define DLL_EXPORTS __declspec(dllexport)
#endif
#else // BUILDING_DLL
#ifdef __GNUC__
#define DLL_EXPORTS __attribute__((dllimport))
#else
#define DLL_EXPORTS __declspec(dllimport)
#endif
#endif // BUILDING_DLL
#else
#if __GNUC__ >= 4
#define DLL_EXPORTS __attribute__((visibility("default")))
#else
#define DLL_EXPORTS
#endif
#endif

#define GEKKO_API DLL_EXPORTS
#define BV_LUT_ITEM_NUM 11

#ifdef __cplusplus
extern "C" {
#endif

typedef enum GEKKO_API {
    VI_MAIN = 0, /* Main Camera */
    VI_SEC = 1,  /* Second Camera */
    VI_THR = 2,  /* Third Camera */
    VI_BUTT,
} VI_NUM;

typedef enum GEKKO_API {
    FPN_TYPE_FULL = 0,
    FPN_TYPE_HALF = 1,
    FPN_TYPE_BUTT,
} FPN_TYPE;

typedef struct GEKKO_API {
    uint32_t bv[BV_LUT_ITEM_NUM];
} BVLut;

typedef union GEKKO_API {
    uint32_t reg_key;
    struct {
        uint32_t ai3dnr_switch : 1; /*reg_key[0]: 1: enable ai3dnr, 0: disable ai3dnr*/
        uint32_t aile_switch : 1;   /*reg_key[1]: 1: enable aile, 0: disable aile*/
        uint32_t reserved : 30;     /*reg_key[2:31]: reserved*/
    };
} ModuleCtrl;

#ifdef __cplusplus
}
#endif

#endif /* __AISP_GEKKO_INFERSDK_INCLUDE_TYPE_H__ */
