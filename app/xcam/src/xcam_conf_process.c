#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <stdbool.h>
#include <signal.h>
#include "xcam_video.h"
#include "xcam_log.h"
#include "xcam_general.h"
#include "xcam_thread.h"
#include "xcam_network.h"
#include "../../daemon/xcam_daemon_protocol.h"
#include "cJSON.h"
#include "../../onvif/src/func_network.h"
#include "xcam_web_process.h"
#include "xcam_ptz.h"
#include "xcam_message_queue.h"
#include "xcam_conf_process.h"
#include "xcam_conf_video.h"
#include "xcam_com.h"
#define CONFIG_VIDEO_FILE "/system/etc/video.json"

extern video_config_info_t stream_attr;
extern char gInterface[20];
extern char gIpAddr[20];
int xcam_osd_set_chnTitles(int , char* , int , int , int , int , int );
int xcam_video_set_isp_qp(uint32_t , uint32_t );
int xcam_network_set_device_ip_mask(char *, char *);
int xcam_Tuning_GetAwbAttr(int t, int *);
int xcam_Tuning_SetAwbAttr(int , int*);
int xcam_Tuning_GetAWBGlobalStatisInfo(int  , int *);
unsigned int xcam_Tuing_GetAwbOnlyReadAttr(int , int *);
#if ((defined T20) || (defined T21))
extern IMPEncoderGOPSizeCfg gopsize_config;
#endif


#define LOG_TAG "cJson_process"

int xcam_json_get_video_isp_fps(void *json_root)
{
	if (json_root == NULL) {
		LOG_ERR(LOG_TAG,"err(%s,%d): json root is NULL\n", __func__, __LINE__);
		return XCAM_ERROR;
	}
	uint32_t fps_num,fps_den=0;
	xcam_video_get_isp_fps(&fps_num,&fps_den);

	cJSON *root = NULL, *_cfg = NULL,*config = NULL,*config_member = NULL, *tag = NULL;
	root = (cJSON *)json_root;
	config = cJSON_GetObjectItem(root, "video.isp.fps");

	cJSON *cfglist = cJSON_GetObjectItem(root, "configlist");
	if (!cfglist) {
		LOG_ERR(LOG_TAG,"err(%s,%d): json has no configlist\n", __func__, __LINE__);
		return XCAM_ERROR;
	}

	_cfg = cfglist->child;
	if (!_cfg)
		cJSON_AddItemToObject(cfglist, "video.isp.fps",cJSON_CreateString("video.isp.fps"));
	else {
		while (_cfg) {
			if (strcmp(_cfg->string, "video.isp.fps") == 0)
				break;
			if ((_cfg = _cfg->next) == NULL)
				cJSON_AddItemToObject(cfglist, "video.isp.fps",cJSON_CreateString("video.isp.fps"));
		}
	}

	if (config == NULL || config->child == NULL) {
		config = cJSON_CreateObject();
		cJSON_AddItemToObject(root, "video.isp.fps", config);
		cJSON_AddItemToObject(config, "fpsnum", cJSON_CreateNumber(fps_num));
		cJSON_AddItemToObject(config, "fpsden", cJSON_CreateNumber(fps_den));
	} else {
		config_member = config->child;
		while (config_member) {
			if (strcmp(config_member->string,"fpsnum") == 0) {
				tag = config_member->prev;
				cJSON_ReplaceItemInObject(config_member,"fpsnum",cJSON_CreateNumber(fps_num));
				config_member = tag->next;
			} else if(strcmp(config_member->string,"fpsden") == 0) {
				tag = config_member->prev;
				cJSON_ReplaceItemInObject(config_member,"fpsden",cJSON_CreateNumber(fps_den));
				config_member = tag->next;
			}
			config_member = config_member->next;
		}
	}

	return XCAM_SUCCESS;
}

int xcam_json_set_video_isp_fps(void *json_root)
{
#if 1
	if (json_root == NULL) {
		LOG_ERR(LOG_TAG,"err(%s,%d): json root is NULL\n", __func__, __LINE__);
		return -1;
	}

	int fpsnum = 0, fpsden = 0, ret = XCAM_SUCCESS;
	cJSON *root = NULL,  *config = NULL, *config_member = NULL;
	root = (cJSON *)json_root;

	config = cJSON_GetObjectItem(root, "video.isp.fps");
	if (config == NULL) {
		LOG_ERR(LOG_TAG,"err(%s,%d): json file has no this config\n", __func__, __LINE__);
		return XCAM_ERROR;
	}

	config_member = config->child;
	while (config_member) {
		if (strcmp(config_member->string, "fpsnum") == 0) {
			fpsnum = config_member->valueint;
		} else if(strcmp(config_member->string, "fpsden") == 0) {
			fpsden = config_member->valueint;
		}
		config_member = config_member->next;
	}
	//设置帧率;
	ret = xcam_video_set_isp_fps(fpsnum,fpsden);
	if (ret < 0) {
		LOG_ERR(LOG_TAG,"Call xcam_video_set_fps fail.\n");
		return ret;
	}
#endif
	return XCAM_SUCCESS;
}

int xcam_json_get_video_fs_fps(void *json_root)
{
	if (json_root == NULL) {
		LOG_ERR(LOG_TAG,"err(%s,%d): json root is NULL\n", __func__, __LINE__);
		return XCAM_ERROR;
	}

	cJSON *root = NULL, *_cfg = NULL,*config = NULL,*config_member = NULL, *config_child = NULL, *part = NULL, *tag = NULL;
	root = (cJSON *)json_root;
	config = cJSON_GetObjectItem(root, "video.fs.fps");

	cJSON *cfglist = cJSON_GetObjectItem(root, "configlist");
	if (!cfglist) {
		LOG_ERR(LOG_TAG,"err(%s,%d): json has no configlist\n", __func__, __LINE__);
		return XCAM_ERROR;
	}

	_cfg = cfglist->child;
	if (!_cfg)
		cJSON_AddItemToObject(cfglist, "video.fs.fps",cJSON_CreateString("video.fs.fps"));
	else {
		while (_cfg) {
			//暂时先这样，调通了再改，这个地方应该是_cfg->strings
			if (strcmp(_cfg->string, "video.fs.fps") == 0)
				break;
			if ((_cfg = _cfg->next) == NULL)
				cJSON_AddItemToObject(cfglist, "video.fs.fps",cJSON_CreateString("video.fs.fps"));
		}
	}

	if (config == NULL || config->child == NULL) {
		config = cJSON_CreateObject();
		cJSON_AddItemToObject(root, "video.fs.fps", config);

		config_child = cJSON_CreateObject();
		cJSON_AddItemToObject(config, "ch0", config_child);
		cJSON_AddItemToObject(config_child, "fpsnum", cJSON_CreateNumber(stream_attr.stream_config[0].fs_attr.outFrmRateNum));
		cJSON_AddItemToObject(config_child, "fpsden", cJSON_CreateNumber(stream_attr.stream_config[0].fs_attr.outFrmRateDen));

		config_child = cJSON_CreateObject();
		cJSON_AddItemToObject(config, "ch1", config_child);
		cJSON_AddItemToObject(config_child, "fpsnum", cJSON_CreateNumber(stream_attr.stream_config[1].fs_attr.outFrmRateNum));
		cJSON_AddItemToObject(config_child, "fpsden", cJSON_CreateNumber(stream_attr.stream_config[1].fs_attr.outFrmRateDen));
	} else {
		config_member = config->child;
		while (config_member) {
			if (strcmp(config_member->string, "ch0") == 0) {
				part = config_member->child;
				while(part){
				    if (strcmp(part->string,"fpsnum") == 0) {
						tag = part->prev;
						cJSON_ReplaceItemInObject(config_member,"fpsnum",cJSON_CreateNumber(stream_attr.stream_config[0].fs_attr.outFrmRateNum));
						part = tag->next;
					} else if(strcmp(part->string,"fpsden") == 0) {
						tag = part->prev;
						cJSON_ReplaceItemInObject(config_member,"fpsden",cJSON_CreateNumber(stream_attr.stream_config[0].fs_attr.outFrmRateDen));
						part = tag->next;
					}
					part = part->next;
				}
			} else if(strcmp(config_member->string, "ch1") == 0) {
				part = config_member->child;
				while(part){
					if (strcmp(part->string,"fpsnum") == 0) {
						tag = part->prev;
						cJSON_ReplaceItemInObject(config_member,"fpsnum",cJSON_CreateNumber(stream_attr.stream_config[1].fs_attr.outFrmRateNum));
						part = tag->next;
					} else if(strcmp(part->string,"fpsden") == 0) {
						tag = part->prev;
						cJSON_ReplaceItemInObject(config_member,"fpsden",cJSON_CreateNumber(stream_attr.stream_config[1].fs_attr.outFrmRateDen));
						part = tag->next;
					}
					part = part->next;
				}
			}
			config_member = config_member->next;
		}
	}

	return XCAM_SUCCESS;
}

int xcam_json_set_video_fs_fps(void *json_root)
{
	if (json_root == NULL) {
		LOG_ERR(LOG_TAG,"err(%s,%d): json root is NULL\n", __func__, __LINE__);
		return -1;
	}

	int ch_num = -1,fpsnum = 0, fpsden = 0, ret = XCAM_SUCCESS;
	cJSON *root = NULL, *config = NULL, *config_member = NULL, *part = NULL;
	root = (cJSON *)json_root;

	config = cJSON_GetObjectItem(root, "video.fs.fps");
	if (config == NULL) {
		LOG_ERR(LOG_TAG,"err(%s,%d): json file has no this config\n", __func__, __LINE__);
		return XCAM_ERROR;
	}

	config_member = config->child;
	while (config_member) {
		if (strcmp(config_member->string, "ch0") == 0) {
			ch_num = STREAM0;
			part = config_member->child;
			while(part){
				if (strcmp(part->string, "fpsnum") == 0) {
					fpsnum = part->valueint;
				} else if(strcmp(part->string, "fpsden") == 0) {
					fpsden = part->valueint;
				}
				part = part->next;
			}
		} else if(strcmp(config_member->string, "ch1")==0) {
			ch_num = STREAM1;
			part = config_member->child;
			while(part) {
				if (strcmp(part->string, "fpsnum") == 0) {
					fpsnum = part->valueint;
				} else if(strcmp(part->string, "fpsden") == 0) {
					fpsden = part->valueint;
				}
				part = part->next;
			}
		} else {
			LOG_ERR(LOG_TAG,"xcam current don't this channel.\n");
		}

		//设置帧率;
		ret = xcam_video_set_fs_fps(ch_num,fpsnum,fpsden);
		if (ret < 0) {
			LOG_ERR(LOG_TAG,"Call xcam_video_set_fps fail.\n");
			return ret;
		}
		config_member = config_member->next;
	}

	return XCAM_SUCCESS;
}

int xcam_json_get_video_enc_fps(void *json_root)
{
	if (json_root == NULL) {
		LOG_ERR(LOG_TAG,"err(%s,%d): json root is NULL\n", __func__, __LINE__);
		return XCAM_ERROR;
	}

	cJSON *root = NULL, *_cfg = NULL,*config = NULL,*config_member = NULL, *config_child = NULL, *part = NULL, *tag = NULL;
	root = (cJSON *)json_root;
	config = cJSON_GetObjectItem(root, "video.enc.fps");

	cJSON *cfglist = cJSON_GetObjectItem(root, "configlist");
	if (!cfglist) {
		LOG_ERR(LOG_TAG,"err(%s,%d): json has no configlist\n", __func__, __LINE__);
		return XCAM_ERROR;
	}

	_cfg = cfglist->child;
	if (!_cfg)
		cJSON_AddItemToObject(cfglist, "video.enc.fps",cJSON_CreateString("video.enc.fps"));
	else {
		while (_cfg) {
			//暂时先这样，调通了再改，这个地方应该是_cfg->strings
			if (strcmp(_cfg->string, "video.enc.fps") == 0)
				break;
			if ((_cfg = _cfg->next) == NULL)
				cJSON_AddItemToObject(cfglist, "video.enc.fps",cJSON_CreateString("video.enc.fps"));
		}
	}

	if (config == NULL || config->child == NULL) {
		config = cJSON_CreateObject();
		cJSON_AddItemToObject(root, "video.enc.fps", config);

		config_child = cJSON_CreateObject();
		cJSON_AddItemToObject(config, "ch0", config_child);
		cJSON_AddItemToObject(config_child, "fpsnum", cJSON_CreateNumber(stream_attr.stream_config[0].enc_attr.rcAttr.outFrmRate.frmRateNum));
		cJSON_AddItemToObject(config_child, "fpsden", cJSON_CreateNumber(stream_attr.stream_config[1].enc_attr.rcAttr.outFrmRate.frmRateDen));

		config_child = cJSON_CreateObject();
		cJSON_AddItemToObject(config, "ch1", config_child);
		cJSON_AddItemToObject(config_child, "fpsnum", cJSON_CreateNumber(stream_attr.stream_config[1].enc_attr.rcAttr.outFrmRate.frmRateNum));
		cJSON_AddItemToObject(config_child, "fpsden", cJSON_CreateNumber(stream_attr.stream_config[1].enc_attr.rcAttr.outFrmRate.frmRateDen));
	} else {
		config_member = config->child;
		while (config_member) {
			if (strcmp(config_member->string, "ch0") == 0) {
				part = config_member->child;
				while(part) {
					if (strcmp(part->string,"fpsnum") == 0) {
						tag = part->prev;
						cJSON_ReplaceItemInObject(config_member,"fpsnum",cJSON_CreateNumber(stream_attr.stream_config[0].enc_attr.rcAttr.outFrmRate.frmRateNum));
						part = tag->next;
					} else if(strcmp(part->string,"fpsden") == 0) {
						tag = part->prev;
						cJSON_ReplaceItemInObject(config_member,"fpsden",cJSON_CreateNumber(stream_attr.stream_config[0].enc_attr.rcAttr.outFrmRate.frmRateDen));
						part = tag->next;
					}
					part = part->next;
				}
			} else if(strcmp(config_member->string, "ch1") == 0) {
				part = config_member->child;
				while(part) {
					if (strcmp(part->string,"fpsnum") == 0) {
						tag = part->prev;
						cJSON_ReplaceItemInObject(config_member,"fpsnum",cJSON_CreateNumber(stream_attr.stream_config[1].enc_attr.rcAttr.outFrmRate.frmRateNum));
						part = tag->next;
					} else if(strcmp(part->string,"fpsden") == 0) {
						tag = part->prev;
						cJSON_ReplaceItemInObject(config_member,"fpsden",cJSON_CreateNumber(stream_attr.stream_config[1].enc_attr.rcAttr.outFrmRate.frmRateDen));
						part = tag->next;
					}
					part = part->next;
				}
			}
			config_member = config_member->next;
		}
	}

	return XCAM_SUCCESS;
}

