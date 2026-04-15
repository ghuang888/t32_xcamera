#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <imp/imp_common.h>
#include <imp/imp_encoder.h>
#include "xcam_log.h"
#include "cJSON.h"
#include "xcam_module.h"
#include "xcam_gb.h"
#include "xcam_conf_gb.h"
#include "xcam_conf_json.h"
#include "xcam_general.h"

#define CONFIG_SIP_FILE "/system/etc/sip_config.json"
#define CONFIG_BASE_FILE "/system/etc/base_config.json"

#define LOG_TAG "GBJsonConf"

conf_gb_t gb_device_default_param = {
	.sipattr = {
		.sipswitch = 0,
		.serverid = "34020000002000000001",
		.serverid_len = 21,
		.serverip = "193.169.4.79",
		.serverip_len = 13,
		.serverport = 5060,
		.logindomain = "3402000000",
		.logindomain_len = 11,
		.deviceid = "34020000001320000001",
		.deviceid_len = 21,
		.localport = 5060,
		.authpasswd = "12345678",
		.authpasswd_len = 9,
		.authmode = 0,
		.transtype = 0,
		.expires = 7200,
		.hbinterval = 30,
		.hbcount = 3,
		.mediaport = 6000,
	},
	.baseattr = {
		.name = "jz-svac",
		.name_len = 8,
		.manufacturer = "ingenic",
		.manufacturer_len = 8,
		.model = "ipc001",
		.model_len = 7,
		.owner = "ingenic",
		.owner_len = 8,
		.civilcode = "0551-gx-000",
		.civilcode_len = 12,
		.block = "0551-110",
		.block_len = 9,
		.address = "Hefei-Gaoxin",
		.address_len = 13,
		.parentid = "34020000001320000001",
		.parentid_len = 21,
		.longitude = 117.7,
		.latitude = 31.5,
		.ptzType = 0,
		.positionType = 0,
		.roomType = 0,
		.useType = 0,
		.supplylightType = 0,
		.directionType = 0,
		.groupID = "34020000001320000001",
		.groupID_len = 21,
	},
};

static void cJSON_add_string_to_object(cJSON * const object, const char * const name, const char * const string)
{
	cJSON *item;
	if (!object|| !name|| !string)
		return;
	item = cJSON_GetObjectItem(object, name);
	if (!item)
		cJSON_AddStringToObject(object, name, string);  // just addstring
	else{
		free(item->valuestring); // free present valuestring
		item->valuestring = strdup(string); // malloc and init new valuestring
	}
}

static void cJSON_add_int_to_object(cJSON * const object, const char * const name, const int value)
{
	cJSON *item;
	if (!object|| !name)
		return;
	item = cJSON_GetObjectItem(object, name);
	if (!item)
		cJSON_AddNumberToObject(object, name, value);  // just addstring
	else{
		item->valueint = value; // malloc and init new valuestring
		item->valuedouble = value; // malloc and init new valuestring
	}
}

