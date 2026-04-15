#include <string.h>
#include <unistd.h>
#include <imp/imp_log.h>
#include <imp/imp_common.h>
#include <imp/imp_system.h>
#include <imp/imp_framesource.h>
#include <ivs/ivs_common.h>
#include <ivs/ivs_interface.h>
#include <ivs/ivs_inf_personvehiclepetDet.h>
#include <imp/imp_ivs.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include "sample-common.h"

#define TAG "SAMPLE-PERSONVEHICLEPETDET"
#define MODEL_BUFF
// #define SETPARAM
// #define DATA

#ifdef MODEL_BUFF
void * model_bin;
#endif

int sample_ivs_personvehiclepetdet_start(int grp_num, int chn_num, IMPIVSInterface **interface) {
    //check ivs version
    //PERSONVEHICLEPETDET_VERSION_NUM defined in ivs_inf_personvehiclepetdet.h .
    uint32_t personvehiclepetdet_ver = personvehiclepetdet_get_version_info();
    if(personvehiclepetdet_ver != PERSONVEHICLEPETDET_VERSION_NUM){
        printf("The version numbers of head file and lib do not match, head file version: %08x, lib version: %08x\n", PERSONVEHICLEPETDET_VERSION_NUM, personvehiclepetdet_ver);
        return -1;
    }
    //check ivs version
    int ret = 0;
    personvehiclepetdet_param_input_t param;

    memset(&param, 0, sizeof(personvehiclepetdet_param_input_t));
    param.frameInfo.width = sensor_sub_width; //640;
    param.frameInfo.height = sensor_sub_height; //360;
    param.ptime = true;
    param.skip_num = 0;      //skip num
    param.max_personvehiclepet_box = 100;
    param.sense = 3;
    param.detdist = 0; //0:640 1:800
    param.switch_track = false;

    param.enable_move = false;
    param.open_move_filter = false;
    if (param.open_move_filter){
        param.enable_move = true;
    }
    if (param.enable_move){
        param.move_sense=2;
        param.move_min_h=20;
        param.move_min_w=20;
        param.move_sldwin_size=4;
        param.move_ids_size = 1;
        param.move_ids[0] = 0;
        // param.move_ids[1] = 0;
    }

    param.model_path = "./model/model.mgk";
    param.check_model = false;
    param.switch_stop_det = false;

    param.observation_period = 2;
    param.active_count = 2;
    param.fil_score = 0.9;

    param.switch_stop_det = false;
    param.fast_update_params = false;

    param.enable_perm = false;
    if (param.enable_perm){
        param.permcnt = 1;
        param.mod = 0; //0 or 1
        if (param.mod == 0){
            param.perms[0].pcnt=6;
            param.perms[1].pcnt=5;
            param.perms[0].p = (IVSPoint *)malloc(6* sizeof(IVSPoint));
            param.perms[1].p = (IVSPoint *)malloc(5* sizeof(IVSPoint));

            int i ,j;
            for(i=0;i<param.permcnt;i++){
                switch(i){
                case 0:
                {
                    for( j=0;j<param.perms[0].pcnt;j++){
                        param.perms[0].p[j].x=(j%3)*70;
                        param.perms[0].p[j].y=(j/3)*170;
                    }
                }
                break;
                case 1:
                {
                    for( j=0;j<param.perms[1].pcnt;j++){
                        param.perms[1].p[j].x=(j%3)*70+160;
                        param.perms[1].p[j].y=(j/3)*170;
                    }
                }
                break;
                }
            }
        }else if(param.mod == 1){
            param.perms[0].r.ul.x = 100;
            param.perms[0].r.ul.y = 100;
            param.perms[0].r.br.x = 292;
            param.perms[0].r.br.y = 292;
            param.perms[0].detdist = 2;
                                             
            param.perms[1].r.ul.x = 320;
            param.perms[1].r.ul.y = 80;
            param.perms[1].r.br.x = 600;
            param.perms[1].r.br.y = 300;
            param.perms[1].detdist = 0;
        }
    }

    *interface = PersonvehiclepetDetInterfaceInit(&param);
    if (*interface == NULL) {
        printf("PersonvehiclepetDetInterfaceInit failed\n");
        return -1;
    }
    ret = (*interface)->init(*interface);
    if(ret < 0){
        printf("interface_init error\n");
        return -1;
    }
    return 0;
}

