#ifndef __IVS_INF_PLATEREC_H__
#define __IVS_INF_PLATEREC_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */

#include "ivs_common.h"

#define PLATEREC_VERSION_NUM 0x000001010
#define NUM_OF_PLATES 10
#define NUM_OF_VEHICLES 10
#define LEN_OF_COLOR 7
#define LEN_OF_LICENCE 19

uint32_t platerec_get_version_info(void);

typedef struct {
    IVSRect box; /**< reserved */
    IVSRect show_box;  /**< 车辆区域坐标 */
    //int class_id; /**< 类别号 0:车辆 1:车牌 >*/
    int type;/**<-1:close switch_plate_vehtype 0:car 1:bus 2:truck*/
    int color;/**<-1:close switch_plate_vehcolor 0:black 1:white 2:blue 3:green 4:yellow 5:orange 6:red 7:brown 8:gray 9:pink 10:purple */
    int track_id; /**< 跟踪ID>*/
    float confidence; /**< 车辆检测结果的置信度 */
}vehicle_info;

typedef struct {
    IVSRect box; /**< reserved */
    IVSRect show_box;  /**< 车辆车牌区域坐标 */
    //int class_id; /**< 类别号 0:车辆 1:车牌 >*/
    int track_id; /**< 跟踪ID>*/
    int color; /**<-1:close switch_plate_placolor 0:black 1:white 2:blue 3:green 4:yellow>*/
    int type;  /**<-1:close switch_plate_platype 0:single 1:double */
    int count; /**< 车牌号位数(7/8) */
    const char* licence; /**< 车牌号 */
    float confidence; /**< 车牌识别结果的置信度 */
    int reg_state; /**<  reserved 0~2, 0:车牌检测状态(检测); \
                      1:车牌识别状态(检测+识别);                        \
                      2:未满足车牌识别高度和角度条件,车牌识别异常状态(检测); */
    //int landmark4[8];
}plate_info;

/*
 * 车牌识别输入结构体
 */
typedef struct {
    bool ptime; /**< 是否打印识别时间 */
    unsigned int max_vehicle_box; /**< 车辆框数量 */
    unsigned int max_plate_box; /**< 车牌框数量 */
    const char* model_path; /**< 模型路径 */
    const char* model_path_ldmk; /**< 车牌关键点模型路径 */
    const char* model_path_vehtype; /**< 车辆类型模型路径 */
    const char* model_path_vehcolor; /**<车辆颜色模型路径 */
    const char* model_path_placolor; /**< 车牌颜色模型路径 */
    const char* model_path_platype; /**< 车牌类型模型路径 */
    const char* model_path_rec; /**< 车牌识别模型路径 */
    bool rot90; /**< 图像是否顺时针旋转90度*/
    int nmem_size; /**<配置参数nmem_size，即init时配的nmem大小，单位Byte，应为整数，
    单跑一个算法或跑多进程时使用sample中给的默认值即可，若使用单进程多线程模式，需要配置算法所需的最大的nmem大小，
    例：单跑算法A需要9M，单跑算法B需要15M，AB算法跑单进程多线程模式，需要将A和B的nmem_size都配15M>**/
    bool switch_track;  /**< 取值true/false; true :开启跟踪，track_id 会输出跟踪ID号，false: 关闭跟踪，track_id=-1 不可用 */
    bool switch_plate_rec; /**< 取值true/false; true :开启车牌识别 false: 关闭车牌识别 */
    bool switch_plate_vehtype;/**< 取值true/false; true :开启车辆类型 false: 关闭车辆类型 */
    bool switch_plate_vehcolor;/**< 取值true/false; true :开启车辆颜色 false: 关闭车辆颜色 */
    bool switch_plate_placolor;/**< 取值true/false; true :开启车牌颜色 false: 关闭车牌颜色 */
    bool switch_plate_platype;/**< 取值true/false; true :开启车牌类型 false: 关闭车牌类型 */
    bool is_roi; /**< reserved 输入的图像是否为检测区域的ROI */
    bool is_bgra;  /**< reserved 输入的图像格式是否为BGRA */
    int skip_num; /**< reserved 跳帧数目 */
    unsigned int delay; /**< reserved 延时识别时间，默认为0 */
    IVSRect win; /**< reserved 车牌识别区域坐标信息 */
    IVSFrameInfo frameInfo; /**< 帧信息 */
    bool enable_move; /**< reserved 是否使用move,打开只识别动的物体 */
    bool enable_perm; /**<reserved */
    int sense; /**< reserved 检测灵敏度 0~4 0:最不灵敏 4:最灵敏 default:2 */
    int detdist; /**< reserved 检测距离 0~3 */
    int detect_buff_count; /**< reserved 车牌检测buff长度(>0), 决定车牌识别频率, 默认3, 即每三次检测进行一次识别 */
    int min_reg_height; /**< 进行车牌识别最小高度(>=50), 默认50 */
    int max_reg_angle; /**< 车牌识别最大角度(roll), 默认20度 */

}platerec_param_input_t;

/*
 * 车牌识别检测输出结构体
 */
typedef struct {
    int vehicle_count; /**< 检测车辆的个数 */
    int plate_count; /**< 车牌识别的个数 */
    vehicle_info vehicle[NUM_OF_VEHICLES]; /**< 车辆信息 */
    plate_info plate[NUM_OF_PLATES]; /**< 车牌信息 */
    int64_t timeStamp; /**< 时间戳 */
}platerec_param_output_t;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
#endif /* __IVS_INF_PLATEREC_H__ */