static int _create_sip_config_file(void)
{
	cJSON *root = NULL;
	cJSON *sipattr = NULL;
	int ret = XCAM_SUCCESS;
	root = cJSON_CreateObject();
	if(!root){
		LOG_ERR(LOG_TAG,"Create the json object of gb fail.\n");
		return XCAM_ERROR;
	}
	sipattr = cJSON_CreateObject();
	cJSON_AddItemToObject(root,"sip.config",sipattr);
	cJSON_AddItemToObject(sipattr,"sipswitch",cJSON_CreateNumber(gb_device_default_param.sipattr.sipswitch));
	cJSON_AddItemToObject(sipattr,"serverid",cJSON_CreateString(gb_device_default_param.sipattr.serverid));
	cJSON_AddItemToObject(sipattr,"serverip",cJSON_CreateString(gb_device_default_param.sipattr.serverip));
	cJSON_AddItemToObject(sipattr,"serverport",cJSON_CreateNumber(gb_device_default_param.sipattr.serverport));
	cJSON_AddItemToObject(sipattr,"logindomain",cJSON_CreateString(gb_device_default_param.sipattr.logindomain));
	cJSON_AddItemToObject(sipattr,"deviceid",cJSON_CreateString(gb_device_default_param.sipattr.deviceid));
	cJSON_AddItemToObject(sipattr,"localport",cJSON_CreateNumber(gb_device_default_param.sipattr.localport));
	cJSON_AddItemToObject(sipattr,"authpasswd",cJSON_CreateString(gb_device_default_param.sipattr.authpasswd));
	cJSON_AddItemToObject(sipattr,"authmode",cJSON_CreateNumber(gb_device_default_param.sipattr.authmode));
	cJSON_AddItemToObject(sipattr,"transtype",cJSON_CreateNumber(gb_device_default_param.sipattr.transtype));
	cJSON_AddItemToObject(sipattr,"expires",cJSON_CreateNumber(gb_device_default_param.sipattr.expires));
	cJSON_AddItemToObject(sipattr,"heartbeatinterval",cJSON_CreateNumber(gb_device_default_param.sipattr.hbinterval));
	cJSON_AddItemToObject(sipattr,"heartbeatcount",cJSON_CreateNumber(gb_device_default_param.sipattr.hbcount));
	cJSON_AddItemToObject(sipattr,"mediaport",cJSON_CreateNumber(gb_device_default_param.sipattr.mediaport));
	ret = json_write_file(CONFIG_SIP_FILE,root);
	if(ret != XCAM_SUCCESS)
		LOG_ERR(LOG_TAG,"errno(%s,%d),Call json_write_file fail.\n",__func__,__LINE__);
	cJSON_Delete(root);
	return ret;
}

static int _create_base_config_file(void)
{
	cJSON *root = NULL;
	cJSON *baseattr = NULL;
	int ret = XCAM_SUCCESS;
	root = cJSON_CreateObject();
	if(!root){
		LOG_ERR(LOG_TAG,"Create the json object of gb fail.\n");
		return XCAM_ERROR;
	}
	baseattr = cJSON_CreateObject();
	cJSON_AddItemToObject(root,"base.config",baseattr);
	cJSON_AddItemToObject(baseattr,"name",cJSON_CreateString(gb_device_default_param.baseattr.name));
	cJSON_AddItemToObject(baseattr,"manufacturer",cJSON_CreateString(gb_device_default_param.baseattr.manufacturer));
	cJSON_AddItemToObject(baseattr,"model",cJSON_CreateString(gb_device_default_param.baseattr.model));
	cJSON_AddItemToObject(baseattr,"owner",cJSON_CreateString(gb_device_default_param.baseattr.owner));
	cJSON_AddItemToObject(baseattr,"civilcode",cJSON_CreateString(gb_device_default_param.baseattr.civilcode));
	cJSON_AddItemToObject(baseattr,"block",cJSON_CreateString(gb_device_default_param.baseattr.block));
	cJSON_AddItemToObject(baseattr,"address",cJSON_CreateString(gb_device_default_param.baseattr.address));
	cJSON_AddItemToObject(baseattr,"parentid",cJSON_CreateString(gb_device_default_param.baseattr.parentid));
	cJSON_AddItemToObject(baseattr,"longitude",cJSON_CreateNumber(gb_device_default_param.baseattr.longitude));
	cJSON_AddItemToObject(baseattr,"latitude",cJSON_CreateNumber(gb_device_default_param.baseattr.latitude));
	cJSON_AddItemToObject(baseattr,"ptzType",cJSON_CreateNumber(gb_device_default_param.baseattr.ptzType));
	cJSON_AddItemToObject(baseattr,"positionType",cJSON_CreateNumber(gb_device_default_param.baseattr.positionType));
	cJSON_AddItemToObject(baseattr,"roomType",cJSON_CreateNumber(gb_device_default_param.baseattr.roomType));
	cJSON_AddItemToObject(baseattr,"useType",cJSON_CreateNumber(gb_device_default_param.baseattr.useType));
	cJSON_AddItemToObject(baseattr,"supplylightType",cJSON_CreateNumber(gb_device_default_param.baseattr.supplylightType));
	cJSON_AddItemToObject(baseattr,"directionType",cJSON_CreateNumber(gb_device_default_param.baseattr.directionType));
	cJSON_AddItemToObject(baseattr,"groupid",cJSON_CreateString(gb_device_default_param.baseattr.groupID));
	ret = json_write_file(CONFIG_BASE_FILE,root);
	if(ret != XCAM_SUCCESS)
		LOG_ERR(LOG_TAG,"errno(%s,%d),Call json_write_file fail.\n",__func__,__LINE__);
	cJSON_Delete(root);
	return ret;
}

