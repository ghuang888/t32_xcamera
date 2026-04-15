#ifndef _XCAM_SYSTEM_H_
#define _XCAM_SYSTEM_H_

#ifdef __cplusplus
extern "C" {
#endif

void xcam_system_reboot();
void xcam_system_reset_factory_settings();
void xcam_system_stop_boa();
void xcam_system_start_boa();
int xcam_fun_af_get_hist();

#ifdef __cplusplus
}
#endif
#endif
