/*
 * sample-common.h
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 */

#ifndef __SAMPLE_COMMON_H__
#define __SAMPLE_COMMON_H__

#include <imp/imp_common.h>
#include <imp/imp_osd.h>
#include <imp/imp_framesource.h>
#include <imp/imp_isp.h>
#include <imp/imp_encoder.h>
#include <unistd.h>

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */

/******************************************** Sensor Attribute Table *********************************************/
/* 		NAME		I2C_ADDR		RESOLUTION		Default_Boot			        							*/
/* 		os04a10		0x36 			2560*1440		2:AISP						*/
/* 		sc200ai		0x30 			1920*1088		3:AISP						*/
/* 		os02h10		0x3c 			1920*1088		1:AISP						*/
/* 		os03a10		0x36 			2560*1440		0:AISP						*/
/******************************************** Sensor Attribute Table *********************************************/

/* first sensor */
#define FIRST_SNESOR_NAME           "os04a10"                        //sensor name (match with snesor driver name)
#define FIRST_I2C_ADDR              0x36                            //sensor i2c address
#define FIRST_I2C_ADAPTER_ID        0                               //sensor controller number used (0/1/2/3)
#define FIRST_SENSOR_WIDTH          2560                            //sensor width
#define FIRST_SENSOR_HEIGHT         1440                            //sensor height
#define FIRST_RST_GPIO              GPIO_PA(18)                     //sensor reset gpio
#define FIRST_PWDN_GPIO             GPIO_PA(19)                     //sensor pwdn gpio
#define FIRST_POWER_GPIO            -1                              //sensor power gpio
#define FIRST_SENSOR_ID             0                               //sensor index
#define FIRST_VIDEO_INTERFACE       IMPISP_SENSOR_VI_MIPI_CSI0      //sensor interface type (dvp/csi0/csi1)
#define FIRST_MCLK                  IMPISP_SENSOR_MCLK0             //sensor clk source (mclk0/mclk1/mclk2)
#define FIRST_DEFAULT_BOOT          2                               //sensor default mode(0/1/2/3/4)

#define CHN0_EN                 1
#define CHN1_EN                 0
#define CHN2_EN                 0

/* Crop_en Choose */
#define FIRST_CROP_EN					0

#define FIRST_SENSOR_FRAME_RATE_NUM			20
#define FIRST_SENSOR_FRAME_RATE_DEN			1

#define FIRST_SENSOR_WIDTH_SECOND			640
#define FIRST_SENSOR_HEIGHT_SECOND			360

#define FIRST_SENSOR_WIDTH_THIRD			1280
#define FIRST_SENSOR_HEIGHT_THIRD			720

#define BITRATE_720P_Kbs        1000
#define NR_FRAMES_TO_SAVE		500
#define STREAM_BUFFER_SIZE		(1 * 1024 * 1024)

#define ENC_VIDEO_CHANNEL		0
#define ENC_JPEG_CHANNEL		1

#define STREAM_FILE_PATH_PREFIX		"/tmp"
#define SNAP_FILE_PATH_PREFIX		"/tmp"

#define OSD_REGION_WIDTH		16
#define OSD_REGION_HEIGHT		34
#define OSD_REGION_WIDTH_SEC	8
#define OSD_REGION_HEIGHT_SEC   18


#define SLEEP_TIME			1

#define FS_CHN_NUM			3
#define IVS_CHN_ID          1

#define CH0_INDEX  	0
#define CH1_INDEX  	1
#define CH2_INDEX  	2

#define CHN_ENABLE 		1
#define CHN_DISABLE 	0

/*#define SUPPORT_RGB555LE*/

struct chn_conf{
	unsigned int index;//0 for main channel ,1 for second channel
	unsigned int enable;
	IMPEncoderProfile payloadType;
	IMPFSChnAttr fs_chn_attr;
	IMPCell framesource_chn;
	IMPCell imp_encoder;
};

typedef struct sample_osd_param{
	int *phandles;
	uint32_t *ptimestamps;
}IMP_Sample_OsdParam;

#define  CHN_NUM  ARRAY_SIZE(chn)

extern char g_sensor_name[128];
extern uint8_t g_i2c_addr;
extern uint32_t g_width;
extern uint32_t g_height;
extern uint8_t g_sboot;

int sample_system_init();
int sample_system_exit();

int sample_framesource_init();
int sample_framesource_exit();

int sample_encoder_init();
int sample_encoder_exit();

int sample_framesource_streamon();
int sample_framesource_streamoff();

int sample_jpeg_init();
int sample_jpeg_exit();

int sample_get_frame();
int sample_get_video_stream();
int sample_get_video_stream_byfd();
int sample_get_h265_jpeg_stream();

int sample_jpeg_ivpu_init();

int sample_get_jpeg_snap(int count);

int64_t sample_gettimeus(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __SAMPLE_COMMON_H__ */
