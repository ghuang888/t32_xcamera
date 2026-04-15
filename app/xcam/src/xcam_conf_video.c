#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <imp/imp_common.h>
#include <imp/imp_encoder.h>
#include <imp/imp_log.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <imp/imp_common.h>

#include "xcam_log.h"
#include "cJSON.h"
#include "xcam_conf_video.h"
#include "xcam_conf_json.h"
#include "xcam_general.h"
#include "xcam_com.h"
#include "xcam_cli_options.h"

#define CONFIG_VIDEO_FILE "/system/etc/video.json"
#define LOG_TAG "VideoJsonConf"
#define TAG "Sample-Common"

static int _xcam_conf_set_video_isp_fps(cJSON *root, int fps_num, int fps_den)
{
	cJSON *member = NULL, *item_1 = NULL, *item_2 = NULL;

	if (((member = cJSON_GetObjectItem(root,"isp")) != NULL) && ((item_1 = cJSON_GetObjectItem(member,"fps_num")) != NULL) && ((item_2 = cJSON_GetObjectItem(member,"fps_den")) != NULL)) {
		item_1->valueint = fps_num;
		item_2->valueint = fps_den;
	} else {
		LOG_ERR(LOG_TAG,"err(%s,%d):configer file error.\n",__func__,__LINE__);
		return XCAM_ERROR;
	}

	return XCAM_SUCCESS;
}

static int _xcam_conf_get_ivdc_status(cJSON *root, int *enable)
{
	cJSON *member = NULL, *item_1 = NULL;

	if (((member = cJSON_GetObjectItem(root, "ivdc")) != NULL) && ((item_1 = cJSON_GetObjectItem(member, "enable")) != NULL)) {
		*enable = item_1->valueint;
	} else {
		LOG_ERR(LOG_TAG,"err(%s,%d):configer file error.\n",__func__,__LINE__);
		return XCAM_ERROR;
	}

	return XCAM_SUCCESS;
}

static int _xcam_conf_get_persondet_status(cJSON *root, int *enable)
{
	cJSON *member = NULL, *item_1 = NULL;

	if (((member = cJSON_GetObjectItem(root, "persondet")) != NULL) && ((item_1 = cJSON_GetObjectItem(member, "enable")) != NULL)) {
		*enable = item_1->valueint;
	} else {
		LOG_ERR(LOG_TAG,"err(%s,%d):configer file error.\n",__func__,__LINE__);
		return XCAM_ERROR;
	}

	return XCAM_SUCCESS;
}

#define ArrayLen(arr) (sizeof(arr) / sizeof(arr[0]))
const char *extraArgValues[]={"movedetEnable", "platerecEnable", "facedetEnable", "pervehpetEnable","fireworksdetEnable"};
static int _xcam_conf_get_extraarg_status(cJSON *root, int *enable, int length)
{
	cJSON *member = NULL, *item_1 = NULL;
	if ((member = cJSON_GetObjectItem(root, "extraarg")) == NULL)  {
		LOG_ERR(LOG_TAG,"err(%s,%d):configer file error.\n",__func__,__LINE__);
		return XCAM_ERROR;
	}
	if(length < ArrayLen(extraArgValues)){
		LOG_ERR(LOG_TAG,"err(%s,%d):item size error error.\n",__func__,__LINE__);
		return XCAM_ERROR;
	}
	for(int i=0; i<length; i++){
		item_1 = cJSON_GetObjectItem(member, extraArgValues[i]);
		if(item_1 != NULL)
			enable[i] = item_1->valueint;
	}
	return XCAM_SUCCESS;
}

static int _xcam_conf_get_extraargaiisp_status(cJSON *root, int *enable){
	cJSON *member = NULL, *item_1 = NULL;
	if ((member = cJSON_GetObjectItem(root, "extraarg")) == NULL)  {
		LOG_ERR(LOG_TAG,"err(%s,%d):configer file error.\n",__func__,__LINE__);
		return XCAM_ERROR;
	}
	item_1 = cJSON_GetObjectItem(member, "aiispEnable");
	if(item_1 != NULL)
		*enable = item_1->valueint;
	return XCAM_SUCCESS;
}
static int _xcam_conf_get_video_isp_fps(cJSON *root, int *fps_num, int *fps_den)
{
	cJSON *member = NULL, *item_1 = NULL, *item_2 = NULL;

	if (((member = cJSON_GetObjectItem(root, "isp")) != NULL) && ((item_1 = cJSON_GetObjectItem(member, "fps_num")) != NULL) && ((item_2 = cJSON_GetObjectItem(member, "fps_den")) != NULL)) {
		*fps_num = item_1->valueint;
		*fps_den = item_2->valueint;
	} else {
		LOG_ERR(LOG_TAG,"err(%s,%d):configer file error.\n",__func__,__LINE__);
		return XCAM_ERROR;
	}

	return XCAM_SUCCESS;
}

static int _xcam_conf_get_video_ispcontrolInfo(cJSON *root, int *nightvision, int *wdr, int *imagemirror, int *imageflip, int *ircut, int *noisereduction, int *drc)
{
	cJSON *item = NULL, *root_value = NULL;

	if ((item = cJSON_GetObjectItem(root, "isp")) == NULL) {
		LOG_ERR(LOG_TAG,"err(%s,%d):configer file fail.\n",__func__,__LINE__);
		return XCAM_ERROR;
	}

	if ((root_value = cJSON_GetObjectItem(item,"nightvisionmode")) == NULL) {
		LOG_ERR(LOG_TAG,"err(%s,%d): do not have nightvision tag\n", __func__, __LINE__);
		return XCAM_ERROR;
	}
	*nightvision = root_value->valueint;

	if ((root_value = cJSON_GetObjectItem(item,"wdrmode")) == NULL) {
		LOG_ERR(LOG_TAG,"err(%s,%d): do not have wdr tag\n", __func__, __LINE__);
		return XCAM_ERROR;
	}
	*wdr = root_value->valueint;

	if ((root_value = cJSON_GetObjectItem(item,"imagemirrormode")) == NULL) {
		LOG_ERR(LOG_TAG,"err(%s,%d): do not have imagemirror tag\n", __func__, __LINE__);
		return XCAM_ERROR;
	}
	*imagemirror = root_value->valueint;

	if ((root_value = cJSON_GetObjectItem(item,"imageflipmode")) == NULL) {
		LOG_ERR(LOG_TAG,"err(%s,%d): do not have imageflip tag\n", __func__, __LINE__);
		return XCAM_ERROR;
	}
	*imageflip = root_value->valueint;

	if ((root_value = cJSON_GetObjectItem(item,"ircutmode")) == NULL) {
		LOG_ERR(LOG_TAG,"err(%s,%d): do not have ircut tag\n", __func__, __LINE__);
		return XCAM_ERROR;
	}
	*ircut = root_value->valueint;

	if ((root_value = cJSON_GetObjectItem(item,"noisereductionmode")) == NULL) {
		LOG_ERR(LOG_TAG,"err(%s,%d): do not have noisereduction tag\n", __func__, __LINE__);
		return XCAM_ERROR;
	}
	*noisereduction = root_value->valueint;

	if ((root_value = cJSON_GetObjectItem(item,"drcmode")) == NULL) {
		LOG_ERR(LOG_TAG,"err(%s,%d): do not have drc tag\n", __func__, __LINE__);
		return XCAM_ERROR;
	}
	*drc = root_value->valueint;

	return XCAM_SUCCESS;
}

