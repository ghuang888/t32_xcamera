#ifndef _XCAM_CONF_VIDEO_H_
#define _XCAM_CONF_VIDEO_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct conf_video_fps_channel {
	int fps_num;
	int fps_den;
} conf_video_fps_channel_t;

typedef struct conf_video_fps {
	//support 2 channels
	conf_video_fps_channel_t ch[2];
} conf_video_fps_t;

typedef struct conf_video {
	conf_video_fps_t video_fps;
} conf_video_t;

int xcam_conf_get_video(conf_video_t *video_conf);
int xcam_conf_set_video(conf_video_t *video_conf);

#ifdef __cplusplus
}
#endif


#endif
