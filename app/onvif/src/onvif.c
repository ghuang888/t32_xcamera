#include <sys/prctl.h>
#include <pthread.h>
#include "soapH.h"    // server stubs, serializers, etc.
#include "wsdd.nsmap" // XML namespace mapping table (only needed once at the global level)
#include "soapStub.h"
#include "func_network.h"
#include "config.h"

#include "func_log.h"
#define LOG_TAG "ONVIF"

// void send_probe(struct soap *soap) {
//     struct wsdd__ProbeType req;
//     struct soap *soap_response = soap_new();
//     struct __wsdd__ProbeMatches resp;

//     soap_init(soap);
//     soap->send_timeout = 10;  // 10 seconds
//     soap->recv_timeout = 10;  // 10 seconds

//     soap_default_wsdd__ProbeType(soap, &req);
//     req.Types = "dn:NetworkVideoTransmitter";
//     req.Scopes = NULL;
//     // req.XAddrs = NULL;
//     // req.MetadataVersion = NULL;

//     soap_set_namespaces(soap, namespaces);

//     if (soap_send___wsdd__Probe(soap, MULTICAST_ADDRESS, MULTICAST_PORT, &req)) {
//         soap_print_fault(soap, stderr);
//     } else {
//         while (soap_recv___wsdd__ProbeMatches(soap_response, &resp) == SOAP_OK) {
//             for (int i = 0; i < resp.wsdd__ProbeMatches->__sizeProbeMatch; i++) {
//                 struct wsdd__ProbeMatchType *match = resp.wsdd__ProbeMatches->ProbeMatch + i;
//                 printf("Device Address: %s\n", match->wsa__EndpointReference.Address);
//                 printf("XAddrs: %s\n", match->XAddrs);
//             }
//         }

//         if (soap_response->error) {
//             soap_print_fault(soap_response, stderr);
//         }
//     }

//     soap_destroy(soap);
//     soap_end(soap);
//     soap_done(soap);
//     free(soap_response);
// }
const char *probe ="<?xml version=\"1.0\" encoding=\"utf-8\"?><s:Envelope xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\" xmlns:sc=\"http://www.w3.org/2003/05/soap-encoding\" xmlns:dn=\"http://www.onvif.org/ver10/network/wsdl\" xmlns:tds=\"http://www.onvif.org/ver10/device/wsdl\" xmlns:d=\"http://schemas.xmlsoap.org/ws/2005/04/discovery\" xmlns:a=\"http://schemas.xmlsoap.org/ws/2004/08/addressing\"><s:Header><a:MessageID>uuid:9328fd5b-4c83-4e4f-9850-66549c6d9136</a:MessageID><a:To>urn:schemas-xmlsoap-org:ws:2005:04:discovery</a:To><a:Action>http://schemas.xmlsoap.org/ws/2005/04/discovery/Hello</a:Action></s:Header><s:Body><d:Hello><a:EndpointReference><a:Address>urn:uuid:4be6bccd-81c0-436c-b55e-173b8c8ec17e</a:Address></a:EndpointReference><d:Types>dn:NetworkVideoTransmitter tds:Device</d:Types><d:Scopes>onvif://www.onvif.org/type/NetworkVideoTransmitter onvif://www.onvif.org/Profile/Streaming onvif://www.onvif.org/location/city/hangzhou onvif://www.onvif.org/name/ONVIF onvif://www.onvif.org/Profile/Q/Operational onvif://www.onvif.org/hardware/IPC</d:Scopes><d:XAddrs>http://172.16.10.6:8000/onvif/device_service</d:XAddrs><d:MetadataVersion>1</d:MetadataVersion></d:Hello></s:Body></s:Envelope>";

/* 发送探测消息（Probe）的目标地址、端口号 */ 
#define CAST_ADDR  "239.255.255.250"  // 多播地址，固定的239.255.255.250 
#define CAST_PORT  3702               // 端口号
static void *discovery_thread(void *data)
{
    int count = 0;
    int ret = 0;
    SOAP_SOCKET socket_s, socket_c;
    struct soap ServerSoap;
    struct ip_mreq mcast;
    struct sockaddr_in multi_addr;
	LOG_DBG(LOG_TAG, "start\n");
    soap_init1(&ServerSoap, SOAP_IO_UDP | SOAP_XML_IGNORENS);
    soap_set_namespaces(&ServerSoap,  namespaces);
       // 设置发送消息的次数
    int message_count = 5;
    int interval = 2; // seconds

    ServerSoap.send_timeout = 100;
    ServerSoap.recv_timeout = 100;
    LOG_DBG(LOG_TAG, "ServerSoap.version = %d\n", ServerSoap.version);
    //soap_register_plugin(&ServerSoap, soap_wsa);
    socket_s = soap_bind(&ServerSoap, NULL, DISCOVERY_LISTEN_PORT, 100);
    if(!soap_valid_socket(socket_s))
    {
	soap_print_fault(&ServerSoap, stderr);
        LOG_ERR(LOG_TAG, "ERROR: soap_bind error! %s\n", strerror(errno));
	exit(1);
    }
    mcast.imr_multiaddr.s_addr = inet_addr("239.255.255.250");
    mcast.imr_interface.s_addr = htonl(INADDR_ANY);

    ret = setsockopt(ServerSoap.master, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mcast, sizeof(mcast));
    if(ret < 0){
	LOG_ERR(LOG_TAG, "setsockopt error! error code = %d,err string = %s\n",errno,strerror(errno));
	return 0;
    }
    int ttl = 10;
 // 设置多播TTL
    if (setsockopt(ServerSoap.master, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl)) < 0) {
        perror("setsockopt IP_MULTICAST_TTL failed");

        return 1;
    }
