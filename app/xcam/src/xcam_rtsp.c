#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <imp/imp_common.h>
#include <imp/imp_encoder.h>
#include "xcam_com.h"
#include "xcam_log.h"
#include "xcam_module.h"
#include "xcam_stream.h"
#include "xcam_rtsp.h"
#include "rtsp/video_source.h"
#include "sys/time.h"
#include "xcam_thread.h"
#include "rtsp/c_liveRTSP.h"
#include "xcam_conf_video.h"
#include "xcam_audio_stream.h"
#include "xcam_conf_video.h"

static int rtsp_init_flag = 0;
int xcam_rtsp_process_msg_impl(msg_stream_t *smsg, int msg_id);
int xcam_rtsp_process_msg(msg_com_data_t *msg);

static int frame_recv_rtsp_init(void)
{
	int ret = 0;
	int enctype_0 = LIVE_RTSP_STREAM_H265;
	int enctype_1 = LIVE_RTSP_STREAM_H265;
	if(xcam_video_conf.conf_enc.ch0_type == PT_H264) {
		enctype_0 = LIVE_RTSP_STREAM_H264;
	} else if (xcam_video_conf.conf_enc.ch0_type == PT_H265) {
		enctype_0 = LIVE_RTSP_STREAM_H265;
	}

	if(xcam_video_conf.conf_enc.ch1_type == PT_H264) {
		enctype_1 = LIVE_RTSP_STREAM_H264;
	} else if (xcam_video_conf.conf_enc.ch1_type == PT_H265) {
		enctype_1 = LIVE_RTSP_STREAM_H265;
	}

	ret = c_RTSP_init();
	if (0 != ret) {
		printf("rtsp: %s,%d, error\n", __func__, __LINE__);
		return ret;
	}

	ret = c_RTSP_set_stream_type(0, enctype_0);
	if (0 != ret) {
		printf("rtsp: %s,%d, error\n", __func__, __LINE__);
		return ret;
	}
	ret = c_RTSP_set_stream_type(1, enctype_1);
	if (0 != ret) {
		printf("rtsp: %s,%d, error\n", __func__, __LINE__);
		return ret;
	}
#ifdef XCAM_DOUBLE_SENSOR
	ret = c_RTSP_set_stream_type(3, enctype_0);
	if (0 != ret) {
		printf("rtsp: %s,%d, error\n", __func__, __LINE__);
		return ret;
	}
#endif
	ret = c_RTSP_start(0, SENSOR_RESOLUTION_WIDTH_MAIN ,SENSOR_RESOLUTION_HEIGHT_MAIN);
	if (0 != ret) {
		printf("rtsp: %s,%d, error\n", __func__, __LINE__);
		return ret;
	}
	ret = c_RTSP_start(1, SENSOR_RESOLUTION_WIDTH_SLAVE, SENSOR_RESOLUTION_HEIGHT_SLAVE);
	if (0 != ret) {
		printf("rtsp: %s,%d, error\n", __func__, __LINE__);
		return ret;
	}
#ifdef XCAM_DOUBLE_SENSOR
	ret = c_RTSP_start(3, SENSOR_RESOLUTION_WIDTH_MAIN ,SENSOR_RESOLUTION_HEIGHT_MAIN);
	if (0 != ret) {
		printf("rtsp: %s,%d, error\n", __func__, __LINE__);
		return ret;
	}
#endif

	return 0;
}

static int frame_recv_rtsp_waitready(int channel)
{
	return 0;
}

static int frame_recv_rtsp_putframe(int channel, char* buf, int size, int64_t ts)
{
	int ret = 0;
	struct timeval tv;
	gettimeofday(&tv, 0);
	ret = c_RTSP_put_source_frame(channel, buf, size, ts);
	return ret;
}

rtsp_module_t rmod[4];

int rtsp_init(void)
{
	return 0;
}

int rtsp_module_create(int channel, int modid)
{
	int mid;
	char name[20];
	int ret = 0;
	if (0 == rtsp_init_flag) {
		ret = frame_recv_rtsp_init();
		if (0 != ret) {
			printf("rtsp: %s,%d, error\n", __func__, __LINE__);
			return ret;
		}
		rtsp_init_flag = 1;
	}
	sprintf(name, "rtsp-%d", channel);
	mid = modid;
	rmod[channel].rtspmod = xcam_module_alloc(name, mid, xcam_rtsp_process_msg);
	rmod[channel].channel = channel;
	return 0;
}

int xcam_rtsp_process_msg(msg_com_data_t *msg)
{
	int i = 0;
	msg_release_f msg_release_fun = NULL;
	msg_stream_t *msg_data_tag = NULL;
	msg_stream_t *msg_data = NULL;
	msg_stream_t msg_data_pack;
	IMPEncoderStream stream;

	msg_data_tag = (msg_stream_t *)msg->pdata;
	assert(msg_data_tag->buf != NULL);
	msg_data = &msg_data_pack;
	memset(msg_data, 0, sizeof(msg_stream_t));
	memset(&stream, 0, sizeof(IMPEncoderStream));
	memcpy(&stream, (IMPEncoderStream *)msg_data_tag->buf, sizeof(IMPEncoderStream));

	for (i = 0; i < stream.packCount; i++) {
		uint32_t size = stream.pack[i].length - 4;
		msg_data->buf = (unsigned char*)(stream.pack[i].virAddr ) + 4;
		msg_data->size = size;
		msg_data->ts = stream.pack[i].timestamp;
		(void)xcam_rtsp_process_msg_impl(msg_data,msg->id);
	}

	msg_release_fun = msg->msg_release_cb;
	if (msg_release_fun != NULL) {
		msg_release_fun(msg);
	}

	return 0;
}

int xcam_rtsp_process_msg_impl(msg_stream_t *smsg, int msg_id)
{
	int channel;
	int msgid = msg_id;
	switch(msgid) {
	case MESG_ID_STREAM0:
	case MESG_ID_STREAM1:
	case MESG_ID_STREAM3:
		if (MESG_ID_STREAM0 == msgid) {
			channel = 0;
		} else if( MESG_ID_STREAM1 == msgid) {
			channel = 1;
		} else if( MESG_ID_STREAM3 == msgid) {
			channel = 3;
		}
		frame_recv_rtsp_waitready(channel);
		frame_recv_rtsp_putframe(channel, (char*)smsg->buf, smsg->size, smsg->ts);
		break;
	default:
		break;
	}
	return 0;
}

// int xcam_rtsp_process_audio_msg(msg_com_data_t *msg)
// {
// 	int msgid = msg->id;
// 	IMPAudioStream audiostream;
// 	msg_release_f msg_release_fun = NULL;
// 	msg_stream_t *msg_data_tag = NULL;
	
// 	msg_data_tag = (msg_stream_t *)msg->pdata;
// 	assert(msg_data_tag->buf != NULL);
// 	memset(&audiostream, 0, sizeof(IMPAudioStream));
// 	memcpy(&audiostream, (IMPAudioStream *)(msg_data_tag->buf), sizeof(IMPAudioStream));
// //  printf("%s[%d]  xxxx msgid=%x %d\n", __func__, __LINE__,  msg_data_tag->buf , audiostream.timeStamp);
// 	c_RTSP_put_source_audioframe(0, audiostream.stream, audiostream.len, audiostream.seq);
	   
// 	msg_release_fun = msg->msg_release_cb;
// 	if (msg_release_fun != NULL) {
// 		msg_release_fun(msg);
// 	}
// 	return 0;
// }
