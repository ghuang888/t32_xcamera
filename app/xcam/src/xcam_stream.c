#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "xcam_com.h"
#include "xcam_log.h"
#include "xcam_thread.h"
#include "xcam_module.h"
#include "xcam_stream.h"
#include "xcam_osd.h"
#include "xcam_video.h"
#include "time.h"
#include "xcam_web_process.h"
#define STREAM_MODULE_RUN 1
#define STREAM_MODULE_STOP 0
#define LOG_TAG __FILE__
stream_module_t vmod[4];
static void* thread_stream(void * param);
int pthread_yield(void);
int stream_msg_release(struct msg_com_data_s *mcd);
unsigned char *msg_data_buf[4];
int stream_module_create(int channel, int modid)
{
	int mid;
	char name[20];
	sprintf(name, "stream-%d", channel);
	mid = modid;
	vmod[channel].xmod = xcam_module_alloc(NULL, mid, NULL);
	vmod[channel].state = STREAM_MODULE_RUN;
	vmod[channel].channel = channel;
	pthread_mutex_init(&vmod[channel].stream_mutex,NULL);
	vmod[channel].tid = xcam_thread_create(name, thread_stream, &vmod[channel]);
	
	sprintf(name, "msg-stream-%d", channel);
	vmod[channel].msgpool = msg_pool_alloc(name, mid, 5, sizeof(msg_stream_t), stream_msg_release);
	if (0 == vmod[channel].msgpool) {
		LOG_ERR(LOG_TAG, "msg pool alloc failed!\n");
		return -1;
	}

	return 0;
}

int stream_msg_release(struct msg_com_data_s *mcd)
{
	int ret = 0;
	ret = msg_pool_put_free_msg(mcd->pdata);
	if (0 != ret) {
		LOG_ERR(LOG_TAG, "stream msg put free failed!\n");
		return -1;
	}
	return 0;
}

/*
 * 暂停模块运行，该接口无特殊原因不可长时间占用，
 * 调用完需立即调用stream_module_start()接口释放
 */
int stream_module_stop(int streamnum)
{
	if (vmod[streamnum].state == STREAM_MODULE_RUN) {
		vmod[streamnum].state = STREAM_MODULE_STOP;
		pthread_mutex_lock(&vmod[streamnum].stream_mutex);
	} else {
		LOG_WAN(LOG_TAG,"Module already stop.\n");
		return -1;
	}

	return 0;
}

//开始模块运行
int stream_module_start(int streamnum)
{
	if (vmod[streamnum].state == STREAM_MODULE_STOP) {
		vmod[streamnum].state = STREAM_MODULE_RUN;
		pthread_mutex_unlock(&vmod[streamnum].stream_mutex);
	} else {
		LOG_WAN(LOG_TAG,"Module already runing.\n");
		return -1;
	}

	return 0;
}
static void* thread_stream(void * param)
{
	stream_module_t *vmod;
	vmod = (stream_module_t*)param;
	int channel = vmod->channel;
	char threadname[20];
	sprintf(threadname, "stream-%d", channel);
	xcam_thread_set_name(threadname);
	
	int frmrate_sp = 0;
	int bitrate_sp = 0;
	long time_now = 0;
	long time_old = 0;

	msg_data_buf[channel]= (unsigned char*)malloc(512000);
	/*
	 *初始化osd标签状态
	 */
	
	int ret = xcam_video_enc_start(channel);
	if (ret < 0) {
		LOG_ERR(LOG_TAG,"channel:%d xcam_video_enc_start failed \n", channel);
		return (void*)-1;	
	}
	while (1) {
		/*
		 * 当系统操作sdk的时候，例如停止某一路码流等操作
		 * 导致xcam没有一些出流的必要的配置的时候，
		 * 停止该模块防止出现不可预见错误
		 */
		pthread_mutex_lock(&vmod->stream_mutex);
		int i = 0;
		IMPEncoderStream stream;
		msg_stream_t *msg_data = NULL;
		ret = xcam_video_get_enc_frame(channel, &stream);
		if(ret < 0){
			pthread_mutex_unlock(&vmod->stream_mutex);
			continue;
		}
		msg_data = msg_pool_get_free_msg(vmod->msgpool);
		if (NULL == msg_data) {
			usleep(500*1000);
			LOG_ERR(LOG_TAG, "channel%d get free msg faild!\n", channel);
			xcam_video_release_enc_frame(channel, &stream);
			pthread_mutex_unlock(&vmod->stream_mutex);
			continue;
		}
		msg_data->type = MSG_VIDEO;
		msg_data->buf = (uint8_t *)&stream;
		xcam_module_process_msg(vmod->xmod->id, MSG_DATA_TO_MSG(msg_data));

		for (i = 0; i < stream.packCount; i++) {
			bitrate_sp += stream.pack[i].length;
		}

		frmrate_sp ++;
		time_now = IMP_System_GetTimeStamp()/1000;
		if (((time_now-time_old)/1000) >= 2) {
			double fps = (double)frmrate_sp / ((double)(time_now - time_old) / 1000);
			double kbr = (double)bitrate_sp * 8 / (double)(time_now - time_old);
			xcam_osd_set_fps_kbps(channel, fps, kbr);
			frmrate_sp = 0;
			bitrate_sp = 0;
			time_old = time_now;
		}

		xcam_video_release_enc_frame(channel, &stream);
		pthread_mutex_unlock(&vmod->stream_mutex);
		if (vmod->state == STREAM_MODULE_STOP)
			//主动让出cpu，预防该线程持续抢占锁，导致其它线程无法正常运行
			pthread_yield();
	}
	return NULL;
}

