#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdbool.h>
#include <errno.h>
#include <error.h>

#if defined DL_ENABLE
#include <imp/imp_osd.h>
#include <ivs/ivs_common.h>
#include <ivs/ivs_interface.h>
#include <ivs/ivs_inf_personvehicleDet.h>
#include <imp/imp_ivs.h>
#endif
#if defined T41
#include "xcam_video_t41.h"
#endif

#include "xcam_video_t32.h"
#include "xcam_video.h"
#include "xcam_extra.h"

#include "xcam_log.h"
#include "xcam_conf_video.h"
#include "xcam_conf_sys.h"
#include "rtsp/c_liveRTSP.h"
#include "xcam_general.h"
#include "xcam_osd.h"
#include "xcam_msg.h"
#include "xcam_module.h"
#include "xcam_stream.h"
#include "xcam_conf_network.h"
#include "xcam_cli_options.h"
#include "xcam_com.h"

#define TAG "HAL_INGENIC"
#define LOG_TAG "VIDEO"
#define CONFIG_SYS_VIDEO_FILE "/system/xcam_video.json"
#define SENSOR_CUBS_TYPE        TX_SENSOR_CONTROL_INTERFACE_I2C
#define BITRATE_720P_Kbs        1000

#if ((defined T20)||(defined T21))
	IMPEncoderGOPSizeCfg gopsize_config;
#elif ((defined T41)||(defined T32)) 
//#define SHOW_FRM_BITRATE

#ifdef SHOW_FRM_BITRATE
#define FRM_BIT_RATE_TIME 2
#define STREAM_TYPE_NUM 3
static int frmrate_sp[STREAM_TYPE_NUM] = { 0 };
static int statime_sp[STREAM_TYPE_NUM] = { 0 };
static int bitrate_sp[STREAM_TYPE_NUM] = { 0 };
#endif

video_config_info_t stream_attr;
IMPSensorInfo sensor_info;
static const IMPEncoderRcMode S_RC_METHOD = ENC_RC_MODE_CBR;
IMPSensorInfo sensor_info_t41[2];
static IMPCell cell_fs0 = { DEV_ID_FS, STREAM0, 0};
static IMPCell cell_enc0 = { DEV_ID_ENC, ENCGROUP0, 0};
static IMPCell cell_fs1 =  { DEV_ID_FS, STREAM1, 0};
static IMPCell cell_enc1 = { DEV_ID_ENC, ENCGROUP1, 0};
static IMPCell cell_osd1 = { DEV_ID_OSD, STREAM1, 0};
// static IMPCell cell_fs2 =  { DEV_ID_FS, STREAM2, 0};  for warning
#ifdef XCAM_DOUBLE_SENSOR
static IMPCell cell_fs3 =  { DEV_ID_FS, STREAM3, 0};
static IMPCell cell_enc3 = { DEV_ID_ENC, ENCGROUP3, 0};
static IMPCell cell_osd3 = { DEV_ID_OSD, STREAM3, 0};
#endif
struct chn_conf chn[FS_CHN_NUM] = {
	{
		.index = CH0_INDEX,
		.enable = CHN0_EN,
		.payloadType = PT_H265,
		.fs_chn_attr = {
			.pixFmt = PIX_FMT_NV12,
			.outFrmRateNum = FIRST_SENSOR_FRAME_RATE_NUM,
			.outFrmRateDen = FIRST_SENSOR_FRAME_RATE_DEN,
			.nrVBs = 2,
			.type = FS_PHY_CHANNEL,

			.crop.enable = FIRST_CROP_EN,
			.crop.top = 0,
			.crop.left = 0,
			.crop.width = FIRST_SENSOR_WIDTH,
			.crop.height = FIRST_SENSOR_HEIGHT,

			.scaler.enable = 0,

			.picWidth = FIRST_SENSOR_WIDTH,
			.picHeight = FIRST_SENSOR_HEIGHT,
		},
		.framesource_chn =  { DEV_ID_FS, CH0_INDEX, 0},
		.imp_encoder = { DEV_ID_ENC, CH0_INDEX, 0},
	},
	{
		.index = CH1_INDEX,
		.enable = CHN1_EN,
		.payloadType = PT_H265,
		.fs_chn_attr = {
			.pixFmt = PIX_FMT_NV12,
			.outFrmRateNum = FIRST_SENSOR_FRAME_RATE_NUM,
			.outFrmRateDen = FIRST_SENSOR_FRAME_RATE_DEN,
			.nrVBs = 1,
			.type = FS_PHY_CHANNEL,

			.crop.enable = 0,
			.crop.top = 0,
			.crop.left = 0,
			.crop.width = FIRST_SENSOR_WIDTH,
			.crop.height = FIRST_SENSOR_HEIGHT,

			.scaler.enable = 1,
			.scaler.outwidth = FIRST_SENSOR_WIDTH_SECOND,
			.scaler.outheight = FIRST_SENSOR_HEIGHT_SECOND,

			.picWidth = FIRST_SENSOR_WIDTH_SECOND,
			.picHeight = FIRST_SENSOR_HEIGHT_SECOND,
		},
		.framesource_chn =  { DEV_ID_FS, CH1_INDEX, 0},
		.imp_encoder = { DEV_ID_ENC, CH1_INDEX, 0},
	},
	{
		.index = CH2_INDEX,
		.enable = CHN2_EN,
		.payloadType = PT_H265,
		.fs_chn_attr = {
			.pixFmt = PIX_FMT_NV12,
			.outFrmRateNum = FIRST_SENSOR_FRAME_RATE_NUM,
			.outFrmRateDen = FIRST_SENSOR_FRAME_RATE_DEN,
			.nrVBs = 1,
			.type = FS_PHY_CHANNEL,

			.crop.enable = 0,
			.crop.top = 0,
			.crop.left = 0,
			.crop.width = FIRST_SENSOR_WIDTH,
			.crop.height = FIRST_SENSOR_HEIGHT,
			.scaler.enable = 1,
			.scaler.outwidth = FIRST_SENSOR_WIDTH_THIRD,
			.scaler.outheight = FIRST_SENSOR_HEIGHT_THIRD,

			.picWidth = FIRST_SENSOR_WIDTH_THIRD,
			.picHeight = FIRST_SENSOR_HEIGHT_THIRD,
		},
		.framesource_chn =  { DEV_ID_FS, CH2_INDEX, 0},
		.imp_encoder = { DEV_ID_ENC, CH2_INDEX, 0},
	},
    {
        .index = CH3_INDEX,
        .enable = CHN3_EN,
        .payloadType = PT_H265,
        .fs_chn_attr = {
            .pixFmt = PIX_FMT_NV12,
            .outFrmRateNum = SECOND_SENSOR_FRAME_RATE_NUM,
            .outFrmRateDen = SECOND_SENSOR_FRAME_RATE_DEN,
            .nrVBs = 2,
            .type = FS_PHY_CHANNEL,

            .crop.enable = SECOND_CROP_EN,
            .crop.top = 0,
            .crop.left = 0,
            .crop.width = SECOND_SENSOR_WIDTH,
            .crop.height = SECOND_SENSOR_HEIGHT,

            .scaler.enable = 1,
            .scaler.outwidth = SECOND_SENSOR_WIDTH,
            .scaler.outheight = SECOND_SENSOR_HEIGHT,

            .picWidth = SECOND_SENSOR_WIDTH,
            .picHeight = SECOND_SENSOR_HEIGHT,
            },
            .framesource_chn =      { DEV_ID_FS, CH3_INDEX, 0},
            .imp_encoder = { DEV_ID_ENC, CH3_INDEX, 0},
    },
};

IMPSensorInfo Def_Sensor_Info[2] = {
	{
		.name = FIRST_SNESOR_NAME,
		.cbus_type = TX_SENSOR_CONTROL_INTERFACE_I2C,
		.i2c = {FIRST_SNESOR_NAME, FIRST_I2C_ADDR, FIRST_I2C_ADAPTER_ID},
		.rst_gpio = FIRST_RST_GPIO,
		.pwdn_gpio = FIRST_PWDN_GPIO,
		.power_gpio = FIRST_POWER_GPIO,
		.switch_gpio = 0,
		.switch_gpio_state = 0,
		.sensor_id = FIRST_SENSOR_ID,
		.video_interface = FIRST_VIDEO_INTERFACE,
		.mclk = FIRST_MCLK,
		.default_boot = FIRST_DEFAULT_BOOT,
		.fps = {15, 0}
	},
    {
        .name = SECOND_SNESOR_NAME,
        .cbus_type = TX_SENSOR_CONTROL_INTERFACE_I2C,
        .i2c = {SECOND_SNESOR_NAME, SECOND_I2C_ADDR, SECOND_I2C_ADAPTER_ID},
        .rst_gpio = SECOND_RST_GPIO,
        .pwdn_gpio = SECOND_PWDN_GPIO,
        .power_gpio = SECOND_POWER_GPIO,
        .switch_gpio = 0,
        .switch_gpio_state = 0,
        .sensor_id = SECOND_SENSOR_ID,
        .video_interface = SECOND_VIDEO_INTERFACE,
        .mclk = SECOND_MCLK,
        .default_boot = SECOND_DEFAULT_BOOT,
        .fps = {15, 0}
    },
};
#endif  //end if defined T32

#if defined DL_ENABLE
#define MAX_IVS_OSD_REGION          25      /* 20 line and rect */
/*static personvehicledet_param_output_t main_result;
static IMPRgnHandle ivsRgnHandler[MAX_IVS_OSD_REGION] = {INVHANDLE};
// static IMPRgnHandle ivsRgnHandler_second[MAX_IVS_OSD_REGION] = {INVHANDLE};
// static int xcam_ivs_persondet_init(IMPIVSInterface **interface);  for warning */
#endif

extern conf_video_t xcam_video_conf;

//同步修改用户态分辨率配置信息
static void _xcam_video_syn_isp_fps(int fps_num, int fps_den)
{
	assert(((fps_num > 0) && (fps_den > 0)));

	stream_attr.fps_num = fps_num;
	stream_attr.fps_den = fps_den;

	return;
}

static void _xcam_video_syn_fs_fps(int channel, int fps_num, int fps_den)
{
	assert(((channel == 0) || (channel == 1)));
	assert(((fps_num > 0) && (fps_den > 0)));

	stream_attr.stream_config[channel].fs_attr.outFrmRateNum = fps_num;
	stream_attr.stream_config[channel].fs_attr.outFrmRateDen = fps_den;

	return;
}

static void _xcam_video_syn_enc_fps(int channel, int fps_num, int fps_den)
{
	assert(((channel == 0) || (channel == 1)));
	assert(((fps_num > 0) && (fps_den > 0)));

	stream_attr.stream_config[channel].enc_attr.rcAttr.outFrmRate.frmRateNum = fps_num;
	stream_attr.stream_config[channel].enc_attr.rcAttr.outFrmRate.frmRateDen = fps_den;

	return;
}

//同步修改用户态分编码模式信息
/*
static void _xcam_video_syn_encAttrRcMode(int encChnNum, IMPEncoderAttrRcMode *encAttrRcMode)
{
	assert(((encChnNum == 0) || (encChnNum == 1)));
	assert(encAttrRcMode != NULL);

	memcpy(&stream_attr.stream_config[encChnNum].enc_attr.rcAttr.attrRcMode, encAttrRcMode, sizeof(IMPEncoderAttrRcMode));

	return;
} for warning */

//同步修改用户态空间码模式信息
static void _xcam_video_syn_encAttr(int encChnNum, IMPEncoderCHNAttr *encAttr)
{
	assert(((encChnNum == 0) || (encChnNum == 1)));
	assert(encAttr != NULL);

	memcpy(&stream_attr.stream_config[encChnNum].enc_attr, encAttr, sizeof(IMPEncoderCHNAttr));

	return;
}

static void _xcam_video_syn_encmode(int channel, int encmode)
{
	assert(((channel == 0) || (channel == 1)));
	// 	int enc_mode = IMP_ENC_PROFILE_HEVC_MAIN;
	// if(xcam_video_conf.conf_enc.ch1_type == PT_H264) {
	// 	enc_mode = IMP_ENC_PROFILE_AVC_MAIN;
	// } else if (xcam_video_conf.conf_enc.ch1_type == PT_H265) {
	// 	enc_mode = IMP_ENC_PROFILE_HEVC_MAIN;
	// }
	stream_attr.stream_config[channel].enc_attr.encAttr.enType = (encmode == IMP_ENC_PROFILE_AVC_MAIN ? PT_H264 : PT_H265 );

	return;
}

//同步修改framrous的配置信息
static void _xcam_video_syn_fsAttr(int channel, IMPFSChnAttr *fsAttr)
{
	assert(((channel == 0) || (channel == 1)));
	assert(fsAttr != NULL);

	memcpy(&stream_attr.stream_config[channel].fs_attr, fsAttr, sizeof(IMPFSChnAttr));

	return;
}

int xcam_video_get_channel_num()
{
	int channel = 2;

	return channel;
}

static int xcam_qparrays[][2] = {  {20, 35}, {30, 40}, {40, 50}};
static int xcam_qparrays_slaveindex[][2] = { {20, 35} , {30, 40}, {40, 50},};
int xcam_video_set_isp_qp(uint32_t chn, uint32_t qp_index)
{
	int ret = XCAM_SUCCESS;

	if(chn == 0) {
		ret =  IMP_Encoder_SetChnQpBounds(chn, xcam_qparrays[qp_index][0], xcam_qparrays[qp_index][1]);
	} else if(chn == 1) {
		ret =  IMP_Encoder_SetChnQpBounds(chn, xcam_qparrays_slaveindex[qp_index][0], xcam_qparrays[qp_index][1]);
	}
	if (ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG,"Failed to IMP_Encoder_SetChnQpBounds, chn=%d, qp_index=%d\n", chn, qp_index);
	}
	return ret;
}

//设置系统帧率
int xcam_video_set_isp_fps(uint32_t fps_num,uint32_t fps_den)
{
	int ret = XCAM_SUCCESS;

	if ((fps_num <= 0)||(fps_den <= 0)) {
		return XCAM_ERROR;
	}

	if ((fps_num == stream_attr.fps_num) && (fps_den == stream_attr.fps_den)) {
		LOG_INF(LOG_TAG,"The current configuration is the same as this configuration.\n");
		return XCAM_SUCCESS;
	}

	IMPISPSensorFps fps;
	fps.num = fps_num;
	fps.den = fps_den;
	ret = IMP_ISP_Tuning_SetSensorFPS(0, &fps);
	if (ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG,"xcam_video_set_isp_fps Call IMP_ISP_SetSensor fail.\n");
		return ret;
	}
	
	ret = xcam_conf_set_video_isp_fps(fps_num,fps_den);
	if (ret < XCAM_SUCCESS) {
		LOG_INF(LOG_TAG,"Failed to save frame rate to json profile.\n");
	}
	
	_xcam_video_syn_isp_fps(fps_num,fps_den);

	return ret;
}

