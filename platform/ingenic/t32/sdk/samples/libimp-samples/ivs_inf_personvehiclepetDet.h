/*
 * 人车检测
 * reserved：代表预留参数，目前未实现其功能；
 * 参数具体含义详见参数注释
 * 基本功能：
 *   降低误检：降低检测灵敏度sense
 *   提升正检：增大检测灵敏度sense
 *   设置nmem大小：配置参数nmem_size，即init时配的nmem大小，单位Byte，应为整数，单跑一个算法或跑多进程时使用sample中给的默认值即可，若使用单进程多线程，需用drivers.m.so统计nmem大小。
 *   移动目标侦测：开启：enable_move置true 输出所有移动目标的结果，可设置move_sense改变灵敏度，设置move_min_h和move_min_w过滤小目标
 *               关闭：enable_move置false
 *   移动人形目标侦测：开启：enable_move置true open_move_filter置true 只输出移动人形结果，可设置move_sense改变灵敏度，设置move_min_h和move_min_w过滤小目标
 *                  关闭： open_move_filter置false
 *   周界功能：开启：enable_perm置true 只输出指定区域内目标信息，周界各个顶点信息IVSPoint *p，不能有线交叉，各点形成的多边形须为凸多边形，定点数量pcnt小于等于8， 最多可设置4个不同的周界
 *           关闭：enable_perm置false
 *   跳帧检测：设置skip_num，每隔skip_num帧检测一次
 *   跟踪：开启：switch_track置true，会输出跟踪ID号track_id，相同ID视为同一个目标，第一帧track_id为-1，不可用
 *        关闭：switch_track置false，track_id为-1
 *   检测功能：开启：switch_stop_det置true，开启检测功能，人形结果正常输出，可通过fast_update_params快速设置更新switch_stop_det值
 *           关闭：switch_stop_det置false，关闭不输出人形结果
 *   快启减少加载模型时间方法：1.用buff地址形式加载模型，提前把模型读好，传模型地址；
 *                         2.把check_model置false；
 */

#ifndef __IVS_INF_PERSONVEHICLEPETDET_H__
#define __IVS_INF_PERSONVEHICLEPETDET_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */

#include "ivs_common.h"

