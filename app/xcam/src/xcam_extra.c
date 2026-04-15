#include "xcam_extra.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <imp/imp_ivs.h>
#include <imp/imp_osd.h>
#include <sys/stat.h>
#include "xcam_extra.h"
#include "xcam_thread.h"
#include "xcam_video.h"
#if defined T41
#include "xcam_video_t41.h"
#endif
#include "xcam_log.h"
#include "xcam_conf_video.h"
#include "xcam_conf_sys.h"
#include "xcam_general.h"
#include "xcam_osd.h"
#include "xcam_msg.h"
#include "xcam_module.h"
#include "xcam_stream.h"
#include "xcam_conf_network.h"
#include "xcam_cli_options.h"
#include "xcam_com.h"
#ifdef GEKKO_ENABLE
#include "gekko.h"
#include "gekko_ai3dnr.h"
#include "type.h"
#include "version.h"
#endif

#include "xcam_video_t32.h"

#ifdef MOVEDET_ENABLE
#include <move/ivs_common.h>
#include <move/ivs_interface.h>
#include <move/ivs_inf_move.h>
#endif

#ifdef PLATEREC_ENABLE
#include <platerec/ivs_common.h>
#include <platerec/ivs_interface.h>
#include <platerec/ivs_inf_plateRec.h>
#endif

#ifdef FACEDET_ENABLE
#include <facedet/ivs_common.h>
#include <facedet/ivs_interface.h>
#include <facedet/ivs_inf_faceDet.h>
#endif

#ifdef FIREWORKSDET_ENABLE
#include <fireworksdet/ivs_common.h>
#include <fireworksdet/ivs_interface.h>
#include <fireworksdet/ivs_inf_fireworksDet.h>
#endif

#ifdef PERVEHPETDET_ENABLE
#include <personvehiclepetdet/ivs_common.h>
#include <personvehiclepetdet/ivs_interface.h>
#include <personvehiclepetdet/ivs_inf_personvehicleDet.h>
#endif 

#define ArrayLen(arr) (sizeof(arr) / sizeof(arr[0]))
#define TAG "HAL_INGENIC"
#define LOG_TAG "EXTRA_IVS"
#define  MOVE_INDEX 0
#define  PERVEH_INDEX 1
#define  FACE_INDEX 2
pthread_t ithreads[3];
int ithreadstat[3];

void drawRect(int flag, float x0, float y0, float w, float h, int handle){
    IMPOSDRgnAttr attr;
    int ret;

	attr.type = OSD_REG_ISP_LINE_RECT;
	attr.osdispdraw.stDrawAttr.pinum = handle;
	attr.osdispdraw.stDrawAttr.type = IMP_ISP_DRAW_WIND;
	attr.osdispdraw.stDrawAttr.color_type = IMPISP_MASK_TYPE_YUV;
	attr.osdispdraw.stDrawAttr.cfg.wind.enable = 1;
	attr.osdispdraw.stDrawAttr.cfg.wind.left = (int)x0;
	attr.osdispdraw.stDrawAttr.cfg.wind.top = (int)y0;
	attr.osdispdraw.stDrawAttr.cfg.wind.width = (int)w;
	attr.osdispdraw.stDrawAttr.cfg.wind.height = (int)h;
	attr.osdispdraw.stDrawAttr.cfg.wind.color.ayuv.y_value = 0;
	attr.osdispdraw.stDrawAttr.cfg.wind.color.ayuv.u_value = 255;
	attr.osdispdraw.stDrawAttr.cfg.wind.color.ayuv.v_value = 0;
	attr.osdispdraw.stDrawAttr.cfg.wind.line_width = 3;
	attr.osdispdraw.stDrawAttr.cfg.wind.alpha = 4;

	if(!flag){
        attr.osdispdraw.stDrawAttr.cfg.wind.enable = 0;
        IMP_OSD_SetRgnAttr_ISP(0, &attr, 0);
        return;
    }
   	ret = IMP_OSD_SetRgnAttr_ISP(0, &attr, 0);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_OSD_SetRgnAttr_ISP failed\n");
	}
}

#if MOVEDET_ENABLE
static int xcam_ivs_move_init(IMPIVSInterface **interface) {

    //check ivs version
    //MOVE_VERSION_NUM defined in ivs_inf_move.h.
	int sensor_sub_width = SENSOR_RESOLUTION_WIDTH_SLAVE;
	int sensor_sub_height = SENSOR_RESOLUTION_HEIGHT_SLAVE;
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
#if 1    
    param.perms[0].p[0].x=0;
    param.perms[0].p[0].y=0;
    param.perms[0].p[1].x=640 /2 -1;
    param.perms[0].p[1].y=0;
    param.perms[0].p[2].x=640 /2 -1;
    param.perms[0].p[2].y=360 /2 -1;
    param.perms[0].p[3].x=0;
    param.perms[0].p[3].y=360 /2 -1;
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
        IMP_LOG_ERR(TAG, "IMP_IVS_CreateGroup(%d) failed\n", __LINE__);
        return -1;
    }
	return 0;
}

