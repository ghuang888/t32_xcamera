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

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */


/********************************** Sensor Attribute Table ***********************************/
/*         NAME        I2C_ADDR        RESOLUTION        Default_Boot                            */
/*         gc2053        0x37             1920*1080        0:25fps_dvp 1:15fps_dvp    2:25fps_mipi    */
/*         jxf37        0x40             1920*1080        0:25fps_dvp 1:25fps_mipi                */
/*         imx327        0x36             1920*1080        0:25fps 1:25fps_2dol                    */
/*         imx335        0x1a             2592*1944        0:15fps 1:25fps                            */
/*         imx415        0x1a             3840*2160        0:15fps                                    */
/*         os08a10        0x36             3840*2160        0:15fps    1:25fps                            */
/*         sc3235        0x30             2304*1296        0:25fps                                    */
/*         sc8238        0x30             3840*2160        0:15fps 1:25fps                            */
/********************************** Sensor Attribute Table ***********************************/
#define IMPISP_SENSOR_VI_MIPI_CSI0 0
#define IMPISP_SENSOR_VI_MIPI_CSI1 1
#define IMPISP_SENSOR_MCLK1 1
#define IMPISP_SENSOR_MCLK2 2
#define IMPVI_MAIN 0

#define GPIO_PA(n) (0 * 32 + (n))
#define GPIO_PB(n) (1 * 32 + (n))
#define GPIO_PC(n) (2 * 32 + (n))
#define GPIO_PD(n) (3 * 32 + (n))

/* first sensor */
// #define FIRST_SNESOR_NAME            "sc500ai"            //sensor name (match with snesor driver name)
// #define FIRST_I2C_ADDR                0x30            //sensor i2c address
// #define    FIRST_I2C_ADAPTER_ID        0 //1                //sensor controller number used (0/1/2/3)
// #define FIRST_SENSOR_WIDTH            2880 //1920            //sensor width
// #define FIRST_SENSOR_HEIGHT            1620 //1080            //sensor height
// #define FIRST_RST_GPIO                GPIO_PA(18) //GPIO_PC(27)        //sensor reset gpio
// #define    FIRST_PWDN_GPIO            GPIO_PA(19)//-1                //sensor pwdn gpio
// #define    FIRST_POWER_GPIO            -1                //sensor power gpio
// #define FIRST_SENSOR_ID                0                //sensor index
// #define    FIRST_VIDEO_INTERFACE        IMPISP_SENSOR_VI_MIPI_CSI0        //sensor interface type (dvp/csi0/csi1)
// #define    FIRST_MCLK                   IMPISP_SENSOR_MCLK0                //sensor clk source (mclk0/mclk1/mclk2)
// #define FIRST_DEFAULT_BOOT            0 //2                //sensor default mode(0/1/2/3/4)

#define FIRST_SNESOR_NAME            "gc4663"//"gc2053"            //sensor name (match with snesor driver name)
#define FIRST_I2C_ADDR                0x29//0x37            //sensor i2c address
#define FIRST_I2C_ADAPTER_ID          0                //sensor controller number used (0/1/2/3)
#define FIRST_SENSOR_WIDTH            2560            //sensor width
#define FIRST_SENSOR_HEIGHT           1440            //sensor height
#define FIRST_RST_GPIO                GPIO_PC(27)        //sensor reset gpio
#define FIRST_PWDN_GPIO               -1                //sensor pwdn gpio
#define FIRST_POWER_GPIO              -1                //sensor power gpio
#define FIRST_SENSOR_ID                0                //sensor index
#define FIRST_VIDEO_INTERFACE         IMPISP_SENSOR_VI_MIPI_CSI0        //sensor interface type (dvp/csi0/csi1)
#define FIRST_MCLK                    IMPISP_SENSOR_MCLK1                //sensor clk source (mclk0/mclk1/mclk2)
#define FIRST_DEFAULT_BOOT            0                //sensor default mode(0/1/2/3/4)



#define SENSOR_FRAME_RATE        25
#define NR_FRAMES_TO_IVS        2000
#define FS_MAIN_CHN             0
#define FS_SUB_CHN                1
#define FS_CHN_NUM                7  //MIN 1,MAX 6
#define OSD_REGION_WIDTH        32
#define OSD_REGION_HEIGHT        32

#define NR_FRAMES_TO_SAVE        200
#define IVS_CHN_ID          1

int sensor_main_width;
int sensor_main_height;
int sensor_sub_width;
int sensor_sub_height;

int sample_system_init();
int sample_system_exit();

int sample_framesource_streamon(int chn_num);
int sample_framesource_streamoff(int chn_num);

int sample_framesource_init(int chn_num, IMPFSChnAttr *imp_chn_attr);
int sample_framesource_exit(int chn_num);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __SAMPLE_COMMON_H__ */