#define PERSONVEHICLEPETDET_VERSION_NUM 0x00000001
#define NUM_OF_PERSONVEHICLEPETS 100
#define NUM_OF_MOVE 100
#define IVS_PERSONVEHICLEPETDET_PERM_MAX_ROI  4
#define IVS_PERSONVEHICLEPETDET_PERM_MAX_RECT 4
#define MOVE_IDS_SIZE 10

    uint32_t personvehiclepetdet_get_version_info(void);

    typedef struct {
        IVSRect box; /**< reserved >*/
        IVSRect show_box; /**< 人形检测坐标 >*/
        int class_id; /**<0:person,1:car,2:bus,3:truck,7:cat,8:dog>*/
        int track_id; /**< 跟踪ID，track_id相同即视为同一个目标  >*/
        float confidence; /**< 人形检测结果的置信度越高被预测成人形的可能性越大 >*/
    }personvehiclepet_info;

    typedef struct {
        //mod = 0: Configure convex polygons
        IVSPoint *p;    /**< 周界各个顶点信息，不能有线交叉，各点形成的多边形须为凸多边形 >*/
        int pcnt;      /**<  周界顶点数量，最多8个点 >*/
        //mod = 1:Configure Rect
        IVSRect r;  /**< reserved >*/
        int detdist; /**< reserved >*/
        uint64_t alarm_last_time;    /**< reserved >*/
    }personvehiclepetdet_perm_t;

    /*
     * 人车检测输入结构体
     >*/
    typedef struct {
        bool ptime; /**< 是否打印检测时间 */
        unsigned int max_personvehiclepet_box; /**< 人车检测结果最多的框数量，若图片中检测目标多于max_personvehiclepet_box则随机输出max_personvehiclepet_box个框 >*/
        // int nmem_size; /**<uranus init时配置的nmem大小，单位兆，单跑一个算法或跑多进程时使用sample中给的默认值即可，若使用单进程多线程模式，需要配置算法所需的最大的nmem大小，例：单跑A需要9M，单跑B需要15M，AB跑单进程多线程模式，需要将A和B的nmem_size都配15M>*/
        bool multiprocess_lock; /** <置true可跑多进程，需配置nmem_size，不可为0，nmem_size大小需为4K的倍数> */
        int nmem_size; /**< uranus init时配置的nmem大小，单位兆，单跑一个算法或跑多进程时使用sample中给的默认值即可，若使用单进程多线程模式，需要配置算法所需的最大的nmem大小，例：单跑A需要9M，单跑B需要15M，AB跑单进程多线程模式，需要将A和B的nmem_size都配15M >*/
        const char* model_path; /** < 人车检测模型路径或地址，输入模型路径时，模型名字应以.bin为后缀；当输入模型地址时需配置模型大小即model_size >*/
        size_t model_size; /**< 模型大小，当model_path输入模型地址时生效 >*/
        bool check_model; /**< 检查模型MD5值，测试时打开，快启类上板可关闭，减少加载模型时间 >*/
        int sense; /** < 检测灵敏度 0~6 0:最不灵敏，误检率最低，正检会下降 6:最灵敏，正检率最高，误检相应增加 default:4 >*/
        int detdist; /** < 0:det_size=640 1:det_size=800 >*/
        int det_size; /** < 当detdist=-1时生效，可任意设置检测输入分辨率最大边大小，注意：1.det_size赋值时要是32的倍数，若不是算法里自动改为32倍数的数；2.算法流分辨率最大值要 >=det_size，不然图片会按最大边等比放大到det_size，导致图片失真影响检测效果 >*/
        bool enable_move; /** < 取值true/false; true: 开启移动侦测，输出移动目标区域坐标 false: 关闭移动侦测，不输出移动目标区域坐标 default: false >*/
        bool open_move_filter; /** < 取值true/false; true: 只输出移动人车目标区域坐标，需enable_move=ture; false: 输出全部移动和不移动人车目标 default: false >*/
        int move_sense; /** < 移动侦测的灵敏度，与目标大小远近无关，范围为0-4，0:最不灵敏，对移动目标位置变化小的不敏感 4:最灵敏，对移动目标最敏感 default 2 >*/
        int move_min_h; /** < 移动侦测物体的最小高度，default 20 >*/
        int move_min_w; /** < 移动侦测物体的最小宽度，default 20 >*/
        int move_sldwin_size; /** < 移动侦测save count，2~4, default 4 >*/
        int move_ids_size;  /** < 类型个数 >*/
        int move_ids[MOVE_IDS_SIZE];  /** < 指定类型个数和class_id号 >*/
        bool enable_perm; /**< 取值true/false; true: 开启周界功能，只输出指定区域内人车框 false: 关闭周界功能，输出所有人车框 default: false >*/
        personvehiclepetdet_perm_t perms[IVS_PERSONVEHICLEPETDET_PERM_MAX_ROI]; /** < 周界信息 >*/
        int permcnt; /** <  设定周界的数量，最多4个 >*/
        int mod; /** <  mod = 0: 检测多边形模式，检测整张图片，只输出多边形内人车信息 >*/
        int skip_num; /** < 跳帧数目，隔帧检测，每隔skip_num帧检测一次 >*/
        unsigned int delay; /** < reserved >*/
        IVSPhyFrameInfo frameInfo; /** < 帧信息，详见ivs_common.h >*/
        bool rot90; /** < reserved >*/
        bool switch_track; /** < 取值true/false; true: 开启跟踪，track_id 会输出跟踪ID号，相同ID视为同一个目标 false: 关闭跟踪，track_id=-1 不可用 >*/
        int track_lifecycle; /** < 跟踪生命周期，已跟踪目标(已存在的track_id)在lifecycle内没有检测到则不再继续跟踪，以帧为单位，默认值为30 >*/
        bool switch_stop_det; /** < 取值true/false; true: 关闭检测，输出人车检测结果，false: 开启检测，不输出人车检测结果 >*/
        bool fast_update_params; /* 快速设置更新switch_stop_det参数，不构造初始化，不释放已开辟资源 >*/
        int observation_period;  /**设置判断周期observation_period和有效个数active_count，默认为5和2，即连续5帧内有2帧结果isperson为true */
        int active_count;  /**设置判断周期observation_period和有效个数active_count，默认为5和2，即连续5帧内有2帧结果isperson为true */
        float fil_score; /**当目标得分>=fil_score时，isperson为true */
    }personvehiclepetdet_param_input_t;
    /*
     * 人车检测输出结构体
     */
    typedef struct {
        int count; /**< 检测人车的个数，小于等于max_personvehiclepet_box >*/
        int count_move; /**< 识别出移动目标的个数 >*/
        IVSRect move_rects[NUM_OF_MOVE]; /**< 识别出移动目标区域坐标 >*/
        personvehiclepet_info personvehiclepet[NUM_OF_PERSONVEHICLEPETS]; /**< 人车信息 >*/
        bool isperson; /**判断周期内是否有人形 */
        int64_t timeStamp; /**< 时间戳 >*/
    }personvehiclepetdet_param_output_t;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
#endif /* __IVS_INF_PERSONVEHICLEPETDET_H__ */
