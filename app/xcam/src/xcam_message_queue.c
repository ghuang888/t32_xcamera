#include <stdio.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <unistd.h>
#include "xcam_general.h"
#include "xcam_message_queue.h"
#include "xcam_network.h"
#include "xcam_video.h"

int _xcam_msg_set_ipaddr_process(xcam_msg_data_t *pstRecvData)
{
	assert(pstRecvData != NULL);
	int ret = XCAM_SUCCESS;
	xcam_msg_set_ipaddr_t *pIpaddr = NULL;

	pIpaddr = (xcam_msg_set_ipaddr_t *)&pstRecvData->data;
	sleep(3);
	printf("recv port interface:%s,ipaddr:%s\n",pIpaddr->interface,pIpaddr->ipaddr);
	ret = xcam_network_set_device_static_ip(pIpaddr->interface,pIpaddr->ipaddr);
	if (ret < XCAM_SUCCESS) {
		return ret;
	}
	sleep(2);
	ret = xcam_video_restart_rtsp();

	return ret;
}

int xcam_msg_information_process(xcam_msg_data_t *pstRecvData)
{
	int ret = XCAM_SUCCESS;

	switch (pstRecvData->info_flag) {
		case XCAM_MESSAGE_SET_IPADDR :
			ret = _xcam_msg_set_ipaddr_process(pstRecvData);
			break;
		default :
			break;
	}

	return ret;
}

int xcam_message_send(xcam_msg_data_t *info_data_t)
{
	int msgid;
	key_t key;
	msg_t msgbuff;
	memset(&msgbuff, 0, sizeof(msg_t));

	key = ftok("/bin", 1);

	msgid = msgget(key, IPC_CREAT | IPC_EXCL | 0664);
	if (msgid == -1) {
		if (errno == EEXIST) {
			//printf("mesage already exist.\n");
			msgid = msgget(key, 0664);
		} else {
			//perror("fail to msgget\n");
			return XCAM_ERROR;
		}
	}

	msgbuff.type = MSGTYPE_SEND ; // 定义消息类型
	memcpy(&msgbuff.transData, info_data_t, sizeof(xcam_msg_data_t));

	msgsnd(msgid, &msgbuff, sizeof(xcam_msg_data_t), 0); //发送消息 .
/*	这部分注释的代码是双向通信的时候使用，目前没有这个需求就先注释了
 *	memset(&msgbuff, 0, sizeof(msg_t));
 *	msgrcv(msgid, &msgbuff, sizeof(msg_t) - sizeof(long), MSGTYPE_RECV,0 );
 *	memcpy(info_data_t, &msgbuff.transData, sizeof(xcam_web_info_t));
 */
	return XCAM_SUCCESS;
}

int xcam_message_receive()
{
	int ret = XCAM_SUCCESS;

	key_t key ;
	int msgid ;
	msg_t msgbuff;

	memset(&msgbuff.transData, 0, sizeof(xcam_msg_data_t));

	key = ftok("/bin", 1);

	msgid = msgget(key, IPC_CREAT | IPC_EXCL | 0664);
	if (msgid == -1) {
		if (errno == EEXIST) {
			msgid = msgget(key, 0664);
		} else {
			perror("fail to msgget\n");
			return XCAM_ERROR ;
		}
	}

	while(1) {
		memset(&msgbuff,0,sizeof(msg_t));
		msgrcv(msgid, &msgbuff, sizeof(msg_t) - sizeof(long), MSGTYPE_SEND, 0);
		printf("msgbuff->type:%ld\n",msgbuff.type);
		if (msgbuff.type != MSGTYPE_SEND) {
			break;
		}
		ret = xcam_msg_information_process(&msgbuff.transData);

	/*	msgbuff.type = MSGTYPE_SEND; // 定义消息类型
		msgsnd(msgid, &msgbuff, sizeof(xcam_msg_data_t), 0); //发送消息*/
	}

	msgctl(msgid, IPC_RMID, NULL);
	return ret;
}

