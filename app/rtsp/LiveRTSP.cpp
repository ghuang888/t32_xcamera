#include <iostream>
#include <thread>

#include "c_liveRTSP.h"

#include "LiveRTSP.hh"
#include "StreamSource.hh"
#include "G711AudioStreamSource.hh"

using namespace std;

#define LOG cout << "RTSP:" << __func__ << "-" << __LINE__ << ","

LiveRTSP liveRTSP;
extern StreamSource streamSource;
extern uint8_t audiodata[400];
extern uint8_t audioflag;
extern uint8_t audiodesflag;
extern G711AudioStreamSource*  my;
// extern G711AudioStreamSource audiostreamSource;

LiveRTSP* LiveRTSP::createNew()
{
	LiveRTSP* newRTSP = new LiveRTSP();

	return newRTSP;
}

LiveRTSP::LiveRTSP() {
	int i = 0;
	reuseFirstSource = False;
	iFramesOnly = False;
	for (i = 0; i < LIVE_RTSP_CHANNEL_MAX; i++) {
		m_channel_stat[i] = LIVE_RTSP_CHANNEL_STAT_STOP;
	}
	for (i = 0; i < LIVE_RTSP_CHANNEL_MAX; i++) {
		m_stream_type[i] = LIVE_RTSP_STREAM_H264;
	}
	for (i = 0; i < LIVE_RTSP_CHANNEL_MAX; i++) {
		m_stream_started[i] = 0;
	}
}

LiveRTSP::~LiveRTSP() {

}

void LiveRTSP::announceStream(RTSPServer* rtspServer, ServerMediaSession* sms,
		char const* streamName, char const* inputFileName) {
	char* url = rtspServer->rtspURL(sms);
	//UsageEnvironment& env = rtspServer->envir();
	LOG <<"Play this stream using the URL: " <<url <<endl;
	delete[] url;
}

int LiveRTSP::runWorkThread(LiveRTSP* obj)
{
	obj->workThread();
	return 0;
}

int LiveRTSP::workThread()
{
	LOG << "work thread start" << endl;
	env->taskScheduler().doEventLoop();
	LOG << "work thread end" << endl;
	return 0;
}

int LiveRTSP::initEnv(void)
{
	// Begin by setting up our usage environment:
	TaskScheduler* scheduler = BasicTaskScheduler::createNew();
	env = BasicUsageEnvironment::createNew(*scheduler);
	m_work_thread = new thread(runWorkThread, this);

#ifdef ACCESS_CONTROL
	// To implement client access control to the RTSP server, do the following:
	authDB = new UserAuthenticationDatabase;
	authDB->addUserRecord("username1", "password1"); // replace these with real strings
	// Repeat the above with each <username>, <password> that you wish to allow
	// access to the server.
#endif
	// Create the RTSP server:
	rtspServer = RTSPServer::createNew(*env, 8554, authDB);
	if (rtspServer == NULL) {
		*env << "Failed to create RTSP server: " << env->getResultMsg() << "\n";
		return -1;
	}
	eventCloseStreamCh0 = env->taskScheduler().createEventTrigger(event_stopRTSPImplCh0);
	eventCloseStreamCh1 = env->taskScheduler().createEventTrigger(event_stopRTSPImplCh1);
	eventCloseStreamCh3 = env->taskScheduler().createEventTrigger(event_stopRTSPImplCh3);
	return 0;
}

