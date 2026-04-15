#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <imp/imp_log.h>

#include <imp/imp_common.h>
#include <imp/imp_system.h>
#include <imp/imp_utils.h>
#include <imp/imp_framesource.h>
#include <imp/imp_encoder.h>
#include <imp/imp_isp.h>
#include "xcam_motor.h"
#define TAG 						"AF"
#define MAX(a,b,c)  (a>b?(a>c?a:c):(b>c?b:c))
//#define AUTO_FOCUS_THREAD
enum {
	MOVE_FORWARD,
	MOVE_BACKWARD
};

static int af_fd = -1;						/**<设备驱动文件描述符*/
static int motor_speed = 500;				/**<电机运动的速度*/
static pthread_t afTid;					/**<清晰度值监控线程ID*/
static pthread_mutex_t afMonitorMutex;		/**<清晰度值监控线程锁*/
static pthread_cond_t  afMonitorCond;		/**<清晰度值监控条件变量*/
static int pic_definition_evaluetion;		/**<清晰度评价值*/
static enum motor_status motorStatus;			/**<电机状态*/
static int focus_max_steps = 3011;			/**<变焦最大步数*/
static int zoom_max_steps = 2342;			/**<变倍最大步数*/
static int zoom_cur_steps = 0;				/**<当前变倍步数*/
static int focus_cur_steps = 0;			/**<当前变焦步数*/
static int zoom_unit_steps = 121;			/**<缩放最小倍数移动的步数*/
static int zoom_ratio = 0;					/**<缩放的倍数*/
static float trigger_ratio = 0.2;			/**<触发自动对焦的系数*/

#define ZOOM_MAX_RATIO 21 //zoom:0.1x
#define ZOOM_MIN_RATIO 0 //zoom:0.1x

typedef struct {
	int focusPos;			/*对焦位置*/
	int focusMotorSpeed;	/*对焦电机转速*/
	int zoomMotorSpeed;		/*变倍电机转速*/
	int irqSpeed;			/*中断速度*/
}ZoomFocusAttr;

