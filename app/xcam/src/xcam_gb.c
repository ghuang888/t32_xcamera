#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "cJSON.h"
#include <imp/imp_common.h>
#include <imp/imp_encoder.h>
#include "xcam_log.h"
#include "xcam_msg.h"
#include "xcam_com.h"
#include "xcam_module.h"
#include "xcam_conf_json.h"
#include "xcam_osd.h"
#include "xcam_network.h"
#include "xcam_video.h"
#include "xcam_stream.h"
#include "svac_interfaces.h"
#include "xcam_conf_gb.h"
#include "xcam_gb.h"
#include "xcam_general.h"

#define LOG_TAG __FILE__
#define PUBLIC_KEY_FILE "/system/etc/test_sm2_pubkey.pem"
#define PRIVATE_KEY_FILE "/system/etc/test_sm3_pkey.pem"
#define GB_MANAGE_START 0
#define GB_MANAGE_STOP 1

#define DEFAULT_VIDEO_CHANEL 0
#define GB_MAXMEMSIZE (1*1024*1024)
#define GB_MAXCACHEFRAME (4)
#define xfree(P) do{if(P!=NULL) free(P);}while(0);

gb_manage_module_t gbmodule;

unsigned int xcamtype_to_svactype(unsigned int video_type)
{
	unsigned int type = 0;
#if((defined T20) || (defined T21))
	if(video_type == PT_H264)
		type = SVAC_TYPE_VIDEO_H264;
#elif T31
	if(video_type == IMP_ENC_TYPE_AVC)
		type = SVAC_TYPE_VIDEO_H264;
	else if(video_type == IMP_ENC_TYPE_HEVC)
		type = SVAC_TYPE_VIDEO_H265;
#elif T40

#endif
	return type;
}

int xcam_gb_set_video_stream(struct svac_videoparam *video)
{
	unsigned int video_enc_type = 0;
	struct svac_videoparam videoparam;
	int ret = 0;

	printf("%s[%d] video channel %d video type %d\n",__func__,__LINE__,video->video_chn,video->video_type);
	printf("%s[%d] video channel %d resolution %d*%d\n",__func__,__LINE__,video->video_chn,video->width,video->height);
	printf("%s[%d] video channel %d fps:%d-%d\n",__func__,__LINE__,video->video_chn,video->fps_num,video->fps_den);
	printf("%s[%d] video channel %d bitrate:%d\n",__func__,__LINE__,video->video_chn,video->maxbitrate);

	if(video->video_chn == -1)//点播未指定通道
		video->video_chn = DEFAULT_VIDEO_CHANEL;
	if(video->video_type == -1){
		//video_enc_type = IMP_ENC_TYPE_AVC;
		video_enc_type = PT_H264;
	}else{
		if(video->video_type == 2 )//点播未指定编码格式
		//	video_enc_type = IMP_ENC_TYPE_AVC;
			video_enc_type = PT_H264;
		else if(video->video_type = 5)
			//video_enc_type = IMP_ENC_TYPE_HEVC;
			video_enc_type = PT_H265;
		else//TODO:svac
			//video_enc_type = IMP_ENC_TYPE_AVC;
			video_enc_type = PT_H264;
	}
	ret = xcam_video_set_encode_mode(video->video_chn, video_enc_type);
	if(ret != 0){
		LOG_ERR(LOG_TAG,"Failed to set video encode format!\n");
		return ret;
	}
	if(video->width != -1 && video->height != -1){//点播未指定分辨率
		ret = xcam_video_set_resolution(video->video_chn,video->width,video->height);
		if(ret != 0){
			LOG_ERR(LOG_TAG,"Failed to set video resolution!\n");
			return ret;
		}
	}

	if(video->fps_num != -1 && 	video->fps_den != -1){//点播未指定帧率
		ret = xcam_video_set_isp_fps(video->fps_num,video->fps_den);
		if(ret != 0){
			LOG_ERR(LOG_TAG,"failed to set video fps!\n");
			return ret;
		}
	}

	if(video->bitrate != -1 && video->bitrate_mode != -1){//点播未指定码率和码率模式
		ret = xcam_video_set_bitrate(video->video_chn,video->bitrate);
		if(ret != 0){
			LOG_ERR(LOG_TAG,"failed to set video bitrate!\n");
			return ret;
		}
	}

	printf("set realplay video format-%d resolution-%d*%d fps-%d!\n",video_enc_type,video->width,video->height,video->fps_num);
	memset(&videoparam,0,sizeof(struct svac_videoparam));
	xcam_video_get_encode_mode(video->video_chn,(int *)&videoparam.video_type);
	videoparam.video_type = xcamtype_to_svactype(videoparam.video_type);
	xcam_video_get_resolution(video->video_chn, (int *)&videoparam.width, (int *)&videoparam.height);
	xcam_video_get_fs_fps(video->video_chn,(uint32_t *)&videoparam.fps_num, (uint32_t *)&videoparam.fps_den);
	xcam_video_get_bitrate(video->video_chn,(int *)&videoparam.maxbitrate);
	set_video_parameter(gbmodule.handle,video->video_chn,&videoparam);
	set_video_channel(gbmodule.handle,video->video_chn);
	return XCAM_SUCCESS;
}

