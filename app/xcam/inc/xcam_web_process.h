#ifndef _XCAM_WEB_PROCESS_H_
#define _XCAM_WEB_PROCESS_H_
#include "xcam_com.h"
#include "../../daemon/xcam_daemon_protocol.h"
int xcam_web_socket_config_process(socket_tcp_link_t *plink);
int xcam_web_set_http_port(int port);
int xcam_web_set_video_nightvisionmode(int nigthvision);
int xcam_web_set_video_wdrmode(int wdr);
int xcam_web_set_video_imagemirrormode(int imagemirror);
int xcam_web_set_video_imageflipmode(int imageflip);
int xcam_web_set_video_ircutmode(int ircut);
int xcam_web_set_video_noisereductionmode(int noisereduction);
int xcam_web_set_video_drcmode(int drc);
int xcam_web_set_video_image_control(int saturation, int brightness, int sharpness, int contrast);
int xcam_web_set_video_image_control_extra(int *extra, int size);
int xcam_web_get_http_port(int *port);
#define EXTRA_ISPCTL_N 10

typedef enum {
    AWB = 0,
    GAMMA,
    FILLLIGHT,
    VISUALANGLE,
    BACKLIGHT,
    NIGHTMODE
} extra_web_param_type;
extra_web_param g_extra_web_param ; //全局isp参数配置
extern bool switch_ch0;
extern bool switch_ch1;
#endif