void *xcam_ivs_move_thread(void *args)
{
	IMPIVSInterface *interface = (IMPIVSInterface *)args;
	int ret = 0, i = 0, j = 0, x0 = 0, y0 = 0, x1 = 0, y1 = 0;
	unsigned char * g_sub_nv12_buf = (unsigned char *)malloc(SENSOR_RESOLUTION_WIDTH_SLAVE * SENSOR_RESOLUTION_HEIGHT_SLAVE * 3 / 2);
	if(g_sub_nv12_buf == NULL) {
		IMP_LOG_ERR(TAG, "malloc sub nv12 buffer failed(%d)\n", __LINE__);
		return NULL;
	}

	IMPFrameInfo sframe;
	move_param_output_t *result = NULL;
    sframe.virAddr = (unsigned int)g_sub_nv12_buf;
	while(1) {
        pthread_mutex_lock(&xcam_snapframe_mutex);
		ret = IMP_FrameSource_SnapFrame(CH1_INDEX, PIX_FMT_NV12, SENSOR_RESOLUTION_WIDTH_SLAVE,  SENSOR_RESOLUTION_HEIGHT_SLAVE, g_sub_nv12_buf, &sframe);
		pthread_mutex_unlock(&xcam_snapframe_mutex);
        if (ret < 0) {
			printf("%d get frame failed try again\n", 1);
			usleep(30*1000);
			continue;
		}

		
		ret = interface->preProcessSync(interface, &sframe);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "preProcessSync_err\n");
			return NULL;
		}

		ret = interface->processAsync(interface, &sframe);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "processAsync_err\n");
			return NULL;
		}

		ret = interface->getResult(interface, (void **)&result);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "getResult_err\n");
			return NULL;
		}
        // printf("%d\n",result->rectcnt);
        
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
        // printf("%d\n",result->detcount);
        // int x0,y0,k;
        // if(result->detcount>0){
        //     for (k=0; k < result->detcount;k++){
        //         x0 = result->g_res.det_rects[k].x;
        //         y0 = result->g_res.det_rects[k].y;
        //         //printf("%d %d\n",x0,y0);

        //     }
        // }
        ret = interface->releaseResult(interface, (void *)result);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "interface->releaseResult failed ret=%d\n", ret);
			return -1;
		}
        if(ithreadstat[MOVE_INDEX] == 2){
            break;
        }
    }
    if(g_sub_nv12_buf)
        free(g_sub_nv12_buf);
	MoveInterfaceExit(interface);
	return ;
}

int xcam_Move_ivs(void)
{
	int ret = 0;
	IMPIVSInterface *interface = NULL;
	ret = xcam_ivs_move_init(&interface);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "xcam_Move_ivs(0, 0) failed\n");
		return -1;
	}

	ret = interface->init(interface);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "xcam_Move_ivs interface_init_err(%d)\n", __LINE__);
		return -1;
	}
#if 1
	pthread_t* moveDetectIvs_t = (ithreads + MOVE_INDEX);
	pthread_attr_t attr;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);  // 设置分离属性
	ret = pthread_create(moveDetectIvs_t, NULL, xcam_ivs_move_thread, (void *)interface);
	if(ret < 0) {
		IMP_LOG_ERR(TAG, "create moveDetect thread failed\n");
		return -1;
	}
#endif
	return ret;
}
#endif


#if PLATEREC_ENABLE
static int xcam_ivs_plateRec_init(IMPIVSInterface **interface) {

     //check ivs version
    //MOVE_VERSION_NUM defined in ivs_inf_move.h.
	int sensor_sub_width = SENSOR_RESOLUTION_WIDTH_SLAVE;
	int sensor_sub_height = SENSOR_RESOLUTION_HEIGHT_SLAVE;
//check ivs version
    //PLATEREC_VERSION_NUM defined in ivs_inf_platerec.h .
    uint32_t platerec_ver = platerec_get_version_info();
    if(platerec_ver != PLATEREC_VERSION_NUM){
        printf("The version numbers of head file and lib do not match, head file version: %08x, lib version: %08x\n",PLATEREC_VERSION_NUM,platerec_ver);
        return -1;
    }

    platerec_param_input_t param;
    memset(&param, 0, sizeof(platerec_param_input_t));
    param.frameInfo.width = sensor_sub_width; //recommend 720P
    param.frameInfo.height = sensor_sub_height;
    param.skip_num = 0;      //skip num
    param.max_vehicle_box = 10;      //real recgonit width
    param.max_plate_box = 10;      //real recgonit width
    param.rot90 = false;
    param.sense = 1;
    param.detdist = 0;
    param.switch_track = true;
    param.switch_plate_rec = true;
    param.switch_plate_vehtype = false;
    param.switch_plate_vehcolor = false;
    param.switch_plate_placolor = true;
    param.switch_plate_platype = true;
    param.enable_move = false;
    param.model_path = "model/plate_det.bin";
    param.model_path_ldmk = "model/plate_ldmk.bin";
    param.model_path_vehtype = "";
    param.model_path_vehcolor = "";
    param.model_path_placolor = "model/plate_color.bin";
    param.model_path_platype = "model/plate_type.bin";
    param.model_path_rec = "model/plate_rec.bin";
    param.ptime = false;
      param.nmem_size = (int)(4*1024*1024);//8.5*1024*1024;
    // param.min_reg_height

    // init
    *interface = PlateRecInterfaceInit(&param);
    if (*interface == NULL) {
        IMP_LOG_ERR(TAG, "IMP_IVS_CreateGroup(%d) failed\n", __LINE__);
        return -1;
    }
	return 0;
}