//获取系统帧率
int xcam_video_get_isp_fps(uint32_t *fps_num,uint32_t *fps_den)
{
	int ret = XCAM_SUCCESS;

	if ((fps_num == NULL)||(fps_den == NULL)) {
		return XCAM_ERROR;
	}

	IMPISPSensorFps fps;
	ret = IMP_ISP_Tuning_GetSensorFPS(0, &fps);
	if (ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG,"xcam_video_get_isp_fps call IMP_ISP_Tuning_GetSensorFPS Fail.\n");
		return ret;
	}

	*fps_num = fps.num;
	*fps_den = fps.den;

	return ret;
}

int xcam_video_set_fs_fps(int channel,uint32_t fps_num,uint32_t fps_den)
{
	int ret = XCAM_SUCCESS;

	if ((fps_num <= 0)||(fps_den <= 0)) {
		return XCAM_ERROR;
	}

	if ((fps_num == stream_attr.stream_config[channel].fs_attr.outFrmRateNum) && (fps_den == stream_attr.stream_config[channel].fs_attr.outFrmRateDen)) {
		LOG_INF(LOG_TAG,"The current configuration is the same as this configuration.\n");
		return XCAM_SUCCESS;
	}

	IMPISPSensorFps fps;
	fps.num = fps_num;
	fps.den = fps_den;
	ret = IMP_ISP_Tuning_SetSensorFPS(channel, &fps);
	if (ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG,"xcam_video_set_isp_fps Call IMP_ISP_SetSensor fail.\n");
		return ret;
	}

	_xcam_video_syn_fs_fps(channel, fps_num, fps_den);

	return ret;
}

int xcam_video_get_fs_fps(int channel,uint32_t *fps_num,uint32_t *fps_den)
{
	int ret = XCAM_SUCCESS;

	if ((fps_num == NULL) || (fps_den == NULL)) {
		return XCAM_ERROR;
	}

	*fps_num = stream_attr.stream_config[channel].fs_attr.outFrmRateNum ;
	*fps_den = stream_attr.stream_config[channel].fs_attr.outFrmRateDen;

	return ret;
}

int xcam_video_set_enc_fps(int channel,uint32_t fps_num,uint32_t fps_den)
{
	int ret = XCAM_SUCCESS;
	IMPEncoderFrmRate stFps;

	if ((fps_num <= 0)||(fps_den <= 0)) {
		return XCAM_ERROR;
	}

	memset(&stFps,0,sizeof(IMPEncoderFrmRate));

	//这个地方还需要重新的修改的的
	if ((fps_num == stream_attr.stream_config[channel].enc_attr.rcAttr.outFrmRate.frmRateNum) && (fps_den == stream_attr.stream_config[channel].enc_attr.rcAttr.outFrmRate.frmRateDen)) {
		LOG_INF(LOG_TAG,"The current configuration is the same as this configuration.\n");
		return XCAM_SUCCESS;
	}

	stFps.frmRateNum = fps_num;
	stFps.frmRateDen = fps_den;

	ret = IMP_Encoder_SetChnFrmRate	(channel, &stFps);
	if (ret < 0) {
		LOG_ERR(LOG_TAG,"Call IMP_Encoder_SetChnFrmRate fail.\n");
	}

	_xcam_video_syn_enc_fps(channel, fps_num, fps_den);

	return ret;
}

int xcam_video_get_enc_fps(int channel, uint32_t *fps_num, uint32_t *fps_den)
{
	int ret = XCAM_SUCCESS;
	IMPEncoderFrmRate stFps;

	if ((fps_num == NULL)||(fps_den == NULL)) {
		return XCAM_ERROR;
	}

	memset(&stFps,0,sizeof(IMPEncoderFrmRate));

	ret = IMP_Encoder_GetChnFrmRate(channel, &stFps);
	if (ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG,"xcam_video_get_fps call IMP_ISP_Tuning_GetSensorFPS Fail.\n");
		return ret;
	}

	*fps_num = stFps.frmRateNum;
	*fps_den = stFps.frmRateDen;

	return ret;
}

//设置系统码率
int xcam_video_set_bitrate(int encChnNum, int bps_num)
{
	int ret = XCAM_SUCCESS;

	ret = IMP_Encoder_SetChnBitRate(encChnNum, bps_num , bps_num * 4 / 3);
	if( ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG,"xcam_video_get_fps call IMP_Encoder_SetChnBitRate Fail.ret=%d\n", ret);
		return ret;
	}	
	ret = xcam_conf_set_video_bps(encChnNum, bps_num);
	if (ret < XCAM_SUCCESS) {
		LOG_INF(LOG_TAG,"err(%s,%d):failed to save bitrate to json profile.\n", __func__, __LINE__);
	}

	return ret;
}

//设置获取系统码率 T32 AVBR SMART未适配
int xcam_video_get_bitrate(int encChnNum, int *bps_num)
{
	int ret = XCAM_SUCCESS;
	IMPEncoderAttrRcMode stRcModeCfg;

	memset(&stRcModeCfg, 0, sizeof(IMPEncoderAttrRcMode));

	ret = IMP_Encoder_GetChnAttrRcMode(encChnNum,&stRcModeCfg);
	if (ret < 0) {
		LOG_ERR(LOG_TAG,"err(%s,%d):Call IMP_Encode_GetChnAttrRcMode fail\n",__func__,__LINE__);
		return ret;
	}
	if (stRcModeCfg.rcMode == ENC_RC_MODE_FIXQP) {
		LOG_INF(LOG_TAG,"info(%s,%d):The encoder mode catn't set bitrate.\n",__func__,__LINE__);
	} else if (stRcModeCfg.rcMode == ENC_RC_MODE_CBR) {
		*bps_num = stRcModeCfg.attrH265Cbr.outBitRate;
	} else if (stRcModeCfg.rcMode == ENC_RC_MODE_VBR) {
		*bps_num = stRcModeCfg.attrH265Vbr.maxBitRate;
	} else if (stRcModeCfg.rcMode == ENC_RC_MODE_CVBR) {
		// *bps_num = stRcModeCfg.attrCappedVbr.uTargetBitRate;
	} else {
		LOG_ERR(LOG_TAG,"Encode don't support this type.\n");
		ret = XCAM_ERROR;
	}

	return ret;
}

//停止stream0
static int set_resolution_stop_stream0(int iFsChnNum)
{
	int ret = XCAM_SUCCESS;
	IMPEncoderCHNStat vencChnStat;
	memset(&vencChnStat, 0, sizeof(IMPEncoderCHNStat));
	//xcam_osd_all_osdrgn_destroy 对于ipuosd未先进行unbind操作，不调用不影响后续操作
	// ret = xcam_osd_all_osdrgn_destroy(OSDGROUP0);
	// if (ret < 0) {
	// 	LOG_ERR(LOG_TAG,"Stop all osd rgn fail.\n");
	// 	return ret;
	// }
	ret = c_RTSP_stop(STREAM0);
	if (ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG,"Check out encode mode before c_RTSP_stop fail.\n");
		return ret;
	}
	ret = IMP_System_UnBind(&cell_fs0, &cell_enc0);
	if (ret < 0) {
		LOG_ERR(TAG, "UnBind FrameSource channel0 and Encoder failed\n");
		return ret;
	}

	ret = IMP_Encoder_StopRecvPic(ENCCHN0);
	if (ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG,"set_resolution_stop_stream call IMP_Encode_StopRecvPic fail.\n");
		return ret;
	}

	ret = IMP_FrameSource_DisableChn(STREAM0);
	if (ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG,"set_resolution_stop_stream call IMP_Encode_StopRecvPic fail.\n");
		return ret;
	}

	ret = IMP_Encoder_Query(ENCCHN0, &vencChnStat);
	if(ret == XCAM_SUCCESS){
		if (vencChnStat.registered) {
			ret = IMP_Encoder_UnRegisterChn(ENCCHN0);
			if(ret < 0){
				LOG_ERR(LOG_TAG,"Encode unregirter fail.\n");
				return ret;
			}
			ret = IMP_Encoder_DestroyChn(ENCCHN0);
			if(ret < 0){
				LOG_ERR(LOG_TAG,"Destroy encode channel fail.\n");
				return ret;
			}
		}
	} else {
		return XCAM_ERROR;
	}

	return ret;
}

//停止stream1 待完成
static int set_resolution_stop_stream1(int iFsChnNum)
{
	int ret = XCAM_SUCCESS;

	IMPEncoderCHNStat vencChnStat;
	memset(&vencChnStat, 0, sizeof(IMPEncoderCHNStat));
	LOG_ERR(LOG_TAG,"Stop all osd rgn fail.\n");
	// ret = xcam_osd_all_osdrgn_destroy(OSDGROUP1);
	// LOG_ERR(LOG_TAG,"Stop all osd rgn fail.\n");
	// if (ret < 0) {
	// 	LOG_ERR(LOG_TAG,"Stop all osd rgn fail.\n");
	// 	return ret;
	// }
	LOG_ERR(LOG_TAG,"Stop all osd rgn fail.\n");
	ret = IMP_System_UnBind(&cell_osd1, &cell_enc1);
	if (ret < 0) {
		LOG_ERR(TAG, "UnBind OSD channel1 and Encoder failed\n");
		return -1;
	}
	LOG_ERR(LOG_TAG,"Stop all osd rgn fail.\n");
	ret = IMP_Encoder_StopRecvPic(ENCCHN1);
	if (ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG,"set_resolution_stop_stream call IMP_Encode_StopRecvPic fail.\n");
		return ret;
	}
LOG_ERR(LOG_TAG,"Stop all osd rgn fail.\n");
	ret = IMP_FrameSource_DisableChn(STREAM1);
	if (ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG,"set_resolution_stop_stream call IMP_Encode_StopRecvPic fail.\n");
		return ret;
	}
LOG_ERR(LOG_TAG,"Stop all osd rgn fail.\n");
	ret = IMP_Encoder_Query(ENCCHN1, &vencChnStat);
	if (ret == XCAM_SUCCESS) {
		if (vencChnStat.registered){
			ret = IMP_Encoder_UnRegisterChn(ENCCHN1);
			LOG_ERR(LOG_TAG,"Stop all osd rgn fail.\n");
			if (ret < 0) {
				LOG_ERR(LOG_TAG,"Encode unregirter fail.\n");
				return ret;
			}
			ret = IMP_Encoder_DestroyChn(ENCCHN1);
			LOG_ERR(LOG_TAG,"Stop all osd rgn fail.\n");
			if (ret < 0) {
				LOG_ERR(LOG_TAG,"Destroy Encode channel fail.\n");
				return ret;
			}
		}
	}
	else {
		return XCAM_ERROR;
	}
LOG_ERR(LOG_TAG,"Stop all osd rgn fail.\n");
	return ret;
}

