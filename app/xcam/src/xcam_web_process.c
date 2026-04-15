#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <assert.h>
#include <imp/imp_isp.h>
#include <fcntl.h>
#include <unistd.h>
#include "xcam_video.h"
#include "xcam_com.h"
#include "xcam_log.h"
#include "xcam_module.h"
#include "xcam_stream.h"
#include "xcam_thread.h"
#include "xcam_general.h"
#include "../../onvif/src/func_network.h"
#include "cJSON.h"
#include "xcam_conf_process.h"
#include "../../daemon/xcam_daemon_protocol.h"
#include "xcam_web_process.h"
#include "xcam_system.h"
#include "xcam_conf_video.h"
#if((defined GB28181) || (defined GB35114))
#include "xcam_conf_gb.h"
#endif

#define LOG_TAG "xcam_web_process"
#define WEB_XCAM_GET_CONFIGS 0
#define WEB_XCAM_SET_CONFIGS 1
#define WEB_CONFIG_FILE_PATH "/system/etc/webServer/conf/boa.conf"

bool switch_ch0 = true;
bool switch_ch1 = true;
static int mirror_falg=0, flip_flag=0;
int xcam_json_set_channel_switch(void *json_root);
int xcam_web_set_http_port(int port)
{
	int ret = XCAM_SUCCESS;
	FILE *fp = fopen(WEB_CONFIG_FILE_PATH, "r");
    	int length = 0;
	char *pbuf = NULL, *pstart = NULL, *pend = NULL, *tmp = NULL;
	char portbuf[16];

    	fseek(fp, 0, SEEK_END);
	length = ftell(fp);

	fseek(fp, 0, SEEK_SET);

	pbuf = (char *)malloc(length+1);
	pbuf[length] = '\0';

	fread(pbuf, 1, length, fp);

	fclose(fp);

	if ((pstart = strstr(pbuf, "Port")))
	{
		tmp = pstart;
		while(*tmp != '\n')
			tmp++;

		pend = tmp;
	}

	fp = fopen(WEB_CONFIG_FILE_PATH, "w");

	sprintf(portbuf, "Port %d", port);

	fwrite(pbuf, 1, pstart-pbuf, fp);
	fwrite(portbuf, 1, strlen(portbuf), fp);
	fwrite(pend, 1, length-(pend-pbuf), fp);

	fclose(fp);
	free(pbuf);
	sleep(1);
	xcam_system_stop_boa();
	xcam_system_start_boa();
	return ret;
}

int xcam_web_get_http_port(int *port)
{
	int ret = XCAM_SUCCESS;
	char strline[1024] = {0};
	FILE *fp = NULL;
	char strPort[10] = {0};

	fp = fopen(WEB_CONFIG_FILE_PATH,"rw");
	if (fp == NULL) {
		LOG_ERR(LOG_TAG,"error(%s,%d),fopen web config file fail.\n");
		return XCAM_ERROR;
	}

	while (!feof(fp)) {
		memset (strline, 0 ,sizeof(strline));
		fgets(strline,1024,fp);
		if((strline[0] == 'P') && (strline[1] == 'o') && (strline[2] == 'r') && (strline [3] = 't')) {
			sscanf(strline, "%*s%s",strPort);
			*port = atoi(strPort);
			break;
		}
	}

	fclose(fp);
	return ret;
}
int xcam_web_set_video_nightvisionmode(int nightvision)
{
	int ret = XCAM_SUCCESS;
 	IMPISPRunningMode mode;
	if(nightvision){
		mode = IMPISP_RUNNING_MODE_NIGHT;
	}else{
		mode = IMPISP_RUNNING_MODE_DAY;
	}
	ret = IMP_ISP_Tuning_SetISPRunningMode(0, &mode);
	if(ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG,"err(%s,%d): set nightvisionmode fail.\n", __func__, __LINE__);
		ret = XCAM_ERROR;
	}
	/*
	ret = xcam_conf_set_video_nightvisionmode(nightvision);
	if(ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG,"err(%s,%d): set nightvisionmode to config fail.\n", __func__, __LINE__);
		ret = XCAM_ERROR;
	}
	*/
	return ret;
}

