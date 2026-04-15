/*
 *
 *
 * */

#ifndef __SVAC_INTERFACES_H__
#define __SVAC_INTERFACES_H__

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "sipsession.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define SVAC_TYPE_VIDEO_H264 0x1
#define SVAC_TYPE_VIDEO_H265 0x2
#define SVAC_TYPE_VIDEO_SVAC 0x3
#define SVAC_TYPE_AUDIO_LPCM 0x4
#define SVAC_TYPE_AUDIO_G711A 0x5
#define SVAC_TYPE_AUDIO_SVAC 0x6

#define CONTROL_MOVE_LEFT 0x01
#define CONTROL_MOVE_RIGHT 0x02
#define CONTROL_MOVE_UP 0x03
#define CONTROL_MOVE_DOWN 0x04
#define CONTROL_ZOOM_IN 0x05
#define CONTROL_ZOOM_OUT 0x06
#define CONTROL_FOCUS_NEAR 0x07
#define CONTROL_FOCUS_FAR 0x08
#define CONTROL_APERTURE_LARGE 0x09
#define CONTROL_APERTURE_SMALL 0x0A
#define CONTROL_DRAG_ZOOM_IN 0x0B
#define CONTROL_DRAG_ZOOM_OUT 0x0C

struct svac_videoparam {
	int video_chn;
	int video_type;
	int width;
	int height;
	int bitrate_mode;
	int bitrate;
	int maxbitrate;
	int fps_num;
	int fps_den;
	int svc_space_mode;
	int svc_time_mode;
};

struct svac_audioparam {
	int audio_type;
	unsigned short bitwidth;
	unsigned short channels;
	int samplerate;
	int bitrate;
};

/* video and audio */
struct svac_video_frame {
	void *addr;
	unsigned int length;
	int64_t timestamp;
	unsigned int idr; /* !0 means that the frame is IDR, 0 isn't IDR */
};

struct svac_audio_frame {
	void *addr;
	unsigned int length;
	int64_t timestamp;
};

struct svac_device_base_attr {
	int name_len;
	char *name;							/**<设备名称 */
	int manufacturer_len;
	char *manufacturer;					/**<设备厂商 */
	char model_len;
	char *model;						/**<设备型号 */
	int owner_len;
	char *owner;							/**<设备归属 */
	int civilcode_len;
	char *civilcode;						/**<行政区域*/
	int block_len;
	char *block;							/**<警区 */
	int address_len;
	char *address;					/**<设备安装地址 */
	int parentid_len;
	char *parentid;					/**<父设备编码 */
	double longitude;					/**<经度 */
	double latitude;					/**<纬度 */
	ipc_type_t ptzType;						/**<摄像机类型拓展 补*/
	ipc_position_type_t positionType;					/**<摄像机位置类型 补*/
	ipc_room_type_t roomType;						/**<摄像机室内室外属性 补*/
	ipc_use_type_t useType;						/**<摄像机用途属性 补*/
	ipc_supplylight_type_t supplylightType;				/**<摄像机补光属性 补*/
	ipc_direction_type_t directionType;					/**<摄像机监视方位属性 补*/
	int groupID_len;
	char *groupID;						/**<虚拟组织所属业务分组ID 补*/
};

struct svac_device_sip_attr {
	int sipswitch;
	int serverid_len;
	char *serverid;						/**<SIP服务器ID*/
	int serverip_len;
	char *serverip;						/**<SIP服务器IP*/
	int serverport;						/**<SIP服务器端口*/
	int logindomain_len;
	char *logindomain;					/**<SIP系统编码*/
	char *deviceid;
	int deviceid_len;
	int localport;						/**<设备SIP端口*/
	int authpasswd_len;
	char *authpasswd;					/**<认证密码*/
	auth_mode_t authmode;				/**<注册模式*/
	trans_protocel_t transtype;			/**<使用网络协议*/
	int expires;						/**<刷新注册间隔时间 单位秒*/
	int hbinterval;						/**<状态报送间隔时间 单位秒*/
	int hbcount;
	int mediaport;						/**<设备媒体数据端口*/
};

