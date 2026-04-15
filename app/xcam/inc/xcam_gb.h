#ifndef _XCAM_GB_H_
#define _XCAM_GB_H_
#include <stdint.h>

#ifdef __cplusplus
extern "c"{
#endif

typedef struct gb_timestamp
{
	int timestamp_x;
	int timestamp_y;
	int timestamp_enable;
	int timestamp_type;
}gb_timestamp_t;

typedef struct gb_text
{
	char osdtoken[32];
	int text_x;
	int text_y;
	char text[32];
}gb_text_t;

typedef struct gb_manage_module{
	xcam_module_t *gbmod;
	int state;
	void *handle;
	pthread_mutex_t gb_mutex;
	unsigned int total_chn;
	unsigned int realplay_chn;
	int OsdWinLength;
	int OsdWinWidth;
	gb_timestamp_t ts;
	int text_enable;
	int text_total;
	gb_text_t text[8];
} gb_manage_module_t;

int gb_manage_module_create(int modid);
int gb_manage_module_destroy(void);
int gb_manage_module_start(void);
int gb_manage_module_stop(void);
int gb_manage_modue_restart(void);

#ifdef __cplusplus
}
#endif

#endif