static int _xcam_conf_get_video_image_control(cJSON *json_root, int *saturation, int *brightness, int *sharpness, int *contrast)
{
	cJSON *root = NULL, *root_value = NULL;

	if ((root = cJSON_GetObjectItem (json_root,"isp")) == NULL) {
		LOG_ERR(LOG_TAG,"err(%s,%d): do not have image_control tag\n", __func__, __LINE__);
		return XCAM_ERROR;
	}

	if ((root_value = cJSON_GetObjectItem(root,"saturation")) == NULL) {
		LOG_ERR(LOG_TAG,"err(%s,%d): do not have saturation tag\n", __func__, __LINE__);
		return XCAM_ERROR;
	}
	*saturation = root_value->valueint;

	if ((root_value = cJSON_GetObjectItem(root,"brightness")) == NULL) {
		LOG_ERR(LOG_TAG,"err(%s,%d): do not have brightness tag\n", __func__, __LINE__);
		return XCAM_ERROR;
	}
	*brightness = root_value->valueint;

	if ((root_value = cJSON_GetObjectItem(root,"sharpness")) == NULL) {
		LOG_ERR(LOG_TAG,"err(%s,%d): do not have sharpness tag\n", __func__, __LINE__);
		return XCAM_ERROR;
	}
	*sharpness = root_value->valueint;

	if ((root_value = cJSON_GetObjectItem(root,"contrast")) == NULL) {
		LOG_ERR(LOG_TAG,"err(%s,%d): do not have contrast tag\n", __func__, __LINE__);
		return XCAM_ERROR;
	}
	*contrast = root_value->valueint;

	return XCAM_SUCCESS;
}

static int _xcam_conf_set_video_bps(cJSON *root, int channel, conf_enc_rc_t *enc_rc )
{
	cJSON *member = NULL, *item_1 = NULL,*item_3 = NULL , *item_4 = NULL;
	if ((member = cJSON_GetObjectItem(root,"enc")) == NULL)
		goto CONFIG_FILE_ERROR;

	if (channel == 0)
		item_1 = cJSON_GetObjectItem(member, "ch0_enc_rc");
	else
		item_1 = cJSON_GetObjectItem(member, "ch1_enc_rc");

	if (enc_rc->rc_type == ENC_RC_MODE_CBR ) {
		item_3 = cJSON_GetObjectItem(item_1, "target_bitrate");
		if (item_3 == NULL)
			goto CONFIG_FILE_ERROR;
		item_3->valueint = enc_rc->rc_cbr.target_bitrate;
	} else if (enc_rc->rc_type == ENC_RC_MODE_VBR) {
		item_3 = cJSON_GetObjectItem(item_1, "target_bitrate");
		item_4 = cJSON_GetObjectItem(item_1, "max_bitrate");
		if ((item_3 == NULL) || (item_4 == NULL))
			goto CONFIG_FILE_ERROR;
		item_3->valueint = enc_rc->rc_vbr.target_bitrate;
		item_4->valueint = enc_rc->rc_vbr.max_bitrate;
	} else if (enc_rc->rc_type == ENC_RC_MODE_CVBR) {
		item_3 = cJSON_GetObjectItem(item_1, "target_bitrate");
		item_4 = cJSON_GetObjectItem(item_1, "max_bitrate");
		if ((item_3 == NULL) || (item_4 == NULL))
			goto CONFIG_FILE_ERROR;
		item_3->valueint = enc_rc->rc_capped_vbr.target_bitrate;
		item_4->valueint = enc_rc->rc_capped_vbr.max_bitrate;
	} else {
		goto CONFIG_FILE_ERROR;
	}

	return XCAM_SUCCESS;
CONFIG_FILE_ERROR:
	LOG_ERR(LOG_TAG,"err(%s,%d): config file is error.\n", __func__, __LINE__);
	return XCAM_ERROR;
}

static int _xcam_conf_get_video_bps(cJSON *root, int channel, conf_enc_rc_t *enc_rc)
{
	cJSON *member = NULL, *item_1 = NULL, *item_3 = NULL, *item_4 = NULL;
	if ((member = cJSON_GetObjectItem(root,"enc")) == NULL)
		goto CONFIG_FILE_ERROR;

	if (channel == 0)
		item_1 = cJSON_GetObjectItem(member, "ch0_enc_rc");
	else
		item_1 = cJSON_GetObjectItem(member, "ch1_enc_rc");

	enc_rc->rc_type = ENC_RC_MODE_VBR;
	if (enc_rc->rc_type == ENC_RC_MODE_CBR ) {
		item_3 = cJSON_GetObjectItem(item_1, "target_bitrate");
		if (item_3 == NULL)
			goto CONFIG_FILE_ERROR;
		enc_rc->rc_cbr.target_bitrate =  item_3->valueint;
	} else if (enc_rc->rc_type == ENC_RC_MODE_VBR) {
		item_3 = cJSON_GetObjectItem(item_1, "target_bitrate");
		item_4 = cJSON_GetObjectItem(item_1, "max_bitrate");
		if ((item_3 == NULL) || (item_4 == NULL))
			goto CONFIG_FILE_ERROR;
		enc_rc->rc_vbr.target_bitrate = item_3->valueint;
		enc_rc->rc_vbr.max_bitrate = item_4->valueint;
	} else if (enc_rc->rc_type == ENC_RC_MODE_CVBR) {
		item_3 = cJSON_GetObjectItem(item_1, "target_bitrate");
		item_4 = cJSON_GetObjectItem(item_1, "max_bitrate");
		if ((item_3 == NULL) || (item_4 == NULL))
			goto CONFIG_FILE_ERROR;
		enc_rc->rc_vbr.target_bitrate = item_3->valueint;
		enc_rc->rc_vbr.max_bitrate = item_4->valueint;
	} else {
		return XCAM_ERROR;
	}
	return XCAM_SUCCESS;

CONFIG_FILE_ERROR:
	LOG_ERR(LOG_TAG,"err(%s,%d): config file is error.\n", __func__, __LINE__);
	return XCAM_ERROR;

}

