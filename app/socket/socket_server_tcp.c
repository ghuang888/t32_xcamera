#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <memory.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <assert.h>
#include <pthread.h>
//#include <netinet/in.h>
//#include <signal.h>
#include "socket_tcp_common.h"

#define SOCKET_TCP_DEFAULT_PORT 11910
#define SOCKET_TCP_DEFAULT_BUF_SIZE 2000
#define SOCKET_TCP_DEFAULT_MAX_LINK 5

#define SOCKET_TCP_LINK_STATE_INIT 0
#define SOCKET_TCP_LINK_STATE_RUN 1
#define SOCKET_TCP_LINK_STATE_CLOSE 2

#define RV_CLIENT_CLOSE	1
#define RV_SUCCESS		0
#define RV_ERR_FAIL		-1

int _receive_data(int socket, char *buf, int len)
{
	int size = 0;
	char *recv_buf = buf;
	int recv_len = len;

retry_recv_data:

	size = recv(socket, recv_buf, recv_len, 0);
	if (-1 == size) {
		printf("server: socket read failed\n");
		return RV_ERR_FAIL;
	}
	if (0 == size) {
		printf("server: client closed\n");
		return RV_CLIENT_CLOSE;
	}
	if (size != recv_len) {
		recv_len -= size;
		recv_buf += size;
		goto retry_recv_data;
	}
	return RV_SUCCESS;
}

int socket_server_tcp_send(socket_tcp_link_t *plink, skt_msg_t *msg)
{
	int ret = 0;
	uint32_t start_code;
	int skt_client;
	int len;
	if (NULL == plink) {
		printf("err(%s,%d): socket server handle\n",
				__func__, __LINE__);
		return -1;
	}
	start_code = SOCKET_TCP_MSG_START_CODE;
	skt_client = plink->skt_client;
	ret = send(skt_client, &start_code, sizeof(start_code), 0);
	if (-1 == ret) {
		printf("err(%s,%d): socket server send\n",
				__func__, __LINE__);
		return -1;
	}
	if (ret != sizeof(start_code)) {
		printf("err(%s,%d): socket server send want %d, real %d\n",
				__func__, __LINE__, sizeof(start_code), ret);
	}

	len = sizeof(skt_msg_header_t) + msg->header.len;
	ret = send(skt_client, msg, len, 0);
	if (-1 == ret) {
		printf("err(%s,%d): socket server send\n",
				__func__, __LINE__);
		return -1;
	}
	if (ret != len) {
		printf("err(%s,%d): socket server send want %d, real %d\n",
				__func__, __LINE__, len, ret);
	}
	printf("send: size = %d\n", len);
	return 0;
}

void *socket_tcp_link(void *param)
{
	int err = 0;
	int size = 0;

	socket_tcp_link_t *plink = (socket_tcp_link_t *)param;
	socket_tcp_t *pskt = plink->skt;
	skt_msg_t *msgbuf = plink->msgbuf;

	int skt_client = plink->skt_client;
	int bufsize = SOCKET_TCP_DEFAULT_BUF_SIZE;
	int receive_state = 0;
	char* buf = msgbuf->data;
	int total_len = 0;
	int left_len = 0;
	pthread_detach(pthread_self());
	for (;;) {
		int tmpsize = 0;
		uint32_t start_code;
		msg_process_func_t fun = pskt->fun;
		skt_msg_header_t header;
		switch (receive_state) {
		case 0:
			start_code = 0;
			size = _receive_data(skt_client, (char*)&start_code, 4);
			if (RV_ERR_FAIL == size) {
				printf("server: socket read failed\n");
				sleep(1);
				continue;
			}
			if (RV_CLIENT_CLOSE == size) {
				printf("_receive_data size equal thread ,server: client closed\n");
				goto done;
			}
#if 0
			if (size != 4) {
				printf("warn(%s,%d): get start code failed size = %d\n",
						__func__, __LINE__, size);
			}
#endif
			if (start_code == SOCKET_TCP_MSG_START_CODE) {
				receive_state = 1;
			} else {
				printf("warn(%s,%d): want start code 0x%08x, but  0x%08x\n",
						__func__, __LINE__, SOCKET_TCP_MSG_START_CODE, start_code);
			}
			break;
		case 1:
			size = _receive_data(skt_client, (char*)&header, sizeof(header));
			if (RV_ERR_FAIL == size) {
				printf("server: socket read failed\n");
				sleep(1);
				continue;
			}
			if (RV_CLIENT_CLOSE == size) {
				printf("server: client closed\n");
				goto done;
			}
#if 0
			if (size != sizeof(header)) {
				printf("err(%s,%d): socket recv failed\n", __func__, __LINE__);
			}
#endif
			left_len = header.len;
			total_len = left_len + sizeof(header);
			receive_state = 2;
			break;
		case 2:
			if (left_len > bufsize) {
				tmpsize = bufsize;
				left_len -= bufsize;
			} else {
				tmpsize = left_len;
				left_len = 0;
			}
			size = _receive_data(skt_client, buf, tmpsize);
			if (RV_ERR_FAIL == size) {
				printf("server: socket read failed\n");
				sleep(1);
				continue;
			}
			if (RV_CLIENT_CLOSE == size) {
				printf("server: client closed\n");
				goto done;
			}
#if 0
			if (size != tmpsize) {
				printf("err(%s,%d): socket recv failed, want %d, get %d\n",
						__func__, __LINE__, tmpsize, size);
			}
#endif
			msgbuf->header.cmd = header.cmd;
			msgbuf->header.len = tmpsize;
			msgbuf->header.flags = 0;
			if (!left_len)
				msgbuf->header.flags |= MSG_FLAGS_DATA_ENDING;
			if (fun) {
				err = fun(plink);
				if (err < 0) {
					printf("server: msg process error\n");
					receive_state = 0;
				}
			}
			if (0 == left_len) {
				receive_state = 0;
				printf("recv tlen = %d\n", total_len);
			}
			break;
		default:
				printf("err(%s,%d): error state\n", __func__, __LINE__);
		}
	}
done:
	plink->state = SOCKET_TCP_LINK_STATE_INIT;
	close(plink->skt_client);
	plink->skt_client = 0;
	return NULL;
}