int sample_ivs_personvehiclepetdet_stop(int chn_num, IMPIVSInterface *interface) {
    int ret = 0;
    interface->exit(interface);
    PersonvehiclepetDetInterfaceExit(interface);
    return 0;
}

int main(int argc, char *argv[]) {
    // while(1){
    printf("personvehiclepet detect\n");
    int ret = 0;
#ifndef DATA
    IMPIVSInterface *interface = NULL;
    personvehiclepetdet_param_output_t *result = NULL;
#endif
    IVSPhyFrame nv12_frame;
    nv12_frame.width = atoi(argv[2]);
    nv12_frame.height = atoi(argv[3]);
    sensor_sub_width = nv12_frame.width;
    sensor_sub_height = nv12_frame.height;

    FILE *fp = NULL;
    int nv12_size = nv12_frame.width*nv12_frame.height*sizeof(unsigned char)*1.5;
#if 1
    unsigned char* nv12_data = (unsigned char*)malloc(nv12_size);
    nv12_frame.data = nv12_data;
    fp = fopen(argv[1], "rb");
    if(fp){
        ret = fread(nv12_data, 1, nv12_size, fp);
        fclose(fp);
    }else{
        printf("open nv12 file failed\n");
        return -1;
    }
#else
    // FILE *fp = NULL;
    int src_w, src_h, src_size;
    int rmem_fp;
    int test_fp;
    void * rmem_vaddr_test;

    nv12_frame.phydata = 0x3000000;//使用物理地址0x3000000
    rmem_fp = open("/dev/rmem", O_RDWR | O_SYNC);
    if(rmem_fp < 0)
    {
        printf("open /dev/rmem failed\n");
        return -1;
    }
    rmem_vaddr_test = mmap(NULL, nv12_size, PROT_READ | PROT_WRITE, MAP_SHARED, rmem_fp, (off_t)nv12_frame.phydata);
    nv12_frame.data = rmem_vaddr_test;
    printf("rmem_vaddr_test = %x\n", rmem_vaddr_test);
    if (rmem_vaddr_test == 0) {
        printf("AIP: ioaddr mmap failed\n");
        ret = -1;
    }
    close(rmem_fp);

    fp = fopen(argv[1], "r");

#if 0
    test_fp = fopen("./test_paddr_nv12.nv12", "w+");
    if(test_fp < 0)
    {
        printf("open test_fp failed\n");
        return -1;
    }
    ret = fwrite(rmem_vaddr_test, 32, nv12_size/32, test_fp);
    fclose(test_fp);
#endif

#endif
#ifndef DATA
    for (int ii=0; ii<1; ii++){
    // while(true){
    /* Step.6 Start to ivs */
    ret = sample_ivs_personvehiclepetdet_start(0, 2, &interface);
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "sample_ivs_personvehiclepetdet_start(0, 2) failed\n");
        return -1;
    }
    IMPFrameInfo nv12_frameinfo;
    /*Step.7 Get result*/
    int j = 1;
#if 0
    struct timeval tv;
    gettimeofday(&tv, NULL);
    uint64_t time_last1 = tv.tv_sec*1000000 + tv.tv_usec;
    printf("1000 start time: %f\n", time_last1*1.0/1000);
