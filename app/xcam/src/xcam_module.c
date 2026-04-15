#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include "xcam_module.h"
#define MSG_DATA_TYPE_OFFSET 16
static xcam_module_t *xmod[XMOD_ID_MAX] = {NULL};

void xcam_module_process_msg(int moduleid, msg_com_data_t *msg)
{
	int i = 0;
	int msgid = msg->id;
	char *msg_data = GET_MSGDATA(msg);
	int msg_type = *((int *)(msg_data + MSG_DATA_TYPE_OFFSET));
	// if(msg_data){
		// printf("err(%s,%d): xcam  modlule ID err %p %d\n", __func__, __LINE__, (msg_data + MSG_DATA_TYPE_OFFSET), *((int *)(msg_data + MSG_DATA_TYPE_OFFSET)));
	// }
	
	if (moduleid > XMOD_ID_MAX) {
		printf("err(%s,%d): xcam  modlule ID err %d\n", __func__, __LINE__,moduleid);
		return;
	}
	if (NULL == xmod[moduleid]) {
		printf("error(%s,%d): XMOD %d have't been created\n", __func__, __LINE__, moduleid);
		return;
	}
	if (msgid > MESG_ID_MAX) {
		printf("err(%s,%d): MSG ID err %d\n", __func__, __LINE__,msgid);
		return;
	}

	if (NULL != xmod[moduleid]->pfun){
		if(msg_type == MSG_VIDEO){
			xmod[moduleid]->pfun(msg);
		}else if(msg_type == MSG_AUDIO){
			// xcam_rtsp_process_audio_msg(msg);
		}
	}
		
	for (i = 0; i < XMOD_ID_MAX; i++) {
		if (true == xmod[moduleid]->forward_msg[msgid][i])
			xcam_module_process_msg(i, msg);
	}
}

void* xcam_module_alloc(char *name, int id, msg_process_f pfun)
{

	int i = 0;
	int size = 0;
	xcam_module_t *xm;
	if (id > XMOD_ID_MAX) {
		printf("err(%s,%d): xcam  modlule ID err %d\n", __func__, __LINE__,id);
		return NULL;
	}

	size = sizeof(xcam_module_t);
	xm = malloc(size);
	if (NULL == xm) {
		printf("err(%s,%d): xcam  modlule malloc err\n", __func__, __LINE__);
		return NULL;
	}
	memset(xm, 0, size);
	if (NULL != name) {
		i = strlen(name);
		if (i > XMOD_NAME_MAX_LEN) {
			printf("warn(%s,%d): XMOD name too long\n", __func__, __LINE__);
			i = XMOD_NAME_MAX_LEN;
		}
		strncpy(xm->name, name, i);
	}
	xm->work_list = msg_work_queue_alloc(XMOD_WORK_LIST_MAX_LEN);
	if (NULL == xm->work_list) {
		printf("err(%s,%d): xcam  modlule x list alloc failed\n", __func__, __LINE__);
		return NULL;
	}
	xm->id = id;
	xm->pfun = pfun;
	if (NULL != xmod[id]) {
		printf("warn(%s,%d): XMOD %d already created\n", __func__, __LINE__, id);
	}
	xmod[id] = xm;
	return xm;
}

void xcam_module_free(void *module, int id)
{
	if (id > XMOD_ID_MAX) {
		printf("err(%s,%d): xcam  modlule ID err %d\n", __func__, __LINE__,id);
		return;
	}
	if (module == xmod[id]) {
		printf("warn(%s,%d): XMOD %d already freed\n", __func__, __LINE__, id);
	}

	if (NULL == xmod[id]) {
		printf("warn(%s,%d): XMOD %d already freed\n", __func__, __LINE__, id);
	}
	free(xmod[id]);
	xmod[id] = NULL;
}

void xcam_module_register_wanted_msg(int moduleid, int forward_moduleid, int msgid)
{
	if (msgid > MESG_ID_MAX) {
		printf("err(%s,%d): MSG ID err %d\n", __func__, __LINE__,msgid);
		return;
	}

	if (moduleid > XMOD_ID_MAX) {
		printf("err(%s,%d): xcam  modlule ID err %d\n", __func__, __LINE__,moduleid);
		return;
	}
	if (NULL == xmod[moduleid]) {
		printf("error(%s,%d): XMOD %d have't been created\n", __func__, __LINE__, moduleid);
		return;
	}
	if (forward_moduleid > XMOD_ID_MAX) {
		printf("err(%s,%d): xcam  modlule ID err %d\n", __func__, __LINE__,forward_moduleid);
		return;
	}
	if (NULL == xmod[forward_moduleid]) {
		printf("error(%s,%d): XMOD %d have't been created\n", __func__, __LINE__, forward_moduleid);
		return;
	}

	xmod[moduleid]->forward_msg[msgid][forward_moduleid] = true;
}


