#ifndef __SIP_SESSION_H__
#define __SIP_SESSION_H__

#define SIP_EVENT_WITH_XML 0X100
#define SIP_EVENT_WITH_SDP 0X400
#define SIP_EVENT_AUTH_SUCCESS 0x800
#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Sip MESSAGE Management
 * @struct sipSession
 */
struct sipSession_t;

/**
 * Structure for sip session parameter description.
 * @var sipSession_param_t
 */
typedef struct sipSession_param sipSession_param_t;

/**
 * Structure for authenticate success parameter description.
 * @var auth_success_t
 */
typedef struct auth_param auth_param_t;

/**
 * Structure for mansdp description.
 * @var mansdp_playload_t
 */
typedef struct mansdp_payload mansdp_payload_t;

/**
 * Structure for playback control description.
 * @var playback_ctrl_t
 */
typedef struct playback_ctrl playback_ctrl_t;

/**
 * Structure for sip event description.
 * @var sip_event_t
 */
typedef struct sip_event sip_event_t;

/**
 * Structure for authenticate mode description
 * @var auto_mode_t
 */
typedef enum {
	SIP_AUTH_MODE_GB28181_UNIDIRECTION, /**<GB28181单向认证*/
	SIP_AUTH_MODE_GB28181_BIDIRECTION,  /**<GB28181双向认证*/
	SIP_AUTH_MODE_GB35114_UNIDIRECTION, /**<GB35114单向认证*/
	SIP_AUTH_MODE_GB35114_BIDIRECTION,  /**<GB35114双向认证*/
}auth_mode_t;

/**
 * Structure for transport protocel description
 * @var trans_protocel_t
 */
typedef enum {
	TRANS_TYPE_UDP,  /*UDP*/
	TRANS_TYPE_TCP,  /*TCP*/
}trans_protocel_t;

typedef enum
{
	PARENTAL_TYPE_NO,   /**<不存在子设备*/
	PARENTAL_TYPE_YES,  /**<存在子设备*/
}parent_type_t;

typedef enum{
	SECRECY_NOT_CLASSIFIED,         /**<不涉密*/
	SECRECY_CLASSIFIED,             /**<涉密*/
}secrecy_type_t;

typedef enum{
	IPC_TYPE_BALLHEAD_CAMERA = 1,   /**<球机*/
	IPC_TYPE_HALF_BALLHEAD_CAMERA,  /**<半球*/
	IPC_TYPE_FIXED_GUN_CAMERA,      /**<固定枪机*/
	IPC_TYPE_REMOTE_CONTROL_CAMERA, /**<遥控枪机*/
}ipc_type_t;

typedef enum{
	IPC_POSITION_PROVINCIAL_CHECKPOINT = 1,/**<省际检查站*/
	IPC_POSITION_PARTY_GOVERNMENT_ORGANS,  /**<党政机关*/
	IPC_POSITION_STATION_WHARF,            /**<车站码头*/
	IPC_POSITION_CENTRAL_PLAZA,            /**<中心广场*/
	IPC_POSITION_SPORTS_VENUES,            /**<体育场馆*/
	IPC_POSITION_BUSINESS_CENTAL,          /**<商业中心*/
	IPC_POSITION_RELIGIOUS_VENUES,         /**<宗教场所*/
	IPC_POSITION_AROUND_CAMPUS,            /**<校园周边*/
	IPC_POSITION_SECURITY_COMPLEX_AREA,    /**<治安复杂区域*/
	IPC_POSITION_ARTERIAL_TRAFFIC,         /**<交通干线*/
}ipc_position_type_t;

typedef enum{
	IPC_ROOM_INDOOR = 1, /**<室内*/
	IPC_ROOM_OUTDOOR,    /**<室外*/
}ipc_room_type_t;

typedef enum{
	IPC_USE_SECURITY = 1,/**<治安*/
	IPC_USE_TRANSPORT,   /**<交通*/
	IPC_USE_KEY_POINT,   /**<重点*/
}ipc_use_type_t;

typedef enum{
	IPC_SUPPLY_LIGHT_NO = 1,       /**<无补光*/
	IPC_SUPPLY_LIGHT_INFRARED,     /**<红外补光*/
	IPC_SUPPLY_LIGHT_WHILT_LIGHT,  /**<白光补光*/
}ipc_supplylight_type_t;

