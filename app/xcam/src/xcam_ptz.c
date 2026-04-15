#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <imp/imp_log.h>
#include <imp/imp_common.h>
#include <imp/imp_system.h>
#include <imp/imp_framesource.h>
#include <imp/imp_encoder.h>
#include "xcam_log.h"
#include "xcam_ptz.h"

#define CHARACTOR_DEVICE_NODE  "/dev/motor_ptz"
#define LOG_TAG "PTZ"

static int fd_flag = -1;
motor_ptz_message_t motor_message_attr;
motor_steps_t motor_steps_attr;
motor_reset_data_t motor_reset_data_attr;

static int _xcam_ptz_opennode()
{
	fd_flag = open(CHARACTOR_DEVICE_NODE, 0);
	if (fd_flag < 0)
	{
		LOG_ERR(LOG_TAG,"Open node failed!\n");
		return -1;
	}

	return 0;
}

static int _xcam_ptz_get_status(struct motor_message *p_motor_message)
{
	int ret = -1;

	ret = ioctl(fd_flag, MOTOR_GET_STATUS, (unsigned long)p_motor_message);
	if (ret < 0) {
		LOG_ERR(LOG_TAG,"Get motor ptz status failed!\n");
	} else {
		/******printf motor info******/
		printf("xcurrent_steps=%d\n",p_motor_message->x);
		printf("ycurrent_steps=%d\n",p_motor_message->y);
		//printf("cur_status=%d\n",p_motor_message->status);
		printf("cur_speed=%d\n",p_motor_message->speed);
	}

	return ret;
}

static int _xcam_ptz_move(int x,int y,int model)
{
	int ret = -1, current_x = 0, current_y = 0;

	ret = xcam_ptz_get_status(&motor_message_attr);
	if (ret < 0)
	{
		LOG_ERR(LOG_TAG,"Get status failed!\n");
		return ret;
	}
	current_x = motor_message_attr.x;
	current_y = motor_message_attr.y;
	switch(model){
		case 1:
			/*  Incoming relative position shift  */
			if (current_x + x > MOTOR_MAX_X || current_x + x < MOTOR_MIN_X)
			{
				LOG_ERR(LOG_TAG,"Moving horizontal coordinates out of maximum or minimum range\n");
				if (current_x + x > MOTOR_MAX_X)
					motor_steps_attr.x = MOTOR_MAX_X;
				if (current_x + x < MOTOR_MIN_X)
					motor_steps_attr.x = MOTOR_MIN_X;
			} else {
				motor_steps_attr.x = x;
			}
			if (current_y + y > MOTOR_MAX_Y || current_y + y < MOTOR_MIN_Y)
			{
				LOG_ERR(LOG_TAG,"Moving vertical coordinates out of maximum or minimum range\n");
				if (current_y + y > MOTOR_MAX_Y)
					motor_steps_attr.y = MOTOR_MAX_Y;
				if (current_y + y < MOTOR_MIN_Y)
					motor_steps_attr.y = MOTOR_MIN_Y;
			} else {
				motor_steps_attr.y = y;
			}

			break;
		case 2:
			/*	Incoming absolute position shift  */
			if(x > MOTOR_MAX_X) {
				x = MOTOR_MAX_X;
				LOG_WAN(LOG_TAG,"The position in X direction exceeds the maximum value!\n");
			}
			if(x < MOTOR_MIN_X) {
				x = MOTOR_MIN_X;
				LOG_WAN(LOG_TAG,"The position in X direction exceeds the minimum value!\n");
			}
			if(y > MOTOR_MAX_Y) {
				y = MOTOR_MAX_Y;
				LOG_WAN(LOG_TAG,"The position in Y direction exceeds the maximum value!\n");
			}
			if(y < MOTOR_MIN_Y) {
				y = MOTOR_MIN_Y;
				LOG_WAN(LOG_TAG,"The position in Y direction exceeds the minimum value!\n");
			}
			motor_steps_attr.x = x - current_x;
			motor_steps_attr.y = y - current_y;

			break;
	}

	printf("current_x = %d, current_y  = %d, move to (%d,%d), x move %d, y move %d", current_x, current_y, x, y, motor_steps_attr.x, motor_steps_attr.y);

	ret = ioctl(fd_flag, MOTOR_MOVE, (unsigned long)&motor_steps_attr);
	if (ret < 0)
	{
		LOG_ERR(LOG_TAG,"Move failed!\n");
		return ret;
	}

	return ret;
}