static int _set_sip_config(struct svac_device_sip_attr *sipattr)
{
	cJSON *root = NULL;
	cJSON *item = NULL;
	cJSON *config = NULL;
	root = json_read_file(CONFIG_SIP_FILE);
	item = root->child;
	if(!item)
		return XCAM_ERROR;
	if(strcmp(item->string,"sip.config") != 0)
		return XCAM_ERROR;
	if(item->child == NULL)
		return XCAM_ERROR;
	config = cJSON_GetObjectItem(root,"sip.config");
	if(!config)
		return XCAM_ERROR;
	cJSON_add_string_to_object(config, "serverip", sipattr->serverip);
	cJSON_add_string_to_object(config, "serverid", sipattr->serverid);
	cJSON_add_string_to_object(config, "logindomain", sipattr->logindomain);
	cJSON_add_string_to_object(config, "deviceid", sipattr->deviceid);
	cJSON_add_string_to_object(config, "authpasswd", sipattr->authpasswd);
	cJSON_add_int_to_object(config,"sipswitch",sipattr->sipswitch);
	cJSON_add_int_to_object(config,"serverport",sipattr->serverport);
	cJSON_add_int_to_object(config,"localport",sipattr->localport);
	cJSON_add_int_to_object(config,"authmode",sipattr->authmode);
	cJSON_add_int_to_object(config,"transtype",sipattr->transtype);
	cJSON_add_int_to_object(config,"expires",sipattr->expires);
	cJSON_add_int_to_object(config,"heartbeatintervalt",sipattr->hbinterval);
	cJSON_add_int_to_object(config,"heartbeatcount",sipattr->hbcount);
	cJSON_add_int_to_object(config,"mediaport",sipattr->mediaport);
	json_write_file(CONFIG_SIP_FILE,root);
	cJSON_Delete(root);
	return XCAM_SUCCESS;
}

static void *_get_sip_config(struct svac_device_sip_attr *sipattr)
{
	cJSON *root = NULL;
	cJSON *item = NULL;
	cJSON *member = NULL;
	root = json_read_file(CONFIG_SIP_FILE);
	item = root->child;
	if(!item)
		return NULL;
	if(strcmp(item->string,"sip.config") != 0)
		return NULL;
	if(item->child == NULL)
		return NULL;
#if 0
	char *json_str = cJSON_Print(root);
	printf("%s-%d\n %s\n",__func__,__LINE__,json_str);
#endif
	member = item->child;
	while(member){
		if(strcmp(member->string,"sipswitch") == 0){
			sipattr->sipswitch = member->valueint;
		}else if(strcmp(member->string,"serverid") == 0){
			sipattr->serverid = member->valuestring;
			sipattr->serverid_len = strlen(member->valuestring)+1;
		}else if(strcmp(member->string,"serverip") == 0){
			sipattr->serverip = member->valuestring;
			sipattr->serverip_len = strlen(member->valuestring)+1;
		}else if(strcmp(member->string,"serverport") == 0){
			sipattr->serverport = member->valueint;
		}else if(strcmp(member->string,"logindomain") == 0){
			sipattr->logindomain = member->valuestring;
			sipattr->logindomain_len = strlen(member->valuestring)+1;
		}else if(strcmp(member->string,"deviceid") == 0){
			sipattr->deviceid = member->valuestring;
			sipattr->deviceid_len = strlen(member->valuestring)+1;
		}else if(strcmp(member->string,"localport") == 0){
			sipattr->localport = member->valueint;
		}else if(strcmp(member->string,"authpasswd") == 0){
			sipattr->authpasswd = member->valuestring;
			sipattr->authpasswd_len = strlen(member->valuestring)+1;
		}else if(strcmp(member->string,"authmode") == 0){
			sipattr->authmode = member->valueint;
		}else if(strcmp(member->string,"transtype") == 0){
			sipattr->transtype = member->valueint;
		}else if(strcmp(member->string,"expires") == 0){
			sipattr->expires = member->valueint;
		}else if(strcmp(member->string,"heartbeatinterval") == 0){
			sipattr->hbinterval = member->valueint;
		}else if(strcmp(member->string,"heartbeatcount") == 0){
			sipattr->hbcount = member->valueint;
		}else if(strcmp(member->string,"mediaport") == 0){
			sipattr->mediaport = member->valueint;
		}else;
		member = member->next;
	}
	return root;
}