static int _xcam_conf_set_video_resolution(cJSON *root, int channel, int picheight, int picwidth)
{
	cJSON *member = NULL, *item_1 = NULL, *item_2 = NULL;
	if ((member = cJSON_GetObjectItem(root ,"fs")) !=NULL) {
		if (channel == 0) {
			item_1 = cJSON_GetObjectItem(member, "ch0_w");
			item_2 = cJSON_GetObjectItem(member, "ch0_h");
			if ( (item_1 == NULL) || (item_2 == NULL) ) {
				LOG_ERR(LOG_TAG,"configuration file error.\n");
				return XCAM_ERROR;
			}
			item_1->valueint = picwidth;
			item_2->valueint = picheight;

		} else {
			item_1 = cJSON_GetObjectItem(member, "ch1_w");
			item_2 = cJSON_GetObjectItem(member, "ch1_h");
			if ( (item_1 == NULL) || (item_2 == NULL) ) {
				LOG_ERR(LOG_TAG,"configuration file error.\n");
				return XCAM_ERROR;
			}
			item_1->valueint = picwidth;
			item_2->valueint = picheight;
		}
	} else {
		LOG_ERR(LOG_TAG,"err(%s,%d):configer file error.\n",__func__,__LINE__);
		return XCAM_ERROR;
	}

	printf("set resolution success, current width = %d, height = %d\n", picwidth, picheight);

	printf("set resolution success, current width = %d, height = %d\n", picwidth, picheight);

	return XCAM_SUCCESS;
}

static int _xcam_conf_get_video_resolution(cJSON *root,int channel, int *picheight, int *picwidth)
{
	cJSON *member = NULL, *item_1 = NULL, *item_2 = NULL;
	if ((member = cJSON_GetObjectItem(root ,"fs")) !=NULL) {
		if (channel == 0) {
			item_1 = cJSON_GetObjectItem(member, "ch0_w");
			item_2 = cJSON_GetObjectItem(member, "ch0_h");
			if ( (item_1 == NULL) || (item_2 == NULL) ) {
				LOG_ERR(LOG_TAG,"configuration file error.\n");
				return XCAM_ERROR;
			}
			*picwidth = item_1->valueint;
			*picheight = item_2->valueint;

		} else {
			item_1 = cJSON_GetObjectItem(member, "ch1_w");
			item_2 = cJSON_GetObjectItem(member, "ch1_h");
			if ( (item_1 == NULL) || (item_2 == NULL) ) {
				LOG_ERR(LOG_TAG,"configuration file error.\n");
				return XCAM_ERROR;
			}
			*picwidth = item_1->valueint;
			*picheight = item_2->valueint;
		}
	} else {
		LOG_ERR(LOG_TAG,"err(%s,%d):configer file error.\n",__func__,__LINE__);
		return XCAM_ERROR;
	}

	return XCAM_SUCCESS;
}

static int _xcam_conf_set_video_enctype(cJSON *root, int channel, int enctype)
{
	cJSON *member = NULL, *item = NULL;
	char *enc_type_string = NULL;
	if(enctype == PT_H264)
		enc_type_string = "h264";
	else if(enctype == PT_H265 )
		enc_type_string = "h265";

	if ((member = cJSON_GetObjectItem(root->child,"enc")) != NULL) {
		if (channel == 0) {
			item = cJSON_GetObjectItem(member,"ch0_type");
			if (item == NULL) {
				LOG_ERR(LOG_TAG,"configuration file error.\n");
				return XCAM_ERROR;
			}
			strcpy(item->valuestring, enc_type_string);
		} else {
			item = cJSON_GetObjectItem(member,"ch1_type");
			if (item == NULL) {
				LOG_ERR(LOG_TAG,"configuration file error.\n");
				return XCAM_ERROR;
			}
			strcpy(item->valuestring, enc_type_string);
		}
	} else {
		LOG_ERR(LOG_TAG,"err(%s,%d):configer file error.\n",__func__,__LINE__);
		return XCAM_ERROR;
	}

	return XCAM_SUCCESS;
}

static int _xcam_conf_get_video_enctype(cJSON *root, int channel, int *enctype)
{
	cJSON *member = NULL, *item = NULL;

	if ((member = cJSON_GetObjectItem(root,"enc")) != NULL) {
		if (channel == 0) {
			item = cJSON_GetObjectItem(member,"ch0_type");
			if (item == NULL) {
				LOG_ERR(LOG_TAG,"configuration file error.\n");
				return XCAM_ERROR;
			}
			if (strcmp(item->valuestring, "h264") == 0)
				*enctype = PT_H264;
			else if(strcmp(item->valuestring,"h265") == 0)
				*enctype = PT_H265;
		} else {
			item = cJSON_GetObjectItem(member,"ch1_type");
			if (item == NULL) {
				LOG_ERR(LOG_TAG,"configuration file error.\n");
				return XCAM_ERROR;
			}

			if (strcmp(item->valuestring, "h264") == 0)
				*enctype = PT_H264;
			else if(strcmp(item->valuestring,"h265") == 0)
				*enctype = PT_H265;
		}
	} else {
		LOG_ERR(LOG_TAG,"err(%s,%d):configer file error.\n",__func__,__LINE__);
		return XCAM_ERROR;
	}

	return XCAM_SUCCESS;
}

static int _xcam_conf_set_video_ispcontrolInfo(cJSON *root, conf_video_set_flag flag, int status)
{
#if 1
	cJSON *item = NULL;
	char *flag_type = NULL;
	item = cJSON_GetObjectItem(root, "isp");
	if (NULL == item)
	{
		LOG_ERR(LOG_TAG,"error(%s,%d),isp node not exist!\n",__func__,__LINE__);
		return -1;
	}
	switch(flag) {
		case SET_CONF_VIDEO_NIGHTVISIONMODE:
			flag_type = "nightvisionmode";
			break;
		case SET_CONF_VIDEO_WDRMODE:
			flag_type = "wdrmode";
			break;
		case SET_CONF_VIDEO_IMAGEMIRRORMODE:
			flag_type = "imagemirrormode";
			break;
		case SET_CONF_VIDEO_IMAGEFLIPMODE:
			flag_type = "imageflipmode";
			break;
		case SET_CONF_VIDEO_IRCUTMODE:
			flag_type = "ircutmode";
			break;
		case SET_CONF_VIDEO_NOISEREDUCTIONMODE:
			flag_type = "noisereductionmode";
			break;
		case SET_CONF_VIDEO_DRCMODE:
			flag_type = "drcmode";
			break;
		default :
			LOG_ERR(LOG_TAG,"flag is error.\n");
			break;
	}

	cJSON_ReplaceItemInObject(item, flag_type, cJSON_CreateNumber(status));
#endif
	return XCAM_SUCCESS;
}

static int _xcam_conf_set_video_image_control(cJSON *root, int saturation, int brightness, int sharpness, int contrast)
{
	cJSON *item = NULL;

	item = cJSON_GetObjectItem(root, "isp");
	if (NULL == item)
	{
		LOG_ERR(LOG_TAG,"error(%s,%d),isp node not exist!\n",__func__,__LINE__);
		return -1;
	}
	cJSON_ReplaceItemInObject(item,"saturation",cJSON_CreateNumber(saturation));
	cJSON_ReplaceItemInObject(item,"brightness",cJSON_CreateNumber(brightness));
	cJSON_ReplaceItemInObject(item,"sharpness",cJSON_CreateNumber(sharpness));
	cJSON_ReplaceItemInObject(item,"contrast",cJSON_CreateNumber(contrast));

	return XCAM_SUCCESS;
}

