#ifndef __XCAM_DAEMON_PROTOCOL_H__
#define __XCAM_DAEMON_PROTOCOL_H__

#include "../socket/socket_tcp_common.h"

#define CMD_ACK_STATUS_ERR -1
#define CMD_ACK_STATUS_SUCCESS 0

typedef enum skt_msg_cmd_e {
	CMD_SYS_SEND_START = 0x00000100,
	SET_SYS_FIRMWARE_SEND = CMD_SYS_SEND_START,

	CMD_SYS_ACK_START = 0x00000200,
	SET_SYS_FIRMWARE_ACK = CMD_SYS_ACK_START,

	CMD_CONFIGS_SEND_START = 0x00000300,
	GET_CONFIGS_SEND = CMD_CONFIGS_SEND_START,
	SET_CONFIGS_SEND,

	CMD_CONFIGS_ACK_START = 0x00000400,
	GET_CONFIGS_ACK = CMD_CONFIGS_ACK_START,
	SET_CONFIGS_ACK,

	CMD_WEB_CFGS_SEND_START = 0x00000500,
	CMD_WEB_CFGS_SEND = CMD_WEB_CFGS_SEND_START,

	CMD_WEB_CFGS_ACK_START = 0x00000600,
	CMD_WEB_CFGS_ACK = CMD_WEB_CFGS_ACK_START,

} skt_msg_cmd_t;

/* list all msg structs */
typedef struct skt_msg_upgrade_ack_s {
	skt_msg_header_t header;
	int32_t status;
} skt_msg_upgrade_ack_t;

int xcam_socket_upgrade_msg_process(socket_tcp_link_t *plink);
#endif