void xcam_module_unregister_wanted_msg(int moduleid, int forward_moduleid, int msgid)
{
	if (msgid > MESG_ID_MAX) {
		printf("err(%s,%d): MSG ID err %d\n", __func__, __LINE__,msgid);
		return;
	}

	if (moduleid > XMOD_ID_MAX) {
		printf("err(%s,%d): xcam  modlule ID err %d\n", __func__, __LINE__,moduleid);
		return;
	}
	if (NULL == xmod[moduleid]) {
		printf("error(%s,%d): XMOD %d have't been created\n", __func__, __LINE__, moduleid);
		return;
	}
	if (forward_moduleid > XMOD_ID_MAX) {
		printf("err(%s,%d): xcam  modlule ID err %d\n", __func__, __LINE__,forward_moduleid);
		return;
	}
	if (NULL == xmod[forward_moduleid]) {
		printf("error(%s,%d): XMOD %d have't been created\n", __func__, __LINE__, forward_moduleid);
		return;
	}

	xmod[moduleid]->forward_msg[msgid][forward_moduleid] = false;
}

#if 0
enum {
	XMOD_ID_VIDEO_STREAM_0,
	XMOD_ID_VIDEO_STREAM_1,
	XMOD_ID_VIDEO_RTSP_0,
	XMOD_ID_VIDEO_RTSP_1,
	XMOD_ID_TFCARD_SAVE,
};

enum {
	MESG_ID_STREAM0,
	MESG_ID_STREAM1,
};


static int test_video0_process_msg(struct msg_com_data_s *msg)
{
	printf("msg process: id: %d, %s\n", msg->id, __func__);
	return 0;
}

static int test_video1_process_msg(struct msg_com_data_s *msg)
{
	printf("msg process: id: %d, %s\n", msg->id, __func__);
	return 0;
}
static int test_rtsp0_process_msg(struct msg_com_data_s *msg)
{
	printf("msg process: id: %d, %s\n", msg->id, __func__);
	return 0;
}
static int test_rtsp1_process_msg(struct msg_com_data_s *msg)
{
	printf("msg process: id: %d, %s\n", msg->id, __func__);
	return 0;
}
static int test_tfcard_process_msg(struct msg_com_data_s *msg)
{
	printf("msg process: id: %d, %s\n", msg->id, __func__);
	return 0;
}

static int test1()
{
	int i = 0;
	void *xmod_video0 = xcam_module_alloc("video0", XMOD_ID_VIDEO_STREAM_0, test_video0_process_msg);
	void *xmod_video1 = xcam_module_alloc("video1", XMOD_ID_VIDEO_STREAM_1, test_video1_process_msg);
	void *xmod_tfcard_save = xcam_module_alloc("tfcard_save", XMOD_ID_TFCARD_SAVE, test_tfcard_process_msg);
	void *xmod_rtsp0 = xcam_module_alloc("rtsp0", XMOD_ID_VIDEO_RTSP_0, test_rtsp0_process_msg);
	void *xmod_rtsp1 = xcam_module_alloc("rtsp1", XMOD_ID_VIDEO_RTSP_1, test_rtsp1_process_msg);
	xcam_module_register_wanted_msg(XMOD_ID_VIDEO_STREAM_0, XMOD_ID_VIDEO_RTSP_0, MESG_ID_STREAM0);
	xcam_module_register_wanted_msg(XMOD_ID_VIDEO_STREAM_0, XMOD_ID_TFCARD_SAVE, MESG_ID_STREAM0);
	xcam_module_register_wanted_msg(XMOD_ID_VIDEO_STREAM_1, XMOD_ID_VIDEO_RTSP_1, MESG_ID_STREAM1);
	msg_com_data_t msg0 = {
		.id = MESG_ID_STREAM0,
	};
	msg_com_data_t msg1 = {
		.id = MESG_ID_STREAM1,
	};
	for (i = 0; i < 100; i++) {
		xcam_module_process_msg(XMOD_ID_VIDEO_STREAM_0, &msg0);
		xcam_module_process_msg(XMOD_ID_VIDEO_STREAM_1, &msg1);
	}
}

int main(int argc, char **argv)
{
	test1();
	return 0;
}

#endif