#if 0
static ZoomFocusAttr ZoomFocusTable[ZOOM_MAX_RATIO] = {
	[0] =  {408,3,1,250},
	[1] =  {806,4,1,250},
	[2] =  {1176,2,1,500},
	[3] =  {1446,2,1,500},
	[4] =  {1710,2,1,500},
	[5] =  {1923,2,1,500},
	[6] =  {2110,2,1,500},
	[7] =  {2267,1,1,500},
	[8] =  {2410,1,1,500},
	[9] =  {2508,1,1,500},
	[10] = {2597,1,1,500},
	[11] = {2677,1,1,500},
	[12] = {2741,1,1,500},
	[13] = {2785,1,1,500},
	[14] = {2808,1,1,500},
	[15] = {2840,1,1,500},
	[16] = {2865,1,1,500},
	[17] = {2890,1,1,500},
	[18] = {2892,1,1,500},
	[19] = {2896,1,1,500},
	[20] = {2898,1,1,500},
};
#else
static ZoomFocusAttr ZoomFocusTable[ZOOM_MAX_RATIO] = {
	[0] =  {427,2,1,250},
	[1] =  {676,2,1,250},
	[2] =  {1076,2,1,500},
	[3] =  {1446,2,1,500},
	[4] =  {1710,2,1,500},
	[5] =  {1923,2,1,500},
	[6] =  {2110,2,1,500},
	[7] =  {2267,1,1,500},
	[8] =  {2410,1,1,500},
	[9] =  {2508,1,1,500},
	[10] = {2597,1,1,500},
	[11] = {2677,1,1,500},
	[12] = {2741,1,1,500},
	[13] = {2785,1,1,500},
	[14] = {2808,1,1,500},
	[15] = {2840,1,1,500},
	[16] = {2865,1,1,500},
	[17] = {2890,1,1,500},
	[18] = {2892,1,1,500},
	[19] = {2896,1,1,500},
	[20] = {2898,1,1,500},
};
#endif
#if 0
static int zoom_ratio_steps[ZOOM_MAX_RATIO] =			    {0,121,242,343,464,585,706,827,948,1069,1190,1311,1432,1553,1674,1795,1916,2036,2157,2278,2342};
static int focus_steps_INF[ZOOM_MAX_RATIO] =				{447,885,1230,1485,1738,1951,2128,2275,2398,2501,2585,2654,2710,2755,2789,2815,2833,2844,2848,2847,2844};
static int focus_steps_centimeter_1000[ZOOM_MAX_RATIO] =  {445,883,1228,1482,1736,1949,2126,2272,2396,2499,2583,2652,2708,2753,2788,2813,2831,2842,2846,2845,2842};
static int focus_steps_centimeter_500[ZOOM_MAX_RATIO] =   {443,881,1226,1481,1734,1946,2124,2271,2394,2497,2581,2650,2706,2751,2785,2811,2829,2840,2844,2843,2840};
static int focus_steps_centimeter_300[ZOOM_MAX_RATIO] =   {440,878,1223,1478,1731,1944,2121,2268,2391,2494,2578,2648,2704,2748,2783,2809,2826,2837,2842,2841,2838};
static int focus_steps_centimeter_100[ZOOM_MAX_RATIO] =    {427,865,1211,1465,1719,1931,2109,2255,2379,2481,2566,2635,2691,2736,2770,2796,2814,2824,2829,2828,2825};
static int focus_steps_centimeter_80[ZOOM_MAX_RATIO] =    {423,861,1206,1461,1714,1927,2104,2251,2374,2477,2561,2630,2687,2731,2766,2792,2809,2820,2824,2823,2821};
static int focus_steps_centimeter_50[ZOOM_MAX_RATIO] =	{408,846,1192,1446,1700,1913,2090,2237,2360,2463,2547,2617,2673,2717,2752,2778,2795,2806,2811,2810,2807};
#endif


static int get_af_metric()
{
	int i;
	int max,min;
	int ret;
	int num[]={0,0,0,0,0};
	int metrics = 0;
	IMPISPAFHist af_hist;
	IMPISPWaitFrameAttr attr;
	attr.timeout = 100;
	//attr.cnt = 0;
#if 0
	ret = IMP_ISP_Tuning_WaitFrame(&attr);/**< 等待一帧*/
	if(ret){
		printf("IMP_ISP_Tuning_WaitFrame error!\n");
		return NULL;
	}
#endif
	for(i = 0; i < 5 ; i++){
		ret = IMP_ISP_Tuning_GetAfHist(&af_hist);
		if(ret < 0)
		{
			printf("%s(%d):IMP_ISP_Tuning_GetAfHist failed\n", __func__, __LINE__);
			return -1;
		}
	//	num[i] = (af_hist.af_stat.af_metrics/100);//主统计值
		num[i] = (af_hist.af_stat.af_metrics_alt/100);//副统计值
	}

	max = num[0];
	min = num[0];
	for(i = 0; i < 5; i++){
		if(max < num[i])
			max = num[i];
		if(min > num[i])
			min = num[i];
		metrics +=num[i];
	}
	metrics = (metrics - max - min)/3;
	printf("metrics  =  %d\n",metrics);
	return metrics;
}

