/* 
 *该文件做conf配置，供rcf服务端调用，进行json字符串解析
 * */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <signal.h>

#include "xcam_video.h"
#include "xcam_ptz.h"
#include "xcam_conf_process.h"
#include "xcam_cJson_process.h"
#include "xcam_thread.h"
#include "../../cJSON/cJSON.h"

#include "conf_process.h"
#include "../af/include/func-af.h"

#define CONF_SUCCESS 0
#define CONF_ERROR -1
#define CONF_GET_CONFGS 0
#define CONF_SET_CONFGS 1
#define SNAP_FRAME 2
extern video_config_info_t stream_attr;
int conf_ptz_get_motor_status(cJSON *root_r);
int conf_get_configlist(char *pstring,char *pstr_ret)
{
    char *data = pstring;
    char *json_str = NULL;
    int ret = 0;
    cJSON *root =NULL,*member = NULL,*ret_s = NULL,*root_err = NULL,*cfglist = NULL;

    if (data == NULL) {
        printf("err(%s,%d): recv data is NULL.\n", __func__, __LINE__);
        ret = CONF_ERROR;
    }

    root = cJSON_Parse(data);
    if (root == NULL) {
        printf("err(%s,%d):recv data format error,cJSON parse fail.\n",__func__,__LINE__);
        ret = CONF_ERROR;
    }
 #if 0
    char *json_recv = cJSON_Print(root);
    printf("GET%s-%d\n %s\n",__func__,__LINE__,json_recv);
    free(json_recv);
 #endif

    root_err = cJSON_CreateObject();
    cfglist = cJSON_GetObjectItem(root, "configlist");
    if(cfglist == NULL) {
        printf("[%s][%d]cJSON_GetObjectItem configlist is NUll\n",__func__,__LINE__);
        ret = CONF_ERROR;
    } else {
        member = cfglist->child;
    }
    if ((ret == CONF_SUCCESS) && (cfglist->child == NULL)) {
        ret |= xcam_json_get_video_isp_fps(root);
        ret |= xcam_json_get_video_bps(root);
        ret |= xcam_json_get_video_resolution(root);
        ret |= xcam_json_get_sys_ipconfig(root);
       // ret |= xcam_json_get_video_encode_mode(root);
        ret |= xcam_json_get_device_info(root);
        ret |= xcam_json_get_video_Iframe_interval(root);
        ret |= xcam_json_get_video_Rcmode(root);
        // ret |= xcam_json_get_web_http_port(root);
        ret |= xcam_json_get_channel_switch(root);
        ret |= xcam_json_get_rtsp_addr(root);
        ret |= conf_ptz_get_motor_status(root);
    } else {
        member = cfglist->child;
        while ((ret == CONF_SUCCESS) && member) {
            if (strcmp(member->string, "video.isp.fps") == 0) {
                ret |= xcam_json_get_video_isp_fps(root);
            } else if (strcmp(member->string, "video.bitrate") == 0) {
                ret |= xcam_json_get_video_bps(root);
            } else if (strcmp(member->string, "video.resolution") == 0) {
                ret |= xcam_json_get_video_resolution(root);
            } else if (strcmp(member->string,"video.enctype") == 0) {
			    ret |= xcam_json_get_video_encode_mode(root);
            } else if (strcmp(member->string,"video.qp") == 0) {
			    ret |= xcam_json_get_video_qp(root);
            } else if (strcmp(member->string,"video.ispcontrolInfo") == 0) {
                ret |= xcam_json_get_video_ispcontrolInfo(root);
            } else if (strcmp(member->string,"video.image_control") == 0) {
                ret |= xcam_json_get_video_image_control(root);
            } else if (strcmp(member->string,"network.information") == 0) {
                // ret |= xcam_json_get_sys_ipconfig(root);
            } else if (strcmp(member->string, "product.information") == 0) {
                // ret |= xcam_json_get_device_info(root);
            } else if (strcmp(member->string, "video.goplength") == 0) {
                ret |= xcam_json_get_video_Iframe_interval(root);
            } else if (strcmp(member->string, "video.isp.awbAttr") == 0) {
                ret |= xcam_json_get_isp_awbAttr(root);
            }else if (strcmp(member->string, "video.rcmode") == 0) {
                ret |= xcam_json_get_video_Rcmode(root);
            } else if (strcmp(member->string, "web.httpport") == 0) {
                // ret |= xcam_json_get_web_http_port(root);
            } else if (strcmp(member->string, "video.switch") == 0) {
                ret |= xcam_json_get_channel_switch(root);
            } else if (strcmp(member->string, "video.rtspaddr") == 0) {
                ret |= xcam_json_get_rtsp_addr(root);
            }else if (strcmp(member->string,"encoder.snap") == 0) {
                // ret |= xcam_json_get_video_encoder_snap(root);
            }else if (strcmp(member->string,"encoder.osd") == 0) {
                ret |= xcam_json_get_video_encoder_osd(root);
            } else if (strcmp(member->string,"conf_ptz_get_motorstatus") == 0) {
                ret |= conf_ptz_get_motor_status(root);
             //#if((defined GB28181) || (defined GB35114))
             } else if (strcmp(member->string, "sip.config") == 0) {
                 //ret |= xcam_json_get_sip_config(root);
             //#endif
            } else if (strcmp(member->string, "action") == 0) {
                member = member->next;
                continue;
            } else {
                printf("inf(%s,%d):xcam server don't support %s.\n",__func__,__LINE__,member->valuestring);
                ret = CONF_ERROR;
            }
            member = member->next;
        }
    }

    if (ret != CONF_SUCCESS) {
        ret_s = cJSON_CreateNumber(CONF_ERROR);
        cJSON_AddItemToObject(root_err, "return_value", ret_s);
        json_str = cJSON_Print(root_err);
    } else {
        ret_s = cJSON_CreateNumber(CONF_SUCCESS);
        cJSON_AddItemToObject(root, "return_value", ret_s);
        json_str = cJSON_Print(root);
    }
     //copy
    if(strlen(json_str)+1 <= MAX_RETSTR_LEN ){
        memcpy(pstr_ret,json_str,strlen(json_str) + 1);
    } else {
        printf("[%s][%d]Error!,json_str longer than mem,check please!\n",__func__,__LINE__);
        ret = CONF_ERROR;
    }

    printf("json_str len = %d %s \n",strlen(json_str), json_str);
    cJSON_Delete(root);
    cJSON_Delete(root_err);
    free(json_str);
    return ret;
}
//1set ptzmove
int conf_ptz_set_move_process(cJSON * root_r)
{
    if(NULL == root_r){
        printf("[%s][%d] root_r is null\n",__func__,__LINE__);
        return -1;
    }
    cJSON *root_sec = NULL, *item = NULL;
    int s32dx = 0,s32dy = 0 ,model = 0,ret = 0;
    root_sec = cJSON_GetObjectItem(root_r, "conf_ptz_set_move");
    if ( NULL == root_sec) {
        printf("err(%s,%d): conf_ptz_set_move get sec root err\n", __func__, __LINE__);
        return -1;
    }
    item = cJSON_GetObjectItem(root_sec,"dx");
    s32dx = item->valueint;
    item = cJSON_GetObjectItem(root_sec,"dy");
    s32dy = item->valueint;
    item = cJSON_GetObjectItem(root_sec,"model");
    model = item->valueint;
    printf("ptz_move_run,dx,dy=(%d,%d),model = %d\n",s32dx,s32dy,model);
#if defined T21
    ret = xcam_ptz_move(s32dx,s32dy,model);
    if(0 != ret){
        printf("[%s][%d]Error!,xcam_ptz_move fail\n",__func__,__LINE__);
        return ret;
    }
#endif
    return ret;
}

