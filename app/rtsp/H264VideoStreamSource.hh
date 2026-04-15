#ifndef H264_VIDEO_STREAMSOURCE_HH
#define H264_VIDEO_STREAMSOURCE_HH

#include <mutex>
#include <condition_variable>
#include <thread>
#include "FramedSource.hh"

using namespace std;

class StreamSource;

class H264VideoStreamSource: public FramedSource {
public:
  static H264VideoStreamSource* createNew(UsageEnvironment& env, int channel, StreamSource* ss);
  H264VideoStreamSource(UsageEnvironment& env, int channel, StreamSource* ss);
  // called only by createNew()
  virtual ~H264VideoStreamSource();

  static void event_putFrame(void* clientData);
  void event_putFrameImpl();

  static void putFrame(void* obj, char* buf, int size, int64_t ts);
  void putFrameImpl(char* buf, int size, int64_t ts);

  static void sourceClose(void* obj);
  void sourceCloseImpl();


private:
  virtual void doGetNextFrame();
  unsigned fLastPlayTime;

  EventTriggerId eventTriggerId;

  int m_channel;
  StreamSource* m_ss;

  mutex m_mutex;
  condition_variable m_cond;

  int m_frame_done;
  int m_source_close;
  char* m_frame_buffer;
  int m_frame_size;

  pthread_mutex_t _mutex;
  pthread_cond_t _cond;

};

#endif
