#include <string.h>
#include <unistd.h>
#include <imp/imp_log.h>
#include <imp/imp_common.h>
#include <imp/imp_system.h>
#include <imp/imp_framesource.h>
#include <ivs/ivs_common.h>
#include <ivs/ivs_interface.h>
#include <ivs/ivs_inf_shade.h>
#include <imp/imp_ivs.h>

#ifdef _DEBUG
#include <stdio.h>
#endif

#include "sample-common.h"

#define TAG "SAMPLE-SHADE"

int sample_ivs_shade_init(int grp_num)
{
    int ret = 0;

    ret = IMP_IVS_CreateGroup(grp_num);
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "IMP_IVS_CreateGroup(%d) failed\n", grp_num);
        return -1;
    }

    return 0;
}


int sample_ivs_shade_exit(int grp_num)
{
    int ret = 0;

    ret = IMP_IVS_DestroyGroup(grp_num);
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "IMP_IVS_DestroyGroup(%d) failed\n", grp_num);
        return -1;
    }
    return 0;
}

int sample_ivs_shade_start(int grp_num, int chn_num, IMPIVSInterface **interface)
{
    int ret = 0;
    shade_param_input_t param;

    memset(&param, 0, sizeof(shade_param_input_t));
    param.frameInfo.width = sensor_sub_width;
    param.frameInfo.height = sensor_sub_height;
    param.night_flag = 0;
    param.night_sense = 0;
    param.start_delay = 40;
    param.update_flag = 0;
    *interface = ShadeInterfaceInit(&param);
    if (*interface == NULL) {
        IMP_LOG_ERR(TAG, "IMP_IVS_CreateGroup(%d) failed\n", grp_num);
        return -1;
    }

    ret = IMP_IVS_CreateChn(chn_num, *interface);
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "IMP_IVS_CreateChn(%d) failed\n", chn_num);
        return -1;
    }

    ret = IMP_IVS_RegisterChn(grp_num, chn_num);
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "IMP_IVS_RegisterChn(%d, %d) failed\n", grp_num, chn_num);
        return -1;
    }

    ret = IMP_IVS_StartRecvPic(chn_num);
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "IMP_IVS_StartRecvPic(%d) failed\n", chn_num);
        return -1;
    }

    return 0;
}

int sample_ivs_shade_stop(int chn_num, IMPIVSInterface *interface)
{
    int ret = 0;

    ret = IMP_IVS_StopRecvPic(chn_num);
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "IMP_IVS_StopRecvPic(%d) failed\n", chn_num);
        return -1;
    }
    sleep(1);

    ret = IMP_IVS_UnRegisterChn(chn_num);
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "IMP_IVS_UnRegisterChn(%d) failed\n", chn_num);
        return -1;
    }

    ret = IMP_IVS_DestroyChn(chn_num);
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "IMP_IVS_DestroyChn(%d) failed\n", chn_num);
        return -1;
    }

    ShadeInterfaceExit(interface);

    return 0;
}
static int NightModeSwitch(int on,int update_flag)
{
    shade_param_input_t param;
    int ret;
    ret = IMP_IVS_GetParam(0,&param);
    if(ret < 0){
        IMP_LOG_ERR(TAG, "IMP_IVS_GetParam(%d) failed\n", IVS_SHADE_DETECT);
        return -1;
    }
    param.night_flag = on;
    param.update_flag = update_flag;
    ret = IMP_IVS_SetParam(0,&param);
    if(ret < 0){
        IMP_LOG_ERR(TAG, "IMP_IVS_SetParam(%d) failed\n", IVS_SHADE_DETECT);
        return -1;
    }
    return 1;
}