//2 set speed
int conf_ptz_set_speed_process(cJSON *root_r)
{
    if(NULL == root_r){
         printf("[%s][%d] root_r is null\n",__func__,__LINE__);
         return -1;
     }
     cJSON *root_sec = NULL, *item = NULL;
     int s32speed = 0 ,ret = 0;
     root_sec = cJSON_GetObjectItem(root_r, "conf_ptz_set_speed");
     if ( NULL == root_sec) {
         printf("err(%s,%d): conf_ptz_set_move get sec root err\n", __func__, __LINE__);
         return -1;
     }
     item = cJSON_GetObjectItem(root_sec,"speed");
     s32speed = item->valueint;

     printf("set speed = %d\n",s32speed);
#if defined T21
	 ret = xcam_ptz_set_speed(s32speed);
     if(0 != ret){
         printf("[%s][%d]Error!,xcam_ptz_move fail\n",__func__,__LINE__);
         return ret;
     }
#endif
     return ret;

}
//3 set af in
int conf_af_set_in_process(cJSON *root_r)
{
    if(NULL == root_r){
        printf("[%s][%d] root_r is null\n",__func__,__LINE__);
        return -1;
    }
    cJSON *root_sec = NULL, *item = NULL;
    int s32steps = 0 ,ret = 0;
    root_sec = cJSON_GetObjectItem(root_r, "conf_af_set_in");
    if ( NULL == root_sec) {
        printf("err(%s,%d): conf_af_set_in get sec root err\n", __func__, __LINE__);
        return -1;
    }

	item = cJSON_GetObjectItem(root_sec,"in_steps");
    s32steps = item->valueint;
    printf("zoom in s32steps = %d\n",s32steps);

#if defined T21
    func_af_zoom_in();
#endif
    return ret;
}

