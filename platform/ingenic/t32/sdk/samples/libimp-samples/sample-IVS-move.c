/*
	sample-IVS-move.c

	Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
	The specific instructions for all API calls in this file can be found in the header file of the proj/sdk-lv3/include/api/cn/imp/

	This sample demonstrates the mobile algorithm.
*/

#include <imp/imp_ivs_move.h>

#include "sample-common.h"

#define TAG "sample-IVS-move"

extern struct chn_conf chn[];
extern int direct_switch;

static int sample_ivs_move_init(int grp_num);
static int sample_ivs_move_exit(int grp_num);
static int sample_ivs_move_start(int grp_num, int chn_num, IMPIVSInterface **interface);
static int sample_ivs_move_stop(int chn_num, IMPIVSInterface *interface);
static int sample_ivs_move_get_result_start(int chn_num, pthread_t *ptid);
static int sample_ivs_move_get_result_stop(pthread_t tid);

int main(int argc, char *argv[])
{
	int i = 0;
	int ret = 0;
	pthread_t ivs_tid;
	IMPIVSInterface *inteface = NULL;

	direct_switch = 0;

	chn[0].enable = 0;
	chn[1].enable = 1;
	chn[2].enable = 0;

	/* Step.1 System init */
	ret = sample_system_init();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "System init failed\n");
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
				IMP_LOG_ERR(TAG, "Encoder CreateGroup(%d) failed\n", chn[i].index);
				return -1;
			}
		}
	}

	ret = sample_video_init();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "Video init failed\n");
		return -1;
	}

	/* Step.4 ivs init */
	ret = sample_ivs_move_init(0);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "ivs move init failed\n");
		return -1;
	}

	/* Step.5 Bind */
	/**
	 * FS.0 ----------------> Encoder.0(Main stream)
	 * FS.1 ----------------> IVS
	 */
	IMPCell ivs = { DEV_ID_IVS, 0, 0 };

	for (i = 0; i < FS_CHN_NUM; i++) {
		if (IVS_CHN_ID == chn[i].index) {
			if (chn[i].enable) {
				ret = IMP_System_Bind(&chn[i].framesource_chn, &ivs);
				if (ret < 0) {
					IMP_LOG_ERR(TAG, "Bind framesource%d and ivs%d failed\n", chn[i].framesource_chn.groupID, ivs.groupID);
					return -1;
				}
			}
		} else {
			if (chn[i].enable) {
				ret = IMP_System_Bind(&chn[i].framesource_chn, &chn[i].imp_encoder);
				if (ret < 0) {
					IMP_LOG_ERR(TAG, "Bind framesource%d and encoder%d failed\n", chn[i].framesource_chn.groupID, chn[i].imp_encoder.groupID);
					return -1;
				}
			}
		}
	}

	/* Step.6 Stream On */
	ret = sample_framesource_streamon();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "FrameSource StreamOn failed\n");
		return -1;
	}

	/* Step.7 ivs move start */
	ret = sample_ivs_move_start(0, IVS_CHN_ID, &inteface);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "ivs move start failed\n");
		return -1;
	}

	/* Step.8 start get ivs move result */
	ret = sample_ivs_move_get_result_start(IVS_CHN_ID, &ivs_tid);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "start get ivs move result failed\n");
		return -1;
	}

	/* Step.9 Get video stream */
	ret = sample_start_get_video_stream();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "Get video stream failed\n");
		return -1;
	}

	sample_stop_get_video_stream();

	/* Step.10 stop get ivs move result */
	ret = sample_ivs_move_get_result_stop(ivs_tid);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "stop get ivs move result failed\n");
		return -1;
	}

	/* Step.11 ivs move stop */
	ret = sample_ivs_move_stop(IVS_CHN_ID, inteface);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "ivs move stop failed\n");
		return -1;
	}

	/* Step.12 Stream Off */
	ret = sample_framesource_streamoff();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "FrameSource StreamOff failed\n");
		return -1;
	}

	/* Step.13 UnBind */
	for (i = 0; i < FS_CHN_NUM; i++) {
		if (chn[i].enable) {
			if (IVS_CHN_ID == chn[i].index) {
				ret = IMP_System_UnBind(&chn[i].framesource_chn, &ivs);
				if (ret < 0) {
					IMP_LOG_ERR(TAG, "UnBind framesource%d and ivs%d failed\n", chn[i].framesource_chn.groupID, ivs.groupID);
					return -1;
				}
			} else {
				ret = IMP_System_UnBind(&chn[i].framesource_chn, &chn[i].imp_encoder);
				if (ret < 0) {
					IMP_LOG_ERR(TAG, "UnBind framesource%d and encoder%d failed\n", chn[i].framesource_chn.groupID, chn[i].imp_encoder.groupID);
					return -1;
				}
			}
		}
	}

	/* Step.14 ivs exit */
	ret = sample_ivs_move_exit(0);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "ivs move exit failed\n");
		return -1;
	}

	/* Step.15 Encoder exit */
	ret = sample_video_exit();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "Video exit failed\n");
		return -1;
	}

	for (i = 0; i < FS_CHN_NUM; i++) {
		if (chn[i].enable) {
			ret = IMP_Encoder_DestroyGroup(chn[i].index);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "Encoder DestroyGroup(%d) failed\n", chn[i].index);
				return -1;
			}
		}
	}

	/* Step.16 FrameSource exit */
	ret = sample_framesource_exit();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "FrameSource exit failed\n");
		return -1;
	}

	/* Step.17 System exit */
	ret = sample_system_exit();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "System exit failed\n");
		return -1;
	}

	return 0;
}