int xcam_web_set_video_wdrmode(int wdr)
{
	int ret = XCAM_SUCCESS;

	if(ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG,"err(%s,%d): set wdrmode fail.\n", __func__, __LINE__);
		ret = XCAM_ERROR;
	}

	/*
	ret = xcam_conf_set_video_wdrmode(wdr);
	if(ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG,"err(%s,%d): set wdrmode to config fail.\n", __func__, __LINE__);
		ret = XCAM_ERROR;
	}
	*/
	return ret;
}

int xcam_web_set_video_imagemirrormode(int imagemirror)
{
	int ret = XCAM_ERROR;
	IMPISPHVFLIPAttr attr;

	if(imagemirror) {
		attr.sensor_mode = IMPISP_FLIP_H_MODE;
		if(flip_flag == 1)
			attr.sensor_mode = IMPISP_FLIP_HV_MODE;
		mirror_falg = 1;
	}
	ret = IMP_ISP_Tuning_SetHVFLIP(0,&attr);
	if(ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG,"err(%s,%d): set imagemirrormode fail.\n", __func__, __LINE__);
		ret = XCAM_ERROR;
	}
	/*
	ret = xcam_conf_set_video_imagemirrormode(imagemirror);
	if(ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG,"err(%s,%d): set imagemirrormode to config fail.\n", __func__, __LINE__);
		ret = XCAM_ERROR;
	}
	*/
	return ret;
}

int xcam_web_set_video_imageflipmode(int imageflip)
{
	int ret = XCAM_ERROR;
	IMPISPHVFLIPAttr attr;
	if(imageflip) {
		attr.sensor_mode = IMPISP_FLIP_V_MODE;
		if(mirror_falg == 1)
			attr.sensor_mode = IMPISP_FLIP_HV_MODE;
		flip_flag= 1;
	}
	ret = IMP_ISP_Tuning_SetHVFLIP(0,&attr);
	if(ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG,"err(%s,%d): set imageflipmode fail.\n", __func__, __LINE__);
		ret = XCAM_ERROR;
	}
	/*
	ret = xcam_conf_set_video_imageflipmode(imageflip);
	if(ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG,"err(%s,%d): set imageflipmode to config fail.\n", __func__, __LINE__);
		ret = XCAM_ERROR;
	}
	*/
	return ret;
}

int xcam_web_set_video_ircutmode(int ircut)
{
	int ret = XCAM_SUCCESS;
	ret = xcam_conf_set_video_ircutmode(ircut);
	if(ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG,"err(%s,%d): set ircutmode to config fail.ret=%d\n", __func__, __LINE__, ret);
		ret = XCAM_ERROR;
	}
	return ret;
}

int xcam_web_set_video_noisereductionmode(int noisereduction)
{
	int ret = XCAM_SUCCESS;
	IMPISPModuleRatioAttr ratio;
	ratio.ratio_attr[IMP_ISP_MODULE_TEMPER].en = noisereduction;
	ratio.ratio_attr[IMP_ISP_MODULE_TEMPER].ratio = 128;

	ret = IMP_ISP_Tuning_SetModule_Ratio(0, &ratio);
	if(ret){
		LOG_ERR(LOG_TAG, "IMP_ISP_Tuning_SetModule_Ratio error !\n");
		return XCAM_ERROR;
	}
	/*
	ret = xcam_conf_set_video_noisereductionmode(noisereduction);
	if(ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG,"err(%s,%d): set noisereductionmode to config fail.\n", __func__, __LINE__);
		ret = XCAM_ERROR;
	}
	*/
	return ret;
}

int xcam_web_set_video_drcmode(int drc)
{
	int ret = XCAM_SUCCESS;

	//ret = IMP_ISP_Tuning_SetDRC_Strength(0,drc);
	if(ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG,"err(%s,%d): set drcmode fail.\n", __func__, __LINE__);
		ret = XCAM_ERROR;
	}
	/*
	ret = xcam_conf_set_video_drcmode(drc);
	if(ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG,"err(%s,%d): set drcmode to config fail.\n", __func__, __LINE__);
		ret = XCAM_ERROR;
	}
	*/
	return ret;
}