void *xcam_ivs_plateRec_thread(void *args)
{
	IMPIVSInterface *interface = (IMPIVSInterface *)args;
	int ret = 0, i = 0, j = 0, x0 = 0, y0 = 0, x1 = 0, y1 = 0;
	unsigned char * g_sub_nv12_buf = (unsigned char *)malloc(SENSOR_RESOLUTION_WIDTH_SLAVE * SENSOR_RESOLUTION_HEIGHT_SLAVE * 3 / 2);
	if(g_sub_nv12_buf == NULL) {
		IMP_LOG_ERR(TAG, "malloc sub nv12 buffer failed(%d)\n", __LINE__);
		return NULL;
	}
    char txtbuff[1000];
    
	IMPFrameInfo sframe;
	platerec_param_output_t *result = NULL;
	while(1) {
            
        pthread_mutex_lock(&xcam_snapframe_mutex);
		ret = IMP_FrameSource_SnapFrame(CH1_INDEX, PIX_FMT_NV12, SENSOR_RESOLUTION_WIDTH_SLAVE,  SENSOR_RESOLUTION_HEIGHT_SLAVE, g_sub_nv12_buf, &sframe);
		pthread_mutex_unlock(&xcam_snapframe_mutex);
        if (ret < 0) {
			printf("%d get frame failed try again\n", 1);
			usleep(30*1000);
			continue;
		}
		sframe.virAddr = (unsigned int)g_sub_nv12_buf;
		ret = interface->preProcessSync(interface, &sframe);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "preProcessSync_err\n");
			return NULL;
		}

		ret = interface->processAsync(interface, &sframe);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "processAsync_err\n");
			return NULL;
		}

		ret = interface->getResult(interface, (void **)&result);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "getResult_err\n");
			return NULL;
		}
        
        for(int k = 0; k < result->vehicle_count; k++) {
            IVSRect* r = &(result->vehicle[k].show_box);
            float conf = result->vehicle[k].confidence;
            int vehicle_type= result->vehicle[k].type;
            int vehicle_color= result->vehicle[k].color;
            int track_id = result->vehicle[k].track_id;
            sprintf(txtbuff, "[vehicle]%d,%d,%d,%d ", r->ul.x, r->ul.y, r->br.x, r->br.y);
         
            printf("%s\n", txtbuff);
        }
        for(int k = 0; k < result->plate_count; k++) {
            IVSRect* r = &(result->plate[k].show_box);
            const char *licence = result->plate[k].licence;
            int id = result->plate[k].track_id;
            int plate_color= result->plate[k].color;
            int plate_type= result->plate[k].type;
            float conf = result->plate[k].confidence;

            sprintf(txtbuff, "[plate]%d,%d,%d,%d ", r->ul.x, r->ul.y, r->br.x, r->br.y);
        
            printf("%s", txtbuff);

            sprintf(txtbuff, "[licence]%s [id]%d [conf]%.2f [type]%d [color]%d", licence, id, conf, plate_type, plate_color);
            printf("%s\n", txtbuff);
        ret = interface->releaseResult(interface, (void *)result);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "interface->releaseResult failed ret=%d\n", ret);
			return -1;
		}
        }
    }
    if(g_sub_nv12_buf)
        free(g_sub_nv12_buf);
	MoveInterfaceExit(interface);
	return ;
}

int xcam_PlateRec_ivs(void)
{
	int ret = 0;
	IMPIVSInterface *interface = NULL;
	ret = xcam_ivs_plateRec_init(&interface);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "xcam_plateRec_ivs(0, 0) failed\n");
		return -1;
	}

	ret = interface->init(interface);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "xcam_plateRec_ivs interface_init_err(%d)\n", __LINE__);
		return -1;
	}
#if 1
	pthread_t plateRecIvs_t;
	pthread_attr_t attr;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);  // 设置分离属性
	ret = pthread_create(&plateRecIvs_t, NULL, xcam_ivs_plateRec_thread, (void *)interface);
	if(ret < 0) {
		IMP_LOG_ERR(TAG, "create plateRec thread failed\n");
		return -1;
	}
#endif
	return ret;
}
#endif