int xcam_conf_get_video(conf_video_t *video_conf)
{
	int ret = XCAM_SUCCESS;
	cJSON *root = NULL;

	if (video_conf == NULL) {
		LOG_ERR(LOG_TAG,"error(%s,%d),illegal parameter.\n",__func__,__LINE__);
		return XCAM_ERROR;
	}

	root = json_read_file(CONFIG_VIDEO_FILE);
	if (root == NULL) {
		LOG_ERR(LOG_TAG,"error(%s,%d): json read file\n",__func__, __LINE__);
		return XCAM_ERROR;
	}

	ret |= _xcam_conf_get_persondet_status(root,&video_conf->conf_persondet.enable);
	ret |= _xcam_conf_get_extraarg_status(root,(int*)(&video_conf->conf_extraarg),sizeof(video_conf->conf_extraarg) / sizeof(int));
	ret |= _xcam_conf_get_extraargaiisp_status(root,&video_conf->conf_extraarg.aiispEnable);
	ret |= _xcam_conf_get_ivdc_status(root,&video_conf->conf_ivdc.enable);
	ret |= _xcam_conf_get_video_isp_fps(root,&video_conf->conf_isp.fps_num,&video_conf->conf_isp.fps_den);
	ret |= _xcam_conf_get_video_ispcontrolInfo(root,&video_conf->conf_isp.nightvision,&video_conf->conf_isp.wdr,&video_conf->conf_isp.imagemirror,&video_conf->conf_isp.imageflip,&video_conf->conf_isp.ircut,&video_conf->conf_isp.noisereduction,&video_conf->conf_isp.drc);
	ret |= _xcam_conf_get_video_image_control(root,&video_conf->conf_isp.saturation,&video_conf->conf_isp.brightness,&video_conf->conf_isp.sharpness,&video_conf->conf_isp.contrast);
	ret |= _xcam_conf_get_video_bps(root,0,&video_conf->conf_enc.ch0_enc_rc);
	ret |= _xcam_conf_get_video_bps(root,1,&video_conf->conf_enc.ch0_enc_rc);
	ret |= _xcam_conf_get_video_resolution(root, 0, &video_conf->conf_fs.ch0_h,&video_conf->conf_fs.ch0_w);
	ret |= _xcam_conf_get_video_resolution(root, 1, &video_conf->conf_fs.ch1_h,&video_conf->conf_fs.ch1_w);
	ret |= _xcam_conf_get_video_enctype(root, 0, &video_conf->conf_enc.ch0_type);
	ret |= _xcam_conf_get_video_enctype(root, 1, &video_conf->conf_enc.ch1_type);
	if (ret != XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG,"error(%s,%d),set video config to jsonroot fail.\n",__func__,__LINE__);
		return XCAM_ERROR;
	}

	return ret;
}

int xcam_conf_set_video(conf_video_set_flag flag,conf_video_t *video_conf)
{
	if ((flag >= SET_CONF_VIDEO_MAX) || (video_conf == NULL)) {
		LOG_ERR(LOG_TAG,"error(%s,%d),illegal parameter.\n",__func__,__LINE__);
		return XCAM_ERROR;
	}

	int ret = XCAM_SUCCESS;
	cJSON *root = NULL;
	root = json_read_file(CONFIG_VIDEO_FILE);
	if (root == NULL)  {
		LOG_ERR(LOG_TAG,"error(%s,%d): json read file\n",__func__, __LINE__);
		return XCAM_ERROR;
	}

	switch (flag) {
		case SET_CONF_VIDEO_ALL:
		case SET_CONF_VIDEO_ISP_FPS:
			ret = _xcam_conf_set_video_isp_fps(root, video_conf->conf_isp.fps_num, video_conf->conf_isp.fps_den);
			if(flag == SET_CONF_VIDEO_ISP_FPS)
				break;
		case SET_CONF_VIDEO_BPS_CH0:
			ret |= _xcam_conf_set_video_bps(root, 0, &video_conf->conf_enc.ch0_enc_rc);
			if(flag == SET_CONF_VIDEO_BPS_CH0)
				break;
		case SET_CONF_VIDEO_BPS_CH1:
			ret |= _xcam_conf_set_video_bps(root, 1, &video_conf->conf_enc.ch1_enc_rc);
			if(flag == SET_CONF_VIDEO_BPS_CH1)
				break;
		case SET_CONF_VIDEO_RESOLUTION_CH0:
			ret |= _xcam_conf_set_video_resolution(root, 0, video_conf->conf_fs.ch0_h, video_conf->conf_fs.ch0_w);
			if(flag == SET_CONF_VIDEO_RESOLUTION_CH0)
				break;
		case SET_CONF_VIDEO_RESOLUTION_CH1:
			ret |= _xcam_conf_set_video_resolution(root, 1, video_conf->conf_fs.ch1_h, video_conf->conf_fs.ch1_w);
			if(flag == SET_CONF_VIDEO_RESOLUTION_CH1)
				break;
		case SET_CONF_VIDEO_ENCTYPE_CH0:
			ret |= _xcam_conf_set_video_enctype(root, 0, video_conf->conf_enc.ch0_type);
			if(flag == SET_CONF_VIDEO_ENCTYPE_CH0)
				break;
		case SET_CONF_VIDEO_ENCTYPE_CH1:
			ret |= _xcam_conf_set_video_enctype(root, 1, video_conf->conf_enc.ch1_type);
			if(flag == SET_CONF_VIDEO_ENCTYPE_CH1)
				break;
		case SET_CONF_VIDEO_NIGHTVISIONMODE:
			ret |= _xcam_conf_set_video_ispcontrolInfo(root, SET_CONF_VIDEO_NIGHTVISIONMODE, video_conf->conf_isp.nightvision);
			if(flag == SET_CONF_VIDEO_NIGHTVISIONMODE)
				break;
		case SET_CONF_VIDEO_WDRMODE:
			ret |= _xcam_conf_set_video_ispcontrolInfo(root, SET_CONF_VIDEO_WDRMODE, video_conf->conf_isp.wdr);
			if(flag == SET_CONF_VIDEO_WDRMODE)
				break;
		case SET_CONF_VIDEO_IMAGEMIRRORMODE:
			ret |= _xcam_conf_set_video_ispcontrolInfo(root, SET_CONF_VIDEO_IMAGEMIRRORMODE, video_conf->conf_isp.imagemirror);
			if(flag == SET_CONF_VIDEO_IMAGEMIRRORMODE)
				break;
		case SET_CONF_VIDEO_IMAGEFLIPMODE:
			ret |= _xcam_conf_set_video_ispcontrolInfo(root, SET_CONF_VIDEO_IMAGEFLIPMODE, video_conf->conf_isp.imageflip);
			if(flag == SET_CONF_VIDEO_IMAGEFLIPMODE)
				break;
		case SET_CONF_VIDEO_IRCUTMODE:
			ret |= _xcam_conf_set_video_ispcontrolInfo(root, SET_CONF_VIDEO_IRCUTMODE, video_conf->conf_isp.ircut);
			if(flag == SET_CONF_VIDEO_IRCUTMODE)
				break;
		case SET_CONF_VIDEO_NOISEREDUCTIONMODE:
			ret |= _xcam_conf_set_video_ispcontrolInfo(root, SET_CONF_VIDEO_NOISEREDUCTIONMODE, video_conf->conf_isp.noisereduction);
			if(flag == SET_CONF_VIDEO_NOISEREDUCTIONMODE)
				break;
		case SET_CONF_VIDEO_DRCMODE:
			ret |= _xcam_conf_set_video_ispcontrolInfo(root, SET_CONF_VIDEO_DRCMODE, video_conf->conf_isp.drc);
			if(flag == SET_CONF_VIDEO_DRCMODE)
				break;
		case SET_CONF_VIDEO_IMAGE_CONTROL:
			ret |= _xcam_conf_set_video_image_control(root, video_conf->conf_isp.saturation, video_conf->conf_isp.brightness, video_conf->conf_isp.sharpness, video_conf->conf_isp.contrast);
			if(flag == SET_CONF_VIDEO_IMAGE_CONTROL)
				break;
		default:
			LOG_INF(LOG_TAG,"info(%s,%d)don't support this flag.\n",__func__,__LINE__);
			break;
	}
	if (ret != 0) {
		LOG_ERR(LOG_TAG,"error(%s,%d),set video config to jsonroot fail.\n",__func__,__LINE__);
		return XCAM_ERROR;
	}
/*
	ret = json_write_file(CONFIG_VIDEO_FILE,root);
	if (ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG,"errno(%s,%d),Call json_write_file fail.\n",__func__,__LINE__);
	}
*/
	return ret;
}

