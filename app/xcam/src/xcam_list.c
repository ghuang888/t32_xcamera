/*
 *
 * mlist is a list struct that alloc and manage memory by itself.
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include "xcam_list.h"

typedef struct mnode{
	struct mnode *next;
	uint8_t  *pdata;
} mnode_t;

typedef struct {
	uint32_t magic;
	uint32_t maxcnt;
	uint32_t cnt;
	uint32_t datasize;

	mnode_t *busy_head;
	mnode_t *busy_tail;
	mnode_t *free_head;
	mnode_t *free_tail;

	//real data
	mnode_t array[0];
} mlist_t;

#define MLIST_DATA_MAGIC (('M'<< 24)|('L'<< 16)|('I'<< 8)|('S'<< 0))

void* mlist_alloc(int cnt, int datasize)
{
	int i = 0;
	int size = 0;
	mlist_t *ml;
	mnode_t *node;
	size = sizeof(mlist_t)+(sizeof(mnode_t)+datasize)*cnt;
	ml = malloc(size);
	if (NULL == ml) {
		printf("err(%s,%d): malloc err\n", __func__, __LINE__);
		return NULL;
	}
	memset(ml, 0, size);
	node = &ml->array[0];
	ml->magic = MLIST_DATA_MAGIC;
	ml->maxcnt = cnt;
	ml->cnt = 0;
	node->pdata = (uint8_t *)ml+sizeof(mlist_t)+sizeof(mnode_t)*cnt;
	ml->free_head = node;
	ml->free_tail = node;
	for (i = 1; i < cnt; i++) {
		mnode_t *node = &ml->array[i];
		node->pdata = (uint8_t *)ml+sizeof(mlist_t)+sizeof(mnode_t)*cnt+(i*datasize);
		ml->free_tail->next = node;
		ml->free_tail = node;
	}
	ml->datasize = datasize;
	return ml;
}

void mlist_free(void *mlist)
{
	mlist_t *ml;
	ml = (mlist_t*)mlist;
	if (MLIST_DATA_MAGIC != ml->magic) {
		printf("err(%s,%d): mlist magic  err\n", __func__, __LINE__);
		return;
	}
	free(mlist);
	return;
}

int32_t mlist_put(void *mlist, uint8_t *data, int32_t size)
{
	mlist_t *ml;
	mnode_t *node;
	ml = (mlist_t*)mlist;
	if (MLIST_DATA_MAGIC != ml->magic) {
		printf("err(%s,%d): mlist magic  err\n", __func__, __LINE__);
		return -1;
	}
	if (NULL == ml->free_head) {
		printf("err(%s,%d): mlist free null\n", __func__, __LINE__);
		return -1;
	}
	if (size != ml->datasize) {
		printf("err(%s,%d): data size error, want %d, but give %d\n",
				__func__, __LINE__, ml->datasize, size);
		return -1;
	}
	//get free node
	node = ml->free_head;
	if (ml->free_head == ml->free_tail) {
		ml->free_head = NULL;
		ml->free_tail = NULL;
	} else {
		ml->free_head = node->next;
	}
	//put busy node
	memcpy(node->pdata, data, ml->datasize);
	node->next = NULL;
	if ((NULL == ml->busy_head)||(NULL == ml->busy_tail)) {
		ml->busy_head = node;
		ml->busy_tail = node;
	} else {
		ml->busy_tail->next = node;
		ml->busy_tail = node;
	}
	ml->cnt++;
	return 0;
}

int32_t mlist_get(void *mlist, uint8_t *data, int32_t size)
{
	mlist_t *ml = mlist;
	mnode_t *node;
	if (MLIST_DATA_MAGIC != ml->magic) {
		printf("err(%s,%d): mlist magic  err\n", __func__, __LINE__);
		return -1;
	}
	if (NULL == ml->busy_head) {
		//printf("warn(%s,%d): mlist busy null\n", __func__, __LINE__);
		return -1;
	}
	if (size != ml->datasize) {
		printf("err(%s,%d): data size error, want %d, but get %d\n",
				__func__, __LINE__, ml->datasize, size);
		return -1;
	}
	//get busy node
	node = ml->busy_head;
	if (ml->busy_head == ml->busy_tail) {
		ml->busy_head = NULL;
		ml->busy_tail = NULL;
	} else {
		ml->busy_head = node->next;
	}
	if (NULL != data)
		memcpy(data, node->pdata, ml->datasize);
	memset(node->pdata, 0, ml->datasize);
	node->next = NULL;
	//put free node
	if ((NULL == ml->free_head)||(NULL == ml->free_tail)) {
		ml->free_head = node;
		ml->free_tail = node;
	} else {
		ml->free_tail->next = node;
		ml->free_tail = node;
	}
	ml->cnt--;
	return 0;
}

int mlist_clear(void *mlist)
{
	mlist_t *ml = mlist;
	mnode_t *node;
	if (MLIST_DATA_MAGIC != ml->magic) {
		printf("err(%s,%d): mlist magic  err\n", __func__, __LINE__);
		return -1;
	}
	if (NULL == ml->busy_head) {
		//printf("warn(%s,%d): mlist already free\n", __func__, __LINE__);
		return 0;
	}
	//get busy node
	node = ml->busy_head;
	while(node) {
		memset(node->pdata, 0, ml->datasize);
		//put free node
		if ((NULL == ml->free_head)||(NULL == ml->free_tail)) {
			ml->free_head = node;
			ml->free_tail = node;
		} else {
			ml->free_tail->next = node;
			ml->free_tail = node;
		}
		ml->cnt--;
		node = node->next;
	}
	ml->busy_head = NULL;
	ml->busy_tail = NULL;
	return 0;
}

int mlist_pre_get(void *mlist, uint8_t *data, int32_t size)
{
	mlist_t *ml = mlist;
	mnode_t *node;
	if (MLIST_DATA_MAGIC != ml->magic) {
		printf("err(%s,%d): mlist magic  err\n", __func__, __LINE__);
		return -1;
	}
	if (NULL == ml->busy_head) {
		printf("err(%s,%d): mlist busy null\n", __func__, __LINE__);
		return -1;
	}
	//get busy node
	node = ml->busy_head;
	memcpy(data, node->pdata, ml->datasize);
	return 0;
}

int mlist_pre_get_ptr(void *mlist, int32_t idx, uint8_t **data)
{
	mlist_t *ml = mlist;
	mnode_t *node;
	int i = 0;
	if (MLIST_DATA_MAGIC != ml->magic) {
		printf("err(%s,%d): mlist magic  err\n", __func__, __LINE__);
		return -1;
	}
	if (NULL == ml->busy_head) {
		//printf("warn(%s,%d): mlist busy null\n", __func__, __LINE__);
		return -1;
	}

	node = ml->busy_head;
	if ((node)&&(idx == i)) {

		*data = node->pdata;
		return 0;
	}
	while(node) {
		node = node->next;
		i++;
		if ((node)&&(idx == i)) {
			*data = node->pdata;
			return 0;
		}
	}
	return -1;
}

int32_t mlist_num(void *mlist)
{
	mlist_t *ml = mlist;
	if (MLIST_DATA_MAGIC != ml->magic) {
		printf("err(%s,%d): mlist magic  err\n", __func__, __LINE__);
		return -1;
	}
	return ml->cnt;
}

void mlist_print(void *mlist)
{
	mlist_t *ml = mlist;
	mnode_t *node;
	printf("----------------------\n");
	if (MLIST_DATA_MAGIC != ml->magic) {
		printf("err(%s,%d): mlist magic  err\n", __func__, __LINE__);
		return;
	}
	printf("cnt = %d\n", ml->cnt);
	node = ml->busy_head;
	while(node) {
		printf("data = 0x%x\n", *(uint32_t*)(node->pdata));
		node = node->next;
	}
}

int32_t mlist_head(void *mlist, void **node, uint8_t **data)
{
	mlist_t *ml = mlist;
	mnode_t *n;
	if (MLIST_DATA_MAGIC != ml->magic) {
		printf("err(%s,%d): mlist magic  err\n", __func__, __LINE__);
		return -1;
	}
	n = ml->busy_head;
	*node = n;
	if (n) {
		*data = n->pdata;
	}
	return 0;
}

int mlist_next(void *mlist, void **node, uint8_t **data)
{
	mlist_t *ml = mlist;
	mnode_t *n;
	if (MLIST_DATA_MAGIC != ml->magic) {
		printf("err(%s,%d): mlist magic  err\n", __func__, __LINE__);
		return -1;
	}
	n = *node;
	if (n) {
		n = n->next;
		*node = n;
		if (n) {
			*data = n->pdata;
		}
	} else {
		return -1;
	}
	return 0;
}

#if 0
int mlist_find(void *mlist, int (*process)(void *arg, int idx, void *pdata), void *arg, void **pdata)
{
	mlist_t *ml = mlist;
	mnode_t *node;
	if (MLIST_DATA_MAGIC != ml->magic) {
		printf("err(%s,%d): mlist magic  err\n", __func__, __LINE__);
		*pdata = NULL;
		return -1;
	}
	node = ml->busy_head;
	int stop = 0;
	int idx = 0;
	while(node) {
		stop = (*process)(arg, idx, node->pdata);
		node = node->next;
		idx++;
		if (stop) {
			*pdata = node->pdata;
			return 0;
		};
	}
	*pdata = NULL;
	return 0;
}
#endif

#ifdef TEST_XCAM_LIST

/*
 *
 * gcc xcam_list.c -I../inc/ -DTEST_XCAM_LIST -o xcam_list
 *
 *
 */

static int test1()
{
	void *mlist = mlist_alloc(100, sizeof(int32_t));
	int32_t i = 0;
	int32_t data = 0;
	for (i = 0; i < 50; i++) {
		mlist_put(mlist, (uint8_t*)&i, sizeof(i));
		assert(mlist_num(mlist) == (i+1));
	}
	for (i = 0; i < 50; i++) {
		mlist_get(mlist, (uint8_t*)&data, sizeof(data));
		assert(data == i);
	}
	mlist_free(mlist);
	return 0;
}

static int test2()
{
	void *mlist = MLIST_ALLOC_INT32(100);
	int32_t i = 0;
	int32_t data = 0;
	for (i = 0; i < 50; i++) {
		MLIST_PUT_INT32(mlist, &i);
		assert(mlist_num(mlist) == (i+1));
	}
	for (i = 0; i < 50; i++) {
		MLIST_GET_INT32(mlist, &data);
		assert(data == i);
	}
	MLIST_FREE(mlist);
	return 0;
}

int main(int argc, char **argv)
{
	test1();
	test2();
	return 0;
}

#endif