int xcam_web_set_video_image_control(int saturation, int brightness, int sharpness, int contrast)
{
	int ret = XCAM_SUCCESS;
	ret = IMP_ISP_Tuning_SetSaturation(0,(uint8_t*)&saturation);
	if(ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG,"err(%s,%d): set saturation fail.\n", __func__, __LINE__);
		ret = XCAM_ERROR;
	}

	ret = IMP_ISP_Tuning_SetBrightness(0,(uint8_t*)&brightness);
	if(ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG,"err(%s,%d): set brightness fail.\n", __func__, __LINE__);
		ret = XCAM_ERROR;
	}

	ret = IMP_ISP_Tuning_SetSharpness(0,(uint8_t*)&sharpness);
	if(ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG,"err(%s,%d): set sharpness fail.\n", __func__, __LINE__);
		ret = XCAM_ERROR;
	}

	ret = IMP_ISP_Tuning_SetContrast(0,(uint8_t*)&contrast);
	if(ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG,"err(%s,%d): set contrast fail.\n", __func__, __LINE__);
		ret = XCAM_ERROR;
	}

	ret = xcam_conf_set_video_image_control(saturation,brightness,sharpness,contrast);
	if(ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG,"err(%s,%d): set image_control to config fail.\n", __func__, __LINE__);
		ret = XCAM_ERROR;
	}
	return ret;
}

//isp效果参数设置
int xcam_web_set_video_image_control_extra(int *extra, int size){
	int ret;
	for(int i = 0; i < size; i++) {
		if(extra[i] < 0) continue;
		if(i == AWB){  //改成switch
			IMPISPWBAttr attr;
			attr.mode = extra[i];
			ret =  IMP_ISP_Tuning_SetAwbAttr(0, &attr);   // set isp awb value
			if(ret < XCAM_SUCCESS) {
				LOG_ERR(LOG_TAG,"err(%s,%d): set awb fail.\n", __func__, __LINE__);
				ret = XCAM_ERROR;
				return ret;
			}
			g_extra_web_param.awb = extra[i];
		} else if(i == GAMMA) {
			g_extra_web_param.gamma = extra[i];
			LOG_DBG(LOG_TAG,"xcam(%s,%d): set gamma=%d\n", __func__, __LINE__, g_extra_web_param.gamma);
		} else if(i == FILLLIGHT) {
			g_extra_web_param.filllight = extra[i];
			LOG_DBG(LOG_TAG,"xcam(%s,%d): set filllight=%d\n", __func__, __LINE__, g_extra_web_param.filllight);
		} else if(i == VISUALANGLE) {
			g_extra_web_param.visualangle = extra[i];
			LOG_DBG(LOG_TAG,"xcam(%s,%d): set visualangle index=%d\n", __func__, __LINE__, g_extra_web_param.visualangle);
		} else if(i == BACKLIGHT) {
			g_extra_web_param.backlight = extra[i];
			LOG_DBG(LOG_TAG,"xcam(%s,%d): set backlight index=%d\n", __func__, __LINE__, g_extra_web_param.backlight);
		} else if(i == NIGHTMODE) {
			g_extra_web_param.nightmode = extra[i];
			LOG_DBG(LOG_TAG,"xcam(%s,%d): set nightmode index=%d\n", __func__, __LINE__, g_extra_web_param.nightmode);
		}
	}
	return 0;
}

#if 0
#define SNAP_FILE_PATH_PREFIX		"/tmp"
#define SNAPMANUAL_FILE_PATH_PREFIX		"/system/etc/webServer/html/img/"
//保存编码码流为文件，舍弃
int save_stream(int fd, IMPEncoderStream *stream)
{
	// int ret, i, nr_pack = stream->packCount;

	// // printf("----------packCount=%d, stream->seq=%u start----------\n", stream->packCount, stream->seq);
	// for (i = 0; i < nr_pack; i++) {
	// 	//IMP_LOG_DBG(LOG_TAG, "[%d]:%10u,%10lld,%10u,%10u,%10u\n", i, stream->pack[i].length, stream->pack[i].timestamp, stream->pack[i].frameEnd, *((uint32_t *)(&stream->pack[i].nalType)), stream->pack[i].sliceType);
	// 	IMPEncoderPack *pack = &stream->pack[i];
	// 	if(pack->length){
	// 		uint32_t remSize = stream->streamSize - pack->offset;
	// 		if(remSize < pack->length){
	// 			ret = write(fd, (void *)(stream->virAddr + pack->offset), remSize);
	// 			if (ret != remSize) {
	// 				LOG_ERR(LOG_TAG, "stream write ret(%d) != pack[%d].remSize(%d) error:%s\n", ret, i, remSize, strerror(errno));
	// 				return -1;
	// 			}
	// 			ret = write(fd, (void *)stream->virAddr, pack->length - remSize);
	// 			if (ret != (pack->length - remSize)) {
	// 				LOG_ERR(LOG_TAG, "stream->virAddr:%x stream write ret(%d) != pack[%d].(length-remSize)(%d) error:%s\n", stream->virAddr, ret, i, (pack->length - remSize), strerror(errno));
	// 				return -1;
	// 			}
	// 		} else {
	// 			ret = write(fd, (void *)(stream->virAddr + pack->offset), pack->length);
	// 			if (ret != pack->length) {
	// 				LOG_ERR(LOG_TAG, "stream write ret(%d) != pack[%d].length(%d) error:%s\n", ret, i, pack->length, strerror(errno));
	// 				return -1;
	// 			}
	// 		}
	// 	}
	// }
	//IMP_LOG_DBG(LOG_TAG, "----------packCount=%d, stream->seq=%u end----------\n", stream->packCount, stream->seq);
	return 0;
}

