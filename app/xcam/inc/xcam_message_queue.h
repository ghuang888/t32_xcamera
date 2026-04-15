#ifndef _XCAM_MESSAGE_QUEUE_H_
#define _XCAM_MESSAGE_QUEUE_H_

#ifdef __cplusplus
extern "C" {
#endif

#define MSGTYPE_SEND 100
#define MSGTYPE_RECV 101

typedef struct xcam_msg_set_ipaddr_s {
	char interface[20];
	char ipaddr[20];
}xcam_msg_set_ipaddr_t;

typedef enum {
	XCAM_MESSAGE_SET_IPADDR,
}xcam_msg_data_flag;

union xcam_trans_data_u {
	xcam_msg_set_ipaddr_t xcam_ipaddr;
};

typedef struct xcam_web_info_s {
	xcam_msg_data_flag info_flag;
	union xcam_trans_data_u data;
}xcam_msg_data_t;

typedef struct {
	long type ;
	xcam_msg_data_t transData;
} msg_t;

int xcam_message_send(xcam_msg_data_t *info_data_t);
int xcam_message_receive(void);

#ifdef __cplusplus
}
#endif
#endif