static int _set_base_config(struct svac_device_base_attr *baseattr)
{
	cJSON *root = NULL;
	cJSON *item = NULL;
	cJSON *member = NULL;
	root = json_read_file(CONFIG_BASE_FILE);
	item = root->child;
	if(!item)
		return XCAM_ERROR;
	if(strcmp(item->string,"base.config") != 0)
		return XCAM_ERROR;
	if(item->child == NULL)
		return XCAM_ERROR;
	member = item->child;
	while(member){
		if(strcmp(member->string,"name") == 0){
		}else if(strcmp(member->string,"manufacturer") == 0){
		}else if(strcmp(member->string,"model") == 0){
		}else if(strcmp(member->string,"owner") == 0){
		}else if(strcmp(member->string,"civilcode") == 0){
		}else if(strcmp(member->string,"block") == 0){
		}else if(strcmp(member->string,"address") == 0){
		}else if(strcmp(member->string,"parentid") == 0){
		}else if(strcmp(member->string,"longitude") == 0){
		}else if(strcmp(member->string,"latitude") == 0){
		}else if(strcmp(member->string,"ptzType") == 0){
		}else if(strcmp(member->string,"positionType") == 0){
		}else if(strcmp(member->string,"roomType") == 0){
		}else if(strcmp(member->string,"useType") == 0){
		}else if(strcmp(member->string,"supplylightType") == 0){
		}else if(strcmp(member->string,"directionType") == 0){
		}else if(strcmp(member->string,"groupid") == 0){
		}else;
		member = member->next;
	}
	cJSON_Delete(root);
	return XCAM_SUCCESS;
}

static void *_get_base_config(struct svac_device_base_attr *baseattr)
{
	cJSON *root = NULL;
	cJSON *item = NULL;
	cJSON *member = NULL;
	root = json_read_file(CONFIG_BASE_FILE);
	item = root->child;
	if(!item)
		return NULL;
	if(strcmp(item->string,"base.config") != 0)
		return NULL;
	if(item->child == NULL)
		return NULL;
	member = item->child;
#if 0
	char *json_str = cJSON_Print(root);
	printf("%s-%d\n %s\n",__func__,__LINE__,json_str);
#endif
	while(member){
		if(strcmp(member->string,"name") == 0){
			baseattr->name = member->valuestring;
			baseattr->name_len = strlen(member->valuestring)+1;
		}else if(strcmp(member->string,"manufacturer") == 0){
			baseattr->manufacturer = member->valuestring;
			baseattr->manufacturer_len = strlen(member->valuestring)+1;
		}else if(strcmp(member->string,"model") == 0){
			baseattr->model = member->valuestring;
			baseattr->model_len = strlen(member->valuestring)+1;
		}else if(strcmp(member->string,"owner") == 0){
			baseattr->owner = member->valuestring;
			baseattr->owner_len = strlen(member->valuestring)+1;
		}else if(strcmp(member->string,"civilcode") == 0){
			baseattr->civilcode = member->valuestring;
			baseattr->civilcode_len = strlen(member->valuestring)+1;
		}else if(strcmp(member->string,"block") == 0){
			baseattr->block = member->valuestring;
			baseattr->block_len = strlen(member->valuestring)+1;
		}else if(strcmp(member->string,"address") == 0){
			baseattr->address = member->valuestring;
			baseattr->address_len = strlen(member->valuestring)+1;
		}else if(strcmp(member->string,"parentid") == 0){
			baseattr->parentid = member->valuestring;
			baseattr->parentid_len = strlen(member->valuestring)+1;
		}else if(strcmp(member->string,"longitude") == 0){
			baseattr->longitude = member->valuedouble;
		}else if(strcmp(member->string,"latitude") == 0){
			baseattr->latitude = member->valuedouble;
		}else if(strcmp(member->string,"ptzType") == 0){
			baseattr->ptzType = member->valueint;
		}else if(strcmp(member->string,"positionType") == 0){
			baseattr->positionType = member->valueint;
		}else if(strcmp(member->string,"roomType") == 0){
			baseattr->roomType = member->valueint;
		}else if(strcmp(member->string,"useType") == 0){
			baseattr->useType = member->valueint;
		}else if(strcmp(member->string,"supplylightType") == 0){
			baseattr->supplylightType = member->valueint;
		}else if(strcmp(member->string,"directionType") == 0){
			baseattr->directionType = member->valueint;
		}else if(strcmp(member->string,"groupid") == 0){
			baseattr->groupID = member->valuestring;
			baseattr->groupID_len = strlen(member->valuestring)+1;
		}else;
		member = member->next;
	}
	return root;
}

