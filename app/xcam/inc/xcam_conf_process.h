#ifndef _XCAM_CONF_PROCESS_H_
#define _XCAM_CONF_PROCESS_H_

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
int xcam_json_get_video_ispcontrolInfo(void *json_root);
int xcam_json_set_video_ispcontrolInfo(void *json_root);
int xcam_json_get_video_image_control(void *json_root);
int xcam_json_set_video_image_control(void *json_root);
int xcam_json_get_sys_ipconfig(void *json_root);
int xcam_json_set_sys_ipconfig(void *json_root);
int xcam_json_get_device_info(void *json_root);
int xcam_json_set_video_Rcmode(void *json_root);
int xcam_json_set_video_Iframe_interval(void *json_root);
int xcam_json_set_web_http_port(void *json_root);
int xcam_json_get_video_Rcmode(void *json_root);
int xcam_json_get_web_http_port(void *json_root);
int xcam_json_get_channel_switch(void *json_root);
int xcam_json_get_rtsp_addr(void *json_root);
int xcam_json_get_video_Iframe_interval(void *json_root);
int xcam_json_get_video_qp(void *json_root);
int xcam_json_get_isp_awbAttr(void *json_root);
int xcam_json_get_video_encoder_osd(void *json_root);
int xcam_json_set_video_qp(void *json_root);
int xcam_json_set_encoder_snap(void *json_root);
int xcam_json_set_isp_awbAttr(void *json_root);
int xcam_json_set_encoder_osd(void *json_root);
#endif
