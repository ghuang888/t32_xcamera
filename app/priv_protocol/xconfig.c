#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../daemon/xcam_daemon_protocol.h"
#include "../cJSON/cJSON.h"

typedef struct msg_data_request_configs_s{
	skt_msg_header_t header;
	char data[2000];
} msg_data_request_configs_t;

typedef struct msg_data_ack_configs_s{
	skt_msg_header_t header;
	char data[2000];
} msg_data_ack_configs_t;

msg_data_request_configs_t msg_data_request_configs;
msg_data_ack_configs_t msg_data_ack_configs;

int main(int argc, char *argv[])
{
	int ret = 0, i;
	void *cskt;
	char command[100];
	char cmd[10];
	char config[20];
	int param0 = -1;
	int param1 = -1;
	if (argc != 3) {
		printf("Usage: ./client [server_ip] [port]\n");
		return -1;
	}
	cskt = socket_client_tcp_alloc(argv[1], atoi(argv[2]));
	assert(cskt != NULL);

	skt_msg_t *msg_send = (skt_msg_t *)&msg_data_request_configs;
	skt_msg_t *msg_recv = (skt_msg_t *)&msg_data_ack_configs;

//get configlist start
	printf("Geting xcamera config list from ip:%s port:%d\n", argv[1], atoi(argv[2]));
	//send empty configlist array:	{"configlist":[]}
	cJSON *root_s = cJSON_CreateObject();
	cJSON *cfglist_s = cJSON_CreateArray();
	cJSON *config_s = NULL;
	cJSON *member_s = NULL;
	cJSON_AddItemToObject(root_s, "configlist", cfglist_s);
	char *json_str = cJSON_Print(root_s);
	int json_len = strlen(json_str);
	//printf("strlen of send json is %d\n%s\n\n", json_len, json_str);

	memset(msg_send->data, 0, sizeof(msg_data_request_configs.data));
	memcpy(msg_send->data, json_str, json_len);
	msg_send->header.cmd = GET_CONFIGS_SEND;
	msg_send->header.flags = MSG_FLAGS_DATA_ENDING;
	msg_send->header.len = json_len;
	cJSON_Delete(root_s);
	root_s = NULL;
	free(json_str);

	ret = socket_client_tcp_send(cskt, msg_send);
	if (ret < 0) {
		printf("err(%s,%d): send error\n", __func__, __LINE__);
		return -1;
	}

	ret = socket_client_tcp_recv(cskt, msg_recv, sizeof(msg_data_ack_configs.data));
	if (ret < 0 || msg_recv->header.cmd != GET_CONFIGS_ACK) {
		printf("err(%s,%d): get configlist ack error\n", __func__, __LINE__);
		return -1;
	}
	printf("%s\n", msg_data_ack_configs.data);
//get configlist end

	cJSON *root_r = cJSON_Parse(msg_data_ack_configs.data);
	cJSON *cfglist_r = cJSON_GetObjectItem(root_r, "configlist");
	cJSON *member = NULL;

	while (1) {
get_cmd:
		printf("\n***********************************\n");
		printf("All the configs server support:\n");
		member = cfglist_r->child;
		while (member) {
			printf("%s ", member->valuestring);
			member = member->next;
		}
		if (root_s != NULL)
			cJSON_Delete(root_s);
		root_s = cJSON_CreateObject();
		cfglist_s = cJSON_CreateArray();
		cJSON_AddItemToObject(root_s, "configlist", cfglist_s);
		printf("\nplease input command:\n");
		param0 = -1;
		param1 = -1;
		memset(command, 0, sizeof(command));
		memset(cmd, 0, sizeof(cmd));
		memset(config, 0, sizeof(config));
		i = 0;
		while ((command[i] = getchar())!='\n') {
			i ++;
			continue;
		}
		ret = sscanf(command, "%s %s %d %d", cmd, config, &param0, &param1);
		printf("ret = %d\n", ret);
		if ((ret == 1) && strcmp(cmd, "quit") == 0)
			break;
		else if (strcmp(cmd, "get") == 0)
			msg_send->header.cmd = GET_CONFIGS_SEND;
		else if (strcmp(cmd, "set") == 0)
			msg_send->header.cmd = SET_CONFIGS_SEND;
		else {
			printf("no such cmd, please input again\n");
			goto get_cmd;
		}

		member = cfglist_r->child;
		while (member) {
			if (strcmp(config, member->valuestring) == 0)
				break;
			if ((member = member->next) == NULL) {
				printf("no such config, please input again\n");
				goto get_cmd;
			}
		}

		if (strcmp(config, "video.fps") == 0) {
			cJSON_AddItemToArray(cfglist_s, cJSON_CreateString("video.fps"));
			if (strcmp(cmd, "get") == 0 && ret > 2) {
				config_s = cJSON_CreateArray();
				member_s = cJSON_CreateObject();
				cJSON_AddItemToObject(root_s, "video.fps", config_s);
				cJSON_AddItemToObject(member_s, "channal", cJSON_CreateNumber(param0));
				cJSON_AddItemToArray(config_s, member_s);
			} else if (strcmp(cmd, "set") == 0 && ret > 3) {
				config_s = cJSON_CreateArray();
				member_s = cJSON_CreateObject();
				cJSON_AddItemToObject(root_s, "video.fps", config_s);
				cJSON_AddItemToObject(member_s, "channal", cJSON_CreateNumber(param0));
				cJSON_AddItemToObject(member_s, "fps", cJSON_CreateNumber(param1));
				cJSON_AddItemToArray(config_s, member_s);
			}
		}

		json_str = cJSON_Print(root_s);
		json_len = strlen(json_str);
		//printf("strlen of send json is %d\n%s\n\n", json_len, json_str);

		memset(msg_send->data, 0, sizeof(msg_data_request_configs.data));
		memcpy(msg_send->data, json_str, json_len);
		msg_send->header.flags = MSG_FLAGS_DATA_ENDING;
		msg_send->header.len = json_len;
		free(json_str);

		ret = socket_client_tcp_send(cskt, msg_send);
		if (ret < 0) {
			printf("err(%s,%d): send error\n", __func__, __LINE__);
			return -1;
		}

		memset(msg_recv->data, 0, sizeof(msg_data_ack_configs.data));
		ret = socket_client_tcp_recv(cskt, msg_recv, sizeof(msg_data_ack_configs.data));
		if (ret < 0) {
			printf("err(%s,%d): get configlist ack error\n", __func__, __LINE__);
			return -1;
		}
		printf("%s\n", msg_recv->data);
	}

	close(((socket_client_tcp_t *)cskt)->skt_client);
	return 0;
}
