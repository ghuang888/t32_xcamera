#ifndef STREAM_SOURCE_HH
#define STREAM_SOURCE_HH

#include <mutex>
#include <condition_variable>
#include <thread>
#ifdef XCAM_DOUBLE_SENSOR
#define RTSP_CHANNEL_MAX 4
#else
#define RTSP_CHANNEL_MAX 2
#endif
using namespace std;

typedef void (*funPutFrame)(void *obj, char* buf, int size, int64_t ts);
typedef void (*funSourceClose)(void *obj);

class StreamSource {
private:

	mutex m_mutex[RTSP_CHANNEL_MAX];
	condition_variable m_cond_put_wait[RTSP_CHANNEL_MAX];

	pthread_mutex_t _mutex[RTSP_CHANNEL_MAX];
	pthread_cond_t _cond[RTSP_CHANNEL_MAX];

	int m_frame_stat[RTSP_CHANNEL_MAX];
	int m_channel_stat[RTSP_CHANNEL_MAX];

	funPutFrame put_frame[RTSP_CHANNEL_MAX];
	funSourceClose source_close[RTSP_CHANNEL_MAX];

	void* objs[RTSP_CHANNEL_MAX];

public:
	StreamSource();
	int streamCreate(int channel);
	int streamRegisterObj(int channel, void* obj);
	int streamRegisterPutFrame(int channel, funPutFrame func);
	int streamRegisterSourceClose(int channel, funSourceClose func);
	int streamDestroy(int channel);

	int streamGetFrame(int channel);
	int streamPutFrame(int channel, char* buf, int size, int64_t ts);
	int streamClose(int channel);
};

#endif
