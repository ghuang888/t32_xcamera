
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
#include <iterator>
#include <RCF/RCF.hpp>

#include "MyService.hpp"
#include "conf_process.h"

MyServiceImpl::MyServiceImpl()
{
}

MyServiceImpl::~MyServiceImpl()
{
}

void MyServiceImpl::rcf_msgprocess(const std::string &str,std::string &str_ret)
{
    //std::cout << "befor deal:"<< str << std::endl;
    int ret = 0;

    /*1、申请资源、string字符转char*/
    char *pcstrtmp = new char[str.length() + 1];
    char *pstr_ret = new char[MAX_RETSTR_LEN];
    strcpy(pcstrtmp,str.c_str() );

    /*2、处理字符串，调用conf里的处理函数，rcf实际只做个传递工具*/
    pthread_mutex_lock(&g_conf_mutex_t);
    ret = conf_process(pcstrtmp,pstr_ret);
    if(0 != ret)
    {
        printf("[%s][%d]:err!!! conf_process run fail\n",__func__, __LINE__);
        pthread_mutex_unlock(&g_conf_mutex_t);
        return;
    }
    pthread_mutex_unlock(&g_conf_mutex_t);

    /*3、返回处理后的json字符*/
    str_ret = pstr_ret;
    //std::cout << "after deal str_ret.length:"<< str_ret.length() << std::endl;

	/*4、释放资源*/
    delete [] pcstrtmp;
    delete [] pstr_ret;
    //printf("rcf_msgprocess run over,just ruturn\n");
    return ;
}

//   typedef boost::shared_ptr<ServerBinding> ServerBindingPtr;
// #include <RCF/ServerStub.hpp>
#define RCF_PORT_START 50001
int rcf_server_start(void)
{
    RCF::RcfInitDeinit rcfInit;
    std::cout << "xcam run here ,start to wait client to connect!" << std::endl;
    // Start a TCP server on port 50001, and expose MyServiceImpl.
    MyServiceImpl myServiceImpl;
    int rcfPort = RCF_PORT_START;
    int rcfIsrunning = 0;
    RCF::RcfServer *server =   (new  RCF::RcfServer( RCF::TcpEndpoint("0.0.0.0", rcfPort)) );
    server->bind<MyService>(myServiceImpl);
    while( rcfIsrunning == 0){
        try {
            rcfIsrunning = 1;
            server->start(); 
        } catch (const RCF::Exception e){
            std::cout <<"rcf server bind port=%d failed over!" << rcfPort << std::endl;
            rcfPort += 1;
            rcfIsrunning = 0;
        }
        if( !rcfIsrunning && server)
            delete server;
            server = new  RCF::RcfServer(RCF::TcpEndpoint("0.0.0.0", rcfPort));
            server->bind<MyService>(myServiceImpl);
    }


  

    // //later use while
	while(1) {
		sleep(1);
	}
    if(server)
        delete server;
    std::cout <<"rcf server run over!" <<std::endl;
    return 0;
}