static int encoder_init(IMPEncoderCHNAttr* pattr, int encmode, int rcmode, int width, int height, int fs_den, int fs_num, int direct_mode, int uTargetBitRate)
{
	int S_RC_METHOD = rcmode;
	int payloadType = encmode;
	int ret = 0;

	ret = IMP_Encoder_SetDefaultParam(pattr, payloadType, S_RC_METHOD, width, height, fs_num, fs_den,
			fs_num * 2 / fs_den, width * height,
			(S_RC_METHOD == ENC_RC_MODE_FIXQP) ? 35 : -1,
			uTargetBitRate);
	if (ret != 0) {
		IMP_LOG_ERR(TAG, "IMP_Encoder_SetDefaultParam error !\n");
		return XCAM_ERROR;
	}

	pattr->bEnableIvdc = (direct_mode == 1) ? true : false;
#if 0
	IMPEncoderRcAttr *rcAttr = &pattr->rcAttr;
	switch (rcAttr->attrRcMode.rcMode) {
		case ENC_RC_MODE_FIXQP:
			rcAttr->attrRcMode.attrH264FixQp.IQp = 38;
			rcAttr->attrRcMode.attrH265FixQp.IQp = 38;
			break;
		case ENC_RC_MODE_CBR:
		    rcAttr->maxGop = 50;
			rcAttr->attrRcMode.attrH264Cbr.outBitRate = uTargetBitRate;
			rcAttr->attrRcMode.attrH264Cbr.initialQp = -1;
			rcAttr->attrRcMode.attrH264Cbr.minIQp = 15;
			rcAttr->attrRcMode.attrH264Cbr.maxIQp = 48;
			rcAttr->attrRcMode.attrH264Cbr.IPfrmQPDelta = -1;
			rcAttr->attrRcMode.attrH264Cbr.PPfrmQPDelta = -1;
			// rcAttr->attrRcMode.attrH264Cbr.eRcOptions = IMP_ENC_RC_SCN_CHG_RES;
			rcAttr->attrRcMode.attrH264Cbr.maxIPictureSize = uTargetBitRate * 2;
			rcAttr->attrRcMode.attrH264Cbr.maxPPictureSize = uTargetBitRate * 2;
			rcAttr->attrRcMode.attrH265Cbr.outBitRate = uTargetBitRate;
			rcAttr->attrRcMode.attrH265Cbr.initialQp = -1;
			rcAttr->attrRcMode.attrH265Cbr.minIQp = 15;
			rcAttr->attrRcMode.attrH265Cbr.maxIQp = 48;
			rcAttr->attrRcMode.attrH265Cbr.IPfrmQPDelta = -1;
			rcAttr->attrRcMode.attrH265Cbr.PPfrmQPDelta = -1;
			// rcAttr->attrRcMode.attrH264Cbr.eRcOptions = IMP_ENC_RC_SCN_CHG_RES;
			rcAttr->attrRcMode.attrH265Cbr.maxIPictureSize = uTargetBitRate * 2;
			rcAttr->attrRcMode.attrH265Cbr.maxPPictureSize = uTargetBitRate * 2;
			break;
		case ENC_RC_MODE_VBR:
			rcAttr->attrRcMode.attrH264Vbr.maxBitRate = uTargetBitRate  * 4 / 3;
			rcAttr->attrRcMode.attrH264Vbr.initialQp = -1;
			rcAttr->attrRcMode.attrH264Vbr.minIQp = 34;
			rcAttr->attrRcMode.attrH264Vbr.maxIQp = 51;
			// rcAttr->attrRcMode.attrH264Vbr.IPfrmQPDelta = -1;
			// rcAttr->attrRcMode.attrH264Vbr.PPfrmQPDelta = -1;
			// rcAttr->attrRcMode.attrH264Cbr.eRcOptions = IMP_ENC_RC_SCN_CHG_RES;
			rcAttr->attrRcMode.attrH264Vbr.maxIPictureSize = uTargetBitRate * 4 / 3;
			rcAttr->attrRcMode.attrH264Vbr.maxPPictureSize = uTargetBitRate * 4 / 3;


			rcAttr->attrRcMode.attrH265Vbr.maxBitRate = uTargetBitRate  * 4 / 3;
			rcAttr->attrRcMode.attrH265Vbr.initialQp = -1;
			rcAttr->attrRcMode.attrH265Vbr.minIQp = 15;
			rcAttr->attrRcMode.attrH265Vbr.maxIQp = 48;
			// rcAttr->attrRcMode.attrH265Vbr.IPfrmQPDelta = -1;
			// rcAttr->attrRcMode.attrH265Vbr.PPfrmQPDelta = -1;
			// rcAttr->attrRcMode.attrH264Cbr.eRcOptions = IMP_ENC_RC_SCN_CHG_RES;
			rcAttr->attrRcMode.attrH265Cbr.maxIPictureSize = uTargetBitRate * 4 / 3;
			rcAttr->attrRcMode.attrH265Cbr.maxPPictureSize = uTargetBitRate * 4 / 3;
			// rcAttr->attrRcMode.attrVbr.uTargetBitRate = uTargetBitRate;
			// rcAttr->attrRcMode.attrVbr.uMaxBitRate = uTargetBitRate * 4 / 3;
			// rcAttr->attrRcMode.attrVbr.iInitialQP = -1;
			// rcAttr->attrRcMode.attrVbr.iMinQP = 34;
			// rcAttr->attrRcMode.attrVbr.iMaxQP = 51;
			// rcAttr->attrRcMode.attrVbr.iIPDelta = -1;
			// rcAttr->attrRcMode.attrVbr.iPBDelta = -1;
			// rcAttr->attrRcMode.attrVbr.eRcOptions = IMP_ENC_RC_SCN_CHG_RES | IMP_ENC_RC_OPT_SC_PREVENTION;
			// rcAttr->attrRcMode.attrVbr.uMaxPictureSize = uTargetBitRate * 4 / 3;
			break;
		case ENC_RC_MODE_CVBR:
			rcAttr->attrRcMode.attrH264CVbr.longMaxBitRate = uTargetBitRate * 4 / 3;
			rcAttr->attrRcMode.attrH265CVbr.longMinBitRate = uTargetBitRate ;
			rcAttr->attrRcMode.attrH264CVbr.initialQp = -1;
			rcAttr->attrRcMode.attrH264CVbr.minIQp = 34;
			rcAttr->attrRcMode.attrH264CVbr.maxIQp = 51;
			// rcAttr->attrRcMode.attrH264CVbr.IPfrmQPDelta = -1;
			// rcAttr->attrRcMode.attrH264CVbr.PPfrmQPDelta = -1;
			// rcAttr->attrRcMode.attrH264Cbr.eRcOptions = IMP_ENC_RC_SCN_CHG_RES;
			rcAttr->attrRcMode.attrH264CVbr.maxIPictureSize = uTargetBitRate * 4 / 3;
			rcAttr->attrRcMode.attrH264CVbr.maxPPictureSize = uTargetBitRate * 4 / 3;

			rcAttr->attrRcMode.attrH265CVbr.longMaxBitRate = uTargetBitRate * 4 / 3;
			rcAttr->attrRcMode.attrH265CVbr.longMinBitRate = uTargetBitRate ;
			rcAttr->attrRcMode.attrH265CVbr.initialQp = -1;
			rcAttr->attrRcMode.attrH265CVbr.minIQp = 34;
			rcAttr->attrRcMode.attrH265CVbr.maxIQp = 51;
			// rcAttr->attrRcMode.attrH265CVbr.IPfrmQPDelta = -1;
			// rcAttr->attrRcMode.attrH265CVbr.PPfrmQPDelta = -1;
			// rcAttr->attrRcMode.attrH264Cbr.eRcOptions = IMP_ENC_RC_SCN_CHG_RES;
			rcAttr->attrRcMode.attrH265CVbr.maxIPictureSize = uTargetBitRate * 4 / 3;
			rcAttr->attrRcMode.attrH265CVbr.maxPPictureSize = uTargetBitRate * 4 / 3;
			// rcAttr->attrRcMode.attrCappedVbr.uTargetBitRate = uTargetBitRate;
			// rcAttr->attrRcMode.attrCappedVbr.uMaxBitRate = uTargetBitRate * 4 / 3;
			// rcAttr->attrRcMode.attrCappedVbr.iInitialQP = -1;
			// rcAttr->attrRcMode.attrCappedVbr.iMinQP = 34;
			// rcAttr->attrRcMode.attrCappedVbr.iMaxQP = 51;
			// rcAttr->attrRcMode.attrCappedVbr.iIPDelta = -1;
			// rcAttr->attrRcMode.attrCappedVbr.iPBDelta = -1;
			// rcAttr->attrRcMode.attrCappedVbr.eRcOptions = IMP_ENC_RC_SCN_CHG_RES | IMP_ENC_RC_OPT_SC_PREVENTION;
			// rcAttr->attrRcMode.attrCappedVbr.uMaxPictureSize = uTargetBitRate * 4 / 3;
			// rcAttr->attrRcMode.attrCappedVbr.uMaxPSNR = 42;
		case ENC_RC_MODE_AVBR:
			rcAttr->attrRcMode.attrH264AVbr.maxBitRate = uTargetBitRate  * 4 / 3;
			rcAttr->attrRcMode.attrH264AVbr.initialQp = -1;
			rcAttr->attrRcMode.attrH264AVbr.minIQp = 34;
			rcAttr->attrRcMode.attrH264AVbr.maxIQp = 51;
			// rcAttr->attrRcMode.attrH264Vbr.IPfrmQPDelta = -1;
			// rcAttr->attrRcMode.attrH264Vbr.PPfrmQPDelta = -1;
			// rcAttr->attrRcMode.attrH264Cbr.eRcOptions = IMP_ENC_RC_SCN_CHG_RES;
			rcAttr->attrRcMode.attrH264AVbr.maxIPictureSize = uTargetBitRate * 4 / 3;
			rcAttr->attrRcMode.attrH264AVbr.maxPPictureSize = uTargetBitRate * 4 / 3;


			rcAttr->attrRcMode.attrH265AVbr.maxBitRate = uTargetBitRate  * 4 / 3;
			rcAttr->attrRcMode.attrH265AVbr.initialQp = -1;
			rcAttr->attrRcMode.attrH265AVbr.minIQp = 15;
			rcAttr->attrRcMode.attrH265AVbr.maxIQp = 48;
			// rcAttr->attrRcMode.attrH265Vbr.IPfrmQPDelta = -1;
			// rcAttr->attrRcMode.attrH265Vbr.PPfrmQPDelta = -1;
			// rcAttr->attrRcMode.attrH264Cbr.eRcOptions = IMP_ENC_RC_SCN_CHG_RES;
			rcAttr->attrRcMode.attrH265AVbr.maxIPictureSize = uTargetBitRate * 4 / 3;
			rcAttr->attrRcMode.attrH265AVbr.maxPPictureSize = uTargetBitRate * 4 / 3;

		case ENC_RC_MODE_SMART:
			rcAttr->attrRcMode.attrH264Smart.maxBitRate = uTargetBitRate  * 4 / 3;
			rcAttr->attrRcMode.attrH264Smart.initialQp = -1;
			rcAttr->attrRcMode.attrH264Smart.minIQp = 34;
			rcAttr->attrRcMode.attrH264Smart.maxIQp = 51;
			// rcAttr->attrRcMode.attrH264Vbr.IPfrmQPDelta = -1;
			// rcAttr->attrRcMode.attrH264Vbr.PPfrmQPDelta = -1;
			// rcAttr->attrRcMode.attrH264Cbr.eRcOptions = IMP_ENC_RC_SCN_CHG_RES;
			rcAttr->attrRcMode.attrH264Smart.maxIPictureSize = uTargetBitRate * 4 / 3;
			rcAttr->attrRcMode.attrH264Smart.maxPPictureSize = uTargetBitRate * 4 / 3;

			rcAttr->attrRcMode.attrH265Smart.maxBitRate = uTargetBitRate  * 4 / 3;
			rcAttr->attrRcMode.attrH265Smart.initialQp = -1;
			rcAttr->attrRcMode.attrH265Smart.minIQp = 15;
			rcAttr->attrRcMode.attrH265Smart.maxIQp = 48;
			// rcAttr->attrRcMode.attrH265Vbr.IPfrmQPDelta = -1;
			// rcAttr->attrRcMode.attrH265Vbr.PPfrmQPDelta = -1;
			// rcAttr->attrRcMode.attrH264Cbr.eRcOptions = IMP_ENC_RC_SCN_CHG_RES;
			rcAttr->attrRcMode.attrH265Smart.maxIPictureSize = uTargetBitRate * 4 / 3;
			rcAttr->attrRcMode.attrH265Smart.maxPPictureSize = uTargetBitRate * 4 / 3;
			break;
		// case IMP_ENC_RC_MODE_CAPPED_QUALITY:
		// 	rcAttr->attrRcMode.attrCappedQuality.uTargetBitRate = uTargetBitRate;
		// 	rcAttr->attrRcMode.attrCappedQuality.uMaxBitRate = uTargetBitRate * 4 / 3;
		// 	rcAttr->attrRcMode.attrCappedQuality.iInitialQP = -1;
		// 	rcAttr->attrRcMode.attrCappedQuality.iMinQP = 34;
		// 	rcAttr->attrRcMode.attrCappedQuality.iMaxQP = 51;
		// 	rcAttr->attrRcMode.attrCappedQuality.iIPDelta = -1;
		// 	rcAttr->attrRcMode.attrCappedQuality.iPBDelta = -1;
		// 	rcAttr->attrRcMode.attrCappedQuality.eRcOptions = IMP_ENC_RC_SCN_CHG_RES | IMP_ENC_RC_OPT_SC_PREVENTION;
		// 	rcAttr->attrRcMode.attrCappedQuality.uMaxPictureSize = uTargetBitRate * 4 / 3;
		// 	rcAttr->attrRcMode.attrCappedQuality.uMaxPSNR = 42;
		// 	break;
		default:
			IMP_LOG_ERR(TAG, "unsupported rcmode:%d, we only support fixqp, cbr vbr and capped vbr\n", rcAttr->attrRcMode.rcMode);
			return -1;
	}
#endif
	return 0;
}

//重启stream0
static int set_resolution_restart_stream0(int picWidth,int picHeight)
{
	int ret = XCAM_SUCCESS;
	IMPFSChnAttr fs0_attr = {
		.pixFmt = PIX_FMT_NV12,
		.outFrmRateNum =FIRST_SENSOR_FRAME_RATE_NUM,
		.outFrmRateDen = 1,
		.nrVBs = 2,
		.type = FS_PHY_CHANNEL,
		.crop.enable = 1,
		.crop.top = 0,
		.crop.left = 0,
		.crop.width = FIRST_SENSOR_WIDTH ,
		.crop.height = FIRST_SENSOR_HEIGHT,
		.scaler.enable = 0,
		.picWidth = SENSOR_RESOLUTION_WIDTH_MAIN,
		.picHeight = SENSOR_RESOLUTION_HEIGHT_MAIN,
	};

	ret = IMP_FrameSource_GetChnAttr(STREAM0, &fs0_attr);
	if (ret < 0 ) {
		LOG_ERR(LOG_TAG,"Call IMP_FrameSource_GetChnAttr fail.\n");
		return ret;
	}

	fs0_attr.picWidth = picWidth;
	fs0_attr.picHeight = picHeight;
	fs0_attr.crop.width = picWidth;
	fs0_attr.crop.height = picHeight;
	fs0_attr.crop.enable = 1;
	int enc_mode=  (stream_attr.stream_config[STREAM0].enc_attr.encAttr.enType == PT_H264 ? IMP_ENC_PROFILE_AVC_MAIN : IMP_ENC_PROFILE_HEVC_MAIN );
	xcam_osd_update_stream_info(STREAM0, picWidth, picHeight);
	ret = IMP_FrameSource_SetChnAttr(STREAM0, &fs0_attr);
	if (ret < 0) {
		LOG_ERR(LOG_TAG,"Call IMP_FrameSource_SetChnAttr fail.\n");
		return ret;
	}

	int enable = 1;
	xcam_conf_get_ivdc_status(&enable);
	IMPEncoderCHNAttr enc0_attr;
	encoder_init(&enc0_attr, enc_mode, ENC_RC_MODE_CBR, fs0_attr.picWidth, fs0_attr.picHeight, 1, fs0_attr.outFrmRateNum, enable, 4*1024);

	ret = IMP_Encoder_CreateChn(ENCCHN0, &enc0_attr);
	if (ret < 0) {
		LOG_ERR(LOG_TAG,"IMP_Encoder_CreateChn(%d) error.\n", ENCCHN0);
		return XCAM_ERROR;
	}

	ret = IMP_Encoder_RegisterChn(ENCGROUP0, ENCCHN0);
	if (ret < 0) {
		LOG_ERR(LOG_TAG,"IMP_Encoder_RegisterChn(%d, %d) error: %d.\n", ENCGROUP0, ENCCHN0, ret);
		return XCAM_ERROR;
	}

	ret =  IMP_Encoder_FlushStream(ENCCHN0);
	if (ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG,"Call IMP_Encode_FlushStream fail.\n");
		return XCAM_ERROR;
	}
	
	ret = IMP_FrameSource_EnableChn(STREAM0);
	if (ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG,"Call IMP_FrameSource_EnableChn(%d) error: %d.\n", STREAM0, ret);
		return XCAM_ERROR;
	}
	ret = IMP_System_Bind(&cell_fs0, &cell_enc0);
	if (ret < 0) {
		LOG_ERR(TAG, "Bind FrameSource channel0 and Encoder failed\n");
		return -1;
	}
	ret = IMP_Encoder_StartRecvPic(ENCCHN0);
	if (ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG,"Call IMP_Encoder_StartRecvPic(%d) error: %d.\n", STREAM0, ret);
		return XCAM_ERROR;
	}
	ret = c_RTSP_start(STREAM0, fs0_attr.picWidth, fs0_attr.picHeight);
	if(ret < XCAM_SUCCESS){
		LOG_ERR(LOG_TAG, "error(%s,%d),Call c_RTSP_start fail.\n");
		return ret;
	}

	_xcam_video_syn_fsAttr(STREAM0,&fs0_attr);
	//向OSD更新改变分辨率后的宽高

	return XCAM_SUCCESS;
}

