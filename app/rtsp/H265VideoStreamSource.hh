#ifndef H265VIDEOSTREAMSOURCE_HH
#define H265VIDEOSTREAMSOURCE_HH

#include <mutex>
#include <condition_variable>
#include <thread>
#include "FramedSource.hh"

using namespace std;

class StreamSource;

class H265VideoStreamSource: public FramedSource {
public:
  static H265VideoStreamSource* createNew(UsageEnvironment& env, int channel, StreamSource* ss);
  H265VideoStreamSource(UsageEnvironment& env, int channel, StreamSource* ss);
  // called only by createNew()
  virtual ~H265VideoStreamSource();

  static void event_putFrame(void* clientData);
  void event_putFrameImpl();

  static void putFrame(void* obj, char* buf, int size, int64_t ts);
  void putFrameImpl(char* buf, int size, int64_t ts);

  static void sourceClose(void* obj);
  void sourceCloseImpl();

  EventTriggerId eventTriggerId;

private:
  virtual void doGetNextFrame();
  unsigned fLastPlayTime;

  int m_channel;
  StreamSource* m_ss;

  mutex m_mutex;
  condition_variable m_cond;

  int m_frame_done;
  int m_source_close;

};

#endif