typedef enum{
	IPC_DIRECTION_EAST = 1,    /**<东*/
	IPC_DIRECTION_WEST,        /**<西*/
	IPC_DIRECTION_SOUTH,       /**<南*/
	IPC_DIRECTION_NORTH,       /**<北*/
	IPC_DIRECTION_SOUTHEAST,   /**<东南*/
	IPC_DIRECTION_NORTHEAST,   /**<东北*/
	IPC_DIRECTION_SOUTHWEST,   /**<西南*/
	IPC_DIRECTION_NORTHWEST,   /**<西北*/
}ipc_direction_type_t;

typedef enum{
	SVC_TYPE_UNSUPPORT,       /**<不支持*/
	SVC_TYPE_ONE_ENHANMENT,   /**<一级增强*/
	SVC_TYPE_TWO_ENHANMENT,   /**<二级增强*/
	SVC_TYPE_THREE_ENHANMENT, /**<三级增强*/
}svc_type_t;

typedef enum status_type
{
	OFF = 0,
	ON = 1,
}status_type_t;
/**
 * Structure for sip session parameter description
 * @struct sip_session_param
 */
struct sipSession_param
{
	char *serverid;                              /**<SIP服务器ID*/
	char *serverip;                              /**<SIP服务器IP*/
	int serverport;                              /**<SIP服务器端口*/
	char *logindomain;                           /**<SIP系统编码*/
	char *clientid;                              /**<设备ID*/
	int clientport;                              /**<设备端口*/
	char *authpasswd;                            /**<认证密码*/
	auth_mode_t authmode;                        /**<注册模式*/
	trans_protocel_t transtype;                  /**<使用协议*/
	int expires;                                 /**<刷新注册间隔时间*/
	int hbinterval;                              /**<状态报送间隔时间*/
	char *name;                                  /**<设备名称 */
	char *manufacturer;                          /**<设备厂商 */
	char *model;                                 /**<设备型号 */
	char *firmware;                              /**<固件版本*/
	char *owner;                                 /**<设备归属 */
	char *civilcode;                             /**<行政区域*/
	char *block;                                 /**<警区 */
	char *address;                               /**<设备安装地址 */
	char *certnum;                               /**<证书序列号 */
	char *endtime;                               /**<证书终止有效期 */
	secrecy_type_t secrecy;                      /**<设备保密属性 */
	double longitude;                            /**<经度 */
	double latitude;                             /**<纬度 */
	ipc_type_t ipcType;                          /**<摄像机类型拓展 */
	ipc_position_type_t positionType;            /**<摄像机位置类型 */
	ipc_room_type_t roomType;                    /**<摄像机室内室外属性 */
	ipc_use_type_t useType;                      /**<摄像机用途属性 */
	ipc_supplylight_type_t supplylightType;      /**<摄像机补光属性 */
	ipc_direction_type_t directionType;          /**<摄像机监视方位属性 */
	char *resolution;                            /**<摄像机支持的分辨率 */
	char *businessgroupID;                       /**<虚拟组织所属业务分组ID */
	svc_type_t svcSpaceDomainMode;               /**<空域编码能力 */
	svc_type_t svcTimeDomainMode;                /**<时域编码能力 */
	svc_type_t svcSpaceSupportMode;              /**<空域编码能力 */
	svc_type_t svcTimeSupportMode;               /**<时域编码能力 */
	int channel;                                 /**<视频输入通道数*/
};

/**<*********************************MANSDP***********************************>*/
/**
 * Structure for mansdp origin description
 * @struct mansdp_origin
 */
struct mansdp_origin
{
	char *username;
	long long int sess_id;
	long long int sess_version;
	char *nettype;
	char *addrtype;
	char *addr;
};

/**
 * Structure for mansdp connection description
 * @struct mansdp_connection
 */
struct mansdp_connection
{
	char *nettype;
	char *addrtype;
	char *address;
};

/**
 * Structure for massdp bandwidth description
 * @struct mansdp_bandwidth
 */
struct mansdp_bandwidth {
	char *bwtype;
	char *bandwidth;
};

