#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <memory.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "socket_tcp_common.h"

#define RV_SOCKET_CLOSE 1
#define RV_SUCCESS      0
#define RV_ERR_FAIL     -1

int _receive_data(int socket, char *buf, int len)
{
	int ret = 0;
	int size = 0;
	char *recv_buf = buf;
	int recv_len = len;

retry_recv_data:

	size = recv(socket, recv_buf, recv_len, 0);
	if (-1 == size) {
		printf("client: socket read failed\n");
		return RV_ERR_FAIL;
	}
	if (0 == size) {
		printf("client: socket closed\n");
		return RV_SOCKET_CLOSE;
	}
	if (size != recv_len) {
		recv_len -= size;
		recv_buf += size;
		goto retry_recv_data;
	}
	return RV_SUCCESS;

}
void* socket_client_tcp_alloc(char *server_ip, unsigned int port)
{
	int ret = 0;
	char *rv;
	struct sockaddr_in server_addr;
	int skt_client;
	socket_client_tcp_t *cskt;
	char *buf = calloc(1, sizeof(socket_client_tcp_t));
	if (NULL == buf) {
		printf("err(%s,%d): calloc failed\n", __func__, __LINE__);
		rv = NULL;
		goto err_skt_malloc;
	}
	cskt = (socket_client_tcp_t *)buf;
	strncpy(cskt->serverip, server_ip, 50);

	skt_client = socket(AF_INET, SOCK_STREAM, 0);
	if (0 > skt_client) {
		printf("err(%s,%d): client socket create failed\n", __func__, __LINE__);
		rv = NULL;
		goto err_skt_create;
	}

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = inet_addr(cskt->serverip);

	cskt->port = port;

	ret = connect(skt_client, (struct sockaddr *)&server_addr, sizeof(struct sockaddr));
	if (0 != ret) {
		printf("err(%s,%d): client socket connect failed\n", __func__, __LINE__);
		rv = NULL;
		goto err_skt_connect;
	}
	cskt->skt_client = skt_client;

	return cskt;

err_skt_connect:
err_skt_create:
	free(cskt);
err_skt_malloc:
	return rv;
}

int socket_client_tcp_send(socket_client_tcp_t *skt, skt_msg_t *msg)
{
	int ret = 0;
	uint32_t start_code;
	int skt_client;
	int len;
	if (NULL == skt) {
		printf("err(%s,%d): socket client handle\n",
				__func__, __LINE__);
		return -1;
	}
	start_code = SOCKET_TCP_MSG_START_CODE;
	skt_client = skt->skt_client;
	ret = send(skt_client, &start_code, sizeof(start_code), 0);
	if (-1 == ret) {
		printf("err(%s,%d): socket client send\n",
				__func__, __LINE__);
		return -1;
	}
	if (ret != sizeof(start_code)) {
		printf("err(%s,%d): socket client send want %ld, real %d\n",
				__func__, __LINE__, sizeof(start_code), ret);
	}

	len = sizeof(skt_msg_t) + msg->header.len;
	ret = send(skt_client, msg, len, 0);
	if (-1 == ret) {
		printf("err(%s,%d): socket client send\n",
				__func__, __LINE__);
		return -1;
	}
	if (ret != len) {
		printf("err(%s,%d): socket client send want %ld, real %d\n",
				__func__, __LINE__, sizeof(start_code), ret);
	}
	//printf("send: size = %d\n", len);
	return 0;
}

//buf_len is the lenght of msg->data
int socket_client_tcp_recv(socket_client_tcp_t *skt, skt_msg_t *msg, int buf_len)
{
	int size = 0;
	int skt_client;
	int receive_state = 0;
	int tmpsize = 0, total_len = 0;
	if (NULL == skt) {
		printf("err(%s,%d): socket client handle\n",
				__func__, __LINE__);
		return -1;
	}
	skt_client = skt->skt_client;

	for (;;) {
		uint32_t start_code;
		switch (receive_state) {
		case 0:
			start_code = 0;
			size = _receive_data(skt_client, (char*)&start_code, 4);
			if (RV_ERR_FAIL == size) {
				printf("client: socket read failed\n");
				sleep(1);
				continue;
			}
			if (RV_SOCKET_CLOSE == size) {
				printf("client: socket closed\n");
				goto err;
			}
#if 0
			if (size != 4) {
				printf("warn(%s,%d): get start code failed size = %d\n",
						__func__, __LINE__, size);
			}
#endif
			if (start_code == SOCKET_TCP_MSG_START_CODE) {
				//printf("start code = 0x%08x\n", start_code);
				receive_state = 1;
			} else {
				printf("warn(%s,%d): want start code 0x%08x, but  0x%08x\n",
						__func__, __LINE__, SOCKET_TCP_MSG_START_CODE, start_code);
			}
			break;
		case 1:
			size = _receive_data(skt_client, (char*)&msg->header, sizeof(skt_msg_header_t));
			if (RV_ERR_FAIL == size) {
				printf("client: socket read failed\n");
				sleep(1);
				continue;
			}
			if (RV_SOCKET_CLOSE == size) {
				printf("client: socket closed\n");
				goto err;
			}
#if 0
			if (size != sizeof(header)) {
				printf("err(%s,%d): socket recv failed\n", __func__, __LINE__);
				printf("%d\n", size);
			}
#endif
			tmpsize = msg->header.len;
			if (tmpsize > buf_len) {
				printf("tmpsize:%d,buf_len:%d\n",tmpsize,buf_len);
				printf("err(%s,%d): recv data lenght bigger than msg->data lenght that we provide\n", __func__, __LINE__);
				goto err;
			}
			receive_state = 2;
			break;
		case 2:
			size = _receive_data(skt_client, msg->data, tmpsize);
			if (RV_ERR_FAIL == size) {
				printf("client: socket read failed\n");
				sleep(1);
				continue;
			}
			if (RV_SOCKET_CLOSE == size) {
				printf("client: socket closed\n");
				goto err;
			}
#if 0
			if (size != tmpsize) {
				printf("err(%s,%d): socket recv failed, want %d, get %d\n",
						__func__, __LINE__, tmpsize, size);
			}
#endif
			total_len = tmpsize + sizeof(skt_msg_header_t);
		//	printf("client recv size = %d\n", total_len);
			goto done;
		default:
			printf("err(%s,%d): error state\n", __func__, __LINE__);
			goto err;
		}
	}
err:
	close(skt_client);
	return -1;
done:
	return 0;
}
