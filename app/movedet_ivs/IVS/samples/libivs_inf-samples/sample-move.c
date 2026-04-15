#include <string.h>
#include <unistd.h>
#include <imp/imp_log.h>
#include <imp/imp_common.h>
#include <imp/imp_system.h>
#include <imp/imp_framesource.h>
#include <ivs/ivs_common.h>
#include <ivs/ivs_interface.h>
#include <ivs/ivs_inf_move.h>
#include <imp/imp_ivs.h>

#include "sample-common.h"

#define TAG "SAMPLE-MOVE"

#define COLUMN (22)
#define ROW (18)



int sample_ivs_move_init(int grp_num) {
    int ret = 0;

    ret = IMP_IVS_CreateGroup(grp_num);
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "IMP_IVS_CreateGroup(%d) failed\n", grp_num);
        return -1;
    }

    return 0;
}

int sample_ivs_move_exit(int grp_num) {
    int ret = 0;

    ret = IMP_IVS_DestroyGroup(grp_num);
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "IMP_IVS_DestroyGroup(%d) failed\n", grp_num);
        return -1;
    }
    return 0;
}

int sample_ivs_move_start(int grp_num, int chn_num, IMPIVSInterface **interface) {

    //check ivs version
    //MOVE_VERSION_NUM defined in ivs_inf_move.h.
    uint32_t move_ver = move_get_version_info();
    if(move_ver != MOVE_VERSION_NUM){
        printf("The version numbers of head file and lib do not match, head file version: %08x, lib version: %08x\n", MOVE_VERSION_NUM, move_ver);
        return -1;
    }
    //check ivs version
    int ret = 0;
    move_param_input_t param;

    memset(&param, 0, sizeof(move_param_input_t));
    param.sense = 2;
    param.frameInfo.width = sensor_sub_width;
    param.frameInfo.height = sensor_sub_height;
    /* param.min_h = 40; */
    /* param.min_w = 40; */

    param.level=0.7;
    param.timeon = 110;
    param.timeoff = 0;
    param.light = 0;
    param.isSkipFrame= 0;
    /* param.det_h = 90; */
    /* param.det_w = 160; */
    param.det_h = 36;
    param.det_w = 64;

    param.permcnt = 1;
    param.perms[0].fun = 0;
    /* param.perms[0].fun = 1; */
    param.perms[0].pcnt=4;

    param.perms[0].p = (int *)malloc(12* sizeof(int));
#if 0    
    param.perms[0].p[0].x=0;
    param.perms[0].p[0].y=0;
    param.perms[0].p[1].x=640-1;
    param.perms[0].p[1].y=0;
    param.perms[0].p[2].x=640-1;
    param.perms[0].p[2].y=360-1;
    param.perms[0].p[3].x=0;
    param.perms[0].p[3].y=360-1;
#else
    //param.cntRoi = 1;
    param.perms[0].p[0].x=0;
    param.perms[0].p[0].y=0;
    param.perms[0].p[1].x=1221;
    param.perms[0].p[1].y=0;
    param.perms[0].p[2].x=1221;
    param.perms[0].p[2].y=680;
    param.perms[0].p[3].x=0;
    param.perms[0].p[3].y=680;
#endif    
    *interface = MoveInterfaceInit(&param);
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

int sample_ivs_move_stop(int chn_num, IMPIVSInterface *interface) {
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

    MoveInterfaceExit(interface);

    return 0;
}

int main(int argc, char *argv[]) {

    printf("move\n");

    int ret = 0;
    IMPIVSInterface *inteface = NULL;
    move_param_output_t *result = NULL;

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
    ret = sample_ivs_move_init(0);
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "sample_ivs_init(0) failed\n");
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

    /* Step.6 Start to ivs */
    ret = sample_ivs_move_start(0, 2, &inteface);
        if (ret < 0) {
        IMP_LOG_ERR(TAG, "sample_ivs_start(0, 0) failed\n");
            return -1;
        }

    /*Step.7 Get result*/
    int i = 0;
    while(1){
        i++;
        ret = IMP_IVS_PollingResult(2, IMP_IVS_DEFAULT_TIMEOUTMS);
        if (ret < 0) {
            IMP_LOG_ERR(TAG, "IMP_IVS_PollingResult(%d, %d) failed\n", 2, IMP_IVS_DEFAULT_TIMEOUTMS);
            return -1;
        }
        ret = IMP_IVS_GetResult(2, (void **)&result);
        if (ret < 0) {
            IMP_LOG_ERR(TAG, "IMP_IVS_GetResult(%d) failed\n", 2);
            return -1;
        }
        //printf("frame[%d], result->ret=%d\n", i, result->ret);

        ret = IMP_IVS_ReleaseResult(2, (void *)result);
        if (ret < 0) {
            IMP_LOG_ERR(TAG, "IMP_IVS_ReleaseResult(%d) failed\n", 2);
            return -1;
        }
        //printf("%d\n",result->rectcnt);
        
        /* int x0, y0, x1, y1,k; */
        /* if(result->rectcnt > 0){ */
        /*     for (k=0; k < result->rectcnt;k++){ */
        /*         x0 = result->rects[k].ul.x; */
        /*         y0 = result->rects[k].ul.y; */
        /*         x1 = result->rects[k].br.x; */
        /*         y1 = result->rects[k].br.y; */
        /*         printf("%d %d %d %d\n",x0,y0,x1,y1); */

        /*     } */
        /* } */
        
        //printf("%d\n",result->detcount);
        int x0,y0,k;
        if(result->detcount>0){
            for (k=0; k < result->detcount;k++){
                x0 = result->g_res.det_rects[k].x;
                y0 = result->g_res.det_rects[k].y;
                //printf("%d %d\n",x0,y0);

            }
        }
    }

    /* Step.8 stop to ivs */
    ret = sample_ivs_move_stop(2, inteface);
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "sample_ivs_stop(0) failed\n");
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
    ret = sample_ivs_move_exit(0);
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
