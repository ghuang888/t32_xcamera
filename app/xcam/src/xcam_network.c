#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <sys/types.h>
#include <dirent.h>
#include <assert.h>
#include <sys/reboot.h>
#include "xcam_conf_network.h"
#include "xcam_general.h"
#include "xcam_log.h"
#include "xcam_conf_network.h"

#define LOG_TAG "xcam_network"

#ifdef __cplusplus
extern "C" {
#endif

#define BUF_SIZE 1024
char gInterface[20]= {0};
char gIpAddr[20] = {0};
int xcam_network_get_device_ip_gateway(char *interface, char *gateway);

//根据进程名获取指定进程的pid
int xcam_network_task_get_pid(char *task_name, pid_t *task_pid)
{
	int ret = XCAM_ERROR;
	DIR *dir = NULL;
	struct dirent *ptr = NULL;
	FILE *fp = NULL;
	char filepath[50] = {'\0'};
	char cur_task_name[50] = {'0'};
	char buf[BUF_SIZE] = {'0'};

	dir = opendir("/proc");
	if (NULL != dir) {
		while ((ptr = readdir(dir)) != NULL) {
			if ((strcmp(ptr->d_name, ".") == 0) || (strcmp(ptr->d_name, "..") == 0)) {
				continue;
			}

			if(DT_DIR != ptr->d_type)
				continue;

			memset(filepath, 0,sizeof(filepath));
			sprintf(filepath, "proc/%s/status", ptr->d_name);
			fp = fopen(filepath, "r");
			if (NULL != fp) {
				memset(buf,0,sizeof(buf));
				if (fgets(buf,BUF_SIZE -1,fp) == NULL) {
					fclose(fp);
					continue;
				}

				//提取进程name
				sscanf(buf, "%*s %s", cur_task_name);

				//如果进程名字匹配上了，则这个文件夹的名字就是task的pid
				if (strcmp(task_name, cur_task_name) == 0) {
					sscanf(ptr->d_name,"%d",task_pid);
					//获取pid成功返回success，没有获取到pid则是error
					ret = XCAM_SUCCESS;
					fclose(fp);
					break;
				}
				fclose(fp);
			}
		}
		closedir(dir);
	}

	return ret;
}

//如若不管行pid的值的话，pid可以为NULL
void xcam_network_dhcp_status_and_pid(bool *pIsDhcpEnable, pid_t *pid)
{
	int ret = XCAM_SUCCESS;
	if (pid == NULL) {
		pid_t pidTemp = 0;
		pid = &pidTemp;
	}

	ret =xcam_network_task_get_pid("udhcpc",pid);
	if (ret == XCAM_ERROR) {
		*pIsDhcpEnable = false;
		LOG_INF(LOG_TAG,"System dhcp is disable.\n");
		return ;
	}

	*pIsDhcpEnable = true;

	return;
}

//设备中启动DHCP服务
void xcam_network_start_dhcp()
{
	bool IsDhcpEnable = false;
	pid_t pid = 0;
	char cmd[20] = {'\0'};

	xcam_network_dhcp_status_and_pid(&IsDhcpEnable, &pid);
	//若果之前已经启动这个服务，就给他关了，这部分代码主要是为了限制出现多个dhcp的进程，根据测试结果来判断是否有必要先杀死后开启，之前碰到多个udhcp的命令，不知道是不是
	//操作命令不对导致的的还是该进程就是这种特性
	if (IsDhcpEnable == true) {
		assert(pid != 0);
	//	kill(pid,SIGTERM);
	//	kill(pid,SIGKILL);这两个不知道开发板上面是不是支持这一种用法，测试的时候测一下这种用法是否支持，要是支持的话就用这个函数

		sprintf(cmd, "kill -9 %d",pid);
		system(cmd);
	}

	printf("start dncp server.\n");
	system("udhcpc -b -i eth0 -R");

	return;
}

void xcam_network_stop_dhcp()
{
	bool IsDhcpEnable = false;
	pid_t pid = 0;
	char cmd[20] = {'\0'};

	xcam_network_dhcp_status_and_pid(&IsDhcpEnable, &pid);
	if (IsDhcpEnable == true) {
		assert(pid != 0);
		sprintf(cmd, "kill -9 %d",pid);
		system(cmd);

		LOG_INF(LOG_TAG,"Stop DHCP success.\n");
		return;
	} else {
		LOG_INF(LOG_TAG,"DHCP disable,you cann't stop DHCP.\n");
	}

	return;
}

int xcam_network_set_device_static_ip(char* interface, char* addr)
{
	char cmd[128] = {'\0'};
	char gateway [20] = {0};
	int ret = XCAM_SUCCESS;

	ret = xcam_network_get_device_ip_gateway(interface,gateway);

	sprintf(cmd, "ifconfig %s %s" ,interface, addr);
	system(cmd);

	if (ret == XCAM_SUCCESS) {
		memset(cmd, 0, sizeof(cmd));
		sprintf(cmd,"route add default gw %s",gateway);
		system(cmd);
	}

	ret = xcam_conf_set_ip_addr(addr);
	if (ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG,"error(%s,%d),save configfile fail.\n",__func__,__LINE__);
	}

	return ret;
}