//4 set af out
int conf_af_set_out_process(cJSON *root_r)
{
    if(NULL == root_r){
        printf("[%s][%d] root_r is null\n",__func__,__LINE__);
        return -1;
    }
    cJSON *root_sec = NULL,*item = NULL;
    int s32steps = 0 ,ret = 0;
    root_sec = cJSON_GetObjectItem(root_r, "conf_af_set_out");
    if ( NULL == root_sec) {
        printf("err(%s,%d): conf_af_set_in get sec root err\n", __func__, __LINE__);
        return -1;
    }

	item = cJSON_GetObjectItem(root_sec,"out_steps");
    s32steps = item->valueint;
    printf("zoom out s32steps = %d\n",s32steps);

#if defined T21
    func_af_zoom_out();
#endif
    return ret;
}

//5 get motor status
#if 1
int conf_ptz_get_motor_status(cJSON *root_r)
{
    if(NULL == root_r){
        printf("[%s][%d] root_r is null\n",__func__,__LINE__);
        return -1;
    }

    int ret = 0;
	// cJSON *pnewconfig = NULL; for warning

    motor_ptz_message_t stmotor_message;
    memset(&stmotor_message,0x0,sizeof(motor_ptz_message_t));

#if defined T21
    ret = xcam_ptz_get_status(&stmotor_message);

    printf("xcurrent_steps=%d\n",stmotor_message.x);
    printf("ycurrent_steps=%d\n",stmotor_message.y);
    printf("cur_status=%d\n",stmotor_message.status);
    printf("cur_speed=%d\n",stmotor_message.speed);

    // 因为子对象挂载到root_r上，后续只需释放root_r的资源即可
    pnewconfig = cJSON_CreateObject();
    cJSON_AddItemToObject(pnewconfig,"ptz_x",cJSON_CreateNumber(stmotor_message.x));
    cJSON_AddItemToObject(pnewconfig,"ptz_y",cJSON_CreateNumber(stmotor_message.y));
    cJSON_AddItemToObject(pnewconfig,"ptz_runstatus",cJSON_CreateNumber(stmotor_message.status));
    cJSON_AddItemToObject(pnewconfig,"ptz_speed",cJSON_CreateNumber(stmotor_message.speed));
    cJSON_AddItemToObject(root_r,"conf_ptz_get_motorstatus",pnewconfig);
#endif

    return ret;
}
#endif

int conf_ptz_reset(cJSON *root_r)
{
    if(NULL == root_r){
        printf("[%s][%d] root_r is null\n",__func__,__LINE__);
        return -1;
    }

    int ret = 0;

    motor_ptz_message_t stmotor_message;
    memset(&stmotor_message,0x0,sizeof(motor_ptz_message_t));
#if defined T21
    ret = xcam_ptz_reset();
#endif

    return ret;
}

