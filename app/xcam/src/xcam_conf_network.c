#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <imp/imp_common.h>
#include <imp/imp_encoder.h>
#include "xcam_log.h"
#include "cJSON.h"
#include "xcam_conf_video.h"
#include "xcam_conf_json.h"
#include "xcam_general.h"
#include "xcam_network.h"
#include "xcam_conf_network.h"
#include "xcam_cli_options.h"

#define CONFIG_NETWORK_FILE "/system/etc/network.json"
#define LOG_TAG "NetworkJsonConf"
#define BUF_SIZE 1024

network_config_t xcam_network_config;
bool network_config_has_read = false;

int xcam_conf_set_ip_addr(char *addr)
{
	assert(addr != NULL);
	int ret = XCAM_SUCCESS;
	cJSON *root = NULL, *root_child = NULL, *item = NULL;
	root = json_read_file(CONFIG_NETWORK_FILE);
	if (root == NULL) {
		LOG_ERR(LOG_TAG,"error(%s,%d): json read file\n",__func__, __LINE__);
		return XCAM_ERROR;
	}

	root_child = cJSON_GetObjectItem(root, "network.ip.config");
	if (root_child) {
		item = cJSON_GetObjectItem(root_child, "addr");
		if (item) {
			cJSON_ReplaceItemInObject(root_child, "addr", cJSON_CreateString(addr));
		} else {
			cJSON_AddItemToObject(root_child, "addr", cJSON_CreateString(addr));
		}
	} else {
		LOG_ERR(LOG_TAG,"error(%s,%d),get ip config fail,config file error.\n",__func__,__LINE__);
		return XCAM_ERROR;
	}

	xcam_network_config.ip_addr = inet_addr(addr);

	ret = json_write_file(CONFIG_NETWORK_FILE,root);
	if (ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG,"errno(%s,%d),Call json_write_file fail.\n",__func__,__LINE__);
	}

	cJSON_Delete(root);

	return ret;
}

int xcam_conf_set_mask(char *mask)
{
	assert(mask != NULL);
	int ret = XCAM_SUCCESS;
	cJSON *root = NULL, *root_child = NULL, *item = NULL;
	root = json_read_file(CONFIG_NETWORK_FILE);
	if (root == NULL) {
		LOG_ERR(LOG_TAG,"error(%s,%d): json read file\n",__func__, __LINE__);
		return XCAM_ERROR;
	}

	root_child = cJSON_GetObjectItem(root, "network.ip.config");
	if (root_child) {
		item = cJSON_GetObjectItem(root_child, "mask");
		if (item) {
			cJSON_ReplaceItemInObject(root_child, "mask", cJSON_CreateString(mask));
		} else {
			cJSON_AddItemToObject(root_child, "mask", cJSON_CreateString(mask));
		}
	} else {
		LOG_ERR(LOG_TAG,"error(%s,%d),get ip config fail,config file error.\n", __func__, __LINE__);
		return XCAM_ERROR;
	}

	xcam_network_config.mask = inet_addr(mask);

	ret = json_write_file(CONFIG_NETWORK_FILE,root);
	if (ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG,"errno(%s,%d),Call json_write_file fail.\n",__func__,__LINE__);
	}

	cJSON_Delete(root);

	return ret;
}

int xcam_conf_set_gateway(char *gateway)
{
	assert(gateway != NULL);
	int ret = XCAM_SUCCESS;
	cJSON *root = NULL, *root_child = NULL, *item = NULL;
	root = json_read_file(CONFIG_NETWORK_FILE);
	if (root == NULL) {
		LOG_ERR(LOG_TAG,"error(%s,%d): json read file\n",__func__, __LINE__);
		return XCAM_ERROR;
	}

	root_child = cJSON_GetObjectItem(root, "network.ip.config");
	if (root_child) {
		item = cJSON_GetObjectItem(root_child, "gateway");
		if (item) {
			cJSON_ReplaceItemInObject(root_child, "gateway", cJSON_CreateString(gateway));
		} else {
			cJSON_AddItemToObject(root_child, "gateway", cJSON_CreateString(gateway));
		}
	} else {
		LOG_ERR(LOG_TAG,"error(%s,%d),get ip config fail,config file error.\n", __func__, __LINE__);
		return XCAM_ERROR;
	}

	xcam_network_config.gateway = inet_addr(gateway);

	ret = json_write_file(CONFIG_NETWORK_FILE,root);
	if (ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG,"errno(%s,%d),Call json_write_file fail.\n",__func__,__LINE__);
	}

	cJSON_Delete(root);

	return ret;
}

