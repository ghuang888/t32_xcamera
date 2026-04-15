/**********
This library is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the
Free Software Foundation; either version 2.1 of the License, or (at your
option) any later version. (See <http://www.gnu.org/copyleft/lesser.html>.)

This library is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License
along with this library; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
**********/
// "liveMedia"
// Copyright (c) 1996-2011 Live Networks, Inc.  All rights reserved.
// A WAV audio file source
// Implementation

#include "G711AudioStreamSource.hh"

// G711AudioStreamSource audiostreamSource;
uint8_t audiodata[400];
uint8_t audioflag = 0 ;
uint8_t audiodesflag = 0 ;
int G711AudioStreamSource::audioGetOneFrame(uint8_t *buf, unsigned int size){
//     int ret;
//          printf("%s[%d] lzhzcj len=%d  %d\n", size, fMaxSize);
// if(fTo)
// 	// if (size <= (int)fMaxSize) {
// 		// memcpy(fTo, buf, size);
//         printf("%s[%d] lzhzcj len=%d\n", size);
// 		// fFrameSize = size;
// 		// gettimeofday(&fPresentationTime, NULL);
// 		// LOG << "stream get frame ch " << m_channel << "size " << size << endl;
// 	// } else {
// 		// envir() <<__func__<<"-"<<__LINE__<<":"<< "FrameSize "<<size<< ", fMaxSize "<<fMaxSize<<"\n";
//     // }

    return 0;
}
G711AudioStreamSource*  my;
G711AudioStreamSource*
G711AudioStreamSource::createNew(UsageEnvironment& env) {
    do {
        G711AudioStreamSource* newSource = new G711AudioStreamSource(env);
        if (newSource != NULL && newSource->bitsPerSample() == 0) {
            // The WAV file header was apparently invalid.
            Medium::close(newSource);
            break;
        }
        my = newSource;
        return newSource;
    } while (0);

    return NULL;
}

G711AudioStreamSource::G711AudioStreamSource(UsageEnvironment& env)
    : FramedSource(env), 
    fNumChannels(0), fSamplingFrequency(0),
    fBitsPerSample(0),
    fLimitNumBytesToStream(False),
    fNumBytesToStream(0),
    fLastPlayTime(0),
    fPlayTimePerSample(0){

    fNumChannels = 1;
    fSamplingFrequency = 8000;
    fBitsPerSample = 16;
    m_source_close = 0;
    pthread_mutex_init(&audio_stream_mutex, NULL);
    eventTriggerId = envir().taskScheduler().createEventTrigger(event_putFrame);
    audiodesflag = 0;
    fMaxSize = 800;
    // audioOpen();
    printf("%s[%d] \n", __func__, __LINE__);
}

G711AudioStreamSource::~G711AudioStreamSource() {
    // audioClose();
    envir().taskScheduler().deleteEventTrigger(eventTriggerId);
    audiodesflag = 1;
    printf("%s[%d] \n", __func__, __LINE__);
}

int G711AudioStreamSource::putFrameImpl(char* buf, int size, int64_t ts){
    // printf("%s[%d] lzhzcj len=%d\n", size);
    // if(buf){
    //      audioGetOneFrame(buf, size);
    // }
    return 0;
}
void G711AudioStreamSource::do_it(int64_t ts){
    // currentTimestamp = ts;
    // doGetAudioNextFrame();
}
// static struct timeval int64_to_timeval(int64_t usec) {
// 	struct timeval tv;
//     tv.tv_sec = usec / 1000000;  // 计算秒数
//     tv.tv_usec = usec % 1000000;  // 计算剩余的微秒数
// 	return tv;
// }
void G711AudioStreamSource::doGetNextFrame(){
    //   pthread_mutex_unlock(&audio_stream_mutex);
        memcpy(fTo, audiodata, 400);

        fFrameSize = 400;
        gettimeofday(&fPresentationTime, NULL);            // memset(audiodata, 0, 400);
            // printf("lzhzcj %s[%d] fMaxSize=%d\n",__func__, __LINE__, fFrameSize);
            audioflag = 0;
         envir().taskScheduler().triggerEvent(eventTriggerId, this);
}

void G711AudioStreamSource::doGetAudioNextFrame() {
    if(audioflag){
        // printf("%s[%d] \n", __func__, __LINE__);
        // pthread_mutex_lock(&audio_stream_mutex);
        
    }
   
}
void G711AudioStreamSource::sourceCloseImpl() {
	unique_lock<mutex> lck(m_mutex);
	m_source_close = 1;
	envir().taskScheduler().triggerEvent(eventTriggerId, this);
}

void G711AudioStreamSource::sourceClose(void* obj) {
	G711AudioStreamSource *object = ((G711AudioStreamSource*)obj);
	object->sourceCloseImpl();
}

void G711AudioStreamSource::event_putFrameImpl() {
	if (m_source_close == 1) {
		handleClosure(this);
		m_source_close = 0;
	} else {
		FramedSource::afterGetting(this);
	}
}

void G711AudioStreamSource::event_putFrame(void* clientData) {
	G711AudioStreamSource* obj = (G711AudioStreamSource*)clientData;
    obj->event_putFrameImpl();
}