static int sample_ivs_move_init(int grp_num)
{
	int ret = 0;

	ret = IMP_IVS_CreateGroup(grp_num);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_IVS_CreateGroup(%d) failed\n", grp_num);
		return -1;
	}

	return 0;
}

static int sample_ivs_move_exit(int grp_num)
{
	int ret = 0;

	ret = IMP_IVS_DestroyGroup(grp_num);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_IVS_DestroyGroup(%d) failed\n", grp_num);
		return ret;
	}

	return 0;
}

static int sample_ivs_move_start(int grp_num, int chn_num, IMPIVSInterface **interface)
{
	int i = 0, j = 0;
	int ret = 0;
	IMP_IVS_MoveParam param;

	memset(&param, 0, sizeof(IMP_IVS_MoveParam));
	param.skipFrameCnt = 1;
	param.frameInfo.width = FIRST_SENSOR_WIDTH_SECOND;
	param.frameInfo.height = FIRST_SENSOR_HEIGHT_SECOND;
	param.roiRectCnt = 1;
	for(i=0; i<param.roiRectCnt; i++){
		param.sense[i] = 4;
	}
	/* printf("param.sense=%d, param.skipFrameCnt=%d, param.frameInfo.width=%d, param.frameInfo.height=%d\n", param.sense, param.skipFrameCnt, param.frameInfo.width, param.frameInfo.height); */
	for (j = 0; j < 2; j++) {
		for (i = 0; i < 2; i++) {
			if((i==0)&&(j==0)){
				param.roiRect[j * 2 + i].p0.x = i * param.frameInfo.width /* / 2 */;
				param.roiRect[j * 2 + i].p0.y = j * param.frameInfo.height /* / 2 */;
				param.roiRect[j * 2 + i].p1.x = (i + 1) * param.frameInfo.width /* / 2 */ - 1;
				param.roiRect[j * 2 + i].p1.y = (j + 1) * param.frameInfo.height /* / 2 */ - 1;
				printf("(%d,%d) = ((%d,%d)-(%d,%d))\n", i, j, param.roiRect[j * 2 + i].p0.x, param.roiRect[j * 2 + i].p0.y,param.roiRect[j * 2 + i].p1.x, param.roiRect[j * 2 + i].p1.y);
			}
			else
			{
				param.roiRect[j * 2 + i].p0.x = param.roiRect[0].p0.x;
				param.roiRect[j * 2 + i].p0.y = param.roiRect[0].p0.y;
				param.roiRect[j * 2 + i].p1.x = param.roiRect[0].p1.x;;
				param.roiRect[j * 2 + i].p1.y = param.roiRect[0].p1.y;;
				printf("(%d,%d) = ((%d,%d)-(%d,%d))\n", i, j, param.roiRect[j * 2 + i].p0.x, param.roiRect[j * 2 + i].p0.y,param.roiRect[j * 2 + i].p1.x, param.roiRect[j * 2 + i].p1.y);
			}
		}
	}

	*interface = IMP_IVS_CreateMoveInterface(&param);
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

static int sample_ivs_move_stop(int chn_num, IMPIVSInterface *interface)
{
	int ret = 0;

	ret = IMP_IVS_StopRecvPic(chn_num);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_IVS_StopRecvPic(%d) failed\n", chn_num);
		return ret;
	}

	sleep(1);

	ret = IMP_IVS_UnRegisterChn(chn_num);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_IVS_UnRegisterChn(%d) failed\n", chn_num);
		return ret;
	}

	ret = IMP_IVS_DestroyChn(chn_num);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_IVS_DestroyChn(%d) failed\n", chn_num);
		return ret;
	}

	IMP_IVS_DestroyMoveInterface(interface);

	return 0;
}

