/*
 * xcam_video_t41.h
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
/*      NAME        I2C_ADDR        RESOLUTION      Default_Boot                                                */
/*      jxf23       0x40            1920*1080       0:25fps_dvp 1:15fps_dvp 2:25fps_mipi                        */
/*      jxf37       0x40            1920*1080       0:25fps_dvp 1:25fps_mipi 2: 25fps_mipi                      */
/*      imx327      0x1a            1920*1080       0:25fps 1:25fps_2dol                                        */
/*      sc430ai     0x30            2688*1520       0:20fps_mipi 1:30fps_mipi 2:25fps_mipi                      */
/*      sc500ai     0x30            2880*1620       0:30fps_mipi                                                */
/*      sc5235      0x30            2592*1944       0:5fps_mipi                                                 */
/*      gc4663      0x29            2560*1440       0:25fps_mipi 1:30fps_mipi                                   */
/*      sc8238      0x30            3840*2160       0:15fps 1:30fps                                             */
/******************************************** Sensor Attribute Table *********************************************/

/* first sensor */
// #ifdef GC4663
#define FIRST_SNESOR_NAME           "gc5613"                   //sensor name (match with snesor driver name)
// #elif defined(GC2063)
// #define FIRST_SNESOR_NAME           "gc2063" 
// #elif defined(IMX415)
// #define FIRST_SNESOR_NAME           "imx415" 
// #endif

#define FIRST_I2C_ADDR              0x31                            //sensor i2c address
#define FIRST_I2C_ADAPTER_ID        0                               //sensor controller number used (0/1/2/3)
#define FIRST_SENSOR_WIDTH          2880                            //sensor width
#define FIRST_SENSOR_HEIGHT         1620                            //sensor height
#define FIRST_RST_GPIO              GPIO_PA(20)                     //sensor reset gpio
#define FIRST_PWDN_GPIO             -1                     //sensor pwdn gpio
#define FIRST_POWER_GPIO            -1                              //sensor power gpio
#define FIRST_SENSOR_ID             0                               //sensor index
#define FIRST_VIDEO_INTERFACE       IMPISP_SENSOR_VI_MIPI_CSI0      //sensor interface type (dvp/csi0/csi1)
#define FIRST_MCLK                  IMPISP_SENSOR_MCLK0             //sensor clk source (mclk0/mclk1/mclk2)
#define FIRST_DEFAULT_BOOT          0                               //sensor default mode(0/1/2/3/4)

/* second sensor */
#define SECOND_SNESOR_NAME          "gc5613s1"          //sensor name (match with snesor driver name)
#define SECOND_I2C_ADDR             0x31                //sensor i2c address
#define SECOND_I2C_ADAPTER_ID       1                   //sensor controller number used (0/1/2/3)
#define SECOND_SENSOR_WIDTH         2880                //sensor width
#define SECOND_SENSOR_HEIGHT        1620                //sensor height
#define SECOND_RST_GPIO             -1         //sensor reset gpio
#define SECOND_PWDN_GPIO            -1                  //sensor pwdn gpio
#define SECOND_POWER_GPIO           -1                  //sensor power gpio
#define SECOND_SENSOR_ID            1                   //sensor index
#define SECOND_VIDEO_INTERFACE      IMPISP_SENSOR_VI_MIPI_CSI1  //sensor interface type (dvp/csi0/csi1)
#define SECOND_MCLK                 IMPISP_SENSOR_MCLK1         //sensor clk source (mclk0/mclk1/mclk2)
#define SECOND_DEFAULT_BOOT         0                           //sensor default mode(0/1/2/3/4)


#define CHN0_EN                	1
#define CHN1_EN                	1
#define CHN2_EN                 0
#define CHN3_EN                	1
#define CHN4_EN                	0
#define CHN5_EN                 0
/* Crop_en Choose */
#define FIRST_CROP_EN					0
#define SECOND_CROP_EN					0
#define FIRST_SENSOR_FRAME_RATE_NUM			15
#define FIRST_SENSOR_FRAME_RATE_DEN			1

#define SECOND_SENSOR_FRAME_RATE_NUM        15 
#define SECOND_SENSOR_FRAME_RATE_DEN		1
#define FIRST_SENSOR_WIDTH_SECOND			704
#define FIRST_SENSOR_HEIGHT_SECOND			576

#define FIRST_SENSOR_WIDTH_THIRD			640
#define FIRST_SENSOR_HEIGHT_THIRD			384

#define ENC_VIDEO_CHANNEL		0
#define ENC_JPEG_CHANNEL		1

#define SLEEP_TIME			1

#define FS_CHN_NUM			6
#define IVS_CHN_ID          1

#define CH0_INDEX  0
#define CH1_INDEX  1
#define CH2_INDEX  2
#define CH3_INDEX  3
#define CHN_DISABLE 0

/*#define SUPPORT_RGB555LE*/

struct chn_conf{
	unsigned int index;//0 for main channel ,1 for second channel
	unsigned int enable;
	IMPEncoderProfile payloadType;
	IMPFSChnAttr fs_chn_attr;
	IMPCell framesource_chn;
	IMPCell imp_encoder;
};

#define  CHN_NUM  ARRAY_SIZE(chn)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __XCAM_VIDEO_T41__H__ */
