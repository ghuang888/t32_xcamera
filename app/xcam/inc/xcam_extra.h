#ifndef _XCAM_VIDEO_H_
#define _XCAM_VIDEO_H_

#include <imp/imp_log.h>
#include <imp/imp_common.h>
#include <imp/imp_system.h>
#include <imp/imp_isp.h>
#include <imp/imp_framesource.h>
#include <imp/imp_encoder.h>

typedef  int (*IvsTaskEnter)(void);
typedef struct IvsTask{
	IvsTaskEnter enter;
	char         *ivsTaskNam;
}IvsTask;
pthread_mutex_t xcam_snapframe_mutex;
#endif //_XCAM_VIDEO_H_