int xcam_conf_set_auto(bool *IsDhcpEnable)
{
	assert(IsDhcpEnable != NULL);
	int ret = XCAM_SUCCESS;
	cJSON *root = NULL, *root_child = NULL, *item = NULL;
	root = json_read_file(CONFIG_NETWORK_FILE);
	if (root == NULL) {
		LOG_ERR(LOG_TAG,"error(%s,%d): json read file\n",__func__, __LINE__);
		return XCAM_ERROR;
	}

	root_child = cJSON_GetObjectItem(root, "network.ip.config");
	if (root_child) {
		item = cJSON_GetObjectItem(root_child, "auto");
		if (item) {
			cJSON_ReplaceItemInObject(root_child, "auto", cJSON_CreateNumber(*IsDhcpEnable));
		} else {
			cJSON_AddItemToObject(root_child, "auto", cJSON_CreateNumber(*IsDhcpEnable));
		}
	} else {
		LOG_ERR(LOG_TAG,"error(%s,%d),get ip config fail,config file error.\n", __func__, __LINE__);
		return XCAM_ERROR;
	}

	if ( *IsDhcpEnable == true )
		xcam_network_config.mode = NETWORK_DHCP;
	else
		xcam_network_config.mode = NETWORK_STATIC;

	ret = json_write_file(CONFIG_NETWORK_FILE,root);
	if (ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG,"errno(%s,%d),Call json_write_file fail.\n",__func__,__LINE__);
	}

	cJSON_Delete(root);

	return ret;
}

