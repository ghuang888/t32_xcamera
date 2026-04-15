#ifndef _C_LIVE_RTSP_H
#define _C_LIVE_RTSP_H

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */

enum {
	LIVE_RTSP_STREAM_H264,
	LIVE_RTSP_STREAM_H265
};

int c_RTSP_init();
int c_RTSP_set_stream_type(int channel, int type);
int c_RTSP_start(int channel, int width, int height);
int c_RTSP_stop(int channel);
int c_RTSP_put_source_frame(int channel, char *buf, int size, int64_t ts);
int c_RTSP_put_source_audioframe(int channel, uint8_t *buf, int size, int ts);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif
