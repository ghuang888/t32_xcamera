#ifndef _XCAM_MSG_H_
#define _XCAM_MSG_H_
#include <stdint.h>

#define MESG_ID_MAX 20
#define MESG_NAME_MAX_LEN 20

struct msg_pool_s;
struct msg_com_data_s;
typedef int (*msg_release_f)(struct msg_com_data_s *mcd);
typedef struct msg_com_data_s {
    uint32_t id;
    uint32_t ref;
    uint32_t datasize;
    msg_release_f msg_release_cb;
    char  pdata[0];
} msg_com_data_t;

typedef struct msg_node_s {
    struct msg_pool_s *mp;
    struct msg_node_s *next;
    /* msg data, common data and private data */
    struct msg_com_data_s com;
} msg_node_t;

typedef struct {
    msg_node_t *head;
    msg_node_t *tail;
} msg_queue_t;

typedef struct msg_pool_s {
    char name[MESG_NAME_MAX_LEN+1];
    uint32_t maxcnt;
    uint32_t busycnt;
    uint32_t freecnt;
    msg_queue_t busy;
    msg_queue_t free;
	pthread_mutex_t mutex;
} msg_pool_t;

#define MSG_DATA_TO_MSG(msg)	((msg_com_data_t*)(((char*)msg)-sizeof(msg_com_data_t)))
#define GET_MSGDATA(msg)	    (((char*)msg) + sizeof(msg_com_data_t))
#define MSG_TO_MSG_ID(msg)		(((msg_com_data_t*)(((char*)msg)-sizeof(msg_com_data_t)))->id)

void* msg_pool_alloc(char *name, int mid, int cnt, int datasize, msg_release_f msg_release_cb);
void msg_pool_free(void *mp);
void* msg_pool_get_free_msg(void *mp);
void* msg_pool_get_busy_msg(void *mp);
void* msg_pool_get_busy_msg(void *mp);
int msg_pool_put_free_msg(void *msg);

void* msg_work_queue_alloc(int maxcnt);
int msg_work_queue_input(void *q, void *work);
void* msg_work_queue_output(void *q);
void msg_work_queue_free(void *q);
#endif