int xcam_gb_mange_start_video_stream(unsigned int chn)
{
	printf("start video stream! video channel-%d!\n",chn);
	if(chn)
		xcam_module_register_wanted_msg(XMOD_ID_VSTREAM_1, XMOD_ID_GB_MANAGE, MESG_ID_STREAM1);
	else
		xcam_module_register_wanted_msg(XMOD_ID_VSTREAM_0, XMOD_ID_GB_MANAGE, MESG_ID_STREAM0);
	return XCAM_SUCCESS;
}

int xcam_gb_manage_stop_video_stream(unsigned int chn)
{
	printf("stop video stream! video channel-%d!\n",chn);
	if(chn)
		xcam_module_unregister_wanted_msg(XMOD_ID_VSTREAM_1, XMOD_ID_GB_MANAGE, MESG_ID_STREAM1);
	else
		xcam_module_unregister_wanted_msg(XMOD_ID_VSTREAM_0, XMOD_ID_GB_MANAGE, MESG_ID_STREAM0);
	return XCAM_SUCCESS;
}

int xcam_gb_manage_alarm_rest(void)
{
	printf("alarm reset\n");
	return XCAM_SUCCESS;
}

int xcam_gb_manage_send_Iframe(unsigned int chnNum)
{
	printf("force send I frame! video channel-%d!\n",chnNum);
	return XCAM_SUCCESS;
}

int xcam_gb_manage_get_maxpacketsize(unsigned int *size)
{
	*size = 512 * 1024;
	return XCAM_SUCCESS;
}

int xcam_gb_manage_set_osd_window(int length,int width)
{
	printf("set osd window!%d-%d\n",length,width);
	return XCAM_SUCCESS;
}

int xcam_gb_manage_set_osd_time(int enable,int x,int y,int time_type)
{
	printf("set osd time!enable %d time_x %d time_y %d time_type %d\n",enable,x,y,time_type);
	return XCAM_SUCCESS;
}

int xcam_gb_manage_set_osd_text(int enable,int x,int y,char *text)
{
	int i = 0;
	int ret = 0;
	gb_text_t *gbtext = &gbmodule.text;
	unsigned int gbtext_num = gbmodule.text_total;
#if 0
	char osdtoken[32]={0};
	printf("set osd time!enable %d text_x %d text_y %d text %s\n",enable,x,y,text);
	if(enable){
		for(i = 0; i < gbtext_num; i++){
			if(gbtext[i].text_x == x && gbtext[i].text_y == y){//already exist gb text
				ret = xcam_osd_set_text(gbmodule.realplay_chn, gbtext[i].osdtoken, x, y, text);
				memcpy(osdtoken,gbtext[i].osdtoken,sizeof(osdtoken));
			}else{
				ret = xcam_osd_create_text(gbmodule.realplay_chn, x, y, text, &osdtoken);
				if(!ret)
					memcpy(gbtext[i].osdtoken,osdtoken,sizeof(osdtoken));
			}
			//xcam_osd_show_text(gbmodule.realplay_chn,osdtoken,1);
		}
	}else{
		for(i = 0; i < gbtext_num; i++){
			//xcam_osd_show_text(gbmodule.realplay_chn,gbtext[i].osdtoken,0);
		}
	}
#endif
	return ret;
}