#if FACEDET_ENABLE
static int xcam_ivs_faceDet_init(IMPIVSInterface **interface) {

     //check ivs version
    //MOVE_VERSION_NUM defined in ivs_inf_move.h.
	int sensor_sub_width = SENSOR_RESOLUTION_WIDTH_SLAVE;
	int sensor_sub_height = SENSOR_RESOLUTION_HEIGHT_SLAVE;
 uint32_t facedet_ver = facedet_get_version_info();
    if(facedet_ver != FACEDET_VERSION_NUM){
        printf("The version numbers of head file and lib do not match, head file version: %08x, lib version: %08x\n", FACEDET_VERSION_NUM, facedet_ver);
        return -1;
    }
    //check ivs version
    int ret = 0;
    facedet_param_input_t param;

    memset(&param, 0, sizeof(facedet_param_input_t));
    param.frameInfo.width = SENSOR_RESOLUTION_WIDTH_SLAVE; //recommend 720P
    param.frameInfo.height = SENSOR_RESOLUTION_HEIGHT_SLAVE;

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
    param.nmem_size = (int)(8*1024*1024);//8.5*1024*1024;
    param.share_mem_mode = 0;

    *interface = FaceDetInterfaceInit(&param);
    if (*interface == NULL) {
        IMP_LOG_ERR(TAG, "IMP_IVS_CreateGroup(%d) failed\n", __LINE__);
        return -1;
    }
	return 0;
}

void *xcam_ivs_faceDet_thread(void *args)
{
	IMPIVSInterface *interface = (IMPIVSInterface *)args;
	int ret = 0, i = 0, j = 0, x0 = 0, y0 = 0, x1 = 0, y1 = 0;
	unsigned char * g_sub_nv12_buf = (unsigned char *)malloc(SENSOR_RESOLUTION_WIDTH_SLAVE * SENSOR_RESOLUTION_HEIGHT_SLAVE * 3 / 2);
	if(g_sub_nv12_buf == NULL) {
		IMP_LOG_ERR(TAG, "malloc sub nv12 buffer failed(%d)\n", __LINE__);
		return NULL;
	}

	IMPFrameInfo sframe;
	facedet_param_output_t *result = NULL;
	while(1) {
        pthread_mutex_lock(&xcam_snapframe_mutex);
		ret = IMP_FrameSource_SnapFrame(CH1_INDEX, PIX_FMT_NV12, SENSOR_RESOLUTION_WIDTH_SLAVE,  SENSOR_RESOLUTION_HEIGHT_SLAVE, g_sub_nv12_buf, &sframe);
		pthread_mutex_unlock(&xcam_snapframe_mutex);
        if (ret < 0) {
			printf("%d get frame failed try again\n", 1);
			usleep(30*1000);
			continue;
		}
		sframe.virAddr = (unsigned int)g_sub_nv12_buf;
		ret = interface->preProcessSync(interface, &sframe);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "preProcessSync_err\n");
			return NULL;
		}

		ret = interface->processAsync(interface, &sframe);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "processAsync_err\n");
			return NULL;
		}

		ret = interface->getResult(interface, (void **)&result);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "getResult_err\n");
			return NULL;
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
        ret = interface->releaseResult(interface, (void *)result);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "interface->releaseResult failed ret=%d\n", ret);
			return -1;
		}
        if(ithreadstat[FACE_INDEX] == 2){
            break;
        }
    }
    if(g_sub_nv12_buf)
        free(g_sub_nv12_buf);
	FaceDetInterfaceExit(interface);
	return ;
}

int xcam_FaceDet_ivs(void)
{
	int ret = 0;
	IMPIVSInterface *interface = NULL;
	ret = xcam_ivs_faceDet_init(&interface);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "xcam_faceDet_ivs(0, 0) failed\n");
		return -1;
	}

	ret = interface->init(interface);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "xcam_faceDet_ivs interface_init_err(%d)\n", __LINE__);
		return -1;
	}
#if 1
	pthread_t* faceDetIvs_t = (ithreads + FACE_INDEX);
	pthread_attr_t attr;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);  // 设置分离属性
	ret = pthread_create(faceDetIvs_t, NULL, xcam_ivs_faceDet_thread, (void *)interface);
	if(ret < 0) {
		IMP_LOG_ERR(TAG, "create faceDet thread failed\n");
		return -1;
	}
#endif
	return ret;
}
#endif






#if FIREWORKSDET_ENABLE 
static int xcam_ivs_fireworksDet_init(IMPIVSInterface **interface) {

     //check ivs version
    //MOVE_VERSION_NUM defined in ivs_inf_move.h.
	int sensor_sub_width = SENSOR_RESOLUTION_WIDTH_SLAVE;
	int sensor_sub_height = SENSOR_RESOLUTION_HEIGHT_SLAVE;
  uint32_t fireworksdet_ver = fireworksdet_get_version_info();
    if(fireworksdet_ver != FIREWORKSDET_VERSION_NUM){
        printf("The version numbers of head file and lib do not match, head file version: %08x, lib version: %08x\n", FIREWORKSDET_VERSION_NUM, fireworksdet_ver);
        return -1;
    }
    //check ivs version
    int ret = 0; 
    fireworksdet_param_input_t param;

    memset(&param, 0, sizeof(fireworksdet_param_input_t));
    param.frameInfo.width = SENSOR_RESOLUTION_WIDTH_SLAVE; //640;
    param.frameInfo.height = SENSOR_RESOLUTION_HEIGHT_SLAVE; //360;

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
	return 0;
}