struct svac_encoder_attr {
	int timeFlag;							/**<是否包括绝对时间信息扩展单元*/
	int eventFlag;							/**<是否支持监控事件信息单元*/
	int alertFlag;							/**<是否支持报警信息单元*/
	int roiFlag;							/**<是否支持感兴趣区域功能设置*/
	svc_type_t spaceDomainMode;					/**<空域编码方式*/
	svc_type_t timeDomainMode;						/**<时域编码方式*/
	svc_type_t svcSpaceSupportMode;				/**<空域编码能力 补*/
	svc_type_t svcTimeSupportMode;					/**<时域编码能力 补*/
};

struct svac_device_encryption_attr {
	void *private_key;					/**<私钥*/
	int prikey_len;						/**<私钥长度*/
	void *public_key;					/**<公钥*/
	int pubkey_len;						/**<公钥长度*/
	secrecy_type_t secrecy;				/**<设备保密属性 补*/
	int certnum_len;
	char *certnum;						/**<证书序列号 */
	int certendtime_len;
	char *certendtime;					/**<证书终止有效期 */
};

struct security_paramters {
	unsigned char encryption_type;			/**<加密所用算法:0:SM1 1:SM4*/
	unsigned char vek_flag;                 /**<是否携带vek*/
	unsigned char iv_flag;					/**<是否携带IV*/
	unsigned char vek_encryption_type;		/**<密钥加密所采用的算法*/
	unsigned char vkek_version[64];	        /**<加密密钥版本号*/
	unsigned char hash_type;				/**<认证采用算法:0:SM3*/
	unsigned char signature_type;			/**<对图像摘要数据进行签名算法：0:SM2*/
	unsigned char iv_gopcycle;				/**<IV更新周期*/
	unsigned char vek_gopcycle;				/**<VEK更新周期*/
	unsigned char iv_length;
	unsigned char evek_length;
	unsigned char vkek_version_length;      /**<加密密钥版本号长度*/
	unsigned int vek_length;
};


/*
 * type: encryption type
 * return: 0 is ok; !0 is error.
 * dstlen < srclen + 128
 * */
typedef int (*SVACEncryptCallback_t)(unsigned char type, char *src, unsigned int srclen, char *dst, unsigned int *dstlen,
													const unsigned char *vek, const unsigned char *iv);

/*
 * htype: hash type
 * stype: signature type
 * key: signature key
 * return: 0 is ok; !0 is error.
 * dstlen < 128 bytes
 * */
typedef int (*SVACSignatureCallback_t)(unsigned char htype, unsigned char stype, char *src, unsigned int len,
														char *dst, int *dstlen);

typedef int (*SVACGetkeysCallback_t)(char *vek,char *evek);

typedef int (*SVACGetivCallback_t)(char *iv);



/* the return value of these callbacks is defined as follow:
 * 	0 is successfully
 * 	!0 is error.
 * 	*/
struct svac_manage_callbacks {
	/*reboot*/
	int (*reboot_now)(void);
	/* calibrate time*/
	void (*calibrate_time)(time_t time);
	/*video realplay*/
	int (*set_video_stream)(struct svac_videoparam *video);
	int (*start_video_stream)(unsigned int chn);
	int (*stop_video_stream)(unsigned int chn);

	int (*send_Iframe)(unsigned int index);
	int (*drag_zoom)(unsigned int index,int zoom_type,unsigned int length,unsigned int width,unsigned int midpointX,unsigned int midpointY,unsigned int lengthX,unsigned int lengthY);
	//audio
	int (*start_audio_stream)(unsigned int index);
	int (*stop_audio_stream)(unsigned int index);

	//osd
	int (*set_osd_window)(int length,int width);
	int (*set_osd_time)(int enable,int x,int y,int time_type);
	int (*set_osd_text)(int enable,int x,int y,char *text);

