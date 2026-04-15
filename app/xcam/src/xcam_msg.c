#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include "xcam_msg.h"

void* msg_pool_alloc(char *name, int id, int cnt, int datasize, msg_release_f msg_release_cb)
{
	int i = 0;
	int size = 0;
	int ret = 0;
	msg_pool_t *mp;
	size = sizeof(msg_pool_t)+(sizeof(msg_node_t)+datasize)*cnt;
	mp = malloc(size);
	if (NULL == mp) {
		printf("err(%s,%d): msg pool malloc err\n", __func__, __LINE__);
		return NULL;
	}
	memset(mp, 0, size);
	if (NULL != name) {
		i = strlen(name);
		if (i > MESG_NAME_MAX_LEN) {
			printf("warn(%s,%d): msg pool name too long\n", __func__, __LINE__);
			i = MESG_NAME_MAX_LEN;
		}
		strncpy(mp->name, name, i);
	}

	ret = pthread_mutex_init(&mp->mutex, NULL);
	if (0 != ret) {
		printf("err(%s,%d): msg pool mutex init error\n", __func__, __LINE__);
		return NULL;
	}

	mp->maxcnt = cnt;
	mp->freecnt = mp->maxcnt;
	mp->busycnt = 0;
	for (i = 0; i < cnt; i++) {
		msg_node_t *node = (msg_node_t *)((char *)mp+sizeof(msg_pool_t)+(sizeof(msg_node_t)+datasize)*i);
		node->mp = mp;
		node->com.id = id;
		node->com.datasize = datasize;
		node->com.msg_release_cb = msg_release_cb;
		if (0 == i) {
			mp->free.head = node;
			mp->free.tail = node;
		} else {
			mp->free.tail->next = node;
			mp->free.tail = node;
		}
	}
	return mp;
}

void msg_pool_free(void *mp)
{
	msg_pool_t *m;
	m = (msg_pool_t*)mp;
	free(m);
	return;
}

static msg_node_t* msg_pool_get_free_node(void *mp)
{
	int ret = 0;
	msg_pool_t *mpool = mp;
	msg_node_t *mn;
	if (0 == mpool->freecnt) {
		//printf("warn(%s,%d): pool free empty\n", __func__, __LINE__);
		return NULL;
	}
	ret = pthread_mutex_lock(&mpool->mutex);
	if (0 != ret) {
		printf("error(%s,%d): msg lock failed\n", __func__, __LINE__);
		return NULL;
	}
	//get free node
	mn = mpool->free.head;
	if (1 == mpool->freecnt) {
		mpool->free.head = NULL;
		mpool->free.tail = NULL;
		mpool->freecnt = 0;
	} else {
		mpool->free.head = mn->next;
		mpool->freecnt--;
	}
	ret = pthread_mutex_unlock(&mpool->mutex);
	if (0 != ret) {
		printf("error(%s,%d): msg unlock failed\n", __func__, __LINE__);
		return NULL;
	}
	return mn;
}

static msg_node_t* msg_pool_get_busy_node(void *mp)
{
	int ret = 0;
	msg_pool_t *mpool = mp;
	msg_node_t *mn;
	if (0 == mpool->busycnt) {
		//printf("warn(%s,%d): pool free empty\n", __func__, __LINE__);
		return NULL;
	}
	ret = pthread_mutex_lock(&mpool->mutex);
	if (0 != ret) {
		printf("error(%s,%d): msg lock failed\n", __func__, __LINE__);
		return NULL;
	}
	//get busy node
	mn = mpool->busy.head;
	if (1 == mpool->busycnt) {
		mpool->busy.head = NULL;
		mpool->busy.tail = NULL;
		mpool->busycnt = 0;
	} else {
		mpool->busy.head = mn->next;
		mpool->busycnt--;
	}
	ret = pthread_mutex_unlock(&mpool->mutex);
	if (0 != ret) {
		printf("error(%s,%d): msg unlock failed\n", __func__, __LINE__);
		return NULL;
	}
	return mn;
}

static int msg_pool_put_busy_node(msg_node_t* mn)
{
	int ret = 0;
	msg_pool_t *mp;
	mp = mn->mp;
	if (NULL == mp) {
		printf("error(%s,%d): error msg pool pointer\n", __func__, __LINE__);
		return -1;
	}
	ret = pthread_mutex_lock(&mp->mutex);
	if (0 != ret) {
		printf("error(%s,%d): msg lock failed\n", __func__, __LINE__);
		return -1;
	}
	//put busy node
	mn->next = NULL;
	if (0 == mp->busycnt) {
		mp->busy.head = mn;
		mp->busy.tail = mn;
	} else {
		mp->busy.tail->next = mn;
		mp->busy.tail = mn;
	}
	mp->busycnt++;
	ret = pthread_mutex_unlock(&mp->mutex);
	if (0 != ret) {
		printf("error(%s,%d): msg unlock failed\n", __func__, __LINE__);
		return -1;
	}
	return 0;
}

static int msg_pool_put_free_node(msg_node_t* mn)
{
	int ret = 0;
	msg_pool_t *mp;
	mp = mn->mp;
	if (NULL == mp) {
		printf("error(%s,%d): error msg pool pointer\n", __func__, __LINE__);
		return -1;
	}
	ret = pthread_mutex_lock(&mp->mutex);
	if (0 != ret) {
		printf("error(%s,%d): msg lock failed\n", __func__, __LINE__);
		return -1;
	}

	//put free node
	mn->next = NULL;

	if (0 == mp->freecnt) {
		mp->free.head = mn;
		mp->free.tail = mn;
	} else {
		mp->free.tail->next = mn;
		mp->free.tail = mn;
	}
	mp->freecnt++;
	ret = pthread_mutex_unlock(&mp->mutex);
	if (0 != ret) {
		printf("error(%s,%d): msg unlock failed\n", __func__, __LINE__);
		return -1;
	}
	return 0;
}