int xcam_conf_set_video_isp_fps(int fps_num, int fps_den)
{
	int ret = XCAM_SUCCESS, flag = SET_CONF_VIDEO_ISP_FPS;

	if (cli_attr.no_operation_file_flag == true)
		return ret;

	xcam_video_conf.conf_isp.fps_num = fps_num;
	xcam_video_conf.conf_isp.fps_den = fps_den;
	ret = xcam_conf_set_video(flag,&xcam_video_conf);
	if (ret < XCAM_SUCCESS)
		LOG_ERR(LOG_TAG,"error,call xcam_conf_set_video fail.\n");

	return ret;
}

int xcam_conf_set_video_bps(int channel,int bps_num)
{
	int ret = XCAM_SUCCESS, flag = XCAM_INVALID_FLAG;

	if (cli_attr.no_operation_file_flag == true)
		return ret;

	if (channel == 0) {
		flag = SET_CONF_VIDEO_BPS_CH0;
		if (xcam_video_conf.conf_enc.ch0_enc_rc.rc_type == ENC_RC_MODE_FIXQP) {
			LOG_INF(LOG_TAG,"fixqp do not support modify bitrate");
			return XCAM_ERROR;
		} else if (xcam_video_conf.conf_enc.ch0_enc_rc.rc_type == ENC_RC_MODE_CBR) {
			xcam_video_conf.conf_enc.ch0_enc_rc.rc_cbr.target_bitrate = bps_num;
		} else if (xcam_video_conf.conf_enc.ch0_enc_rc.rc_type == ENC_RC_MODE_VBR) {
			xcam_video_conf.conf_enc.ch0_enc_rc.rc_vbr.target_bitrate = bps_num;
			xcam_video_conf.conf_enc.ch0_enc_rc.rc_vbr.max_bitrate = bps_num;
		} else if (xcam_video_conf.conf_enc.ch0_enc_rc.rc_type == ENC_RC_MODE_CVBR) {
			xcam_video_conf.conf_enc.ch0_enc_rc.rc_capped_vbr.target_bitrate = bps_num;
			xcam_video_conf.conf_enc.ch0_enc_rc.rc_capped_vbr.max_bitrate = bps_num;
		} else {
			LOG_ERR(LOG_TAG,"rc type is error.\n");
			return XCAM_ERROR;
		}
	} else if(channel == 1) {
		flag = SET_CONF_VIDEO_BPS_CH1;
		if (xcam_video_conf.conf_enc.ch1_enc_rc.rc_type == ENC_RC_MODE_FIXQP) {
			LOG_INF(LOG_TAG,"fixqp do not support modify bitrate.\n");
			return XCAM_ERROR;
		} else if (xcam_video_conf.conf_enc.ch1_enc_rc.rc_type == ENC_RC_MODE_CBR) {
			xcam_video_conf.conf_enc.ch1_enc_rc.rc_cbr.target_bitrate = bps_num;
		} else if (xcam_video_conf.conf_enc.ch1_enc_rc.rc_type == ENC_RC_MODE_VBR) {
			xcam_video_conf.conf_enc.ch1_enc_rc.rc_vbr.target_bitrate = bps_num;
			xcam_video_conf.conf_enc.ch1_enc_rc.rc_vbr.max_bitrate = bps_num;
		} else if (xcam_video_conf.conf_enc.ch1_enc_rc.rc_type == ENC_RC_MODE_CVBR) {
			xcam_video_conf.conf_enc.ch1_enc_rc.rc_capped_vbr.target_bitrate = bps_num;
			xcam_video_conf.conf_enc.ch1_enc_rc.rc_capped_vbr.max_bitrate = bps_num;
		} else {
			LOG_ERR(LOG_TAG,"rc type is error.\n");
			return XCAM_ERROR;
		}
	} else {
		LOG_ERR(LOG_TAG,"error,don't support this type.\n");
		return XCAM_ERROR;
	}

	ret = xcam_conf_set_video(flag,&xcam_video_conf);
	if (ret < 0)
		LOG_ERR(LOG_TAG,"error,call xcam_conf_set_video fail.\n");

	return ret;
}

int xcam_conf_set_video_resolution(int channel,int picheight,int picwidth)
{
	int ret = XCAM_SUCCESS, flag = XCAM_INVALID_FLAG;

	if (cli_attr.no_operation_file_flag == true)
		return ret;

	if (channel == 0) {
		flag = SET_CONF_VIDEO_RESOLUTION_CH0;
		xcam_video_conf.conf_fs.ch0_w = picwidth;
		xcam_video_conf.conf_fs.ch0_h = picheight;
	} else if(channel == 1) {
		flag = SET_CONF_VIDEO_RESOLUTION_CH1;
		xcam_video_conf.conf_fs.ch1_w = picwidth;
		xcam_video_conf.conf_fs.ch1_h = picheight;
	} else {
		LOG_ERR(LOG_TAG,"errno(%s,%d),don't support this type.\n",__func__,__LINE__);
		return XCAM_ERROR;
	}

	printf("#######[%d]- [%d]\n", picheight, picwidth);
	ret = xcam_conf_set_video(flag, &xcam_video_conf);
	if (ret < 0)
		LOG_ERR(LOG_TAG,"error(%s,%d),call xcam_conf_set_video fail.\n",__func__,__LINE__);

	return ret;
}

