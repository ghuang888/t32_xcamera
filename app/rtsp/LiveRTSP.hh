#ifndef LIVE_RTSP_HH
#define LIVE_RTSP_HH

#include <thread>
#include "liveMedia.hh"
#include "BasicUsageEnvironment.hh"
#include "H264VideoServerMediaSubsession.hh"
#include "H265VideoServerMediaSubsession.hh"
#include "G711AudioStreamServerMediaSubsession.hh"

#define LIVE_RTSP_CHANNEL_MAX 4

enum {
	LIVE_RTSP_CHANNEL_STAT_STOP = 0,
	LIVE_RTSP_CHANNEL_STAT_START
};

/* defined in c_liveRTSP.h */
/*
enum {
	LIVE_RTSP_STREAM_H264,
	LIVE_RTSP_STREAM_H265
};
*/

class LiveRTSP {

public:
    static LiveRTSP* createNew();
    LiveRTSP();
    virtual ~LiveRTSP();


private:
	std::thread* m_work_thread;
	static int runWorkThread(LiveRTSP* obj);
    int workThread(void);

	int m_stream_type[LIVE_RTSP_CHANNEL_MAX];
	int m_channel_stat[LIVE_RTSP_CHANNEL_MAX];
	int m_stream_started[LIVE_RTSP_CHANNEL_MAX];

public:

    void announceStream(RTSPServer* rtspServer, ServerMediaSession* sms, char const* streamName, char const* inputFileName);

	int initEnv(void);
	int startRTSP(int channel, int width, int height);
	int stopRTSP(int channel);
	int setStreamTypeAndRun(int channel, int type);
	int setStreamType(int channel, int type);
	int getStreamType(int channel);

	int putFrame(int channel, char *buf, int size, int64_t ts);
	int putAudioFrame(int channel, char *buf, int size, int64_t ts);

	static void event_stopRTSPImplCh0(void* clientData);
	static void event_stopRTSPImplCh1(void* clientData);
	static void event_stopRTSPImplCh3(void* clientData);
	EventTriggerId eventCloseStreamCh0;
	EventTriggerId eventCloseStreamCh1;
	EventTriggerId eventCloseStreamCh3;
	char m_ch0_stop_done;
	char m_ch1_stop_done;
	char m_ch3_stop_done;
	H265VideoServerMediaSubsession* m_subsession_h264_video0;
    H265VideoServerMediaSubsession* m_subsession_h264_video1;
	H265VideoServerMediaSubsession* m_subsession_h264_video3;
protected:


    // To make the second and subsequent client for each stream reuse the same
    // input stream as the first client (rather than playing the file from the
    // start for each client), change the following "False" to "True":
    Boolean reuseFirstSource;

    // To stream *only* MPEG-1 or 2 video "I" frames
    // (e.g., to reduce network bandwidth),
    // change the following "False" to "True":
    Boolean iFramesOnly;



	H265VideoServerMediaSubsession* m_subsession_h265_video0;
    H265VideoServerMediaSubsession* m_subsession_h265_video1;
	H265VideoServerMediaSubsession* m_subsession_h265_video3;

	G711AudioStreamServerMediaSubsession* m_audiossbsession_g711;

    ServerMediaSession* m_sms0;
    ServerMediaSession* m_sms1;
	ServerMediaSession* m_sms3;

    UsageEnvironment* env;
    UserAuthenticationDatabase* authDB;
    RTSPServer* rtspServer;

};

#endif
