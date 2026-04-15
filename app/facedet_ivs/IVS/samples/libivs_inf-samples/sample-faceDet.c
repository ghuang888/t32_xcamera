#include <string.h>
#include <unistd.h>
#include <imp/imp_log.h>
#include <imp/imp_common.h>
#include <imp/imp_system.h>
#include <imp/imp_framesource.h>
#include <ivs/ivs_common.h>
#include <ivs/ivs_interface.h>
#include <ivs/ivs_inf_faceDet.h>
#include <imp/imp_ivs.h>

#include "sample-common.h"

#define TAG "SAMPLE-FACEDET"

int sample_ivs_facedet_init(int grp_num) {
    int ret = 0;

    ret = IMP_IVS_CreateGroup(grp_num);
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "IMP_IVS_CreateGroup(%d) failed\n", grp_num);
        return -1;
    }

    return 0;
}

int sample_ivs_facedet_exit(int grp_num) {
    int ret = 0;
    ret = IMP_IVS_DestroyGroup(grp_num);
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "IMP_IVS_DestroyGroup(%d) failed\n", grp_num);
        return -1;
    }
    return 0;
}

int sample_ivs_facedet_start(int grp_num, int chn_num, IMPIVSInterface **interface) {
    //check ivs version
    //FACEDET_VERSION_NUM defined in ivs_inf_faceDet.h .
    uint32_t facedet_ver = facedet_get_version_info();
    if(facedet_ver != FACEDET_VERSION_NUM){
        printf("The version numbers of head file and lib do not match, head file version: %08x, lib version: %08x\n", FACEDET_VERSION_NUM, facedet_ver);
        return -1;
    }
    //check ivs version
    int ret = 0;
    facedet_param_input_t param;

    memset(&param, 0, sizeof(facedet_param_input_t));
    param.frameInfo.width = sensor_sub_width; //recommend 720P
    param.frameInfo.height = sensor_sub_height;

    param.model_path = "./model/face_det.bin";
    param.skip_num = 0;
    param.max_face_box = 10;
    param.sense = 2;
    param.detdist = 1;
    param.switch_track = true;
    param.ptime = false;
    param.rot90 = false; // reserved
    param.switch_liveness = false; // reserved
    param.switch_face_pose = false; // reserved
    param.switch_face_blur = false; // reserved
    param.nmem_size = (int)(8.5*1024*1024);//8.5*1024*1024;
    param.share_mem_mode = 4;

    *interface = FaceDetInterfaceInit(&param);
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

int sample_ivs_facedet_stop(int chn_num, IMPIVSInterface *interface) {
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

    FaceDetInterfaceExit(interface);

    return 0;
}

int main(int argc, char *argv[]) {
    printf("face detect\n");
    int ret = 0;
    IMPIVSInterface *inteface = NULL;
    facedet_param_output_t *result = NULL;

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
    /* fs_chn_attr.pixFmt = PIX_FMT_BGR24; */
    fs_chn_attr.outFrmRateNum = SENSOR_FRAME_RATE;
    fs_chn_attr.outFrmRateDen = 1;
    /* fs_chn_attr.nrVBs = 2; */
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
    ret = sample_ivs_facedet_init(0);
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
    ret = sample_ivs_facedet_start(0, 2, &inteface);
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "sample_ivs_facedet_start(0, 2) failed\n");
        return -1;
    }

    /*Step.7 Get result*/
    while(true){
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

        facedet_param_output_t* r = (facedet_param_output_t*)result;
        int i = 0;
        for(i = 0; i < r->count; i++) {
            IVSRect* show_rect = &r->face[i].show_box;
            if(r->face[i].class_id == 0){
                printf("face location:%d, %d, %d, %d\n", show_rect->ul.x, show_rect->ul.y, show_rect->br.x, show_rect->br.y);
                printf("face confidence:%f\n", r->face[i].confidence);
            }
        }
        ret = IMP_IVS_ReleaseResult(2, (void *)result);
        if (ret < 0) {
            IMP_LOG_ERR(TAG, "IMP_IVS_ReleaseResult(%d) failed\n", 2);
            return -1;
        }
    }

    /* Step.8 stop to ivs */
    ret = sample_ivs_facedet_stop(2, inteface);
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "sample_ivs_facedet_stop(0) failed\n");
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
    ret = sample_ivs_facedet_exit(0);
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