int xcam_json_set_video_enc_fps(void *json_root)
{
	if (json_root == NULL) {
		LOG_ERR(LOG_TAG,"err(%s,%d): json root is NULL\n", __func__, __LINE__);
		return -1;
	}

	int ch_num = -1,fpsnum = 0, fpsden = 0, ret = XCAM_SUCCESS;
	cJSON *root = NULL, *config = NULL, *config_member = NULL, *part = NULL;
	root = (cJSON *)json_root;

	config = cJSON_GetObjectItem(root, "video.enc.fps");
	if (config == NULL) {
		LOG_ERR(LOG_TAG,"err(%s,%d): json file has no this config\n", __func__, __LINE__);
		return XCAM_ERROR;
	}

	config_member = config->child;
	while (config_member) {
		if (strcmp(config_member->string, "ch0") == 0) {
			ch_num = STREAM0;
			part = config_member->child;
			while(part){
				if (strcmp(part->string, "fpsnum") == 0) {
					fpsnum = part->valueint;
				} else if(strcmp(part->string, "fpsden") == 0) {
					fpsden = part->valueint;
				}
				part = part->next;
			}
		} else if(strcmp(config_member->string, "ch1")==0) {
			ch_num = STREAM1;
			part = config_member->child;
			while(part){
				if (strcmp(part->string, "fpsnum") == 0) {
					fpsnum = part->valueint;
				} else if(strcmp(part->string, "fpsden") == 0) {
					fpsden = part->valueint;
				}
				part = part->next;
			}
		} else {
			LOG_ERR(LOG_TAG,"xcam current don't this channel.\n");
		}

		//设置帧率;
		ret = xcam_video_set_enc_fps(ch_num,fpsnum,fpsden);
		if (ret < 0) {
			LOG_ERR(LOG_TAG,"Call xcam_video_set_fps fail.\n");
			return ret;
		}

		config_member = config_member->next;
	}

	return XCAM_SUCCESS;
}

int xcam_json_get_video_bps(void *json_root)
{
	if (json_root == NULL) {
		LOG_ERR(LOG_TAG,"err(%s,%d): json root is NULL\n", __func__, __LINE__);
		return XCAM_ERROR;
	}

	int ret = XCAM_SUCCESS, bps0 = 0, bps1 = 0;
	cJSON *root = NULL, *_cfg = NULL, *config = NULL, *config_member = NULL, *config_child = NULL, *cfglist = NULL;
	root = (cJSON *)json_root;
	config = cJSON_GetObjectItem(root, "video.bitrate");

	cfglist = cJSON_GetObjectItem(root, "configlist");
	if (!cfglist) {
		LOG_ERR(LOG_TAG,"err(%s,%d): json has no configlist\n", __func__, __LINE__);
		return XCAM_ERROR;
	}

	_cfg = cfglist->child;
	if (!_cfg)
		cJSON_AddItemToObject(cfglist, "video.bitrate",cJSON_CreateString("video.bitrate"));
	else {
		while (_cfg) {
			if (strcmp(_cfg->string, "video.bitrate") == 0)
				break;
			if ((_cfg = _cfg->next) == NULL)
				cJSON_AddItemToObject(cfglist, "video.bitrate",cJSON_CreateString("video.bitrate"));
		}
	}

	ret |= xcam_video_get_bitrate(ENCCHN0,&bps0);
	ret |= xcam_video_get_bitrate(ENCCHN1,&bps1);
	if (ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG,"get xcam video bitrate fail.\n");
		return ret;
	}

	if (config == NULL || config->child == NULL) {
		config = cJSON_CreateObject();
		cJSON_AddItemToObject(root, "video.bitrate", config);

		config_child = cJSON_CreateObject();
		cJSON_AddItemToObject(config, "ch0", config_child);
		cJSON_AddItemToObject(config_child, "bitrate", cJSON_CreateNumber(bps0));

		config_child = cJSON_CreateObject();
		cJSON_AddItemToObject(config, "ch1", config_child);
		cJSON_AddItemToObject(config_child, "bitrate", cJSON_CreateNumber(bps1));
	} else {
		config_member = config->child;
		while (config_member) {
			if (strcmp(config_member->string, "ch0") == 0) {
				if(strcmp(config_member->child->string,"bitrate") == 0){
					cJSON_ReplaceItemInObject(config_member,"bitrate",cJSON_CreateNumber(bps0));
				}else{
					cJSON_AddItemToObject(config_member, "bitrate", cJSON_CreateNumber(bps0));
				}
			} else if(strcmp(config_member->string, "ch1") == 0) {
				if(strcmp(config_member->child->string,"bitrate") == 0){
					cJSON_ReplaceItemInObject(config_member,"bitrate",cJSON_CreateNumber(bps1));
				}else{
					cJSON_AddItemToObject(config_member, "bitrate", cJSON_CreateNumber(bps1));
				}
			}
			config_member = config_member->next;
		}
	}

	return XCAM_SUCCESS;
}

int xcam_json_set_video_bps(void *json_root)
{
	if (json_root == NULL) {
		LOG_ERR(LOG_TAG,"err(%s,%d): json root is NULL\n", __func__, __LINE__);
		return XCAM_ERROR;
	}

	int ch_num = -1, bps = -1,ret = XCAM_SUCCESS;
	cJSON *root = NULL,  *config = NULL, *config_member = NULL;
	root = (cJSON *)json_root;

	config = cJSON_GetObjectItem(root, "video.bitrate");
	if (config == NULL) {
		LOG_ERR(LOG_TAG,"err(%s,%d): json file has no this config\n", __func__, __LINE__);
		return XCAM_ERROR;
	}

	config_member = config->child;
	while (config_member) {
		if (strcmp(config_member->string, "ch0") == 0) {
			ch_num = STREAM0;
			bps = config_member->child->valueint;
		} else if(strcmp(config_member->string, "ch1")==0) {
			ch_num = STREAM1;
			bps = config_member->child->valueint;
		}else{
			LOG_INF(LOG_TAG,"Info(%s,%d):xcam current don't this channel.\n",__func__,__LINE__);
		}

		if (ch_num == -1 || bps < 100 || bps > 10000) {
			LOG_ERR(LOG_TAG,"err(%s,%d): ch_num or bps is not right\n", __func__, __LINE__);
			return XCAM_ERROR;
		}

		//set bitrate;
		ret = xcam_video_set_bitrate(ch_num, bps);
		if (ret < XCAM_SUCCESS) {
			LOG_ERR(LOG_TAG,"error(%s,%d):call xcam_video_set_fps fail.\n",__func__,__LINE__);
			return ret;
		}

		config_member = config_member->next;
	}

	return XCAM_SUCCESS;
}

int xcam_json_get_video_resolution(void *json_root)
{
	if (json_root == NULL) {
		LOG_ERR(LOG_TAG,"Error(%s,%d): json root is NULL\n", __func__, __LINE__);
		return XCAM_ERROR;
	}

	cJSON *root = NULL, *_cfg = NULL, *config = NULL, *config_member = NULL, *config_child = NULL, *part = NULL, *tag = NULL;
	root = (cJSON *)json_root;
	config = cJSON_GetObjectItem(root, "video.resolution");

	cJSON *cfglist = cJSON_GetObjectItem(root, "configlist");
	if (!cfglist) {
		LOG_ERR(LOG_TAG,"Error(%s,%d):json has no configlist\n", __func__, __LINE__);
		return XCAM_ERROR;
	}

	_cfg = cfglist->child;
	if (!_cfg)
		cJSON_AddItemToObject(cfglist, "video.resolution",cJSON_CreateString("video.resolution"));
	else {
		while (_cfg) {
			if (strcmp(_cfg->string, "video.resolution") == 0)
				break;
			if ((_cfg = _cfg->next) == NULL)
				cJSON_AddItemToObject(cfglist, "video.resolution",cJSON_CreateString("video.resolution"));
		}
	}

	if (config == NULL || config->child == NULL) {
		config = cJSON_CreateObject();
		cJSON_AddItemToObject(root, "video.resolution", config);

		config_child = cJSON_CreateObject();
		cJSON_AddItemToObject(config, "ch0", config_child);
		cJSON_AddItemToObject(config_child, "picheight", cJSON_CreateNumber(stream_attr.stream_config[0].fs_attr.picHeight));
		cJSON_AddItemToObject(config_child, "picwidth", cJSON_CreateNumber(stream_attr.stream_config[0].fs_attr.picWidth));
		config_child = cJSON_CreateObject();
		cJSON_AddItemToObject(config, "ch1", config_child);
		cJSON_AddItemToObject(config_child, "picheight", cJSON_CreateNumber(stream_attr.stream_config[1].fs_attr.picHeight));
		cJSON_AddItemToObject(config_child, "picwidth", cJSON_CreateNumber(stream_attr.stream_config[1].fs_attr.picWidth));
	} else {
		config_member = config->child;
		while (config_member) {
			if (strcmp(config_member->string, "ch0") == 0) {
				part = config_member->child;
				while (part) {
					if (strcmp(part->string,"picheight") == 0) {
						tag = part->prev;
						cJSON_ReplaceItemInObject(config_member,"picheight",cJSON_CreateNumber(stream_attr.stream_config[0].fs_attr.picHeight));
						part = tag->next;
					} else if(strcmp(part->string,"picwidth") == 0) {
						tag = part->prev;
						cJSON_ReplaceItemInObject(config_member,"picwidth",cJSON_CreateNumber(stream_attr.stream_config[0].fs_attr.picWidth));
						part = tag->next;
					}
					part = part->next;
				}

				if (part == NULL) {
					cJSON_AddItemToObject(config_member,"picheight",cJSON_CreateNumber(stream_attr.stream_config[0].fs_attr.picHeight));
					cJSON_AddItemToObject(config_member,"picwidth",cJSON_CreateNumber(stream_attr.stream_config[0].fs_attr.picWidth));
				}
			} else if(strcmp(config_member->string, "ch1") == 0) {
				part = config_member->child;
				while (part) {
					if (strcmp(part->string,"picheight") == 0) {
						tag = part->prev;
						cJSON_ReplaceItemInObject(config_member,"picheight",cJSON_CreateNumber(stream_attr.stream_config[1].fs_attr.picHeight));
						part = tag->next;
					} else if(strcmp(part->string,"picwidth") == 0) {
						tag = part->prev;
						cJSON_ReplaceItemInObject(config_member,"picwidth",cJSON_CreateNumber(stream_attr.stream_config[1].fs_attr.picWidth));
						part = tag->next;
					}
					part = part->next;
				}

				if (part == NULL) {
					cJSON_AddItemToObject(config_member,"picheight",cJSON_CreateNumber(stream_attr.stream_config[1].fs_attr.picHeight));
					cJSON_AddItemToObject(config_member,"picwidth",cJSON_CreateNumber(stream_attr.stream_config[1].fs_attr.picWidth));
				}
			}
			config_member = config_member->next;
		}
	}

	return XCAM_SUCCESS;
}

