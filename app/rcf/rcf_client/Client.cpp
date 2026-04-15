#include <algorithm>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>
#include "transform_json.h"
#include "MyService.hpp"

#include <RCF/RCF.hpp>

int main(int argc,char *argv[])
{
	int num;
	RCF::RcfInitDeinit rcfInit;
	std::string networkInterface;
	int port = 50001;
	if(argc >= 2) {
		networkInterface = argv[1];
	} else {
		printf("not support yet,please ./client IP\n");
		printf("Example: ./client 193.169.3.135\n");
		return -1;
	}
	std::cout << "Connecting to server on " << networkInterface << ":" << port << "." << std::endl;
	RcfClient<MyService> client( RCF::TcpEndpoint(networkInterface, port) );
	num = atoi(argv[2]);

	try {
#if 1
		/* msg process */
		char *pdata =(char*)malloc(*pdata);
		//pdata = transform_af_zoom_out(10);
		//pdata = transform_ptz_getstatus();
		//pdata = transform_af_zoom_in(10);
		//pdata = transform_ptz_speed(500);
		//pdata = transform_ptz_move(1000,0,1);
		//pdata = transform_ptz_reset();
		//pdata = transform_ptz_cruise();
		//pdata = transform_af_forward(50);
		pdata = transform_af_backward(num);
		//pdata = transform_af_reset();
		//pdata = transform_ptz_xvsy(2);

		std::string str;
		str = pdata;
		std::string str_ret;
		client.rcf_msgprocess(str,str_ret);
		//std::cout<< "after server deal:" << str_ret << std::endl;
#endif

#if 0
		/* start snap */
		RCF::ByteBuffer buffer = client.DownloadPictureMain(1);	// 0:main stream, 1:slave stream
        std::cout << "snap frame success\n";
		FILE *fp = NULL;
		fp = fopen("/system/bin/test.yuv","wb");
		fwrite(buffer.getPtr(), 640 * 360 *3 / 2, 1 ,fp);
		fclose(fp);
		int result = client.DownloadExit();
		if (result < 0) {
			printf("Download picture failed to exit normally\n");
			return -1;
		}
#endif

    }
    catch(const RCF::Exception & e) {
        //std::cout << "client Caught exception:\n";
        //std::cout << e.getError().getErrorString() << std::endl;
        return 0;
    }

    return 0;
}
