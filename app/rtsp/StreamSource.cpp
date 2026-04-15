#include <iostream>

#include "StreamSource.hh"
#include <sys/time.h>
#define LOG cout << "RTSP:" << __func__ << "-" << __LINE__ << ","

using namespace std;

StreamSource streamSource;

enum {
	CHANNEL_STAT_INVALID = 0,
	CHANNEL_STAT_VALID
};

enum {
	FRAME_STAT_NONE,
	FRAME_STAT_READY,
};

StreamSource::StreamSource()
{
	LOG << endl;

	pthread_mutex_init(&_mutex[0],NULL);
	pthread_mutex_init(&_mutex[1],NULL);
	pthread_mutex_init(&_mutex[2],NULL);
	pthread_mutex_init(&_mutex[3],NULL);
	pthread_cond_init(&_cond[0],NULL);
	pthread_cond_init(&_cond[1],NULL);
	pthread_cond_init(&_cond[2],NULL);
	pthread_cond_init(&_cond[3],NULL);
}
// extern int xcam_video_enc_start(int channel);
int StreamSource::streamCreate(int channel)
{
	LOG << "channel: " << channel << endl;
	//unique_lock<mutex> lck(m_mutex[channel]);

    pthread_mutex_lock(&_mutex[channel]);

	m_channel_stat[channel] = CHANNEL_STAT_VALID;
	m_frame_stat[channel] = FRAME_STAT_NONE;

    pthread_mutex_unlock(&_mutex[channel]);
	//lck.unlock();
	return 0;
}

int StreamSource::streamRegisterPutFrame(int channel, funPutFrame func)
{
	put_frame[channel] = func;
	return 0;
}

int StreamSource::streamRegisterSourceClose(int channel, funSourceClose func)
{
	source_close[channel] = func;
	return 0;
}

int StreamSource::streamRegisterObj(int channel, void* obj)
{
	objs[channel] = obj;
	return 0;
}

int StreamSource::streamDestroy(int channel)
{

	LOG << endl;
	//unique_lock<mutex> lck(m_mutex[channel]);

	
    pthread_mutex_lock(&_mutex[channel]);
	m_channel_stat[channel] = CHANNEL_STAT_INVALID;
	m_frame_stat[channel] = FRAME_STAT_NONE;
	objs[channel] = NULL;
    pthread_mutex_unlock(&_mutex[channel]);
	LOG << endl;
	//lck.unlock();
	return 0;
}

int StreamSource::streamClose(int channel)
{

	LOG << endl;
	//unique_lock<mutex> lck(m_mutex[channel]);
    pthread_mutex_lock(&_mutex[channel]);
	if (m_channel_stat[channel] == CHANNEL_STAT_VALID)
		(*source_close[channel])(objs[channel]);
	m_channel_stat[channel] = CHANNEL_STAT_INVALID;
	m_frame_stat[channel] = FRAME_STAT_NONE;
    pthread_mutex_unlock(&_mutex[channel]);
	LOG << endl;
	//lck.unlock();
	return 0;
}

int StreamSource::streamPutFrame(int channel, char *buf, int size, int64_t ts)
{
	struct timeval now;
	// struct timespec timeout;
	gettimeofday(&now, NULL);
	// timeout.tv_sec = now.tv_sec + 0;
	// timeout.tv_nsec = now.tv_usec * 5000;
	//unique_lock<mutex> lck(m_mutex[channel]);
	int ret;
    ret = pthread_mutex_lock(&_mutex[channel]);
	if(ret){
#ifdef DEBUG
		LOG << "rtsp stream already close" << "channel: " << channel << endl;
#endif
		//获取锁超时，强制返回,不影响出流
		return 0;
	}
	if (m_channel_stat[channel] == CHANNEL_STAT_VALID  && objs[channel] != NULL) {
		while (m_frame_stat[channel] != FRAME_STAT_READY) {
			//m_cond_put_wait[channel].wait(lck);
			ret = pthread_cond_wait(&_cond[channel], &_mutex[channel]);
// 			if(ret){
// #ifdef DEBUG
// 				LOG << "rtsp stream already close " << "channel: " << channel << endl;
// #endif
// 				//等待帧信号量超时，强制返回,不影响出流
// 				pthread_mutex_unlock(&_mutex[channel]);
// 				return 0;
// 			}
		}
		m_frame_stat[channel] = FRAME_STAT_NONE;
		if (m_channel_stat[channel] == CHANNEL_STAT_VALID) {
			(*put_frame[channel])(objs[channel], buf, size, ts);
		} else
			LOG << "stream drop due to channel close" << endl;
	}
    pthread_mutex_unlock(&_mutex[channel]);
	//lck.unlock();
	return 0;
}

int StreamSource::streamGetFrame(int channel)
{
	int ret = 0;
	//unique_lock<mutex> lck(m_mutex[channel]);
	pthread_mutex_lock(&_mutex[channel]);
	if (m_channel_stat[channel] == CHANNEL_STAT_VALID ) {
		if (m_frame_stat[channel] == FRAME_STAT_READY)
			LOG << "stream frame already ready " << "channel: " << channel << endl;
		m_frame_stat[channel] = FRAME_STAT_READY;
		//m_cond_put_wait[channel].notify_all();
		pthread_cond_signal(&_cond[channel]);
		ret = 0;
	} else {
		ret = -1;
	}
	pthread_mutex_unlock(&_mutex[channel]);
	//LOG << "exit  " << "channel: " << channel << endl;
	//lck.unlock();
	return ret;
}
