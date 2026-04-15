#ifndef _XCAM_CONF_H_
#define _XCAM_CONF_H_

#ifdef __cplusplus
extern "C" {
#endif

int xcam_read_profile(const char *filename, const char *token, const char *parameter, char *value);
int xcam_write_profile(char *filename, char *token, char *parameter, char *value);

#ifdef __cplusplus
}
#endif


#endif //__PROFILES_MANAGEMENT__
