#ifndef _XCAM_CONF_SYS_H_
#define _XCAM_CONF_SYS_H_

extern unsigned char sensor_name[10];
extern unsigned int  sensor_i2c;
extern unsigned int  sensor_width;
extern unsigned int  sensor_height;

extern int xcam_load_sys_config(void);
#endif