//重启stream 1
static int set_resolution_restart_stream1(int picWidth,int picHeight)
{
	int ret = XCAM_SUCCESS;
	IMPFSChnAttr fs1_attr = {
		.pixFmt = PIX_FMT_NV12,
		.outFrmRateNum =FIRST_SENSOR_FRAME_RATE_NUM,
		.outFrmRateDen = 1,
		.nrVBs = 2,
		.type = FS_PHY_CHANNEL,
		.crop.enable = 0,
		.scaler.enable = 1,
		.scaler.outwidth = SENSOR_RESOLUTION_WIDTH_SLAVE,
		.scaler.outheight = SENSOR_RESOLUTION_HEIGHT_SLAVE,
		.picWidth = SENSOR_RESOLUTION_WIDTH_SLAVE,
		.picHeight = SENSOR_RESOLUTION_HEIGHT_SLAVE,
	};

	ret = IMP_FrameSource_GetChnAttr(STREAM1, &fs1_attr);
	if(ret < XCAM_SUCCESS ){
		LOG_ERR(LOG_TAG,"Call IMP_FrameSource_GetChnAttr fail.\n");
		return ret;
	}

	fs1_attr.picWidth = picWidth;
	fs1_attr.picHeight = picHeight;
	fs1_attr.scaler.outwidth = picWidth;
	fs1_attr.scaler.outheight = picHeight;

	int enc_mode=  (stream_attr.stream_config[STREAM0].enc_attr.encAttr.enType == PT_H264 ? IMP_ENC_PROFILE_AVC_MAIN : IMP_ENC_PROFILE_HEVC_MAIN );
	ret = IMP_FrameSource_SetChnAttr(STREAM1, &fs1_attr);
	if(ret < XCAM_SUCCESS){
		LOG_ERR(LOG_TAG,"Call IMP_FrameSource_SetChnAttr fail.\n");
		return ret;
	}

	IMPEncoderCHNAttr enc1_attr;
	encoder_init(&enc1_attr, enc_mode, ENC_RC_MODE_CBR, fs1_attr.picWidth, fs1_attr.picHeight, 1, fs1_attr.outFrmRateNum, 0, 1*1024);

	ret = IMP_Encoder_CreateChn(ENCCHN1, &enc1_attr);
	if (ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG,"IMP_Encoder_CreateChn(%d) error.\n", ENCCHN1);
		return XCAM_ERROR;
	}

	ret = IMP_Encoder_RegisterChn(ENCGROUP1, ENCCHN1);
	if (ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG,"IMP_Encoder_RegisterChn(%d, %d) error: %d.\n", ENCGROUP1, ENCCHN1, ret);
		return XCAM_ERROR;
	}

	ret =  IMP_Encoder_FlushStream(ENCCHN1);
	if(ret < XCAM_SUCCESS)
	{
		LOG_ERR(LOG_TAG,"IMP_Encode_FlushStream fail.\n");
		return XCAM_ERROR;
	}

	ret = IMP_FrameSource_EnableChn(STREAM1);
	if (ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG,"IMP_FrameSource_EnableChn(%d) error: %d.\n",ENCCHN1,ret);
		return XCAM_ERROR;
	}
	ret = IMP_System_Bind(&cell_osd1, &cell_enc1);
	if (ret < 0) {
		LOG_ERR(TAG, "Bind OSD channel1 and Encoder failed\n");
		return -1;
	}
	ret = IMP_Encoder_StartRecvPic(ENCCHN1);

	ret = IMP_OSD_Start(OSDGROUP1);
	if (ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG,"IMP_OSD_Start error.\n");
		return XCAM_ERROR;
	}

	_xcam_video_syn_fsAttr(STREAM1, &fs1_attr);
	// xcam_osd_all_osdrgn_restart(STREAM1,picWidth,picHeight);

	return XCAM_SUCCESS;
}

//设置系统分辨率 待完成
int xcam_video_set_resolution(int fsChnNum,int picWidth,int picHeight)
{
	int ret = XCAM_SUCCESS;

	if (fsChnNum >= 2) {
		LOG_ERR(LOG_TAG,"Framesources channel exceeding the limit.\n");
		return XCAM_ERROR;
	}

	if ((picWidth <= 0) || (picWidth > SENSOR_RESOLUTION_WIDTH_MAIN) || (picHeight <= 0) || (picHeight > SENSOR_RESOLUTION_HEIGHT_MAIN)) {
		LOG_ERR(LOG_TAG,"Resolution data error.\n");
		return XCAM_ERROR;
	}

	//判断即将修改的分辨率是否和当前的分辨率相同
	if ((picWidth == stream_attr.stream_config[fsChnNum].fs_attr.picWidth) && (picHeight == stream_attr.stream_config[fsChnNum].fs_attr.picHeight)) {
		LOG_INF(LOG_TAG,"The current resolution config is the same as this one.\n");
		return XCAM_SUCCESS;
	}

	ret = stream_module_stop(fsChnNum);
	if (ret < 0) {
		LOG_ERR(LOG_TAG,"Stop stream module fail.\n");
		// return ret;
	}
LOG_ERR(LOG_TAG,"Stop stream module fail.\n");
	if (fsChnNum == STREAM0) {
		ret = set_resolution_stop_stream0(fsChnNum);
		if (ret < XCAM_SUCCESS) {
			LOG_ERR(LOG_TAG,"error(%s,%d):Call set_resolution_stop_stream fail.\n",__func__,__LINE__);
			(void)stream_module_start(fsChnNum);
			return ret;
		}

		ret = set_resolution_restart_stream0(picWidth, picHeight);
		if (ret < XCAM_SUCCESS) {
			LOG_ERR(LOG_TAG,"error(%s,%d):Call set_resolution_restart_stream fail.\n",__func__,__LINE__);
			(void)stream_module_start(fsChnNum);
			return ret;
		}
	}
	else if(fsChnNum == STREAM1) {
		LOG_ERR(LOG_TAG,"Stop stream module fail.\n");
		ret = set_resolution_stop_stream1(fsChnNum);
		if (ret < XCAM_SUCCESS) {
			LOG_ERR(LOG_TAG,"error(%s,%d):Call set_resolution_stop_stream fail.\n",__func__,__LINE__);
			(void)stream_module_start(fsChnNum);
			return ret;
		}
LOG_ERR(LOG_TAG,"Stop stream module fail.\n");
		ret = set_resolution_restart_stream1(picWidth, picHeight);
		if (ret < XCAM_SUCCESS) {
			LOG_ERR(LOG_TAG,"error(%s,%d):Call set_resolution_restart_stream fail.\n",__func__,__FILE__);
			(void)stream_module_start(fsChnNum);
			return ret;
		}
		LOG_ERR(LOG_TAG,"Stop stream module fail.\n");
	}

	ret = stream_module_start(fsChnNum);
	if (ret < 0) {
		LOG_ERR(LOG_TAG,"Stream module start fail.\n");
		// return ret;
	}
LOG_ERR(LOG_TAG,"Stop stream module fail.\n");
	ret = xcam_conf_set_video_resolution(fsChnNum,picHeight,picWidth);
	if (ret < 0) {
		LOG_INF(LOG_TAG,"Failed to save resolution to json profile.\n");
	}

	return ret;
}

//获取系统的分辨率
int xcam_video_get_resolution(int fsChnNum, int *picWidth, int *picHeight)
{
	int ret = XCAM_SUCCESS;
	IMPFSChnAttr fsChnAttr;

	memset(&fsChnAttr, 0, sizeof(IMPFSChnAttr));

	ret = IMP_FrameSource_GetChnAttr(fsChnNum, &fsChnAttr);
	if (ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG,"Call IMP_FrameSource_GetChnAttr fial.\n");
		return ret;
	}

	*picWidth = fsChnAttr.picWidth;
	*picHeight = fsChnAttr.picHeight;

	return ret;
}

int xcam_video_restart_rtsp()
{
	int ret = XCAM_SUCCESS;
	int streamnum = 0;
	int width = 0,height = 0;

	while (streamnum < 2) {
		ret = stream_module_stop(streamnum);
		if (ret < XCAM_SUCCESS) {
			LOG_ERR(LOG_TAG,"Stop stream module fail.\n");
			// return ret;
		}

		ret = c_RTSP_stop(streamnum);
		if (ret < XCAM_SUCCESS) {
			LOG_ERR(LOG_TAG,"Check out encode mode before c_RTSP_stop fail.\n");
			return ret;
		}

		if (streamnum == 0) {
			 width = SENSOR_RESOLUTION_WIDTH_MAIN;
			 height = SENSOR_RESOLUTION_HEIGHT_MAIN;
		} else if (streamnum == 1) {
			width = SENSOR_RESOLUTION_WIDTH_SLAVE;
			height = SENSOR_RESOLUTION_HEIGHT_SLAVE;
		}

		ret = c_RTSP_start(streamnum, height, width);
		if(ret < XCAM_SUCCESS){
			LOG_ERR(LOG_TAG, "error(%s,%d),Call c_RTSP_start fail.\n");
			return ret;
		}

		ret = stream_module_start(streamnum);
		if (ret < XCAM_SUCCESS) {
			LOG_ERR(LOG_TAG,"Stream module start fail.\n");
			// return ret;
		}

		streamnum ++;
	}

	return ret;
}

static int xcam_video_set_encode_mode_channel0(int enc_mode)
{
	int ret = XCAM_SUCCESS;
	IMPFSChnAttr fs0_attr = {
		.pixFmt = PIX_FMT_NV12,
		.outFrmRateNum = 30,
		.outFrmRateDen = 1,
		.nrVBs = 2,
		.type = FS_PHY_CHANNEL,
		.crop.enable = 1,
		.crop.top = 0,
		.crop.left = 0,
		.crop.width =SENSOR_RESOLUTION_WIDTH_MAIN,
		.crop.height = SENSOR_RESOLUTION_HEIGHT_MAIN,
		.scaler.enable = 0,
		.picWidth = SENSOR_RESOLUTION_WIDTH_MAIN,
		.picHeight = SENSOR_RESOLUTION_HEIGHT_MAIN,
	};

	ret = IMP_FrameSource_GetChnAttr(STREAM0, &fs0_attr);
	if (ret < 0 ) {
		LOG_ERR(LOG_TAG,"Call IMP_FrameSource_GetChnAttr fail.\n");
		return ret;
	}
	ret = c_RTSP_stop(STREAM0);
	if (ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG,"Check out encode mode before c_RTSP_stop fail.\n");
		return ret;
	}

	IMPEncoderCHNStat vencChnStat;
	memset(&vencChnStat, 0, sizeof(IMPEncoderCHNStat));
	//销毁通道
	ret = IMP_Encoder_StopRecvPic(ENCCHN0);
	if (ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG, "set_resolution_stop_stream call IMP_Encode_StopRecvPic.\n");
		return ret;
	}

	ret = IMP_System_UnBind(&cell_fs0, &cell_enc0);
	if (ret < 0) {
		LOG_ERR(TAG, "UnBind FrameSource channel0 and Encoder failed\n");
		return -1;
	}
	ret = IMP_Encoder_Query(ENCCHN0, &vencChnStat);
	if (ret == XCAM_SUCCESS) {
		if (vencChnStat.registered) {
			ret = IMP_Encoder_UnRegisterChn(ENCCHN0);
			if (ret < XCAM_SUCCESS) {
				LOG_ERR(LOG_TAG, "encode unregirter fail\n");
				return ret;
			}
			ret = IMP_Encoder_DestroyChn(ENCCHN0);
			if (ret < XCAM_SUCCESS) {
				LOG_ERR(LOG_TAG, "destroy chn fail\n");
				return ret;
			}
		}
	} else {
		return XCAM_ERROR;
	}

	int enable = 1; 
	xcam_conf_get_ivdc_status(&enable);
	//重新注册通道
	IMPEncoderCHNAttr enc0_attr;
	encoder_init(&enc0_attr, enc_mode, ENC_RC_MODE_VBR, fs0_attr.picWidth, fs0_attr.picHeight, 1, fs0_attr.outFrmRateNum, enable, 4*1024);

	ret = IMP_Encoder_CreateChn(ENCCHN0, &enc0_attr);
	if (ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG, "IMP_Encoder_CreateChn(%d) error !\n", 0);
		return ret;
	}

	ret = IMP_Encoder_RegisterChn(ENCGROUP0, ENCCHN0);
	if (ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG, "IMP_Encoder_RegisterChn(%d, %d) error: %d\n", 0, 0, ret);
		return ret;
	}

	ret = IMP_Encoder_FlushStream(ENCCHN0);
	if(ret < XCAM_SUCCESS)
	{
		LOG_ERR(LOG_TAG, "IMP_Encode_FlushStream fail\n");
		return ret;
	}

	ret = IMP_System_Bind(&cell_fs0, &cell_enc0);
	if (ret < 0) {
		LOG_ERR(TAG, "Bind FrameSource channel0 and Encoder failed\n");
		return -1;
	}
	ret = IMP_Encoder_StartRecvPic(ENCCHN0);
	int modetype = -1;

	if(enc_mode == IMP_ENC_PROFILE_AVC_MAIN){
		modetype = LIVE_RTSP_STREAM_H264;
	}else if(enc_mode == IMP_ENC_PROFILE_HEVC_MAIN){
		modetype = LIVE_RTSP_STREAM_H265;
	}

	ret = c_RTSP_set_stream_type(STREAM0, modetype);
	if(ret < XCAM_SUCCESS){
		LOG_ERR(LOG_TAG, "error(%s,%d),Call c_RTSP_set_stream_type fail.\n");
		return ret;
	}

	ret = c_RTSP_start(STREAM0, fs0_attr.picWidth, fs0_attr.picHeight);
	if(ret < XCAM_SUCCESS){
		LOG_ERR(LOG_TAG, "error(%s,%d),Call c_RTSP_start fail.\n");
		return ret;
	}

	return ret;
}