/**
 * Structure for massdp time description
 * @struct mansdp_time
 */
struct mansdp_time {
	time_t start_time;
	time_t stop_time;
};

/**
 * Structure for massdp media info description
 * @struct mansdp_info
 */
struct mansdp_info {
	char *type;
	int port;
	int port_n;
	char *proto;
	int fmt[10];
	int fmt_count;
};

typedef enum video_encode_format
{
	VIDEO_ENCODE_FORMAT_MP4 = 1,
	VIDEO_ENCODE_FORMAT_H264,
	VIDEO_ENCODE_FORMAT_SVAC,
	VIDEO_ENCODE_FORMAT_3GP,
}video_encode_format_t;

typedef enum video_resolution
{
	VIDEO_RESOLUTION_QCIF = 1,
	VIDEO_RESOLUTION_CIF,
	VIDEO_RESOLUTION_D1,
	VIDEO_RESOLUTION_720P,
	VIDEO_RESOLUTION_1080P,
}video_resolution_t;

typedef enum video_bitrate_mode
{
	VIDEO_BITRATE_MODE_CBR = 1,
	VIDEO_BITRATE_MODE_VBR,
}video_bitrate_mode_t;

/**
 * Structure for mansdp media encode video description
 * @struct mansdp_video
 */
struct mansdp_video{
	video_encode_format_t encode_format; /**<编码格式*/
	video_resolution_t resolution;       /**<分辨率*/
	unsigned int fps;                    /**<帧率*/
	video_bitrate_mode_t bitrate_type;   /**<码率类型*/
	unsigned int bitrate;                /**<比特率*/
};

typedef enum{
	AUDIO_ENCODE_FORMAT_G711 = 1,
	AUDIO_ENCODE_FORMAT_G723,
	AUDIO_ENCODE_FORMAT_G729,
	AUDIO_ENCODE_FORMAT_G722_1,
}audio_encode_format_t;

typedef enum{
	AUDIO_BITRATE_5_3KBPS=1,
	AUDIO_BITRATE_6_3KBPS,
	AUDIO_BITRATE_8KBPS,
	AUDIO_BITRATE_16KBPS,
	AUDIO_BITRATE_24KBPS,
	AUDIO_BITRATE_32KBPS,
	AUDIO_BITRATE_48KBPS,
	AUDIO_BITRATE_64KBPS,
}audio_bitrate_t;

typedef enum{
	AUDIO_SAMPLERATE_8KHZ=1,
	AUDIO_SAMPLERATE_14KHZ,
	AUDIO_SAMPLERATE_16KHZ,
	AUDIO_SAMPLERATE_32KHZ,
}audio_samplerate_t;

/**
 * Structure for mansdp media encode audio description
 * @struct mansdp_audio
 */
struct mansdp_audio{
	audio_encode_format_t encode_format;  /**<编码格式*/
	audio_bitrate_t bitrate;              /**<音频编码码率*/
	audio_samplerate_t sample_rate;       /**<采样率*/
};

/**
 * Structure for massdp media encode info description
 * @struct mansdp_encode
 */
struct mansdp_encode{
	struct mansdp_video video;
	struct mansdp_audio audio;
};

/**
 * Structure for mansdp description
 * @struct mansdp_payload
 */
struct mansdp_payload {
	char *_payload;
	//Session description
	unsigned char proto_version;             /**<v = 0*/
	struct mansdp_origin origin;             /**<o = 用户名 会话ID 版本 网络类型 地址类型 地址*/
	char *session_name;                      /**<s = 点播/回放/下载/语音对讲*/
	char *uri;                               /**<u = URI*/
	struct mansdp_connection conn_ses;       /**<c = 网络类型 地址类型 多点会议的地址*/
	//time description
	struct mansdp_time times;                /**<t = 开始时间 结束时间*/
	//Media description
	struct mansdp_info info;                 /**<m = 媒体 端口 传送层协议 负载类型*/
	struct mansdp_connection conn_media;     /**<c = 网络类型 地址类型 多点会议的地址*/
	struct mansdp_bandwidth bw;              /**<b = 带宽*/
	char **attributes;                       /**<a = 属性参数*/
	size_t attributes_count;
	char *ssrc;                              /**<y = SSRC*/
	struct mansdp_encode encode;             /**<f = v/码率格式/分辨率/帧率/码率类型/码率大小a/编码格式/音频编码码率/采样率*/
};