int LiveRTSP::startRTSP(int channel, int width, int height)
{
	LOG << "RTSP start channel " << channel << endl;

	if (m_channel_stat[channel] != LIVE_RTSP_CHANNEL_STAT_START) {

		char const* descriptionString
			= "Session streamed by \"testOnDemandRTSPServer\"";

		if (0 == channel && m_sms0 == NULL) {
			if (m_stream_started[0] == 0) {
				char const* streamName = "stream0";
				char const* inputFileName = "stream0";
				// char const* streamAudioName = "audiostream0";
				m_sms0 = ServerMediaSession::createNew(*env, streamName, NULL, descriptionString, False);
				m_subsession_h264_video0 = H265VideoServerMediaSubsession::createNew(*env, 0, &streamSource);
				// m_subsession_h264_video0->setStreamType(m_stream_type[0]);
				m_audiossbsession_g711 = G711AudioStreamServerMediaSubsession::createNew(*env, true);

				m_sms0->addSubsession(m_subsession_h264_video0);
				// m_sms0->addSubsession(m_audiossbsession_g711);
				rtspServer->addServerMediaSession(m_sms0);
				m_stream_started[0] = 1;
				announceStream(rtspServer, m_sms0, streamName, inputFileName);
			}
		} else if (1 == channel && m_sms1 == NULL) {
			if (m_stream_started[1] == 0) {
				char const* streamName = "stream1";
				char const* inputFileName = "stream1";

				m_sms1 = ServerMediaSession::createNew(*env, streamName, NULL, descriptionString, False);
				m_subsession_h264_video1 = H265VideoServerMediaSubsession::createNew(*env, 1, &streamSource);
				// m_subsession_h264_video1->setStreamType(m_stream_type[1]);
				m_sms1->addSubsession(m_subsession_h264_video1);
				rtspServer->addServerMediaSession(m_sms1);
				m_stream_started[1] = 1;
				announceStream(rtspServer, m_sms1, streamName, inputFileName);
			}
		}  
#ifdef XCAM_DOUBLE_SENSOR
		 else if(3 == channel && m_sms3 == NULL) {
				LOG << "RTSP start channel " << channel << endl;
				char const* streamName = "stream3";
				char const* inputFileName = "stream3";
				m_sms3 = ServerMediaSession::createNew(*env, streamName, NULL, descriptionString, False);
				m_subsession_h264_video3 = H265VideoServerMediaSubsession::createNew(*env, 3, &streamSource);	
				m_sms3->addSubsession(m_subsession_h264_video3);
				rtspServer->addServerMediaSession(m_sms3);
				m_stream_started[3] = 1;
				announceStream(rtspServer, m_sms3, streamName, inputFileName);
		}  
#endif		
		else {
			LOG << "error channel " << channel << endl;
		}
		m_channel_stat[channel] = LIVE_RTSP_CHANNEL_STAT_START;
	} else {
		if (0 == channel) {
			// m_subsession_h264_video0->setStreamType(m_stream_type[0]);
			// m_subsession_h264_video0->updateSdpLines();
			streamSource.streamClose(channel);
		} else if (1 == channel) {
			// m_subsession_h264_video1->setStreamType(m_stream_type[1]);
			// m_subsession_h264_video1->updateSdpLines();
			streamSource.streamClose(channel);
		} 
#ifdef XCAM_DOUBLE_SENSOR
		else if(3 == channel) {
			streamSource.streamClose(channel);
		}
#endif
		else {
			LOG << "error channel " << channel << endl;
		}
		usleep(1*1000*1000);
	}
	LOG << "error channel " << channel << endl;
	return 0;
}

int LiveRTSP::stopRTSP(int channel)
{
#if 0
	if (0 == channel) {
		// m_subsession_h264_video0->setStreamType(m_stream_type[0]);
		// m_subsession_h264_video0->updateSdpLines();
		streamSource.streamClose(channel);
	} else if (1 == channel) {
		// m_subsession_h264_video1->setStreamType(m_stream_type[1]);
		// m_subsession_h264_video1->updateSdpLines();
		streamSource.streamClose(channel);
	} 
#ifdef XCAM_DOUBLE_SENSOR
	else if(3 == channel) {
		streamSource.streamClose(channel);
	}
#endif	
	else {
		LOG << "error channel " << channel << endl;
	}

	usleep(1*1000*1000);
#endif
LOG << "error channel " << channel << endl;
	return 0;
}

int LiveRTSP::setStreamTypeAndRun(int channel, int type)
{
	if (0 == channel) {
		m_stream_type[0] = type;
		// m_subsession_h264_video0->setStreamType(type);
		// m_subsession_h264_video0->updateSdpLines();
		streamSource.streamClose(channel);
	} else if (1 == channel) {
		m_stream_type[1] = type;
		// m_subsession_h264_video1->setStreamType(type);
		// m_subsession_h264_video1->updateSdpLines();
		streamSource.streamClose(channel);
	} else {
		LOG << "error channel " << channel << endl;
	}
	usleep(1*1000*1000);
	return 0;
}

