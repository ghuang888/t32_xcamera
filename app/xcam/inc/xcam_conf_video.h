#ifndef _XCAM_CONF_VIDEO_H_
#define _XCAM_CONF_VIDEO_H_

#ifdef __cplusplus
extern "C" {
#endif
typedef enum{
	SET_CONF_VIDEO_ISP_FPS,
	SET_CONF_VIDEO_FS_FPS_CH0,
	SET_CONF_VIDEO_FS_FPS_CH1,
	SET_CONF_VIDEO_ENC_FPS_CH0,
	SET_CONF_VIDEO_ENC_FPS_CH1,
	SET_CONF_VIDEO_BPS_CH0,
	SET_CONF_VIDEO_BPS_CH1,
	SET_CONF_VIDEO_RESOLUTION_CH0,
	SET_CONF_VIDEO_RESOLUTION_CH1,
	SET_CONF_VIDEO_ENCTYPE_CH0,
	SET_CONF_VIDEO_ENCTYPE_CH1,
	SET_CONF_VIDEO_ALL,
	SET_CONF_VIDEO_NIGHTVISIONMODE,
	SET_CONF_VIDEO_WDRMODE,
	SET_CONF_VIDEO_IMAGEMIRRORMODE,
	SET_CONF_VIDEO_IMAGEFLIPMODE,
	SET_CONF_VIDEO_IRCUTMODE,
	SET_CONF_VIDEO_NOISEREDUCTIONMODE,
	SET_CONF_VIDEO_DRCMODE,
	SET_CONF_VIDEO_IMAGE_CONTROL,
	SET_CONF_VIDEO_MAX
}conf_video_set_flag;

typedef struct enc_rc_cbr {
	int target_bitrate;
} enc_rc_cbr_t;

typedef struct enc_rc_vbr {
	int target_bitrate;
	int max_bitrate;
} enc_rc_vbr_t;

typedef struct enc_rc_capped_vbr {
	int target_bitrate;
	int max_bitrate;
} enc_rc_capped_vbr_t;

typedef struct enc_rc_fixqp {
	int qp;
} enc_rc_fixqp_t;

typedef struct enc_rc_smart {
	int max_bitrate;
} enc_rc_smart_t;

typedef struct conf_enc_rc {
	int rc_type;
	union {
		enc_rc_cbr_t rc_cbr;
		enc_rc_vbr_t rc_vbr;
		enc_rc_fixqp_t rc_fixqp;
		enc_rc_capped_vbr_t rc_capped_vbr;
	};
} conf_enc_rc_t;

typedef struct conf_video_isp {
	int fps_num;
	int fps_den;
	int nightvision;
	int wdr;
	int imagemirror;
	int imageflip;
	int ircut;
	int noisereduction;
	int drc;
	int saturation;
	int brightness;
	int sharpness;
	int contrast;
} conf_video_isp_t;

typedef struct conf_video_fs {
	int ch0_w;
	int ch0_h;
	int ch1_w;
	int ch1_h;
} conf_video_fs_t;

typedef struct conf_video_enc {
	int ch0_type;
	int ch1_type;
	conf_enc_rc_t ch0_enc_rc;
	conf_enc_rc_t ch1_enc_rc;
} conf_video_enc_t;

typedef struct conf_persondet {
	int enable;
} conf_persondet_t;

typedef struct conf_ivdc {
	int enable;
} conf_ivdc_t;

typedef struct conf_extraarg {
	int movedetEnable;
	int platerecEnable;
	int facedetEnable;
	int pervehpetEnable;
	int fireworksdetEnable;
	int aiispEnable;
} conf_extraarg_t;

typedef struct conf_video {
	conf_video_isp_t conf_isp;
	conf_video_fs_t conf_fs;
	conf_video_enc_t conf_enc;
	conf_persondet_t conf_persondet;
	conf_ivdc_t conf_ivdc;
	conf_extraarg_t conf_extraarg;
}conf_video_t;

int xcam_conf_get_video(conf_video_t *video_conf);
int xcam_conf_set_video_all(conf_video_t *video_conf);
int xcam_conf_set_video_bps(int channel,int bps_num);
int xcam_conf_set_video_isp_fps(int fps_num,int fps_den);
int xcam_conf_set_video_fs_fps(int channel,int fps_num,int fps_den);
int xcam_conf_set_video_enc_fps(int channel,int fps_num,int fps_den);
int xcam_conf_set_video_resolution(int channel,int picheight,int picwidth);
int xcam_conf_set_video_nightvisionmode(int nightvisionmode);
int xcam_conf_set_video_wdrmode(int wdr);
int xcam_conf_set_video_imagemirrormode(int imagemirror);
int xcam_conf_set_video_imageflipmode(int imageflip);
int xcam_conf_set_video_ircutmode(int ircut);
int xcam_conf_set_video_noisereductionmode(int noisereduction);
int xcam_conf_set_video_drcmode(int drc);
int xcam_conf_set_video_image_control(int saturation,int brightness,int sharpness,int contrast);
int xcam_conf_set_video_enctype(int channel, int enctype);
void xcam_conf_video_init();
int xcam_conf_get_persondet_status(int *enable);
int xcam_conf_get_ivdc_status(int *enable);
int xcam_conf_get_extraarg_status(char *itemV,int length);
conf_video_t xcam_video_conf;
#ifdef __cplusplus
}
#endif
#endif