/**<*********************************MANSCDP***********************************>*/
typedef enum result_type
{
	ERROR = 0,
	OK = 1,
}result_type_t;

typedef enum duty_status_type
{
	OFFDUTY = 0,
	ONDUTY = 1,
	ALARM = 2,
}duty_status_type_t;

typedef enum record_type
{
	StopRecord,
	Record,
}record_type_t;

typedef enum guard_type
{
	ResetGuard,
	SetGuard,
}guard_type_t;

typedef enum signature_type
{
	Signature_stop,
	Signature_start,
}signature_type_t;

typedef enum encryption_type
{
	Encryption_stop,
	Encryption_start,
}encryption_type_t;

/**------------------------CONTROL---------------------------**/

typedef enum devicecontrol_subcmd
{
	PTZCmd = 0x01,                  /**<PTZ控制*/
	TeleBoot = 0x02,                /**<设备重启*/
	RecordCmd = 0x04,               /**<录像*/
	GuardCmd = 0x08,                /**<布防*/
	AlarmCmd = 0x10,                /**<报警复位*/
	IFameCmd = 0x20,                /**<强制I帧*/
	DragZoomIn = 0x40,              /**<拉框放大*/
	DragZoomOut = 0x80,             /**<拉框缩小*/
	HomePosition = 0x100,           /**<看守位控制*/
}devicecontrol_subcmd_t;

typedef enum ptz_cmd_type
{
	PTZ_CMD_TYPE_UNSUPPORT = 0,
	PTZ_CMD_TYPE_PTZ,              /**<PTZ指令*/
	PTZ_CMD_TYPE_FI,               /**<FI指令*/
	PTZ_CMD_TYPE_PRESET,           /**<预置位*/
	PTZ_CMD_TYPE_PATROL,           /**<巡航*/
	PTZ_CMD_TYPE_SCAN,             /**<扫描*/
	PTZ_CMD_TYPE_AUX,              /**<辅助开关*/
	PTZ_CMD_TYPE_CONTROL_STOP      /**<停止控制*/
}ptz_cmd_type_t;

typedef enum ptz_type
{
	PTZ_LEFT = 0x01,               /**<向左*/
	PTZ_RIGHT = 0x02,              /**<向右*/
	PTZ_UP = 0x04,                 /**<向上*/
	PTZ_DOWN = 0x08,               /**<向下*/
	PTZ_LARGE = 0x10,              /**<放大*/
	PTZ_SMALL = 0x20,              /**<缩小*/
}ptz_type_t;

typedef enum fi_type
{
	FI_FOCUS_FAR = 0x01,           /**<聚焦远*/
	FI_FOCUS_NEAR = 0x02,          /**<聚焦近*/
	FI_APERTURE_LARGE = 0x04,      /**<光圈放大*/
	FI_APERTURE_SMALL = 0x08,      /**<光圈缩小*/
}fi_type_t;

typedef enum preset_type
{
	PRESET_SET,                    /**<设置预置位*/
	PRESET_CALL,                   /**<调用预置位*/
	PRESET_DELE,                   /**<删除预置位*/
}preset_type_t;

typedef enum patrol_type
{
	PATROL_ADD,                    /**<加入一个巡航点*/
	PATROL_DELE,                   /**<删除一个巡航点*/
	PATROL_SET_SPEED,              /**<设置巡航速度*/
	PATROL_SET_TIME,               /**<设置停留时间*/
	PATROL_START,                  /**<开始巡航*/
}patrol_type_t;

typedef enum scan_type
{
	SCAN_START,                    /**<开始自动扫描*/
	SCAN_SET_LEFT_BORDER,          /**<设置自动扫描左边界*/
	SCAN_SET_RIGHT_BORDER,         /**<设置自动扫描右边界*/
	SCAN_SET_SPEED                 /**<设置自动扫描速度*/
}scan_type_t;

typedef struct ptz{
	ptz_type_t ptz_type;
	unsigned char horizontal_speed;     /**<水平控制速度*/
	unsigned char vertical_speed;       /**<垂直控制速度*/
	unsigned char zoom_speed;           /**<缩放速度*/
}ptz_t;