int xcam_json_set_video_resolution(void *json_root)
{

	if (json_root == NULL) {
		LOG_ERR(LOG_TAG,"Error(%s,%d):json root is NULL.\n", __func__, __LINE__);
		return XCAM_ERROR;
	}

	int ch_num = -1, picheight = 0, picwidth = 0, ret = 0;
	cJSON *root = NULL, *config = NULL, *config_member = NULL, *part = NULL;
	root = (cJSON *)json_root;

	config = cJSON_GetObjectItem(root, "video.resolution");
	if (config == NULL) {
		LOG_ERR(LOG_TAG,"Error(%s,%d): json file has no this config\n", __func__, __LINE__);
		return XCAM_ERROR;
	}

	config_member = config->child;
	while (config_member) {
		if (strcmp(config_member->string, "ch0") == 0) {
			ch_num = STREAM0;
			part = config_member->child;
			while(part)
			{
				if (strcmp(part->string,"picheight") == 0) {
					picheight = part->valueint;
				} else if(strcmp(part->string,"picwidth") == 0) {
					picwidth = part->valueint;
				}
				part = part->next;
			}
		} else if (strcmp(config_member->string, "ch1")==0) {
			ch_num = STREAM1;
			part = config_member->child;
			while(part)
			{
				if (strcmp(part->string,"picheight") == 0) {
					picheight = part->valueint;
				} else if (strcmp(part->string,"picwidth") == 0) {
					picwidth = part->valueint;
				}
				part = part->next;
			}
		} else {
			LOG_ERR(LOG_TAG,"xcam current don't this channel.\n");
		}
		ret = xcam_video_set_resolution(ch_num,picwidth,picheight);
		if (ret < XCAM_SUCCESS) {
			LOG_ERR(LOG_TAG,"Call xcam_video_set_resolution fail.\n");
			return ret;
		}

		config_member = config_member->next;
	}

	return XCAM_SUCCESS;
}

int xcam_json_get_video_encode_mode(void *json_root)
{
	if (json_root == NULL) {
		LOG_ERR(LOG_TAG,"Error(%s,%d):json root is NULL\n", __func__, __LINE__);
		return XCAM_ERROR;
	}

	int ret = XCAM_SUCCESS,enc0_mode = -1,enc1_mode = -1;
	char *enc0_mode_string = NULL, *enc1_mode_string =NULL;
	cJSON *root = (cJSON *)json_root;
	cJSON *_cfg = NULL;
	cJSON *config = cJSON_GetObjectItem(root, "video.enctype");
	cJSON *config_member = NULL;
	cJSON *config_child = NULL;

	cJSON *cfglist = cJSON_GetObjectItem(root, "configlist");
	if (!cfglist) {
		LOG_ERR(LOG_TAG,"Error(%s,%d):json has no configlist.\n", __func__, __LINE__);
		return -1;
	}

	ret |= xcam_video_get_encode_mode(0,&enc0_mode);
	ret |= xcam_video_get_encode_mode(1,&enc1_mode);
	if (ret != 0) {
		LOG_ERR(LOG_TAG,"Error(%s,%d):Call xcam_video_set_encode_mode.\n",__func__,__LINE__);
		return XCAM_ERROR;
	}

	if (enc0_mode == PT_H264) {
		enc0_mode_string = "h264";
	} else if(enc0_mode == PT_H265) {
		enc0_mode_string = "h265";
	} else {
		enc0_mode_string = "";
	}

	if (enc1_mode == PT_H264) {
		enc1_mode_string = "h264";
	} else if(enc1_mode == PT_H265) {
		enc1_mode_string = "h265";
	} else {
		enc1_mode_string = "";
	}

	_cfg = cfglist->child;
	if (!_cfg)
		cJSON_AddItemToObject(cfglist, "video.enctype",cJSON_CreateString("video.enctype"));
	else {
		while (_cfg) {
			//暂时先这样，调通了再改，这个地方应该是_cfg->strings
			if (strcmp(_cfg->string, "video.enctype") == 0)
				break;
			if ((_cfg = _cfg->next) == NULL)
				cJSON_AddItemToObject(cfglist, "video.enctype",cJSON_CreateString("video.enctype"));
		}
	}

	if (config == NULL || config->child == NULL) {
		config = cJSON_CreateObject();
		cJSON_AddItemToObject(root, "video.enctype", config);

		config_child = cJSON_CreateObject();
		cJSON_AddItemToObject(config, "ch0", config_child);
		cJSON_AddItemToObject(config_child, "enctype", cJSON_CreateString(enc0_mode_string));

		config_child = cJSON_CreateObject();
		cJSON_AddItemToObject(config, "ch1", config_child);
		cJSON_AddItemToObject(config_child, "enctype", cJSON_CreateString(enc1_mode_string));
	} else {
		config_member = config->child;
		while (config_member) {
			if (strcmp(config_member->string, "ch0") == 0) {
				if(strcmp(config_member->child->string,"enctype") == 0) {
					cJSON_ReplaceItemInObject(config_member,"enctype",cJSON_CreateString(enc0_mode_string));
				} else {
					cJSON_AddItemToObject(config_member, "enctype", cJSON_CreateString(enc0_mode_string));
				}
			} else if(strcmp(config_member->string, "ch1") == 0) {
				if(strcmp(config_member->child->string,"enctype") == 0){
					cJSON_ReplaceItemInObject(config_member,"enctype",cJSON_CreateString(enc1_mode_string));
				} else {
					cJSON_AddItemToObject(config_member, "enctype", cJSON_CreateString(enc1_mode_string));
				}
			}
			config_member = config_member->next;
		}
	}

	return XCAM_SUCCESS;
}

int xcam_json_set_video_encode_mode(void *json_root)
{
	if (json_root == NULL) {
		LOG_ERR(LOG_TAG,"err(%s,%d): json root is NULL\n", __func__, __LINE__);
		return XCAM_ERROR;
	}

	int ch_num = -1,ret = XCAM_SUCCESS, enc_mode = -1;
	cJSON *root = (cJSON *)json_root;
	cJSON *config = NULL;
	cJSON *config_member = NULL;

	config = cJSON_GetObjectItem(root, "video.enctype");
	if (config == NULL) {
		LOG_ERR(LOG_TAG,"err(%s,%d): json file has no this config\n", __func__, __LINE__);
		return XCAM_ERROR;
	}
	config_member = config->child;
	while (config_member) {
		if (strcmp(config_member->string, "ch0") == 0) {
			ch_num = STREAM0;
			if(strcmp(config_member->child->valuestring, "h264") == 0) {
				enc_mode = PT_H264;
			} else if(strcmp(config_member->child->valuestring,"h265") == 0) {
				enc_mode = PT_H265;
			} else {
				return XCAM_ERROR;
			}
		} else if(strcmp(config_member->string, "ch1")==0) {
			ch_num = STREAM1;
			if(strcmp(config_member->child->valuestring, "h264") == 0) {
				enc_mode = PT_H264;
			} else if(strcmp(config_member->child->valuestring,"h265") == 0) {
				enc_mode = PT_H265;
			} else {
				return XCAM_ERROR;
			}
		} else {
			LOG_ERR(LOG_TAG,"err(%s,%d),xcam current don't this channel.\n",__func__,__LINE__);
		}

		//设置编码方式;
		ret = xcam_video_set_encode_mode(ch_num,enc_mode);
		if (ret < 0) {
			LOG_ERR(LOG_TAG,"err(%s,%d),Call xcam_video_set_encode_mode fail.\n",__func__,__LINE__);
			return ret;
		}

		config_member = config_member->next;
	}

	return XCAM_SUCCESS;
}

int xcam_json_get_video_image_control(void *json_root)
{
	if(json_root == NULL) {
		LOG_ERR(LOG_TAG,"err(%s,%d): json root is NULL.\n",__func__,__LINE__);
		return XCAM_ERROR;
	}

	cJSON *root = NULL, *_cfg = NULL, *config = NULL, *configlist = NULL;
	root = (cJSON *)json_root;
	int ret = XCAM_ERROR;

	configlist = cJSON_GetObjectItem(root, "configlist");
	if (!configlist) {
		LOG_ERR(LOG_TAG,"err(%s,%d): json has no configlist\n", __func__, __LINE__);
		return XCAM_ERROR;
	}

	_cfg = configlist->child;
	if (_cfg ) {
		while ( _cfg ) {
			if (strcmp(_cfg->string,"video.image_control") == 0 )
				break;
			if ((_cfg = _cfg->next) == NULL) {
				LOG_ERR(LOG_TAG,"err(%s,%d): configlist has no member video.image_control\n", __func__, __LINE__);
				return XCAM_ERROR;
			}
		}
	} else {
		LOG_ERR(LOG_TAG,"err(%s,%d): configlist child is null,web transfor data format error\n", __func__, __LINE__);
		return XCAM_ERROR;
	}

	config = cJSON_GetObjectItem(root,"video.image_control");
	if ( config == NULL ) {
		config = cJSON_CreateObject();


		uint8_t saturation = 128, contrast = 128,  brightness = 128, sharpness = 128;
		// cJSON *config_file_root = NULL, *json_value, *json_value_child;

		IMP_ISP_Tuning_GetBrightness(0,&brightness);
		IMP_ISP_Tuning_GetContrast(0,&contrast);
		IMP_ISP_Tuning_GetSharpness(0,&sharpness);
		IMP_ISP_Tuning_GetSaturation(0,&saturation);
		cJSON_AddItemToObject(config,"saturation",cJSON_CreateNumber(saturation));
		cJSON_AddItemToObject(config,"brightness",cJSON_CreateNumber(brightness));
		cJSON_AddItemToObject(config,"sharpness",cJSON_CreateNumber(sharpness));
		cJSON_AddItemToObject(config,"contrast",cJSON_CreateNumber(contrast));
		cJSON_AddItemToObject(config,"awb",cJSON_CreateNumber(g_extra_web_param.awb));
		cJSON_AddItemToObject(config,"gamma",cJSON_CreateNumber(g_extra_web_param.gamma));
		cJSON_AddItemToObject(config,"filllight",cJSON_CreateNumber(g_extra_web_param.filllight));
		cJSON_AddItemToObject(config,"visualangle",cJSON_CreateNumber(g_extra_web_param.visualangle));
		cJSON_AddItemToObject(config,"backlight",cJSON_CreateNumber(g_extra_web_param.backlight));
		cJSON_AddItemToObject(config,"nightmode",cJSON_CreateNumber(g_extra_web_param.nightmode));
		cJSON_AddItemToObject(root,"video.image_control",config);
		ret = XCAM_SUCCESS;
	} else {
		LOG_ERR(LOG_TAG,"err(%s,%d): get video.image_control fail.\n", __func__, __LINE__);
		ret = XCAM_ERROR;
	}
	return ret;
}
extern int ithreadstat[3];
int xcam_json_get_video_encoder_osd(void *json_root)
{
	if(json_root == NULL) {
		LOG_ERR(LOG_TAG,"err(%s,%d): json root is NULL.\n",__func__,__LINE__);
		return XCAM_ERROR;
	}

	cJSON *root = NULL, *_cfg = NULL, *config = NULL, *configlist = NULL;
	root = (cJSON *)json_root;
	int ret = XCAM_ERROR;

	configlist = cJSON_GetObjectItem(root, "configlist");
	if (!configlist) {
		LOG_ERR(LOG_TAG,"err(%s,%d): json has no configlist\n", __func__, __LINE__);
		return XCAM_ERROR;
	}

	_cfg = configlist->child;
	if (_cfg ) {
		while ( _cfg ) {
			if (strcmp(_cfg->string,"encoder.osd") == 0 )
				break;
			if ((_cfg = _cfg->next) == NULL) {
				LOG_ERR(LOG_TAG,"err(%s,%d): configlist has no member encoder.osd\n", __func__, __LINE__);
				return XCAM_ERROR;
			}
		}
	} else {
		LOG_ERR(LOG_TAG,"err(%s,%d): configlist child is null,web transfor data format error\n", __func__, __LINE__);
		return XCAM_ERROR;
	}

	config = cJSON_GetObjectItem(root,"encoder.osd");
	if ( config == NULL ) {
		config = cJSON_CreateObject();
		cJSON_AddItemToObject(config,"timestampshow",cJSON_CreateNumber(g_extra_web_param.timestampshow));
		cJSON_AddItemToObject(config,"fpsshow",cJSON_CreateNumber(g_extra_web_param.fpsshow));
		cJSON_AddItemToObject(config,"logoshow",cJSON_CreateNumber(g_extra_web_param.titleshow));
		cJSON_AddItemToObject(config,"strin",cJSON_CreateString(g_extra_web_param.osdtitle));
		cJSON_AddItemToObject(config,"titlepos",cJSON_CreateNumber(g_extra_web_param.titlepos));
		cJSON_AddItemToObject(config,"ivsmove",cJSON_CreateNumber(ithreadstat[0]));
		cJSON_AddItemToObject(config,"ivspervehpet",cJSON_CreateNumber(ithreadstat[1]));
		cJSON_AddItemToObject(config,"ivsface",cJSON_CreateNumber(ithreadstat[2]));		
		cJSON_AddItemToObject(root,"encoder.osd",config);
		ret = XCAM_SUCCESS;
	} else {
		LOG_ERR(LOG_TAG,"err(%s,%d): get encoder.osd fail.\n", __func__, __LINE__);
		ret = XCAM_ERROR;
	}
	return ret;
}
int xcam_json_get_video_encoder_snap(void *json_root)
{
	if(json_root == NULL) {
		LOG_ERR(LOG_TAG,"err(%s,%d): json root is NULL.\n",__func__,__LINE__);
		return XCAM_ERROR;
	}

	cJSON *root = NULL, *_cfg = NULL, *config = NULL, *configlist = NULL;
	root = (cJSON *)json_root;
	int ret = XCAM_ERROR;

	configlist = cJSON_GetObjectItem(root, "configlist");
	if (!configlist) {
		LOG_ERR(LOG_TAG,"err(%s,%d): json has no configlist\n", __func__, __LINE__);
		return XCAM_ERROR;
	}

	_cfg = configlist->child;
	if (_cfg ) {
		while ( _cfg ) {
			if (strcmp(_cfg->string,"encoder.snap") == 0 )
				break;
			if ((_cfg = _cfg->next) == NULL) {
				LOG_ERR(LOG_TAG,"err(%s,%d): configlist has no member encoder.snap\n", __func__, __LINE__);
				return XCAM_ERROR;
			}
		}
	} else {
		LOG_ERR(LOG_TAG,"err(%s,%d): configlist child is null,web transfor data format error\n", __func__, __LINE__);
		return XCAM_ERROR;
	}

	config = cJSON_GetObjectItem(root,"encoder.snap");
	if ( config == NULL ) {
		config = cJSON_CreateObject();
		cJSON_AddItemToObject(config,"snapTimSel",cJSON_CreateNumber(g_extra_web_param.snapTimSel));
		cJSON_AddItemToObject(root,"encoder.snap",config);
		ret = XCAM_SUCCESS;
	} else {
		LOG_ERR(LOG_TAG,"err(%s,%d): get encoder.snap fail.\n", __func__, __LINE__);
		ret = XCAM_ERROR;
	}
	return ret;
}

