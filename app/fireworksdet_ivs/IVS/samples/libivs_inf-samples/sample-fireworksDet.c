#include <string.h>
#include <unistd.h>
#include <imp/imp_log.h>
#include <imp/imp_common.h>
#include <imp/imp_system.h>
#include <imp/imp_framesource.h>
#include <ivs/ivs_common.h>
#include <ivs/ivs_interface.h>
#include <ivs/ivs_inf_fireworksDet.h>
#include <imp/imp_ivs.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include "sample-common.h"

#define TAG "SAMPLE-FIREWORKSDET"
#define MODEL_BUFF

#ifdef MODEL_BUFF
void * model_bin;
#endif

int sample_ivs_fireworksdet_start(int grp_num, int chn_num, IMPIVSInterface **interface) {
    //check ivs version
    //FIREWORKSDET_VERSION_NUM defined in ivs_inf_fireworksdet.h .
    uint32_t fireworksdet_ver = fireworksdet_get_version_info();
    if(fireworksdet_ver != FIREWORKSDET_VERSION_NUM){
        printf("The version numbers of head file and lib do not match, head file version: %08x, lib version: %08x\n", FIREWORKSDET_VERSION_NUM, fireworksdet_ver);
        return -1;
    }
    //check ivs version
    int ret = 0; 
    fireworksdet_param_input_t param;

    memset(&param, 0, sizeof(fireworksdet_param_input_t));
    param.frameInfo.width = sensor_sub_width; //640;
    param.frameInfo.height = sensor_sub_height; //360;

    param.skip_num = 0;      //skip num
    param.max_fireworks_box = 20;
    param.sense = 4;//
    param.detdist = 4;//  默认 4
    param.switch_track = true;
    param.nmem_size = 15*1024*1024; 
    param.ptime = true;
#ifndef MODEL_BUFF
    param.model_path = "./model/fireworks_det.bin";
#else
    param.model_path = "./model/fireworks_det.bin";
    // FILE *fp = NULL;
    // fp = fopen(param.model_path, "r");
    // struct  stat statbuf;
    // stat (param.model_path, &statbuf);
    // size_t model_size = statbuf.st_size;

    // model_bin = malloc(sizeof(char)*model_size);
    // if(fp == NULL){
	// 	printf("open model file failed\n");
	// 	return -1;
	// }else {
	// 	ret = fread(model_bin, 1, model_size, fp);
	// 	fclose(fp);
	// }
    // param.model_path = model_bin;
#endif
    param.switch_stop_det = false;
    param.fast_update_params = false;

    *interface = FireworksDetInterfaceInit(&param);
    if (*interface == NULL) {
        printf("FireworksDetInterfaceInit failed\n");
        return -1;
    }
    ret = (*interface)->init(*interface);
    if(ret < 0){
        printf("interface_init error\n");
        return -1;
    }
    return 0;
}

int sample_ivs_fireworksdet_stop(int chn_num, IMPIVSInterface *interface) {
    int ret = 0;
    interface->exit(interface);
    FireworksDetInterfaceExit(interface);
    return 0;
}

int main(int argc, char *argv[]) {
    printf("personvehicle detect\n");
    int ret = 0;
    IMPIVSInterface *interface = NULL;
    fireworksdet_param_output_t *result = NULL;

    IVSPhyFrame nv12_frame;
    nv12_frame.width = atoi(argv[2]);
    nv12_frame.height = atoi(argv[3]);
    sensor_sub_width = nv12_frame.width;
    sensor_sub_height = nv12_frame.height;

	FILE *fp = NULL;
	int nv12_size = nv12_frame.width*nv12_frame.height*sizeof(unsigned char)*1.5;

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
    
    /* Step.6 Start to ivs */
    ret = sample_ivs_fireworksdet_start(0, 2, &interface);
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "sample_ivs_fireworksdet_start(0, 2) failed\n");
        return -1;
    }
    IMPFrameInfo nv12_frameinfo;
    /*Step.7 Get result*/
    int j = 1;
#if 1
    struct timeval tv;
    gettimeofday(&tv, NULL);
    uint64_t time_last1 = tv.tv_sec*1000000 + tv.tv_usec;
    printf("1000 start time: %f\n", time_last1*1.0/1000);
#endif

    int rrr = 0;
    while(true){
    // for (int ii=0; ii<20; ii++){
        j++;
        nv12_frameinfo.width = nv12_frame.width;
        nv12_frameinfo.height = nv12_frame.height;
        nv12_frameinfo.virAddr = (uint32_t)nv12_frame.data;
        nv12_frameinfo.phyAddr = nv12_frame.phydata;//使用物理地址
        nv12_frameinfo.pixfmt = PIX_FMT_NV12;
        nv12_frameinfo.size = nv12_size;

        ret = interface->preProcessSync(interface, &nv12_frameinfo);
        if (ret < 0) {
            printf("preProcessSync error\n");
            return -1;
        }
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
        fireworksdet_param_output_t* r = (fireworksdet_param_output_t*)result;

        for(int i = 0; i < r->count; i++) {
            int track_id = r->fireworks[i].track_id;
            int class_id = r->fireworks[i].class_id;
            float confidence = r->fireworks[i].confidence;
            IVSRect* show_rect = &r->fireworks[i].show_box;

            int x0,y0,x1,y1;
            x0 = (int)show_rect->ul.x;
            y0 = (int)show_rect->ul.y;
            x1 = (int)show_rect->br.x;
            y1 = (int)show_rect->br.y;
            printf("fireworks location: trackID[%d], classID[%d], confidence[%f], [%d, %d, %d, %d] \n", track_id, class_id, confidence, x0, y0, x1, y1);
        }
        ret = interface->releaseResult(interface, (void**)&result);
        if (ret < 0) {
            printf("releaseResult error\n");
        return -1;
        }
    }
#if 1
    gettimeofday(&tv, NULL);
    time_last1 = tv.tv_sec*1000000 + tv.tv_usec - time_last1;
    float time_ms1 = time_last1*1.0/1000;
    printf("1000 time : %f\n", time_ms1);
#endif
    // printf("rrr: %d\n", rrr);

    /* Step.8 stop to ivs */
    ret = sample_ivs_fireworksdet_stop(2, interface);
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "sample_ivs_fireworksdet_stop(0) failed\n");
        return -1;
    }
    free(nv12_data);
#ifdef MODEL_BUFF
    free(model_bin);
#endif
    return 0;
}
