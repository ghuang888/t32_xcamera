#ifndef _XCAM_CLI_OPTIONS_H_
#define _XCAM_CLI_OPTIONS_H_

#ifdef __cplusplus
extern "C" {
#endif
#define XCAM_CLI_SENSOR_NAME_MAX_LEN 10

typedef struct cli_config_s{
	char sensor_name[XCAM_CLI_SENSOR_NAME_MAX_LEN + 1];
	int pic_width;
	int pic_height;
	int fps_num;
	int fps_den;
//	int BpsNum;
	bool no_operation_file_flag;//不操控配置文件标记
	int nrvbs_num;
}cli_config_t;
int xcam_cli_check_args(int argc, char**argv);
cli_config_t cli_attr;
#ifdef __cplusplus
}
#endif
#endif