int xcam_json_set_encoder_snap(void *json_root)
{
	if (json_root == NULL) {
		LOG_ERR(LOG_TAG,"err(%s,%d): json root is NULL.\n", __func__, __LINE__);
		return XCAM_ERROR;
	}
LOG_ERR(LOG_TAG,"err(%s,%d): json file has no this config.\n", __func__, __LINE__);
	cJSON *root = NULL, *config = NULL, *config_member = NULL;
	root = (cJSON *)json_root;

	config = cJSON_GetObjectItem(root, "encoder.snap");
	if (config == NULL) {
		LOG_ERR(LOG_TAG,"err(%s,%d): json file has no this config.\n", __func__, __LINE__);
		return XCAM_ERROR;
	}
	config_member = config->child;
	while (config_member) {
	
		if (strcmp(config_member->string, "snapChnSel") == 0) {
			g_extra_web_param.snapChnSel = config_member->valueint;
		} else if (strcmp(config_member->string, "snapTimSel") == 0) {
			g_extra_web_param.snapTimSel = config_member->valueint;
		} else if (strcmp(config_member->string, "snapManual") == 0) {
			g_extra_web_param.snapmanual = config_member->valueint;
			LOG_ERR(LOG_TAG,"err(%s,%d): json file has no this config. %d\n", __func__, config_member->valueint, g_extra_web_param.snapmanual );
		}
		config_member = config_member->next;
	}
	
	return 0;
}

extern int  xcam_ivs_start_thread(int ivs_index);
extern int  xcam_ivs_cancel_thread(int ivs_index);
int xcam_json_set_encoder_osd(void *json_root)
{
	if (json_root == NULL) {
		LOG_ERR(LOG_TAG,"err(%s,%d): json root is NULL.\n", __func__, __LINE__);
		return XCAM_ERROR;
	}

	cJSON *root = NULL, *config = NULL, *config_member = NULL;
	root = (cJSON *)json_root;
	// int ret = XCAM_ERROR, saturation = 0, brightness = 0, sharpness = 0, contrast = 0;
	config = cJSON_GetObjectItem(root, "encoder.osd");
	if (config == NULL) {
		LOG_ERR(LOG_TAG,"err(%s,%d): json file has no this config.\n", __func__, __LINE__);
		return XCAM_ERROR;
	}
	config_member = config->child;
	while (config_member) {
	
		if (strcmp(config_member->string, "timestampshow") == 0) {
			g_extra_web_param.timestampshow = config_member->valueint;
		}else if (strcmp(config_member->string, "fpsshow") == 0) {
			g_extra_web_param.fpsshow = config_member->valueint;
		}else if (strcmp(config_member->string, "logoshow") == 0) {
			g_extra_web_param.titleshow = config_member->valueint;
		}else if (strcmp(config_member->string, "strin") == 0) {
			memcpy(g_extra_web_param.osdtitle, config_member->valuestring, 20);
		}else if (strcmp(config_member->string, "titlepos") == 0) {
			g_extra_web_param.titlepos = config_member->valueint;
		} else if (strcmp(config_member->string, "ivsmove") == 0) {
			if(config_member->valueint){
				xcam_ivs_start_thread(0);
			}else{
				xcam_ivs_cancel_thread(0);
			}
			LOG_ERR(LOG_TAG,"err(%s,%d): json file has no this config.  %d\n", __func__, __LINE__, config_member->valueint);
		} else if (strcmp(config_member->string, "ivspervehpet") == 0) {
			if(config_member->valueint){
				xcam_ivs_start_thread(1);
			}else{
				xcam_ivs_cancel_thread(1);
			}
			LOG_ERR(LOG_TAG,"err(%s,%d): json file has no this config.  %d\n", __func__, __LINE__, config_member->valueint);
		} else if (strcmp(config_member->string, "ivsface") == 0) {
			if(config_member->valueint){
				xcam_ivs_start_thread(2);
			}else{
				xcam_ivs_cancel_thread(2);
			}
			LOG_ERR(LOG_TAG,"err(%s,%d): json file has no this config.  %d\n", __func__, __LINE__, config_member->valueint);
		}
		config_member = config_member->next;
	}
	xcam_osd_set_chnTitles(0, g_extra_web_param.osdtitle, 20, g_extra_web_param.timestampshow , g_extra_web_param.fpsshow, g_extra_web_param.titleshow, g_extra_web_param.titlepos);	
	xcam_osd_set_chnTitles(1, g_extra_web_param.osdtitle, 20, g_extra_web_param.timestampshow , g_extra_web_param.fpsshow, g_extra_web_param.titleshow, g_extra_web_param.titlepos);	
	return 0;
}

int xcam_json_set_video_qp(void *json_root)
{
	if (json_root == NULL) {
		LOG_ERR(LOG_TAG,"err(%s,%d): json root is NULL.\n", __func__, __LINE__);
		return XCAM_ERROR;
	}

	cJSON *root = NULL, *config = NULL, *config_member = NULL;
	root = (cJSON *)json_root;
	int ret = XCAM_ERROR;

	config = cJSON_GetObjectItem(root, "video.qp");
	if (config == NULL) {
		LOG_ERR(LOG_TAG,"Error(%s,%d): json file has no this config\n", __func__, __LINE__);
		return XCAM_ERROR;
	}
	config_member = config->child;
	while (config_member) {
		if(strcmp(config_member->string, "ch0") == 0) {
			int qp = (config_member->child->valueint);
			g_extra_web_param.ch0_qp = qp;
			ret = xcam_video_set_isp_qp(0, qp);
			if (ret < XCAM_SUCCESS) {
				LOG_ERR(LOG_TAG,"error(%s,%d),set video qp fail.\n",__func__,__LINE__);
				return ret;
			}
		} else if (strcmp(config_member->string, "ch1") == 0) {
			int qp = (config_member->child->valueint);
			g_extra_web_param.ch1_qp = qp;
			ret = xcam_video_set_isp_qp(1, qp);
			if (ret < XCAM_SUCCESS) {
				LOG_ERR(LOG_TAG,"error(%s,%d),set video qp fail.\n",__func__,__LINE__);
				return ret;
			}
		} else {
			LOG_ERR(LOG_TAG,"error(%s,%d),this config don't support.\n");
			return XCAM_ERROR;
		}

		config_member = config_member->next;
	}

	return XCAM_SUCCESS;
}

int xcam_json_get_video_qp(void *json_root)
{
	if (json_root == NULL) {
		LOG_ERR(LOG_TAG,"Error(%s,%d):json root is NULL\n", __func__, __LINE__);
		return XCAM_ERROR;
	}

	cJSON *root = (cJSON *)json_root;
	cJSON *_cfg = NULL;
	cJSON *config = cJSON_GetObjectItem(root, "video.qp");
	cJSON *config_member = NULL;
	cJSON *config_child = NULL;

	cJSON *cfglist = cJSON_GetObjectItem(root, "configlist");
	if (!cfglist) {
		LOG_ERR(LOG_TAG,"Error(%s,%d):json has no configlist.\n", __func__, __LINE__);
		return -1;
	}

	
	_cfg = cfglist->child;
	if (!_cfg)
		cJSON_AddItemToObject(cfglist, "video.qp",cJSON_CreateString("video.qp"));
	else {
		while (_cfg) {
			//暂时先这样，调通了再改，这个地方应该是_cfg->strings
			if (strcmp(_cfg->string, "video.qp") == 0)
				break;
			if ((_cfg = _cfg->next) == NULL)
				cJSON_AddItemToObject(cfglist, "video.qp",cJSON_CreateString("video.qp"));
		}
	}

	if (config == NULL || config->child == NULL) {
		config = cJSON_CreateObject();
		cJSON_AddItemToObject(root, "video.qp", config);

		config_child = cJSON_CreateObject();
		cJSON_AddItemToObject(config, "ch0", config_child);
		cJSON_AddItemToObject(config_child, "qp", cJSON_CreateNumber(g_extra_web_param.ch0_qp));

		config_child = cJSON_CreateObject();
		cJSON_AddItemToObject(config, "ch1", config_child);
		cJSON_AddItemToObject(config_child, "qp", cJSON_CreateNumber(g_extra_web_param.ch1_qp));
	} else {
		config_member = config->child;
		while (config_member) {
			if (strcmp(config_member->string, "ch0") == 0) {
				if(strcmp(config_member->child->string,"qp") == 0) {
					cJSON_ReplaceItemInObject(config_member,"qp",cJSON_CreateNumber(g_extra_web_param.ch0_qp));
				} else {
					cJSON_AddItemToObject(config_member, "qp", cJSON_CreateNumber(g_extra_web_param.ch0_qp));
				}
			} else if(strcmp(config_member->string, "ch1") == 0) {
				if(strcmp(config_member->child->string,"qp") == 0){
					cJSON_ReplaceItemInObject(config_member,"qp",cJSON_CreateNumber(g_extra_web_param.ch1_qp));
				} else {
					cJSON_AddItemToObject(config_member, "qp", cJSON_CreateNumber(g_extra_web_param.ch1_qp));
				}
			}
			config_member = config_member->next;
		}
	}

	return XCAM_SUCCESS;
}

int xcam_json_set_video_image_control(void *json_root)
{
	if (json_root == NULL) {
		LOG_ERR(LOG_TAG,"err(%s,%d): json root is NULL.\n", __func__, __LINE__);
		return XCAM_ERROR;
	}

	cJSON *root = NULL, *config = NULL, *config_member = NULL;
	root = (cJSON *)json_root;
	int ret = XCAM_ERROR, saturation = 0, brightness = 0, sharpness = 0, contrast = 0;
	int xcam_ispctl[EXTRA_ISPCTL_N]={-1, -1, -1, -1, -1, -1};

	config = cJSON_GetObjectItem(root, "video.image_control");
	if (config == NULL) {
		LOG_ERR(LOG_TAG,"err(%s,%d): json file has no this config.\n", __func__, __LINE__);
		return XCAM_ERROR;
	}

	config_member = config->child;
	while (config_member) {
		if (strcmp(config_member->string, "saturation") == 0) {
			saturation = config_member->valueint;
		} else if (strcmp(config_member->string, "brightness") == 0) {
			brightness = config_member->valueint;
		} else if (strcmp(config_member->string, "sharpness") == 0) {
			sharpness = config_member->valueint;
		} else if (strcmp(config_member->string, "contrast") == 0) {
			contrast = config_member->valueint;
		} else if (strcmp(config_member->string, "awb") == 0) {
			xcam_ispctl[AWB] = config_member->valueint;
		} else if (strcmp(config_member->string, "gamma") == 0) {
			xcam_ispctl[GAMMA] = config_member->valueint;
		} else if (strcmp(config_member->string, "filllight") == 0) {
			xcam_ispctl[FILLLIGHT] = config_member->valueint;
		} else if (strcmp(config_member->string, "visualangle") == 0) {
			xcam_ispctl[VISUALANGLE] = config_member->valueint;
		} else if (strcmp(config_member->string, "backlight") == 0) {
			xcam_ispctl[BACKLIGHT] = config_member->valueint;
		} else if (strcmp(config_member->string, "nightmode") == 0) {
			xcam_ispctl[NIGHTMODE] = config_member->valueint;
		}
		config_member = config_member->next;
	}
	//设置image_control
	ret = xcam_web_set_video_image_control(saturation,brightness,sharpness,contrast);
	if (ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG,"err(%s,%d): set image_control fail.\n", __func__, __LINE__);
	}

	ret = xcam_web_set_video_image_control_extra(xcam_ispctl,6);
	if (ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG,"err(%s,%d): set image_control extra fail.\n", __func__, __LINE__);
	}
	return ret;
}

