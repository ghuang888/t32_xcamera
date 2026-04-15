#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>

/*客户端发送和接受对应服务端接受和发送*/
#define MSG_FTOK_RCV_KEY 124
#define MSG_FTOK_SND_KEY 123

typedef struct msgbuf {
    long mtype;
    char mtext[1024];
}MSGBUF;

int msg_snd(int msgid,int msgtype,void* pdata)
{
    if(NULL == pdata){
        printf("[%s][%d]pdata is NULL\n",__func__,__LINE__);
        return -1;
    }
    MSGBUF stmsgsnd;
    //time_t t;

    stmsgsnd.mtype = msgtype;

    //time(&t);
    //snprintf(stmsgsnd.mtext, sizeof(stmsgsnd.mtext), "a message at %s",ctime(&t));
    snprintf(stmsgsnd.mtext, sizeof(stmsgsnd.mtext), "%s",(char*)pdata);

    if (msgsnd(msgid, (void *) &stmsgsnd, sizeof(stmsgsnd.mtext),0) == -1) {
       perror("msgsnd error");
       exit(EXIT_FAILURE);
    }
    //printf("clint sent: %s\n", stmsgsnd.mtext);
    return 0;
}

int msg_get(int msgid, int msgtype)
{
    MSGBUF stmsgbuf;
    memset(&stmsgbuf,0x0,sizeof(MSGBUF));
    if (msgrcv(msgid, (void *) &stmsgbuf, sizeof(stmsgbuf.mtext), msgtype,0) == -1) {
        if (errno != ENOMSG) {
            perror("msgrcv");
            exit(EXIT_FAILURE);
        }
        printf("No message available for msgrcv()\n");
    } else {
        printf("%s\n", stmsgbuf.mtext);
    }

    return 0;

}

int msg_server_start(void *arg)
{
    int msgid_rcv,msgid_snd;
    key_t key_rcv,key_snd;
    int msgtype = 1;
    int ret = 0;

    key_rcv = ftok("/bin",MSG_FTOK_RCV_KEY);
    key_snd = ftok("/bin",MSG_FTOK_SND_KEY);

    msgid_rcv = msgget(key_rcv, IPC_CREAT | 0666);
    msgid_snd = msgget(key_snd, IPC_CREAT | 0666);
    if (msgid_rcv == -1 || msgid_snd == -1) {
        perror("msgget");
        exit(EXIT_FAILURE);
    }


    ret = msg_snd(msgid_snd,msgtype,arg);
    if(ret < 0){
        printf("msg_snd err\n");
        return -1;
    }

#if 1
    //这里的msgtype是否需要=0
    msgtype = 0;
    ret = msg_get(msgid_rcv,msgtype);
    if(ret < 0){
        printf("msg_get err\n");
        return -1;
    }
#endif

    return 0;

}

void cJson_msg_start(void* arg)
{
     int ret = 0;
     ret = msg_server_start(arg);
     if(0 != ret ){
         printf("msg_server_start run err\n");
         return ;
     }

     return ;
}

#if 0
void msg_process_init(void)
{
    xcam_thread_create("msg_main",cJson_msg_start,NULL);
    return;
}
#endif

#if 1
int main(int argc, char *argv[])
{
#if 0
    if(2 != argc){
        printf("use method:./a.out file.json\n");
        return -1;
    }
    FILE *fp = NULL;
    fp = fopen(argv[1],"r");
    fseek(fp,0,SEEK_END);
    int size = ftell(fp);
    char *pdata =(char*)malloc(size);
    fseek(fp,0,SEEK_SET);
    fread(pdata,1,size,fp);
    printf("pdata:%s\n", pdata);
#endif

    char *plen = NULL,*pdata = NULL;
    int bufsize = 0;
    printf("Content-type: application/json;charset='utf-8'\n\n");
    plen = getenv("CONTENT_LENGTH");
    if(NULL == plen){
        printf("plen is null\n");
        return -1;
    }
    bufsize = atoi(plen) + 1;

    pdata = calloc(1,bufsize);
    if(NULL == pdata){
        printf("[%s][%d]pdata is null\n",__func__,__LINE__);
        return -1;
    }
    (void)fgets(pdata,bufsize,stdin);
    cJson_msg_start(pdata);
    sleep(1);
    free(pdata);
    return 0;
}
#endif