typedef struct fi
{
	fi_type_t fi_type;
	unsigned char focus_speed;          /**<聚焦速度*/
	unsigned char aperture_speed;       /**<光圈缩放速度*/
}fi_t;

typedef struct preset
{
	preset_type_t preset_type;
	unsigned char preset_index;         /**<预置位编号*/
}preset_t;

typedef struct patrol
{
	patrol_type_t  patrol_type;
	unsigned char patrol_id;            /**<巡航组号*/
	unsigned char preset_id;            /**<预置位号*/
	unsigned char value;                /**<数据，速度和停留时间使用*/
}patrol_t;

typedef struct scan
{
	scan_type_t scan_type;
	unsigned char scan_id;              /**<扫描组号*/
	unsigned char speed;                /**<scan速度*/
}scan_t;

/**<PTZCmd*/
typedef struct ptz_cmd
{
	unsigned char first_byte;               /**<A5H*/
	unsigned char version;                  /**<版本信息*/
	unsigned char check_code;               /**<校验码*/
	ptz_cmd_type_t ptz_cmd_type;
	ptz_t ptz;
	fi_t fi;
	preset_t preset;
	patrol_t patrol;
	scan_t scan;
}ptz_cmd_t;

/**<DragZoom*/
typedef struct drag_zoom
{
	int length;               /**<播放窗口长度像素值*/
	int width;                /**<播放窗口宽度像素值*/
	int midpointX;            /**<拉框中心横轴坐标像素值*/
	int midpointY;            /**<拉框中心纵轴坐标像素值*/
	int lengthX;              /**<拉框长度像素值*/
	int lengthY;              /**<拉框宽度像素值*/
}drag_zoom_t;

/**<HomePosition*/
typedef struct home_position
{
	status_type_t enable;     /**<看守位使能:1开启0关闭*/
	int resetTime;            /**<自动归位时间间隔(s)*/
	int presetIndex;          /**<调用预置位编号*/
}home_position_t;

/**<设备控制结构体*/
typedef struct device_control
{
	devicecontrol_subcmd_t deviceControlType;
	ptz_cmd_t *ptzCmd;                             /**<PTZcmd*/
	int teleboot;                                  /**<是否重启*/
	record_type_t recordCmd;                       /**<是否录像*/
	guard_type_t guardCmd;                         /**<是否布防*/
	int alarmCmd;                                  /**<报警复位*/
	int IFameSend;                                 /**<强制发送I帧*/
	drag_zoom_t *dragZoomParam;                    /**<拉框放大和缩小*/
	home_position_t *homePosition;                 /**<移动位置查询*/
}device_control_t;

typedef enum deviceconfig_subcmd
{
	BasicParam = 0x01,             /**<基本参数配置*/
	SVACEncodeConfig = 0x02,       /**<SVAC编码配置*/
}deviceconfig_subcmd_t;

typedef struct roi_item{
	int roiSeq;                 /**<感兴趣区域编号*/
	int topLeft;                /**<感兴趣区域左上角坐标*/
	int bottomRight;            /**<感兴趣区域右下角坐标*/
	int roiQP;                  /**<ROI区域编码质量等级*/
}roi_item_t;

typedef	struct svac_config{
	status_type_t roiFlag;                       /**<感兴趣区域开关*/
	int roiNum;                                  /**<感兴趣区域数量*/
	roi_item_t roiItem[16];
	int bgQP;                                    /**<背景区域编码质量等级*/
	status_type_t bgSkipFlag;                    /**<背景跳过开关*/
	status_type_t audioRecongnitionFlag;         /**<声音识别特征参数开关*/
}svac_config_t;

typedef	struct svac_query{
	status_type_t roiFlag;                       /**<感兴趣区域开关*/
	roi_item_t roiItem[16];
	int roiNum;                                  /**<感兴趣区域数量*/
	int bgQP;                                    /**<背景区域编码质量等级*/
	status_type_t bgSkipFlag;                    /**<背景跳过开关*/
	status_type_t timeFlag;                      /**<绝对时间信息开关*/
	status_type_t eventFlag;                     /**<监控事件信息开关*/
	status_type_t alertFlag;                     /**<报警信息开关*/
	status_type_t audioRecongnitionFlag;         /**<声音识别特征参数开关*/
}svac_query_t;

