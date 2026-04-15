#include <iostream>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>

#include "H264VideoStreamSource.hh"
#include "StreamSource.hh"
#include "c_liveRTSP.h"

using namespace std;

#define LOG cout << "RTSP:" << __func__ << "-" << __LINE__ << ","

H264VideoStreamSource*
H264VideoStreamSource::createNew(UsageEnvironment& env, int channel, StreamSource* ss)
{
	return new H264VideoStreamSource(env, channel, ss);
}

H264VideoStreamSource::H264VideoStreamSource(UsageEnvironment& env, int channel, StreamSource* ss): FramedSource(env), eventTriggerId(0){
	eventTriggerId = envir().taskScheduler().createEventTrigger(event_putFrame);
	m_channel = channel;
	m_ss = ss;
	LOG << "stream create ch-" << m_channel << endl;
	m_ss->streamCreate(m_channel);
	m_ss->streamRegisterObj(m_channel, (void*)this);
	m_ss->streamRegisterPutFrame(m_channel, H264VideoStreamSource::putFrame);
	m_ss->streamRegisterSourceClose(m_channel, H264VideoStreamSource::sourceClose);
	m_frame_done = 0;
	m_source_close = 0;

	pthread_mutex_init(&_mutex,NULL);
	pthread_cond_init(&_cond,NULL);
}

H264VideoStreamSource::~H264VideoStreamSource() {
	envir().taskScheduler().deleteEventTrigger(eventTriggerId);
	LOG << "stream destroy ch-" << m_channel << endl;
	printf("%s[%d] \n", __func__, __LINE__);
	m_ss->streamDestroy(m_channel);
	m_ss = NULL;
	//envir() << "###" << __func__ << __LINE__ << "\n";
}

void H264VideoStreamSource::doGetNextFrame() {
	//LOG << "stream get frame ch-" << m_channel << endl;
	int ret = 0;
	ret = m_ss->streamGetFrame(m_channel);
	if(ret) {
		LOG << "stream get frame ch-" << m_channel << "failed"<< endl;
	}
}

void H264VideoStreamSource::putFrameImpl(char* buf, int size, int64_t ts) {
	if (size <= (int)fMaxSize) {
		memcpy(fTo, buf, size);
		fFrameSize = size;
		gettimeofday(&fPresentationTime, NULL);
	} else {
		envir() <<__func__<<"-"<<__LINE__<<":"<< "FrameSize "<<size<< ", fMaxSize "<<fMaxSize<<"\n";
    }
	envir().taskScheduler().triggerEvent(eventTriggerId, this);
}

void H264VideoStreamSource::putFrame(void* obj, char* buf, int size, int64_t ts) {
	H264VideoStreamSource *object = ((H264VideoStreamSource*)obj);
	object->putFrameImpl(buf, size, ts);
}

void H264VideoStreamSource::sourceCloseImpl() {
	LOG << " ch-" << m_channel << endl;
	unique_lock<mutex> lck(m_mutex);
	m_source_close = 1;
	envir().taskScheduler().triggerEvent(eventTriggerId, this);
}

void H264VideoStreamSource::sourceClose(void* obj) {
	LOG << endl;
	H264VideoStreamSource *object = ((H264VideoStreamSource*)obj);
	object->sourceCloseImpl();
}

void H264VideoStreamSource::event_putFrameImpl() {
	if (m_source_close == 1) {
		handleClosure(this);
		m_source_close = 0;
	} else {
		FramedSource::afterGetting(this);
	}
}

void H264VideoStreamSource::event_putFrame(void* clientData) {
	H264VideoStreamSource* obj = (H264VideoStreamSource*)clientData;
	obj->event_putFrameImpl();
}