static int _xcam_ptz_set_speed(int speed)
{
	int ret = -1;

	if (speed > MOTOR1_MAX_SPEED) {
		speed = MOTOR1_MAX_SPEED;
		LOG_WAN(LOG_TAG,"The speed exceeds the maximum value. The speed has been set to the maximum value!\n");
	}
	if (speed < MOTOR1_MIN_SPEED) {
		speed = MOTOR1_MIN_SPEED;
		LOG_WAN(LOG_TAG,"The speed exceeds the minimum value. The speed has been set to the minimum value!\n");
	}

	ret = ioctl(fd_flag,MOTOR_SPEED,(unsigned long)&speed);
	if (ret < 0)
	{
		LOG_ERR(LOG_TAG,"Set speed failed!\n");
		return ret;
	}

	return ret;
}

static int _xcam_ptz_reset()
{
	int ret = -1;
	memset(&motor_reset_data_attr, 0, sizeof(motor_reset_data_attr));
/*
	_xcam_ptz_get_status(&motor_message_attr);
	motor_reset_data_attr.x_max_steps = MOTOR_MAX_X;
	motor_reset_data_attr.y_max_steps = MOTOR_MAX_Y;
	motor_reset_data_attr.x_cur_step = motor_message_attr.x;
	motor_reset_data_attr.x_cur_step = motor_message_attr.y;
*/
	ret = ioctl(fd_flag, MOTOR_RESET, &motor_reset_data_attr);
	printf("x_max_steps=%d,y_max_steps=%d,x_cur_step=%d,y_cur_step=%d\n",
			motor_reset_data_attr.x_max_steps,motor_reset_data_attr.y_max_steps,
			motor_reset_data_attr.x_cur_step,motor_reset_data_attr.y_cur_step);
	if (ret < 0)
	{
		LOG_ERR(LOG_TAG,"Reset failed!\n");
		return ret;
	}

	return ret;
}

static int _xcam_ptz_stop()
{
	int ret = -1;

	ret = ioctl(fd_flag, MOTOR_STOP, 0);
	if (ret < 0)
	{
		LOG_ERR(LOG_TAG,"Stop failed!\n");
		return ret;
	}

	return ret;
}

static int _xcam_ptz_cruise()
{
	int ret = -1;

	ret = ioctl(fd_flag, MOTOR_CRUISE, 0);
	if (ret < 0)
	{
		LOG_ERR(LOG_TAG,"Cruise failed!\n");
		return ret;
	}

	return ret;
}

static int _xcam_ptz_goback()
{
	int ret = -1;

	ret = ioctl(fd_flag, MOTOR_GOBACK, 0);
	if (ret < 0)
	{
		LOG_ERR(LOG_TAG,"Goback failed!\n");
		return ret;
	}

	return ret;
}

static int _xcam_ptz_xvsy(int hmotor2vmotor)
{
	int ret = -1;

	ret = ioctl(fd_flag,MOTOR_XVSY,(unsigned long)&hmotor2vmotor);
	if (ret < 0)
	{
		LOG_ERR(LOG_TAG,"Set hmotor2vmotor failed!\n");
		return ret;
	}

	return ret;
}