void *xcam_ivs_fireworksDet_thread(void *args)
{
	IMPIVSInterface *interface = (IMPIVSInterface *)args;
	int ret = 0, i = 0, j = 0, x0 = 0, y0 = 0, x1 = 0, y1 = 0;
	unsigned char * g_sub_nv12_buf = (unsigned char *)malloc(SENSOR_RESOLUTION_WIDTH_SLAVE * SENSOR_RESOLUTION_HEIGHT_SLAVE * 3 / 2);
	if(g_sub_nv12_buf == NULL) {
		IMP_LOG_ERR(TAG, "malloc sub nv12 buffer failed(%d)\n", __LINE__);
		return NULL;
	}

	IMPFrameInfo sframe;
	fireworksdet_param_output_t *result = NULL;
	while(1) {
        pthread_mutex_lock(&xcam_snapframe_mutex);
		ret = IMP_FrameSource_SnapFrame(CH1_INDEX, PIX_FMT_NV12, SENSOR_RESOLUTION_WIDTH_SLAVE,  SENSOR_RESOLUTION_HEIGHT_SLAVE, g_sub_nv12_buf, &sframe);
		pthread_mutex_unlock(&xcam_snapframe_mutex);
        if (ret < 0) {
			printf("%d get frame failed try again\n", 1);
			usleep(30*1000);
			continue;
		}
		sframe.virAddr = (unsigned int)g_sub_nv12_buf;
		ret = interface->preProcessSync(interface, &sframe);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "preProcessSync_err\n");
			return NULL;
		}

		ret = interface->processAsync(interface, &sframe);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "processAsync_err\n");
			return NULL;
		}

		ret = interface->getResult(interface, (void **)&result);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "getResult_err\n");
			return NULL;
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
        ret = interface->releaseResult(interface, (void *)result);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "interface->releaseResult failed ret=%d\n", ret);
			return -1;
		}
    }
    if(g_sub_nv12_buf)
        free(g_sub_nv12_buf);
	FireworksDetInterfaceExit(interface);
	return ;
}

int xcam_FireworksDet_ivs(void)
{
	int ret = 0;
	IMPIVSInterface *interface = NULL;
	ret = xcam_ivs_fireworksDet_init(&interface);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "xcam_fireworksDet_ivs(0, 0) failed\n");
		return -1;
	}

	ret = interface->init(interface);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "xcam_fireworksDet_ivs interface_init_err(%d)\n", __LINE__);
		return -1;
	}
#if 1
	pthread_t fireworksDetIvs_t;
	pthread_attr_t attr;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);  // 设置分离属性
	ret = pthread_create(&fireworksDetIvs_t, NULL, xcam_ivs_fireworksDet_thread, (void *)interface);
	if(ret < 0) {
		IMP_LOG_ERR(TAG, "create fireworksDet thread failed\n");
		return -1;
	}
#endif
	return ret;
}
#endif


#if PERVEHPETDET_ENABLE

#define MODEL_BUFF

#ifdef MODEL_BUFF
void * model_bin;
#endif