static int _get_sip_attr(void *handle)
{
	int ret = XCAM_SUCCESS;
	cJSON *root = NULL;
	struct svac_device_sip_attr sipattr;
	memset(&sipattr,0,sizeof(struct svac_device_sip_attr));
	if((root = _get_sip_config(&sipattr)) == NULL)
		return XCAM_ERROR;

	ret = set_device_sip_attr(handle,&sipattr);
	cJSON_Delete(root);
	return ret;
}

static int _get_base_attr(void *handle)
{
	int ret = XCAM_SUCCESS;
	cJSON *root = NULL;
	struct svac_device_base_attr baseattr;
	memset(&baseattr,0,sizeof(struct svac_device_base_attr));
	if((root = _get_base_config(&baseattr)) == NULL)
		return XCAM_ERROR;
	ret = set_device_base_attr(handle,&baseattr);
	cJSON_Delete(root);
	return ret;
}

int xcam_gb_manage_conf_init(void *handle)
{
	int ret = XCAM_SUCCESS;
	FILE *fp= NULL;
	fp = fopen(CONFIG_SIP_FILE,"r");
	//if (access(CONFIG_SIP_FILE,F_OK) != 0){//NFS might be error
	if(fp == NULL){
		ret =  _create_sip_config_file();
		if(ret != XCAM_SUCCESS)
			return XCAM_ERROR;
		ret = set_device_sip_attr(handle,&gb_device_default_param.sipattr);
		if(ret != XCAM_SUCCESS)
			return XCAM_ERROR;
		//todo:[encode/encryption]
	}else{
		ret = _get_sip_attr(handle);
		fclose(fp);
		if(ret)
			return XCAM_ERROR;
		//todo:[encode/encryption]
	}
	fp = fopen(CONFIG_BASE_FILE,"r");
	//if (access(CONFIG_BASE_FILE,F_OK) != 0){//NFS might be error
	if(fp == NULL){
		ret =  _create_base_config_file();
		if(ret != XCAM_SUCCESS)
			return XCAM_ERROR;
		ret = set_device_base_attr(handle,&gb_device_default_param.baseattr);
		if(ret != XCAM_SUCCESS)
			return XCAM_ERROR;
	}else{
		ret = _get_base_attr(handle);
		fclose(fp);
	}
	return ret;
}