#ifdef GB35114
int xcam_gb_manage_init_security_parameters(int chn,struct security_paramters *sp, unsigned char *camera_idc, unsigned char *camera_id)
{
	printf("init security parameters!\n");
	return XCAM_SUCCESS;
}

int xcam_gb_manage_enable_encryption(int chn,unsigned int enable,SVACEncryptCallback_t fn,SVACGetkeysCallback_t key_fn,SVACGetivCallback_t iv_fn)
{
	printf("enable encryption!\n");
	return XCAM_SUCCESS;
}

int xcam_gb_manage_disable_encryption(int chn)
{
	printf("disable encryption!\n");
	return XCAM_SUCCESS;
}

int xcam_gb_manage_enable_signature(int chn,unsigned int enable,SVACSignatureCallback_t fn)
{
	printf("enable signature!\n");
	return XCAM_SUCCESS;
}

int xcam_gb_manage_disable_signature(int chn)
{
	printf("disable signature!\n");
	return XCAM_SUCCESS;
}

int xcam_gb_manage_update_security_parameters(int chn,struct security_paramters *sp)
{
	printf("update security parameters!\n");
	return XCAM_SUCCESS;
}
#endif

static int xcam_gb_manage_control_start_move(int direction, unsigned char horizontal_speed, unsigned char vertical_speed)
{
	printf("start move!\n");
	return XCAM_SUCCESS;
}

static int xcam_gb_manage_control_adjust_zoom(int zoom_type,unsigned char zoom_speed)
{
	printf("adjust zoom!\n");
	return XCAM_SUCCESS;
}

static int xcam_gb_manage_control_adjust_focus(int focus_type,unsigned char focus_speed)
{
	printf("start focus!\n");
	return XCAM_SUCCESS;
}

static int xcam_gb_manage_control_adjust_aperture(int aperture_type,unsigned char aperture_speed)
{
	printf("adjust aperature!\n");
	return XCAM_SUCCESS;
}

static int xcam_gb_manage_control_set_preset(int index)
{
	printf("set preset-%d!\n",index);
	return XCAM_SUCCESS;
}

static int xcam_gb_manage_control_call_preset(int index)
{
	printf("call preset-%d!\n",index);
	return XCAM_SUCCESS;
}

static int xcam_gb_manage_control_delete_preset(int index)
{
	printf("delete preset-%d!\n",index);
	return XCAM_SUCCESS;
}

static int xcam_gb_manage_control_add_patrol(int patrol_id,int preset_index)
{
	return XCAM_SUCCESS;
}

static int xcam_gb_manage_control_delete_patrol(int patrol_id,int preset_index)
{
	return XCAM_SUCCESS;
}

static int xcam_gb_manage_control_set_patrol_speed(int patrol_id,int preset_index,unsigned int patrol_speed)
{
	return XCAM_SUCCESS;
}

static int xcam_gb_manage_control_set_stay_time(int patrol_id,int preset_index,unsigned int time)
{
	return XCAM_SUCCESS;
}

static int xcam_gb_manage_control_start_patrol(int patrol_id,int preset_index)
{
	return XCAM_SUCCESS;
}

static int xcam_gb_manage_control_start_scan(int scan_id,unsigned int scan_speed)
{
	return XCAM_SUCCESS;
}

static int xcam_gb_manage_control_scan_left_border(int scan_id,unsigned int scan_speed)
{
	return XCAM_SUCCESS;
}

static int xcam_gb_manage_control_scan_right_border(int scan_id,unsigned int scan_speed)
{
	return XCAM_SUCCESS;
}

static int xcam_gb_manage_control_set_scan_speed(int scan_id,unsigned int scan_speed)
{
	return XCAM_SUCCESS;
}

static int xcam_gb_manage_control_open_homeposition(unsigned int preset_index,unsigned int reset_time)
{
	return XCAM_SUCCESS;
}

static int xcam_gb_manage_control_close_homeposition(void)
{
	return XCAM_SUCCESS;
}

static int xcam_gb_manage_control_stop(void)
{
	printf("stop control\n");
	return XCAM_SUCCESS;
}

