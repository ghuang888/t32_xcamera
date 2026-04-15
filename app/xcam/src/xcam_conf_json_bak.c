#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>

#include "xcam_conf_json.h"

cJSON *json_read_file(char *filename)
{
	FILE *file;
	int len;
	char *data;
	cJSON *json;
	cJSON *ret;

	file = fopen(filename,"r");
	if (NULL == file) {
		printf("error(%s,%d): file open %s\n",
				__func__, __LINE__, strerror(errno));
		ret = NULL;
		goto err_fopen;
	}
	fseek(file, 0, SEEK_END);
	len = ftell(file);
	fseek(file, 0, SEEK_SET);

	data = (char*)malloc(len+1);
	if (NULL == data) {
		printf("error(%s,%d): malloc %s\n",
				__func__, __LINE__, strerror(errno));
		ret = NULL;
		goto err_malloc;
	}

	fread(data, 1, len, file);
	data[len]='\0';

	json = cJSON_Parse(data);
	if (!json) {
		printf("error(%s,%d): json parse %s\n",
				__func__, __LINE__, cJSON_GetErrorPtr());
		ret = NULL;
		goto err_json_parse;
	}

	ret = json;
#if 0
	{
		char *json_str;
		printf("file:%s\n", data);
		json_str = cJSON_Print(json);
		printf("json:%s\n", json_str);
		free(json_str);
	}
#endif

	free(data);
	return ret;
err_json_parse:
	free(data);
err_malloc:
	fclose(file);
err_fopen:
	return ret;
}

int json_write_file(char *filename, cJSON *root)
{
	int ret = -1;
	FILE *file = NULL;
	char *json_str;
	int len = 0;

	json_str = cJSON_Print(root);
	file = fopen(filename,"w");
	if (NULL == file) {
		printf("error(%s,%d): file open %s\n",
				__func__, __LINE__, strerror(errno));
		goto err_fopen;
	}

	len = fprintf(file, "%s", json_str);
	if (strlen(json_str) != len) {
		printf("error(%s,%d): file fprintf %s\n",
				__func__, __LINE__, strerror(errno));
		goto err_file_fprintf;
	}

	free(json_str);
	fclose(file);
	return 0;

err_file_fprintf:
	free(json_str);
	fclose(file);
err_fopen:
	return ret;
}

#ifdef TEST_XCAM_CONF_JSON

/*
 *
 * gcc xcam_conf_json.c ../../cJSON/cJSON.c -I../inc/ -I../../cJSON/ -DTEST_XCAM_CONF_JSON -lm -o test_xcam_conf_json
 *
 *
 */
int main()
{
	int ret = 0;
	cJSON *root;
	root = json_read_file("test.json");
	assert(root != NULL);
	ret = json_write_file("tmp.json", root);
	assert(ret == 0);
	return 0;

}
#endif