int xcam_json_set_isp_awbAttr(void *json_root)
{
	if (json_root == NULL) {
		LOG_ERR(LOG_TAG,"err(%s,%d): json root is NULL.\n", __func__, __LINE__);
		return XCAM_ERROR;
	}

	cJSON *root = NULL, *config = NULL, *config_member = NULL;
	root = (cJSON *)json_root;
	int ret = XCAM_ERROR;

	config = cJSON_GetObjectItem(root, "video.isp.awbAttr");
	if (config == NULL) {
		LOG_ERR(LOG_TAG,"err(%s,%d): json file has no this config.\n", __func__, __LINE__);
		return XCAM_ERROR;
	}
	
	config_member = config->child;
	while (config_member) {
		if (strcmp(config_member->string, "awb_mode") == 0) {
			int count = cJSON_GetArraySize(config_member);
			
			int awb_modes[32];
			for (int i = 0; i < count; i++) {
				cJSON *awb_mode = cJSON_GetArrayItem(config_member, i);
				if (awb_mode == NULL) {
					LOG_ERR(LOG_TAG,"err(%s,%d): awb_mode is NULL.\n", __func__, __LINE__);
					return XCAM_ERROR;
				}
				awb_modes[i] = atoi(awb_mode->valuestring);
				
			}
			ret = xcam_Tuning_SetAwbAttr( 0, awb_modes);
		} else{
			LOG_ERR(LOG_TAG,"err(%s,%d): unknown isp attr\n", __func__, __LINE__);
		}
		config_member = config_member->next;

	}

	return ret;
}


int xcam_json_set_video_ispcontrolInfo(void *json_root)
{
	if (json_root == NULL) {
		LOG_ERR(LOG_TAG,"err(%s,%d): json root is NULL.\n", __func__, __LINE__);
		return XCAM_ERROR;
	}

	cJSON *root = NULL, *config = NULL, *config_member = NULL;
	root = (cJSON *)json_root;
	int ret = XCAM_ERROR, nightvision = 0, wdr = 0, imagemirror = 0, imageflip = 0, ircut = 0, noisereduction = 0, drc = 0;

	config = cJSON_GetObjectItem(root, "video.ispcontrolInfo");
	if (config == NULL) {
		LOG_ERR(LOG_TAG,"err(%s,%d): json file has no this config.\n", __func__, __LINE__);
		return XCAM_ERROR;
	}

	config_member = config->child;
	while (config_member) {
		if (strcmp(config_member->string, "nightvisionmode") == 0) {
			nightvision = config_member->valueint;
			ret = xcam_web_set_video_nightvisionmode(nightvision);
			if (ret < XCAM_SUCCESS) {
				LOG_ERR(LOG_TAG,"err(%s,%d): Set nightvisonmode fail.\n", __func__, __LINE__);
				return ret;
			}

			ircut = config_member->valueint;
			ret = xcam_web_set_video_ircutmode(ircut);
			if (ret < XCAM_SUCCESS) {
				LOG_ERR(LOG_TAG,"err(%s,%d): Set ircutmode fail.\n", __func__, __LINE__);
				return ret;
			}
		} else if (strcmp(config_member->string, "wdr") == 0) {
			wdr = config_member->valueint;
			ret = xcam_web_set_video_wdrmode(wdr);
			if (ret < XCAM_SUCCESS) {
				LOG_ERR(LOG_TAG,"err(%s,%d): Set wdrmode fail.\n", __func__, __LINE__);
				return ret;
			}
		} else if (strcmp(config_member->string, "imagemirror") == 0) {
			imagemirror = config_member->valueint;
			ret = xcam_web_set_video_imagemirrormode(imagemirror);
			if (ret < XCAM_SUCCESS) {
				LOG_ERR(LOG_TAG,"err(%s,%d): Set imagemirrormode fail.\n", __func__, __LINE__);
				return ret;
			}
		} else if (strcmp(config_member->string, "imageflip") == 0) {
			imageflip = config_member->valueint;
			ret = xcam_web_set_video_imageflipmode(imageflip);
			if (ret < XCAM_SUCCESS) {
				LOG_ERR(LOG_TAG,"err(%s,%d): Set imageflipmode fail.\n", __func__, __LINE__);
				return ret;
			}
		} else if (strcmp(config_member->string, "ircut") == 0) {
			// ircut = config_member->valueint;
			// ret = xcam_web_set_video_ircutmode(ircut);
			// if (ret < XCAM_SUCCESS) {
			// 	LOG_ERR(LOG_TAG,"err(%s,%d): Set ircutmode fail.\n", __func__, __LINE__);
			// 	return ret;
			// }
			ret = 0;
		} else if (strcmp(config_member->string, "noisereduction") == 0) {
			noisereduction = config_member->valueint;
			ret = xcam_web_set_video_noisereductionmode(noisereduction);
			if (ret < XCAM_SUCCESS) {
				LOG_ERR(LOG_TAG,"err(%s,%d): Set noisereductionmode fail.\n", __func__, __LINE__);
				return ret;
			}
		} else if (strcmp(config_member->string, "drc") == 0) {
			drc = config_member->valueint;
			ret = xcam_web_set_video_drcmode(drc);
			if (ret < XCAM_SUCCESS) {
				LOG_ERR(LOG_TAG,"err(%s,%d): Set drc fail.\n", __func__, __LINE__);
				return ret;
			}
		} else {
			LOG_ERR(LOG_TAG,"err(%s,%d): Invalid parameter.\n", __func__, __LINE__);
			return XCAM_ERROR;
		}
		config_member = config_member->next;
	}

	return ret;
}

int xcam_json_get_video_ispcontrolInfo(void *json_root)
{
	if(json_root == NULL) {
		LOG_ERR(LOG_TAG,"err(%s,%d): json root is NULL.\n",__func__,__LINE__);
		return XCAM_ERROR;
	}

	cJSON *root = NULL, *_cfg = NULL, *config = NULL, *configlist = NULL;
	root = (cJSON *)json_root;
	int ret = XCAM_ERROR;

	configlist = cJSON_GetObjectItem(root, "configlist");
	if (!configlist) {
		LOG_ERR(LOG_TAG,"err(%s,%d): json has no configlist\n", __func__, __LINE__);
		return XCAM_ERROR;
	}

	_cfg = configlist->child;
	if (_cfg ) {
		while ( _cfg ) {
			if (strcmp(_cfg->string,"video.ispcontrolInfo") == 0 )
				break;
			if ((_cfg = _cfg->next) == NULL) {
				LOG_ERR(LOG_TAG,"err(%s,%d): configlist has no member isp\n", __func__, __LINE__);
				return XCAM_ERROR;
			}
		}
	} else {
		LOG_ERR(LOG_TAG,"err(%s,%d): configlist child is null,web transfor data format error\n", __func__, __LINE__);
		return XCAM_ERROR;
	}

	config = cJSON_GetObjectItem(root,"video.ispcontrolInfo");
	if ( config == NULL ) {
		config = cJSON_CreateObject();
		int nightvision = 0, wdr = 0, imagemirror = 0, imageflip = 0, drc = 0, noisereduction = 0, ircut = 0;

		IMPISPRunningMode isp_runningmode = IMPISP_RUNNING_MODE_DAY;
		IMP_ISP_Tuning_GetISPRunningMode(0,&isp_runningmode);
		if(IMPISP_RUNNING_MODE_NIGHT == isp_runningmode){
			nightvision = 1;
		} else if(IMPISP_RUNNING_MODE_DAY == isp_runningmode) {
			nightvision = 0;
		} else {
			nightvision = 0;
		}

		IMPISPHVFLIPAttr hvfattr;
		IMPISPHVFLIP hvflipmode;
		IMP_ISP_Tuning_GetHVFLIP(0, &hvfattr);
		hvflipmode = hvfattr.sensor_mode;
		switch(hvflipmode) {
			case IMPISP_FLIP_NORMAL_MODE:
				imagemirror = 0;
				imageflip = 0;
				break;
			case IMPISP_FLIP_H_MODE:
				imagemirror = 1;
				imageflip = 0;
				break;
			case IMPISP_FLIP_V_MODE:
				imagemirror = 0;
				imageflip = 1;
				break;
			case IMPISP_FLIP_HV_MODE:
				imagemirror = 1;
				imageflip = 1;
				break;
			default:
				break;
		}

		cJSON_AddItemToObject(config,"nightvisionmode",cJSON_CreateNumber(nightvision));
		cJSON_AddItemToObject(config,"wdrmode",cJSON_CreateNumber(wdr));
		cJSON_AddItemToObject(config,"imagemirrormode",cJSON_CreateNumber(imagemirror));
		cJSON_AddItemToObject(config,"imageflipmode",cJSON_CreateNumber(imageflip));
		cJSON_AddItemToObject(config,"ircutmode",cJSON_CreateNumber(ircut));
		cJSON_AddItemToObject(config,"drcmode",cJSON_CreateNumber(drc));
		cJSON_AddItemToObject(config,"noisereductionmode",cJSON_CreateNumber(noisereduction));

		cJSON_AddItemToObject(root,"video.ispcontrolInfo",config);
		ret = XCAM_SUCCESS;
	} else {
		LOG_ERR(LOG_TAG,"err(%s,%d): get video.ispcontrolInfo fail.\n", __func__, __LINE__);
		ret = XCAM_ERROR;
	}
	return ret;
}

int xcam_json_get_sys_ipconfig(void *json_root)
{
	if (json_root == NULL) {
		LOG_ERR(LOG_TAG,"err(%s,%d): json root is NULL.\n", __func__, __LINE__);
		return XCAM_ERROR;
	}

	cJSON *root = NULL, *_cfg = NULL, *config = NULL, *configlist = NULL;
	root = (cJSON *)json_root;
	char ip_addr[20] = {0};
	char mask [20] = {0};
	char gateway [20] = {0};
	char DNSaddr [20] = {0};
	char interface[20] = {0};
	unsigned char mac[48] = {0};
	int ret = XCAM_SUCCESS, auto_type = 0;
	bool IsDhcpEnable = false;

	configlist = cJSON_GetObjectItem(root, "configlist");
	if (!configlist) {
		LOG_ERR(LOG_TAG,"err(%s,%d): json has no configlist\n", __func__, __LINE__);
		return XCAM_ERROR;
	}

	_cfg = cJSON_GetObjectItem(configlist,"network.information");
	if (_cfg == NULL) {
	/*	LOG_ERR(LOG_TAG, "error(%s,%d),cfglist child equal to NULL, web transfor data format error.\n",__func__,__LINE__);
		return XCAM_ERROR;*/
		cJSON_AddItemToObject(configlist,"network.information", cJSON_CreateString("network.information"));
	}

	ret = xcam_network_get_net_interface(interface);
	if (ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG,"error(%s,%d),get network interface fail.\n",__func__,__LINE__);
		return ret;
	}

	ret = xcam_network_get_device_ip(interface, ip_addr);
	if (ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG,"error(%s,%d),get device ip addr fail.\n",__func__,__LINE__);
		return ret;
	}

	ret = xcam_network_get_device_mac(interface, mac);
	if (ret < XCAM_SUCCESS ) {
		LOG_ERR(LOG_TAG,"error(%s, %d),get devide mac addr fail.\n",__func__,__LINE__);
		return ret;
	}

	ret = xcam_network_get_device_ip_gateway(interface, gateway);
	if (ret < 0) {
		LOG_ERR(LOG_TAG,"error(%s,%d),get device gateway fail.\n",__func__,__LINE__);
		return ret;
	}

	ret = xcam_network_get_device_ip_mask(interface, mask);
	if (ret < 0) {
		LOG_ERR(LOG_TAG,"error(%s,%d),get device ip mask fail.\n",__func__,__LINE__);
		return ret;
	}

	ret = xcam_network_get_device_DNS_server_addr(DNSaddr);
	if (ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG,"error(%s,%d),get DNS server addr fail.\n",__func__,__LINE__);
		return ret;
	}

	xcam_network_dhcp_status_and_pid(&IsDhcpEnable,NULL);
	if (IsDhcpEnable == true)
		auto_type = 1;
	else
		auto_type = 0;

	config = cJSON_GetObjectItem(root, "network.information");
	if ( config == NULL ) {
		config = cJSON_CreateObject();
		cJSON_AddItemToObject(config, "auto",cJSON_CreateNumber(auto_type));
		cJSON_AddItemToObject(config, "ipaddr",cJSON_CreateString(ip_addr));
		cJSON_AddItemToObject(config, "macaddr",cJSON_CreateString((char *)mac));
		cJSON_AddItemToObject(config, "dns",cJSON_CreateString(DNSaddr));
		cJSON_AddItemToObject(config, "mask",cJSON_CreateString(mask));
		cJSON_AddItemToObject(config,"gateway",cJSON_CreateString(gateway));
		cJSON_AddItemToObject(root,"network.information",config);
	} else {
		LOG_ERR(LOG_TAG,"error(%s,%d),web transfor data format error.\n",__func__,__LINE__);
		ret = XCAM_ERROR;
	}

	return ret;
}

