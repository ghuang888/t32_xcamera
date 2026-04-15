#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <assert.h>
#include "../../socket/socket_tcp_common.h"
#include "../xcam_daemon_protocol.h"

typedef struct msg_data_request_firmware_upgrade_s{
	skt_msg_header_t header;
	char data[8*1024*1024];
} msg_data_request_firmware_upgrade_t;

typedef struct msg_data_ack_firmware_upgrade_s{
	skt_msg_header_t header;
	char data[4];
} msg_data_ack_firmware_upgrade_t;

msg_data_request_firmware_upgrade_t msgmsg_request_firmware_upgrade;
msg_data_ack_firmware_upgrade_t msgmsg_ack_firmware_upgrade;

int main(int argc, char *argv[])
{
	if (argc != 4) {
		printf("usage: ./client [server IP] [port] [file_path]\n");
		return -1;
	}

	int ret = 0;
	void *cskt;
	cskt = socket_client_tcp_alloc(argv[1], atoi(argv[2]));
	assert(cskt != NULL);
	msgmsg_request_firmware_upgrade.header.cmd = SET_SYS_FIRMWARE_SEND;
	msgmsg_request_firmware_upgrade.header.flags = 0;

	//get upgrade file data and len
	int file_len = 0;
	FILE *fp = fopen(argv[3], "r");
	if (!fp) {
		printf("open file: %s failed\n", argv[3]);
		return -1;
	}
	if(fseek(fp, 0, SEEK_END)) {
		printf("fseek to calc file lenght failed\n");
		return -1;
	}
	file_len = ftell(fp);
	if(fseek(fp, 0, SEEK_SET)) {
		printf("fseek to calc file lenght failed\n");
		return -1;
	}
	fread(msgmsg_request_firmware_upgrade.data, 1, file_len, fp);
	fclose(fp);

	//send upgrade file to server and wait for server ack
	msgmsg_request_firmware_upgrade.header.len = file_len;
	ret = socket_client_tcp_send(cskt, (skt_msg_t*)&msgmsg_request_firmware_upgrade);
	if (ret < 0) {
		printf("upgrade failed: send upgrade package failed\n");
		return -1;
	}
	ret = socket_client_tcp_recv(cskt, (skt_msg_t*)&msgmsg_ack_firmware_upgrade, sizeof(msgmsg_ack_firmware_upgrade.data));
	if (ret < 0) {
		printf("upgrade failed: recv upgrade ack failed\n");
		return -1;
	}

	//check ack
	skt_msg_upgrade_ack_t *upgrade_ack = (skt_msg_upgrade_ack_t *)&msgmsg_ack_firmware_upgrade;
	printf("\nGet ack msg:\nheader.cmd=0x%08x\nheader.len=%d\nheader.flags=0x%08x\nstatus=%d\n",
			upgrade_ack->header.cmd, upgrade_ack->header.len, upgrade_ack->header.flags, upgrade_ack->status);
	if (upgrade_ack->status == CMD_ACK_STATUS_SUCCESS && upgrade_ack->header.cmd == SET_SYS_FIRMWARE_ACK) {
		printf("upgrade success, server closed!\n");
		close(((socket_client_tcp_t *)cskt)->skt_client);
	} else {
		printf("upgrade failed: recv wrong ack\n");
		close(((socket_client_tcp_t *)cskt)->skt_client);
		return -1;
	}

	return 0;
}