#if 0
static int sample_ivs_set_sense(int chn_num, int sensor)
{
	int i = 0;
	int ret = 0;
	IMP_IVS_MoveParam param;

	ret = IMP_IVS_GetParam(chn_num, &param);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_IVS_GetParam(%d) failed\n", chn_num);
		return -1;
	}
	for( i = 0 ; i < param.roiRectCnt ; i++){
		param.sense[i] = sensor;
	}
	ret = IMP_IVS_SetParam(chn_num, &param);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_IVS_SetParam(%d) failed\n", chn_num);
		return -1;
	}

	return 0;
}
#endif

static void *sample_ivs_move_get_result_process(void *arg)
{
	int i = 0;
	int ret = 0;
	int chn_num = (int)arg;
	IMP_IVS_MoveOutput *result = NULL;

	for (i = 0; i < NR_FRAMES_TO_SAVE; i++) {
		ret = IMP_IVS_PollingResult(chn_num, IMP_IVS_DEFAULT_TIMEOUTMS);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "IMP_IVS_PollingResult(%d, %d) failed\n", chn_num, IMP_IVS_DEFAULT_TIMEOUTMS);
			return NULL;
		}
		ret = IMP_IVS_GetResult(chn_num, (void **)&result);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "IMP_IVS_GetResult(%d) failed\n", chn_num);
			return NULL;
		}
		//IMP_LOG_INO(TAG, "frame[%d], result->retRoi(%d,%d,%d,%d)\n", i, result->retRoi[0], result->retRoi[1], result->retRoi[2], result->retRoi[3]);
		printf("frame[%d], result->retRoi(%d,%d,%d,%d)\n", i, result->retRoi[0], result->retRoi[1], result->retRoi[2], result->retRoi[3]);
		ret = IMP_IVS_ReleaseResult(chn_num, (void *)result);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "IMP_IVS_ReleaseResult(%d) failed\n", chn_num);
			return NULL;
		}
#if 0
		if (i % 20 == 0) {
			ret = sample_ivs_set_sense(chn_num, i % 5);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "sample_ivs_set_sense(%d, %d) failed\n", chn_num, i % 5);
				return NULL;
			}
		}
#endif
	}

	return NULL;
}

static int sample_ivs_move_get_result_start(int chn_num, pthread_t *ptid)
{
	if (pthread_create(ptid, NULL, sample_ivs_move_get_result_process, (void *)chn_num) < 0) {
		IMP_LOG_ERR(TAG, "create sample_ivs_move_get_result_process failed\n");
		return -1;
	}

	return 0;
}

static int sample_ivs_move_get_result_stop(pthread_t tid)
{
	pthread_join(tid, NULL);

	return 0;
}
