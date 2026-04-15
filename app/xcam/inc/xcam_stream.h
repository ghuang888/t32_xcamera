#ifndef _XCAM_STREAM_H_
#define _XCAM_STREAM_H_

typedef struct msg_stream_s {
	unsigned char* buf;
	int size;
	int64_t ts;
	int type;
	void* priv;
} msg_stream_t;

typedef struct stream_msg_s {
	msg_com_data_t com;
	msg_stream_t msg;
} stream_msg_t;

typedef struct stream_module_s {
	xcam_module_t *xmod;
	int channel;
	int state;
	int tid;
	void *msgpool;
	pthread_mutex_t stream_mutex;//用于控制模块之间数据传输启动停止的锁
} stream_module_t;

int stream_module_create(int channel, int modid);
int stream_module_stop(int streamnum);
int stream_module_start(int streamnum);
#endif
