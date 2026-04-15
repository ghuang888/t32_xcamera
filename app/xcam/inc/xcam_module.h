#ifndef _XCAM_MOD_H_
#define _XCAM_MOD_H_

#include <stdbool.h>
#include "xcam_msg.h"

#define XMOD_ID_MAX				(20)
#define XMOD_NAME_MAX_LEN		(20)
#define XMOD_WORK_LIST_MAX_LEN 	(10)

typedef int (*msg_process_f)(struct msg_com_data_s *);
typedef enum{
	MSG_VIDEO,
	MSG_AUDIO
}MODULE__TYPE;

typedef struct xcam_module_s {
    char name[XMOD_NAME_MAX_LEN+1];
    int id;
    int state;
    msg_process_f pfun;
	void *work_list;
	/*
	use metrix struct to forward message.
	think of use message link future.
	message0--->[module0,module1,module2,...]
	message1--->[module0,module1,module2,...]
	*/
    bool forward_msg[MESG_ID_MAX][XMOD_ID_MAX];
} xcam_module_t;

void xcam_module_process_msg(int moduleid, msg_com_data_t *msg);
void* xcam_module_alloc(char *name, int id, msg_process_f pfun);
void xcam_module_free(void *module, int id);
void xcam_module_register_wanted_msg(int moduleid, int forward_moduleid, int msgid);
void xcam_module_unregister_wanted_msg(int moduleid, int forward_moduleid, int msgid);
#endif
