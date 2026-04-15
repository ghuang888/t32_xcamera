#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>
#include "cJSON.h"
#include "xcam_log.h"

#define LOG_TAG "Json"
#define XCAM_SUCCESS 0
#define XCAM_ERROR -1

int json_write_file(char *filename, cJSON *root)
{
	int ret = XCAM_ERROR;
	FILE *file = NULL;
	char *json_str;
	int len = 0;

	if (filename == NULL || root == NULL) {
		LOG_ERR(LOG_TAG,"%s,%d,illegal parameter.\n",__func__,__LINE__);
		return XCAM_ERROR;
	}

	json_str = cJSON_Print(root);
	file = fopen(filename,"w+");
	if (NULL == file) {
		LOG_ERR(LOG_TAG,"error(%s,%d): file open fail\n",__func__, __LINE__);
		goto err_fopen;
	}

	len = fprintf(file, "%s", json_str);
	if (strlen(json_str) != len) {
		LOG_ERR(LOG_TAG,"error(%s,%d): file fprintf fail\n",__func__, __LINE__);
		goto err_file_fprintf;
	}

	fclose(file);
	return XCAM_SUCCESS;

err_file_fprintf:
	fclose(file);
err_fopen:
	return ret;
}

cJSON *json_read_file(char *filename)
{
	FILE *file;
	int len;
	char *data;
	cJSON *json;
	cJSON *ret;
	if (filename == NULL) {
		LOG_ERR(LOG_TAG,"%s,%d,illegal parameter.\n",__func__,__LINE__);
		return NULL;
	}

	file = fopen(filename,"r");
	if (NULL == file) {
		LOG_ERR(LOG_TAG,"error(%s,%d): file open fail\n",__func__, __LINE__);
		ret = NULL;
		goto err_fopen;
	}

	fseek(file, 0, SEEK_END);
	len = ftell(file);
	fseek(file, 0, SEEK_SET);

	data = (char*)malloc(len+4);
	if (NULL == data) {
		LOG_ERR(LOG_TAG,"error(%s,%d): malloc fail\n",__func__, __LINE__);
		ret = NULL;
		goto err_malloc;
	}

	fread(data, 1, len, file);
	data[len+1]='\0';

	json = cJSON_Parse(data);
	if (!json) {
		LOG_ERR(LOG_TAG,"error(%s,%d): json parse %s\n",__func__, __LINE__, cJSON_GetErrorPtr());
		ret = NULL;
		goto err_json_parse;
	}
	ret = json;

	free(data);
	fclose(file);
	return ret;
err_json_parse:
	free(data);
err_malloc:
err_fopen:
	fclose(file);
  	return ret;
}

