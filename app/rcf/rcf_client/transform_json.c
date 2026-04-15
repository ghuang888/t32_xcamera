#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "transform_json.h"

motor_set_message_t g_set_message;
char g_json[512] = {0};

static char *_transform_ptz_speed(motor_set_message_t *pmessage)
{
	memset(g_json,0,sizeof(g_json));
	char json[] =
		"{\n"
			"\t\"configlist\" : {\n"
				"\t\t\"conf_ptz_set_speed\" : \"conf_ptz_set_speed\",\n"
				"\t\t\"action\" : 1\n"
			"\t},\n"
			"\t\"conf_ptz_set_speed\": {\n"
				"\t\t\"speed\":%d\n"
			"\t}\n"
		"}\n";
	sprintf(g_json,json,pmessage->speed);
	return g_json;
}

static char *_transform_af_zoom_in(motor_set_message_t *pmessage)
{
	memset(g_json,0,sizeof(g_json));
	char json[]=
		"{\n"
			"\t\"configlist\" : {\n"
				"\t\t\"conf_af_set_in\" : \"conf_af_set_in\",\n"
				"\t\t\"action\" : 1\n"
			"\t},\n"
			"\t\"conf_af_set_in\": {\n"
				"\t\t\"in_steps\":%d\n"
			"\t}\n"
		"}\n";
	sprintf(g_json,json,pmessage->zoom_in);
	return g_json;
}

static char *_transform_af_zoom_out(motor_set_message_t *pmessage)
{
	memset(g_json,0,sizeof(g_json));
	char json[] =
		"{\n"
			"\t\"configlist\" : {\n"
				"\t\t\"conf_af_set_out\" : \"conf_af_set_out\",\n"
				"\t\t\"action\" : 1\n"
			"\t},\n"
			"\t\"conf_af_set_out\": {\n"
				"\t\t\"out_steps\":%d\n"
			"\t}\n"
		"}\n";
	sprintf(g_json,json,pmessage->zoom_out);
	return g_json;
}

static char *_transform_ptz_move(motor_set_message_t *pmessage)
{
	memset(g_json,0,sizeof(g_json));
	char json[] =
		"{\n"
			"\t\"configlist\" : {\n"
				"\t\t\"conf_ptz_set_move\" : \"conf_ptz_set_move\",\n"
				"\t\t\"action\" : 1\n"
			"\t},\n"
			"\t\"conf_ptz_set_move\": {\n"
				"\t\t\"dx\":%d,\n"
				"\t\t\"dy\":%d,\n"
				"\t\t\"model\":%d\n"
			"\t}\n"
		"}\n";
	sprintf(g_json,json,pmessage->dx,pmessage->dy,pmessage->model);
	return g_json;
}

static char *_transform_ptz_getstatus()
{
	memset(g_json,0,sizeof(g_json));
	char json[] =
		"{\n"
			"\t\"configlist\" : {\n"
				"\t\t\"conf_ptz_get_motorstatus\" : \"conf_ptz_get_motorstatus\",\n"
				"\t\t\"action\" : 0\n"
			"\t}\n"
		"}\n";
	sprintf(g_json,json);
	return g_json;
}

static char *_transform_ptz_reset()
{
	memset(g_json,0,sizeof(g_json));
	char json[] =
		"{\n"
			"\t\"configlist\" : {\n"
				"\t\t\"conf_ptz_reset\" : \"conf_ptz_reset\",\n"
				"\t\t\"action\" : 1\n"
			"\t},\n"
			"\t\"conf_ptz_reset\": {\n"
			"\t}\n"
		"}\n";
	sprintf(g_json,json);
	return g_json;
}

static char *_transform_ptz_cruise()
{
	memset(g_json,0,sizeof(g_json));
	char json[] =
		"{\n"
			"\t\"configlist\" : {\n"
				"\t\t\"conf_ptz_cruise\" : \"conf_ptz_cruise\",\n"
				"\t\t\"action\" : 1\n"
			"\t},\n"
			"\t\"conf_ptz_cruise\": {\n"
			"\t}\n"
		"}\n";
	sprintf(g_json,json);
	return g_json;
}

static char *_transform_af_backward(motor_set_message_t *pmessage)
{
	memset(g_json,0,sizeof(g_json));
	char json[] =
		"{\n"
			"\t\"configlist\" : {\n"
				"\t\t\"conf_af_backward\" : \"conf_af_backward\",\n"
				"\t\t\"action\" : 1\n"
			"\t},\n"
			"\t\"conf_af_backward\": {\n"
				"\t\t\"finetuning_step\":%d\n"
			"\t}\n"
		"}\n";
	sprintf(g_json,json,pmessage->finetuning_step);
	return g_json;
}

static char *_transform_af_forward(motor_set_message_t *pmessage)
{
	memset(g_json,0,sizeof(g_json));
	char json[] =
		"{\n"
			"\t\"configlist\" : {\n"
				"\t\t\"conf_af_forward\" : \"conf_af_forward\",\n"
				"\t\t\"action\" : 1\n"
			"\t},\n"
			"\t\"conf_af_forward\": {\n"
				"\t\t\"finetuning_step\":%d\n"
			"\t}\n"
		"}\n";
	sprintf(g_json,json,pmessage->finetuning_step);
	return g_json;
}