static int xcam_ivs_personvehiclepetDet_ivs_init(IMPIVSInterface **interface) {

     //check ivs version
    //PERSONVEHICLEDET_VERSION_NUM defined in ivs_inf_personvehicledet.h .
    uint32_t personvehicledet_ver = personvehicledet_get_version_info();
    if(personvehicledet_ver != PERSONVEHICLEDET_VERSION_NUM){
        printf("The version numbers of head file and lib do not match, head file version: %08x, lib version: %08x\n", PERSONVEHICLEDET_VERSION_NUM, personvehicledet_ver);
        return -1;
    }
    //check ivs version
    int ret = 0;
    personvehicledet_param_input_t param;

    memset(&param, 0, sizeof(personvehicledet_param_input_t));
    param.frameInfo.width = FIRST_SENSOR_WIDTH_THIRD; //640;
    param.frameInfo.height = FIRST_SENSOR_HEIGHT_THIRD; //360;
    param.ptime = false;
    param.skip_num = 5;      //skip num
    param.max_personvehicle_box = 20;
    param.sense = 6;
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
    param.model_path = "/system/model/model.mgk";
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

    *interface = PersonvehicleDetInterfaceInit(&param);
    if (*interface == NULL) {
        printf("PersonvehicleDetInterfaceInit failed\n");
        return -1;
    }
    ret = (*interface)->init(*interface);
    if(ret < 0){
        printf("interface_init error\n");
        return -1;
    }
    return 0;
}
struct timeval start_time, stop_time;
double __get_us(struct timeval t) { 
	return (t.tv_sec * 1000000 + t.tv_usec); 
}
void *xcam_ivs_personvehiclepetDet_thread(void *args)
{
	IMPIVSInterface *interface = (IMPIVSInterface *)args;
	unsigned char * g_sub_nv12_buf = (unsigned char *)malloc(FIRST_SENSOR_WIDTH_THIRD * FIRST_SENSOR_HEIGHT_THIRD * 3 / 2);
	if(g_sub_nv12_buf == NULL) {
		IMP_LOG_ERR(TAG, "malloc sub nv12 buffer failed(%d)\n", __LINE__);
		return NULL;
	}
    int frame_num = 0; 
    int last_frame_num = 0; 
    int osd_count = 0; 
    int frame_cnt = 0;
    int ret;
	IMPFrameInfo sframe;
	personvehicledet_param_output_t *result = NULL;
    xcam_thread_set_name("xcam_ivs_personvehicl");
    float  slave_multi_w = ((float)SENSOR_RESOLUTION_WIDTH_MAIN / FIRST_SENSOR_WIDTH_THIRD);
    // float  slave_multi_h = (SENSOR_RESOLUTION_HEIGHT_MAIN / FIRST_SENSOR_HEIGHT_THIRD); for warning
	while(1) {
        pthread_mutex_lock(&xcam_snapframe_mutex);
		ret = IMP_FrameSource_SnapFrame(1, PIX_FMT_NV12, FIRST_SENSOR_WIDTH_THIRD,  FIRST_SENSOR_HEIGHT_THIRD, g_sub_nv12_buf, &sframe);
        frame_num += 1 ;
    	pthread_mutex_unlock(&xcam_snapframe_mutex);
        if (ret < 0) {
			printf("%d get frame failed try again\n", 1);
			usleep(30*1000);
			continue;
		}
		sframe.virAddr = (unsigned int)g_sub_nv12_buf;
		ret = interface->preProcessSync(interface, &sframe);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "preProcessSync_err\n");
			return NULL;
		}

		ret = interface->processAsync(interface, &sframe);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "processAsync_err\n");
			return NULL;
		}

		ret = interface->getResult(interface, (void **)&result);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "getResult_err\n");
			return NULL;
		}
        personvehicledet_param_output_t* r = (personvehicledet_param_output_t*)result;
        for(int i = 0; i < r->count; i++) {
           /* int track_id = r->personvehicle[i].track_id;
            int class_id = r->personvehicle[i].class_id;
            float confidence = r->personvehicle[i].confidence;  for warning*/
            IVSRect* show_rect = &r->personvehicle[i].show_box;  
            //  slave_multi_w = (SENSOR_RESOLUTION_WIDTH_MAIN / SENSOR_RESOLUTION_WIDTH_SLAVE);
            int x0,y0,x1,y1;
            x0 = (int)show_rect->ul.x  ;
            y0 = (int)show_rect->ul.y;
            x1 = (int)show_rect->br.x;
            y1 = (int)show_rect->br.y;
            drawRect(1, x0 * slave_multi_w + 10, y0 * slave_multi_w - 40, (x1 -x0) * slave_multi_w * 0.85, (y1 - y0) * slave_multi_w  * 0.75, i);
            osd_count = osd_count >= r->count -1 ? osd_count : r->count -1;
            
        }
        
        if(frame_num - last_frame_num > 20  ){
            for(; osd_count >= 0; osd_count--)
                drawRect(0, 0, 0, 0, 0, osd_count);
            last_frame_num = frame_num ;
        }
        ret = interface->releaseResult(interface, (void *)result);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "interface->releaseResult failed ret=%d\n", ret);
			return (void *)XCAM_ERROR;
		}
        frame_cnt += 1;
        if(ithreadstat[PERVEH_INDEX] == 2){
            break;
        }
    }
    if(g_sub_nv12_buf)
        free(g_sub_nv12_buf);
	PersonvehicleDetInterfaceExit(interface);
	return (void*)XCAM_SUCCESS;
}

int xcam_PersonvehiclepetDet_ivs(void)
{
	int ret = 0;
	IMPIVSInterface *interface = NULL;
	ret = xcam_ivs_personvehiclepetDet_ivs_init(&interface);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "xcam_PersonvehiclepetDet_ivs(0, 0) failed\n");
		return -1;
	}

	ret = interface->init(interface);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "xcam_PersonvehiclepetDet_ivs interface_init_err(%d)\n", __LINE__);
		return -1;
	}
#if 1
	pthread_t* personvehiclepetDetIvs_t = (ithreads + PERVEH_INDEX);
	pthread_attr_t attr;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);  // 设置分离属性
	ret = pthread_create(personvehiclepetDetIvs_t, NULL, xcam_ivs_personvehiclepetDet_thread, (void *)interface);
	if(ret < 0) {
		IMP_LOG_ERR(TAG, "create personvehiclepetDet_ivs thread failed\n");
		return -1;
	}
#endif
	return ret;
}
#endif


#ifdef AIISP

