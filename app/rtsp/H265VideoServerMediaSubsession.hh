
#ifndef H265_VIDEO_SERVER_MEDIA_SUBSESSION_HH
#define H265_VIDEO_SERVER_MEDIA_SUBSESSION_HH

#include "OnDemandServerMediaSubsession.hh"
#include "H265VideoStreamSource.hh"
#include "video_source.h"

class H265VideoServerMediaSubsession: public OnDemandServerMediaSubsession {
public:
  static H265VideoServerMediaSubsession* createNew(UsageEnvironment& env, int channel, StreamSource* ss);


  // Used to implement "getAuxSDPLine()":
  void checkForAuxSDPLine1();
  void afterPlayingDummy1();

protected:
  H265VideoServerMediaSubsession(UsageEnvironment& env, int channel, StreamSource* ss);
  virtual ~H265VideoServerMediaSubsession();

  void setDoneFlag() { fDoneFlag = ~0; }
protected:

  virtual char const* getAuxSDPLine(RTPSink* rtpSink,
				    FramedSource* inputSource);

  virtual FramedSource* createNewStreamSource(unsigned clientSessionId, unsigned& estBitrate);
  virtual RTPSink* createNewRTPSink(Groupsock* rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic, FramedSource* inputSource);
private:
  int m_channel;
  StreamSource* m_ss;

  char* fAuxSDPLine;
  char fDoneFlag; // used when setting up "fAuxSDPLine"
  RTPSink* fDummyRTPSink; // ditto
};

#endif