//network_config_t xcam_network_config;
int xcam_conf_write_network_config(network_config_t *pstconfig)
{
	int ret = XCAM_SUCCESS;
	cJSON *root = NULL, *root_child = NULL, *item = NULL;
	struct in_addr addr;
	struct in_addr mask;
	struct in_addr gateway;
	struct in_addr DNSaddr;

	assert(pstconfig != NULL);
	memset(&addr,0,sizeof(struct in_addr));
	memset(&mask,0,sizeof(struct in_addr));
	memset(&gateway,0,sizeof(struct in_addr));
	memset(&DNSaddr,0,sizeof(struct in_addr));

	memcpy(&addr,&pstconfig->ip_addr,4);
	memcpy(&mask,&pstconfig->mask,4);
	memcpy(&gateway,&pstconfig->gateway,4);
	memcpy(&DNSaddr,&pstconfig->DNSaddr,4);
	root = json_read_file(CONFIG_NETWORK_FILE);
	if (root == NULL) {
		LOG_ERR(LOG_TAG,"error(%s,%d): json read file\n",__func__, __LINE__);
		return XCAM_ERROR;
	}

	root_child = cJSON_GetObjectItem(root, "network.ip.config");
	if (root_child) {
		item = cJSON_GetObjectItem(root_child, "auto");
		if (item) {
			cJSON_ReplaceItemInObject(root_child, "auto", cJSON_CreateNumber(pstconfig->mode));
		} else {
			cJSON_AddItemToObject(root_child, "auto", cJSON_CreateNumber(pstconfig->mode));
		}

		item = cJSON_GetObjectItem(root_child, "flag");
		if (item) {
			cJSON_ReplaceItemInObject(root_child, "flag", cJSON_CreateNumber(pstconfig->net_flag));
		} else {
			cJSON_AddItemToObject(root_child, "flag", cJSON_CreateNumber(pstconfig->net_flag));
		}

		item = cJSON_GetObjectItem(root_child, "addr");
		if (item) {
			cJSON_ReplaceItemInObject(root_child, "addr", cJSON_CreateString(inet_ntoa(addr)));
		} else {
			cJSON_AddItemToObject(root_child, "addr", cJSON_CreateString(inet_ntoa(addr)));
		}

		item = cJSON_GetObjectItem(root_child,"mask");
		if (item) {
			cJSON_ReplaceItemInObject(root_child, "mask", cJSON_CreateString(inet_ntoa(mask)));
		} else {
			cJSON_AddItemToObject(root_child, "mask", cJSON_CreateString(inet_ntoa(mask)));
		}

		item = cJSON_GetObjectItem(root_child,"gateway");
		if (item) {
			cJSON_ReplaceItemInObject(root_child, "gateway", cJSON_CreateString(inet_ntoa(gateway)));
		} else {
			cJSON_AddItemToObject(root_child, "gateway", cJSON_CreateString(inet_ntoa(gateway)));
		}

		item = cJSON_GetObjectItem(root_child,"DNSaddr");
		if (item) {
			cJSON_ReplaceItemInObject(root_child, "DNSaddr", cJSON_CreateString(inet_ntoa(DNSaddr)));
		} else {
			cJSON_AddItemToObject(root_child, "DNSaddr", cJSON_CreateString(inet_ntoa(DNSaddr)));
		}
	} else {
		root_child = cJSON_CreateObject();
		cJSON_AddItemToObject(root_child, "auto", cJSON_CreateNumber(pstconfig->mode));
		cJSON_AddItemToObject(root_child, "flag", cJSON_CreateNumber(pstconfig->net_flag));
		cJSON_AddItemToObject(root_child, "addr", cJSON_CreateString(inet_ntoa(addr)));
		cJSON_AddItemToObject(root_child, "mask", cJSON_CreateString(inet_ntoa(mask)));
		cJSON_AddItemToObject(root_child, "gateway", cJSON_CreateString(inet_ntoa(gateway)));
		cJSON_AddItemToObject(root_child, "DNSaddr", cJSON_CreateString(inet_ntoa(DNSaddr)));
		cJSON_AddItemToObject(root,"network.ip.config",root_child);
	}

	ret = json_write_file(CONFIG_NETWORK_FILE,root);
	if (ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG,"errno(%s,%d),Call json_write_file fail.\n",__func__,__LINE__);
	}

	cJSON_Delete(root);
	return ret;
}

int xcam_conf_read_network_config(network_config_t *pstconfig)
{
	cJSON *root = NULL, *root_child = NULL, *item = NULL;

	assert(pstconfig != NULL);
	root = json_read_file(CONFIG_NETWORK_FILE);
	if (root == NULL) {
		LOG_ERR(LOG_TAG,"error(%s,%d): json read file\n",__func__, __LINE__);
		return XCAM_ERROR;
	}

	root_child = cJSON_GetObjectItem(root, "network.ip.config");
	if (root_child) {
		item = cJSON_GetObjectItem(root_child, "auto");
		if (item) {
			pstconfig->mode = item->valueint;
		} else {
			LOG_INF(LOG_TAG,"error(%s,%d),get config fail.\n",__func__,__LINE__);
			return XCAM_ERROR;
		}

		item = cJSON_GetObjectItem(root_child, "flag");
		if (item) {
			pstconfig->net_flag = item->valueint;
		} else {
			LOG_INF(LOG_TAG,"error(%s,%d),get config fail.\n",__func__,__LINE__);
			return XCAM_ERROR;
		}

		item = cJSON_GetObjectItem(root_child, "addr");
		if (item ) {
			pstconfig->ip_addr = inet_addr(item->valuestring);
		} else {
			LOG_INF(LOG_TAG,"error(%s,%d),get config fail.\n",__func__,__LINE__);
			return XCAM_ERROR;
		}

		item = cJSON_GetObjectItem(root_child, "mask");
		if (item ) {
			pstconfig->mask = inet_addr(item->valuestring);
		} else {
			LOG_INF(LOG_TAG,"error(%s,%d),get config fail.\n",__func__,__LINE__);
			return XCAM_ERROR;
		}

		item = cJSON_GetObjectItem(root_child, "gateway");
		if (item ) {
			pstconfig->gateway = inet_addr(item->valuestring);
		} else {
			LOG_INF(LOG_TAG,"error(%s,%d),get config fail.\n",__func__,__LINE__);
			return XCAM_ERROR;
		}

		item = cJSON_GetObjectItem(root_child, "DNSaddr");
		if (item ) {
			pstconfig->DNSaddr = inet_addr(item->valuestring);
		} else {
			LOG_INF(LOG_TAG,"error(%s,%d),get config fail.\n",__func__,__LINE__);
			return XCAM_ERROR;
		}

	} else {
		LOG_ERR(LOG_TAG,"error(%s,%d),config file non-existent this config.\n");
		return XCAM_ERROR;
	}

	return XCAM_SUCCESS;
}

