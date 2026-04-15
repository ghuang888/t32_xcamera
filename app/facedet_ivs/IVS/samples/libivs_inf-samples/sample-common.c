/*
 * sample-common.c
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 */

#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <imp/imp_log.h>
#include <imp/imp_common.h>
#include <imp/imp_system.h>
#include <imp/imp_isp.h>
#include <imp/imp_framesource.h>

#include "sample-common.h"

#define TAG "Sample-Common"

IMPSensorInfo Def_Sensor_Info[1] = {
    {
        FIRST_SNESOR_NAME,
        TX_SENSOR_CONTROL_INTERFACE_I2C,
        {FIRST_SNESOR_NAME, FIRST_I2C_ADDR, FIRST_I2C_ADAPTER_ID},
        FIRST_RST_GPIO,
        FIRST_PWDN_GPIO,
        FIRST_POWER_GPIO,
        FIRST_SENSOR_ID,
        FIRST_VIDEO_INTERFACE,
        FIRST_MCLK,
        FIRST_DEFAULT_BOOT
    },
};

IMPSensorInfo sensor_info[1];
int sensor_main_width = 2880;
int sensor_main_height = 1620;
int sensor_sub_width = 1280;
int sensor_sub_height = 720;

int sample_system_init()
{
    int ret = 0;

    memset(&sensor_info, 0, sizeof(sensor_info));
        memcpy(&sensor_info[0], &Def_Sensor_Info[0], sizeof(IMPSensorInfo));

    ret = IMP_ISP_Open();
    if(ret < 0){
        IMP_LOG_ERR(TAG, "failed to EmuISPOpen\n");
        return -1;
    }

    ret = IMP_ISP_AddSensor(IMPVI_MAIN, &sensor_info);
    if(ret < 0){
        IMP_LOG_ERR(TAG, "failed to AddSensor\n");
        return -1;
    }

    ret = IMP_ISP_EnableSensor(IMPVI_MAIN, &sensor_info);
    if(ret < 0){
        IMP_LOG_ERR(TAG, "failed to EnableSensor\n");
        return -1;
    }

    ret = IMP_System_Init();
    if(ret < 0){
        IMP_LOG_ERR(TAG, "IMP_System_Init failed\n");
        return -1;
    }

    /* enable turning, to debug graphics */
    ret = IMP_ISP_EnableTuning();
    if(ret < 0){
        IMP_LOG_ERR(TAG, "IMP_ISP_EnableTuning failed\n");
        return -1;
    }

    IMP_LOG_DBG(TAG, "ImpSystemInit success\n");

    return 0;
}

int sample_system_exit()
{
    int ret = 0;

    IMP_LOG_DBG(TAG, "sample_system_exit start\n");

    IMP_System_Exit();

    ret = IMP_ISP_DisableSensor(IMPVI_MAIN);
    if(ret < 0){
        IMP_LOG_ERR(TAG, "failed to EnableSensor\n");
        return -1;
    }

    ret = IMP_ISP_DelSensor(IMPVI_MAIN, &sensor_info);
    if(ret < 0){
        IMP_LOG_ERR(TAG, "failed to AddSensor\n");
        return -1;
    }

    if(IMP_ISP_Close()){
        IMP_LOG_ERR(TAG, "failed to EmuISPOpen\n");
        return -1;
    }

    IMP_LOG_DBG(TAG, " sample_system_exit success\n");

    return 0;
}

int sample_framesource_streamon(int chn_num)
{
    int ret = 0;
    /* Enable channels */
    ret = IMP_FrameSource_EnableChn(chn_num);
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "IMP_FrameSource_EnableChn(%d) error: %d\n", ret, chn_num);
        return -1;
    }

    return 0;
}

int sample_framesource_streamoff(int chn_num)
{
    int ret = 0;
    /* Enable channels */
    ret = IMP_FrameSource_DisableChn(chn_num);
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "IMP_FrameSource_DisableChn(%d) error: %d\n", ret, chn_num);
        return -1;
}

    return 0;
}

int sample_framesource_init(int chn_num, IMPFSChnAttr *imp_chn_attr)
{
    int ret = 0;

    /*create channel chn_num*/
    ret = IMP_FrameSource_CreateChn(chn_num, imp_chn_attr);
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "IMP_FrameSource_CreateChn(chn%d) error!\n", chn_num);
        return -1;
}

    ret = IMP_FrameSource_SetChnAttr(chn_num, imp_chn_attr);
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "IMP_FrameSource_SetChnAttr(%d) error: %d\n", ret, chn_num);
                return -1;
    }

    /* Check channel chn_num attr */
    IMPFSChnAttr imp_chn_attr_check;
    ret = IMP_FrameSource_GetChnAttr(chn_num, &imp_chn_attr_check);
            if(ret < 0){
        IMP_LOG_ERR(TAG, "IMP_FrameSource_GetChnAttr(%d) error: %d\n", ret, chn_num);
                return -1;
            }

    IMP_LOG_DBG(TAG, "framesource_Init success\n");

    return 0;
}

int sample_framesource_exit(int chn_num)
{
    int ret = 0;

    /*Destroy channel i*/
    ret = IMP_FrameSource_DestroyChn(chn_num);
    if(ret < 0){
        IMP_LOG_ERR(TAG, "IMP_FrameSource_DestroyChn(%d) error: %d\n", chn_num, ret);
        return -1;
    }
    return 0;
}