void LiveRTSP::event_stopRTSPImplCh0(void* clientData) {
	LOG << "RTSP stop channel impl ch0"  << endl;
	LiveRTSP* rtsp = (LiveRTSP*)clientData;
	rtsp->rtspServer->deleteServerMediaSession(rtsp->m_sms0);
	//rtsp->rtspServer->removeServerMediaSession(rtsp->m_sms0);
	rtsp->m_sms0->deleteAllSubsessions();
	rtsp->m_channel_stat[0] = LIVE_RTSP_CHANNEL_STAT_STOP;
	rtsp->m_ch0_stop_done = 1;
	LOG << "RTSP stop channel impl ch0 done"  << endl;
}

void LiveRTSP::event_stopRTSPImplCh1(void* clientData) {
	LOG << "RTSP stop channel impl ch1"  << endl;
	LiveRTSP* rtsp = (LiveRTSP*)clientData;
	rtsp->rtspServer->deleteServerMediaSession(rtsp->m_sms1);
	//rtsp->rtspServer->removeServerMediaSession(rtsp->m_sms1);
	rtsp->m_sms1->deleteAllSubsessions();
	rtsp->m_channel_stat[1] = LIVE_RTSP_CHANNEL_STAT_STOP;
	rtsp->m_ch1_stop_done = 1;
	LOG << "RTSP stop channel impl ch1 done"  << endl;
}

void LiveRTSP::event_stopRTSPImplCh3(void* clientData) {
	LOG << "RTSP stop channel impl ch3"  << endl;
	LiveRTSP* rtsp = (LiveRTSP*)clientData;
	rtsp->rtspServer->deleteServerMediaSession(rtsp->m_sms3);
	//rtsp->rtspServer->removeServerMediaSession(rtsp->m_sms3);
	rtsp->m_sms3->deleteAllSubsessions();
	rtsp->m_channel_stat[3] = LIVE_RTSP_CHANNEL_STAT_STOP;
	rtsp->m_ch3_stop_done = 1;
	LOG << "RTSP stop channel impl ch3 done"  << endl;
}

int LiveRTSP::setStreamType(int channel, int type)
{
	m_stream_type[channel] = type;
	return 0;
}

int LiveRTSP::getStreamType(int channel)
{
	return m_stream_type[channel];
}

int  LiveRTSP::putFrame(int channel, char *buf, int size, int64_t ts)
{
	return streamSource.streamPutFrame(channel, buf, size, ts);
}

int  LiveRTSP::putAudioFrame(int channel, char *buf, int size, int64_t ts)
{

		// return m_audiossbsession_g711->g_g711Source->putFrameImpl(buf, size, ts);
		return 0;
}

/* c interface wrapper */
int c_RTSP_init()
{
	return liveRTSP.initEnv();
}

int c_RTSP_set_stream_type(int channel, int type)
{
	return liveRTSP.setStreamType(channel, type);
}

int c_RTSP_start(int channel, int width, int height)
{
	return liveRTSP.startRTSP(channel, width, height);
}

int c_RTSP_stop(int channel)
{
	return liveRTSP.stopRTSP(channel);
}

int c_RTSP_put_source_frame(int channel, char *buf, int size, int64_t ts)
{
	struct timeval now;
    gettimeofday(&now, NULL);
    ts = (now.tv_sec * 1000000LL + now.tv_usec); // 统一时间基准
	// audioflag = 1;
	// 	if(my)
	// 		my->do_it();
	return liveRTSP.putFrame(channel, buf, size, ts);
}
int c_RTSP_put_source_audioframe(int channel, uint8_t *buf, int size, int ts)
{
	struct timeval now;
    gettimeofday(&now, NULL);
    ts = (now.tv_sec * 1000000LL + now.tv_usec); // 统一时间基准
	if(my && audiodesflag == 0 ){
		audioflag = 1 ;
				
		memcpy(audiodata, buf, size);
		//  printf("%s[%d]  xxxx msgid=%d  sep=%d\n", __func__, __LINE__,  size, ts);
		// my->do_it(ts);
		}	
	// }
	// return liveRTSP.putAudioFrame(channel, buf, size, ts);
	return 0;
}