void _xcam_conf_network_init(network_config_t *pstconfig)
{
	int ret = XCAM_SUCCESS;
	char ip[20] = {'\0'};
	char mask[20] = {'\0'};
	char gateway[20] = {'\0'};
	char DNSaddr[20] = {'\0'};

	ret |= xcam_network_get_device_ip("eth0",ip);
	ret |= xcam_network_get_device_ip_mask("eth0", mask);
	ret |= xcam_network_get_device_ip_gateway("eth0", gateway);
	ret |= xcam_network_get_device_DNS_server_addr(DNSaddr);
	if (ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG,"error(%s,%d):Call xcam_network_get_device_ip fail.\n",__func__,__LINE__);
		return;
	}

	//其实这个地方加上一个判断是最好的。判断到底是静态的还是动态的
	xcam_network_config.mode = NETWORK_STATIC;
	xcam_network_config.net_flag = 	XCAM_IPV4;
	xcam_network_config.ip_addr = inet_addr(ip);
	xcam_network_config.mask = inet_addr(mask);
	xcam_network_config.gateway = inet_addr(gateway);
	xcam_network_config.DNSaddr = inet_addr(DNSaddr);

	return;
}

void xcam_conf_network_init()
{
	int ret = XCAM_SUCCESS;
	FILE *file = NULL;
	cJSON *root = NULL;
	char *json_str = NULL;

	memset(&xcam_network_config, 0, sizeof(network_config_t));
	if (cli_attr.no_operation_file_flag == true)
		return;

	file = fopen(CONFIG_NETWORK_FILE,"r");
	if (file == NULL) {
		//配置文件不存在情况
		root = cJSON_CreateObject();
		json_str = cJSON_Print(root);
		file = fopen(CONFIG_NETWORK_FILE,"w+");
		if (file == NULL) {
			cJSON_Delete(root);
			free(json_str);
			LOG_ERR(LOG_TAG,"error(%s,%d):Init network configuration file fail.\n",__func__,__LINE__);
			return;
		}

		fprintf(file,"%s",json_str);
		free(json_str);
		cJSON_Delete(root);
		root = NULL;
		fclose(file);

		_xcam_conf_network_init(&xcam_network_config);
		ret = xcam_conf_write_network_config(&xcam_network_config);
		if (ret < 0) {
			LOG_ERR(LOG_TAG,"error(%s,%d):Call xcam_conf_set_network fail.\n");
		}

		return;
	} else {
		ret = xcam_conf_read_network_config(&xcam_network_config);
		if (ret < XCAM_SUCCESS) {
			LOG_ERR(LOG_TAG,"error(%s.%d),call xcam_conf_get_network fail.\n");
		}
	}

	fclose(file);
	return ;
}
