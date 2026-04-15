#ifndef __IVS_INF_MOVE_H__
#define __IVS_INF_MOVE_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */

#include "ivs_common.h"

#define MOVE_VERSION_NUM 0x00000101
uint32_t move_get_version_info();

#define NUM_OF_RECTS 8
#define MAX_DET_NUM 256
#define IVS_PERM_MAX_ROI  8
#define IVS_MOVE_MAX_ROI  64 /**IVS_MOVE_MAX_ROI=[(frame_width/det_w)*(frame_height/det_h)] */
#define IVS_PERM_MAX_RECT 8
/*
 * 高级移动侦测算法的输入结构体
 */

/*
 * 周界信息结构体
 */
/* typedef struct{ */
/*     int pattern;/\**<0：虚线；1：实线*\/ */
/* }line_det; */



/*
 * 周界信息结构体
 */
typedef struct{
  IVSPoint *p;    /**< 周界各个顶点信息，不能有线交叉，最多6个点 */
  /* line_det *l; */
  int pcnt;      /**< 周界顶点数量 */
  int fun;       /**<功能：0：周界检测；1：热区检测 ;2:虚线实线检测*/
  uint64_t alarm_last_time;    /**< 持续报警时间 */
}single_perm_tt;


typedef struct{
    IVSPoint det_rects[MAX_DET_NUM];/**< 检测出移动区域的宫格坐标 */
    int area[MAX_DET_NUM];/**< 移动区域的area */
}move_g;

typedef struct {
    single_perm_tt perms[IVS_PERM_MAX_ROI];  /**< 周界信息 */
    int permcnt;                            /**< 周界数量 */

    int sense; /**< 高级移动侦测的灵敏度，范围为0-4 */

    int min_h; /**< 高级移动侦测物体的最小高度 */
    int min_w; /**< 高级移动侦测物体的最小宽度 */

  IVSRect* rois; /**< 高级移动侦测待检测的区域信息 */
  int cntRoi; /**< 高级移动侦测待检测区域的数量 */

  int isSkipFrame; /**< 高级移动侦测跳帧处理开关 */
    bool light;/**< 开灯 >  */
    float level;/**0-1,屏幕检测程度  */
  int timeon;/**开灯用时  */
  int timeoff;/**开灯间隔  */
  int isLightRemove; /**< 高级移动侦测光照处理开关 */

    int det_w; /**<宫格最小单元宽度*/
    int det_h; /**<宫格最小单元高度 */

    IVSFrameInfo frameInfo; /**< 帧信息 */
}move_param_input_t;

/*
 * 高级移动侦测算法的输出结构体
 */
typedef struct {
  int ret;        /**< 是否检测出移动区域 */
  int count; /**< 检测出移动区域的数量 */
  IVSRect rects[NUM_OF_RECTS]; /**< 检测出移动区域的信息 */
  int blockSize; /**< 检测出移动区域块大小 */
  IVSPoint *pt; /**< 移动区域左上角点坐标 */

  move_g g_res; /**< 检测出移动区域的宫格信息 */
  int detcount; /**< 检测出移动区域的宫格数量 */


  int is_alarmed[IVS_PERM_MAX_ROI];     /**< 那个周界出现越界 */
  /* IVSRect rects[IVS_PERM_MAX_RECT];     /\**< 越界物体的矩形信息 *\/ */
  int rectcnt;                            /**< 越界物体的数量 */



}move_param_output_t;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
#endif /* __IVS_INF_MOVE_H__ */