static int motor_move_step(int steps_x, int steps_y,int unit_steps_x, int unit_steps_y, int speed)
{
	int ret = -1;
	int delay_cnt = 0;
	int zsteps = 0;
	int fsteps = 0;
	motor_steps_t steps;
	memset(&steps, 0, sizeof(motor_steps_t));

	if(unit_steps_x > 5 || unit_steps_y > 5){
		printf("motor one interrupt %d steps is too big\n");
		return -1;
	}

	//printf("steps_x=%d steps_y=%d unit_steps_x=%d unit_steps_y=%d\n",steps_x,steps_y,unit_steps_x,unit_steps_y);

	fsteps	= steps_y/unit_steps_y * unit_steps_y;
	zsteps= steps_x/unit_steps_x * unit_steps_x;
	steps.x = zsteps;
	steps.y = fsteps;
	steps.x_unit_steps = unit_steps_x;
	steps.y_unit_steps = unit_steps_y;
	steps.motor_speed = speed;
	ret = ioctl(af_fd, MOTOR_MOVE, (unsigned long)&steps);
	if(ret < 0){
		close(af_fd);
		printf(" af ioctl error!\n");
		return -1;
	}
	if(steps_y > 0)
		focus_cur_steps -= abs(fsteps);
	else
		focus_cur_steps += abs(fsteps);
	if(steps_x > 0)
		zoom_cur_steps -= abs(zsteps);
	else
		zoom_cur_steps += abs(zsteps);

	usleep(2300 * (delay_cnt = abs(zsteps) > abs(fsteps) ? abs(zsteps) : abs(fsteps)));
//	usleep(4000 * (delay_cnt = abs(zsteps) > abs(fsteps) ? abs(zsteps) : abs(fsteps)));
	return 0;
}

static int focus_move(int dir,int steps)
{
	int ret = -1;
	if(steps <= 0) return -1;
	printf("CUR(zoom)=%d CUR(focus)=%d\n",zoom_cur_steps, focus_cur_steps);
	if(dir == MOVE_FORWARD){
		ret = motor_move_step(0, steps,1,1,200);
	}else{
		ret = motor_move_step(0, -steps,1,1,200);
	}
	usleep(1000);
	return ret;
}

static void focus_calibration(int calibratio_cnt,int calibration_dir,int unit_calib_steps,int level)
{
	int i = 0;
	int com[100] = {0};
	int max = 0;
	int min = 0;
	int max_pos = 0;
	int droptimes = 0;
	int calib_time = 0;
	int dir = calibration_dir;
	if(calibratio_cnt > 100){
		printf("calibration too many times!!!");
		return;
	}
	for(i = 0; i < calibratio_cnt; i++){
		com[i] = get_af_metric();
		focus_move(calibration_dir,unit_calib_steps);

		if(i == 0){
			max = com[i];
			min = com[i];
		}else{
			if(max < com[i]){
				max = com[i];
				max_pos = i;
			}
			if(min > com[i])
				min = com[i];
		}

		if(com[i] < max) droptimes++;
		calib_time++;
		printf("[%d]%d ",i,com[i]);
		if(i%16 == 0) printf("\n");
		if(droptimes >= level)
			break;
	}
	printf("\n");
	printf("max = %d %d\n",max,max_pos);
	printf("move_back=%d\n",unit_calib_steps * (calib_time-max_pos));
	dir = dir == MOVE_BACKWARD ? MOVE_FORWARD : MOVE_BACKWARD;
	focus_move(dir,unit_calib_steps * (calib_time - max_pos));
}

static int get_pic_definition_evaluetion(int delay_ms)
{
	int com[10]={0};
	int sum = 0;
	int max = 0;
	int min = 0;
	int current = 0;
	int ret = -1;
	int i = 0;
	IMPISPAFHist af_hist;
	for(i = 0; i < 10; i++){
		ret = IMP_ISP_Tuning_GetAfHist(&af_hist);
		if(ret < 0){
			printf("IMP_ISP_Tuning_GetAfHist failed\n");
			return NULL;
		}
		//com[i] = (af_hist.af_stat.af_metrics);//主统计值
		com[i] = (af_hist.af_stat.af_metrics_alt/100);//副统计值
		sum += com[i];
		if(i == 0){
			max = com[i];
			min = com[i];
		}else{
			if(max < com[i]) max = com[i];
			if(min > com[i]) min = com[i];
		}
		usleep(delay_ms * 1000);
	}
	current = (sum - max - min)/8;
	return current;
}