int xcam_network_set_device_DNS_server_addr(char *DNSaddr)
{
	int ret = XCAM_SUCCESS;

	printf("xcam network set DNS server addr fail.\n");

	return ret;
}

int xcam_network_get_device_DNS_server_addr(char *DNSaddr)
{
	FILE *fp = NULL;
	char cmd[128] = {'\0'};
	int i = 0;

	sprintf(cmd," nslookup www.baidu.com | grep Server | awk '{print $2}' ");

	fp = popen(cmd, "r");
	if ( fp == NULL ) {
		LOG_ERR(LOG_TAG,"error(%s,%d),popen fail,get gateway fail\n",__func__,__FILE__);
		return XCAM_ERROR;
	}

	fread(DNSaddr,1,19,fp);
	pclose(fp);
	char * temp = DNSaddr;
	for (i = 0; i < 19;i++ ) {
		if (*temp == '\0' )
			break;
		if (*temp == '\n')
			*temp = '\0';
		temp ++;
	}

	return XCAM_SUCCESS;
}

/*添加默认网关的*/
int xcam_network_add_device_ip_gateway(char *gateway)
{
	char cmd[128] = {0};

	sprintf(cmd,"route add default gw %s",gateway);

	system(cmd);

	return XCAM_SUCCESS;
}

/*修改默认网关*/
int xcam_network_set_device_ip_gateway(char *gateway)
{
	char cmd[128] = {0};
	int ret = XCAM_SUCCESS;

	/*先删除*/
	sprintf(cmd,"route del default");
	system(cmd);

	/*后添加*/
	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd,"route add default gw %s",gateway);
	system(cmd);

	ret = xcam_conf_set_gateway(gateway);
	if (ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG,"error(%s,%d),save configfile fail.\n",__func__,__LINE__);
	}

	return ret;
}

int xcam_network_get_device_ip_gateway(char *interface, char *gateway)
{
	FILE *fp = NULL;
	int i = 0;
	char cmd[128] = {'\0'};

	sprintf(cmd ,"route -n | grep %s | grep UG | awk '{print $2}' ",interface);
	//sprintf(cmd ,"route | grep %s | grep default | awk '{print $2}' ",interface);

	fp = popen(cmd,"r");
	if ( fp == NULL ) {
		LOG_ERR(LOG_TAG,"error(%s,%d),popen fail,get gateway fail\n",__func__,__LINE__);
		return XCAM_ERROR;
	}

	fread(gateway,1,19,fp);
	pclose(fp);

	char * temp = gateway;
	for (i = 0; i < 19;i++ ) {
		if (*temp == '\0' )
			break;
		if (*temp == '\n')
			*temp = '\0';
		temp ++;
	}

	return XCAM_SUCCESS;
}

