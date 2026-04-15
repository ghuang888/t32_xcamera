/*
	sample-Snap-Raw.c

	Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
	The specific instructions for all API calls in this file can be found in the header file of the proj/sdk-lv3/include/api/cn/imp/

	This sample demonstrates the operation of grabbing raw data.
*/

#include "sample-common.h"

#define TAG "sample-Snap-Raw"

#define BYPASS_ENABLE	1
#define BYPASS_DISABLE	0

extern struct chn_conf chn[];

int sensor_bypass[3] = { 0, 0, 1 };

int main(int argc, char *argv[])
{
	int ret = 0;

	/* Step.1 System init */
	ret = sample_system_init();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "System init failed\n");
		return -1;
	}

	chn[0].fs_chn_attr.pixFmt = PIX_FMT_RAW;
	chn[0].fs_chn_attr.nrVBs = 2;

	IMPISPOpsMode enable;
	enable = IMPISP_OPS_MODE_ENABLE;
	ret = IMP_ISP_SetISPBypass(IMPVI_MAIN, &enable);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_SetISPBpass failed\n");
		return -1;
	}

	/* Step.2 FrameSource init */
	ret = sample_framesource_init();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "FrameSource init failed\n");
		return -1;
	}

	/* Step.3 Stream On */
	ret = sample_framesource_streamon();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "FrameSource StreamOn failed\n");
		return -1;
	}

	/* Step.4 Get frame */
	ret = sample_get_frame();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "get frame failed\n");
		return -1;
	}

	/* Step.5 Stream Off */
	enable = IMPISP_OPS_MODE_DISABLE;
	ret = IMP_ISP_SetISPBypass(IMPVI_MAIN, &enable);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_SetISPBpass failed\n");
		return -1;
	}

	ret = sample_framesource_streamoff();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "FrameSource StreamOff failed\n");
		return -1;
	}

	/* Step.6 FrameSource exit */
	ret = sample_framesource_exit();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "FrameSource exit failed\n");
		return -1;
	}

	/* Step.7 System exit */
	ret = sample_system_exit();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "System exit failed\n");
		return -1;
	}

	return 0;
}