static void *AfMonitorThread(void *argv)/**< 监控清晰度反馈值*/
{
	int current = 0;
	float ratio = 0;
	while(1){
		pthread_mutex_lock(&afMonitorMutex);
		pthread_cond_wait(&afMonitorCond, &afMonitorMutex);
		pthread_mutex_unlock(&afMonitorMutex);
		for(;;){
			if(motorStatus == MOTOR_IS_RUNNING)
				break;
			/**<每个800ms读取一次清晰度值*/
			ratio = 0;
			current = 0;
			current = get_pic_definition_evaluetion(40);
			usleep(800 * 1000);
			if(current >= pic_definition_evaluetion){
				pic_definition_evaluetion = current;
			}else{
				ratio = (float)(pic_definition_evaluetion - current) / (float)pic_definition_evaluetion;
			}
			printf("\rpicture  definition evaluetion %d ratio = %0.7lf",pic_definition_evaluetion,ratio);
			fflush(stdout);
			if(fabs(ratio) >= trigger_ratio){
				printf("@@@@@@@@@current=%d ratio=%0.7lf\n",current,ratio);
				printf("@@@@@@@@@@trigger focus again!!@@@@@@@@@@@@@@\n");

				focus_calibration(60,MOVE_FORWARD,10,7);
				focus_calibration(50,MOVE_BACKWARD,8,6);
				focus_calibration(40,MOVE_FORWARD,6,5);
				focus_calibration(30,MOVE_BACKWARD,4,5);

				current = get_pic_definition_evaluetion(40);

				pic_definition_evaluetion = current;
			//	pthread_cond_signal(&focusTuningCond);
				//break;
			}
		}
	}
	return NULL;
}

int xcam_af_zoom_in(int zoom_coef)
{
	int focus_steps = 0;
	int tmp = zoom_coef;
	if(zoom_coef <= 0){
		printf("zoom coef error!!\n");
		return -1;
	}
	if(zoom_ratio >= ZOOM_MAX_RATIO-1){
		printf("Max zoom ratio!!\n");
		return -1;
	}
	while(tmp--){
		if(zoom_ratio >= ZOOM_MAX_RATIO-1){
			printf("Max zoom ratio!!\n");
			break;
		}
		zoom_ratio++;
		focus_steps = abs(ZoomFocusTable[zoom_ratio].focusPos - focus_cur_steps);
		motorStatus = MOTOR_IS_RUNNING;
		motor_move_step(-zoom_unit_steps, -focus_steps,
				ZoomFocusTable[zoom_ratio].zoomMotorSpeed,
				ZoomFocusTable[zoom_ratio].focusMotorSpeed,
				ZoomFocusTable[zoom_ratio].irqSpeed);
		motorStatus = MOTOR_IS_STOP;
	}
	motorStatus = MOTOR_IS_RUNNING;
	focus_calibration(50,MOVE_FORWARD,6,5);
	focus_calibration(50,MOVE_BACKWARD,4,5);
	//focus_calibration(50,MOVE_FORWARD,4,5);
	//focus_calibration(50,MOVE_BACKWARD,4,5);
	focus_calibration(50,MOVE_FORWARD,2,5);
	focus_calibration(10,MOVE_BACKWARD,2,5);
	pic_definition_evaluetion = get_af_metric();
	motorStatus = MOTOR_IS_STOP;

#ifdef AUTO_FOCUS_THREAD
	pthread_cond_signal(&afMonitorCond);
#endif
	return 0;
}

