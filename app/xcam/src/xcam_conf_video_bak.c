#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#include "xcam_conf_json.h"
#include "xcam_conf_video.h"

#define CONFIG_VIDEO_FILE "/system/video.json"

#ifdef TEST_XCAM_CONF_VIDEO
#undef CONFIG_VIDEO_FILE
#define CONFIG_VIDEO_FILE "video.json"
#endif

static conf_video_t video_config;
static bool video_config_has_read = false;

static int video_parse_fps(cJSON *item, conf_video_t *video_conf)
{
	int ret = 0;
	int i = 0;
	int array_num = 0;
	//printf("type : %d\n", item->type);
	//printf("num : %d\n", cJSON_GetArraySize(item));
	//support 2 channels
	array_num = cJSON_GetArraySize(item);
	if ((item->type != cJSON_Array)&&(array_num != 2)) {
		printf("error(%s,%d): item type %d\n",
				__func__, __LINE__, item->type);
		return -1;
	}
	for (i = 0; i < array_num; i++) {
		cJSON *item_ch, *item_fps_num, *item_fps_den;
		item_ch = cJSON_GetObjectItem(cJSON_GetArrayItem(item, i), "ch");
		item_fps_num = cJSON_GetObjectItem(cJSON_GetArrayItem(item, i), "fps_num");
		item_fps_den = cJSON_GetObjectItem(cJSON_GetArrayItem(item, i), "fps_den");
		if ((item_ch == NULL)||(item_fps_num == NULL)||(item_fps_den == NULL)) {
			printf("error(%s,%d): video fps config error\n",
					__func__, __LINE__);
			return -1;
		}
		int ch = item_ch->valueint;
		int fps_num = item_fps_num->valueint;
		int fps_den = item_fps_den->valueint;
		if ((ch >= 0)&&(ch << 1)) {
			video_conf->video_fps.ch[ch].fps_num = fps_num;
			video_conf->video_fps.ch[ch].fps_den = fps_den;
		}
	}
	return 0;
}

static int xcam_conf_read_video(conf_video_t *video_conf)
{
	int ret = -1;
	int i = 0;
	cJSON *root, *item;
	root = json_read_file(CONFIG_VIDEO_FILE);
	if (NULL == root)  {
		printf("error(%s,%d): json read file\n",
				__func__, __LINE__);
		return -1;
	}
	for (i = 0; i < cJSON_GetArraySize(root); i++) {
		item = cJSON_GetArrayItem(root, i);
		if (0 == strcmp(item->string, "video.fps")) {
			ret = video_parse_fps(item, video_conf);
			if (0 != ret) {
				printf("error(%s,%d): video parse fps\n",
						__func__, __LINE__);
			}
		} else {
			char *str_item = cJSON_Print(item);
			printf("unknown item: %s\n", str_item);
			free(str_item);
		}
	}
	return 0;
}

static int xcam_conf_write_video(conf_video_t *video_conf)
{
	int ret = -1;
	int i = 0;
	cJSON *root, *array, *item;

	root = cJSON_CreateObject();
	array = cJSON_CreateArray();
	cJSON_AddItemToObject(root, "video_fps", array);
	for (i = 0; i < 2; i++) {
		item = cJSON_CreateObject();
		cJSON_AddItemToObject(item, "ch", cJSON_CreateNumber(i));
		cJSON_AddItemToObject(item, "fps_num", cJSON_CreateNumber(video_conf->video_fps.ch[i].fps_num));
		cJSON_AddItemToObject(item, "fps_den", cJSON_CreateNumber(video_conf->video_fps.ch[i].fps_den));
		cJSON_AddItemToArray(array, item);
	}
	ret = json_write_file(CONFIG_VIDEO_FILE, root);
	if (0 != ret) {
		printf("error(%s,%d): json_write_file\n",
				__func__, __LINE__);
		ret = -1;
		goto err_json_write_file;
	}
	cJSON_Delete(root);
	return 0;

err_json_write_file:
	cJSON_Delete(root);
	return ret;
}

