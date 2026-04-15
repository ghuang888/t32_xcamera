#ifndef _XCAM_VIDEO_H_
#define _XCAM_VIDEO_H_

#include <imp/imp_log.h>
#include <imp/imp_common.h>
#include <imp/imp_system.h>
#include <imp/imp_isp.h>
#include <imp/imp_framesource.h>
#include <imp/imp_encoder.h>

typedef struct video_str_info{
	IMPEncoderCHNAttr enc_attr;
	IMPFSChnAttr fs_attr;
	IMPEncoderGOPSizeCfg pstGOPSizeCfg;
}video_str_info_t;

typedef struct video_cofig_info{
	int fps_num;  //ISP中的帧率
	int fps_den;
	video_str_info_t stream_config[2];
}video_config_info_t;

#define STREAM0 0
#define STREAM1 1
#define STREAM2 2
#define STREAM3 3
#define OSDGROUP0 0
#define OSDGROUP1 1
#define OSDGROUP3 3
#define ENCGROUP0 0
#define ENCGROUP1 1
#define ENCGROUP3 3
#define ENCCHN0 0
#define ENCCHN1 1
#define ENCCHN2 2
#define ENCCHN3 3
#define IVSGROUP0 0
#define IVSGROUP1 1
#define IVSGROUP3 3

int xcam_video_init(void);
int xcam_video_deinit(void);
int xcam_video_enc_start(int channel);
int xcam_video_enc_stop(int channel);
int xcam_video_get_enc_frame(int channel, IMPEncoderStream* stream);
int xcam_video_release_enc_frame(int channel, IMPEncoderStream* stream);

int xcam_video_set_isp_fps(uint32_t fps_num,uint32_t fps_den);
int xcam_video_get_isp_fps(uint32_t *fps_num,uint32_t *fps_den);
int xcam_video_set_fs_fps(int channel,uint32_t fps_num,uint32_t fps_den);
int xcam_video_get_fs_fps(int channel,uint32_t *fps_num,uint32_t *fps_den);
int xcam_video_set_enc_fps(int channel,uint32_t fps_num,uint32_t fps_den);
int xcam_video_get_enc_fps(int channel, uint32_t *fps_num, uint32_t *fps_den);
int xcam_video_set_bitrate(int encChnNum ,int bps_num);
int xcam_video_get_bitrate(int encChnNum, int *bps_num);
int xcam_video_set_resolution(int fsChnNum,int picWidth,int picHeigth);
int xcam_video_get_resolution(int fsChnNum, int *picWidth, int *picHeigth);
int xcam_video_set_encode_mode(int streamnum, int enc_type);
int xcam_video_get_encode_mode(int streamnum, int *enc_type);
int xcam_video_get_channel_num();
int xcam_video_get_encode_GopLength(int channel ,int *gopLength);
int xcam_video_set_encode_GopLength(int channel, int gopLength);
int xcam_video_get_encode_Rcmode(int channel , int *rcmode);
int xcam_video_set_encode_Rcmode(int channel, int rcmode);
int xcam_video_restart_rtsp();
int xcam_personDet_ivs();
#endif
