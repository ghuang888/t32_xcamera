#ifndef _XCAM_CONF_GB_H_
#define _XCAM_CONF_GB_H_
#include "svac_interfaces.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct conf_gb{
	struct svac_device_base_attr baseattr;
	struct svac_device_sip_attr sipattr;
}conf_gb_t;

int xcam_gb_manage_conf_init(void *handle);
int xcam_json_get_sip_config(void *json_root);
int xcam_json_set_sip_config(void *json_root);

#ifdef __cplusplus
}
#endif
#endif

