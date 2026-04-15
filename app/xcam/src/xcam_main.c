#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <assert.h>
#include <signal.h>
#include <imp/imp_log.h>
#include <imp/imp_common.h>
#include <imp/imp_system.h>
#include <imp/imp_isp.h>
#include <xcam_thread.h>
#include "xcam_conf_sys.h"
#include "xcam_com.h"
#include "xcam_module.h"
#include "xcam_rtsp.h"
#include "xcam_onvif.h"
#include "xcam_stream.h"
#include "xcam_log.h"
#include "xcam_osd.h"
#include "xcam_video.h"
#include "xcam_general.h"
#if((defined GB28181) || (defined GB35114))
#include "xcam_gb.h"
#endif
#include "xcam_conf_process.h"
#include "xcam_cli_options.h"
#include "xcam_system.h"
#include "xcam_message_queue.h"
#include "conf_process.h"
#include "xcam_ptz.h"

#define LOG_TAG "MAIN"

struct af_definition {
	int frameNum;
	int definValue;
};
struct af_definition afDefinInfo;
int (*pFunc_af_get_hist)(struct af_definition *afDefin);
int xcam_gekko_stopanddeinit();
int xcam_conf_get_extraargaiisp_status(int*);
void rcf_process_init();
int xcam_func_af_get_hist(struct af_definition *afDefin);
void Stop(int);

char* start_str =                  \
    "\n __  __                      \n"\
    " \\ \\/ /___ __ _ _ __ ___     \n"\
    "  \\  // __/ _` | '_ ` _ \\    \n"\
    "  /  \\ (_| (_| | | | | | |    \n"\
    " /_/\\_\\___\\__,_|_| |_| |_|  \n\n";

void pthread_param_init(void)
{
    pthread_mutex_init(&g_conf_mutex_t, NULL);
    return ;
}

void Stop(int signum)
{
	printf("监测ctrl c信号 %d，程序退出......\n", signum);
#ifdef AIISP
	int enable;
	xcam_conf_get_extraargaiisp_status(&enable);
	if(enable)
		xcam_gekko_stopanddeinit();
#endif
	exit(0);
	return ;
}
int main(int argc, char** argv)
{
	signal(SIGINT, Stop);
    int ret = XCAM_SUCCESS;

	// Step.1 printf xcamera logo
    printf("%s", start_str);

    pthread_param_init();

    ret = xcam_cli_check_args(argc, argv);
    if (ret != XCAM_SUCCESS) {
		printf("error return\n");
        return XCAM_ERROR;
    }

	cJson_process_init();

    rcf_process_init();
    msg_process_init();

	xcam_video_init();

    xcam_osd_init();

	// onvif_init();
	// xcam_audio_stream_init();
	/* stream channel 0 */
	stream_module_create(XCAM_STREAM_CHANNEL_0, XMOD_ID_VSTREAM_0);
	rtsp_module_create(XCAM_STREAM_CHANNEL_0, XMOD_ID_RTSP_0);
	xcam_module_register_wanted_msg(XMOD_ID_VSTREAM_0, XMOD_ID_RTSP_0, MESG_ID_STREAM0);

	/* stream channel 1 */
	stream_module_create(XCAM_STREAM_CHANNEL_1, XMOD_ID_VSTREAM_1);
	rtsp_module_create(XCAM_STREAM_CHANNEL_1, XMOD_ID_RTSP_1);
	xcam_module_register_wanted_msg(XMOD_ID_VSTREAM_1, XMOD_ID_RTSP_1, MESG_ID_STREAM1);
	
#ifdef XCAM_DOUBLE_SENSOR
	stream_module_create(XCAM_STREAM_CHANNEL_3, XMOD_ID_VSTREAM_3);
	rtsp_module_create(XCAM_STREAM_CHANNEL_3, XMOD_ID_RTSP_3);
	xcam_module_register_wanted_msg(XMOD_ID_VSTREAM_3, XMOD_ID_RTSP_3, MESG_ID_STREAM3);
#endif

	IMP_ISP_EnableTuning();

	xcam_system_start_boa();
	(void)xcam_message_receive();
	xcam_thread_set_name("xcam_main");
	while (1) {
		usleep(1000*1000);
	}

	return XCAM_SUCCESS;
}