#endif
    int rrr = 0;
    while(1){
    // for (int ii=0; ii<10; ii++){
        // if(fp == NULL){
        //     printf("open nv12 file failed\n");
        //     return -1;
        // }else {
        //     ret = fread(rmem_vaddr_test, 32, nv12_size/32, fp);
        // }

#if 0
        test_fp = fopen("./test_paddr_nv12.nv12", "w+");
        if(test_fp < 0)
        {
            printf("open test_fp failed\n");
            return -1;
        }
        ret = fwrite(rmem_vaddr_test, 32, nv12_size/32, test_fp);
        fclose(test_fp);
#endif

        // if (ret == 0){
        //     break;
        // }

#ifdef SETPARAM
        if(ii%1 == 0){//间隔20帧更新一下参数
            printf("jjj: %d\n", j);
            int retk;
            personvehiclepetdet_param_input_t param;
            if(retk = interface->getParam(interface, (personvehiclepetdet_param_input_t *)(&param)) < 0){//if (interface->getParam && ((retk = interface->getParam(interface, &param) < 0))){
                IMP_LOG_ERR(TAG, "interface->getParam failed,ret=%d\n", retk);
                return -1;
            }
            param.sense = 4;
            param.enable_move = false;
            if(retk = interface->setParam(interface, (personvehiclepetdet_param_input_t *)(&param)) < 0){//if (interface->setParam && ((retk = interface->setParam(interface, &param) < 0))){
                IMP_LOG_ERR(TAG, "interface->setParam failed,ret=%d\n", retk);
                return -1;
            }
        }
#endif
        j++;
        nv12_frameinfo.width = nv12_frame.width;
        nv12_frameinfo.height = nv12_frame.height;
        nv12_frameinfo.virAddr = (uint32_t)nv12_frame.data;
        nv12_frameinfo.phyAddr = nv12_frame.phydata;//使用物理地址
        nv12_frameinfo.pixfmt = PIX_FMT_NV12;
        nv12_frameinfo.size = nv12_size;

        // printf("preProcessSync\n");
        ret = interface->preProcessSync(interface, &nv12_frameinfo);
        if (ret < 0) {
            printf("preProcessSync error\n");
            return -1;
        }
        // printf("processAsync\n");
        ret = interface->processAsync(interface, &nv12_frameinfo);
        if (ret < 0) {
            printf("processAsync error\n");
            return -1;
        }
        ret = interface->getResult(interface, (void**)&result);
        if (ret < 0) {
            printf("getResult error\n");
            return -1;
        }
        personvehiclepetdet_param_output_t* r = (personvehiclepetdet_param_output_t*)result;
     
        printf("r->count %d\n", r->count);
#if 1
        for(int i = 0; i < r->count; i++) {
            rrr++;
            int track_id = r->personvehiclepet[i].track_id;
            int class_id = r->personvehiclepet[i].class_id;
            float confidence = r->personvehiclepet[i].confidence;
            IVSRect* show_rect = &r->personvehiclepet[i].show_box;

            int x0,y0,x1,y1;
            x0 = (int)show_rect->ul.x;
            y0 = (int)show_rect->ul.y;
            x1 = (int)show_rect->br.x;
            y1 = (int)show_rect->br.y;
            printf("personvehiclepet location: trackID[%d], classID[%d], confidence[%f], [%d, %d, %d, %d] \n", track_id, class_id, confidence, x0, y0, x1, y1);
        }
        printf("isperson: %d\n\n", r->isperson);
#else
        printf("isperson: %d\n", r->isperson);
        for(int i = 0; i < r->count; i++) {
            if ((r->isperson && r->personvehiclepet[i].class_id == 0) || r->personvehiclepet[i].class_id != 0){
                rrr++;
                int track_id = r->personvehiclepet[i].track_id;
                int class_id = r->personvehiclepet[i].class_id;
                float confidence = r->personvehiclepet[i].confidence;
                IVSRect* show_rect = &r->personvehiclepet[i].show_box;

                int x0,y0,x1,y1;
                x0 = (int)show_rect->ul.x;
                y0 = (int)show_rect->ul.y;
                x1 = (int)show_rect->br.x;
                y1 = (int)show_rect->br.y;
                printf("personvehiclepet location: trackID[%d], classID[%d], confidence[%f], [%d, %d, %d, %d] \n", track_id, class_id, confidence, x0, y0, x1, y1);
            }
        }
     
#endif
        ret = interface->releaseResult(interface, (void**)&result);
        if (ret < 0) {
            printf("releaseResult error\n");
        return -1;
        }
    }
#if 0
    gettimeofday(&tv, NULL);
    time_last1 = tv.tv_sec*1000000 + tv.tv_usec - time_last1;
    float time_ms1 = time_last1*1.0/1000;
    printf("1000 time : %f\n", time_ms1);
#endif

    /* Step.8 stop to ivs */
    ret = sample_ivs_personvehiclepetdet_stop(2, interface);
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "sample_ivs_personvehiclepetdet_stop(0) failed\n");
        return -1;
    }
    free(nv12_data);
#ifdef MODEL_BUFF
    free(model_bin);
#endif
    }
#endif
    // }
    return 0;
}