int xcam_af_zoom_out(int zoom_coef)
{
	int focus_steps = 0;
	int tmp = zoom_coef;
	if(zoom_coef <= 0){
		printf("zoom coef error!!\n");
		return -1;
	}
	if(zoom_ratio <= ZOOM_MIN_RATIO){
		printf("Min zoom ratio!!\n");
		return -1;
	}
	while(tmp--){
		if(zoom_ratio <= ZOOM_MIN_RATIO){
			printf("Min zoom ratio!!\n");
			break;
		}
		zoom_ratio --;
		focus_steps = abs(ZoomFocusTable[zoom_ratio].focusPos  - focus_cur_steps);
		motorStatus = MOTOR_IS_RUNNING;
		motor_move_step(zoom_unit_steps, focus_steps,
				ZoomFocusTable[zoom_ratio].zoomMotorSpeed,
				ZoomFocusTable[zoom_ratio].focusMotorSpeed,
				ZoomFocusTable[zoom_ratio].irqSpeed);
	}
	focus_calibration(50,MOVE_FORWARD,6,5);
	focus_calibration(50,MOVE_BACKWARD,4,5);
	//focus_calibration(50,MOVE_FORWARD,4,5);
	//focus_calibration(50,MOVE_BACKWARD,4,5);
	focus_calibration(50,MOVE_FORWARD,2,5);
	focus_calibration(20,MOVE_BACKWARD,2,5);
	pic_definition_evaluetion = get_af_metric();
#ifdef AUTO_FOCUS_THREAD
	pthread_cond_signal(&afMonitorCond);
#endif
	return 0;
}

int xcam_af_move_forward(int steps)
{
	focus_move(MOVE_FORWARD,steps);
	return 0;
}

int xcam_af_move_backward(int steps)
{
	focus_move(MOVE_BACKWARD,steps);
	return 0;
}

void xcam_af_focus_calibration(void)
{
	motorStatus = MOTOR_IS_RUNNING;
	focus_calibration(50,MOVE_FORWARD,6,5);
	focus_calibration(50,MOVE_BACKWARD,4,5);
	//focus_calibration(50,MOVE_FORWARD,4,5);
	//focus_calibration(50,MOVE_BACKWARD,4,5);
	focus_calibration(50,MOVE_FORWARD,2,5);
	focus_calibration(10,MOVE_BACKWARD,2,5);
	pic_definition_evaluetion = get_af_metric();
	motorStatus = MOTOR_IS_STOP;
}

int xcam_af_reset(void)
{
	int ret = -1;
	struct motor_reset_data motor_reset_data;
	memset(&motor_reset_data, 0, sizeof(motor_reset_data));
	motorStatus = MOTOR_IS_RUNNING;
	motor_reset_data.x_max_steps = zoom_max_steps;
	motor_reset_data.x_cur_step = 0;
	motor_reset_data.y_max_steps = focus_max_steps;
	motor_reset_data.y_cur_step = 0;
	ret = ioctl(af_fd, MOTOR_RESET, &motor_reset_data);
	if(ret){
		printf("af reset error !\n");
		return -1;
	}
	zoom_cur_steps = 0;
	focus_cur_steps = 0;
	zoom_ratio = 0;
	motorStatus = MOTOR_IS_STOP;

	focus_move(MOVE_BACKWARD,ZoomFocusTable[zoom_ratio].focusPos);
	focus_calibration(50,MOVE_FORWARD,6,5);
	focus_calibration(50,MOVE_BACKWARD,4,5);
	//focus_calibration(50,MOVE_FORWARD,4,5);
	//focus_calibration(50,MOVE_BACKWARD,4,5);
	focus_calibration(50,MOVE_FORWARD,2,5);
	//focus_calibration(50,MOVE_BACKWARD,2,5);
	pic_definition_evaluetion = get_af_metric();
	return 0;
}

