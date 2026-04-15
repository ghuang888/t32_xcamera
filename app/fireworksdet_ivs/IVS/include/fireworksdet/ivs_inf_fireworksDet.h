/*
 * 烟火检测
 * reserved：代表预留参数，目前未实现其功能；
 * 参数具体含义详见参数注释
 * 基本功能：
 *   设置nmem大小：配置参数nmem_size，即init时配的nmem大小，单位Byte，应为整数，单跑一个算法或跑多进程时使用sample中给的默认值即可，若使用单进程多线程模式，需要配置算法所需的最大的nmem大小，例：单跑算法A需要9M，单跑算法B需要15M，AB算法跑单进程多线程模式，需要将A和B的nmem_size都配15M;
 *   设置nmem分配模式：share_memory_mode置0：默认模式；
                     share_memory_mode置1：智能释放nmem模式，比默认模式使用nmem要少;
 *   降低误检：降低检测灵敏度sense；
 *   提升正检：增大检测灵敏度sense；
 *   移动目标侦测：开启：enable_move置true 输出所有移动目标的结果，可设置move_sense改变灵敏度，设置move_min_h和move_min_w过滤小目标；
 *               关闭：enable_move置false；
 *   移动烟火目标侦测：开启：enable_move置true open_move_filter置true 只输出移动烟火结果，可设置move_sense改变灵敏度，设置move_min_h和move_min_w过滤小目标；
 *                  关闭： open_move_filter置false；
 *   周界功能：开启：enable_perm置true 只输出指定区域内目标信息，周界各个顶点信息IVSPoint *p，不能有线交叉，各点形成的多边形须为凸多边形，最多可设置4个不同的周界；
 *           关闭：enable_perm置false；
 *   跳帧检测：设置skip_num，每隔skip_num帧检测一次；
 *   跟踪：开启：switch_track置true，会输出跟踪ID号track_id，相同ID视为同一个目标，第一帧track_id为-1，不可用；
 *        关闭：switch_track置false，track_id为-1；
 *   检测功能：开启：switch_stop_det置true，开启检测功能，烟火结果正常输出，可通过fast_update_params快速设置更新switch_stop_det值；
 *           关闭：switch_stop_det置false，关闭不输出烟火结果；
 *   set params：fast_update_params置true, 只能修改switch_stop_det值；
 *               fast_update_params置false，所有的参数都可以修改包括switch_stop_det；
 *               区别:fast_update_params置true可快速设置switch_stop_det参数。置false会重新初始化设置参数；
 *   打印检测时间：开启：ptime置true;
 *               关闭：ptime置false;
 */

#ifndef __IVS_INF_FIREWORKSDET_H__
#define __IVS_INF_FIREWORKSDET_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */

#include "ivs_common.h"

#define FIREWORKSDET_VERSION_NUM 0x00000100 /*2023-00227*/
#define NUM_OF_FIREWORKSS 50
#define NUM_OF_MOVE 50
#define IVS_FIREWORKSDET_PERM_MAX_ROI  4
#define IVS_FIREWORKSDET_PERM_MAX_RECT 4

uint32_t fireworksdet_get_version_info(void);

typedef struct {
        IVSRect box; /**< reserved >*/
        IVSRect show_box; /**< 烟火检测坐标 >*/
        int class_id; /**< reserved >*/
        int track_id; /**< 跟踪ID，track_id相同即视为同一个目标  >*/
        float confidence; /**< 烟火检测结果的置信度越高被预测成烟火的可能性越大 >*/
}fireworks_info;

typedef struct {
    //mod = 0: Configure convex polygons
        IVSPoint *p;    /**< 周界各个顶点信息，不能有线交叉，各点形成的多边形须为凸多边形 >*/
        int pcnt;      /**<  周界顶点数量，最多8个点 >*/
    //mod = 1:Configure Rect
        IVSRect r;  /**< reserved >*/
        int detdist; /**< reserved >*/
        uint64_t alarm_last_time;    /**< reserved >*/
}fireworksdet_perm_t;
typedef struct {
    int index;			/**< frame index */
    unsigned char *data; /**< 帧的起始地址 */
    unsigned char *phydata; /**< 帧的起始地址 */
    int width;         /**< 帧分辨率的宽 */
    int height;         /**< 帧分辨率的高 */
    int pixfmt;         /**< 帧的图像格式 */
    int64_t timeStamp; /**< 帧的时间戳 */
}IVSPhyFrame; 
typedef IVSPhyFrame IVSPhyFrameInfo;
/*
 * 烟火检测输入结构体
     >*/
typedef struct {
        unsigned int max_fireworks_box; /**< 烟火检测结果最多的框数量，若图片中检测目标多于max_fireworks_box则随机输出max_fireworks_box个框 <30 >*/
        int nmem_size; /**<配置参数nmem_size，即init时配的nmem大小，单位Byte，应为整数，单跑一个算法或跑多进程时使用sample中给的默认值即可，若使用单进程多线程模式，需要配置算法所需的最大的nmem大小，例：单跑算法A需要9M，单跑算法B需要15M，AB算法跑单进程多线程模式，需要将A和B的nmem_size都配15M>*/
        int share_memory_mode; /**<nmem分配方式：0：默认模式； 1：智能释放nmem模式，比默认模式使用nmem要少>*/
        const char* model_path; /**< 烟火检测模型路径 >*/
        // int model_type;   /**0:4.1M  提供了1个不同大小的烟火模型，根据需要设置model_type参数选取相应的模型，模型越大效果越好，>*/
        int sense; /**< 检测灵敏度 0~6 0:最不灵敏，误检率最低，正检会下降 6:最灵敏，正检率最高，误检相应增加 default:4 >*/
        int detdist; /**< reserved >*/
        fireworksdet_perm_t perms[IVS_FIREWORKSDET_PERM_MAX_ROI]; /**< 周界信息 >*/
        int skip_num; /**< 跳帧数目，隔帧检测，每隔skip_num帧检测一次 >*/
        unsigned int delay; /**< reserved >*/
        IVSPhyFrame frameInfo; /**< 帧信息，详见ivs_common.h >*/
        bool rot90; /**< reserved >*/
        bool switch_track; /**< 取值true/false; true: 开启跟踪，track_id 会输出跟踪ID号，相同ID视为同一个目标 false: 关闭跟踪，track_id=-1 不可用 >*/
        bool switch_stop_det; /**< 取值true/false; true: 开启检测，输出烟火检测结果，false: 关闭检测，不输出烟火检测结果 >*/
        bool fast_update_params; /**< 快速设置更新switch_stop_det参数，不构造初始化，不释放已开辟资源 >*/
        bool ptime; /**< 是否打印检测时间 >*/

}fireworksdet_param_input_t;
/*
     * 烟火检测输出结构体
 */
typedef struct {
        int count; /**< 检测烟火的个数，小于等于max_fireworks_box >*/
        int count_move; /**< 识别出移动目标的个数 >*/
        IVSRect move_rects[NUM_OF_MOVE]; /**< 识别出移动目标区域坐标 >*/
        fireworks_info fireworks[NUM_OF_FIREWORKSS]; /**< 烟火信息 >*/
        int64_t timeStamp; /**< 时间戳 >*/
}fireworksdet_param_output_t;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
#endif /* __IVS_INF_FIREWORKSDET_H__ */