int xcam_conf_get_video(conf_video_t *video_conf)
{
	int ret = -1;
	cJSON *root;
	if (true == video_config_has_read) {
		memcpy(video_conf, &video_config, sizeof(video_config));
		return 0;
	}
	ret = xcam_conf_read_video(&video_config);
	if (0 != ret)  {
		printf("error(%s,%d): xcam conf read video\n",
				__func__, __LINE__);
		return -1;
	}
	memcpy(video_conf, &video_config, sizeof(video_config))	;
	video_config_has_read = true;
}

int xcam_conf_set_video(conf_video_t *video_conf)
{
	int ret = -1;
	cJSON *root;
	if (true != video_config_has_read) {
		ret = xcam_conf_read_video(&video_config);
		if (0 != ret)  {
			printf("error(%s,%d): xcam conf read video\n",
					__func__, __LINE__);
			return ret;
		}
		video_config_has_read = true;
	}
	if (0 != memcmp(video_conf, &video_config, sizeof(video_config))) {
		ret = xcam_conf_write_video(video_conf);
		if (0 != ret)  {
			printf("error(%s,%d): xcam conf write video\n",
					__func__, __LINE__);
			return ret;
		}
		memcpy(&video_config, video_conf, sizeof(video_config))	;
	}
	return 0;
}

#ifdef TEST_XCAM_CONF_VIDEO

/*
 *
 * gcc xcam_conf_video.c xcam_conf_json.c ../../cJSON/cJSON.c -I../inc/ -I../../cJSON/ -DTEST_XCAM_CONF_VIDEO -lm -o test_xcam_conf_video
 *
 *
 */

int test_prepare()
{

	int ret = -1;
	FILE *file = NULL;
	int len = 0;

	file = fopen("video.json","w");
	if (NULL == file) {
		printf("error(%s,%d): file open\n",
				__func__, __LINE__);
		return -1;
	}
	fprintf(file, "%s", "{\"video.fps\":[{\"ch\":0,\"fps_num\":25,\"fps_den\":1}, {\"ch\":1,\"fps_num\":25,\"fps_den\":1}]}");
	fclose(file);
	return 0;
}

static int test1(void)
{
	int ret = 0;
	conf_video_t video_conf;
	test_prepare();
	ret = xcam_conf_read_video(&video_conf);
	assert(0 == ret);
	ret = xcam_conf_write_video(&video_conf);
	assert(0 == ret);
	return ret;
}

static int test2(void)
{
	int ret = 0;
	conf_video_t video_conf;
	test_prepare();
	ret = xcam_conf_get_video(&video_conf);
	assert(0 == ret);
	video_conf.video_fps.ch[0].fps_num = 10;
	video_conf.video_fps.ch[0].fps_den = 2;
	video_conf.video_fps.ch[1].fps_num = 15;
	video_conf.video_fps.ch[1].fps_den = 3;
	ret = xcam_conf_set_video(&video_conf);
	assert(0 == ret);
	return ret;
}

cJSON *items[100];
int json_parse_index = 0;

int json_parse(cJSON *root)
{
	int num = 0;
	int i = 0;
	static int level;
	level++;
	num = cJSON_GetArraySize(root);
	for (i = 0; i < num; i++) {
		cJSON *item = cJSON_GetArrayItem(root, i);
		if (item->type == cJSON_Array) {
			//do not support now
		} else if(item->type == cJSON_Object) {
			printf("index%d level%d	%s:\n", json_parse_index, level, item->string);
			items[json_parse_index] = item;
			json_parse_index++;
			json_parse(item);
		} else {
			printf("index%d level%d	%s, %d\n", json_parse_index, level, item->string, item->valueint);
			items[json_parse_index] = item;
			json_parse_index++;
		}
	}
	level--;
}

static int test3(void)
{
	int ret = 0;

	cJSON * root = json_read_file("video.json");
	json_parse_index = 0;
	json_parse(root);
	cJSON_SetIntValue(items[2], 15);
	json_write_file("video_modify.json", root);

	return 0;
}

int main()
{
	//test1();
	//test2();
	test3();
	return 0;
}
#endif