static int xcam_gb_manage_start_record(unsigned int index)
{
	printf("start record channel-%d!\n",index);
	return XCAM_SUCCESS;
}

static int xcam_gb_manage_stop_record(unsigned int index)
{
	printf("stop record channel-%d!\n",index);
	return XCAM_SUCCESS;
}

static int xcam_gb_manage_start_guard(unsigned int index)
{
	printf("start guard channel-%d!\n",index);
	return XCAM_SUCCESS;
}

static int xcam_gb_manage_stop_guard(unsigned int index)
{
	printf("stop guard channel-%d!\n",index);
	return XCAM_SUCCESS;
}

static void xcam_gb_manage_reboot(void)
{
	printf("reboot now!\n");
	xcam_reboot();
}

static void xcam_gb_manage_calibrate_time(time_t time)
{
	printf("calibrate time!\n");
}

int xcam_gb_manage_process_msg(msg_com_data_t *msg)
{
	int msgid = msg->id;
	struct svac_video_frame videostream;
	msg_stream_t *smsg = NULL;
	IMPEncoderStream *stream = NULL;
	int stream_type = 0;
	int i = 0;
	int ret = XCAM_SUCCESS;
	memset(&videostream,0,sizeof(struct svac_video_frame));

	switch(msgid){
		case MESG_ID_STREAM0:
			xcam_video_get_encode_mode(0,(int *)&stream_type);
			break;
		case MESG_ID_STREAM1:
			xcam_video_get_encode_mode(1,(int *)&stream_type);
			break;
		default:
			break;
	}
	gbmodule.realplay_chn = msgid == MESG_ID_STREAM0 ? 0 : 1;
	smsg = (msg_stream_t *)msg->pdata;
	stream = (IMPEncoderStream *)smsg->buf;
	pthread_mutex_lock(&gbmodule.gb_mutex);
	videostream.timestamp = stream->pack[0].timestamp;
	for(i = 0; i < stream->packCount;i++){
		videostream.length += stream->pack[i].length;
	}
#if((defined T20) || (defined T21))
	videostream.addr = stream->pack[0].virAddr;
	videostream.idr = stream->refType == IMP_Encoder_FSTYPE_IDR ? 1 : 0;
#elif T31
	videostream.addr = (void*)stream->virAddr;
	if(stream_type == IMP_ENC_TYPE_AVC){
		videostream.idr = stream->pack[0].nalType.h264NalType == IMP_H264_NAL_SPS ? 1 : 0;
	}else if(stream_type == IMP_ENC_TYPE_HEVC){
		videostream.idr = stream->pack[0].nalType.h265NalType == IMP_H265_NAL_SPS ? 1 : 0;
	}
#elif T40

#endif
	ret = svac_push_stream(gbmodule.handle, &videostream, NULL);
	if(ret)
		printf("stream is larger than cache buffer size, it will be abandoned!\n");

	pthread_mutex_unlock(&gbmodule.gb_mutex);
#if 0
	msg_release_f msg_release_fun = msg->msg_release_cb;
	if (msg_release_fun != NULL) {
		msg_release_fun(msg);
	}
#endif
	return ret;
}

int gb_manage_set_video_param(void)
{
	struct svac_videoparam videoparam;
	int ichn = 0;
	for(ichn = 0; ichn < gbmodule.total_chn; ichn++){
		memset(&videoparam,0,sizeof(videoparam));
		xcam_video_get_encode_mode(ichn,(int *)&videoparam.video_type);
		videoparam.video_type = xcamtype_to_svactype(videoparam.video_type);
		xcam_video_get_resolution(ichn, (int *)&videoparam.width, (int *)&videoparam.height);
		xcam_video_get_fs_fps(ichn,(uint32_t *)&videoparam.fps_num, (uint32_t *)&videoparam.fps_den);
		xcam_video_get_bitrate(ichn,(int *)&videoparam.maxbitrate);
		set_video_parameter(gbmodule.handle,ichn,&videoparam);
		//printf("%s[%d] video channel %d video type %d\n",__func__,__LINE__,ichn,videoparam.video_type);
		//printf("%s[%d] video channel %d resolution %d*%d\n",__func__,__LINE__,ichn,videoparam.width,videoparam.height);
		//printf("%s[%d] video channel %d fps:%d-%d\n",__func__,__LINE__,ichn,videoparam.fps_num,videoparam.fps_den);
		//printf("%s[%d] video channel %d bitrate:%d\n",__func__,__LINE__,ichn,videoparam.maxbitrate);
	}
	return XCAM_SUCCESS;
}