static int xcam_video_set_encode_mode_channel1(int enc_mode)
{
	int ret = XCAM_SUCCESS;

	IMPEncoderCHNStat vencChnStat;
	memset(&vencChnStat, 0, sizeof(IMPEncoderCHNStat));
	IMPFSChnAttr fs1_attr = {
		.pixFmt = PIX_FMT_NV12,
		.outFrmRateNum = FIRST_SENSOR_FRAME_RATE_NUM,
		.outFrmRateDen = 1,
		.nrVBs = 2,
		.type = FS_PHY_CHANNEL,
		.crop.enable = 0,
		.scaler.enable = 1,
		.scaler.outwidth = SENSOR_RESOLUTION_WIDTH_SLAVE,
		.scaler.outheight = SENSOR_RESOLUTION_HEIGHT_SLAVE,
		.picWidth = SENSOR_RESOLUTION_WIDTH_SLAVE,
		.picHeight = SENSOR_RESOLUTION_HEIGHT_SLAVE,
	};

	ret = c_RTSP_stop(STREAM1);
	if (ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG,"Check out encode mode before c_RTSP_stop fail.\n");
		return ret;
	}

	//销毁通道
	ret = IMP_Encoder_StopRecvPic(ENCCHN1);
	if (ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG,"set_resolution_stop_stream call IMP_Encode_StopRecvPic.\n");
		return ret;
	}
	// 	ret = IMP_FrameSource_DisableChn(ENCCHN1);
	// if (ret < XCAM_SUCCESS) {
	// 	LOG_ERR(LOG_TAG,"set_resolution_stop_stream call IMP_Encode_StopRecvPic fail.\n");
	// 	return ret;
	// }

	ret = IMP_System_UnBind(&cell_osd1, &cell_enc1);
	if (ret < 0) {
		LOG_ERR(TAG, "UnBind OSD channel1 and Encoder failed\n");
		return -1;
	}
	ret = IMP_Encoder_Query(ENCCHN1, &vencChnStat);
	if (ret == XCAM_SUCCESS) {
		if (vencChnStat.registered) {
			ret = IMP_Encoder_UnRegisterChn(ENCCHN1);
			if (ret < XCAM_SUCCESS) {
				LOG_ERR(LOG_TAG,"encode unregirter fail\n");
				return ret;
			}
			ret = IMP_Encoder_DestroyChn(ENCCHN1);
			if (ret < XCAM_SUCCESS) {
				LOG_ERR(LOG_TAG,"destroy chn fail\n");
				return ret;
			}

		}
	} else {
		return XCAM_ERROR;
	}

	IMPEncoderCHNAttr enc1_attr;
	encoder_init(&enc1_attr, enc_mode, ENC_RC_MODE_VBR, fs1_attr.picWidth, fs1_attr.picHeight, 1, fs1_attr.outFrmRateNum, 0, 1*1024);

	ret = IMP_Encoder_CreateChn(ENCCHN1, &enc1_attr);
	if (ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG,"IMP_Encoder_CreateChn error !\n");
		return ret;
	}

	ret = IMP_Encoder_RegisterChn(ENCGROUP1, ENCCHN1);
	if (ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG,"IMP_Encoder_RegisterChn(%d, %d) error: %d\n", 0, 0, ret);
		return ret;
	}

	ret = IMP_Encoder_FlushStream(ENCCHN1);
	if (ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG,"IMP_Encode_FlushStream fail\n");
		return ret;
	}

	// ret = IMP_FrameSource_EnableChn(ENCCHN1);
	// if (ret < XCAM_SUCCESS) {
	// 	LOG_ERR(LOG_TAG,"IMP_FrameSource_EnableChn(%d) error: %d\n", ret, 0);
	// 	return ret;
	// }
	ret = IMP_System_Bind(&cell_osd1, &cell_enc1);
	if (ret < 0) {
		LOG_ERR(TAG, "Bind OSD channel1 and Encoder failed\n");
		return -1;
	}
	ret = IMP_Encoder_StartRecvPic(ENCCHN1);
	int modetype = -1;

	if (enc_mode == IMP_ENC_PROFILE_AVC_MAIN) {
		modetype = LIVE_RTSP_STREAM_H264;
	} else if(enc_mode == IMP_ENC_PROFILE_HEVC_MAIN) {
		modetype = LIVE_RTSP_STREAM_H265;
	}

	ret = c_RTSP_set_stream_type(STREAM1,modetype);
	if (ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG,"error(%s,%d),Call c_RTSP_set_stream_type fail.\n");
		return ret;
	}

	ret = c_RTSP_start(STREAM1, SENSOR_RESOLUTION_WIDTH_SLAVE, SENSOR_RESOLUTION_HEIGHT_SLAVE);
	if (ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG,"error(%s,%d),Call c_RTSP_start fail.\n");
		return ret;
	}

	return ret;
}

int xcam_video_set_encode_mode(int streamnum, int enc_type)
{
	int ret = XCAM_SUCCESS;
	int enc_mode = -1;
	if (enc_type == PT_H264) {
		enc_mode = IMP_ENC_PROFILE_AVC_MAIN;
	} else if (enc_type == PT_H265) {
		enc_mode = IMP_ENC_PROFILE_HEVC_MAIN;
	} else {
		LOG_ERR(LOG_TAG,"Don't support this encode type checkout.\n");
		return XCAM_ERROR;
	}

	if (enc_type == stream_attr.stream_config[streamnum].enc_attr.encAttr.enType ) {
		LOG_INF(LOG_TAG,"Info(%s,%d),encode type configuration is same as curreently configuration.\n",__func__,__LINE__);
		return XCAM_SUCCESS;
	}

	ret = stream_module_stop(streamnum);
	if (ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG,"Stop stream module fail.\n");
		return ret;
	}

	if (streamnum == STREAM0) {
		ret = xcam_video_set_encode_mode_channel0(enc_mode);
		if(ret < 0){
			LOG_ERR(LOG_TAG,"set channel0 encode mode fail.\n");
			return ret;
		}
	} else if(streamnum == STREAM1) {
		ret = xcam_video_set_encode_mode_channel1(enc_mode);
		if (ret < XCAM_SUCCESS) {
			LOG_ERR(LOG_TAG,"set channel1 encode mode fail.\n");
			return ret;
		}
	}

	ret = stream_module_start(streamnum);
	if (ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG,"Stream module start fail.\n");
		// return ret;
	}

	//配置同步和配置保存
	_xcam_video_syn_encmode(streamnum,enc_mode);
	/*
	ret = xcam_conf_set_video_enctype(streamnum, enc_type);
	if (ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG,"Save profile faile.\n");
		return ret;
	}
	*/
	return ret;
}

int xcam_video_get_encode_mode(int streamnum, int *enc_type)
{
	int ret = XCAM_SUCCESS;

	IMPPayloadType enctype;
	ret = IMP_Encoder_GetChnEncType (streamnum,&enctype);
	if (ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG,"Get encode mode fail.\n");
		return ret;
	}

	*enc_type = enctype;
	return ret;
}

int  xcan_vide_encode_attrRcMode_init(int channel, int  rcmode, IMPEncoderAttrRcMode *pstAttrRcMode)
{
	assert(pstAttrRcMode != NULL);
	IMPEncoderCHNAttr attr;
	int ret = XCAM_SUCCESS;

	// attr.encAttr.enType = PT_H265;
	int enable = 1;
	int bps = channel == 0 ? 4 * 1024 : 1024 ;
	int enc_mode=  (stream_attr.stream_config[channel].enc_attr.encAttr.enType == PT_H264 ? IMP_ENC_PROFILE_AVC_MAIN : IMP_ENC_PROFILE_HEVC_MAIN );
	xcam_conf_get_ivdc_status(&enable);
	//这个地方的帧率不应该就这样写死了
	ret = encoder_init(&attr, enc_mode, rcmode,stream_attr.stream_config[channel].fs_attr.picWidth, stream_attr.stream_config[channel].fs_attr.picHeight, 1, FIRST_SENSOR_FRAME_RATE_NUM, enable, bps);
	if (ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG,"error(%s,%d),rcmode init fail.ret=%d\n",__func__, __LINE__, ret);
		return XCAM_ERROR;
	}

	memcpy(pstAttrRcMode, &attr.rcAttr.attrRcMode, sizeof(IMPEncoderAttrRcMode));
	return XCAM_SUCCESS;
}

/*设置编码的控制属性*/
int xcam_video_set_encode_Rcmode(int channel, int rcmode)
{
	int ret = XCAM_SUCCESS;
	IMPEncoderAttrRcMode rcModeAttr;
	memset(&rcModeAttr, 0,sizeof(IMPEncoderAttrRcMode));

	ret = IMP_Encoder_GetChnAttrRcMode(channel, &rcModeAttr);
	if (ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG,"error(%s,%d),get rncode chn attr rcmode fail.\n",__func__,__FILE__);
		return XCAM_ERROR;
	}
	
	ret = xcan_vide_encode_attrRcMode_init(channel, rcmode, &rcModeAttr);
	if (ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG,"error(%s,%d),get attRcMode init fail.\n",__func__, __LINE__);
		return XCAM_ERROR;
	}

	// if (rcModeAttr.rcMode != rcmode) {
		ret = IMP_Encoder_SetChnAttrRcMode(channel, &rcModeAttr);
		if (ret < XCAM_SUCCESS) {
			LOG_ERR(LOG_TAG,"error(%s,%d),set rncode chn arrt rcmode fail.\n",__func__,__LINE__);
			return XCAM_ERROR;
		}
	// }

	//保存配置
	memcpy(&stream_attr.stream_config[channel].enc_attr.rcAttr.attrRcMode, &rcModeAttr, sizeof(IMPEncoderAttrRcMode));

	return ret;
}

/*获取编码的控制属性*/
int xcam_video_get_encode_Rcmode(int channel , int *rcmode)
{
	int ret = XCAM_SUCCESS;
	IMPEncoderAttrRcMode rcModeAttr;
	memset(&rcModeAttr, 0,sizeof(IMPEncoderAttrRcMode));

	ret = IMP_Encoder_GetChnAttrRcMode(channel, &rcModeAttr);
	if (ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG,"error(%s,%d),get rncode chn attr rcmode fail.\n",__func__,__FILE__);
		return XCAM_ERROR;
	}

	*rcmode = rcModeAttr.rcMode;
	return ret;
}

/*获取I帧的时间间隔*/
int xcam_video_get_encode_GopLength(int channel ,int *gopLength)
{
	int ret = XCAM_SUCCESS;
	IMPEncoderGOPSizeCfg gopAttr;
	memset(&gopAttr, 0, sizeof(IMPEncoderGOPSizeCfg));

	ret = IMP_Encoder_GetGOPSize(channel, &gopAttr);
	if (ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG,"error(%s,%d),get this channel GopLength fail. ret=%d\n",__func__,__LINE__, ret);
		return ret;
	}
	*gopLength = gopAttr.gopsize;
	return ret;
}

/*设置I帧的时间间隔*/
int xcam_video_set_encode_GopLength(int channel, int gopLength)
{
	int ret = XCAM_SUCCESS;
	/*判断gopLeng值的合法范围*/
	if ((gopLength < 0) || (gopLength > 360)) {
		LOG_ERR(LOG_TAG,"error(%s,%d),this params is error.\n",__func__,__LINE__);
		return XCAM_ERROR;
	}

	int uGopLength = gopLength ;
	IMPEncoderGOPSizeCfg pstGOPSizeCfg;
	pstGOPSizeCfg.gopsize = uGopLength;
	ret = IMP_Encoder_SetGOPSize(channel, &pstGOPSizeCfg);
	if(ret != 0)
	{
		LOG_ERR(LOG_TAG,"IMP_Encoder_SetChnGopLength [%d], %d, %s error.\n", channel, __LINE__,__func__);
		return XCAM_ERROR;
	}
	stream_attr.stream_config[channel].pstGOPSizeCfg.gopsize = uGopLength;
	return ret;
}

/*hesadecimal string to intager*/
int xstr_to_xint(char* pcStr)
{
	unsigned int uiStrLen = 0;
	int sensor_i2c_temp = 0;
	int i = 0;

	uiStrLen = strlen(pcStr);
	if ((pcStr[0] == '0') && ((pcStr[1] =='x') || (pcStr[1] == 'X'))) {
		i = 2;
	}

	for ( ; i < uiStrLen; i++) {
		if ((pcStr[i] >= '0') && (pcStr[i] <= '9')) {
			sensor_i2c_temp += (pcStr[i] - '0')*pow(16,(uiStrLen-(i+1)));
		} else if((pcStr[i] >= 'A') && (pcStr[i] <= 'F')) {
			sensor_i2c_temp += (pcStr[i] - 'A' + 10)*pow(16,(uiStrLen-(i+1)));
		} else if((pcStr[i] >= 'a') && (pcStr[i] <= 'f')) {
			sensor_i2c_temp += (pcStr[i] - 'a' + 10)*pow(16,(uiStrLen-(i+1)));
		} else {
			LOG_ERR(LOG_TAG,"Sensor i2c enviroment format error.\n");
			return XCAM_ERROR;
		}
	}

	return sensor_i2c_temp;
}

//获取系统sensor的参数
int get_sensor_params()
{
	char* sensor_env = NULL;

	/* 获取sensor name*/
	sensor_env = getenv("SENSOR_NAME");
	if (NULL == sensor_env) {
		LOG_ERR(LOG_TAG,"Get sensor name fail.\n");
		return XCAM_ERROR;
	}
	strlcpy((char *)sensor_name, sensor_env, 10);
	sensor_env = NULL;

	/*获取 sensor i2c*/
	sensor_env = getenv("SENSOR_I2C");
	if (NULL == sensor_env) {
		LOG_ERR(LOG_TAG,"Get sensor i2c string fail.\n");
		return XCAM_ERROR;
	}
	sensor_i2c = xstr_to_xint(sensor_env);
	if (XCAM_ERROR == sensor_i2c) {
		LOG_ERR(LOG_TAG,"Get sensor i2c int fail.\n");
		return XCAM_ERROR;
	}
	sensor_env = NULL;

	/*获取 sensor width*/
	sensor_env = getenv("SENSOR_WIDTH");
	if (NULL == sensor_env) {
		LOG_ERR(LOG_TAG,"Get sensor width fail.\n");
		return XCAM_ERROR;
	}
	sensor_width = atoi(sensor_env);
	if ((0 == sensor_width) || (-1 == sensor_width)) {
		LOG_ERR(LOG_TAG,"Get sensor width fail.\n");
		return XCAM_ERROR;
	}
	sensor_env = NULL;

	/*获取 sensor heigth*/
	sensor_env = getenv("SENSOR_HEIGHT");
	if (NULL == sensor_env) {
		LOG_ERR(LOG_TAG,"Get sensor heigth fail.\n");
		return XCAM_ERROR;
	}
	sensor_height = atoi(sensor_env);
	if ((0 == sensor_height) || (-1 == sensor_height)) {
		LOG_ERR(LOG_TAG,"Get sensor heigth fail int fail.\n");
		return XCAM_ERROR;
	}

	return XCAM_SUCCESS;
}

