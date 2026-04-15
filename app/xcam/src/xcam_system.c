#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/reboot.h>
#include <dirent.h>
#include "xcam_general.h"
#include "xcam_log.h"
#include <imp/imp_isp.h>
#include "func-af.h"

#define LOG_TAG "xcam_system"
#define FACTORY_CONFIGS "/system/conf/factory_configs"
#define RUNING_CONFIHS	"/system/etc"
#define WEB_BOA "/system/bin/boa"

/*重启设备*/
void xcam_system_reboot()
{
	sleep(10);
	sync();
	reboot(RB_AUTOBOOT);
}

/*重启进程暂时做不到，没有守护进程可以做到将xcam拉起来*/
void xcam_system_reset_factory_settings()
{
	char cmd[128] = {0};
	DIR *dir = NULL;

	/*这里只判断了出厂配置的文件夹存不存在，没有判断出厂的配置文件是不是存在的*/
	dir = opendir(FACTORY_CONFIGS);
	if (dir == NULL) {
		LOG_ERR(LOG_TAG,"error(%s,%d),factory configs does not exist.\n",__func__,__LINE__);
		return;
	}

	/*删除用户配置，将用户配置删除*/
	system("rm /system/etc/video.json /system/etc/network.json");

	/*将出厂配置文件拷贝到启动文件夹中，如果出厂配置文件不存在则直接按照*/
	sprintf(cmd, "cp %s/* %s",FACTORY_CONFIGS,RUNING_CONFIHS);
	system(cmd);

	/*目前是按照重启设备,重启进程目前没有去初始化函数*/
	xcam_system_reboot();

	return;
}

/*后期开发建议每个进程都有一个去初始化的函数,结束进程最后不要kill -9 太暴力了*/
void xcam_system_stop_boa()
{
	char cmd[128] = {0};
	FILE *fp = NULL;
	char cpid[10] = {0};
	int i = 0;

	/*这个接口需要测试不存在的情况*/
	sprintf(cmd," ps | grep -v grep | grep boa | awk '{print $1 }' ");

	fp = popen(cmd, "r");
	if ( fp == NULL ) {
		LOG_ERR(LOG_TAG,"error(%s,%d),popen fail,get boa pid fail\n",__func__,__FILE__);
		return;
	}

	fread(cpid,1,9,fp);
	pclose(fp);
	char * temp = cpid;
	for (i = 0; i < 9;i++ ) {
		if (*temp == '\0' )
			break;
		if (*temp == '\n')
			*temp = '\0';
		temp ++;
	}

	int pid = atoi(cpid);
	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd,"kill -9 %d",pid);
	system(cmd);

	return;
}

void xcam_system_start_boa()
{
	//可以提前判断一下文件存不存在，这里就先不判断了
	system("/system/bin/boa &");
	return;
}

int xcam_func_af_get_hist(struct af_definition *afDefin)
{
	int ret = -1;
#ifdef T21
	IMPISPAFHist af_hist;
	ret = IMP_ISP_Tuning_GetAfHist(&af_hist);
	if(ret < 0) {
		printf( "%s(%d):IMP_ISP_Tuning_GetAfHist failed\n", __func__, __LINE__);
		return -1;
	}
	afDefin->frameNum   = (int)af_hist.af_frame_num;
	afDefin->definValue = af_hist.af_stat.af_metrics_alt;
#endif
	return ret;
}

