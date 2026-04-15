#ifndef __ptz_H__
#define __ptz_H__
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>

/* motor status */
#define MOTOR_MOVE_STOP		0x0
#define MOTOR_MOVE_RUN		0x1

/* motor move range */
#define MOTOR_MAX_X 3200
#define MOTOR_MIN_X 0
#define MOTOR_MAX_Y 1100
#define MOTOR_MIN_Y 0

/* motor speed range */
#define MOTOR1_MAX_SPEED	1000
#define MOTOR1_MIN_SPEED	10

/* ioctl cmd */
#define MOTOR_STOP		0x1
#define MOTOR_RESET		0x2
#define MOTOR_MOVE		0x3
#define MOTOR_GET_STATUS	0x4
#define MOTOR_SPEED		0x5
#define MOTOR_GOBACK	0x6
#define MOTOR_CRUISE	0x7
#define MOTOR_XVSY  0x8

enum motor_status {
	MOTOR_STOP_STATUS,
	MOTOR_RUNNING_STATUS,
};

typedef struct motor_message {
	int x;
	int y;
	enum motor_status status;
	int speed;
}motor_ptz_message_t;

typedef struct motor_steps{
	int x;
	int y;
	int x_unit_steps;
	int y_unit_steps;
	int motor_speed;
}motor_steps_t;

typedef struct motor_ptz_reset_data {
	unsigned int x_max_steps;
	unsigned int y_max_steps;
	unsigned int x_cur_step;
	unsigned int y_cur_step;
}motor_reset_data_t;


/*  ptz interface  */
/**
 * @fn int xcam_ptz_init(void);
 *
 * 电机初始化.
 *
 * @retval 0 成功.
 * @retval 非0 失败.
 *
 * @remarks 操作云台之前需要初始化.
 *
 * @attention 无.
 */
int xcam_ptz_init(void);

/**
 * @fn int xcam_ptz_move(int x, int y);
 *
 * 移动
 *
 * @param[in] x 移动至某个x位置，最小单位是1 steps
 * @param[in] y 移动至某个y位置，最小单位是1 steps
 * @param[in] model 移动模式：1 为相对移动， 2为绝对移动。
 *
 * @retval 0 成功.
 * @retval 非0 失败.
 *
 * @remarks 移动至所需绝对位置.超过结构限制，则移动到能够移动的最大位置或最小位置
 *
 * @attention 无.
 */
int xcam_ptz_move(int x, int y, int model);

/**
 * @fn int xcam_ptz_set_speed(int speed);
 *
 * 设置速度
 *
 * @param[in] speed
 *
 * @retval 0 成功.
 * @retval 非0 失败.
 *
 * @remarks 设置电机速度，超过电机速度范围，则设置为最大或最小速度
 *
 * @attention 无.
 */
int xcam_ptz_set_speed(int speed);

/**
 * @fn int xcam_ptz_xvsy(int hmotor2vmotor);
 *
 * 设置x方向和y方向中断比
 *
 * @param[in] hmotor2vmotor
 *
 * @retval 0 成功.
 * @retval 非0 失败.
 *
 * @remarks 无
 *
 * @attention 无.
 */
int xcam_ptz_xvsy(int hmotor2vmotor);

/**
 * @fn xcam_ptz_reset(void);
 *
 * 云台复位，复位成功之后，电机移动到标准位置，初始化之后调用一次保证初始位置正确
 *
 * @retval 0 成功.
 * @retval 非0 失败.
 *
 * @remarks 设备长时间工作，可能有丢步，需要复位进行校准.
 *
 * @attention 无.
 */
int xcam_ptz_reset(void);

/**
 * @fn int xcam_ptz_stop(void);
 *
 * 云台停止运动
 *
 * @retval 0 成功.
 * @retval 非0 失败.
 *
 * @remarks 使电机立即停止运动
 *
 * @attention 无.
 */
int xcam_ptz_stop(void);

/**
 * @fn int xcam_ptz_get_status(struct motor_message *p_motor_message);
 *
 * 或者云台电机基本信息
 *
 * @param[in] p_motor_message 云台电机信息结构体
 *
 * @retval 0 成功.
 * @retval 非0 失败.
 *
 * @remarks 可获得当前电机的位置、速度等信息
 *
 * @attention 无.
 */
int xcam_ptz_get_status(struct motor_message *p_motor_message);

/**
 * @fn int xcam_ptz_cruise(void);
 *
 * 巡航
 *
 * @retval 0 成功.
 * @retval 非0 失败.
 *
 * @remarks 开启电机巡航功能，在此之前应当减小电机速度
 *
 * @attention 无.
 */
int xcam_ptz_cruise(void);

/**
 * @fn int xcam_ptz_goback(void);
 *
 * 复位
 *
 * @retval 0 成功.
 * @retval 非0 失败.
 *
 * @remarks 电机回到初始位置
 *
 * @attention 无.
 */
int xcam_ptz_goback(void);

/**
 * @fn int xcam_ptz_deinit(void);
 *
 * 云台关闭.
 *
 * @retval 0 成功.
 * @retval 非0 失败.
 *
 * @remarks 无.
 *
 * @attention 无.
 */
int xcam_ptz_deinit(void);

/* snap interface */
/**
 * @fn int xcam_snap_nv12(int channel);
 *
 * 截取nv12图片.
 *
 * @retval 0 成功.
 * @retval 非0 失败.
 *
 * @remarks 无.
 *
 * @attention 无.
 */
int xcam_snap_nv12(int channel);

#endif
