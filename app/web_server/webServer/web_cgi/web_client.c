#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <assert.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "socket_tcp_common.h"
#include "xcam_daemon_protocol.h"

#define WEB_SUCCESS 0
#define WEB_ERROR -1

struct msg_data_ack_cfg_s {
	skt_msg_header_t header;
	char data[2000];
};

int xcam_network_get_net_interface(char *pInterface)
{
	char interface [20] = {0};
	FILE *fp = NULL;
	char cmd[128] = {0};
	int i = 0;

	sprintf(cmd,"cat /proc/net/dev | awk '{i++; if(i>2){print $1}}' | sed 's/^[\t]*//g' | sed 's/[:]*$//g'|grep -v lo");

	fp = popen(cmd,"r");
	if ( fp == NULL ) {
		return WEB_ERROR;
	}

	fread(interface,1,19,fp);
	memcpy(pInterface,interface,sizeof(interface));

	char *temp = pInterface;
	for (i = 0; i < 47; i++) {
		if (*temp == '\0')
			break;
		if (*temp == '\n')
			*temp = '\0';
		temp ++;
	}

	pclose(fp);

	return WEB_SUCCESS;
}

int xcam_network_get_device_ip(char* interface, char* addr)
{
	int ret = WEB_SUCCESS;
    int sock = -1;
	struct ifreq ifr;

	memset(&ifr, 0, sizeof(struct ifreq));

	sock = socket(AF_INET,SOCK_DGRAM,0);
	if (sock < 0) {
		return WEB_ERROR;
	}

    strcpy(ifr.ifr_name, interface);
	ret = ioctl(sock, SIOCGIFADDR, &ifr);
	if (ret < WEB_SUCCESS) {
		close(sock);
		return ret;
	}

	strcpy(addr, inet_ntoa(((struct sockaddr_in *)&(ifr.ifr_addr))->sin_addr));

	close(sock);
	return ret;
}

int web_xcame_cfg_process(skt_msg_t *msg)
{
	int ret = WEB_SUCCESS;
	void *clientCfg = NULL;
	struct msg_data_ack_cfg_s recv_msg;
	skt_msg_t* recvMsg = NULL;
	char interface[20] = {0},ip_addr[20] = {0};

	recvMsg = (skt_msg_t *)&recv_msg;
	memset(recvMsg, 0, sizeof(skt_msg_t));

	ret = xcam_network_get_net_interface(interface);
	if (ret < WEB_SUCCESS) {
		printf("get interface fail.\n");
		return WEB_ERROR;
	}

	ret = xcam_network_get_device_ip(interface, ip_addr);
	if (ret < WEB_SUCCESS) {
		printf("get ipaddr fail.\n");
		return WEB_ERROR;
	}

	clientCfg = socket_client_tcp_alloc(ip_addr,51000);
	if (clientCfg == NULL) {
		printf("create socket client fail.\n");
		ret = WEB_ERROR;
	}

	ret = socket_client_tcp_send(clientCfg, msg);
	if (ret < WEB_SUCCESS) {
		printf("send message fail.\n");
	}

	ret = socket_client_tcp_recv(clientCfg, recvMsg, sizeof(recv_msg.data));

	if (ret < WEB_SUCCESS ) {
		printf("recv message fail.\n");
		close(((socket_client_tcp_t *)clientCfg)->skt_client);
		return ret;
	}

	if (recvMsg->header.cmd != CMD_WEB_CFGS_ACK) {
		printf("recv data cmd error");
		close(((socket_client_tcp_t *)clientCfg)->skt_client);
		return WEB_ERROR;
	}

	printf ("%s\n",recvMsg->data);

	close(((socket_client_tcp_t *)clientCfg)->skt_client);
	return ret;
}

int main (int argc , char *argv[])
{
	int ret = WEB_SUCCESS;
	skt_msg_t *msg = NULL;
	char *buf = NULL;
	char *len = NULL;
	int bufsize = 0;
	int msgsize = sizeof(skt_msg_t);
	printf("Content-type: application/json;charset='utf-8'\n\n");
	len = getenv("CONTENT_LENGTH");
	bufsize = atoi(len)+1;
	msg = (skt_msg_t *)calloc(1,sizeof(skt_msg_t)+bufsize);

	(void)fgets(msg->data,bufsize,stdin);
	msg->header.cmd = CMD_WEB_CFGS_SEND;
	msg->header.flags = MSG_FLAGS_DATA_ENDING;
	msg->header.len = bufsize;
//	printf("11112:%d\n",len);
	//printf("11111:%s\n",msg->data);
	ret = web_xcame_cfg_process(msg);
	if (ret < WEB_SUCCESS) {
		printf("transfor message fail.\n");
		free(buf);
		return ret;
	}

	free(buf);
	return ret;
}
