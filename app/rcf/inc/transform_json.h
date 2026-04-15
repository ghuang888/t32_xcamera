#ifndef _TRANSFORM_JSON_H_
#define _TRANSFORM_JSON_H_
#include <stdio.h>
#include <stdlib.h>

enum transform_type {
	PTZ_GET_STATUS = 0,
	PTZ_SET_SPEED,
	PTZ_SET_MOVE,
	AF_SET_ZOOM_IN,
	AF_SET_ZOOM_OUT,
	SNAP_NV12,
	PTZ_RESET,
	PTZ_CRUISE,
	AF_BACKWARD,
	AF_FORWARD,
	AF_RESET,
	PTZ_SET_XVSY,
};

typedef struct motor_set_message {
	int dx;
	int dy;
	int speed;
	int zoom_in;
	int zoom_out;
	int channel;
	int finetuning_step;
	int model;
	int hmotor2vmotor;
}motor_set_message_t;

char *transform_json(int, motor_set_message_t);

//ptz
char *transform_ptz_getstatus();
char *transform_ptz_speed(int speed);
char *transform_ptz_move(int dx, int dy, int model);
char *transform_ptz_reset();
char *transform_ptz_cruise();
char *transform_ptz_xvsy(int hmotor2vmotor);

//af
char *transform_af_zoom_in(int zoom_in);
char *transform_af_zoom_out(int zoom_out);
char *transform_af_reset();
char *transform_af_forward(int step);
char *transform_af_backward(int step);

//snap
char *transform_snap_nv12(int channel);

#endif