/**-------------------------QUERY----------------------------**/

typedef struct devce_status
{
	status_type_t      status;           /**<设备工作状态*/
	status_type_t      encode;           /**<设备是否编码*/
	status_type_t      record;           /**<设备是否录像*/
	duty_status_type_t dutyStatus;       /**<报警状态*/
}device_status_t;

typedef enum{
	RECORD_FILE_TYPE_TIME = 0,
	RECORD_FILE_TYPE_ALARM,
	RECORD_FILE_TYPE_MANUAL,
}record_file_type_t;

typedef struct item_file
{
	char filepath[128];                /**<文件路径名*/
	char address[64];                  /**<录像地址*/
	char starttime[64];                /**<录像开始时间*/
	char endtime[64];                  /**<录像结束时间*/
	secrecy_type_t secrecy;            /**<保密属性*/
	record_file_type_t type;           /**<录像产生类型*/
	char recorderID[25];               /**<录像触发者ID*/
	int filesize;                      /**<录像文件大小*/
}item_file_t;

/**<设备目录检索查询应答结构体*/
typedef struct record_query
{
	char *starttime;                  /**<录像起始时间*/
	char *endtime;                    /**<录像终止时间*/
	char *filepath;                   /**<文件路径名*/
	char *address;                    /**<录像地址*/
	secrecy_type_t secrecy;           /**<保密属性*/
	record_file_type_t type;          /**<录像产生类型*/
	char *recorderid;                 /**<录像触发者ID*/
}record_query_t;

typedef struct alarm_query
{

}alarm_info_t;

typedef struct preset_item
{
	char presetID[30];
	char presetName[20];
}item_preset_t;

typedef struct mobile_position
{

}mobile_position_t;

/**-------------------------NOTIFY----------------------------**/
typedef enum alarm_priority
{
	LEVEL_ONE_PRIORITY = 1,         /**<一级警情*/
	LEVEL_TWO_PRIORITY,             /**<二级警情*/
	LEVEL_THREE_PRIORITY,           /**<三级警情*/
	LEVEL_FOUR_PRIORITY,            /**<四级警情*/
}alarm_priority_t;

typedef enum alarm_method
{
	ALARM_METHOD_PHONE = 1,       /**<电话报警*/
	ALARM_METHOD_DEVICE,          /**<设备报警*/
	ALARM_METHOD_SMS,             /**<短信报警*/
	ALARM_METHOD_GPS,             /**<GPS报警*/
	ALARM_METHOD_VIDEO,           /**<视频报警*/
	ALARM_METHOD_DEVICE_FAILURE,  /**<设备故障报警*/
	ALARM_METHOD_OTHER,           /**<其他报警*/
}alarm_method_t;

typedef enum alarm_type
{
	//Alarm method : ALARM_METHOD_DEVICE
	ALARM_TYPE_DEVICE_VIDEO_LOSS = 1,                /**<视频丢失报警*/
	ALARM_TYPE_DEVICE_APART,                         /**<设备防拆报警*/
	ALARM_TYPE_DEVICE_MEMORY_FULL,                   /**<存储设备磁盘满报警*/
	ALARM_TYPE_DEVICE_HIGN_TEMPERATURE,              /**<设备高温报警*/
	ALARM_TYPE_DEVICE_LOW_TEMPERATURE,               /**<设备低温报警*/
	//Alarm method : ALARM_METHOD_VIDEO
	ALARM_TYPE_VIDEO_MANUAL_VIDEO,                   /**<人工视频报警*/
	ALARM_TYPE_VIDEO_MOVE_DETECT,                    /**<运动目标检测报警*/
	ALARM_TYPE_VIDEO_RESIDUE_DETECT,                 /**<遗留物检测报警*/
	ALARM_TYPE_VIDEO_OBJ_REMOVE,                     /**<物体移除报警*/
	ALARM_TYPE_VIDEO_LINE_DETECT,                    /**<绊线检测报警*/
	ALARM_TYPE_VIDEO_INTRUSION_DECTECT,              /**<入侵检测报警*/
	ALARM_TYPE_VIDEO_RETROGRADE,                     /**<逆行检测报警*/
	ALARM_TYPE_VIDEO_WANDER,                         /**<徘徊检测报警*/
	ALARM_TYPE_VIDEO_TRAFFIC_STATICTICS,             /**<流量统计报警*/
	ALARM_TYPE_VIDEO_DENSITY,                        /**<密度检测报警*/
	ALARM_TYPE_VIDEO_ABNORMAL,                       /**<视频异常检测报警*/
	ALARM_TYPE_VIDEO_FAST_MOVE,                      /**<快速移动报警*/
	//Alarm method : ALARM_METHOD_DEVICE_FAILURE
	ALARM_TYPE_FAILURE_DISK_ERROR,                   /**<存储设备磁盘故障报警*/
	ALARM_TYPE_FAILURE_FAN_ERROR,                    /**<存储设备风扇故障报警*/
}alarm_type_t;