int xcam_conf_set_video_enctype(int channel, int enctype)
{
	int ret = XCAM_SUCCESS, flag = XCAM_INVALID_FLAG;

	if (cli_attr.no_operation_file_flag == true)
		return ret;

	if (channel == 0) {
		flag = SET_CONF_VIDEO_ENCTYPE_CH0;
		xcam_video_conf.conf_enc.ch0_type = enctype;
	} else if(channel == 1) {
		flag = SET_CONF_VIDEO_ENCTYPE_CH1;
		xcam_video_conf.conf_enc.ch1_type = enctype;
	} else {
		LOG_ERR(LOG_TAG,"errno,don't support this type.\n");
		return XCAM_ERROR;
	}

	ret = xcam_conf_set_video(flag,&xcam_video_conf);
	if (ret < 0)
		LOG_ERR(LOG_TAG,"error,call xcam_conf_set_video fail.\n");

	return ret;
}

int xcam_conf_set_video_all(conf_video_t *video_conf)
{
	int ret = XCAM_SUCCESS, flag = SET_CONF_VIDEO_ALL;

	if (cli_attr.no_operation_file_flag == true)
		return ret;

	ret = xcam_conf_set_video(flag,&xcam_video_conf);
	if (ret < XCAM_SUCCESS)
		LOG_ERR(LOG_TAG,"error,call xcam_conf_set_video fail.\n");

	return ret;
}

int xcam_conf_set_video_nightvisionmode(int nightvision)
{
	int ret = XCAM_SUCCESS, flag = SET_CONF_VIDEO_NIGHTVISIONMODE;

	if (cli_attr.no_operation_file_flag == true)
		return ret;

	xcam_video_conf.conf_isp.nightvision = nightvision;
	ret = xcam_conf_set_video(flag,&xcam_video_conf);
	if (ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG,"error(%s,%d),call xcam_conf_set_video_nightvisionmode fail.\n",__func__,__LINE__);
	}

	return ret;
}

int xcam_conf_set_video_wdrmode(int wdr)
{
	int ret = XCAM_SUCCESS, flag = SET_CONF_VIDEO_WDRMODE;

	if (cli_attr.no_operation_file_flag == true)
		return ret;

	xcam_video_conf.conf_isp.wdr = wdr;
	ret = xcam_conf_set_video(flag,&xcam_video_conf);
	if (ret < XCAM_SUCCESS)
		LOG_ERR(LOG_TAG,"error(%s,%d),call xcam_conf_set_video_wdr fail.\n",__func__,__LINE__);

	return ret;
}

int xcam_conf_set_video_imagemirrormode(int imagemirror)
{
	int ret = XCAM_SUCCESS, flag = SET_CONF_VIDEO_IMAGEMIRRORMODE;

	if (cli_attr.no_operation_file_flag == true)
		return ret;

	xcam_video_conf.conf_isp.imagemirror = imagemirror;
	ret = xcam_conf_set_video(flag,&xcam_video_conf);
	if (ret < XCAM_SUCCESS)
		LOG_ERR(LOG_TAG,"error,call xcam_conf_set_video_imagemirror fail.\n");

	return ret;
}

int xcam_conf_set_video_imageflipmode(int imageflip)
{
	int ret = XCAM_SUCCESS, flag = SET_CONF_VIDEO_IMAGEFLIPMODE;

	if (cli_attr.no_operation_file_flag == true)
		return ret;

	xcam_video_conf.conf_isp.imageflip = imageflip;
	ret = xcam_conf_set_video(flag,&xcam_video_conf);
	if (ret < XCAM_SUCCESS)
		LOG_ERR(LOG_TAG,"error,call xcam_conf_set_video_imageflip fail.\n");

	return ret;
}

int xcam_conf_set_video_ircutmode(int ircut)
{
	int ret = XCAM_SUCCESS; //flag = SET_CONF_VIDEO_IRCUTMODE fo warning

	if (cli_attr.no_operation_file_flag == true)
		return ret;
	ircut = !ircut;
	xcam_video_conf.conf_isp.ircut = ircut;
	/*
	ret = xcam_conf_set_video(flag,&xcam_video_conf);
	if (ret < XCAM_SUCCESS)
		LOG_ERR(LOG_TAG,"error,call xcam_conf_set_video_ircut fail.\n");
	*/
	/*
	   IRCUTN = PB17
	   IRCUTP = PB18
	*/
	int fd, fd0, fd1;
	char on[4], off[4];

	int gpio0 = 49;
	int gpio1 = 50;
	char tmp[128];

	if (!access("/tmp/setir",0)) {
		if (ircut) {
			system("/tmp/setir 0 1");
		} else {
			system("/tmp/setir 1 0");
		}
		return 0;
	}

	fd = open("/sys/class/gpio/export", O_WRONLY);
	if(fd < 0) {
		IMP_LOG_DBG(TAG, "open /sys/class/gpio/export error !");
		return -1;
	}

	sprintf(tmp, "%d", gpio0);
	write(fd, tmp, 2);
	sprintf(tmp, "%d", gpio1);
	write(fd, tmp, 2);

	close(fd);

	sprintf(tmp, "/sys/class/gpio/gpio%d/direction", gpio0);
	fd0 = open(tmp, O_RDWR);
	if(fd0 < 0) {
		IMP_LOG_DBG(TAG, "open /sys/class/gpio/gpio%d/direction error !", gpio0);
		return -1;
	}

	sprintf(tmp, "/sys/class/gpio/gpio%d/direction", gpio1);
	fd1 = open(tmp, O_RDWR);
	if(fd1 < 0) {
		IMP_LOG_DBG(TAG, "open /sys/class/gpio/gpio%d/direction error !", gpio1);
		return -1;
	}

	write(fd0, "out", 3);
	write(fd1, "out", 3);

	close(fd0);
	close(fd1);

	sprintf(tmp, "/sys/class/gpio/gpio%d/active_low", gpio0);
	fd0 = open(tmp, O_RDWR);
	if(fd0 < 0) {
		IMP_LOG_DBG(TAG, "open /sys/class/gpio/gpio%d/active_low error !", gpio0);
		return -1;
	}

	sprintf(tmp, "/sys/class/gpio/gpio%d/active_low", gpio1);
	fd1 = open(tmp, O_RDWR);
	if(fd1 < 0) {
		IMP_LOG_DBG(TAG, "open /sys/class/gpio/gpio%d/active_low error !", gpio1);
		return -1;
	}

	write(fd0, "0", 1);
	write(fd1, "0", 1);

	close(fd0);
	close(fd1);

	sprintf(tmp, "/sys/class/gpio/gpio%d/value", gpio0);
	fd0 = open(tmp, O_RDWR);
	if(fd0 < 0) {
		IMP_LOG_DBG(TAG, "open /sys/class/gpio/gpio%d/value error !", gpio0);
		return -1;
	}

	sprintf(tmp, "/sys/class/gpio/gpio%d/value", gpio1);
	fd1 = open(tmp, O_RDWR);
	if(fd1 < 0) {
		IMP_LOG_DBG(TAG, "open /sys/class/gpio/gpio%d/value error !", gpio1);
		return -1;
	}

	sprintf(on, "%d", ircut);
	sprintf(off, "%d", !ircut);

	write(fd0, "0", 1);
	usleep(10*1000);

	write(fd0, on, strlen(on));
	write(fd1, off, strlen(off));

	if (!ircut) {
		usleep(10*1000);
		write(fd0, off, strlen(off));
	}

	close(fd0);
	close(fd1);

	return ret;
}