//xcam系统初始化
static int _system_init()
{
	int ret = XCAM_SUCCESS;
#ifdef XCAM_DOUBLE_SENSOR
	IMPISPCameraInputMode mode = {
			.sensor_num = 2,
	};
#endif
	memset(&sensor_info_t41, 0, sizeof(sensor_info_t41) * 2);
	memcpy(&sensor_info_t41[0], &Def_Sensor_Info[0], sizeof(IMPSensorInfo));
	memcpy(&sensor_info_t41[1], &Def_Sensor_Info[1], sizeof(IMPSensorInfo));
	IMP_OSD_SetPoolSize(96*1024);
	IMP_ISP_Tuning_SetOsdPoolSize(170 * 1024);
	// IMP_Encoder_SetJpegBsSize()
	IMP_Encoder_SetJpegBsSize(500 * 1024);
	IMP_Encoder_SetIvpuBsSize(512 * 1024);

	if(IMP_Encoder_SetMultiSectionMode(0 , 100, 4) < 0 ){
		IMP_LOG_ERR(TAG, "failed to set IMP_Encoder_SetMultiSectionMode \n");
	}
	LOG_DBG(TAG, "system_init start\n");

	ret = IMP_ISP_Open();
	if (ret < XCAM_SUCCESS) {
		IMP_LOG_ERR(TAG, "failed to open ISP\n");
		return XCAM_ERROR;
	}

#ifdef XCAM_DOUBLE_SENSOR
 	ret = IMP_ISP_SetCameraInputMode(&mode);
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "IMP_ISP_SetCameraInputMode failed\n");
        return -1;
    }
#endif

	/* add sensor */
	ret = IMP_ISP_AddSensor(IMPVI_MAIN, &sensor_info_t41[0]);
	if(ret < 0){
		IMP_LOG_ERR(TAG, "failed to AddSensor\n");
		return -1;
	}

#ifdef XCAM_DOUBLE_SENSOR
		/* add sensor */
	ret = IMP_ISP_AddSensor(IMPVI_SEC, &sensor_info_t41[1]);
	if(ret < 0){
		IMP_LOG_ERR(TAG, "failed to AddSensor\n");
		return -1;
	}
#endif

	/* enable sensor */
	ret = IMP_ISP_EnableSensor(IMPVI_MAIN, &sensor_info_t41[0]);
	if(ret < 0){
		IMP_LOG_ERR(TAG, "failed to EnableSensor\n");
		return -1;
	}

#ifdef XCAM_DOUBLE_SENSOR
	/* enable sensor */
	ret = IMP_ISP_EnableSensor(IMPVI_SEC, &sensor_info_t41[1]);
	if(ret < 0){
		IMP_LOG_ERR(TAG, "failed to EnableSensor\n");
		return -1;
	}
#endif

	ret = IMP_System_Init();
	if (ret < XCAM_SUCCESS) {
		IMP_LOG_ERR(TAG, "IMP_System_Init failed\n");
		return XCAM_ERROR;

	}

	/* enable turning, to debug graphics */
	ret = IMP_ISP_EnableTuning();
	if (ret < XCAM_SUCCESS) {
		IMP_LOG_ERR(TAG, "IMP_ISP_EnableTuning failed\n");
		return XCAM_ERROR;
	}

	IMPISPAeExpListAttr aeexpattr;
    ret = IMP_ISP_Tuning_GetAeExpList(IMPVI_MAIN,&aeexpattr);
    if(ret < 0) {
    	IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_GetAeExpList error\n");               
        //return -1;
    }   
    aeexpattr.mode = IMPISP_TUNING_OPS_MODE_DISABLE;
    ret = IMP_ISP_Tuning_SetAeExpList(IMPVI_MAIN,&aeexpattr);
    if(ret < 0) {
        IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_SetAeExpList error\n");
        //return -1;
    } 

	// unsigned char value = 128;
	// IMP_ISP_Tuning_SetContrast(IMPVI_MAIN, &value);
	// IMP_ISP_Tuning_SetSharpness(IMPVI_MAIN, &value);
	// IMP_ISP_Tuning_SetSaturation(IMPVI_MAIN, &value);
	// IMP_ISP_Tuning_SetBrightness(IMPVI_MAIN, &value);

	IMPISPRunningMode dn = IMPISP_RUNNING_MODE_DAY;
	ret = IMP_ISP_Tuning_SetISPRunningMode(IMPVI_MAIN, &dn);
	if (ret < 0){
		IMP_LOG_ERR(TAG, "failed to set running mode\n");
		return -1;
	}
	IMPISPSensorFps setFps = { FIRST_SENSOR_FRAME_RATE_NUM, FIRST_SENSOR_FRAME_RATE_DEN };
	ret = IMP_ISP_Tuning_SetSensorFPS(IMPVI_MAIN, &setFps);
	if (ret < 0){
		IMP_LOG_ERR(TAG, "failed to set sensor fps\n");
		return -1;
	}
	IMP_LOG_DBG(TAG, "ImpSystemInit success\n");

	return XCAM_SUCCESS;
}

#define AIISP
//初始换图像流通道0
static int _stream0_init()
{
	int ret = XCAM_SUCCESS;

	IMPFSChnAttr fs0_attr;
	// memset(&fs0_attr, 0, sizeof(fs0_attr));
	// fs0_attr.pixFmt = PIX_FMT_NV12;
	// fs0_attr.outFrmRateNum = 25;
	// fs0_attr.outFrmRateDen = 1;
	// fs0_attr.nrVBs = 2;
	// fs0_attr.type = FS_PHY_CHANNEL;
	// fs0_attr.crop.enable = 1;
	// fs0_attr.crop.top = 0;
	// fs0_attr.crop.left = 0;
	// fs0_attr.crop.width = SENSOR_RESOLUTION_WIDTH_MAIN;
	// fs0_attr.crop.height = SENSOR_RESOLUTION_HEIGHT_MAIN;
	// fs0_attr.scaler.enable = 0;
	// fs0_attr.picWidth = SENSOR_RESOLUTION_WIDTH_MAIN;
	// fs0_attr.picHeight = SENSOR_RESOLUTION_HEIGHT_MAIN;
	fs0_attr = chn[0].fs_chn_attr;
	fs0_attr.outFrmRateNum = FIRST_SENSOR_FRAME_RATE_NUM;
	fs0_attr.outFrmRateDen = FIRST_SENSOR_FRAME_RATE_DEN;
	//配置从配置文件中恢复
	/*fs0_attr.crop.height = xcam_video_conf.video_resolution.ch[STREAM0].picheight;
	  fs0_attr.crop.width = xcam_video_conf.video_resolution.ch[STREAM0].picwidth;
	  fs0_attr.picHeight = xcam_video_conf.video_resolution.ch[STREAM0].picheight;
	  fs0_attr.picWidth = xcam_video_conf.video_resolution.ch[STREAM0].picwidth;
	  fs0_attr.outFrmRateNum = xcam_video_conf.video_fs_fps.ch[STREAM0].fps_num;
	  fs0_attr.outFrmRateDen = xcam_video_conf.video_fs_fps.ch[STREAM0].fps_den;
	*/

	int enc_mode = IMP_ENC_PROFILE_HEVC_MAIN;
	if (xcam_video_conf.conf_enc.ch0_type == PT_H264) {
		enc_mode = IMP_ENC_PROFILE_AVC_MAIN;
	} else if (xcam_video_conf.conf_enc.ch0_type == PT_H265) {
		enc_mode = IMP_ENC_PROFILE_HEVC_MAIN;
	}

	ret = IMP_FrameSource_CreateChn(STREAM0, &fs0_attr);
	if (ret < XCAM_SUCCESS) {
		IMP_LOG_ERR(TAG, "IMP_FrameSource_CreateChn(chn%d) error !\n", STREAM0);
		return XCAM_ERROR;
	}

	ret = IMP_Encoder_CreateGroup(ENCGROUP0);
	if (ret < XCAM_SUCCESS) {
		IMP_LOG_ERR(TAG, "IMP_Encoder_CreateGroup(%d) error !\n", STREAM0);
		return XCAM_ERROR;
	}

	IMPEncoderCHNAttr enc0_attr;
	int enable = 1;
	xcam_conf_get_ivdc_status(&enable);
	encoder_init(&enc0_attr, enc_mode, ENC_RC_MODE_CBR, fs0_attr.picWidth, fs0_attr.picHeight, 1, fs0_attr.outFrmRateNum, enable, 4 * 2 * 1024);
	if (ret < XCAM_SUCCESS) {
		IMP_LOG_ERR(TAG, "encoder_init(%d) error !\n", STREAM0);
		return XCAM_ERROR;
	}

	ret = IMP_Encoder_CreateChn(ENCCHN0, &enc0_attr);
	if (ret < XCAM_SUCCESS) {
		IMP_LOG_ERR(TAG, "IMP_Encoder_CreateChn(%d) error !\n", 0);
		return XCAM_ERROR;
	}

	ret = IMP_Encoder_RegisterChn(ENCGROUP0, ENCCHN0);
	if (ret < XCAM_SUCCESS) {
		IMP_LOG_ERR(TAG, "IMP_Encoder_RegisterChn(%d, %d) error: %d\n", 0, 0, ret);
		return XCAM_ERROR;
	}

	ret = IMP_System_Bind(&cell_fs0, &cell_enc0);
	if (ret < XCAM_SUCCESS) {
		IMP_LOG_ERR(TAG, "Bind FrameSource channel%d and OSD failed\n",0);
		return XCAM_ERROR;
	}

	ret = IMP_FrameSource_EnableChn(STREAM0);
	if (ret < XCAM_SUCCESS) {
		LOG_ERR(TAG, "IMP_FrameSource_EnableChn(%d) error: %d\n", ret, 0);
		return XCAM_ERROR;
	}

#if defined DL_ENABLE
	// IMPOSDRgnAttr rAttr = {
	// 	.type = OSD_REG_RECT,
	// 	.rect = {{0, 0}, {0, 0}},
	// 	.fmt = PIX_FMT_MONOWHITE,
	// 	.data = {
	// 		.lineRectData = {
	// 			.color = OSD_GREEN,
	// 			.linewidth = 1,
	// 		},
	// 	},
	// };

	// IMPOSDGrpRgnAttr grAttr;
	// memset(&grAttr, 0, sizeof(IMPOSDGrpRgnAttr));
	// grAttr.scalex = 1;
	// grAttr.scaley = 1;

	// int m = 0;
	// for (m = 0; m < 10; m++) {
	// 	ivsRgnHandler[m] = IMP_OSD_CreateRgn(&rAttr);
	// 	if (ivsRgnHandler[m] == INVHANDLE) {
	// 		IMP_LOG_ERR(TAG, "IVS IMP_OSD_CreateRgn %d failed\n", m);
	// 		return NULL;
	// 	}
	// 	if (IMP_OSD_RegisterRgn(ivsRgnHandler[m], 0, &grAttr) < 0) {
	// 		IMP_LOG_ERR(TAG, "IVS IMP_OSD_RegisterRgn %d failed\n",ivsRgnHandler[m]);
	// 		return NULL;
	// 	}
	// }
#endif
#ifdef AIISP
	// int ai_enable;
	// xcam_conf_get_extraargaiisp_status(&ai_enable);
	// if(ai_enable){
	// 	ret = xcam_gekko_init();
	// 	if (ret < XCAM_SUCCESS) {
	// 		IMP_LOG_ERR(TAG, "xcam_gekko_init failed\n");
	// 		return XCAM_ERROR;
	// 	}
	// 	ret = xcam_ai3drn_init();
	// 	if (ret < XCAM_SUCCESS) {
	// 		IMP_LOG_ERR(TAG, "xcam_ai3drn_init failed\n");
	// 		return XCAM_ERROR;
	// 	}
	// 	ret = xcam_gekko_start();
	// 	if (ret < XCAM_SUCCESS) {
	// 		IMP_LOG_ERR(TAG, "xcam_gekko_start failed\n");
	// 		return XCAM_ERROR;
	// 	}
	// 	xcam_bv_enable();
	// }

#endif
	/*配置同步*/
	_xcam_video_syn_encAttr(STREAM0, &enc0_attr);
	_xcam_video_syn_fsAttr(STREAM0, &fs0_attr);

	return XCAM_SUCCESS;
}

