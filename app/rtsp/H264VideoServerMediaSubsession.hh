#ifndef H264_VIDEO_SERVER_MEDIA_SUBSESSION_HH
#define H264_VIDEO_SERVER_MEDIA_SUBSESSION_HH

#include "OnDemandServerMediaSubsession.hh"

class StreamSource;

class H264VideoServerMediaSubsession: public OnDemandServerMediaSubsession {
public:
 static H264VideoServerMediaSubsession* createNew(UsageEnvironment& env, int channel, StreamSource* ss, int width, int height);
  // Used to implement "getAuxSDPLine()":
  void checkForAuxSDPLine1();
  void afterPlayingDummy1();
  int updateSdpLines();
  //char const* sdpLines();
  int setStreamType(int streamtype);

protected:
  H264VideoServerMediaSubsession(UsageEnvironment& env, int channel, StreamSource* ss, int width, int height);
  virtual ~H264VideoServerMediaSubsession();

  void setDoneFlag() { fDoneFlag = ~0; }
protected:

  virtual char const* getAuxSDPLine(RTPSink* rtpSink,
				    FramedSource* inputSource);

  virtual FramedSource* createNewStreamSource(unsigned clientSessionId, unsigned& estBitrate);
  virtual RTPSink* createNewRTPSink(Groupsock* rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic, FramedSource* inputSource);

private:
  int m_channel;
  StreamSource* m_ss;
  int m_stream_type;

  char* fAuxSDPLine;
  char fDoneFlag; // used when setting up "fAuxSDPLine"
  RTPSink* fDummyRTPSink; // ditto
};

#endif