// int xcam_file_fm_write(int fd, char *buf, size_t size){
// 		int        ret = file_fm_write(fd, buf, size);

           
// }

int save_stream_video(int fd, IMPEncoderStream *stream)
{
	// int ret, i, nr_pack = stream->packCount;

	// // printf("----------packCount=%d, stream->seq=%u start----------\n", stream->packCount, stream->seq);
	// for (i = 0; i < nr_pack; i++) {
	// 	//IMP_LOG_DBG(LOG_TAG, "[%d]:%10u,%10lld,%10u,%10u,%10u\n", i, stream->pack[i].length, stream->pack[i].timestamp, stream->pack[i].frameEnd, *((uint32_t *)(&stream->pack[i].nalType)), stream->pack[i].sliceType);
	// 	IMPEncoderPack *pack = &stream->pack[i];
	// 	if(pack->length){
	// 		uint32_t remSize = stream->streamSize - pack->offset;
	// 		if(remSize < pack->length){
	// 			ret = file_fm_write(fd, (void *)(stream->virAddr + pack->offset), remSize);
	// 			if (ret != remSize) {
	// 				LOG_ERR(LOG_TAG, "stream write ret(%d) != pack[%d].remSize(%d) error:%s\n", ret, i, remSize, strerror(errno));
	// 				return -1;
	// 			}
	// 			ret = file_fm_write(fd, (void *)stream->virAddr, pack->length - remSize);
	// 			if (ret != (pack->length - remSize)) {
	// 				LOG_ERR(LOG_TAG, "stream->virAddr:%x stream write ret(%d) != pack[%d].(length-remSize)(%d) error:%s\n", stream->virAddr, ret, i, (pack->length - remSize), strerror(errno));
	// 				return -1;
	// 			}
	// 		} else {
	// 			ret = file_fm_write(fd, (void *)(stream->virAddr + pack->offset), pack->length);
	// 			// LOG_ERR(LOG_TAG, "stream write ret(%d) != pack[%d].length(%d) \n", ret, i, pack->length);
	// 			if (ret != pack->length) {
	// 				LOG_ERR(LOG_TAG, "stream write ret(%d) != pack[%d].length(%d) error:%s\n", ret, i, pack->length, strerror(errno));
	// 				return -1;
	// 			}
	// 		}
	// 	}
	// }
	//IMP_LOG_DBG(LOG_TAG, "----------packCount=%d, stream->seq=%u end----------\n", stream->packCount, stream->seq);
	return 0;
}

