#include <iostream>
#include <unistd.h>
#include <assert.h>
#include <sys/time.h>


#include "H265VideoStreamSource.hh"
#include "StreamSource.hh"
#include "c_liveRTSP.h"
#include "LiveRTSP.hh"

#define LOG cout << "RTSP:" << __func__ << "-" << __LINE__ << ","
extern LiveRTSP liveRTSP;
using namespace std;
H265VideoStreamSource*
H265VideoStreamSource::createNew(UsageEnvironment& env, int channel, StreamSource* ss)
{
	return new H265VideoStreamSource(env, channel, ss);
}

H265VideoStreamSource::H265VideoStreamSource(UsageEnvironment& env, int channel, StreamSource* ss): FramedSource(env), eventTriggerId(0){
	eventTriggerId = envir().taskScheduler().createEventTrigger(event_putFrame);
	m_channel = channel;
	m_ss = ss;
	LOG << "stream create ch-" << m_channel << endl;
	m_ss->streamCreate(m_channel);
	m_ss->streamRegisterObj(m_channel, (void*)this);
	m_ss->streamRegisterPutFrame(m_channel, H265VideoStreamSource::putFrame);
	m_ss->streamRegisterSourceClose(m_channel, H265VideoStreamSource::sourceClose);
	m_frame_done = 0;
	m_source_close = 0;
	fMaxSize = 5097152;
}

H265VideoStreamSource::~H265VideoStreamSource() {
	envir().taskScheduler().deleteEventTrigger(eventTriggerId);
	m_ss->streamDestroy(m_channel);
	m_ss = NULL;
	LOG << "stream destroy ch-" << m_channel << endl;
	//envir() << "" << __func__ << __LINE__ << "\n";
}

void H265VideoStreamSource::doGetNextFrame() {
	//LOG << "stream get frame ch-" << m_channel << endl;
	int ret = 0;
	ret = m_ss->streamGetFrame(m_channel);
	if(ret) {
		LOG << "stream get frame ch-" << m_channel  << "failed" << endl;
	}
}
// static struct timeval int64_to_timeval(int64_t usec) {
// 	struct timeval tv;
//     tv.tv_sec = usec / 1000000;  // 计算秒数
//     tv.tv_usec = usec % 1000000;  // 计算剩余的微秒数
// 	return tv;
// }
void H265VideoStreamSource::putFrameImpl(char* buf, int size, int64_t ts) {
	if (size <= (int)fMaxSize && fTo && m_ss && buf && this) {
		if((m_channel == 0 &&  liveRTSP.m_subsession_h264_video0) || (m_channel == 1 &&  liveRTSP.m_subsession_h264_video1) || (m_channel == 3 &&  liveRTSP.m_subsession_h264_video3)){
			memcpy(fTo, buf, size);
			fFrameSize = size;
			gettimeofday(&fPresentationTime, NULL);       
		}
	} else {
		envir() <<__func__<<"-"<<__LINE__<<":"<< "FrameSize "<<size<< ", fMaxSize "<<fMaxSize<<"\n";
    }
	envir().taskScheduler().triggerEvent(eventTriggerId, this);
}

void H265VideoStreamSource::putFrame(void* obj, char* buf, int size, int64_t ts) {
	H265VideoStreamSource *object = ((H265VideoStreamSource*)obj);
	object->putFrameImpl(buf, size, ts);
}

void H265VideoStreamSource::sourceCloseImpl() {
	LOG << " ch-" << m_channel << endl;
	unique_lock<mutex> lck(m_mutex);
	m_source_close = 1;
	envir().taskScheduler().triggerEvent(eventTriggerId, this);
}

void H265VideoStreamSource::sourceClose(void* obj) {
	H265VideoStreamSource *object = ((H265VideoStreamSource*)obj);
	object->sourceCloseImpl();
}

void H265VideoStreamSource::event_putFrameImpl() {
	if (m_source_close == 1) {
		handleClosure(this);
		m_source_close = 0;
	} else {
		FramedSource::afterGetting(this);
	}
}

void H265VideoStreamSource::event_putFrame(void* clientData) {
	H265VideoStreamSource* obj = (H265VideoStreamSource*)clientData;
	obj->event_putFrameImpl();
}

//int H265VideoStreamSource::putFrame(unsigned char* buf, int size, int64_t ts) {
//    if (size <= (int)fMaxSize) {
//        //struct timeval tv;
//        memcpy(fTo, buf, size);
//        fFrameSize = size;
//#if 0
//        // Set the 'presentation time':
//        {
//            if (fPresentationTime.tv_sec == 0 && fPresentationTime.tv_usec == 0) {
//                // This is the first frame, so use the current time:
//                gettimeofday(&fPresentationTime, NULL);
//            } else {
//                // Increment by the play time of the previous data:
//                unsigned uSeconds	= fPresentationTime.tv_usec + fLastPlayTime;
//                fPresentationTime.tv_sec += uSeconds/1000000;
//                fPresentationTime.tv_usec = uSeconds%1000000;
//            }
//            // Remember the play time of this data:
//            fLastPlayTime = (30000);
//            fDurationInMicroseconds = fLastPlayTime;
//        }
//#endif
//        fPresentationTime.tv_sec = ts/1000000;
//        fPresentationTime.tv_usec = ts%1000000;
//        //gettimeofday(&tv, NULL);
//        //fPresentationTime = tv;
//        //cout <<__func__<<"-"<<__LINE__<<":"<<fPresentationTime.tv_sec <<"-"<<fPresentationTime.tv_usec <<" size: "<<size  <<"\n";
//        //printf("%s,%d: %d, %d\n", __func__, __LINE__, fPresentationTime.tv_sec, fPresentationTime.tv_usec);
//        // Inform the downstream object that it has data:
//        FramedSource::afterGetting(this);
//    } else {
//        envir() <<__func__<<"-"<<__LINE__<<":"<< "FrameSize "<<size<< ", fMaxSize "<<fMaxSize<<"\n";
//    }
//    return 0;
//}