int conf_ptz_stop(cJSON *root_r)
{
    if(NULL == root_r){
        printf("[%s][%d] root_r is null\n",__func__,__LINE__);
        return -1;
    }

    int ret = 0;

    motor_ptz_message_t stmotor_message;
    memset(&stmotor_message,0x0,sizeof(motor_ptz_message_t));
#if defined T21
    ret = xcam_ptz_stop();
#endif

    return ret;
}

int conf_ptz_cruise(cJSON *root_r)
{
    if(NULL == root_r){
        printf("[%s][%d] root_r is null\n",__func__,__LINE__);
        return -1;
    }

    int ret = 0;

    motor_ptz_message_t stmotor_message;
    memset(&stmotor_message,0x0,sizeof(motor_ptz_message_t));

#if defined T21
    ret = xcam_ptz_cruise();
#endif

    return ret;
}

int conf_af_reset(cJSON *root_r)
{
    if(NULL == root_r){
        printf("[%s][%d] root_r is null\n",__func__,__LINE__);
        return -1;
    }

    int ret = 0;

    motor_ptz_message_t stmotor_message;
    memset(&stmotor_message,0x0,sizeof(motor_ptz_message_t));

#if defined T21
    func_af_focus_reset();
#endif

    return ret;
}

int conf_af_backward(cJSON *root_r)
{
    if(NULL == root_r){
        printf("[%s][%d] root_r is null\n",__func__,__LINE__);
        return -1;
    }
    cJSON *root_sec = NULL,*item = NULL;
    int s32steps = 0 ,ret = 0;
    root_sec = cJSON_GetObjectItem(root_r, "conf_af_backward");
    if ( NULL == root_sec) {
        printf("err(%s,%d): conf_af_move_backward get sec root err\n", __func__, __LINE__);
        return -1;
    }

	item = cJSON_GetObjectItem(root_sec,"finetuning_step");
    s32steps = item->valueint;
    printf("backward s32steps = %d\n",s32steps);

#if defined T21
	func_af_focus_add(s32steps);
#endif
    return ret;
}

int conf_af_forward(cJSON *root_r)
{
    if(NULL == root_r){
        printf("[%s][%d] root_r is null\n",__func__,__LINE__);
        return -1;
    }
    cJSON *root_sec = NULL,*item = NULL;
    int s32steps = 0 ,ret = 0;
    root_sec = cJSON_GetObjectItem(root_r, "conf_af_forward");
    if ( NULL == root_sec) {
        printf("err(%s,%d): conf_af_move_forward get sec root err\n", __func__, __LINE__);
        return -1;
    }

	item = cJSON_GetObjectItem(root_sec,"finetuning_step");
    s32steps = item->valueint;
    printf("forward s32steps = %d\n",s32steps);

#if defined T21
	func_af_focus_reduce(s32steps);
#endif
    return ret;
}

int conf_ptz_xvsy(cJSON *root_r)
{
    if(NULL == root_r){
        printf("[%s][%d] root_r is null\n",__func__,__LINE__);
        return -1;
    }
    cJSON *root_sec = NULL,*item = NULL;
    int hmotor2vmotor = 0 ,ret = 0;
    root_sec = cJSON_GetObjectItem(root_r, "conf_ptz_xvsy");
    if ( NULL == root_sec) {
        printf("err(%s,%d): conf_af_move_backward get sec root err\n", __func__, __LINE__);
        return -1;
    }

	item = cJSON_GetObjectItem(root_sec,"hmotor2vmotor");
    hmotor2vmotor = item->valueint;
    printf("hmotor2vmotor = %d\n",hmotor2vmotor);

#if defined T21
	xcam_ptz_xvsy(hmotor2vmotor);
#endif
    return ret;
}

