#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
//#include <pthread.h>
#include "../inc/conf_process.h"

#define MSG_FTOK_RCV_KEY 123
#define MSG_FTOK_SND_KEY 124
/*MAX_RETSTR_LEN,若字符大于这个长度，需要修改这里,最大不超过8192*/
#define MAXSTRSIZE 2048

typedef struct msgbuf {
    long mtype;
    char mtext[MAXSTRSIZE];
}MSGBUF;

int msg_snd(int msgid,int msgtype,char* pstdst)
{
    if(NULL == pstdst){
        printf("[%s][%d]pstdst is NULL\n",__func__,__LINE__);
        return -1;
    }

    MSGBUF stmsgsnd;
    memset(&stmsgsnd,0x0,sizeof(stmsgsnd));
    //time_t t;

    stmsgsnd.mtype = msgtype;
    if(sizeof(stmsgsnd.mtext) > strlen(pstdst)+1){
        memcpy(stmsgsnd.mtext,pstdst,strlen(pstdst)+1);
    } else {
        printf("[%s][%d] pstdst'len out range\n",__func__,__LINE__);
        return -2;
    }

    //time(&t);
    //snprintf(stmsgsnd.mtext, sizeof(stmsgsnd.mtext), "a message at %s",ctime(&t));

    //send data to client
    if (msgsnd(msgid, (void *) &stmsgsnd, sizeof(stmsgsnd.mtext),0) != 0) {
       perror("msgsnd error");
       exit(EXIT_FAILURE);
    }
    printf("server sent: %d %d \n",msgid, sizeof( stmsgsnd.mtext));

    return 0;
}

int msg_get(int msgid, int msgtype,char* pstret)
{
    if(NULL == pstret){
        printf("[%s][%d]pstret is NULL\n",__func__,__LINE__);
        return -1;
    }
    memset(pstret,0x0,strlen(pstret)+1);

    MSGBUF stmsgbuf;
    memset(&stmsgbuf,0x0,sizeof(MSGBUF));

    //flag == 0,wait mode
    if (msgrcv(msgid, (void *) &stmsgbuf, sizeof(stmsgbuf.mtext), msgtype,0) == -1) {
        if (errno != ENOMSG) {
            perror("msgrcv");
            exit(EXIT_FAILURE);
        }
        printf("No message available for msgrcv()\n");
    } else {

        printf("sever received: %s\n", stmsgbuf.mtext);
        memcpy(pstret,stmsgbuf.mtext,sizeof(stmsgbuf.mtext));
    }

    return 0;

}

int msg_server_start()
{
    int msgid_rcv,msgid_snd;
    key_t key_rcv,key_snd;
    int msgtype = 0;//标志为0 ，返回队列中最早的一个消息
    int ret = 0;
    char* pstsrc = NULL,*pstdst = NULL;

    key_rcv = ftok("/bin",MSG_FTOK_RCV_KEY);
    key_snd = ftok("/system/bin",MSG_FTOK_SND_KEY);
    if(-1 == key_rcv || -1 == key_snd){
        printf("[%s][%d]ftok err\n",__func__,__LINE__);
        return -1;
    }

    msgid_rcv = msgget(key_rcv, IPC_CREAT | 0666);
    msgid_snd = msgget(key_snd, IPC_CREAT | 0666);
    if (-1 == msgid_rcv || -1 == msgid_snd ) {
        perror("msgget");
        exit(EXIT_FAILURE);
    }

    pstsrc = (char*)malloc(MAXSTRSIZE);
    pstdst = (char*)malloc(MAXSTRSIZE);
    if(NULL == pstsrc || NULL == pstdst){
        printf("[%s][%d]malloc err\n",__func__,__LINE__);
        return -1;
    }
    memset(pstsrc,0x0,MAXSTRSIZE);
    memset(pstdst,0x0,MAXSTRSIZE);

    while(1)
    {
        msgtype = 0;
        ret = msg_get(msgid_rcv,msgtype,pstsrc);
        if(ret < 0){
            printf("[%s][%d]msg_get err\n",__func__,__LINE__);
            return -2;
        }
        //printf("sever rcv client:%s,start conf_process\n",pstsrc);

        pthread_mutex_lock(&g_conf_mutex_t);
        ret = conf_process(pstsrc,pstdst);
        if(ret < 0){
            printf("[%s][%d]conf_process run err\n",__func__,__LINE__);
            msgtype = 1;
            pthread_mutex_unlock(&g_conf_mutex_t);
            msg_snd(msgid_snd,msgtype,"error param");
        }else{
            pthread_mutex_unlock(&g_conf_mutex_t);

            //这里的msgtype必须大于0
            msgtype = 1;
            ret = msg_snd(msgid_snd,msgtype,pstdst);
            if(ret < 0){
                printf("msg_snd err\n");
                return -1;
            }
        }
        

    }

    free(pstsrc);
    free(pstdst);
    return 0;

}

void* cJson_msg_start(void* arg)
{
     int ret = 0;
     ret = msg_server_start();
     if(0 != ret ){
         printf("msg_server_start run err\n");
         return NULL;
     }

     return NULL;
 }

#if 1
void msg_process_init(void)
{
    xcam_thread_create("msg_main",cJson_msg_start,NULL);
    return;
}
#endif

#if 0

int main()
{
    cJson_msg_start(NULL);
    return 0;
}

#endif