int xcam_json_set_sys_ipconfig(void *json_root)
{
	if (json_root == NULL) {
		LOG_ERR(LOG_TAG,"err(%s,%d): json root is NULL.\n", __func__, __LINE__);
		return -1;
	}

	char ip_addr[20] = {0} , ip_addr_temp[20]  = {0};
	char mask[20] = {0} , maskTemp[20] = {0};
	char gateway[20] = {0}, gatewayTemp[20] = {0};
	char DNSaddr[20] = {0}, DNSaddrTemp[20] = {0};
	char interface[20] = {0} ;
	int ret = XCAM_SUCCESS, auto_type = -1, auto_type_temp = -1;
	bool IsDhcpEnable = false;
	cJSON *root = NULL, *config = NULL, *member = NULL;

	root = (cJSON *)json_root;
	config = cJSON_GetObjectItem(root, "network.information");
	if (config == NULL) {
		LOG_ERR(LOG_TAG,"error(%s,%d),transfor data format call function error.\n",__func__,__LINE__);
		return XCAM_ERROR;
	}

	/*获取当前root中的数据信息*/
	member = cJSON_GetObjectItem(config, "auto");
	if (member) {
		auto_type_temp = member->valueint;
	}

	member = cJSON_GetObjectItem(config,"ipaddr");
	if (member) {
		strlcpy(ip_addr_temp, member->valuestring, sizeof(ip_addr_temp));
	}

	member = cJSON_GetObjectItem(config, "dns");
	if (member) {
		strlcpy(DNSaddrTemp, member->valuestring, sizeof(DNSaddrTemp));
	}

	member = cJSON_GetObjectItem(config,"mask");
	if (member) {
		strlcpy(maskTemp, member->valuestring, sizeof(maskTemp));
	}

	member = cJSON_GetObjectItem(config, "gateway");
	if (member) {
		strlcpy(gatewayTemp,member->valuestring, sizeof(gatewayTemp));
	}

	ret = xcam_network_get_net_interface(interface);
	if (ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG,"error(%s,%d),get network interface fail.\n",__func__,__LINE__);
		return ret;
	}

	xcam_network_dhcp_status_and_pid(&IsDhcpEnable,NULL);
	if (IsDhcpEnable == true)
		auto_type = 1;
	else
		auto_type = 0;

	if (auto_type == 0 && auto_type_temp ==1) {
		//启动dhcp
		xcam_network_start_dhcp();
	} else if (auto_type == 1 && auto_type_temp == 0 ) {
		//停止dhcp
		xcam_network_stop_dhcp();
	} else {
		//状态一样不做任何动作
		LOG_INF(LOG_TAG,"DBG(%s,%d),auto status is equal to current configration.\n",__func__,__LINE__);
	}

	//只有DHCP服务未启动的时候再可以设置IP地址掩码网关
	if (((auto_type == 1) && (auto_type_temp == 0)) || ((auto_type == 0) && ( auto_type_temp == 0))) {
		//判断是都设置ip/mask/gateway
		ret = xcam_network_get_device_ip(interface, ip_addr);
		if (ret < XCAM_SUCCESS) {
			LOG_ERR(LOG_TAG,"error(%s,%d),get device ip addr fail.\n",__func__,__LINE__);
			return ret;
		} else {
			if (strcmp(ip_addr,ip_addr_temp) != 0) {
				xcam_msg_data_t info;
				memset(&info,0,sizeof(xcam_msg_data_t));
				info.info_flag = XCAM_MESSAGE_SET_IPADDR;
				memcpy(info.data.xcam_ipaddr.interface, interface, 20);
				memcpy(info.data.xcam_ipaddr.ipaddr,ip_addr_temp,20);
				ret = xcam_message_send(&info);
				if (ret < XCAM_SUCCESS) {
					LOG_ERR(LOG_TAG,"error(%s,%d),set static ip addr fail.\n",__func__,__LINE__);
					return ret;
				}
			} else {
				LOG_INF(LOG_TAG,"DBG(%s,%d),ip addr is equal to current configration.\n",__func__,__LINE__);
			}
		}

		ret = xcam_network_get_device_ip_gateway(interface, gateway);
		if (ret < 0) {
			LOG_ERR(LOG_TAG,"error(%s,%d),get device gateway fail.\n",__func__,__LINE__);
			return ret;
		} else {
			if(strcmp (gateway,gatewayTemp) != 0 ) {
				ret = xcam_network_set_device_ip_gateway(gatewayTemp);
				if (ret < XCAM_SUCCESS) {
					LOG_ERR(LOG_TAG,"error(%s,%d),set sevide ip gateway fail.\n",__func__,__LINE__);
					return ret;
				}
			} else {
				LOG_INF(LOG_TAG,"DBG(%s,%d),ip gateway is equal to current configration.\n",__func__,__LINE__);
			}
		}

		ret = xcam_network_get_device_ip_mask(interface, mask);
		if (ret < 0) {
			LOG_ERR(LOG_TAG,"error(%s,%d),get device ip mask fail.\n",__func__,__LINE__);
			return ret;
		} else {
			if (strcmp(mask,maskTemp) != 0) {
				ret = xcam_network_set_device_ip_mask(interface, maskTemp);
				if (ret < XCAM_SUCCESS) {
					LOG_ERR(LOG_TAG,"error(%s,%d),set devide ip mask fail.\n",__func__,__LINE__);
					return ret;
				}
			} else {
				LOG_INF(LOG_TAG,"DBG(%s,%d),ip mask is equal to current configration.\n",__func__,__LINE__);
			}
		}
	}

	//mac地址是只读元素,不可以设置的,让xcamtool和web中做判断不可以设置
	/*	DNS 地址暂时也不支持修改 , 目前set接口是伪代码*/
		ret = xcam_network_get_device_DNS_server_addr(DNSaddr);
		if (ret < XCAM_SUCCESS) {
			LOG_ERR(LOG_TAG,"error(%s,%d),get DNS server addr fail.\n",__func__,__LINE__);
			return ret;
		} else {
			if (strcmp(DNSaddr,DNSaddrTemp) != 0) {
				ret = xcam_network_set_device_DNS_server_addr(DNSaddrTemp);
				if (ret < 0) {
					LOG_ERR(LOG_TAG,"error(%s,%d),set devide DNS addr fail.\n",__func__,__LINE__);
					return ret;
				}
			} else {
				LOG_INF(LOG_TAG,"DBG(%s,%d),DNS addr is equal to current configration.\n",__func__,__LINE__);
			}
		}

	return ret;
}

int xcam_json_get_video_Iframe_interval(void *json_root)
{
	if (json_root == NULL) {
		LOG_ERR(LOG_TAG,"err(%s,%d): json root is NULL.\n", __func__, __LINE__);
		return XCAM_ERROR;
	}

	cJSON *root = NULL, *_cfg = NULL, *config = NULL, *configlist = NULL, *ch = NULL;
	root = (cJSON *)json_root;

	configlist = cJSON_GetObjectItem(root, "configlist");
	if (!configlist) {
		LOG_ERR(LOG_TAG,"err(%s,%d): json has no configlist\n", __func__, __LINE__);
		return XCAM_ERROR;
	}

	_cfg = cJSON_GetObjectItem(configlist,"video.goplength");
	if (_cfg == NULL) {
		/*获取全部的时候configlist里面是为空的*/
		cJSON_AddItemToObject(configlist,"video.goplength", cJSON_CreateString("video.goplength"));
	}

	config = cJSON_GetObjectItem(root, "video.goplength");
	if (config == NULL) {
		config = cJSON_CreateObject();
		cJSON_AddItemToObject(root,"video.goplength",config);

		ch = cJSON_CreateObject();

		cJSON_AddItemToObject(ch, "goplength",cJSON_CreateNumber(stream_attr.stream_config[0].pstGOPSizeCfg.gopsize));
		cJSON_AddItemToObject(config, "ch0",ch);

		ch = cJSON_CreateObject();
		cJSON_AddItemToObject(ch, "goplength",cJSON_CreateNumber(stream_attr.stream_config[1].pstGOPSizeCfg.gopsize));
		cJSON_AddItemToObject(config, "ch1",ch);
	} else {
		LOG_ERR(LOG_TAG,"error(%s,%d),web transfor data format error.\n",__func__,__LINE__);
		return XCAM_ERROR;
	}

	return XCAM_SUCCESS;
}

int xcam_json_get_isp_awbAttr(void *json_root){
	int ret = 0;
	if (json_root == NULL) {
			LOG_ERR(LOG_TAG,"err(%s,%d): json root is NULL.\n", __func__, __LINE__);
			return XCAM_ERROR;
		}

	cJSON *root = NULL, *_cfg = NULL, *config = NULL, *configlist = NULL, *ch = NULL, *config_mode = NULL;
	root = (cJSON *)json_root;

	configlist = cJSON_GetObjectItem(root, "configlist");
	if (!configlist) {
		LOG_ERR(LOG_TAG,"err(%s,%d): json has no configlist\n", __func__, __LINE__);
		return XCAM_ERROR;
	}

	_cfg = cJSON_GetObjectItem(configlist,"video.isp.awbAttr");
	if (_cfg == NULL) {
		/*获取全部的时候configlist里面是为空的*/
		cJSON_AddItemToObject(configlist,"video.isp.awbAttr", cJSON_CreateString("video.isp.awbAttr"));
	}
	config_mode  = cJSON_GetObjectItem(root,"video_isp_awbAttr");
	if(config_mode == NULL){
		LOG_ERR(LOG_TAG,"error(%s,%d),web transfor data format error., no config_mode\n",__func__,__LINE__);
		return XCAM_ERROR;
	}else{
		config_mode  = cJSON_GetObjectItem(config_mode,"modes");
	}
	config = cJSON_GetObjectItem(root, "video.isp.awbAttr");
	if (config == NULL) {
		int awb_mode[32];
		int size = cJSON_GetArraySize(config_mode);
		config = cJSON_CreateObject();
		ch = cJSON_CreateObject();
		cJSON_AddItemToObject(root,"video.isp.awbAttr",config);
		for(int i = 0; i < size; i++){
			cJSON *awb_modex = cJSON_GetArrayItem(config_mode, i);
			if(!strncmp(awb_modex->valuestring, "IMPISPWBAttr", sizeof("IMPISPWBAttr"))){
				ret = xcam_Tuning_GetAwbAttr(0, awb_mode);
				cJSON_AddItemToObject(ch, "IMPISPWBAttr",cJSON_CreateIntArray(awb_mode,15));
			} else if(!strncmp(awb_modex->valuestring, "IMPISPAWBGlobalStatisInfo", sizeof("IMPISPAWBGlobalStatisInfo"))){
				ret = xcam_Tuning_GetAWBGlobalStatisInfo(0, awb_mode);
				cJSON_AddItemToObject(ch, "IMPISPAWBGlobalStatisInfo",cJSON_CreateIntArray(awb_mode,4));
			} else if(!strncmp(awb_modex->valuestring, "IMPISPAwbOnlyReadAttr", sizeof("IMPISPAwbOnlyReadAttr"))){
				ret = xcam_Tuing_GetAwbOnlyReadAttr(0, awb_mode);
				cJSON_AddItemToObject(ch, "IMPISPAwbOnlyReadAttr",cJSON_CreateIntArray((const int*)awb_mode,1));
			}
		}
		cJSON_AddItemToObject(config, "sensor0",ch);

		// ch = cJSON_CreateObject();
		// cJSON_AddItemToObject(ch, "goplength",cJSON_CreateNumber(stream_attr.stream_config[1].pstGOPSizeCfg.gopsize));
		// cJSON_AddItemToObject(config, "ch1",ch);
	} else {
		LOG_ERR(LOG_TAG,"error(%s,%d),web transfor data format error.\n",__func__,__LINE__);
		return XCAM_ERROR;
	}

	return ret;
}
int xcam_json_set_video_Iframe_interval(void *json_root)
{
	if (json_root == NULL) {
		LOG_ERR(LOG_TAG,"err(%s,%d): json root is NULL.\n", __func__, __LINE__);
		return XCAM_ERROR;
	}

	cJSON *root = NULL, *config = NULL, *config_member = NULL, *part = NULL;
	root = (cJSON *)json_root;
	int ret = XCAM_ERROR, goplength = 0;

	config = cJSON_GetObjectItem(root, "video.goplength");
	if (config == NULL) {
		LOG_ERR(LOG_TAG,"Error(%s,%d): json file has no this config\n", __func__, __LINE__);
		return XCAM_ERROR;
	}

	config_member = config->child;
	while(config_member){
		if(strcmp(config_member->string, "ch0") == 0) {
			part = config_member->child;
			goplength = part->valueint;
			ret = xcam_video_set_encode_GopLength(0, goplength);
			if (ret < XCAM_SUCCESS) {
				LOG_ERR(LOG_TAG,"error(%s,%d),set video goplength fail.\n",__func__,__LINE__);
				return ret;
			}
		} else if (strcmp(config_member->string, "ch1") == 0) {
			part = config_member->child;
			goplength = part->valueint;
			ret = xcam_video_set_encode_GopLength(1, goplength);
			if (ret < XCAM_SUCCESS) {
				LOG_ERR(LOG_TAG,"error(%s,%d),set video goplength fail.\n",__func__,__LINE__);
				return ret;
			}
		} else {
			LOG_ERR(LOG_TAG,"error(%s,%d),this config don't support.\n");
			return XCAM_ERROR;
		}

		config_member = config_member->next;
	}

	return XCAM_SUCCESS;
}