void* msg_pool_get_free_msg(void *mp)
{
	msg_node_t *mn;
	if (NULL == mp) {
		printf("error(%s,%d): msg pool pointer is NULL\n", __func__, __LINE__);
		return NULL;
	}
	mn = msg_pool_get_free_node(mp);
	if (NULL == mn) {
		printf("error(%s,%d): free message pointer NULL\n", __func__, __LINE__);
		return NULL;
	}
	return mn->com.pdata;
}

void* msg_pool_get_busy_msg(void *mp)
{
	msg_node_t *mn;
	if (NULL == mp) {
		printf("error(%s,%d): msg pool pointer is NULL\n", __func__, __LINE__);
		return NULL;
	}
	mn = msg_pool_get_busy_node(mp);
	if (NULL == mn) {
		printf("error(%s,%d): busy message pointer NULL\n", __func__, __LINE__);
		return NULL;
	}
	return mn->com.pdata;
}

int msg_pool_put_busy_msg(void *msg)
{
	msg_node_t *mn;
	if (NULL == msg) {
		printf("error(%s,%d): msg data pointer is NULL\n", __func__, __LINE__);
		return -1;
	}
	mn = (msg_node_t*)(((char*)msg)-sizeof(msg_node_t));
	msg_pool_put_busy_node(mn);
	return 0;
}

int msg_pool_put_free_msg(void *msg)
{
	msg_node_t *mn;
	if (NULL == msg) {
		printf("error(%s,%d): msg data pointer is NULL\n", __func__, __LINE__);
		return -1;
	}
	mn = (msg_node_t*)(((char*)msg)-sizeof(msg_node_t));
	msg_pool_put_free_node(mn);
	return 0;
}

/* work queue, store work msg pointer */
void* msg_work_queue_alloc(int maxcnt)
{
	void *q;
	q = msg_pool_alloc(NULL, 0, maxcnt, sizeof(void*), NULL);
	return q;
}

int msg_work_queue_input(void *q, void *work)
{
	int ret = 0;
	void **msg;
	msg = msg_pool_get_free_msg(q);
	if (NULL == msg)
		return -1;
	*msg = work;
	ret = msg_pool_put_busy_msg(msg);
	return ret;
}

void* msg_work_queue_output(void *q)
{
	int ret = 0;
	void **msg;
	void **data;
	msg = msg_pool_get_busy_msg(q);
	if (NULL == msg) {
		printf("error(%s,%d): get busy msg error msg = %p\n", __func__, __LINE__, msg);
		return NULL;
	}
	data = msg;
	ret = msg_pool_put_free_msg(msg);
	if (0 != ret) {
		printf("error(%s,%d): put free msg error msg = %p\n", __func__, __LINE__, msg);
		return NULL;
	}
	return data;
}

void msg_work_queue_free(void *q)
{
	msg_pool_free(q);
	return;
}

#if 0

int test1(void)
{
	int i = 0;
	int testnum = 0;
	void* mp_video;
	typedef struct video_date_s {
		int framenum;
		int framesize;
		int frametype;
	} video_date_t;
	mp_video = msg_pool_alloc("video_date", 10, 100, sizeof(video_date_t), NULL);
	assert(NULL != mp_video);
	testnum = 1000;
retry:
	for (i = 0; i < 100; i++) {
		video_date_t *msg = msg_pool_get_free_msg(mp_video);
		msg->framenum = i;
		msg->framesize = 1000;
		msg->frametype = 2;
		msg_pool_put_busy_msg(msg);
		assert(10 == MSG_TO_MSG_ID(msg));
	}
	for (i = 0; i < 100; i++) {
		video_date_t *msg = msg_pool_get_busy_msg(mp_video);
		assert(msg->framenum == i);
		assert(msg->framesize == 1000);
		assert(msg->frametype == 2);
		msg_pool_put_free_msg(msg);
#if 0
		printf("check result: %3d, num = %3d, size = %3d, type = %d\n",
				i, msg->framenum, msg->framesize, msg->frametype);
#endif
	}
	if (testnum--)
		goto retry;
	msg_pool_free(mp_video);
	return 0;
}

int test2()
{
	int i = 0;
	void *queue;
	void *data;
	queue = msg_work_queue_alloc(200);
	printf("------------------------\n");
	for (i = 0; i < 200; i++) {
		data = (void*)(long)i;
		msg_work_queue_input(queue, data);
		data = msg_work_queue_output(queue);
		printf("%3d, data = %3d\n", i, *(int*)data);
		assert(i == *(long*)data);
	}
	printf("------------------------\n");
	for (i = 0; i < 200; i++) {
		data = (void*)(long)i;
		msg_work_queue_input(queue, data);
	}
	for (i = 0; i < 200; i++) {
		data = msg_work_queue_output(queue);
		printf("%3d, data = %3d\n", i, *(int*)data);
		assert(i == *(long*)data);
	}
	return 0;
}

int main(int argc, char **argv)
{
	test1();
	test2();
	return 0;
}

#endif