//定时或者手动保存码流为图片,舍弃
// int xcam_web_snapJpeg(IMPEncoderStream* stream, int chn){
// 	int i, ret;
// 	char snap_path[128];
// 	if(g_extra_web_param.snapmanual == 1){
// 		sprintf(snap_path, "%s/snap-%d.hevc",
// 			SNAPMANUAL_FILE_PATH_PREFIX,chn);
// 	}else {
// 		sprintf(snap_path, "%s/snap-%d.jpg",
// 			SNAP_FILE_PATH_PREFIX,chn);
// 	}
// 	// LOG_ERR(LOG_TAG, "Open Snap file %s\n", snap_path);
// 	// LOG_ERR(LOG_TAG, "Open Snap file %s\n", snap_path);
// 	int snap_fd = open(snap_path, O_RDWR | O_CREAT | O_TRUNC, 0777);
// 	if (snap_fd < 0) {
// 		LOG_ERR(LOG_TAG, "failed: %s\n", strerror(errno));
// 		return -1;
// 	}
// 	if(stream)
// 		ret = save_stream(snap_fd, stream);
// 	close(snap_fd);
// 	return ret;
// }
#endif
int xcam_web_socket_set_configs_process(socket_tcp_link_t *plink)
{
//	skt_msg_header_t *header = (skt_msg_header_t *)plink->msgbuf;
	char *data = plink->msgbuf->data;
	char *json_str = NULL;
	int ret = XCAM_SUCCESS;
	cJSON *root_s =NULL,*ret_s = NULL,*root_r = NULL,*member = NULL;

	if (data == NULL) {
		LOG_ERR(LOG_TAG,"err(%s,%d): recv data is NULL.\n", __func__, __LINE__);
		ret = XCAM_ERROR;
	}

	root_s = cJSON_CreateObject();
	root_r = cJSON_Parse(data);

#if 1
	char *json_recv = cJSON_Print(root_r);
	printf("SET %s-%d\n %s\n",__func__,__LINE__,json_recv);
#endif
	//这两个地方肯定不能这样直接不等于NULL就返回了肯定不对啊,recv就没有了,set和get的地方都记得要修改
	if (root_r == NULL) {
		LOG_ERR(LOG_TAG,"err(%s,%d):recv data format error,cJSON parse fail.\n",__func__,__LINE__);
		ret = XCAM_ERROR;
	}

	member = root_r->child;
	if (member == NULL) {
		LOG_ERR(LOG_TAG,"error(%s,%d),root no anything member.\n",__func__,__LINE__);
		ret = XCAM_ERROR;
	}

	//这四行是调试代码
	char *json_str1 = NULL;
	json_str1 = cJSON_Print(root_r);
	free(json_str1);

	while ((ret == XCAM_SUCCESS) && member) {
		if (strcmp(member->string, "video.isp.fps") == 0) {
			ret |= xcam_json_set_video_isp_fps(root_r);
		} else if (strcmp(member->string, "video.bitrate") == 0) {
			ret |= xcam_json_set_video_bps(root_r);
		} else if (strcmp(member->string, "video.resolution") == 0) {
			ret |= xcam_json_set_video_resolution(root_r);
		} else if (strcmp(member->string, "network.information") == 0) {
			ret |= xcam_json_set_sys_ipconfig(root_r);
		} else if (strcmp(member->string, "video.enctype") == 0) {
			ret |= xcam_json_set_video_encode_mode(root_r);
		} else if (strcmp(member->string,"video.ispcontrolInfo") == 0) {
			ret |= xcam_json_set_video_ispcontrolInfo(root_r);
		} else if (strcmp(member->string,"video.image_control") == 0) {
			ret |= xcam_json_set_video_image_control(root_r);
		} else if (strcmp(member->string, "video.goplength") == 0) {
			ret |= xcam_json_set_video_Iframe_interval(root_r);
		} else if (strcmp(member->string, "video.rcmode") == 0){
			ret |= xcam_json_set_video_Rcmode(root_r);
		} else if (strcmp(member->string, "web.httpport") == 0) {
			ret |= xcam_json_set_web_http_port(root_r);
		} else if (strcmp (member->string, "video.switch") == 0) {
			ret |= xcam_json_set_channel_switch(root_r);
/*
		#if((defined GB28181) || (defined GB35114))
		} else if (strcmp(member->string, "sip.config") == 0){
			ret |= xcam_json_set_sip_config(root_r);
		#endif
*/
		} else if ((strcmp (member->string, "return_value") == 0) || (strcmp(member->string, "configlist") == 0)) {
			member = member->next;
			continue;
		} else {
			LOG_INF(LOG_TAG,"inf(%s,%d):xcam server don't support %s.\n",__func__,__LINE__,member->valuestring);
			ret = XCAM_ERROR;
		}
		member = member->next;
	}
	if (ret != XCAM_SUCCESS) {
		ret_s = cJSON_CreateNumber(CMD_ACK_STATUS_ERR);
		cJSON_AddItemToObject(root_s, "return_value", ret_s);
	} else {
		ret_s = cJSON_CreateNumber(CMD_ACK_STATUS_SUCCESS);
		cJSON_AddItemToObject(root_s, "return_value", ret_s);
	}

	json_str = cJSON_Print(root_s);
	cJSON_Delete(root_r);
	cJSON_Delete(root_s);

	skt_msg_t *buf = (skt_msg_t *)malloc(sizeof(skt_msg_t) + strlen(json_str));
	memset(buf, 0, sizeof(skt_msg_t) + strlen(json_str));
	buf->header.cmd = CMD_WEB_CFGS_ACK;
	buf->header.len = strlen(json_str);
	buf->header.flags = 1;
	memcpy(buf->data, json_str, strlen(json_str));
	if (socket_server_tcp_send(plink, (skt_msg_t *)buf) < 0) {
		LOG_ERR(LOG_TAG,"err(%s,%d): server send error\n", __func__, __LINE__);
		ret = XCAM_ERROR;
	}

	free(json_str);
	free(buf);
	return ret;
}