int xcam_gekko_init(){
        //获取库的版本
    unsigned int gekko_ver = gekko_get_version();
    if (GEKKO_SDK_VERSION_NUM != gekko_ver) {
        IMP_LOG_ERR(TAG, "gekko 3dnr lib version doesn't match!\n");
        return -1;
    }
    IMP_LOG_INFO(TAG,"GEKKO_SDK_VERSION_NUM:%08x\n",gekko_ver);

    //注意此版本（GEKKO_SDK_VERSION_NUM >0.6）要放到IMP_System_Init之后
    int ret = gekko_init(VI_MAIN,40*1024*1024);
    if (0 != ret) {
        IMP_LOG_ERR(TAG, "gekko init failed\n");
        return -1;
    }
    return 0;
}


int xcam_ai3drn_init(){
    /*ai3dnr init*/

    char  gekkoIQbin_path[1024] = {};
    sprintf(gekkoIQbin_path,"/etc/sensor/%s-t41@gekko.bin",FIRST_SNESOR_NAME);
    int g_width = FIRST_SENSOR_WIDTH;
    int g_height = FIRST_SENSOR_HEIGHT;
    AI3DNRConfig ai3dnr_cfg;
    int ret = 0;
    ai3dnr_cfg.mode_path = "/system/model_file/ai3dnr.bin";//配置自己存放模型的路径，名字可修改
    ai3dnr_cfg.fpn_path = "/system/model_file/00efcd.bin";//配置自己存放FPN（标定文件）的路径，名字可修改
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
    return 0;
}

int  xcam_gekko_start(){
          //AISP开始处理信息
    int ret = gekko_start(VI_MAIN);//切IQ bin与切FPS，跟gekko_start走
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "gekko start failed\n");
        return -1;
    }
}   

void  xcam_bv_enable(){

    /*********************************Use Bv sample ***************************************************/
    //注意当前BV 概念只在开启AISP后使用；关闭AISP后传统环境使用EV
    // {
    //   BVLut bv_list;
    //   int ret = gekko_get_bv_lut(VI_MAIN,&bv_list);
    //   if (ret < 0) {
    //       IMP_LOG_ERR(TAG, "gekko_get_bv_lut failed\n");
    //       // return -1;
    //   }
    //   for (int i=0; i<BV_LUT_ITEM_NUM; i++) {
    //     IMP_LOG_INFO(TAG,"bv[%d]:%d\n",i,bv_list.bv[i]);
    //   }
    //   int real_bv;
    //   ret = IMP_ISP_Tuning_GetAeBv(IMPVI_MAIN,&real_bv);
    //   if (ret < 0) {
    //       IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_GetAeBv failed\n");
    //       // return -1;
    //   }
    //   IMP_LOG_INFO(TAG,"real_bv:%d\n",real_bv);
    //   //这里按照IQ bin中的BV list中最大值，作为切换AISP的阈值
    //   if (real_bv>bv_list.bv[BV_LUT_ITEM_NUM-1]) {
    //     //走关闭AISP流程，此处不写了，参考下面Disable AISP       
    //   }
    // }
    /*********************************End Use Bv sample ***************************************************/
}

int xcam_gekko_stopanddeinit(){
     //开关AISP可以在出流中进行,但请按照本文件中Enable AISP中与Disenable AISP中调用顺序
    /********************************* Disable AISP ***************************************************/
    char  NogekkoIQbin_path[1024] = {};
    sprintf(NogekkoIQbin_path,"/etc/sensor/%s-t41.bin",FIRST_SNESOR_NAME);
    int ret = gekko_stop(VI_MAIN);//对应gekko_start;切IQ bin与切FPS，跟gekko_stop走
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
          IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_SwitchBin error %s!\n",NogekkoIQbin_path);
          // return -1;
      }
      //切FPS,如果报错(EXP LIST MODE,SET FPS FAILED),打开以下两个函数的调用
      IMPISPAeExpListAttr aeexpattr; 
      ret = IMP_ISP_Tuning_GetAeExpList(IMPVI_MAIN,&aeexpattr);
      if(ret < 0) {
        IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_GetAeExpList error\n");
        return -1;
      }
      aeexpattr.mode = IMPISP_TUNING_OPS_MODE_DISABLE;
      ret = IMP_ISP_Tuning_SetAeExpList(IMPVI_MAIN,&aeexpattr);
      if(ret < 0) {
        IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_SetAeExpList error\n");
        return -1;
      }

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
}
#endif


IvsTask  ivsTask[]={
#ifdef FIREWORKSDET_ENABLE
    {.enter = xcam_FireworksDet_ivs, .ivsTaskNam = "fireworksDet" },
#else
    {.enter = NULL, .ivsTaskNam = "fireworksDet" },
#endif
#ifdef FACEDET_ENABLE
    {.enter = xcam_FaceDet_ivs, .ivsTaskNam = "faceDet" },
#else
    {.enter = NULL, .ivsTaskNam = "faceDet" },
#endif
#ifdef PERVEHPETDET_ENABLE
    {.enter = xcam_PersonvehiclepetDet_ivs, .ivsTaskNam = "pervonvehiclepetDet_ivs" },
#else
    {.enter = NULL, .ivsTaskNam = "pervonvehiclepetDet_ivs" },
#endif
#ifdef PLATEREC_ENABLE
    {.enter = xcam_PlateRec_ivs, .ivsTaskNam = "plateRec" },
#else
    {.enter = NULL, .ivsTaskNam = "plateRec" },
#endif
#ifdef MOVEDET_ENABLE
    {.enter = xcam_Move_ivs, .ivsTaskNam = "moveDetect" },
#else
    {.enter = NULL, .ivsTaskNam = "moveDetect" },
#endif
};

