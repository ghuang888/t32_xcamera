
#ifndef _CONF_PROCESS_H
#define _CONF_PROCESS_H

/*pstr_ret cjson处理后返回的字符串内存大小，暂定1024*/
#define MAX_RETSTR_LEN 2048

#ifdef __cplusplus
extern "C" {
#endif
#include <pthread.h>
#include <xcam_thread.h>
#include <assert.h>
pthread_mutex_t g_conf_mutex_t;

void msg_process_init(void);
int conf_process(char *pstring,char *pstr_ret);
int rcf_server_start(void);

//int server_start();

#ifdef __cplusplus
}
#endif

#endif //!_CONF_PROCESS_H