	/* max rtp's packetsize */
	int (*get_maxpacketsize)(unsigned int *size);

	/* get some status */

	/* these are callback of encrypting stream */
	/* camera_idc: It is a member of security_parameter_rbsp, its length is 152bytes
	 * camera_id: It is a member of security_parameter_rbsp, its length is 160bytes
	 * */
	int (*init_security_paramters)(int chn,struct security_paramters *sp, unsigned char *camera_idc, unsigned char *camera_id);
	int (*enable_encryption)(int chn,unsigned int enable, SVACEncryptCallback_t enc_fn,SVACGetkeysCallback_t key_fn,SVACGetivCallback_t iv_fn);
	int (*disable_encryption)(int chn);
	int (*enable_signature)(int chn,unsigned int enable, SVACSignatureCallback_t sig_fn);
	int (*disable_signature)(int chn);
	int (*update_security_paramters)(int chn,struct security_paramters *sp);

	/* device control*/
	//镜头移动和缩放
	int (*start_move)(int direction, unsigned char horizontal_speed, unsigned char vertical_speed);
	int (*adjust_zoom)(int zoom_type,unsigned char zoom_speed);
	int (*adjust_focus)(int focus_type,unsigned char focus_speed);
	int (*adjust_aperture)(int aperture_type,unsigned char aperture_speed);
	//预置位
	int (*set_preset)(int index);
	int (*call_preset)(int index);
	int (*delete_preset)(int index);
	//巡航
	int (*add_patrol)(int patrol_id,int preset_index);
	int (*delete_patrol)(int patrol_id, int preset_index);
	int (*set_patrol_speed)(int patrol_id, int preset_index,unsigned int patrol_speed);
	int (*set_stay_time)(int patrol_id, int preset_index,unsigned int time);
	int (*start_patrol)(int patrol_id,int preset_index);
	//扫描
	int (*start_scan)(int scan_id,unsigned int scan_speed);
	int (*scan_left_border)(int scan_id,unsigned int scan_speed);
	int (*scan_right_border)(int scan_id,unsigned int scan_speed);
	int (*set_scan_speed)(int scan_id,unsigned int scan_speed);
	//看守位
	int (*open_homeposition)(unsigned int preset_index,unsigned int reset_time);
	int (*close_homeposition)(void);
	//报警复位
	int (*alarm_reset)(void);
	//PTZ精准控制
	int (*set_ptz_precise)(double pan,double tilt,double zoom);

	//停止以上所有控制
	int (*stop_control)(void);

	/*Record*/
	int (*start_record)(unsigned int index);
	int (*stop_record)(unsigned int index);

	/*Guard*/
	int (*start_guard)(unsigned int index);
	int (*stop_guard)(unsigned int index);
};

int svac_push_stream (void *handle, struct svac_video_frame *video, struct svac_audio_frame *audio);
void *svac_create_manage(unsigned int maxmemsize, unsigned int maxcacheframes, struct svac_manage_callbacks *callbacks,char *prikey_path,char *pubkey_path);
int svac_destroy_manage(void *handle);
int svac_start_manage(void *handle);
int svac_stop_manage(void *handle);
//video
int video_parameters_init(void *handle,unsigned int videos);
int set_video_parameter(void *handle,unsigned int video_chn,struct svac_videoparam *param);
int set_video_channel(void *handle,unsigned int video_chn);
//audio
int set_audio_parameters(void *handle, unsigned int audios, struct svac_audioparam *param);

int set_device_base_attr(void *handle, struct svac_device_base_attr *attr);
int set_device_sip_attr(void *handle, struct svac_device_sip_attr *attr);
int set_device_encryption_attr(void *handle, struct svac_device_encryption_attr *attr);
int set_device_encoder_attr(void *handle, struct svac_encoder_attr *attr);

#ifdef __cplusplus
}
#endif
#endif /* __SVAC_INTERFACES_H__ */
