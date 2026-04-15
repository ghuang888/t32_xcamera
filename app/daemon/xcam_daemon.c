#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

#include "xcam_daemon_protocol.h"

int msg_process_func(socket_tcp_link_t *plink)
{
#if 0
	printf("msg recv: cmd = 0x%08x, len = 0x%08x, action = 0x%08x\n",
			msg_header->cmd, msg_header->len, msg_header->action);
#endif
	int ret = 0;
	switch(plink->msgbuf->header.cmd) {
		case SET_SYS_FIRMWARE_SEND:
			ret = xcam_socket_upgrade_msg_process(plink);
			if (ret < 0) {
				printf("err(%s,%d): xcam_socket_update_msg_process error!\n", __func__, __LINE__);
				skt_msg_upgrade_ack_t buf;
				buf.header.cmd = SET_SYS_FIRMWARE_ACK;
				buf.header.len = sizeof(skt_msg_upgrade_ack_t) - sizeof(skt_msg_header_t);
				buf.header.flags = 1;
				buf.status = CMD_ACK_STATUS_ERR;
				if (socket_server_tcp_send(plink, (skt_msg_t *)&buf) < 0)
					printf("err(%s,%d): server send error\n", __func__, __LINE__);
				return -1;
			}
			break;
		default:
			printf("err(%s,%d): cmd not support\n", __func__, __LINE__);
			break;
	}
	return 0;
}

int main() {
	int ret;
	void *skt;
	skt = socket_server_tcp_alloc(11910, 5);
	assert(NULL != skt);
	ret = socket_server_tcp_set_msg_process_cb(skt, msg_process_func);
	assert(0 == ret);
	ret = socket_server_tcp_start(skt);
	assert(0 == ret);
	printf("xcam_daemon now working background..\n");
	while (1)
		sleep(1);
	return 0;
}
