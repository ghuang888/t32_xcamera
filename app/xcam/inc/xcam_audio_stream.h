#ifndef _XCAM_AUDIO_STREAM_H__
#define _XCAM_AUDIO_STREAM_H__
#include <imp/imp_audio.h>
#include <imp/imp_log.h>

typedef struct{
  IMPAudioFrame frm;
  int handle_g711a;
  int handle_g711u;
  int AeChn;
  int devID;
  int chnID;
  int playdevID;
  int playchnID;
  int handle_g711ud;
  int adChn;
}xcam_audio_stream;

#endif