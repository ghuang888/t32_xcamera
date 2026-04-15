#ifndef _XCAM_OSD_H_
#define _XCAM_OSD_H_
#include <imp/imp_osd.h>

#define XCAM_OSD_TOKEN_MAX_LEN 20
#define XCAM_OSD_RGN_TEXT_MAX_LEN 30

typedef struct xcam_link {
	struct xcam_link *next;
	struct xcam_link *pre;
} xcam_link_t;

typedef struct osd_rgn_config {
	xcam_link_t node;
	char osd_rgn_token[XCAM_OSD_TOKEN_MAX_LEN];
	char text[XCAM_OSD_RGN_TEXT_MAX_LEN];
	int streamnum;
	int osdrgnhandle;
	IMPOsdRgnType type;
	int showflag;
	int maxlen;
	int pos0_x;
	int pos0_y;
	int picHeight;
	int picWidth;
	char* data;
} osd_rgn_config_t;

typedef struct xcam_osd
{
	int state;
	xcam_link_t osd_link_head[4];
	pthread_mutex_t osd_mutex;
}xcam_osd_t;

enum osd_time_type {
	TIEM_TYPE_0, //YYYY-MM-DD ����W HH:MM:SS
	TIME_TYPE_1, //DD-MM-YYYY ����W HH:MM:SS
	TIME_TYPE_2, //YYYY��MM��DD�� ����W HH:MM:SS
	TIME_TYPE_3, //MM��DD��YYYY�� ����W HH:MM:SS
	TIME_TYPE_4, //YYYY-MM-DD HH:MM:SS
	TIME_TYPE_5, //DD-MM-YYYY HH:MM:SS
	TIME_TYPE_6, //YYYY��MM��DD�� HH:MM:SS
	TIME_TYPE_7,//MM��DD��YYYY�� HH:MM:SS
};

int xcam_osd_init();
int xcam_osd_set_fps_kbps(int ch, double fps, double kbps);
int xcam_osd_all_osdrgn_destroy(int iFsChnNum);
void xcam_osd_all_osdrgn_restart(int streamnum,int picWidth,int picHeight);
int xcam_osd_delete_osdrgn(int stream_num, char* token);
int xcam_osd_create_text(int stream_num, int x, int y, char* text, char *osdtoken);
int xcam_osd_set_text(int streamnum, char* osdtoken, int x, int y, char *text);
int xcam_osd_get_osdrgns_num(int streamnum);
int xcam_osd_get_timestamp(int streamnum, int* x, int* y, char* osdtoken);
int xcam_osd_remove_show(int streamnum, char* token);
int xcam_osd_get_max_effective_range(int streamnum, int* height, int* width);
void xcam_osd_update_stream_info(int streamnum, int picWidth, int picHeight);
#endif
