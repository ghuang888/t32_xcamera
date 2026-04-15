/*
 *         (C) COPYRIGHT Ingenic Limited
 *              ALL RIGHT RESERVED
 *
 * File        : test.c
 * Authors     : klyu
 * Create Time : 2024-03-06 16:02:35 (CST)
 * Description :
 *
 */

#include "gekko.h"
#include "gekko_ai3dnr.h"
#include "type.h"
#include "version.h"
#include <imp/imp_common.h>
#include <imp/imp_encoder.h>
#include <imp/imp_framesource.h>
#include <imp/imp_log.h>
#include <imp/imp_system.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include "sample-common.h"

#define TAG "Sample-Encoder-video"

extern struct chn_conf chn[];
static int byGetFd = 0;

char g_sensor_name[128]={0};
uint8_t g_i2c_addr;
uint32_t g_width;
uint32_t g_height;
uint8_t g_sboot;
char *gekkoIQbin_path;
char *NogekkoIQbin_path;

// ./app sensor_name i2c_addr w h sboot gekkoIQbin NogekkoIQbin

/******************************************** Sensor Attribute Table *********************************************/
/* 		NAME		I2C_ADDR		RESOLUTION		Default_Boot			        							*/
/* 		os04a10		0x36 			2560*1440		2:AISP						*/
/* 		sc200ai		0x30 			1920*1088		3:AISP						*/
/* 		os02h10		0x3c 			1920*1088		1:AISP						*/
/* 		os03a10		0x36 			2560*1440		0:AISP						*/
/******************************************** Sensor Attribute Table *********************************************/