int xcam_web_socket_get_configs_process(socket_tcp_link_t *plink)
{
//	skt_msg_header_t *header = (skt_msg_header_t *)plink->msgbuf;
	char *data = plink->msgbuf->data;
	char *json_str = NULL;
	int ret = XCAM_SUCCESS;
	cJSON *root =NULL,*member = NULL,*ret_s = NULL,*root_err = NULL,*cfglist = NULL;

	if (data == NULL) {
		LOG_ERR(LOG_TAG,"err(%s,%d): recv data is NULL.\n", __func__, __LINE__);
		ret = XCAM_ERROR;
	}

	root = cJSON_Parse(data);
	if (root == NULL) {
		LOG_ERR(LOG_TAG,"err(%s,%d):recv data format error,cJSON parse fail.\n",__func__,__LINE__);
		ret = XCAM_ERROR;
	}
#if 1
	char *json_recv = cJSON_Print(root);
	printf("GET%s-%d\n %s\n",__func__,__LINE__,json_recv);
#endif

	root_err = cJSON_CreateObject();
	cfglist = cJSON_GetObjectItem(root, "configlist");
	if(cfglist == NULL) {
		ret = XCAM_ERROR;
	} else {
		member = cfglist->child;
	}

	if ((ret == XCAM_SUCCESS) && (cfglist->child == NULL)) {
		ret |= xcam_json_get_video_isp_fps(root);
		ret |= xcam_json_get_video_bps(root);
		ret |= xcam_json_get_video_resolution(root);
		ret |= xcam_json_get_sys_ipconfig(root);
		ret |= xcam_json_get_video_encode_mode(root);
		ret |= xcam_json_get_device_info(root);
		ret |= xcam_json_get_video_Iframe_interval(root);
		ret |= xcam_json_get_video_Rcmode(root);
		ret |= xcam_json_get_web_http_port(root);
		ret |= xcam_json_get_channel_switch(root);
		ret |= xcam_json_get_rtsp_addr(root);
	} else {
		member = cfglist->child;
		while ((ret == XCAM_SUCCESS) && member) {
			if (strcmp(member->string, "video.isp.fps") == 0) {
				ret |= xcam_json_get_video_isp_fps(root);
			} else if (strcmp(member->string, "video.bitrate") == 0) {
				ret |= xcam_json_get_video_bps(root);
			} else if (strcmp(member->string, "video.resolution") == 0) {
				ret |= xcam_json_get_video_resolution(root);
			} else if (strcmp(member->string,"video.enctype") == 0) {
				ret |= xcam_json_get_video_encode_mode(root);
			} else if (strcmp(member->string,"video.ispcontrolInfo") == 0) {
				ret |= xcam_json_get_video_ispcontrolInfo(root);
			} else if (strcmp(member->string,"video.image_control") == 0) {
				ret |= xcam_json_get_video_image_control(root);
			} else if (strcmp(member->string,"network.information") == 0) {
				ret |= xcam_json_get_sys_ipconfig(root);
			} else if (strcmp(member->string, "product.information") == 0) {
				ret |= xcam_json_get_device_info(root);
			} else if (strcmp(member->string, "video.goplength") == 0) {
				ret |= xcam_json_get_video_Iframe_interval(root);
			} else if (strcmp(member->string, "video.rcmode") == 0) {
				ret |= xcam_json_get_video_Rcmode(root);
			} else if (strcmp(member->string, "web.httpport") == 0) {
				ret |= xcam_json_get_web_http_port(root);
			} else if (strcmp(member->string, "video.switch") == 0) {
				ret |= xcam_json_get_channel_switch(root);
			} else if (strcmp(member->string, "video.rtspaddr") == 0) {
				ret |= xcam_json_get_rtsp_addr(root);
/*
			#if((defined GB28181) || (defined GB35114))
			} else if (strcmp(member->string, "sip.config") == 0) {
				ret |= xcam_json_get_sip_config(root);
			#endif
*/
			} else if (strcmp(member->string, "action") == 0) {
				member = member->next;
				continue;
			} else {
				LOG_INF(LOG_TAG,"inf(%s,%d):xcam server don't support %s.\n",__func__,__LINE__,member->valuestring);
				ret = XCAM_ERROR;
			}
			member = member->next;
		}
	}

	if (ret != XCAM_SUCCESS) {
		ret_s = cJSON_CreateNumber(CMD_ACK_STATUS_ERR);
		cJSON_AddItemToObject(root_err, "return_value", ret_s);
		json_str = cJSON_Print(root_err);
	} else {
		ret_s = cJSON_CreateNumber(CMD_ACK_STATUS_SUCCESS);
		cJSON_AddItemToObject(root, "return_value", ret_s);
		json_str = cJSON_Print(root);
	}

	cJSON_Delete(root);
	cJSON_Delete(root_err);

	skt_msg_t *buf = (skt_msg_t *)malloc(sizeof(skt_msg_t) + strlen(json_str));
	memset(buf, 0, sizeof(skt_msg_t) + strlen(json_str));
	buf->header.cmd = CMD_WEB_CFGS_ACK;
	buf->header.len = strlen(json_str);
	buf->header.flags = 1;
	memcpy(buf->data, json_str, strlen(json_str));
	if (socket_server_tcp_send(plink, (skt_msg_t *)buf) < 0) {
		LOG_ERR(LOG_TAG,"err(%s,%d): server send error.\n", __func__, __LINE__);
		ret = XCAM_ERROR;
	}
	free(json_str);
	free(buf);
	return ret;
}

