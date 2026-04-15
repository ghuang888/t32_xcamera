#ifndef _XCAM_RTSP_H_
#define _XCAM_RTSP_H_
#include <stdint.h>

typedef struct rtsp_module_s {
    xcam_module_t *rtspmod;
    int channel;
    int state;
} rtsp_module_t;
int rtsp_init(void);
int rtsp_module_create(int channel, int modid);
#endif
