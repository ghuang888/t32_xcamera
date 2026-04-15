
#include "H265VideoRTPSink.hh"
#include "H265VideoStreamDiscreteFramer.hh"
#include "H265VideoServerMediaSubsession.hh"
#include "H265VideoStreamSource.hh"

H265VideoServerMediaSubsession*
H265VideoServerMediaSubsession
::createNew(UsageEnvironment& env, int channel, StreamSource* ss) {
    return new H265VideoServerMediaSubsession(env, channel, ss);
}

H265VideoServerMediaSubsession ::H265VideoServerMediaSubsession(UsageEnvironment& env, int channel, StreamSource* ss)
	: OnDemandServerMediaSubsession(env, True/*reuse the first source*/), fAuxSDPLine(NULL), fDoneFlag(0), fDummyRTPSink(NULL) {
	envir() << __func__ << __LINE__ << "\n";
	m_channel = channel;
	m_ss = ss;
}

H265VideoServerMediaSubsession
::~H265VideoServerMediaSubsession() {
	envir() << __func__ << __LINE__ << "\n";
  delete[] fAuxSDPLine;
}


static void afterPlayingDummy(void* clientData) {
  H265VideoServerMediaSubsession* subsess = (H265VideoServerMediaSubsession*)clientData;
  subsess->afterPlayingDummy1();
}

void H265VideoServerMediaSubsession::afterPlayingDummy1() {
  // Unschedule any pending 'checking' task:
  envir().taskScheduler().unscheduleDelayedTask(nextTask());
  // Signal the event loop that we're done:
  setDoneFlag();
}

static void checkForAuxSDPLine(void* clientData) {
  H265VideoServerMediaSubsession* subsess = (H265VideoServerMediaSubsession*)clientData;
  subsess->checkForAuxSDPLine1();
}

void H265VideoServerMediaSubsession::checkForAuxSDPLine1() {
  nextTask() = NULL;

  char const* dasl;
  if (fAuxSDPLine != NULL) {
    // Signal the event loop that we're done:
    setDoneFlag();
  } else if (fDummyRTPSink != NULL && (dasl = fDummyRTPSink->auxSDPLine()) != NULL) {
    fAuxSDPLine = strDup(dasl);
    fDummyRTPSink = NULL;

    // Signal the event loop that we're done:
    setDoneFlag();
  } else if (!fDoneFlag) {
    // try again after a brief delay:
    int uSecsToDelay = 100000; // 100 ms
    nextTask() = envir().taskScheduler().scheduleDelayedTask(uSecsToDelay,
			      (TaskFunc*)checkForAuxSDPLine, this);
  }
}

char const* H265VideoServerMediaSubsession::getAuxSDPLine(RTPSink* rtpSink, FramedSource* inputSource) {
  if (fAuxSDPLine != NULL) return fAuxSDPLine; // it's already been set up (for a previous client)

  if (fDummyRTPSink == NULL) { // we're not already setting it up for another, concurrent stream
    // Note: For H265 video files, the 'config' information ("profile-level-id" and "sprop-parameter-sets") isn't known
    // until we start reading the file.  This means that "rtpSink"s "auxSDPLine()" will be NULL initially,
    // and we need to start reading data from our file until this changes.
    fDummyRTPSink = rtpSink;

    // Start reading the file:
    fDummyRTPSink->startPlaying(*inputSource, afterPlayingDummy, this);

    // Check whether the sink's 'auxSDPLine()' is ready:
    checkForAuxSDPLine(this);
  }

  envir().taskScheduler().doEventLoop(&fDoneFlag);

  return fAuxSDPLine;
}

FramedSource* H265VideoServerMediaSubsession::createNewStreamSource(unsigned /*clientSessionId*/, unsigned& estBitrate) {
	estBitrate = 8*1000/*kbps*/;
    // Create a framer for the Video Elementary Stream:
    //envir() <<__func__<<"-"<<__LINE__<<":"<< "\n";
    return H265VideoStreamDiscreteFramer::createNew(envir(), H265VideoStreamSource::createNew(envir(), m_channel, m_ss));
}

RTPSink* H265VideoServerMediaSubsession
::createNewRTPSink(Groupsock* rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic, FramedSource* /*inputSource*/) {
    OutPacketBuffer::maxSize = 2*1024*1024;
    return H265VideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic);
}