multi_addr.sin_family = AF_INET; // 搜索IPC：使用UDP向指定地址发送探测消息(Probe) 
	multi_addr.sin_port = htons(CAST_PORT); 
	multi_addr.sin_addr.s_addr = inet_addr(CAST_ADDR);

       // Loop to send probe messages with interval
    for (int i = 0; i < message_count; i++) {
        	ret = sendto(ServerSoap.master, probe, strlen(probe), 0, (struct sockaddr*)&multi_addr, sizeof(multi_addr)); 
	if (ret < 0) 
	{ 
		perror("sendto error");
		return -1; 
	} 
        if (i < message_count - 1) {
            printf("Waiting for %d seconds before sending next probe message...\n", interval);
            sleep(interval);
        }
    }
    printf("lzhzcj %s[%d]\n", __func__, __LINE__);
    for(;;) {
        socket_c = soap_accept(&ServerSoap);
        printf("lzhzcj %s[%d]\n", __func__, __LINE__);
        if (!soap_valid_socket(socket_c)){
            soap_print_fault(&ServerSoap, stderr);
            LOG_ERR(LOG_TAG, "ERROR: valid_socket: socket = %d, %s\n", socket_c, strerror(errno));
        }
    printf("lzhzcj %s[%d]\n", __func__, __LINE__);
		if(soap_serve(&ServerSoap)) {
			soap_print_fault(&ServerSoap, stderr);
		}
    printf("lzhzcj %s[%d]\n", __func__, __LINE__);
		soap_destroy(&ServerSoap);
		soap_end(&ServerSoap);
    printf("lzhzcj %s[%d]\n", __func__, __LINE__);
		LOG_DBG(LOG_TAG, "RECEIVE count %d, connection from IP = %u.%u.%u.%u socket = %d\n",
			count, ((ServerSoap.ip)>>24)&0xFF, ((ServerSoap.ip)>>16)&0xFF,
			((ServerSoap.ip)>>8)&0xFF, (ServerSoap.ip)&0xFF, (ServerSoap. socket));

		count++;
	}

    soap_done(&ServerSoap);

    return NULL;
}

static void *server_thread(void *data)
{
    SOAP_SOCKET socket_s, socket_c;
    struct soap soap;

    soap_init(&soap);
    soap_set_namespaces(&soap, namespaces);
    //soap_set_mode(&soap, SOAP_C_UTFSTRING);
    soap.bind_flags   = SO_REUSEADDR;
    soap.send_timeout = 100; /* 10 sec */
    soap.recv_timeout = 100; /* 10 sec */
    socket_s = soap_bind(&soap, NULL, SERVER_LISTEN_PORT, 100);
    if (!soap_valid_socket(socket_s)) {
	soap_print_fault(&soap, stderr);
	LOG_ERR(LOG_TAG, "ERROR: soap_bind error! %s\n", strerror(errno));
	return NULL;
    }

    LOG_DBG(LOG_TAG, "INFO: Socket connection successful, server socket = %d\n", socket_s);

    for (;;) {
	socket_c = soap_accept(&soap);
	if (!soap_valid_socket(socket_c)) {
	    soap_print_fault(&soap, stderr);
	    LOG_ERR(LOG_TAG, "ERROR: valid_socket: socket = %d, %s\n", socket_c, strerror(errno));
	    break;
	}
	soap_serve(&soap);
	soap_destroy(&soap);
	soap_end(&soap);
    }
    soap_done(&soap);

    return NULL;
}

void *onvif_start(void* arg)
{
    int ret;
    pthread_t discovery_thid, server_thid;
       printf("lzhzcj %s[%d]\n", __func__, __LINE__);
    prctl(PR_SET_NAME, "onvif_main", 0, 0, 0);
    ret = pthread_create(&discovery_thid, NULL, discovery_thread, NULL);
    if (ret != 0) {
	LOG_ERR(LOG_TAG, "[ERROR] %s: pthread_create discovery_thread failed\n", __func__);
	return NULL;
    }

    ret = pthread_create(&server_thid, NULL, server_thread, NULL);
    if (ret != 0) {
	LOG_ERR(LOG_TAG, "[ERROR] %s: pthread_create server_thread failed\n", __func__);
	return NULL;
    }

    pthread_join(discovery_thid, NULL);
    pthread_join(server_thid, NULL);

	return NULL;
}


#ifdef ONVIF_WITH_CARRIER_SERVER
int main(int argc,char ** argv)
{
    onvif_start(NULL);
    return 0;
}
#endif