int xcam_json_get_sip_config(void *json_root)
{
	struct svac_device_sip_attr sipattr;
	int ret = XCAM_SUCCESS;
	cJSON *root = NULL, *_cfg = NULL,*config = NULL,*config_member = NULL, *tag = NULL,*sip_root = NULL;
	if (json_root == NULL) {
		LOG_ERR(LOG_TAG,"err(%s,%d): json root is NULL\n", __func__, __LINE__);
		return XCAM_ERROR;
	}
	memset(&sipattr,0,sizeof(struct svac_device_sip_attr));
	root = (cJSON *)json_root;

	cJSON *cfglist = cJSON_GetObjectItem(root, "configlist");
	if (!cfglist) {
		LOG_ERR(LOG_TAG,"err(%s,%d): json has no configlist\n", __func__, __LINE__);
		return XCAM_ERROR;
	}
	if((sip_root = _get_sip_config(&sipattr)) == NULL){
		return XCAM_ERROR;
	}
	_cfg = cfglist->child;
	if (!_cfg)
		cJSON_AddItemToObject(cfglist, "sip.config",cJSON_CreateString("sip.config"));
	else {
		while (_cfg) {
			if (strcmp(_cfg->string, "sip.config") == 0)
				break;
			if ((_cfg = _cfg->next) == NULL)
				cJSON_AddItemToObject(cfglist, "sip.config",cJSON_CreateString("sip.config"));
		}
	}

	config = cJSON_GetObjectItem(root, "sip.config");
	if (config == NULL || config->child == NULL) {
		config = cJSON_CreateObject();
		cJSON_AddItemToObject(root, "sip.config", config);
		cJSON_AddItemToObject(config,"sipswitch",cJSON_CreateNumber(sipattr.sipswitch));
		cJSON_AddItemToObject(config,"serverid",cJSON_CreateString(sipattr.serverid));
		cJSON_AddItemToObject(config,"serverip",cJSON_CreateString(sipattr.serverip));
		cJSON_AddItemToObject(config,"serverport",cJSON_CreateNumber(sipattr.serverport));
		cJSON_AddItemToObject(config,"logindomain",cJSON_CreateString(sipattr.logindomain));
		cJSON_AddItemToObject(config,"deviceid",cJSON_CreateString(sipattr.deviceid));
		cJSON_AddItemToObject(config,"localport",cJSON_CreateNumber(sipattr.localport));
		cJSON_AddItemToObject(config,"authpasswd",cJSON_CreateString(sipattr.authpasswd));
		cJSON_AddItemToObject(config,"authmode",cJSON_CreateNumber(sipattr.authmode));
		cJSON_AddItemToObject(config,"transtype",cJSON_CreateNumber(sipattr.transtype));
		cJSON_AddItemToObject(config,"expires",cJSON_CreateNumber(sipattr.expires));
		cJSON_AddItemToObject(config,"heartbeatinterval",cJSON_CreateNumber(sipattr.hbinterval));
		cJSON_AddItemToObject(config,"heartbeatcount",cJSON_CreateNumber(sipattr.hbcount));
		cJSON_AddItemToObject(config,"mediaport",cJSON_CreateNumber(sipattr.mediaport));
	} else {
		config_member = config->child;
		while (config_member) {
			if (strcmp(config_member->string,"sipswitch") == 0) {
				tag = config_member->prev;
				cJSON_ReplaceItemInObject(config_member,"sipswitch",cJSON_CreateNumber(sipattr.sipswitch));
				config_member = tag->next;
			} else if (strcmp(config_member->string,"serverid") == 0) {
				tag = config_member->prev;
				cJSON_ReplaceItemInObject(config_member,"serverid",cJSON_CreateString(sipattr.serverid));
				config_member = tag->next;
			} else if(strcmp(config_member->string,"serverip") == 0) {
				tag = config_member->prev;
				cJSON_ReplaceItemInObject(config_member,"serverip",cJSON_CreateString(sipattr.serverip));
				config_member = tag->next;
			} else if(strcmp(config_member->string,"serverport") == 0) {
				tag = config_member->prev;
				cJSON_ReplaceItemInObject(config_member,"serverport",cJSON_CreateNumber(sipattr.serverport));
				config_member = tag->next;
			} else if(strcmp(config_member->string,"logindomain") == 0) {
				tag = config_member->prev;
				cJSON_ReplaceItemInObject(config_member,"logindomain",cJSON_CreateString(sipattr.logindomain));
				config_member = tag->next;
			} else if(strcmp(config_member->string,"deviceid") == 0) {
				tag = config_member->prev;
				cJSON_ReplaceItemInObject(config_member,"deviceid",cJSON_CreateString(sipattr.deviceid));
				config_member = tag->next;
			} else if(strcmp(config_member->string,"localport") == 0) {
				tag = config_member->prev;
				cJSON_ReplaceItemInObject(config_member,"localport",cJSON_CreateNumber(sipattr.localport));
				config_member = tag->next;
			} else if(strcmp(config_member->string,"authpasswd") == 0) {
				tag = config_member->prev;
				cJSON_ReplaceItemInObject(config_member,"authpasswd",cJSON_CreateString(sipattr.authpasswd));
				config_member = tag->next;
			} else if(strcmp(config_member->string,"authmode") == 0) {
				tag = config_member->prev;
				cJSON_ReplaceItemInObject(config_member,"authmode",cJSON_CreateNumber(sipattr.authmode));
				config_member = tag->next;
			} else if(strcmp(config_member->string,"transtype") == 0) {
				tag = config_member->prev;
				cJSON_ReplaceItemInObject(config_member,"transtype",cJSON_CreateNumber(sipattr.transtype));
				config_member = tag->next;
			} else if(strcmp(config_member->string,"expires") == 0) {
				tag = config_member->prev;
				cJSON_ReplaceItemInObject(config_member,"expires",cJSON_CreateNumber(sipattr.expires));
				config_member = tag->next;
			} else if(strcmp(config_member->string,"heartbeatinterval") == 0) {
				tag = config_member->prev;
				cJSON_ReplaceItemInObject(config_member,"heartbeatinterval",cJSON_CreateNumber(sipattr.hbinterval));
				config_member = tag->next;
			} else if(strcmp(config_member->string,"heartbeatcount") == 0) {
				tag = config_member->prev;
				cJSON_ReplaceItemInObject(config_member,"heartbeatcount",cJSON_CreateNumber(sipattr.hbcount));
				config_member = tag->next;
			} else if(strcmp(config_member->string,"mediaport") == 0) {
				tag = config_member->prev;
				cJSON_ReplaceItemInObject(config_member,"mediaport",cJSON_CreateNumber(sipattr.mediaport));
				config_member = tag->next;
			}
			config_member = config_member->next;
		}
	}
	cJSON_Delete(sip_root);
	return XCAM_SUCCESS;
}