int xcam_web_socket_config_process(socket_tcp_link_t *plink)
{
	int ret = XCAM_SUCCESS;
	char *data = plink->msgbuf->data;
	cJSON *root =NULL, *cfglist = NULL, *pAction = NULL;
	int action = -1;

	assert(plink != NULL);
	if (data == NULL) {
		LOG_ERR(LOG_TAG,"err(%s,%d): recv data is NULL.\n", __func__, __LINE__);
		ret |= XCAM_ERROR;
	}

	root = cJSON_Parse(data);
	if (root == NULL) {
		LOG_ERR(LOG_TAG,"err(%s,%d):recv data format error,cJSON parse fail.\n",__func__,__LINE__);
		ret |= XCAM_ERROR;
	}

	cfglist = cJSON_GetObjectItem(root, "configlist");
	if (cfglist == NULL) {
		ret |= XCAM_ERROR;
		LOG_ERR(LOG_TAG,"err(%s,%d):get configlist fail.\n",__func__,__LINE__);
	}

	pAction = cJSON_GetObjectItem(cfglist, "action");
	if (pAction == NULL) {
		ret |= XCAM_ERROR;
		LOG_ERR(LOG_TAG,"err(%s,%d):get action fail.\n",__func__,__LINE__);
	} else {
		action = pAction->valueint;
	}

	cJSON_Delete(root);
	switch (action) {
		case WEB_XCAM_GET_CONFIGS :
			ret |= xcam_web_socket_get_configs_process(plink);
			if (ret < XCAM_SUCCESS) {
				LOG_ERR(LOG_TAG,"error(%s,%d),call xcam_web_socket_get_configs_process fail.\n",__func__,__LINE__);
			}
			break;
		case WEB_XCAM_SET_CONFIGS :
			ret |= xcam_web_socket_set_configs_process(plink);
			if (ret < 0) {
				LOG_ERR(LOG_TAG,"error(%s,%d).call xcam_web_socket_set_config_process fail.\n",__func__,__LINE__);
			}
			break;
		default :
			LOG_ERR(LOG_TAG,"error(%s,%d),the action don't support.\n",__func__,__LINE__);
			break;
	}

	return ret;
}