int  xcam_ivs_cancel_thread(int ivs_index){
    if((ivs_index >= 0 && ivs_index <=2) &&  ithreadstat[ivs_index] == 1) {
        pthread_cancel((ithreads[ivs_index]));
        ithreadstat[ivs_index] = 2;
    }
    return 0;
}

int  xcam_ivs_start_thread(int ivs_index){
    IvsTask *tIvsTask;
    if(ivs_index == 0) {
        tIvsTask =  &ivsTask[4];
    } else if(ivs_index == 1 ) {
        tIvsTask =  &ivsTask[ivs_index + 1];
    } else if(ivs_index == 2 ) {
        tIvsTask =  &ivsTask[ivs_index -1];
    }
    if(tIvsTask &&  (ithreadstat[ivs_index] == 0 ||ithreadstat[ivs_index] == 2)) {
        ithreadstat[ivs_index] = 1;
        tIvsTask->enter();
    }
    return 0;
}
// #include <stdio.h>
// #include <stdlib.h>
// #include <sys/wait.h> 
// int xcam_upgrade(){
//     // 要执行的升级脚本命令
//     // char script_path[BUFFER_SIZE];
//     // snprintf(script_path, sizeof(script_path), "sh /system/bin/read_firmware.sh %s%s", UPLOAD_DIR, "app.bin");
//     const char *script_path = "sh /system/bin/read_firmware.sh  /dev/app.bin";
//     FILE *fp = popen(script_path, "r");
//      struct timespec start, end;
//      clock_gettime(CLOCK_MONOTONIC, &start);
//     if (fp == NULL) {
//         perror("popen");
//         exit(EXIT_FAILURE);
//     }
//     char output[256];
//     while (fgets(output, sizeof(output), fp) != NULL) {
//         printf("Upgrade: %s", output);
//     }

//     // 关闭管道并获取脚本的退出状态
//     int status = pclose(fp);
//     if (status == -1) {
//         perror("pclose");
//         return -1;
//     } else {
//         // WIFEXITED: 如果为非零值表示进程正常退出
//         // WEXITSTATUS: 获取进程的退出状态
//         if (WIFEXITED(status)) {
//             printf("Script exited with status %d\n", WEXITSTATUS(status));
//         } else {
//             printf("Script did not exit normally\n");
//             return -1;
//         }
//     }
//      clock_gettime(CLOCK_MONOTONIC, &end);
//       double elapsed = (end.tv_sec - start.tv_sec) + 
//                      (end.tv_nsec - start.tv_nsec) / 1000000000.0;
    
//     printf("Elapsed time: %.9f seconds\n", elapsed);
//     return 0;
// }

// void xcam_set_day_or_night(int mode){
//        int ret = 0;
//     IMPISPBinAttr attr;
//     char name[50];
//     switch (mode)
//     {
//         case IMPISP_BIN_MODE_DAY:
//             snprintf(name, sizeof(name), "/etc/sensor/%s-PRJ007-%s.bin", FIRST_SNESOR_NAME, "day");
//             break;
//         case IMPISP_BIN_MODE_NIGHT:
//             snprintf(name, sizeof(name), "/etc/sensor/%s-PRJ007-%s.bin", FIRST_SNESOR_NAME, "night");
//             break;
//         default:
//             break;
//     }
//     attr.enable = IMPISP_TUNING_OPS_MODE_ENABLE;
//     attr.mode = IMPISP_BIN_MODE_NIGHT;
//     memcpy(attr.bname, name, sizeof(name));
//     printf ("IMP_ISP_Tuning_SwitchBin name=%s\n", name);
//     ret = IMP_ISP_Tuning_SwitchBin(IMPVI_MAIN, &attr);
//     if(ret){
//     	IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_SwitchBin error !\n");
//     	return -1;
//     }
// }
void xcam_extra_main(void){
    int i;

    pthread_mutex_init(&xcam_snapframe_mutex, NULL);
    for(i=0; i<ArrayLen(ivsTask); i++){
        IvsTask tIvsTask = ivsTask[i];
        int enable = xcam_conf_get_extraarg_status(tIvsTask.ivsTaskNam,ArrayLen(ivsTask));
        if( NULL != tIvsTask.enter && enable == 1) {
            tIvsTask.enter();
        }else if(NULL == tIvsTask.enter && enable == 1) {
            LOG_ERR(LOG_TAG,"err(%s,%d):%s is not open\n",__func__,__LINE__, tIvsTask.ivsTaskNam);
        }
    }
}