int main(int argc, char *argv[])
{
    int ret = 0, i = 0;
    IMPIVSInterface *inteface = NULL;
    shade_param_output_t *result = NULL;
    int on; /*day:on = 0,night:on = 1,dim:on = 2 */
    int update_flag=0; /* update_flag = 1:reset background */
    /* Step.1 System init */
    ret = sample_system_init();
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "IMP_System_Init() failed\n");
        return -1;
    }

    /* Step.2 FrameSource init */
    IMPFSChnAttr fs_chn_attr;
    memset(&fs_chn_attr, 0, sizeof(IMPFSChnAttr));
    fs_chn_attr.pixFmt = PIX_FMT_NV12;
    fs_chn_attr.outFrmRateNum = SENSOR_FRAME_RATE;
    fs_chn_attr.outFrmRateDen = 1;
    fs_chn_attr.nrVBs = 3;
    fs_chn_attr.type = FS_PHY_CHANNEL;

    fs_chn_attr.crop.enable = 0;
    fs_chn_attr.crop.top = 0;
    fs_chn_attr.crop.left = 0;
    fs_chn_attr.crop.width = sensor_main_width;
    fs_chn_attr.crop.height = sensor_main_height;

    fs_chn_attr.scaler.enable = 1;    /* ivs use the second framesource channel, need scale*/
    fs_chn_attr.scaler.outwidth = sensor_sub_width;
    fs_chn_attr.scaler.outheight = sensor_sub_height;

    fs_chn_attr.picWidth = sensor_sub_width;
    fs_chn_attr.picHeight = sensor_sub_height;

    ret = sample_framesource_init(FS_SUB_CHN, &fs_chn_attr);
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "FrameSource init failed\n");
        return -1;
    }

    /* Step.3 Encoder init */
    ret = sample_ivs_shade_init(0);
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "sample_ivs_shade_init(0) failed\n");
        return -1;
    }

    /* Step.4 Bind */
    IMPCell framesource_cell = {DEV_ID_FS, FS_SUB_CHN, 0};
    IMPCell ivs_cell = {DEV_ID_IVS, 0, 0};

    ret = IMP_System_Bind(&framesource_cell, &ivs_cell);
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "Bind FrameSource channel%d and ivs0 failed\n", FS_SUB_CHN);
        return -1;
    }

    /* Step.5 Stream On */
    IMP_FrameSource_SetFrameDepth(FS_SUB_CHN, 0);
    ret = sample_framesource_streamon(FS_SUB_CHN);
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "ImpStreamOn failed\n");
        return -1;
    }

    ret = sample_ivs_shade_start(0, 2, &inteface);
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "sample_ivs_shade_start(0, 0) failed\n");
        return -1;
    }

    /* open a file to save debug information */
#ifdef _DEBUG
    FILE *handle = fopen("./debugInfo.txt","wa");
    char debugBuff[64];
#endif
    /* Step.6 Get result */
    for (i = 0;/* i < NR_FRAMES_TO_IVS*/; i++) {
        //ret = NightModeSwitch(on,update_flag);
        /* After updating background,need to reset the update_flag*/
        //update_flag = 1;
        //if(ret < 0){
        //    IMP_LOG_ERR(TAG,"NightModeSwitch failed\n");
        //    return -1;
        //}
        //fprintf(stderr,"++++++++++++++++++++++++++++++++\n");
        ret = IMP_IVS_PollingResult(2, IMP_IVS_DEFAULT_TIMEOUTMS);
        if (ret < 0) {
            IMP_LOG_ERR(TAG, "IMP_IVS_PollingResult(%d, %d) failed\n", 0, IMP_IVS_DEFAULT_TIMEOUTMS);
            return -1;
        }
        ret = IMP_IVS_GetResult(2, (void **)&result);
        if (ret < 0) {
            IMP_LOG_ERR(TAG, "IMP_IVS_GetResult(%d) failed\n", 0);
            return -1;
        }
        IMP_LOG_INFO(TAG, "frame[%d], result->flag=%d\n", i, result->flag);
        if(result->shade_enable == 0)
            IMP_LOG_INFO(TAG,"Waiting Update------\n");
        /* write debug information to a file */
#ifdef _DEBUG
        memset(debugBuff,0,64);
        sprintf(debugBuff,"%lld,%f,%f,%d\n",
                result->timeStamp,result->debugInfo.diff_frame,
                result->debugInfo.diff_background,result->flag);
        fwrite(debugBuff,sizeof(char),strlen(debugBuff),handle);
#endif
        ret = IMP_IVS_ReleaseResult(2, (void *)result);
        if (ret < 0) {
            IMP_LOG_ERR(TAG, "IMP_IVS_ReleaseResult(%d) failed\n", 0);
            return -1;
        }
    }
#ifdef _DEBUG
    fclose(handle);
#endif
    fprintf(stderr,"PollingResult end\n");

    ret = sample_ivs_shade_stop(2, inteface);
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "sample_ivs_shade_stop(0) failed\n");
        return -1;
    }

    /* Step.9 Stream Off */
    ret = sample_framesource_streamoff(FS_SUB_CHN);
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "FrameSource StreamOff failed\n");
        return -1;
    }

    /* Step.10 UnBind */
    ret = IMP_System_UnBind(&framesource_cell, &ivs_cell);
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "UnBind FrameSource channel%d and ivs0 failed\n", FS_SUB_CHN);
        return -1;
    }

    /* Step.11 ivs exit */
    ret = sample_ivs_shade_exit(0);
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "ivs mode exit failed\n");
        return -1;
    }

    /* Step.12 FrameSource exit */
    ret = sample_framesource_exit(FS_SUB_CHN);
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "FrameSource(%d) exit failed\n", FS_SUB_CHN);
        return -1;
    }

    /* Step.13 System exit */
    ret = sample_system_exit();
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "sample_system_exit() failed\n");
        return -1;
    }

    return 0;
}
