#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#include "../cJSON/cJSON.h"
#include "../daemon/xcam_daemon_protocol.h"
#include "xcam_json_process.h"

int xcam_socket_set_configs_process(socket_tcp_link_t *plink);
int xcam_socket_get_configs_process(socket_tcp_link_t *plink);

int msg_process_func(socket_tcp_link_t *plink)
{
#if 0
	printf("msg recv: cmd = 0x%08x, len = 0x%08x, action = 0x%08x\n",
			msg_header->cmd, msg_header->len, msg_header->action);
#endif

	int ret = 0;
	switch(plink->msgbuf->header.cmd) {
		case SET_CONFIGS_SEND:
			ret = xcam_socket_set_configs_process(plink);
			if (ret < 0) {
				printf("err(%s,%d): set configs err\n", __func__, __LINE__);
				return -1;
			}
			break;
		case GET_CONFIGS_SEND:
			ret = xcam_socket_get_configs_process(plink);
			if (ret < 0) {
				printf("err(%s,%d): get configs err\n", __func__, __LINE__);
				return -1;
			}
			break;
		default:
			printf("err(%s,%d): cmd not support\n", __func__, __LINE__);
			return -1;
			break;
	}
	return 0;
}

int main(int argc, char *argv[])
{
	int ret;
	void *skt;
	skt = socket_server_tcp_alloc(51000, 5);
	assert(NULL != skt);
	ret = socket_server_tcp_set_msg_process_cb(skt, msg_process_func);
	assert(0 == ret);
	ret = socket_server_tcp_start(skt);
	assert(0 == ret);
	while (1)
		sleep(1);
	return 0;
}

int xcam_socket_set_configs_process(socket_tcp_link_t *plink)
{
	skt_msg_header_t *header = (skt_msg_header_t *)plink->msgbuf;
	char *data = plink->msgbuf->data;
	char *json_str = NULL;
	int ret;

	if (data == NULL) {
		printf("err(%s,%d): recv data is NULL\n", __func__, __LINE__);
		return -1;
	}
#if 1
	printf("%s\n", data);
#endif
	cJSON *root_r = cJSON_Parse(data);
	cJSON *root_s = cJSON_CreateObject();
	cJSON *ret_s = NULL;

	if (cJSON_GetObjectItem(root_r, "video.fps") != NULL) {
		ret = xcam_json_set_video_fps(root_r);
	}

	if (ret != 0) {
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
		printf("err(%s,%d): server send error\n", __func__, __LINE__);
		return -1;
	}
	free(json_str);
	free(buf);
	return 0;
}

int xcam_socket_get_configs_process(socket_tcp_link_t *plink)
{
	skt_msg_header_t *header = (skt_msg_header_t *)plink->msgbuf;
	char *data = plink->msgbuf->data;
	char *json_str = NULL;
	int ret, i;

	if (data == NULL) {
		printf("err(%s,%d): recv data is NULL\n", __func__, __LINE__);
		return -1;
	}
	cJSON *root = cJSON_Parse(data);
	cJSON *root_err = cJSON_CreateObject();
	cJSON *cfglist = cJSON_GetObjectItem(root, "configlist");
	cJSON *member = NULL;
	cJSON *ret_s = NULL;

	int array_size = cJSON_GetArraySize(cfglist);
	printf("array size %d\n", array_size);
	if (!array_size) {
		ret = xcam_json_get_video_fps(root);
	} else {
		for (i = 0; i < array_size; i ++) {
			member = cJSON_GetArrayItem(cfglist, i);
			if (strcmp(member->valuestring, "\"video.fps\"")) {
				ret = xcam_json_get_video_fps(root);
			}
		}
	}

	if (ret != 0) {
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
		printf("err(%s,%d): server send error\n", __func__, __LINE__);
		return -1;
	}
	free(json_str);
	free(buf);
	return 0;
}
