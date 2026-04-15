#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <getopt.h>
#include <unistd.h>
#include "xcam_conf.h"
#include "xcam_conf_sys.h"
#include "xcam_general.h"
#include "xcam_log.h"
#include "xcam_cli_options.h"

#define LOG_TAG "XCAM_CLI_OPTIONS"
#define CONFIG_SYS_FILE "/system/sys.ini"

//cli_config_t CliConfig;
/*static int strToInt(char const* str) {
	int val;

	if (sscanf(str, "%d", &val) == 1) return val;
	return -1;
} for warnig */

int xcam_cli_check_args(int argc, char**argv)
{
	int ret = XCAM_SUCCESS;
	int c = -1;
	char const* option = NULL;
	int option_index;

	memset (&cli_attr, 0,sizeof(cli_config_t));
	while (1) {
		option_index = 0;
		//添加长选项命令
		static struct option long_options[] = {
			{"noconfig", no_argument, 0, 0},
			{"fps", required_argument, 0, 0},
			{"st", required_argument, 0, 0},
			{"nrvbs", required_argument, 0, 0},
			{0, 0, 0, 0}
		};

		//第二个参数为短选项命令
		c = getopt_long_only(argc, argv, "w:h:",long_options, &option_index);
		if(c == -1) break;

		switch(c) {
			case 0:
				option = long_options[option_index].name;
				if( strcmp(option,"fps") == 0) {
					cli_attr.fps_num =  atoi(optarg);
					cli_attr.fps_den = 1;
				} else if (strcmp(option,"noconfig") == 0) {
					cli_attr.no_operation_file_flag = true;
				} else if (strcmp(option,"st") == 0) {
					if((strcmp(optarg, "os04b10") == 0) || (strcmp(optarg,"gc2053") == 0) ||(strcmp(optarg,"gc2063") == 0) || (strcmp(optarg, "sc2235") == 0)) {
						strncpy(cli_attr.sensor_name,optarg,XCAM_CLI_SENSOR_NAME_MAX_LEN);
					} else {
						LOG_INF(LOG_TAG,"Incorrect input parameters.The sensor don't support or sensor name error.\n");
						return XCAM_ERROR;
					}
				} else if (strcmp(optarg, "nrvbs") == 0) {
					cli_attr.nrvbs_num = atoi(optarg);
				} else {
					LOG_INF(LOG_TAG,"Incorrect input parameters.Please again input.\n");
					return XCAM_ERROR;
				}
				break;
			case 'w':
				cli_attr.pic_width = atoi(optarg);
				break;
			case 'h':
				cli_attr.pic_height = atoi(optarg);
				break;
			default:
				LOG_INF(LOG_TAG,"Incorrect input parameters.Please again input.\n");
				return XCAM_ERROR;
		}
	}

	return ret;
}