int xcam_json_set_sip_config(void *json_root)
{
	int ret = XCAM_SUCCESS;
	struct svac_device_sip_attr sipattr;
	cJSON *root = NULL,  *config = NULL, *config_member = NULL;
	root = (cJSON *)json_root;
	if (json_root == NULL) {
		LOG_ERR(LOG_TAG,"err(%s,%d): json root is NULL\n", __func__, __LINE__);
		return XCAM_ERROR;
	}
	memset(&sipattr,0,sizeof(struct svac_device_sip_attr));
	config = cJSON_GetObjectItem(root, "sip.config");
	if (config == NULL) {
		LOG_ERR(LOG_TAG,"err(%s,%d): json file has no this config\n", __func__, __LINE__);
		return XCAM_ERROR;
	}
#if 0
	char *json_str = cJSON_Print(root);
	printf("%s-%d\n %s\n",__func__,__LINE__,json_str);
#endif

	config_member = config->child;
	while (config_member) {
		if(strcmp(config_member->string,"sipswitch")== 0){
			sipattr.sipswitch = config_member->valueint;
		}else if(strcmp(config_member->string,"serverid") == 0){
			sipattr.serverid = config_member->valuestring;
		}else if(strcmp(config_member->string,"serverip") == 0){
			sipattr.serverip = config_member->valuestring;
		}else if(strcmp(config_member->string,"serverport") == 0){
			sipattr.serverport = config_member->valueint;
		}else if(strcmp(config_member->string,"logindomain") == 0){
			sipattr.logindomain = config_member->valuestring;
		}else if(strcmp(config_member->string,"deviceid") == 0){
			sipattr.deviceid = config_member->valuestring;
		}else if(strcmp(config_member->string,"localport") == 0){
			sipattr.localport = config_member->valueint;
		}else if(strcmp(config_member->string,"authpasswd") == 0){
			sipattr.authpasswd = config_member->valuestring;
		}else if(strcmp(config_member->string,"authmode") == 0){
			sipattr.authmode = config_member->valueint;
		}else if(strcmp(config_member->string,"transtype") == 0){
			sipattr.transtype = config_member->valueint;
		}else if(strcmp(config_member->string,"expires") == 0){
			sipattr.expires = config_member->valueint;
		}else if(strcmp(config_member->string,"heartbeatinterval") == 0){
			sipattr.hbinterval = config_member->valueint;
		}else if(strcmp(config_member->string,"heartbeatcount") == 0){
			sipattr.hbcount = config_member->valueint;
		}else if(strcmp(config_member->string,"mediaport") == 0){
			sipattr.mediaport = config_member->valueint;
		}else;
		config_member = config_member->next;
	}
	ret = _set_sip_config(&sipattr);
	if(ret != XCAM_SUCCESS)
		return ret;
	if(sipattr.sipswitch)
		return gb_manage_module_restart();
	else
		return gb_manage_module_stop();
}

