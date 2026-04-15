#include <stdio.h>
#include <stdlib.h>

#include "../cJSON/cJSON.h"
#include "xcam_json_process.h"

int xcam_json_get_video_fps(void *json_root)
{
	if (json_root == NULL) {
		printf("err(%s,%d): json root is NULL\n", __func__, __LINE__);
		return -1;
	}

	int ch_num = -1, array_size = -1;
	cJSON *root = (cJSON *)json_root;
	cJSON *_cfg = NULL;
	cJSON *config = cJSON_GetObjectItem(root, "video.fps");
	cJSON *member = NULL;
	cJSON *part = NULL;
	cJSON *ch = NULL;

	cJSON *cfglist = cJSON_GetObjectItem(root, "configlist");
	if (!cfglist) {
		printf("err(%s,%d): json has no configlist\n", __func__, __LINE__);
		return -1;
	}
	_cfg = cfglist->child;
	if (!_cfg)
		cJSON_AddItemToArray(cfglist, cJSON_CreateString("video.fps"));
	else {
		while (_cfg) {
			if (strcmp(_cfg->valuestring, "video.fps") == 0)
				break;
			if ((_cfg = _cfg->next) == NULL)
				cJSON_AddItemToArray(cfglist, cJSON_CreateString("video.fps"));
		}
	}

	if (config == NULL) {
		config = cJSON_CreateArray();
		cJSON_AddItemToObject(root, "video.fps", config);

		ch = cJSON_CreateObject();
		cJSON_AddItemToObject(ch, "channal", cJSON_CreateNumber(0));
		cJSON_AddItemToObject(ch, "fps", cJSON_CreateNumber(30));
		cJSON_AddItemToArray(config, ch);

		ch = cJSON_CreateObject();
		cJSON_AddItemToObject(ch, "channal", cJSON_CreateNumber(1));
		cJSON_AddItemToObject(ch, "fps", cJSON_CreateNumber(20));
		cJSON_AddItemToArray(config, ch);
	} else {
		member = config->child;
		while (member) {
			part = member->child;
			while (part) {
				if (strcmp(part->string, "channal") == 0) {
					ch_num = part->valueint;
					printf("get channal%d fps\n", ch_num);
					//sdk_get_fps();
					if (ch_num == 0)
						cJSON_AddItemToObject(member, "fps", cJSON_CreateNumber(30));
					else if (ch_num == 1)
						cJSON_AddItemToObject(member, "fps", cJSON_CreateNumber(20));
				}
				part = part->next;
			}
			member = member->next;
		}
	}
	return 0;
}

int xcam_json_set_video_fps(void *json_root)
{
	if (json_root == NULL) {
		printf("err(%s,%d): json root is NULL\n", __func__, __LINE__);
		return -1;
	}

	int ch_num = -1, fps = -1;
	cJSON *root = (cJSON *)json_root;
	cJSON *config = NULL;
	cJSON *member = NULL;
	cJSON *part = NULL;

	config = cJSON_GetObjectItem(root, "video.fps");
	if (config == NULL) {
		printf("err(%s,%d): json file has no this config\n", __func__, __LINE__);
		return -1;
	}
	member = config->child;
	while (member) {
		part = member->child;
		while (part) {
			if (strcmp(part->string, "channal") == 0)
				ch_num = part->valueint;
			else if (strcmp(part->string, "fps") == 0)
				fps = part->valueint;
			part = part->next;
		}
		printf("ch:%d fps:%d\n", ch_num, fps);
		if (ch_num == -1 || fps < 5 || fps > 25) {
			printf("err(%s,%d): ch_num or fps is not right\n", __func__, __LINE__);
			return -1;
		}
		//sdk_set_fps(ch_num, fps);
		member = member->next;
	}

	return 0;
}