/**<报警结构体*/
typedef struct alarm_info
{
	alarm_priority_t alarmPriority;    /**<报警级别*/
	alarm_method_t alarmMethod;        /**<报警方式*/
	char alarmTime[64];                /**<报警时间*/
	char alarmContent[64];             /**<报警内容描述(optional)*/
	alarm_type_t alarmType;            /**<报警类型*/
	int eventType;                     /**<入侵检测报警时携带：0进入区域1离开区域(optional)*/
}alarm_notify_t;

/**
 * Structure for manrtsp cmd type description
 * @var manrtsp_cmd_t
 */
typedef enum manrtsp_cmd
{
	MANRTSP_PLAY,         /**<播放*/
	MANRTSP_PAUSE,        /**<暂停*/
	MANRTSP_TEARDOWN      /**<停止*/
}manrtsp_cmd_t;

/**
 * Structure for history video playback control description
 * @struct playback_ctrl
 */
struct playback_ctrl
{
	manrtsp_cmd_t cmd;    /**<manrtsp的指令类型*/
	int scale;            /**<播放速度*/
	int range_begin;      /**<播放开始时间*/
	int range_end;        /**<播放结束时间*/
	int pause_time;       /**<暂停时间*/
};

/**
 * Structure for sip event type description
 * @var sip_event_type_t
 */
typedef enum{
	event_type_default,
	event_type_info_palyback_ctrl,                                 /**<历史视音频流播放控制*/
	event_type_ack,                                                /**<媒体连接建立*/
	event_type_bye,                                                /**<媒体连接断开*/
	event_type_control_device_control = SIP_EVENT_WITH_XML,        /**<设备控制*/
	event_type_control_device_config,                              /**<设备配置*/
	event_type_control_signature_start,                            /**<开始签名*/
	event_type_control_signature_stop,                             /**<停止签名*/
	event_type_control_encryption_start,                           /**<开始加密*/
	event_type_control_encryption_stop,                            /**<停止加密*/
	event_type_query_status,                                       /**<状态查询*/
	event_type_query_recordinfo,                                   /**<文件目录检索*/
	event_type_query_alarm,                                        /**<设备报警查询*/
	event_type_query_configdownload,                               /**<设备配置查询*/
	event_type_query_preset,                                       /**<设备预置位查询*/
	event_type_query_mobileposition ,                              /**<移动设备位置查询*/
	event_type_notify_alarm,                                       /**<报警通知*/
	event_type_notify_fileend,                                     /**<文件末尾通知*/
	event_type_notify_mobileposition,                              /**<移动设备位置数据通知*/
	event_type_invite = SIP_EVENT_WITH_SDP,                        /**<媒体连接请求*/
	event_type_auth_success = SIP_EVENT_AUTH_SUCCESS,              /**<注册成功*/
} sip_event_type_t;

typedef struct call_id
{
	char *host;           /**< Call-ID host information */
	char *number;         /**< Call-ID number */
}call_id_t;

/**
 * Structure for sip event description
 * @struct sip_event
 */
struct sip_event{
	sip_event_type_t type;  /**<event type*/
	call_id_t *cid;         /**<call-id Header*/
	result_type_t result;   /**<handle result*/
	int id;                 /**<transaction ID*/
	int sn;                 /**<SN from xml*/
	void *data;             /**<struct data*/
};