int main(int argc, char **argv){
    int ret = 0;
    int i = 0;

    if (argc <8) {
      printf("./app sensor_name i2c_addr w h sboot gekkoIQbin_path NogekkoIQbin_path\n");
      return -1;
    }
    strcpy(g_sensor_name, argv[1]);
    char *endptr;
    g_i2c_addr = strtol(argv[2]+2,&endptr,16);
    g_width=atoi(argv[3]);
    g_height=atoi(argv[4]);
    g_sboot=atoi(argv[5]);
    gekkoIQbin_path=argv[6];
    NogekkoIQbin_path=argv[7];
    printf("sensor:%s\n",g_sensor_name);
    printf("i2c:0x%02x\n",g_i2c_addr);
    printf("width:%d\n",g_width);
    printf("height:%d\n",g_height);
    printf("sboot:%d\n",g_sboot);
    printf("gekkoIQbin_path:%s\n",gekkoIQbin_path);
    printf("NogekkoIQbin_path:%s\n",NogekkoIQbin_path);


    /* Step.1 System init */
    ret = sample_system_init();
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "IMP_System_Init() failed\n");
        return -1;
    }

    /* Step.2 FrameSource init */
    ret = sample_framesource_init();
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "FrameSource init failed\n");
        return -1;
    }

    /* Step.3 Encoder init */
    for (i = 0; i < FS_CHN_NUM; i++) {
        if (chn[i].enable) {
            ret = IMP_Encoder_CreateGroup(chn[i].index);
            if (ret < 0) {
                IMP_LOG_ERR(TAG, "IMP_Encoder_CreateGroup(%d) error !\n", chn[i].index);
                return -1;
            }
        }
    }

    ret = sample_encoder_init();
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "Encoder init failed\n");
        return -1;
    }

    /* Step.4 Bind */
    for (i = 0; i < FS_CHN_NUM; i++) {
        if (chn[i].enable) {
            ret = IMP_System_Bind(&chn[i].framesource_chn, &chn[i].imp_encoder);
            if (ret < 0) {
                IMP_LOG_ERR(TAG, "Bind FrameSource channel%d and Encoder failed\n", i);
                return -1;
            }
        }
    }

    /* Step.5 Stream On */
    ret = sample_framesource_streamon();
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "ImpStreamOn failed\n");
        return -1;
    }


    /********************************* Enable AISP ***************************************************/
    //获取库的版本
    unsigned int gekko_ver = gekko_get_version();
    if (GEKKO_SDK_VERSION_NUM != gekko_ver) {
        IMP_LOG_ERR(TAG, "gekko 3dnr lib version doesn't match!\n");
        return -1;
    }
    IMP_LOG_INFO(TAG,"GEKKO_SDK_VERSION_NUM:%08x\n",gekko_ver);

    //注意此版本（GEKKO_SDK_VERSION_NUM >0.6）要放到IMP_System_Init之后
    ret = gekko_init(VI_MAIN,46*1024*1024);
    if (0 != ret) {
        IMP_LOG_ERR(TAG, "gekko init failed\n");
        return -1;
    }

    /*ai3dnr init*/
    AI3DNRConfig ai3dnr_cfg;
    ai3dnr_cfg.mode_path = "/mnt/model_file/ai3dnr.bin";//配置自己存放模型的路径，名字可修改
    ai3dnr_cfg.fpn_path = "/mnt/model_file/00efcd.bin";//配置自己存放FPN（标定文件）的路径，名字可修改
    ai3dnr_cfg.fpn_type = FPN_TYPE_FULL;//暂时不用管
    if (g_width*g_height>3000000) {//就目前（2024-03-21）支持的分辨率
      ai3dnr_cfg.net_type = AI3DNR_NET_TF;//使用2MP sensor配置为AI3DNR_NET_TSF；使用4MP sensor配置为AI3DNR_NET_TF
    }else {
      ai3dnr_cfg.net_type = AI3DNR_NET_TSF;//使用2MP sensor配置为AI3DNR_NET_TSF；使用4MP sensor配置为AI3DNR_NET_TF
    }
    ai3dnr_cfg.is_hdr_mode = false;//暂时不用管



    ret = gekko_ai3dnr_init(VI_MAIN, &ai3dnr_cfg);
    if (0 != ret) {
        IMP_LOG_ERR(TAG, "ai3dnr init failed\n");
        return -1;
    }

    ret = gekko_ai3dnr_load_model(VI_MAIN);
    if (0 != ret) {
        IMP_LOG_ERR(TAG, "ai3dnr load model failed\n");
        return -1;
    }
    AI3DNRAttr ai3dnr_attr;
    for (int i=0; i<BV_LUT_ITEM_NUM; i++) {
      ai3dnr_attr.tfs_lut[i]=255;//3D时域滤波强度（3D降噪强度）
      ai3dnr_attr.sfs_lut[i]=512;//3D空域滤波强度（2D降噪强度）
    }
    gekko_ai3dnr_set_attr(VI_MAIN, &ai3dnr_attr);

    {
      //AISP开始处理数据之前，切fps和IQ bin
      IMPISPBinAttr attr;
      attr.enable = IMPISP_TUNING_OPS_MODE_ENABLE;
      // char bin_path[]="/etc/sensor/os04a10-t41@gekko.bin";
      memset(attr.bname,0,strlen(gekkoIQbin_path)+1);
      memcpy(attr.bname, gekkoIQbin_path, strlen(gekkoIQbin_path));
      ret = IMP_ISP_Tuning_SwitchBin(IMPVI_MAIN, &attr);
      if(ret){
          IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_SwitchBin error !\n");
          // return -1;
      }
      //切FPS，最好是先切bin,因为gekko的IQ bin，没有开自动降帧
      IMPISPSensorFps fpsAttr;
      fpsAttr.num = 10;//AISP模式下最高支持10fps ！！！
      fpsAttr.den = 1;
      ret = IMP_ISP_Tuning_SetSensorFPS(IMPVI_MAIN, &fpsAttr);
      if (ret) {
          IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_SetSensorFPS error !\n");
          // return -1;
      }
    }

    //AISP开始处理信息
    ret = gekko_start(VI_MAIN);//切IQ bin与切FPS，跟gekko_start走
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "gekko start failed\n");
        return -1;
    }
    /*********************************End Enable AISP ***************************************************/

    /* Step.6 Get stream */
    if (byGetFd) {
        ret = sample_get_video_stream_byfd();
        if (ret < 0) {
            IMP_LOG_ERR(TAG, "Get video stream byfd failed\n");
            return -1;
        }
    } else {
        ret = sample_get_video_stream();
        if (ret < 0) {
            IMP_LOG_ERR(TAG, "Get video stream failed\n");
            return -1;
        }
    }

    /*********************************Use Bv sample ***************************************************/
    //注意当前BV 概念只在开启AISP后使用；关闭AISP后传统环境使用EV
    {
      BVLut bv_list;
      ret = gekko_get_bv_lut(VI_MAIN,&bv_list);
      if (ret < 0) {
          IMP_LOG_ERR(TAG, "gekko_get_bv_lut failed\n");
          // return -1;
      }
      for (int i=0; i<BV_LUT_ITEM_NUM; i++) {
        IMP_LOG_INFO(TAG,"bv[%d]:%d\n",i,bv_list.bv[i]);
      }
      int real_bv;
      ret = IMP_ISP_Tuning_GetAeBv(IMPVI_MAIN,&real_bv);
      if (ret < 0) {
          IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_GetAeBv failed\n");
          // return -1;
      }
      IMP_LOG_INFO(TAG,"real_bv:%d\n",real_bv);
      //这里按照IQ bin中的BV list中最大值，作为切换AISP的阈值
      if (real_bv>bv_list.bv[BV_LUT_ITEM_NUM-1]) {
        //走关闭AISP流程，此处不写了，参考下面Disable AISP       
      }
    }
    /*********************************End Use Bv sample ***************************************************/


    //开关AISP可以在出流中进行,但请按照本文件中Enable AISP中与Disenable AISP中调用顺序
    /********************************* Disable AISP ***************************************************/
    ret = gekko_stop(VI_MAIN);//对应gekko_start;切IQ bin与切FPS，跟gekko_stop走
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "gekko stop failed\n");
        return -1;
    }
    {
      //AISP停止处理数据之后，切fps和IQ bin
      IMPISPBinAttr attr;
      attr.enable = IMPISP_TUNING_OPS_MODE_ENABLE;
      // char bin_path[]="/etc/sensor/os04a10-t41.bin";
      //!!!注意切回传统ISP的IQ bin,请使用gekko sdk中提供的传统bin
      memset(attr.bname,0,strlen(NogekkoIQbin_path)+1);
      memcpy(attr.bname, NogekkoIQbin_path, strlen(NogekkoIQbin_path));
      ret = IMP_ISP_Tuning_SwitchBin(IMPVI_MAIN, &attr);
      if(ret){
          IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_SwitchBin error !\n");
          // return -1;
      }
      //切FPS,如果报错(EXP LIST MODE,SET FPS FAILED),打开以下两个函数的调用
      // IMPISPAeExpListAttr aeexpattr; 
      // ret = IMP_ISP_Tuning_GetAeExpList(IMPVI_MAIN,&aeexpattr);
      // if(ret < 0) {
      //   IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_GetAeExpList error\n");
      //   return -1;
      // }
      // aeexpattr.mode = IMPISP_TUNING_OPS_MODE_DISABLE;
      // ret = IMP_ISP_Tuning_SetAeExpList(IMPVI_MAIN,&aeexpattr);
      // if(ret < 0) {
      //   IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_SetAeExpList error\n");
      //   return -1;
      // }

      IMPISPSensorFps fpsAttr;
      fpsAttr.num = FIRST_SENSOR_FRAME_RATE_NUM;
      fpsAttr.den = FIRST_SENSOR_FRAME_RATE_DEN;
      ret = IMP_ISP_Tuning_SetSensorFPS(IMPVI_MAIN, &fpsAttr);
      if (ret) {
          IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_SetSensorFPS error !\n");
          // return -1;
      }
    }

    ret = gekko_ai3dnr_disable(VI_MAIN);
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "gekko ai3dnr disable failed\n");
        return -1;
    }

    ret = gekko_ai3dnr_unload_model(VI_MAIN);
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "gekko ai3dnr unload model failed\n");
        return -1;
    }

    ret = gekko_ai3dnr_deinit(VI_MAIN);
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "gekko_ai3dnr_deinit failed\n");
        return -1;
    }
    ret = gekko_deinit(VI_MAIN);
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "gekko_deinit failed\n");
        return -1;
    }
    /*********************************End Disable AISP ***************************************************/
    
    
    /* Exit sequence as follow */
    /* Step.a Stream Off */
    ret = sample_framesource_streamoff();
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "FrameSource StreamOff failed\n");
        return -1;
    }

    /* Step.b UnBind */
    for (i = 0; i < FS_CHN_NUM; i++) {
        if (chn[i].enable) {
            ret = IMP_System_UnBind(&chn[i].framesource_chn, &chn[i].imp_encoder);
            if (ret < 0) {
                IMP_LOG_ERR(TAG, "UnBind FrameSource channel%d and Encoder failed\n", i);
                return -1;
            }
        }
    }
    /* Step.c Encoder exit */
    ret = sample_encoder_exit();
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "Encoder exit failed\n");
        return -1;
    }

    /* Step.d FrameSource exit */
    ret = sample_framesource_exit();
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "FrameSource exit failed\n");
        return -1;
    }

    /* Step.e System exit */
    ret = sample_system_exit();
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "sample_system_exit() failed\n");
        return -1;
    }
 
    return 0;
}