int xcam_network_set_device_ip_mask(char *interface, char *mask)
{
	char cmd[128] = {0};
	char gateway [20] = {0};
	int ret = XCAM_SUCCESS;

	/*这步必须要在设置掩码之前*/
	ret = xcam_network_get_device_ip_gateway(interface,gateway);

	sprintf(cmd,"ifconfig %s netmask %s",interface,mask);
	system(cmd);

	if (ret == XCAM_SUCCESS) {
		memset(cmd, 0, sizeof(cmd));
		sprintf(cmd,"route add default gw %s",gateway);
		system(cmd);
	}

	ret = xcam_conf_set_mask(mask);
	if (ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG,"error(%s,%d),save configfile fail.\n",__func__,__LINE__);
	}

	return ret;
}

int xcam_network_get_device_ip_mask(char *interface, char *mask)
{
	int ret = XCAM_SUCCESS;
    int sock = -1;
	struct ifreq ifr;

	memset(&ifr, 0, sizeof(struct ifreq));

	sock = socket(AF_INET,SOCK_DGRAM,0);
	if (sock < 0) {
		LOG_ERR(LOG_TAG,"create socket fail.\n");
		return XCAM_ERROR;
	}

    strcpy(ifr.ifr_name, interface);

	ret = ioctl(sock, SIOCGIFNETMASK, &ifr);
	if (ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG,"error(%s,%d),call ioctl fial.\n",__func__,__LINE__);
		close(sock);
		return ret;
	}

	strcpy(mask, inet_ntoa(((struct sockaddr_in *)&(ifr.ifr_addr))->sin_addr));
	close(sock);

	return ret;
}

int xcam_network_get_device_ip(char* interface, char* addr)
{
	int ret = XCAM_SUCCESS;
    int sock = -1;
	struct ifreq ifr;

	memset(&ifr, 0, sizeof(struct ifreq));

	sock = socket(AF_INET,SOCK_DGRAM,0);
	if (sock < 0) {
		LOG_ERR(LOG_TAG,"create socket fail.\n");
		return XCAM_ERROR;
	}

    strcpy(ifr.ifr_name, interface);
	ret = ioctl(sock, SIOCGIFADDR, &ifr);
	if (ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG,"error(%s,%d),call ioctl fial.\n",__func__,__LINE__);
		close(sock);
		return ret;
	}

	strcpy(addr, inet_ntoa(((struct sockaddr_in *)&(ifr.ifr_addr))->sin_addr));

	close(sock);
	return ret;
}

void xcam_reboot(void)
{
    sync();
    reboot(RB_AUTOBOOT);
}

int xcam_network_get_device_mac(char *interface, unsigned char *pMac)
{
	struct ifreq ifreq;
	int sockfd = 0;
	unsigned char mac[6] = {0};

	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		return -1;

	strcpy(ifreq.ifr_name, interface);

	if (ioctl(sockfd, SIOCGIFHWADDR, &ifreq) < 0) {
		close(sockfd);
		return -2;
	}

	memcpy(mac, ifreq.ifr_hwaddr.sa_data, 6);
	LOG_DBG(LOG_TAG, "MAC:%02x:%02x:%02x:%02x:%02x:%02x\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

	if (pMac != NULL) {
		sprintf((char*)pMac, "%02X-%02X-%02X-%02X-%02X-%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	}

	close(sockfd);
	return 0;
}

/*获取网卡名，目前该接口只支持单网卡设备，不支持多网卡设备*/
int xcam_network_get_net_interface(char *pInterface)
{
	char interface [20] = {0};
	FILE *fp = NULL;
	char cmd[128] = {0};
	int i = 0;

	sprintf(cmd,"cat /proc/net/dev | awk '{i++; if(i>2){print $1}}' | sed 's/^[\t]*//g' | sed 's/[:]*$//g'|grep -v lo");

	fp = popen(cmd,"r");
	if ( fp == NULL ) {
		LOG_ERR(LOG_TAG,"error(%s,%d),popen fail,get gateway fail\n",__func__,__LINE__);
		return XCAM_ERROR;
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

	return XCAM_SUCCESS;
}

#ifdef __cplusplus
}
#endif
