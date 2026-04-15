#ifndef __FUNC_AF_H__
#define __FUNC_AF_H__

#include <stdlib.h>

/* ioctl cmd */
#define MOTOR_STOP	     0x1
#define MOTOR_RESET	     0x2
#define MOTOR_MOVE	     0x3
#define MOTOR_GET_STATUS 0x4
#define MOTOR_SPEED	     0x5
#define MOTOR_GOBACK     0x6
#define MOTOR_CRUISE     0x7
#define MOTOR_SCALE	     0x8

enum motor_status_info {
	MOTOR_IS_STOP = 0,
	MOTOR_IS_RUNNING,
};

struct motors_steps{
	int x;
	int x_unit_steps;
	int y;
	int y_unit_steps;
	int motor_speed;
};

struct af_definition {
    int frameNum;
    int definValue;
};

struct af_ctrl_par {
    uint16_t zoomMaxSteps;		 /**<变倍最大步数*/
    uint16_t focusMaxSteps;      /**<变焦最大步数*/
};

struct motor_reset_data {
	unsigned int x_max_steps;
	unsigned int y_max_steps;
	unsigned int x_cur_step;
	unsigned int y_cur_step;
};

void func_af_focus_reset(void);
void func_af_focus_add(uint8_t step);
void func_af_focus_reduce(uint8_t step);
int (*pFunc_af_get_hist)(struct af_definition *afDefin);
void func_af_zoom_in(void);
void func_af_zoom_out(void);
void func_af_ctrl_init(void);
void func_af_ctrl_deinit(void);

//func_af_get_hist pFunc_af_get_hist;
#endif