//初始化图像流通道1
static int _stream1_init()
{
	int ret = XCAM_SUCCESS;

	IMPFSChnAttr fs1_attr = {
		.pixFmt = PIX_FMT_NV12,
		.outFrmRateNum = FIRST_SENSOR_FRAME_RATE_NUM,
		.outFrmRateDen = 1,
		.nrVBs = 2,
		.type = FS_PHY_CHANNEL,
		.crop.enable = 0,
		.scaler.enable = 1,
		.scaler.outwidth = SENSOR_RESOLUTION_WIDTH_SLAVE,
		.scaler.outheight = SENSOR_RESOLUTION_HEIGHT_SLAVE,
		.picWidth = SENSOR_RESOLUTION_WIDTH_SLAVE,
		.picHeight = SENSOR_RESOLUTION_HEIGHT_SLAVE,
		//	.fcrop.enable = 0,
	};

	/*fs1_attr.scaler.outheight = xcam_video_conf.video_resolution.ch[STREAM1].picheight;
	fs1_attr.scaler.outwidth = xcam_video_conf.video_resolution.ch[STREAM1].picwidth;
	fs1_attr.picHeight = xcam_video_conf.video_resolution.ch[STREAM1].picheight;
	fs1_attr.picWidth = xcam_video_conf.video_resolution.ch[STREAM1].picwidth; */

	int enc_mode = IMP_ENC_PROFILE_HEVC_MAIN;
	if(xcam_video_conf.conf_enc.ch1_type == PT_H264) {
		enc_mode = IMP_ENC_PROFILE_AVC_MAIN;
	} else if (xcam_video_conf.conf_enc.ch1_type == PT_H265) {
		enc_mode = IMP_ENC_PROFILE_HEVC_MAIN;
	}

	ret = IMP_FrameSource_CreateChn(STREAM1, &fs1_attr);
	if (ret < XCAM_SUCCESS) {
		IMP_LOG_ERR(TAG, "IMP_FrameSource_CreateChn(chn%d) error !\n", STREAM1);
		return XCAM_ERROR;
	}

	ret = IMP_FrameSource_SetChnAttr(STREAM1, &fs1_attr);
	if (ret < 0) {
		IMP_LOG_ERR(LOG_TAG, "IMP_FrameSource_SetChnAttr(chn%d) error !\n",  STREAM1);
		return XCAM_ERROR;
	}

	ret = IMP_OSD_CreateGroup(STREAM1);
	if (ret < 0) {
		IMP_LOG_ERR(LOG_TAG, "IMP_Encoder_CreateGroup(%d) error !\n", STREAM1);
		return XCAM_ERROR;
	}

	ret = IMP_Encoder_CreateGroup(ENCGROUP1);
	if (ret < XCAM_SUCCESS) {
		IMP_LOG_ERR(TAG, "IMP_Encoder_CreateGroup(%d) error !\n", STREAM1);
		return XCAM_ERROR;
	}

	IMPEncoderCHNAttr enc1_attr;
	encoder_init(&enc1_attr, enc_mode, ENC_RC_MODE_VBR, fs1_attr.picWidth, fs1_attr.picHeight, 1, fs1_attr.outFrmRateNum, 0, 1*1024);
	if (ret < XCAM_SUCCESS) {
		IMP_LOG_ERR(TAG, "encoder_init(%d) error !\n", STREAM1);
		return XCAM_ERROR;
	}

	ret = IMP_Encoder_CreateChn(ENCCHN1, &enc1_attr);
	if (ret < XCAM_SUCCESS) {
		IMP_LOG_ERR(TAG, "IMP_Encoder_CreateChn(%d) error !\n", STREAM1);
		return XCAM_ERROR;
	}

	ret = IMP_Encoder_RegisterChn(ENCGROUP1, ENCCHN1);
	if (ret < XCAM_SUCCESS) {
		IMP_LOG_ERR(TAG, "IMP_Encoder_RegisterChn(%d, %d) error: %d\n", 1, 1, ret);
		return XCAM_ERROR;
	}

	ret = IMP_System_Bind(&cell_fs1, &cell_osd1);
	if (ret < XCAM_SUCCESS) {
		IMP_LOG_ERR(TAG, "Bind channel%d FrameSource and OSD failed\n",1);
		return XCAM_ERROR;
	}

	ret = IMP_System_Bind(&cell_osd1, &cell_enc1);
	if (ret < XCAM_SUCCESS) {
		IMP_LOG_ERR(TAG, "Bind channel%d OSD and ENC failed\n", 1);
		return XCAM_ERROR;
	}

	ret = IMP_FrameSource_EnableChn(ENCCHN1);
	if (ret < XCAM_SUCCESS) {
		IMP_LOG_ERR(TAG, "IMP_FrameSource_EnableChn(%d) error: %d\n", ret, 1);
		return XCAM_ERROR;
	}

#if defined DL_ENABLE
	// IMPOSDRgnAttr rAttr = {
	// 	.type = OSD_REG_RECT,
	// 	.rect = {{0, 0}, {0, 0}},
	// 	.fmt = PIX_FMT_MONOWHITE,
	// 	.data = {
	// 		.lineRectData = {
	// 			.color = OSD_GREEN,
	// 			.linewidth = 1,
	// 		},
	// 	},
	// };

	// IMPOSDGrpRgnAttr grAttr;
	// memset(&grAttr, 0, sizeof(IMPOSDGrpRgnAttr));
	// grAttr.scalex = 1;
	// grAttr.scaley = 1;
	// int m = 0;
	// for (m = 0; m < 10; m++) {
	// 	ivsRgnHandler_second[m] = IMP_OSD_CreateRgn(&rAttr);
	// 	if (ivsRgnHandler_second[m] == INVHANDLE) {
	// 		IMP_LOG_ERR(TAG, "IVS IMP_OSD_CreateRgn %d failed\n", m);
	// 		return NULL;
	// 	}
	// 	if (IMP_OSD_RegisterRgn(ivsRgnHandler_second[m], 1, &grAttr) < 0) {
	// 		IMP_LOG_ERR(TAG, "IVS IMP_OSD_RegisterRgn %d failed\n",ivsRgnHandler_second[m]);
	// 		return NULL;
	// 	}
	// }
#endif

	/*配置同步*/
	_xcam_video_syn_encAttr(STREAM1, &enc1_attr);
	_xcam_video_syn_fsAttr(STREAM1, &fs1_attr);

	return XCAM_SUCCESS;
}


//初始化图像流通道1
static int _stream2_init()
{
	int ret = XCAM_SUCCESS;

	IMPFSChnAttr fs2_attr = {
			.pixFmt = PIX_FMT_NV12,
			.outFrmRateNum = FIRST_SENSOR_FRAME_RATE_NUM,
			.outFrmRateDen = FIRST_SENSOR_FRAME_RATE_DEN,
			.nrVBs = 1,
			.type = FS_PHY_CHANNEL,

			.crop.enable = 0,
			.crop.top = 0,
			.crop.left = 0,
			.crop.width = FIRST_SENSOR_WIDTH,
			.crop.height = FIRST_SENSOR_HEIGHT,
			.scaler.enable = 1,
			.scaler.outwidth = FIRST_SENSOR_WIDTH_THIRD,
			.scaler.outheight = FIRST_SENSOR_HEIGHT_THIRD,

			.picWidth = FIRST_SENSOR_WIDTH_THIRD,
			.picHeight = FIRST_SENSOR_HEIGHT_THIRD,
	};

	ret = IMP_FrameSource_CreateChn(STREAM2, &fs2_attr);
	if (ret < XCAM_SUCCESS) {
		IMP_LOG_ERR(TAG, "IMP_FrameSource_CreateChn(chn%d) error !\n", STREAM2);
		return XCAM_ERROR;
	}

	ret = IMP_FrameSource_SetChnAttr(STREAM2, &fs2_attr);
	if (ret < 0) {
		IMP_LOG_ERR(LOG_TAG, "IMP_FrameSource_SetChnAttr(chn%d) error !\n",  STREAM2);
		return XCAM_ERROR;
	}

	ret = IMP_FrameSource_EnableChn(STREAM2);
	if (ret < XCAM_SUCCESS) {
		IMP_LOG_ERR(TAG, "IMP_FrameSource_EnableChn(%d) error: %d\n", ret, 1);
		return XCAM_ERROR;
	}

	return XCAM_SUCCESS;
}

#ifdef XCAM_DOUBLE_SENSOR
static int _stream3_init()
{
	int ret = XCAM_SUCCESS;

	IMPFSChnAttr fs3_attr;
	// memset(&fs0_attr, 0, sizeof(fs0_attr));
	// fs0_attr.pixFmt = PIX_FMT_NV12;
	// fs0_attr.outFrmRateNum = 25;
	// fs0_attr.outFrmRateDen = 1;
	// fs0_attr.nrVBs = 2;
	// fs0_attr.type = FS_PHY_CHANNEL;
	// fs0_attr.crop.enable = 1;
	// fs0_attr.crop.top = 0;
	// fs0_attr.crop.left = 0;
	// fs0_attr.crop.width = SENSOR_RESOLUTION_WIDTH_MAIN;
	// fs0_attr.crop.height = SENSOR_RESOLUTION_HEIGHT_MAIN;
	// fs0_attr.scaler.enable = 0;
	// fs0_attr.picWidth = SENSOR_RESOLUTION_WIDTH_MAIN;
	// fs0_attr.picHeight = SENSOR_RESOLUTION_HEIGHT_MAIN;
	fs3_attr = chn[3].fs_chn_attr;
	// fs0_attr.outFrmRateNum = FIRST_SENSOR_FRAME_RATE_NUM;
	// fs0_attr.outFrmRateDen = FIRST_SENSOR_FRAME_RATE_DEN;
	//配置从配置文件中恢复
	/*fs0_attr.crop.height = xcam_video_conf.video_resolution.ch[STREAM0].picheight;
	  fs0_attr.crop.width = xcam_video_conf.video_resolution.ch[STREAM0].picwidth;
	  fs0_attr.picHeight = xcam_video_conf.video_resolution.ch[STREAM0].picheight;
	  fs0_attr.picWidth = xcam_video_conf.video_resolution.ch[STREAM0].picwidth;
	  fs0_attr.outFrmRateNum = xcam_video_conf.video_fs_fps.ch[STREAM0].fps_num;
	  fs0_attr.outFrmRateDen = xcam_video_conf.video_fs_fps.ch[STREAM0].fps_den;
	*/

	int enc_mode = IMP_ENC_PROFILE_HEVC_MAIN;
	if (xcam_video_conf.conf_enc.ch0_type == PT_H264) {
		enc_mode = IMP_ENC_PROFILE_AVC_MAIN;
	} else if (xcam_video_conf.conf_enc.ch0_type == PT_H265) {
		enc_mode = IMP_ENC_PROFILE_HEVC_MAIN;
	}

	ret = IMP_FrameSource_CreateChn(STREAM3, &fs3_attr);
	if (ret < XCAM_SUCCESS) {
		IMP_LOG_ERR(TAG, "IMP_FrameSource_CreateChn(chn%d) error !\n", STREAM3);
		return XCAM_ERROR;
	}

	ret = IMP_OSD_CreateGroup(STREAM3);
	if (ret < 0) {
		IMP_LOG_ERR(LOG_TAG, "IMP_Encoder_CreateGroup(%d) error !\n", STREAM1);
		return XCAM_ERROR;
	}

	ret = IMP_Encoder_CreateGroup(ENCGROUP3);
	if (ret < XCAM_SUCCESS) {
		IMP_LOG_ERR(TAG, "IMP_Encoder_CreateGroup(%d) error !\n", STREAM3);
		return XCAM_ERROR;
	}

	IMPEncoderCHNAttr enc3_attr;
	int enable = 0;
	xcam_conf_get_ivdc_status(&enable);
	encoder_init(&enc3_attr, enc_mode, ENC_RC_MODE_CBR, fs3_attr.picWidth, fs3_attr.picHeight, 1, fs3_attr.outFrmRateNum, 0, 4 * 2 * 1024);

	if (ret < XCAM_SUCCESS) {
		IMP_LOG_ERR(TAG, "encoder_init(%d) error !\n", STREAM3);
		return XCAM_ERROR;
	}

	ret = IMP_Encoder_CreateChn(ENCCHN3, &enc3_attr);
	if (ret < XCAM_SUCCESS) {
		IMP_LOG_ERR(TAG, "IMP_Encoder_CreateChn(%d) error !\n", 0);
		return XCAM_ERROR;
	}

	ret = IMP_Encoder_RegisterChn(ENCGROUP3, ENCCHN3);
	if (ret < XCAM_SUCCESS) {
		IMP_LOG_ERR(TAG, "IMP_Encoder_RegisterChn(%d, %d) error: %d\n", 0, 0, ret);
		return XCAM_ERROR;
	}


	ret = IMP_System_Bind(&cell_fs3, &cell_osd3);
	if (ret < XCAM_SUCCESS) {
		IMP_LOG_ERR(TAG, "Bind channel%d FrameSource and OSD failed\n",1);
		return XCAM_ERROR;
	}

	ret = IMP_System_Bind(&cell_osd3, &cell_enc3);
	if (ret < XCAM_SUCCESS) {
		IMP_LOG_ERR(TAG, "Bind channel%d OSD and ENC failed\n", 1);
		return XCAM_ERROR;
	}

	ret = IMP_FrameSource_EnableChn(STREAM3);
	if (ret < XCAM_SUCCESS) {
		LOG_ERR(TAG, "IMP_FrameSource_EnableChn(%d) error: %d\n", ret, 0);
		return XCAM_ERROR;
	}
	/*配置同步*/
	// _xcam_video_syn_encAttr(STREAM0, &enc0_attr);
	// _xcam_video_syn_fsAttr(STREAM0, &fs0_attr);

	return XCAM_SUCCESS;
}
#endif

int xcam_video_enc_FlushStream(int channel)
{
	int ret = XCAM_SUCCESS;
	assert((channel > -1) && (channel < 2));

	ret = IMP_Encoder_FlushStream(channel);
	if (ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG,"error(%s,%d),flush stream fail.\n",__func__,__LINE__);
	}

	return ret;
}

//启动编码器
int xcam_video_enc_start(int channel)
{
	int ret = XCAM_SUCCESS;
	ret = IMP_Encoder_StartRecvPic(channel);
	if (ret != XCAM_SUCCESS) {
		printf("%s,%d: error channel %d, ret = %d\n", __func__, __LINE__, channel, ret);
		return XCAM_ERROR;
	}
	return XCAM_SUCCESS;
}

//停止编码通道
int xcam_video_enc_stop(int channel)
{
	int ret = XCAM_SUCCESS;
	ret = IMP_Encoder_StopRecvPic(channel);
	if (ret != XCAM_SUCCESS) {
		printf("%s,%d: error channel %d, ret = %d\n", __func__, __LINE__, channel, ret);
		return XCAM_ERROR;
	}
	return XCAM_SUCCESS;
}

// #define DEBUG_SAVE_STREAM
#ifdef DEBUG_SAVE_STREAM

#define INIT_SAVE_HANDLER(chanId, filename) \
	struct SaveHandler_t s_h##chanId = { \
	filename,	 \
	chanId,		\
	0,			\
	0			\
	}

#define SH_TO_SNAME(chanId) \
	s_h##chanId

struct SaveHandler_t
{
	const char *filename;
	int channel;
	int fd;
	int init_flag;
};
#define MAX_SAVE_HANDLER   12

INIT_SAVE_HANDLER(0, "/tmp/stream_0.hevc");

struct SaveHandler_t *_shs[MAX_SAVE_HANDLER] = {
	&SH_TO_SNAME(0),
	NULL
};

static void save_file_name(struct SaveHandler_t *h, int chanId, IMPEncoderStream* stream)
{
	if(!h)
		return;
	if(!h->init_flag) {
		int fd = open(h->filename, O_RDWR, 777);
		if(fd < 0) {
			printf("error: %s, %s, %d\n", strerror(errno), __FUNCTION__, __LINE__);
			exit(-1);
		}
		h->init_flag = !h->init_flag;
		h->channel = chanId;
		h->fd = fd;
	}
	int i = 0;
	int ret = 0;
	int nr_pack = stream->packCount;

	for (i = 0; i < nr_pack; i++) {
		ret = write(h->fd, (void *)stream->pack[i].virAddr, stream->pack[i].length);
		if (ret != stream->pack[i].length) {
			printf( "stream write failed\n");
		}
	}


}

#endif



//获取图像帧
int xcam_video_get_enc_frame(int channel, IMPEncoderStream* stream)
{
	int ret = XCAM_SUCCESS;

	ret = IMP_Encoder_PollingStream(channel, 1000);
	if (ret < XCAM_SUCCESS) {
		// LOG_ERR(LOG_TAG,"Func:%s,Line:%d,error channel %d, ret = %d\n", __func__, __LINE__, channel, ret);
		return XCAM_ERROR;
	}
	/* Get H264 or H265 Stream */
	ret = IMP_Encoder_GetStream(channel, stream, 1);
	if (ret < XCAM_SUCCESS) {
		// LOG_ERR(LOG_TAG,"Func:%s,Line:%d,error channel %d, ret = %d\n", __func__, __LINE__, channel, ret);
		return XCAM_ERROR;
	}

#ifdef DEBUG_SAVE_STREAM
	if(channel < MAX_SAVE_HANDLER && channel >= 0) {
		save_file_name(_shs[channel], channel, &stream);
	}
#endif

	return XCAM_SUCCESS;
}