int gb_manage_module_create(int modid)
{
	int ret = XCAM_SUCCESS;
	int mid;
	void *handle = NULL;
	unsigned int channels = 0;
	char name[20] = {0};
	sprintf(name,"gbmanage");
	mid = modid;
	gbmodule.gbmod = xcam_module_alloc(name, mid, xcam_gb_manage_process_msg);
	if(!gbmodule.gbmod){
		LOG_ERR(LOG_TAG,"xcam gb module alloc error.\n");
		return XCAM_ERROR;
	}
	pthread_mutex_init(&gbmodule.gb_mutex,NULL);
	struct svac_manage_callbacks gb_callbacks;
	gb_callbacks.set_video_stream = xcam_gb_set_video_stream;
	gb_callbacks.start_video_stream = xcam_gb_mange_start_video_stream;
	gb_callbacks.stop_video_stream = xcam_gb_manage_stop_video_stream;
	gb_callbacks.send_Iframe = xcam_gb_manage_send_Iframe;
	gb_callbacks.get_maxpacketsize = xcam_gb_manage_get_maxpacketsize;
	gb_callbacks.set_osd_window = xcam_gb_manage_set_osd_window;
	gb_callbacks.set_osd_time = xcam_gb_manage_set_osd_time;
	gb_callbacks.set_osd_text = xcam_gb_manage_set_osd_text;
	//security
#ifdef GB35114
	gb_callbacks.init_security_paramters = xcam_gb_manage_init_security_parameters;
	gb_callbacks.enable_encryption = xcam_gb_manage_enable_encryption;
	gb_callbacks.disable_encryption = xcam_gb_manage_disable_encryption;
	gb_callbacks.enable_signature =xcam_gb_manage_enable_signature;
	gb_callbacks.disable_signature =xcam_gb_manage_disable_signature;
	gb_callbacks.update_security_paramters = xcam_gb_manage_update_security_parameters;
#endif
	//control
	gb_callbacks.start_move = xcam_gb_manage_control_start_move;
	gb_callbacks.adjust_zoom = xcam_gb_manage_control_adjust_zoom;
	gb_callbacks.adjust_focus = xcam_gb_manage_control_adjust_focus;
	gb_callbacks.adjust_aperture = xcam_gb_manage_control_adjust_aperture;
	//预置位
	gb_callbacks.set_preset = xcam_gb_manage_control_set_preset;
	gb_callbacks.call_preset = xcam_gb_manage_control_call_preset;
	gb_callbacks.delete_preset = xcam_gb_manage_control_delete_preset;
	//巡航
	gb_callbacks.add_patrol = xcam_gb_manage_control_add_patrol;
	gb_callbacks.delete_patrol = xcam_gb_manage_control_delete_patrol;
	gb_callbacks.set_patrol_speed = xcam_gb_manage_control_set_patrol_speed;
	gb_callbacks.set_stay_time = xcam_gb_manage_control_set_stay_time;
	gb_callbacks.start_patrol = xcam_gb_manage_control_start_patrol;
	//扫描
	gb_callbacks.start_scan = xcam_gb_manage_control_start_scan;
	gb_callbacks.scan_left_border = xcam_gb_manage_control_scan_left_border;
	gb_callbacks.scan_right_border = xcam_gb_manage_control_scan_right_border;
	gb_callbacks.set_scan_speed = xcam_gb_manage_control_set_scan_speed;
	//看守位
	gb_callbacks.open_homeposition = xcam_gb_manage_control_open_homeposition;
	gb_callbacks.close_homeposition = xcam_gb_manage_control_close_homeposition;
	//停止以上所有控制
	gb_callbacks.stop_control = xcam_gb_manage_control_stop;

	/*Record*/
	gb_callbacks.start_record = xcam_gb_manage_start_record;
	gb_callbacks.stop_record = xcam_gb_manage_stop_record;

	/*Guard*/
	gb_callbacks.start_guard = xcam_gb_manage_start_guard;
	gb_callbacks.stop_guard = xcam_gb_manage_stop_guard;
	/*reboot*/
	gb_callbacks.reboot_now = xcam_gb_manage_reboot;
	gb_callbacks.calibrate_time = xcam_gb_manage_calibrate_time;
#ifdef GB28181
	handle = svac_create_manage(GB_MAXMEMSIZE, GB_MAXCACHEFRAME, &gb_callbacks,NULL,NULL);
#elif GB35114
	handle = svac_create_manage(GB_MAXMEMSIZE, GB_MAXCACHEFRAME, &gb_callbacks,PRIVATE_KEY_FILE,PUBLIC_KEY_FILE);
#endif
	if(!handle){
		LOG_ERR(LOG_TAG,"Failed to create svac manage!\n");
		goto err_create_manage;
	}
	gbmodule.handle = handle;
	ret = xcam_gb_manage_conf_init(handle);
	if(ret != XCAM_SUCCESS){
		LOG_ERR(LOG_TAG,"GB manage config initialize failed!\n");
		goto err_init_conf_gb;
	}
	gbmodule.total_chn = xcam_video_get_channel_num();
	if(gbmodule.total_chn <= 0){
		LOG_ERR(LOG_TAG,"GB manage get video total channel failed!\n");
		goto err_get_video_channel;
	}
	ret = video_parameters_init(handle,gbmodule.total_chn);
	if(ret != XCAM_SUCCESS){
		LOG_ERR(LOG_TAG,"GB manage video init failed!\n");
		goto err_init_video;
	}
	gb_manage_set_video_param();
	gbmodule.state = GB_MANAGE_STOP;
	return XCAM_SUCCESS;
err_init_video:
err_get_video_channel:
err_init_conf_gb:
	svac_destroy_manage(handle);
err_create_manage:
	xcam_module_free(gbmodule.gbmod,modid);
	return XCAM_ERROR;
}