int xcam_conf_set_video_noisereductionmode(int noisereduction)
{
	int ret = XCAM_SUCCESS, flag = SET_CONF_VIDEO_NOISEREDUCTIONMODE;

	if (cli_attr.no_operation_file_flag == true)
		return ret;

	xcam_video_conf.conf_isp.noisereduction = noisereduction;
	ret = xcam_conf_set_video(flag,&xcam_video_conf);
	if (ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG,"error,call xcam_conf_set_video_ircut fail.\n");
	}

	return ret;
}

int xcam_conf_set_video_drcmode(int drc)
{
	int ret = XCAM_SUCCESS, flag = SET_CONF_VIDEO_DRCMODE;

	if (cli_attr.no_operation_file_flag == true)
		return ret;

	xcam_video_conf.conf_isp.drc = drc;
	ret = xcam_conf_set_video(flag,&xcam_video_conf);
	if (ret < XCAM_SUCCESS)
		LOG_ERR(LOG_TAG,"error,call xcam_conf_set_video_ircut fail.\n");

	return ret;
}

int xcam_conf_set_video_image_control(int saturation, int brightness, int sharpness, int contrast)
{
	int ret = XCAM_SUCCESS, flag = SET_CONF_VIDEO_IMAGE_CONTROL;

	if (cli_attr.no_operation_file_flag == true)
		return ret;

	xcam_video_conf.conf_isp.saturation = saturation;
	xcam_video_conf.conf_isp.brightness = brightness;
	xcam_video_conf.conf_isp.sharpness = sharpness;
	xcam_video_conf.conf_isp.contrast = contrast;
	ret = xcam_conf_set_video(flag,&xcam_video_conf);
	if (ret < XCAM_SUCCESS)
		LOG_ERR(LOG_TAG,"error,call xcam_conf_set_video_image_control fail.\n");

	return ret;
}

void _xcam_conf_str_init(conf_video_t *video_conf)
{
	memset(video_conf, 0, sizeof(conf_video_t));
	video_conf->conf_persondet.enable = 1;
	video_conf->conf_extraarg.movedetEnable = 0;
	video_conf->conf_extraarg.platerecEnable = 0;
	video_conf->conf_extraarg.facedetEnable = 0;
	video_conf->conf_extraarg.pervehpetEnable = 1;
	video_conf->conf_extraarg.fireworksdetEnable = 0;
	video_conf->conf_extraarg.aiispEnable = 0;
	video_conf->conf_isp.fps_num = 25;
	video_conf->conf_isp.fps_den = 1;
	video_conf->conf_isp.nightvision = 0;
	video_conf->conf_isp.wdr = 0;
	video_conf->conf_isp.imagemirror = 0;
	video_conf->conf_isp.imageflip = 0;
	video_conf->conf_isp.ircut = 0;
	video_conf->conf_isp.drc = 128;
	video_conf->conf_isp.noisereduction = 128;
	video_conf->conf_isp.saturation = 128;
	video_conf->conf_isp.brightness = 128;
	video_conf->conf_isp.sharpness = 128;
	video_conf->conf_isp.contrast = 128;
	video_conf->conf_ivdc.enable = 1;
	video_conf->conf_fs.ch0_w = SENSOR_RESOLUTION_WIDTH_MAIN;
	video_conf->conf_fs.ch0_h = SENSOR_RESOLUTION_HEIGHT_MAIN;
	video_conf->conf_fs.ch1_w = SENSOR_RESOLUTION_WIDTH_SLAVE;
	video_conf->conf_fs.ch1_h = SENSOR_RESOLUTION_HEIGHT_SLAVE;


	//stream 0
	video_conf->conf_enc.ch0_type = PT_H265;
	video_conf->conf_enc.ch0_enc_rc.rc_type = ENC_RC_MODE_VBR;
	video_conf->conf_enc.ch0_enc_rc.rc_vbr.target_bitrate = 1000;
	video_conf->conf_enc.ch0_enc_rc.rc_vbr.max_bitrate = 1000;
	//stream 1
	video_conf->conf_enc.ch1_type = PT_H265;
	video_conf->conf_enc.ch1_enc_rc.rc_type = ENC_RC_MODE_VBR;
	video_conf->conf_enc.ch1_enc_rc.rc_vbr.target_bitrate = 1000;
	video_conf->conf_enc.ch1_enc_rc.rc_vbr.max_bitrate = 1000;
}