int xcam_json_get_video_Rcmode(void *json_root)
{
	if (json_root == NULL) {
		LOG_ERR(LOG_TAG,"Error(%s,%d):json root is NULL.\n", __func__, __LINE__);
		return XCAM_ERROR;
	}

	cJSON *root = NULL, *_cfg = NULL, *config = NULL, *configlist = NULL, *ch = NULL;
	root = (cJSON *)json_root;
	int i = 0;
	char ch_name [5] = {0};

	configlist = cJSON_GetObjectItem(root, "configlist");
	if (!configlist) {
		LOG_ERR(LOG_TAG,"err(%s,%d): json has no configlist\n", __func__, __LINE__);
		return XCAM_ERROR;
	}

	_cfg = cJSON_GetObjectItem(configlist,"video.rcmode");
	if (_cfg == NULL) {
		/*获取全部的时候configlist里面是为空的*/
		cJSON_AddItemToObject(configlist,"video.rcmode", cJSON_CreateString("video.rcmode"));
	}

	config = cJSON_GetObjectItem(root, "video.rcmode");
	if (config == NULL) {
		config = cJSON_CreateObject();
		cJSON_AddItemToObject(root,"video.rcmode",config);

		for (i =0 ; i< 2; i++) {
			if (stream_attr.stream_config[i].enc_attr.rcAttr.attrRcMode.rcMode == 1) {
				ch = cJSON_CreateObject();
				cJSON_AddItemToObject(ch, "rcmode",cJSON_CreateString("cbr"));
				sprintf(ch_name,"ch%d",i);
				cJSON_AddItemToObject(config, ch_name, ch);
			} else if (stream_attr.stream_config[i].enc_attr.rcAttr.attrRcMode.rcMode == 2) {
				ch = cJSON_CreateObject();
				cJSON_AddItemToObject(ch,"rcmode",cJSON_CreateString("vbr"));
				sprintf(ch_name,"ch%d",i);
				cJSON_AddItemToObject(config, ch_name, ch);
			} else if (stream_attr.stream_config[i].enc_attr.rcAttr.attrRcMode.rcMode == 3) {
				ch = cJSON_CreateObject();
				cJSON_AddItemToObject(ch,"rcmode",cJSON_CreateString("smart"));
				sprintf(ch_name,"ch%d",i);
				cJSON_AddItemToObject(config, ch_name, ch);
			} else if (stream_attr.stream_config[i].enc_attr.rcAttr.attrRcMode.rcMode == 4) {
				ch = cJSON_CreateObject();
				cJSON_AddItemToObject(ch,"rcmode",cJSON_CreateString("cvbr"));
				sprintf(ch_name,"ch%d",i);
				cJSON_AddItemToObject(config, ch_name, ch);
			} else if (stream_attr.stream_config[i].enc_attr.rcAttr.attrRcMode.rcMode == 5) {
				ch = cJSON_CreateObject();
				cJSON_AddItemToObject(ch,"rcmode",cJSON_CreateString("avbr"));
				sprintf(ch_name,"ch%d",i);
				cJSON_AddItemToObject(config, ch_name, ch);
			}else {
				LOG_ERR(LOG_TAG,"error(%s,%d),web transfor data format error. rcmode=%d\n",__func__,__LINE__, stream_attr.stream_config[i].enc_attr.rcAttr.attrRcMode.rcMode);
				return XCAM_ERROR;
			}
		}
	} else {
		LOG_ERR(LOG_TAG,"error(%s,%d),web transfor data format error.\n",__func__,__LINE__);
		return XCAM_ERROR;
	}

	return XCAM_SUCCESS;
}

int xcam_json_set_video_Rcmode(void *json_root)
{
	if (json_root == NULL) {
		LOG_ERR(LOG_TAG,"err(%s,%d): json root is NULL.\n", __func__, __LINE__);
		return XCAM_ERROR;
	}

	cJSON *root = NULL, *config = NULL, *config_member = NULL, *part = NULL;
	root = (cJSON *)json_root;
	int ret = XCAM_ERROR, rcmode = -1;

	config = cJSON_GetObjectItem(root, "video.rcmode");
	if (config == NULL) {
		LOG_ERR(LOG_TAG,"Error(%s,%d): json file has no this config\n", __func__, __LINE__);
		return XCAM_ERROR;
	}

	config_member = config->child;
	while (config_member) {
		if(strcmp(config_member->string, "ch0") == 0) {
			part = config_member->child;
			if (strcmp(part->valuestring, "cbr") == 0) {
				rcmode = 1;
			} else if (strcmp(part->valuestring, "vbr") == 0) {
				rcmode = 2;
			} else if (strcmp(part->valuestring, "smart") == 0) {
				rcmode = 3;
			} else if (strcmp(part->valuestring, "cvbr") == 0) {
				rcmode = 4;
			} else if (strcmp(part->valuestring, "avbr") == 0) {
				rcmode = 5;
			}else {
				LOG_ERR(LOG_TAG,"error(%s,%d),Invalid parameter.\n",__func__,__LINE__);
				return XCAM_ERROR;
			}

			ret = xcam_video_set_encode_Rcmode(0, rcmode);
			if (ret < XCAM_SUCCESS) {
				LOG_ERR(LOG_TAG,"error(%s,%d),set video rcmode fail.\n",__func__,__LINE__);
				return ret;
			}
		} else if (strcmp(config_member->string, "ch1") == 0) {
			part = config_member->child;
			if (strcmp(part->valuestring, "cbr") == 0) {
				rcmode = 1;
			} else if (strcmp(part->valuestring, "vbr") == 0) {
				rcmode = 2;
			} else if (strcmp(part->valuestring, "smart") == 0) {
				rcmode = 3;
			} else if (strcmp(part->valuestring, "cvbr") == 0) {
				rcmode = 4;
			} else if (strcmp(part->valuestring, "avbr") == 0) {
				rcmode = 5;
			} else {
				LOG_ERR(LOG_TAG,"error(%s,%d),Invalid parameter.\n",__func__,__LINE__);
				return XCAM_ERROR;
			}

			ret = xcam_video_set_encode_Rcmode(1, rcmode);
			if (ret < XCAM_SUCCESS) {
				LOG_ERR(LOG_TAG,"error(%s,%d),set video rcmode fail.\n",__func__,__LINE__);
				return ret;
			}
		} else {
			LOG_ERR(LOG_TAG,"error(%s,%d),this config don't support.\n");
			return XCAM_ERROR;
		}

		config_member = config_member->next;
	}

	return XCAM_SUCCESS;
}

int xcam_json_get_web_http_port(void *json_root)
{
	if (json_root == NULL) {
		LOG_ERR(LOG_TAG,"err(%s,%d): json root is NULL.\n", __func__, __LINE__);
		return XCAM_ERROR;
	}

	cJSON *root = NULL, *_cfg = NULL,*config = NULL, *cfglist = NULL;
	root = (cJSON *)json_root;
	int ret = XCAM_ERROR, port = 0;

	cfglist = cJSON_GetObjectItem(root, "configlist");
	if (!cfglist) {
		LOG_ERR(LOG_TAG,"err(%s,%d): json has no configlist\n", __func__, __LINE__);
		return XCAM_ERROR;
	}

	_cfg = cfglist->child;
	if ( _cfg ) {
		while (_cfg) {
			if (strcmp(_cfg->string, "web.httpport") == 0)
				break;
			if ((_cfg = _cfg->next) == NULL) {
				LOG_ERR(LOG_TAG,"error(%s,%d),configlist no member web.httpport,call function error.\n",__func__,__LINE__);
				return XCAM_ERROR;
			}
		}
	} else {
		LOG_ERR(LOG_TAG, "error(%s,%d),cfglist child equal to NULL, web transfor data format error.\n",__func__,__LINE__);
		return XCAM_ERROR;
	}

	ret = xcam_web_get_http_port(&port);
	if(ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG,"error(%s,%d),get http port fail.\n",__func__,__LINE__);
		return ret;
	}
	config = cJSON_GetObjectItem(root, "web.httpport");
	if ( config == NULL) {
		config = cJSON_CreateObject();
		cJSON_AddItemToObject(config, "port", cJSON_CreateNumber(port));
		cJSON_AddItemToObject(root, "web.httpport", config);
	} else {
		LOG_ERR(LOG_TAG,"error(%s,%d),web transfor data format error.\n",__func__,__LINE__);
		ret = XCAM_ERROR;
	}

	return ret;
}

int xcam_json_set_web_http_port(void *json_root)
{
	if (json_root == NULL) {
		LOG_ERR(LOG_TAG,"err(%s,%d): json root is NULL.\n", __func__, __LINE__);
		return XCAM_ERROR;
	}

	cJSON *root = NULL, *config = NULL, *config_member = NULL;
	root = (cJSON *)json_root;
	int ret = XCAM_ERROR, port = 0;

	config = cJSON_GetObjectItem(root, "web.httpport");
	if (config == NULL) {
		LOG_ERR(LOG_TAG,"Error(%s,%d): json file has no this config\n", __func__, __LINE__);
		return XCAM_ERROR;
	}

	config_member = config->child;
	while (config_member) {
		if (strcmp(config_member->string, "port") == 0) {
			port = config_member->valueint;
		} else {
			LOG_ERR(LOG_TAG,"error(%s,%d),Invalid parameter.\n",__func__,__LINE__);
			return XCAM_ERROR;
		}
		config_member = config_member->next;
	}

	ret = xcam_web_set_http_port(port);
	if(ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG,"error(%s,%d),set http port fail.\n",__func__,__LINE__);
		return ret;
	}

	return ret;
}

int xcam_json_get_channel_switch(void *json_root)
{
	if (json_root == NULL) {
		LOG_ERR(LOG_TAG,"Error(%s,%d):json root is NULL.\n", __func__, __LINE__);
		return XCAM_ERROR;
	}

	cJSON *root = NULL, *_cfg = NULL, *config = NULL, *configlist = NULL, *ch = NULL;
	root = (cJSON *)json_root;

	configlist = cJSON_GetObjectItem(root, "configlist");
	if (!configlist) {
		LOG_ERR(LOG_TAG,"err(%s,%d): json has no configlist\n", __func__, __LINE__);
		return XCAM_ERROR;
	}

	_cfg = cJSON_GetObjectItem(configlist,"video.switch");
	if (_cfg == NULL) {
		/*获取全部的时候configlist里面是为空的*/
		cJSON_AddItemToObject(configlist,"video.switch", cJSON_CreateString("video.switch"));
	}

	config = cJSON_GetObjectItem(root, "video.switch");
	if (config == NULL) {
		config = cJSON_CreateObject();
		cJSON_AddItemToObject(root, "video.switch", config);

		ch = cJSON_CreateObject();
		cJSON_AddItemToObject(ch, "status", cJSON_CreateNumber((int)switch_ch0));
		cJSON_AddItemToObject(config, "ch0", ch);

		ch = cJSON_CreateObject();
		cJSON_AddItemToObject(ch, "status", cJSON_CreateNumber((int)switch_ch1));
		cJSON_AddItemToObject(config, "ch1", ch);
	} else {
		LOG_ERR(LOG_TAG,"error(%s,%d),web transfor data format error.\n",__func__,__LINE__);
		return XCAM_ERROR;
	}

	return XCAM_SUCCESS;
}