void *socket_tcp_server(void *param)
{
	int err = 0;
	int skt_client;
	struct sockaddr_in clinet_addr;
	socket_tcp_t *pskt = (socket_tcp_t *)param;
	while (1) {
		err = listen(pskt->skt_server, pskt->maxlink);
		if (0 > err) {
			printf("err(%s,%d): socket listen\n", __func__, __LINE__);
			sleep(3);
			continue;
		}
		socklen_t addrlen = sizeof(clinet_addr);
		skt_client = accept(pskt->skt_server, (struct sockaddr *)&clinet_addr, &addrlen);
		if (0 > skt_client) {
			printf("err(%s,%d): socket accept\n", __func__, __LINE__);
			sleep(3);
			continue;
		} else {
			printf("info: socket accept\n");
			int i = 0;
			int cnt = pskt->maxlink;
			socket_tcp_link_t *link = NULL;
			skt_msg_t *msgbuf = NULL;
			socket_tcp_link_t *links = pskt->links;
			for (i = 0; i < cnt; i++ ) {
				if (SOCKET_TCP_LINK_STATE_INIT == links[i].state) {
					links[i].state = SOCKET_TCP_LINK_STATE_RUN;
					link = &links[i];
					msgbuf = (skt_msg_t *)((char*)pskt->msgbufs+(i*(sizeof(skt_msg_t)+SOCKET_TCP_DEFAULT_BUF_SIZE)));
					break;
				}
			}
			if (NULL == link) {
				printf("error(%s,%d): find empty link failed\n", __func__, __LINE__);
			} else {
				link->skt_client = skt_client;
				link->skt = pskt;
				link->msgbuf = msgbuf;
				err = pthread_create(&link->tid_link, NULL, socket_tcp_link, link);
				if (0 != err) {
					printf("error(%s,%d): thread create failed %s\n", __func__, __LINE__, strerror(errno));
				}
				continue;
			}
		}
	}
}

void* socket_server_tcp_alloc(unsigned int port, int maxlink)
{
	int ret;
	int maxl;
	int skt_server;
	struct sockaddr_in server_addr;
	void *rv;
	int bufsize = 0;
	socket_tcp_t *pskt;

	maxl = maxlink<=0?SOCKET_TCP_DEFAULT_MAX_LINK:maxlink;
	bufsize = sizeof(socket_tcp_t);
	bufsize += maxl*sizeof(socket_tcp_link_t);
	bufsize += maxl*(sizeof(skt_msg_t)+SOCKET_TCP_DEFAULT_BUF_SIZE);
	char *buf = calloc(1, bufsize);
	if (NULL == buf) {
		printf("err(%s,%d): calloc failed\n", __func__, __LINE__);
		rv = NULL;
		goto err_skt_malloc;
	}
	pskt = (socket_tcp_t *)buf;
	pskt->links = (socket_tcp_link_t*)(buf+sizeof(socket_tcp_t));
	pskt->msgbufs = buf+sizeof(socket_tcp_t)+maxl*sizeof(socket_tcp_link_t);
	skt_server = socket(AF_INET, SOCK_STREAM, 0);
	if (0 > skt_server) {
		printf("err(%s,%d): socket failed\n", __func__, __LINE__);
		rv = NULL;
		goto err_skt_socket;
	}
	pskt->skt_server = skt_server;
	pskt->maxlink = maxlink;
	pskt->bufsize = SOCKET_TCP_DEFAULT_BUF_SIZE;
	pskt->port = port;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(port);

	ret = bind(skt_server, (struct sockaddr *)&server_addr, sizeof(struct sockaddr));
	if (0 > ret) {
		printf("err(%s,%d): socket bind failed\n", __func__, __LINE__);
		rv = NULL;
		goto err_skt_bind;
	}
	return pskt;

err_skt_bind:
err_skt_socket:
	free(pskt);
err_skt_malloc:
	return rv;

}

int socket_server_tcp_start(socket_tcp_t *skt)
{
	int ret;
	if (NULL == skt) {
		printf("err(%s,%d): socket server tcp start\n", __func__, __LINE__);
		return -1;
	}
	ret =  pthread_create(&skt->tid_server, NULL, socket_tcp_server, skt);
	if (0 != ret) {
		printf("error(%s,%d): thread create failed %s\n", __func__, __LINE__, strerror(errno));
		return -1;
	}
	return 0;
}

int socket_server_tcp_set_msg_process_cb(socket_tcp_t *skt, msg_process_func_t fun)
{
	if (NULL == skt) {
		printf("err(%s,%d): socket server tcp set msg process fun\n", __func__, __LINE__);
		return -1;
	}
	skt->fun = fun;
	return 0;
}