void _xcam_conf_file_init(conf_video_t *video_conf)
{
	cJSON *root = NULL ,*item = NULL , *item_1 = NULL, *item_2 = NULL;
	int ret = XCAM_SUCCESS;
	root = json_read_file(CONFIG_VIDEO_FILE);
	if (root == NULL) {
		LOG_ERR(LOG_TAG,"error(%s,%d): json read file\n",__func__, __LINE__);
	}

	assert(cJSON_GetObjectItem(root, "persondet") == NULL);
	assert(cJSON_GetObjectItem(root, "ivdc") == NULL);
	assert(cJSON_GetObjectItem(root, "isp") == NULL);
	assert(cJSON_GetObjectItem(root, "fs") == NULL);
	assert(cJSON_GetObjectItem(root, "enc") == NULL);
	assert(cJSON_GetObjectItem(root, "extraArg") == NULL);

	item = cJSON_CreateObject();
	cJSON_AddItemToObject(item, "enable", cJSON_CreateNumber(video_conf->conf_persondet.enable));
	cJSON_AddItemToObject(root, "persondet", item);

	item = cJSON_CreateObject();
	cJSON_AddItemToObject(item, "movedetEnable", cJSON_CreateNumber(video_conf->conf_extraarg.movedetEnable));
	cJSON_AddItemToObject(item, "platerecEnable", cJSON_CreateNumber(video_conf->conf_extraarg.platerecEnable));
	cJSON_AddItemToObject(item, "facedetEnable", cJSON_CreateNumber(video_conf->conf_extraarg.facedetEnable));
	cJSON_AddItemToObject(item, "pervehpetEnable", cJSON_CreateNumber(video_conf->conf_extraarg.pervehpetEnable));
	cJSON_AddItemToObject(item, "fireworksdetEnable", cJSON_CreateNumber(video_conf->conf_extraarg.fireworksdetEnable));
	cJSON_AddItemToObject(item, "aiispEnable", cJSON_CreateNumber(video_conf->conf_extraarg.aiispEnable));
	cJSON_AddItemToObject(root, "extraarg", item);

	item = cJSON_CreateObject();
	cJSON_AddItemToObject(item, "enable", cJSON_CreateNumber(video_conf->conf_persondet.enable));
	cJSON_AddItemToObject(root, "ivdc", item);

	item = cJSON_CreateObject();
	cJSON_AddItemToObject(item, "fps_num", cJSON_CreateNumber(video_conf->conf_isp.fps_num));
	cJSON_AddItemToObject(item, "fps_den", cJSON_CreateNumber(video_conf->conf_isp.fps_den));
	cJSON_AddItemToObject(item, "nightvisionmode", cJSON_CreateNumber(video_conf->conf_isp.nightvision));
	cJSON_AddItemToObject(item, "wdrmode", cJSON_CreateNumber(video_conf->conf_isp.wdr));
	cJSON_AddItemToObject(item, "imagemirrormode", cJSON_CreateNumber(video_conf->conf_isp.imagemirror));
	cJSON_AddItemToObject(item, "imageflipmode", cJSON_CreateNumber(video_conf->conf_isp.imageflip));
	cJSON_AddItemToObject(item, "ircutmode", cJSON_CreateNumber(video_conf->conf_isp.ircut));
	cJSON_AddItemToObject(item, "drcmode", cJSON_CreateNumber(video_conf->conf_isp.drc));
	cJSON_AddItemToObject(item, "noisereductionmode", cJSON_CreateNumber(video_conf->conf_isp.noisereduction));
	cJSON_AddItemToObject(item, "saturation", cJSON_CreateNumber(video_conf->conf_isp.saturation));
	cJSON_AddItemToObject(item, "brightness", cJSON_CreateNumber(video_conf->conf_isp.brightness));
	cJSON_AddItemToObject(item, "sharpness", cJSON_CreateNumber(video_conf->conf_isp.sharpness));
	cJSON_AddItemToObject(item, "contrast", cJSON_CreateNumber(video_conf->conf_isp.contrast));
	cJSON_AddItemToObject(root, "isp", item);

	item = cJSON_CreateObject();
	cJSON_AddItemToObject(item, "ch0_w", cJSON_CreateNumber(video_conf->conf_fs.ch0_w));
	cJSON_AddItemToObject(item, "ch0_h", cJSON_CreateNumber(video_conf->conf_fs.ch0_h));
	cJSON_AddItemToObject(item, "ch1_w", cJSON_CreateNumber(video_conf->conf_fs.ch1_w));
	cJSON_AddItemToObject(item, "ch1_h", cJSON_CreateNumber(video_conf->conf_fs.ch1_h));
	cJSON_AddItemToObject(root, "fs", item);

	item = cJSON_CreateObject();
	cJSON_AddItemToObject(item, "ch0_type", cJSON_CreateString("h265"));
	cJSON_AddItemToObject(item, "ch1_type", cJSON_CreateString("h265"));
	item_1 = cJSON_CreateObject();
	cJSON_AddItemToObject(item_1, "rc_type", cJSON_CreateString("vbr"));
	cJSON_AddItemToObject(item_1, "max_bitrate", cJSON_CreateNumber(video_conf->conf_enc.ch0_enc_rc.rc_vbr.max_bitrate));
	cJSON_AddItemToObject(item_1, "target_bitrate", cJSON_CreateNumber(video_conf->conf_enc.ch0_enc_rc.rc_vbr.target_bitrate));
	cJSON_AddItemToObject(item, "ch0_enc_rc", item_1);

	item_2 = cJSON_CreateObject();
	cJSON_AddItemToObject(item_2, "rc_type", cJSON_CreateString("vbr"));
	cJSON_AddItemToObject(item_2, "max_bitrate", cJSON_CreateNumber(video_conf->conf_enc.ch0_enc_rc.rc_vbr.max_bitrate));
	cJSON_AddItemToObject(item_2, "target_bitrate", cJSON_CreateNumber(video_conf->conf_enc.ch1_enc_rc.rc_vbr.target_bitrate));
	cJSON_AddItemToObject(item, "ch1_enc_rc", item_2);
	cJSON_AddItemToObject(root, "enc", item);

	ret = json_write_file(CONFIG_VIDEO_FILE, root);
	if (ret < XCAM_SUCCESS)
		LOG_ERR(LOG_TAG,"errno,Call json_write_file fail.\n");
	cJSON_Delete(root);
}

int xcam_conf_get_ivdc_status(int *enable)
{
	*enable = xcam_video_conf.conf_ivdc.enable;
	return 0;
}

int xcam_conf_get_persondet_status(int *enable)
{
	*enable = xcam_video_conf.conf_persondet.enable;
	return 0;
}

int xcam_conf_get_extraarg_status(char *itemV,int length)
{
	int enable =0;
	if(length < ArrayLen(extraArgValues)){
		LOG_ERR(LOG_TAG,"err(%s,%d):item size error error.\n",__func__,__LINE__);
		return XCAM_ERROR;
	}
	for(int i=0; i<length; i++){
		if(!strncmp(itemV,extraArgValues[i],4)){
			enable = ((int*)(&xcam_video_conf.conf_extraarg))[i];
		}
		   
	}
	return enable;
}

int xcam_conf_get_extraargaiisp_status(int *enable)
{
	*enable = xcam_video_conf.conf_extraarg.aiispEnable;
	return 0;
}

void xcam_conf_video_init()
{
	int ret = XCAM_SUCCESS;
	FILE* file = NULL;
	cJSON* root = NULL;
	char* json_str = NULL;
	 xcam_video_conf.conf_ivdc.enable = 1;
	_xcam_conf_str_init(&xcam_video_conf);
	//如果命令行指定不操控配置文件，直接返回,用户态也按照默认配置走
	if (cli_attr.no_operation_file_flag == true)
		return;

	file = fopen(CONFIG_VIDEO_FILE,"r");
	if (file == NULL) {
		//配置文件不存在情况
		root = cJSON_CreateObject();
		json_str = cJSON_Print(root);
		file = fopen(CONFIG_VIDEO_FILE,"w+");
		if (file == NULL) {
			cJSON_Delete(root);
			free(json_str);
			LOG_ERR(LOG_TAG,"Init Video configuration file fail.\n");
			return;
		}
		fprintf(file,"%s",json_str);
		free(json_str);
		cJSON_Delete(root);
		root = NULL;
		fclose(file);
		_xcam_conf_file_init(&xcam_video_conf);
	} else {
		fclose(file);
		ret = xcam_conf_get_video(&xcam_video_conf);
		if (ret < XCAM_SUCCESS)
			LOG_ERR(LOG_TAG,"error,call xcam_conf_get_video fail.\n");
	}

	return;
}

