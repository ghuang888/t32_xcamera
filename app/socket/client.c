#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <assert.h>
#include "socket_tcp_common.h"

typedef struct msg_data_request_firmware_upgrade_s{
	skt_msg_header_t header;
	char data[8*1024*1024];
} msg_data_request_firmware_upgrade_t;

msg_data_request_firmware_upgrade_t msgmsg_request_firmware_upgrade;

int main(int argc, char *argv[])
{
	int ret = 0;
	void *cskt;
	int test_count = 10000;
	cskt = socket_client_tcp_alloc("193.169.3.128", 11910);
	assert(cskt != NULL);
	msgmsg_request_firmware_upgrade.header.cmd = 0x11111111;
	msgmsg_request_firmware_upgrade.header.flags = MSG_FLAGS_DATA_ENDING;
	msgmsg_request_firmware_upgrade.header.len= sizeof(msgmsg_request_firmware_upgrade.data);

	while (1) {
		msgmsg_request_firmware_upgrade.header.len = (random()%20000)+100;
		ret = socket_client_tcp_send(cskt, (skt_msg_t*)&msgmsg_request_firmware_upgrade);
		//ret = socket_client_tcp_recv(cskt, (skt_msg_t*)&msgmsg_request_firmware_upgrade);
		assert(0 == ret);
	}

	return 0;
}
