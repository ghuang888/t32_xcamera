#ifndef _XCAM_GENERAL_H_
#define _XCAM_GENERAL_H_

void cJson_process_init(void);
int xcam_json_get_video_isp_fps(void *json_root);
int xcam_json_set_video_isp_fps(void *json_root);
int xcam_json_get_video_fs_fps(void *json_root);
int xcam_json_set_video_fs_fps(void *json_root);
int xcam_json_get_video_enc_fps(void *json_root);
int xcam_json_set_video_enc_fps(void *json_root);
int xcam_json_get_video_bps(void *json_root);
int xcam_json_set_video_bps(void *json_root);
int xcam_json_get_video_resolution(void *json_root);
int xcam_json_set_video_resolution(void *json_root);
int xcam_json_get_video_encode_mode(void *json_root);
int xcam_json_set_video_encode_mode(void *json_root);
int xcam_json_get_sys_ipconfig(void *json_root);
int xcam_json_set_sys_ipconfig(void *json_root);
int xcam_json_get_video_Iframe_interval(void *json_root);
int xcam_json_set_video_Iframe_interval(void *json_root);
int xcam_json_get_video_Rcmode(void *json_root);
int xcam_json_set_video_Rcmode(void *json_root);
int xcam_json_get_web_http_port(void *json_root);
int xcam_json_set_web_http_port(void *json_root);
int xcam_json_get_channel_switch(void *json_root);
int xcam_json_set_channel_switch(void *json_root);
int xcam_json_get_rtsp_addr(void *json_root);
int xcam_json_get_device_info(void *json_root);

#endif