struct sip_callbacks
{
	int (*generate_random)(char *dst,int *dstlen);
	int (*signature_data)(unsigned char htype, unsigned char stype, char *src, unsigned int srclen,char *dst, int *dstlen);
	int (*verify_data)(char *plaintext,int plaintext_len,char *cipher,int cipher_len);
	int (*digest_data)(char *src,int srclen,char *dst,int *dstlen);
};

/**
 * Allocate memory for  sip session paramter.
 */
sipSession_param_t *sipSession_param_malloc(void);

/**
* Free memory for sip session parameter.
* @param para The sip session parameter.
*/
void sipSession_param_free(sipSession_param_t *para);

/**
* Allocate Memory for sip session.
*/
struct sipSession_t *sipSession_malloc(void);

/**
* Initialize sip session.
* @param s The sip session.
*/
int sipSession_init(struct sipSession_t *s,struct sip_callbacks *callbacks);

/**
* Free memory for sip session
* @param s The sip session.
*/
void sipSession_free(struct sipSession_t *s);

/**
* Start the sip session.
* @param s The sip session.
* @param para The sip session parameter.
*/
int sipSession_start(struct sipSession_t *s,sipSession_param_t *para);

/**
* Stop the sip session.
* @param s The sip session.
*/
void sipSession_stop(struct sipSession_t *s);

/**
* Get the sip event from sip session
* @param s The sip session.
* @param tv_s The delay of seconds
* @param tv_ms The delay of milliseconds
*/
sip_event_t *sipSession_event_get(struct sipSession_t *s, int tv_s, int tv_ms);

/**
* Response a sip event via the sip session.
* @param s The sip session.
* @param event Response the sip event.
*/
int sipSession_event_response(struct sipSession_t *s, sip_event_t *event);

char* sipSession_event_get_vkek(struct sipSession_t *s);

char* sipSession_event_get_keyversion(struct sipSession_t *s);

/**
* Disconnect a media invite sip event.
* @param event the sdp of media sip event.
*/
int sipSession_event_media_disconnect(struct mansdp_payload *sdp);

/**
* Check whether the two events are a series of events.
* @param event1 the call-id of sip event1.
* @param event2 the call-id of sip event2.
*/
int sipSession_event_match(call_id_t *call_id1,call_id_t *call_id2);

/**
* Get media request device ID.
* @param sdp the media request device ID information.
*/
char *sipSession_event_get_media_device_id(struct mansdp_payload *sdp);

/**
* Get media request ip address.
* @param sdp the media request sdp information.
*/
char *sipSession_event_get_media_ip(struct mansdp_payload *sdp);

/**
* Get media request INVITE type.
* @param sdp the media request sdp INVITE type.
*/
int sipSession_event_get_media_invite_type(struct mansdp_payload *sdp);

/**
* Get media request ip port.
* @param sdp the media request sdp information.
*/
int sipSession_event_get_media_port(struct mansdp_payload *sdp);

/**
* Get media request start time.
* @param sdp the media request sdp start time information.
*/
int sipSession_event_get_media_start_time(struct mansdp_payload *sdp);

/**
* Get media request stop time.
* @param sdp the media request sdp stop time information.
*/
int sipSession_event_get_media_stop_time(struct mansdp_payload *sdp);

/**
* Get media request SSRC.
* @param sdp the media request sdp information.
*/
int sipSession_event_get_media_ssrc(struct mansdp_payload *sdp);

/**
* Set a record file item for the query of record information.
* @param s The sip session.
* @param file Record file item.
*/
int sipSession_event_set_record_file_item(struct sipSession_t *s,item_file_t *file);

/**
* Set a preset item for the query of preset.
* @param s The sip session.
* @param preset preset.
*/
int sipSession_event_set_preset_item(struct sipSession_t *s,item_preset_t *preset);

/**
* Send file end to server.
* @param s The sip session.
*/
int sipSession_event_send_fileend(struct sipSession_t *s);

/**
* Send alarm information to server.
* @param s The sip session.
* @param alarm alarm information.
*/
int sipSession_event_send_alarm(struct sipSession_t *s,alarm_notify_t *alarm);

#ifdef __cplusplus
}
#endif

#endif