int xcam_json_set_channel_switch(void *json_root)
{
	if (json_root == NULL) {
		LOG_ERR(LOG_TAG,"err(%s,%d): json root is NULL.\n", __func__, __LINE__);
		return XCAM_ERROR;
	}

	cJSON *root = NULL, *config = NULL, *config_member = NULL, *part = NULL;
	root = (cJSON *)json_root;
	int ret = XCAM_SUCCESS;

	config = cJSON_GetObjectItem(root, "video.switch");
	if (config == NULL) {
		LOG_ERR(LOG_TAG,"Error(%s,%d): json file has no this config\n", __func__, __LINE__);
		return XCAM_ERROR;
	}

	config_member = config->child;
	while (config_member) {
		if(strcmp(config_member->string, "ch0") == 0) {
			part = config_member->child;
			switch_ch0 = (bool)part->valueint;
		} else if (strcmp(config_member->string, "ch1") == 0) {
			part = config_member->child;
			switch_ch1 = (bool)part->valueint;
		} else {
			LOG_ERR(LOG_TAG,"error(%s,%d),this config don't support.\n",__func__,__LINE__);
			return XCAM_ERROR;
		}

		config_member = config_member->next;
	}

	return ret;
}

int xcam_json_get_rtsp_addr(void *json_root)
{
	if (json_root == NULL) {
		LOG_ERR(LOG_TAG,"Error(%s,%d):json root is NULL.\n", __func__, __LINE__);
		return XCAM_ERROR;
	}

	cJSON *root = NULL, *_cfg = NULL, *config = NULL, *configlist = NULL, *ch = NULL;
	root = (cJSON *)json_root;
	int ret = XCAM_SUCCESS;
	char rtsp[128] = {0}, interface[20] = {0},ip_addr[20] = {0};

	ret = xcam_network_get_net_interface(interface);
	if (ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG,"error(%s,%d),get network interface fail.\n",__func__,__LINE__);
		return ret;
	}

	ret = xcam_network_get_device_ip(interface, ip_addr);
	if (ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG,"error(%s,%d),get device ip addr fail.\n",__func__,__LINE__);
		return ret;
	}

	sprintf(rtsp,"rtsp://%s:8554/stream0",ip_addr);

	configlist = cJSON_GetObjectItem(root, "configlist");
	if (!configlist) {
		LOG_ERR(LOG_TAG,"err(%s,%d): json has no configlist\n", __func__, __LINE__);
		return XCAM_ERROR;
	}

	_cfg = cJSON_GetObjectItem(configlist,"video.rtspaddr");
	if (_cfg == NULL) {
		/*获取全部的时候configlist里面是为空的*/
		cJSON_AddItemToObject(configlist,"video.rtspaddr", cJSON_CreateString("video.rtspaddr"));
	}

	config = cJSON_GetObjectItem(root, "video.rtspaddr");
	if (config == NULL) {
		config = cJSON_CreateObject();
		cJSON_AddItemToObject(root, "video.rtspaddr", config);

		ch = cJSON_CreateObject();
		sprintf(rtsp,"rtsp://%s:8554/stream0",ip_addr);
		cJSON_AddItemToObject(ch, "rtspaddr", cJSON_CreateString(rtsp));
		cJSON_AddItemToObject(ch, "port", cJSON_CreateNumber(8554));
		cJSON_AddItemToObject(ch, "width", cJSON_CreateNumber(stream_attr.stream_config[0].fs_attr.picWidth));
		cJSON_AddItemToObject(ch, "height", cJSON_CreateNumber(stream_attr.stream_config[0].fs_attr.picHeight));
#ifdef XCAM_DOUBLE_SENSOR
		cJSON_AddItemToObject(ch, "sec_sensor", cJSON_CreateNumber(1));
#endif
		cJSON_AddItemToObject(config, "ch0", ch);

		ch = cJSON_CreateObject();
		sprintf(rtsp,"rtsp://%s:8554/stream1",ip_addr);
		cJSON_AddItemToObject(ch, "rtspaddr", cJSON_CreateString(rtsp));
		cJSON_AddItemToObject(ch, "port", cJSON_CreateNumber(8554));
		cJSON_AddItemToObject(ch, "width", cJSON_CreateNumber(stream_attr.stream_config[1].fs_attr.picWidth));
		cJSON_AddItemToObject(ch, "height", cJSON_CreateNumber(stream_attr.stream_config[1].fs_attr.picHeight));
		cJSON_AddItemToObject(config, "ch1", ch);
	} else {
		LOG_ERR(LOG_TAG,"error(%s,%d),web transfor data format error.\n",__func__,__LINE__);
		return XCAM_ERROR;
	}

	return XCAM_SUCCESS;
}

int xcam_json_get_device_info(void *json_root)
{
	if (json_root == NULL) {
		LOG_ERR(LOG_TAG,"err(%s,%d): json root is NULL.\n", __func__, __LINE__);
		return XCAM_ERROR;
	}

	int ret = XCAM_SUCCESS;
	cJSON *root = NULL, *_cfg = NULL,*config = NULL, *cfglist = NULL;

	root = (cJSON *)json_root;

	cfglist = cJSON_GetObjectItem(root, "configlist");
	if (!cfglist) {
		LOG_ERR(LOG_TAG,"err(%s,%d): json has no configlist\n", __func__, __LINE__);
		return XCAM_ERROR;
	}

	_cfg = cfglist->child;
	if ( _cfg ) {
		while (_cfg) {
			if (strcmp(_cfg->string, "product.information") == 0)
				break;
			if ((_cfg = _cfg->next) == NULL) {
				LOG_ERR(LOG_TAG,"error(%s,%d),configlist no member product.information,call function error.\n",__func__,__LINE__);
				return XCAM_ERROR;
			}
		}
	} else {
		LOG_ERR(LOG_TAG, "error(%s,%d),cfglist child equal to NULL, web transfor data format error.\n",__func__,__LINE__);
		return XCAM_ERROR;
	}

	config = cJSON_GetObjectItem(root, "product.information");
	if ( config == NULL) {
		config = cJSON_CreateObject();
		cJSON_AddItemToObject(config, "name", cJSON_CreateString("JZ-XCAM-IPC"));
		cJSON_AddItemToObject(config, "model", cJSON_CreateString("T41-N"));
		cJSON_AddItemToObject(config, "number", cJSON_CreateString("ingenic:2020-0720-2345"));
		cJSON_AddItemToObject(config, "kernel_version", cJSON_CreateString("Linux-2.6.1804"));
		cJSON_AddItemToObject(config, "server_version", cJSON_CreateString("Boa-0.94.13"));
		cJSON_AddItemToObject(root, "product.information", config);
	} else {
		LOG_ERR(LOG_TAG,"error(%s,%d),web transfor data format error.\n",__func__,__LINE__);
		ret = XCAM_ERROR;
	}

	return ret;
}

int xcam_socket_set_configs_process(socket_tcp_link_t *plink)
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

	/*根据json表的信息进行对应的处理*/
	while ((ret == XCAM_SUCCESS) && member) {
		if (strcmp(member->string,"video.isp.fps") == 0) {
			ret |= xcam_json_set_video_isp_fps(root_r);
		} else if(strcmp(member->string, "video.fs.fps") == 0) {
			ret |= xcam_json_set_video_fs_fps(root_r);
		} else if(strcmp(member->string, "video.enc.fps") == 0) {
			ret |= xcam_json_set_video_enc_fps(root_r);
		} else if (strcmp(member->string, "video.bitrate") == 0) {
			ret |= xcam_json_set_video_bps(root_r);
		} else if (strcmp(member->string, "video.resolution") == 0) {
			ret |= xcam_json_set_video_resolution(root_r);
		} else if (strcmp(member->string, "network.information") == 0) {
			ret |= xcam_json_set_sys_ipconfig(root_r);
		} else if (strcmp(member->string, "video.enctype") == 0) {
			ret |= xcam_json_set_video_encode_mode(root_r);
		} else if (strcmp(member->string, "video.goplength") == 0) {
			ret |= xcam_json_set_video_Iframe_interval(root_r);
		} else if (strcmp(member->string,"video.rcmode") == 0) {
			ret |= xcam_json_set_video_Rcmode(root_r);
		} else if (strcmp(member->string, "return_value") == 0) {
			member = member->next;
			continue;
		} else {
			LOG_INF(LOG_TAG,"inf(%s,%d):xcam server don't support %s.\n",__func__,__LINE__,member->valuestring);
			ret = XCAM_ERROR;
		}
		member = member->next;
	}

	/*根据返回值组装返回客户端的信息*/
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
	buf->header.cmd = SET_CONFIGS_ACK;
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

int xcam_socket_get_configs_process(socket_tcp_link_t *plink)
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
	if(root == NULL){
		LOG_ERR(LOG_TAG,"err(%s,%d):recv data format error,cJSON parse fail.\n",__func__,__LINE__);
		ret = XCAM_ERROR;
	}

	root_err = cJSON_CreateObject();
	cfglist = cJSON_GetObjectItem(root, "configlist");
	if(cfglist == NULL) {
		ret = XCAM_ERROR;
	} else {
		member = cfglist->child;
	}

	if ((ret == XCAM_SUCCESS) && (cfglist->child == NULL)) {
		ret |= xcam_json_get_video_isp_fps(root);
		ret |= xcam_json_get_video_fs_fps(root);
		ret |= xcam_json_get_video_enc_fps(root);
		ret |= xcam_json_get_video_bps(root);
		ret |= xcam_json_get_video_resolution(root);
		ret |= xcam_json_get_sys_ipconfig(root);
		ret |= xcam_json_get_video_encode_mode(root);
		ret |= xcam_json_get_video_Iframe_interval(root);
		ret |= xcam_json_get_video_Rcmode(root);
		ret |= xcam_json_get_video_ispcontrolInfo(root);
	} else {
		member = cfglist->child;
		while ((ret == XCAM_SUCCESS) && member) {
			if (strcmp(member->string, "video.isp.fps") == 0) {
				ret |= xcam_json_get_video_isp_fps(root);
			} else if(strcmp(member->string, "video.fs.fps") == 0) {
				ret |= xcam_json_get_video_fs_fps(root);
			} else if(strcmp(member->string, "video.enc.fps") == 0) {
				ret |= xcam_json_get_video_enc_fps(root);
			} else if(strcmp(member->string, "video.bitrate") == 0) {
				ret |= xcam_json_get_video_bps(root);
			} else if(strcmp(member->string, "video.resolution") == 0) {
				ret |= xcam_json_get_video_resolution(root);
			} else if(strcmp(member->string,"video.enctype") == 0) {
				ret |= xcam_json_get_video_encode_mode(root);
			} else if(strcmp(member->string,"network.information") == 0) {
				ret |= xcam_json_get_sys_ipconfig(root);
			} else if (strcmp(member->string,"video.goplength") == 0) {
				ret |= xcam_json_get_video_Iframe_interval(root);
			} else if (strcmp(member->string,"video.rcmode") == 0) {
				ret |= xcam_json_get_video_Rcmode(root);
			} else if (strcmp(member->string,"video.ispcontrolInfo") == 0) {
				ret |= xcam_json_get_video_ispcontrolInfo(root);
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
	buf->header.cmd = GET_CONFIGS_ACK;
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

int msg_process_func(socket_tcp_link_t *plink)
{
	int ret = XCAM_SUCCESS;

	switch(plink->msgbuf->header.cmd) {
		case SET_CONFIGS_SEND:
			ret = xcam_socket_set_configs_process(plink);
			if (ret < XCAM_SUCCESS) {
				LOG_ERR(LOG_TAG,"err(%s,%d): set configs err.\n", __func__, __LINE__);
				return -1;
			}
			break;
		case GET_CONFIGS_SEND:
			ret = xcam_socket_get_configs_process(plink);
			if (ret < XCAM_SUCCESS) {
				LOG_ERR(LOG_TAG,"err(%s,%d): get configs err.\n", __func__, __LINE__);
				return XCAM_ERROR;
			}
			break;
		case CMD_WEB_CFGS_SEND:
			ret = xcam_web_socket_config_process(plink);
			if (ret < XCAM_SUCCESS) {
				LOG_ERR(LOG_TAG,"err(%s,%d): set web configs err.\n",__func__,__LINE__);
				return XCAM_ERROR;
			}
			break;
		default:
			LOG_ERR(LOG_TAG,"err(%s,%d): cmd not support.\n", __func__, __LINE__);
			return XCAM_ERROR;
			break;
	}

	return XCAM_SUCCESS;
}

#define PORT_START 51002
void* cJson_start(void* arg)
{
	int ret = XCAM_SUCCESS;
	void *skt = NULL;
	int retry = 1;
	skt = socket_server_tcp_alloc(PORT_START,10);
	while(skt == NULL){
		skt = socket_server_tcp_alloc(PORT_START + retry, 10);
		retry += 1;
	}
	assert(NULL != skt);
	ret = socket_server_tcp_set_msg_process_cb(skt, msg_process_func);
	assert(0 == ret);
	ret = socket_server_tcp_start(skt);
	assert(0 == ret);
	while(1) {
		sleep(1);
	}

	return NULL;
}

void cJson_process_init(void)
{
    xcam_thread_create("cjson_main", cJson_start, NULL);
	return;
}