static char *_transform_af_reset()
{
	memset(g_json,0,sizeof(g_json));
	char json[] =
		"{\n"
			"\t\"configlist\" : {\n"
				"\t\t\"conf_af_reset\" : \"conf_af_reset\",\n"
				"\t\t\"action\" : 1\n"
			"\t},\n"
			"\t\"conf_af_reset\": {\n"
			"\t}\n"
		"}\n";
	sprintf(g_json,json);
	return g_json;
}

static char *_transform_ptz_xvsy(motor_set_message_t *pmessage)
{
	memset(g_json,0,sizeof(g_json));
	char json[] =
		"{\n"
			"\t\"configlist\" : {\n"
				"\t\t\"conf_ptz_xvsy\" : \"conf_ptz_xvsy\",\n"
				"\t\t\"action\" : 1\n"
			"\t},\n"
			"\t\"conf_ptz_xvsy\": {\n"
				"\t\t\"hmotor2vmotor\":%d\n"
			"\t}\n"
		"}\n";
	sprintf(g_json,json,pmessage->hmotor2vmotor);
	return g_json;
}

static char *_transform_snap_nv12(motor_set_message_t *pmessage)
{
	memset(g_json,0,sizeof(g_json));
	char json[] =
		"{\n"
			"\t\"configlist\" : {\n"
				"\t\t\"snap_nv12\" : \"snap_nv12\",\n"
				"\t\t\"action\" : 2\n"
			"\t},\n"
			"\t\"snap_nv12\": {\n"
				"\t\t\"channel\":%d\n"
			"\t}\n"
		"}\n";
	sprintf(g_json,json,pmessage->channel);
	return g_json;
}

char *transform_json(int type,motor_set_message_t *pmessage)
{
	char *json = NULL;
	if(type < 0)
		printf("This type is NULL\n");
	switch(type)
	{
		case 0:
			json = _transform_ptz_getstatus();
			break;
		case 1:
			json = _transform_ptz_speed(pmessage);
			break;
		case 2:
			json = _transform_ptz_move(pmessage);
			break;
		case 3:
			json = _transform_af_zoom_in(pmessage);
			break;
		case 4:
			json = _transform_af_zoom_out(pmessage);
			break;
		case 5:
			json = _transform_snap_nv12(pmessage);
			break;
		case 6:
			json = _transform_ptz_reset();
			break;
		case 7:
			json = _transform_ptz_cruise();
			break;
		case 8:
			json = _transform_af_backward(pmessage);
			break;
		case 9:
			json = _transform_af_forward(pmessage);
			break;
		case 10:
			json = _transform_af_reset();
			break;
		case 11:
			json = _transform_ptz_xvsy(pmessage);
			break;

	}
	return json;
}

char *transform_ptz_getstatus()
{
	char *result = transform_json(PTZ_GET_STATUS, &g_set_message);
	return result;
}

char *transform_ptz_speed(int speed)
{
	if (speed == 0)
		printf("This speed is NULL\n");
	g_set_message.speed = speed;
	char *result = transform_json(PTZ_SET_SPEED, &g_set_message);
	return result;
}

char *transform_ptz_move(int dx, int dy, int model)
{
	g_set_message.dx = dx;
	g_set_message.dy = dy;
	g_set_message.model = model;
	char *result = transform_json(PTZ_SET_MOVE, &g_set_message);
	return result;
}

char *transform_af_zoom_in(int zoom_in)
{
	g_set_message.zoom_in = zoom_in;
	char *result = transform_json(AF_SET_ZOOM_IN, &g_set_message);
	return result;
}

char *transform_af_zoom_out(int zoom_out)
{
	g_set_message.zoom_out = zoom_out;
	char *result = transform_json(AF_SET_ZOOM_OUT, &g_set_message);
	return result;
}

char *transform_af_backward(int finetuning_step)
{
	g_set_message.finetuning_step = finetuning_step;
	char *result = transform_json(AF_BACKWARD, &g_set_message);
	return result;
}

char *transform_af_forward(int finetuning_step)
{
	g_set_message.finetuning_step = finetuning_step;
	char *result = transform_json(AF_FORWARD, &g_set_message);
	return result;
}

char *transform_af_reset()
{
	char *result = transform_json(AF_RESET, &g_set_message);
	return result;
}

char *transform_ptz_xvsy(int hmotor2vmotor)
{
	g_set_message.hmotor2vmotor = hmotor2vmotor;
	char *result = transform_json(PTZ_SET_XVSY, &g_set_message);
	return result;
}

char *transform_snap_nv12(int channel)
{
	g_set_message.channel = channel;
	char *result = transform_json(SNAP_NV12, &g_set_message);
	return result;
}

char *transform_ptz_reset()
{
	char *result = transform_json(PTZ_RESET, &g_set_message);
	return result;
}

char *transform_ptz_cruise()
{
	char *result = transform_json(PTZ_CRUISE, &g_set_message);
	return result;
}