int conf_set_configlist(char *pstring,char *pstr_ret)
{
    char *data = pstring;
    char *json_str = NULL;
    int ret = CONF_SUCCESS;
    cJSON *root_s =NULL,*ret_s = NULL,*root_r = NULL,*member = NULL;

    if (data == NULL) {
        printf("err(%s,%d): recv data is NULL.\n", __func__, __LINE__);
        ret = CONF_ERROR ;
    }

    root_s = cJSON_CreateObject();
    root_r = cJSON_Parse(data);

#if 0
    char *json_recv = cJSON_Print(root_r);
    printf("SET %s-%d\n %s\n",__func__,__LINE__,json_recv);
    free(json_recv);
#endif
    if (root_r == NULL) {
     printf("err(%s,%d):recv data format error,cJSON parse fail.\n",__func__,__LINE__);
     ret = CONF_ERROR;
    }

    member = root_r->child;
    if (member == NULL) {
        printf("error(%s,%d),root no anything member.\n",__func__,__LINE__);
        ret = CONF_ERROR;
    }

    while ((ret == CONF_SUCCESS) && member) {
        if (strcmp(member->string, "video.isp.fps") == 0) {
			ret |= xcam_json_set_video_isp_fps(root_r);
               printf("%s %d ret=%d\n", __func__, __LINE__, ret);
        } else if (strcmp(member->string, "network.information") == 0) {
            ret |= xcam_json_set_sys_ipconfig(root_r);
        } else if (strcmp(member->string, "video.resolution") == 0) {
            ret |= xcam_json_set_video_resolution(root_r);
               printf("%s %d ret=%d\n", __func__, __LINE__, ret);
        } else if (strcmp(member->string, "video.enctype") == 0) {
            // ret |= xcam_json_set_video_encode_mode(root_r);
            //  printf("%s %d ret=%d\n", __func__, __LINE__, ret);
        } else if (strcmp(member->string,"video.ispcontrolInfo") == 0) {
            ret |= xcam_json_set_video_ispcontrolInfo(root_r);
        } else if (strcmp(member->string,"video.image_control") == 0) {
            ret |= xcam_json_set_video_image_control(root_r);
        } else if (strcmp(member->string, "video.goplength") == 0) {
            ret |= xcam_json_set_video_Iframe_interval(root_r);
               printf("%s %d ret=%d\n", __func__, __LINE__, ret);
        } else if (strcmp(member->string, "video.rcmode") == 0){
            ret |= xcam_json_set_video_Rcmode(root_r);
               printf("%s %d ret=%d\n", __func__, __LINE__, ret);
        } else if (strcmp(member->string, "web.httpport") == 0) {
            ret |= xcam_json_set_web_http_port(root_r);
        } else if (strcmp(member->string, "video.switch") == 0) {
            ret |= xcam_json_set_channel_switch(root_r);
        } else if (strcmp(member->string, "video.qp") == 0){
			ret |= xcam_json_set_video_qp(root_r);
		} else if (strcmp(member->string, "video.bitrate") == 0) {
            ret |= xcam_json_set_video_bps(root_r);
               printf("%s %d ret=%d\n", __func__, __LINE__, ret);
        } else if (strcmp(member->string, "encoder.snap") == 0) {
            ret |= xcam_json_set_encoder_snap(root_r);
        } else if (strcmp(member->string, "video.isp.awbAttr") == 0) {
            ret |= xcam_json_set_isp_awbAttr(root_r);
        } else if (strcmp(member->string, "encoder.osd") == 0) {
            ret |= xcam_json_set_encoder_osd(root_r);
        } else if (strcmp(member->string,"conf_ptz_set_move") == 0) {
            ret |= conf_ptz_set_move_process(root_r);
        } else if (strcmp(member->string,"conf_ptz_set_speed") == 0) {
            ret |= conf_ptz_set_speed_process(root_r);
        } else if (strcmp(member->string,"conf_af_set_in") == 0) {
            ret |= conf_af_set_in_process(root_r);
        } else if (strcmp(member->string,"conf_af_set_out") == 0) {
            ret |= conf_af_set_out_process(root_r);
        } else if (strcmp(member->string,"conf_ptz_reset") == 0) {
            ret |= conf_ptz_reset(root_r);
        } else if (strcmp(member->string,"conf_ptz_cruise") == 0) {
            ret |= conf_ptz_cruise(root_r);
        } else if (strcmp(member->string,"conf_af_backward") == 0) {
            ret |= conf_af_backward(root_r);
        } else if (strcmp(member->string,"conf_af_forward") == 0) {
            ret |= conf_af_forward(root_r);
        } else if (strcmp(member->string,"conf_af_reset") == 0) {
            ret |= conf_af_reset(root_r);
        } else if (strcmp(member->string,"conf_ptz_stop") == 0) {
            ret |= conf_ptz_stop(root_r);
        } else if (strcmp(member->string,"conf_ptz_xvsy") == 0) {
            ret |= conf_ptz_xvsy(root_r);
        } else if ((strcmp (member->string, "return_value") == 0) || (strcmp(member->string, "configlist") == 0)) {
            member = member->next;
            continue;
        } else {
            printf("inf(%s,%d):xcam server don't support %s.\n",__func__,__LINE__,member->valuestring);
            ret = CONF_ERROR;
        }
            member = member->next;
        }

        if (ret != CONF_SUCCESS) {
            ret_s = cJSON_CreateNumber(CONF_ERROR);
            cJSON_AddItemToObject(root_s, "return_value", ret_s);
        } else {
            ret_s = cJSON_CreateNumber(CONF_SUCCESS);
            cJSON_AddItemToObject(root_s, "return_value", ret_s);
     }
    printf("%s %d ret=%d\n", __func__, __LINE__, ret);
    //copy
    json_str = cJSON_Print(root_s);
    if(strlen(json_str)+1 <= MAX_RETSTR_LEN ){
        memcpy(pstr_ret,json_str,strlen(json_str) + 1);
    } else {
        printf("[%s][%d]Error!,json_str longer than mem,check please!\n",__func__,__LINE__);
    }

    cJSON_Delete(root_r);
    cJSON_Delete(root_s);
    free(json_str);
     return ret;

}
int conf_get_cmdaction(const char *pstring)
{
    const char *data = pstring;
    int ret = CONF_SUCCESS;
    int action = -1;
    cJSON *root_r = NULL,*cfglist = NULL,*pAction = NULL;

    root_r = cJSON_Parse(data);
    if (root_r == NULL) {
        printf("err(%s,%d):recv data format error,cJSON parse fail.\n",__func__,__LINE__);
        ret = CONF_ERROR;
		return ret;
    }

    cfglist = cJSON_GetObjectItem(root_r, "configlist");
    if (cfglist == NULL) {
        ret = CONF_ERROR;
    }

    pAction = cJSON_GetObjectItem(cfglist, "action");
    if (pAction == NULL) {
        ret = CONF_ERROR;
		return ret;
    } else {
        action = pAction->valueint;
    }

    cJSON_Delete(root_r);

    return action;
}

