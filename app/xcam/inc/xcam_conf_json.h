#ifndef _XCAM_CONF_JSON_H_
#define _XCAM_CONF_JSON_H_

#ifdef __cplusplus
extern "C" {
#endif

cJSON *json_read_file(char *filename);
int json_write_file(char *filename, cJSON *root);

#ifdef __cplusplus
}
#endif


#endif