int gb_manage_module_destroy(void)
{
	xcam_module_t *mod = gbmodule.gbmod;
	xcam_module_free(mod,mod->id);
	svac_destroy_manage(gbmodule.handle);
	return XCAM_SUCCESS;
}

int gb_manage_module_start(void)
{
	int ret = XCAM_SUCCESS;
	printf("gb manage start\n");
	if(gbmodule.state != GB_MANAGE_START){
		ret = svac_start_manage(gbmodule.handle);
		if(ret){
			LOG_ERR(LOG_TAG,"Failed to start gb manage\n");
			return XCAM_ERROR;
		}
		gbmodule.state = GB_MANAGE_START;
	}
	return XCAM_SUCCESS;
}

int gb_manage_module_stop(void)
{
	int ret = XCAM_SUCCESS;
	printf("gb manage stop\n");
	if(gbmodule.state != GB_MANAGE_STOP){
		ret = svac_stop_manage(gbmodule.handle);
		if(ret){
			LOG_ERR(LOG_TAG,"Failed to stop gb manage\n");
			return XCAM_ERROR;
		}
		gbmodule.state = GB_MANAGE_STOP;
	}
	return XCAM_SUCCESS;
}

int gb_manage_module_restart(void)
{
	int ret = XCAM_SUCCESS;
	printf("gb manage restart\n");
	xcam_module_unregister_wanted_msg(XMOD_ID_VSTREAM_1, XMOD_ID_GB_MANAGE, MESG_ID_STREAM1);
	xcam_module_unregister_wanted_msg(XMOD_ID_VSTREAM_0, XMOD_ID_GB_MANAGE, MESG_ID_STREAM0);
	ret = xcam_gb_manage_conf_init(gbmodule.handle);
	if(ret != XCAM_SUCCESS){
		LOG_ERR(LOG_TAG,"GB manage config initialize failed!\n");
		return XCAM_ERROR;
	}
	gb_manage_set_video_param();
	if(gb_manage_module_stop()){
		return XCAM_ERROR;
	}
	if(gb_manage_module_start()){
		return XCAM_ERROR;
	}
	return XCAM_SUCCESS;
}