int conf_process(char* pstring,char *pstr_ret)
{
    if(NULL == pstring || NULL == pstr_ret ){
        printf("[%s][%d]:err,pstring is NULL or pstr_ret is NULL\n",__func__, __LINE__);
        return -1;
    }

    int ret = CONF_SUCCESS;
    int action = -1;

    action = conf_get_cmdaction(pstring);
	//printf("action = %d\n",action);
    switch (action) {
         case CONF_GET_CONFGS :
             ret = conf_get_configlist(pstring,pstr_ret);
             if (ret < 0) {
                 printf("[%s][%d]conf_get_configlist err\n",__func__,__LINE__);
             }
             break;
         case CONF_SET_CONFGS:
             ret = conf_set_configlist(pstring,pstr_ret);
             if (ret < 0) {
                 printf("[%s][%d]conf_set_configlist err\n",__func__,__LINE__);
             }
             break;
         default :
             printf("error(%s,%d),the action don't support.\n",__func__,__LINE__);
             ret = CONF_ERROR;
             break;
     }

     return ret;
}

void* cJson_rcf_start(void* arg)
{
    int ret = CONF_SUCCESS;
    ret = rcf_server_start();
    if(CONF_SUCCESS != ret ){
        printf("rcf_server_start run err\n");
        return NULL;
    }

    while(1) {
        sleep(1);
    }

    return NULL;
}

void rcf_process_init(void)
{
    xcam_thread_create("cjson_rcf_main", cJson_rcf_start, NULL);
    return;
}