static int _xcam_snap_nv12(int channel)
{
	int ret = -1,m = 0;
	IMPFrameInfo *frame_bak;

	FILE *fp;
	fp = fopen("/tmp/snap.yuv", "wb");
	if(fp == NULL) {
		LOG_ERR(LOG_TAG,"Open snap.yuv failed\n");
		return ret;
	}

	/* Snap YUV Start */
	ret = IMP_FrameSource_SetFrameDepth(channel, 1);
	if (ret < 0) {
		IMP_LOG_ERR(LOG_TAG, "%s(%d):IMP_FrameSource_SetFrameDepth failed\n", __func__, __LINE__);
		return -1;
	}

	for (m = 1; m <= 51; m++) {
		ret = IMP_FrameSource_GetFrame(channel, &frame_bak);
		if (ret < 0) {
			IMP_LOG_ERR(LOG_TAG, "%s(%d):IMP_FrameSource_GetFrame failed\n", __func__, __LINE__);
			return -1;
		}
		if(m % 50 == 0) {
			fwrite((void *)frame_bak->virAddr, frame_bak->size, 1, fp);
			fclose(fp);
		}

		IMP_FrameSource_ReleaseFrame(channel, frame_bak);
		if (ret < 0) {
			IMP_LOG_ERR(LOG_TAG, "%s(%d):IMP_FrameSource_ReleaseFrame failed\n", __func__, __LINE__);
			return -1;
		}
	}
	ret = IMP_FrameSource_SetFrameDepth(channel, 0);
	if (ret < 0) {
		IMP_LOG_ERR(LOG_TAG, "%s(%d):IMP_FrameSource_SetFrameDepth failed\n", __func__, __LINE__);
		return -1;
	}

	return ret;
}

int xcam_ptz_init()
{
	int ret = -1;

	if (fd_flag < 0)
		_xcam_ptz_opennode();

	ret =  xcam_ptz_reset();
	if (ret < 0)
	{
		LOG_ERR(LOG_TAG,"Get status failed!\n");
		return ret;
	}

	ret = xcam_ptz_get_status(&motor_message_attr);
	if (ret < 0)
	{
		LOG_ERR(LOG_TAG,"Get status failed!\n");
		return ret;
	}

	return ret;
#if 0
	ret =  xcam_ptz_set_speed(300);
	if (ret < 0)
	{
		LOG_ERR(LOG_TAG,"Set speed failed!\n");
		return ret;
	}

	ret =  xcam_ptz_move(1000,100);
	if (ret < 0)
	{
		LOG_ERR(LOG_TAG,"Move failed!\n");
		return ret;
	}

	ret = xcam_ptz_goback();
	if (ret < 0)
	{
		LOG_ERR(LOG_TAG,"Goback failed!\n");
		return ret;
	}

	ret = xcam_ptz_stop();
	if (ret < 0)
	{
		LOG_ERR(LOG_TAG,"Stop failed!\n");
		return ret;
	}

	ret = xcam_ptz_cruise();
	if (ret < 0)
	{
		LOG_ERR(LOG_TAG,"Cruise failed!\n");
		return ret;
	}
#endif
}

static int _xcam_ptz_deinit()
{
	int ret = -1;
	ret = ioctl(fd_flag, MOTOR_STOP, 0);
	if(ret < 0){
		close(fd_flag);
		printf(" motor ioctl failed!\n");
		return -1;
	}
	close(fd_flag);
	fd_flag = -1;
	return 0;
}

int xcam_ptz_move(int x,int y,int model)
{
	return _xcam_ptz_move(x,y,model);
}

int xcam_ptz_set_speed(int speed)
{
	return _xcam_ptz_set_speed(speed);
}

int xcam_ptz_reset()
{
	return _xcam_ptz_reset();
}

int xcam_ptz_stop()
{
	return _xcam_ptz_stop();
}

int xcam_ptz_get_status(struct motor_message *p_motor_message)
{
	return _xcam_ptz_get_status(p_motor_message);
}

int xcam_ptz_cruise()
{
	return _xcam_ptz_cruise();
}

int xcam_ptz_goback()
{
	return _xcam_ptz_goback();
}

int xcam_ptz_xvsy(int hmotor2vmotor)
{
	return _xcam_ptz_xvsy(hmotor2vmotor);
}

int xcam_ptz_deinit()
{
	return _xcam_ptz_deinit();
}

int xcam_snap_nv12(int channel)
{
	return _xcam_snap_nv12(channel);
}
