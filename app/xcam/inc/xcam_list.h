#ifndef _LIST_H_
#define _LIST_H_


#include <stdint.h>

void* mlist_alloc(int cnt, int datasize);
void mlist_free(void *mlist);
int32_t mlist_put(void *mlist, uint8_t *data, int32_t size);
int32_t mlist_get(void *mlist, uint8_t *data, int32_t size);
int32_t mlist_clear(void *mlist);

//macro for pointer data type
#define MLIST_ALLOC_PTR(cnt) mlist_alloc(cnt, sizeof(intptr_t))
#define MLIST_PUT_PTR(ml, data) mlist_put(ml, (uint8_t *)data, sizeof(intptr_t))
#define MLIST_GET_PTR(ml, data) mlist_get(ml, (uint8_t *)data, sizeof(intptr_t))

#define MLIST_FREE(ml) mlist_free(ml)
#define MLIST_CLEAR(ml) mlist_clear(ml)

//macro for int32_t data type
#define MLIST_ALLOC_INT32(cnt) mlist_alloc(cnt, sizeof(int32_t))
#define MLIST_PUT_INT32(ml, data) mlist_put(ml, (uint8_t *)data, sizeof(int32_t))
#define MLIST_GET_INT32(ml, data) mlist_get(ml, (uint8_t *)data, sizeof(int32_t))

#endif