int xcam_af_init(void)
{
	int ret = 0;
	pthread_attr_t attr;
	IMPISPWeight af_weight = {
		.weight = {
			{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
			{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
			{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
			{1,1,1,8,8,8,8,8,8,8,8,1,1,1,1},
			{1,1,1,8,8,8,8,8,8,8,8,1,1,1,1},
			{1,1,1,8,8,8,8,8,8,8,8,1,1,1,1},
			{1,1,1,8,8,8,8,8,8,8,8,1,1,1,1},
			{1,1,1,8,8,8,8,8,8,8,8,1,1,1,1},
			{1,1,1,8,8,8,8,8,8,8,8,1,1,1,1},
			{1,1,1,8,8,8,8,8,8,8,8,1,1,1,1},
			{1,1,1,8,8,8,8,8,8,8,8,1,1,1,1},
			{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
			{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
			{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
			{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
		},
	};
	IMPISPAFHist af_hist;
	if(af_fd > 0){
		printf("/dev/motor already open!\n");
		return -1;
	}else{
		af_fd = open("/dev/motor",O_RDWR);
		if(af_fd < 0){
			printf("open /dev/motor failed!\n");
			return -1;
		}
		ret = ioctl(af_fd, MOTOR_SPEED, (unsigned long)&motor_speed);
		if(ret){
			printf("set motor speed failed!\n");
			return -1;
		}
	}
#if 0
	ret = IMP_ISP_Tuning_SetAfWeight(&af_weight);/**<设置Af统计区域的权重*/
	if(ret){
		printf("IMP_ISP_Tuning_SetAfWeight error!\n");
		return -1;
	}
#endif
	/*
	*
	* AF统计值参数
	*
	* typedef struct {
	*	struct isp_core_af_sta_info af_stat;AF统计值信息
	*	unsigned char af_enable;			AF功能开关
	*	unsigned char af_metrics_shift;		AF统计值缩小参数 默认是0，1代表缩小2倍
	*	unsigned short af_delta;			AF主统计值阈值 [0 ~ 64]
	*	unsigned short af_theta;			AF主统计值阈值 [0 ~ 64]
	*	unsigned short af_hilight_th;		AF高亮点统计阈值 [0 ~ 255]
	*	unsigned short af_alpha_alt;		AF次统计值阈值 [0 ~ 64]
	*	unsigned char  af_hstart;			AF统计值横向起始点 [0 ~ 15]
	*	unsigned char  af_vstart;			AF统计值竖向起始点 [0 ~ 15]
	*	unsigned char  af_stat_nodeh;		水平方向有效统计区域个数 [0 ~ 15]
	*	unsigned char  af_stat_nodev;		垂直方向有效统计区域个数 [0 ~ 15]
	* } IMPISPAFHist;
	*
	* struct isp_core_af_sta_info{
	*     unsigned int af_metrics;		AF主统计值
	*     unsigned int af_metrics_alt;	AF次统计值
    * };
	*/
	ret = IMP_ISP_Tuning_GetAfHist(&af_hist);
	if(ret < 0){
		printf("%s(%d):IMP_ISP_Tuning_GetAfHist failed\n", __func__, __LINE__);
		return -1;
	}
	af_hist.af_metrics_shift = 1;
//	af_hist.af_stat_nodeh = 15;
//	af_hist.af_stat_nodev = 15;
	ret = IMP_ISP_Tuning_SetAfHist(&af_hist);
	if(ret < 0){
		printf("%s(%d):IMP_ISP_Tuning_SetAfHist failed\n", __func__, __LINE__);
		return -1;
	}
#ifdef AUTO_FOCUS_THREAD
	pthread_cond_init(&afMonitorCond, NULL);
	pthread_mutex_init(&afMonitorMutex, NULL);
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	ret = pthread_create(&afTid,&attr,AfMonitorThread,NULL);
	if(ret != 0){
		printf("create auto focus thread failed!\n");
		return -1;
	}
#endif
	return 0;
}

int xcam_af_deinit(void)
{
	int ret = -1;
	motorStatus = MOTOR_IS_STOP;
#ifdef AUTO_FOCUS_THREAD
	pthread_cancel(afTid);
	pthread_join(afTid,NULL);
	pthread_cond_destroy(&afMonitorCond);
	pthread_mutex_destroy(&afMonitorMutex);
#endif
	ret = ioctl(af_fd, MOTOR_STOP, 0);
	if(ret < 0){
		close(af_fd);
		printf(" motor ioctl failed!\n");
		return -1;
	}
	close(af_fd);
	af_fd = -1;
	return 0;
}
