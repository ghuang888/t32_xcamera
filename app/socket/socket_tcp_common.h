#ifndef _SOCKET_TCP_COMMON_H_
#define _SOCKET_TCP_COMMON_H_

#include <stdint.h>
#include <pthread.h>

#define SOCKET_TCP_MSG_START_CODE (0x5a5aa5a5)

#define MSG_FLAGS_DATA_ENDING (1<<0)

typedef struct skt_msg_header_s {
	uint32_t cmd;
	//data len, not include header
	uint32_t len;
	uint32_t flags;
} skt_msg_header_t;

typedef struct skt_msg_t {
	skt_msg_header_t header;
	char data[0];
} skt_msg_t;

struct socket_tcp_s;

typedef struct socket_tcp_link_s {
	unsigned int state;
	int skt_client;
	pthread_t tid_link;
	skt_msg_t *msgbuf;
	struct socket_tcp_s *skt;
} socket_tcp_link_t;

typedef int (*msg_process_func_t)(socket_tcp_link_t *);

typedef struct socket_tcp_s {
	unsigned int port;
	int skt_server;
	int maxlink;
	int bufsize;
	pthread_t tid_server;
	msg_process_func_t fun;
	socket_tcp_link_t *links;
	char *msgbufs;
} socket_tcp_t;

void* socket_server_tcp_alloc(unsigned int port, int maxlink);
int socket_server_tcp_start(socket_tcp_t *s);
int socket_server_tcp_set_msg_process_cb(socket_tcp_t *s, msg_process_func_t fun);
int socket_server_tcp_send(socket_tcp_link_t *plink, skt_msg_t *msg);

//client
typedef struct socket_client_tcp_s {
	int skt_client;
	unsigned int port;
	char serverip[50];
} socket_client_tcp_t;

void* socket_client_tcp_alloc(char *server_ip, unsigned int port);
int socket_client_tcp_send(socket_client_tcp_t *skt, skt_msg_t *msg);
int socket_client_tcp_recv(socket_client_tcp_t *skt, skt_msg_t *msg, int buf_len);

#endif