int xcam_video_release_enc_frame(int channel, IMPEncoderStream* stream)
{
	IMP_Encoder_ReleaseStream(channel, stream);
	return XCAM_SUCCESS;
}

int xcam_Tuning_SetAwbAttr(int sensor , int *awb_modes){
	IMPISPWBAttr attr;
	memcpy(&attr, awb_modes, sizeof(IMPISPWBAttr));
	int  ret = IMP_ISP_Tuning_SetAwbAttr(sensor, &attr);
	if(ret){
		IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_GetAwbAttr error !\n");
		return XCAM_ERROR;
	}
	return XCAM_SUCCESS;
}

int xcam_Tuning_GetAwbAttr(int sensor , int *awb_modes){
	IMPISPWBAttr attr;
	memset(&attr, 0, sizeof(IMPISPWBAttr));
	int  ret = IMP_ISP_Tuning_GetAwbAttr(sensor, &attr);
	if(ret){
		IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_GetAwbAttr error !\n");
		return XCAM_ERROR;
	}
	memcpy(awb_modes, &attr, sizeof(IMPISPWBAttr));
	return XCAM_SUCCESS;
}

unsigned int xcam_Tuing_GetAwbOnlyReadAttr(int sensor, int *awb_modes){
	IMPISPAwbOnlyReadAttr attr;
	if(IMP_ISP_Tuning_GetAwbOnlyReadAttr(sensor, &attr)){
		IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_GetAwbOnlyReadAttr error !\n");
		return XCAM_ERROR;
	}
	memcpy(awb_modes, &attr, sizeof(IMPISPAwbOnlyReadAttr));
	return XCAM_SUCCESS;
}

int xcam_Tuning_GetAWBGlobalStatisInfo(int sensor , int *awb_modes){
	IMPISPAWBGlobalStatisInfo  awb_statis;
	memset(&awb_statis, 0, sizeof(IMPISPAWBGlobalStatisInfo));
	int  ret =  IMP_ISP_Tuning_GetAwbGlobalStatistics(sensor, &awb_statis);
	if(ret){
		IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_GetAwbGlobalStatistics error !\n");
		return XCAM_ERROR;
	}
	memcpy(awb_modes, &awb_statis, sizeof(IMPISPAWBGlobalStatisInfo));
	return XCAM_SUCCESS;
}

int xcam_Tuning_SetAwbCtTrendOffset(int sensor , int *aoffset){
	IMPISPAwbCtTrendOffset offset;
	memcpy(&offset, aoffset, sizeof(IMPISPAwbCtTrendOffset));
	// int  ret = IMP_ISP_Tuning_SetAwbCtTrendOffset(sensor, &offset);
    // if(ret){
    //    IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_SetAwbCtTrendOffset error !\n");
    //    return XCAM_ERROR;
  	// }
	return XCAM_SUCCESS;
}

int xcam_Tuning_GetAwbCtTrendOffset(int sensor , int *aoffset){
	IMPISPAwbCtTrendOffset offset;
	memset(&offset, 0, sizeof(IMPISPAwbCtTrendOffset));
	// int  ret = IMP_ISP_Tuning_GetAwbCtTrendOffset(sensor, &offset);
    // if(ret){
    //    IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_GetAwbCtTrendOffset error !\n");
    //    return XCAM_ERROR;
    // }
    // memcpy(aoffset, &offset, sizeof(IMPISPAwbCtTrendOffset));
	return XCAM_SUCCESS;
}
#if 0
static int drawLineRectShow(IMPRgnHandle handler, int x0, int y0, int x1, int y1, int color, IMPOsdRgnType type, int frame_num)
{
	IMPOSDRgnAttr rAttr;
	if (IMP_OSD_GetRgnAttr(handler, &rAttr) < 0) {
		IMP_LOG_ERR(TAG, "%s(%d):IMP_OSD_GetRgnAttr failed\n", __func__, __LINE__);
		return -1;
	}

	rAttr.type = type;
	rAttr.rect.p0.x = x0;
	rAttr.rect.p0.y = y0;
	rAttr.rect.p1.x = x1;
	rAttr.rect.p1.y = y1;
	rAttr.data.lineRectData.color = color;
	if (IMP_OSD_SetRgnAttr(handler, &rAttr) < 0) {
		IMP_LOG_ERR(TAG, "%s(%d):IMP_OSD_SetRgnAttr failed\n", __func__, __LINE__);
		return -1;
	}

	printf("person location:channel = %d, %d, %d, %d, %d\n", frame_num, x0, y0, x1, y1);
#if 1
	if (0 == frame_num) {
		if (IMP_OSD_ShowRgn(handler, 0, 1) < 0) {
			IMP_LOG_ERR(TAG, "%s(%d): IMP_OSD_ShowRgn failed\n", __func__, __LINE__);
			return -1;
		}
	} else {
		if (IMP_OSD_ShowRgn(handler, 1, 1) < 0) {
			IMP_LOG_ERR(TAG, "%s(%d): IMP_OSD_ShowRgn failed\n", __func__, __LINE__);
			return -1;
		}
	}
#endif
	return 0;
}

static int xcam_ivs_persondet_init(IMPIVSInterface **interface) {
	int ret = 0;
	uint32_t personvehicledet_ver = personvehicledet_get_version_info();
	int sensor_sub_width = SENSOR_RESOLUTION_WIDTH_SLAVE;
	int sensor_sub_height = SENSOR_RESOLUTION_HEIGHT_SLAVE;
	if(personvehicledet_ver != PERSONVEHICLEDET_VERSION_NUM){
		printf("The version numbers of head file and lib do not match, head file version: %08x, lib version: %08x\n", PERSONVEHICLEDET_VERSION_NUM, personvehicledet_ver);
		return -1;
	}
	personvehicledet_param_input_t param;

	memset(&param, 0, sizeof(personvehicledet_param_input_t));
	param.frameInfo.width = SENSOR_RESOLUTION_WIDTH_SLAVE; //640;
	param.frameInfo.height = SENSOR_RESOLUTION_HEIGHT_SLAVE; //360;
	param.ptime = true;//
	param.skip_num = 0;      //skip num
	param.max_personvehicle_box = 20;
	param.sense = 5;//
	param.detdist = 0; //0:640 1:800
	param.switch_track = false;
	param.enable_move = false;
	param.move_sense=2;
	param.move_min_h=20;
	param.move_min_w=20;
	param.move_sldwin_size=4;

	param.open_move_filter = false;
	param.nmem_size=3120*1024;
	param.multiprocess_lock = true;

	param.model_path = "./model_file/model.mgk";
	if(access(param.model_path, 0))
	{
		printf("read ./model_file/model.mgk failed !!!\n");
	}
	param.switch_stop_det = false;
#if 0
	int i ,j;
	for(i=0;i<param.permcnt;i++){
		switch(i){
			case 0:
				{
					param.perms[0].p[0].x = 0;
					param.perms[0].p[0].y = 0;
					param.perms[0].p[1].x = sensor_sub_width/4;
					param.perms[0].p[1].y = 0;
					param.perms[0].p[2].x = sensor_sub_width/2;
					param.perms[0].p[2].y = 0;

					param.perms[0].p[3].x = sensor_sub_width/2;
					param.perms[0].p[3].y = sensor_sub_height-1;
					param.perms[0].p[4].x = sensor_sub_width/4;
					param.perms[0].p[4].y = sensor_sub_height-1;
					param.perms[0].p[5].x = 0;
					param.perms[0].p[5].y = sensor_sub_height-1;
				}
				break;
			case 1:
				{
					param.perms[0].p[0].x = 0+160;
					param.perms[0].p[0].y = 0;

					param.perms[0].p[1].x = sensor_sub_width/4+160;
					param.perms[0].p[1].y = 0;

					param.perms[0].p[2].x = sensor_sub_width/2+160;
					param.perms[0].p[2].y = 0;

					param.perms[0].p[3].x = sensor_sub_width/2+160;
					param.perms[0].p[3].y = sensor_sub_height-1;

					param.perms[0].p[4].x = sensor_sub_width/4+160;
					param.perms[0].p[4].y = sensor_sub_height-1;
				}
				break;
		}
	}
#endif
	*interface = PersonvehicleDetInterfaceInit(&param);
	if (*interface == NULL) {
		IMP_LOG_ERR(TAG, "PersonvehicleDetInterfaceInit failed\n");
		return -1;
	}

	return 0;
}

void *xcam_ivs_persondet_thread(void *args)
{
	IMPIVSInterface *interface = (IMPIVSInterface *)args;
	int ret = 0, i = 0, j = 0, x0 = 0, y0 = 0, x1 = 0, y1 = 0;
	unsigned char * g_sub_nv12_buf = (unsigned char *)malloc(SENSOR_RESOLUTION_WIDTH_SLAVE * SENSOR_RESOLUTION_HEIGHT_SLAVE * 3 / 2);
	if(g_sub_nv12_buf == NULL) {
		IMP_LOG_ERR(TAG, "malloc sub nv12 buffer failed(%d)\n", __LINE__);
		return NULL;
	}

	ret = IMP_FrameSource_SetFrameDepth(CH1_INDEX, 0);
	if(ret < 0)
	{
		printf("set frame depth is failed\n");
	}
	IMPFrameInfo sframe;
	personvehicledet_param_output_t *result = NULL;
	while(1) {
		ret = IMP_FrameSource_SnapFrame(CH1_INDEX, PIX_FMT_NV12, SENSOR_RESOLUTION_WIDTH_SLAVE,  SENSOR_RESOLUTION_HEIGHT_SLAVE, g_sub_nv12_buf, &sframe);
		if (ret < 0) {
			printf("%d get frame failed try again\n", 1);
			usleep(30*1000);
			continue;
		}

		sframe.virAddr = (unsigned int)g_sub_nv12_buf;
		ret = interface->preProcessSync(interface, &sframe);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "preProcessSync_err\n");
			return NULL;
		}

		ret = interface->processAsync(interface, &sframe);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "processAsync_err\n");
			return NULL;
		}

		ret = interface->getResult(interface, (void **)&result);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "getResult_err\n");
			return NULL;
		}

		personvehicledet_param_output_t* r = (personvehicledet_param_output_t*)result;
		// for(i = 0; i < r->count; i++) {
		// 	IVSRect* rect = &r->personvehicle[i].show_box;
		// 	//printf("person location:%d, %d, %d, %d\n", rect->ul.x, rect->ul.y, rect->br.x, rect->br.y);
		// }

		memcpy(&main_result, result, sizeof(personvehicledet_param_output_t));

		if (interface->releaseResult && ((ret = interface->releaseResult(interface, (void *)result)) < 0)) {
			IMP_LOG_ERR(TAG, "interface->releaseResult failed ret=%d\n", ret);
			return -1;
		}

		// float scalex = (float)SENSOR_RESOLUTION_WIDTH_MAIN / SENSOR_RESOLUTION_WIDTH_SLAVE;
		// float scaley = (float)SENSOR_RESOLUTION_HEIGHT_MAIN / SENSOR_RESOLUTION_HEIGHT_SLAVE;
		// //draw rect
		// static int draw_save = 0;
		// IMPRgnHandle handler;
		// for(i = 0; i < main_result.count; i++) {
		// 	IVSRect* rect = &main_result.personvehicle[i].show_box;
		// 	//printf("person location:%d, %d, %d, %d\n", rect->ul.x, rect->ul.y, rect->br.x, rect->br.y);
		// 	for(j = 0; j < 2; j++) {
		// 		if (0 == j) {
		// 			handler = ivsRgnHandler[i];
		// 			x0 = rect->ul.x * scalex;
		// 			y0 = rect->ul.y * scaley;
		// 			x1 = rect->br.x * scalex;
		// 			y1 = rect->br.y * scaley;
		// 		} else {
		// 			handler = ivsRgnHandler_second[i];
		// 			x0 = rect->ul.x;
		// 			y0 = rect->ul.y;
		// 			x1 = rect->br.x;
		// 			y1 = rect->br.y;
		// 		}
		// 		//drawLineRectShow(handler, x0, y0, x1, y1, OSD_GREEN, OSD_REG_RECT, j);
		// 	}
		// }
#if 0
	if(main_result.count > 0) {
			for (i = 0; i < main_result.count; i++) {
				IMP_OSD_ShowRgn(ivsRgnHandler[i], 0, 1);
				IMP_OSD_ShowRgn(ivsRgnHandler_second[i], 1, 1);
			}

			for (i = main_result.count; i < 10; i++) {
				IMP_OSD_ShowRgn(ivsRgnHandler[i], 0, 0);
				IMP_OSD_ShowRgn(ivsRgnHandler_second[i], 1, 0);
			}
			draw_save = 0;
		} else {
			draw_save++;
			if(draw_save == 3) {
				for (i = 0; i < 10; i++) {
					IMP_OSD_ShowRgn(ivsRgnHandler[i], 0, 0);
					IMP_OSD_ShowRgn(ivsRgnHandler_second[i], 1, 0);
				}
			}
		}
#endif
	}
	ret = IMP_FrameSource_SetFrameDepth(1, 0);
	PersonDetInterfaceExit(interface);
	free(g_sub_nv12_buf);
	return ;
}

int xcam_personDet_ivs(void)
{
	int ret = 0;
	IMPIVSInterface *interface = NULL;
	ret = xcam_ivs_persondet_init(&interface);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "xcam_ivs_persondet_start(0, 0) failed\n");
		return -1;
	}

	ret = interface->init(interface);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "interface_init_err(%d)\n", __LINE__);
		return -1;
	}
#if 1
	pthread_t persondet_t;
	pthread_attr_t attr;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);  // 设置分离属性
	ret = pthread_create(&persondet_t, NULL, xcam_ivs_persondet_thread, (void *)interface);
	if(ret < 0) {
		IMP_LOG_ERR(TAG, "create persondet thread failed\n");
		return -1;
	}
#endif
	return ret;
}
#endif

//系统初始化和图像流通道初始化
int xcam_video_init(void)
{
	// int ret = XCAM_ERROR for warning

	/*
	ret = get_sensor_params();
	if (ret != XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG,"Get sensor params error.\n");
	} */

	xcam_conf_network_init();
	xcam_conf_video_init();
	_system_init();

	_stream0_init();
	_stream2_init();
	_stream1_init();

#ifdef XCAM_DOUBLE_SENSOR
	_stream3_init();
#endif
	// xcam_extra_main();
	return XCAM_SUCCESS;
}
