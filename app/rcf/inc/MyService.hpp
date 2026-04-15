
#ifndef INCLUDE_MYSERVICE_HPP
#define INCLUDE_MYSERVICE_HPP

#include <string>
#include <vector>

#include <RCF/Idl.hpp>
#include <SF/vector.hpp>

RCF_BEGIN(MyService, "MyService")

//rcf msgprocess for deal cJSON
RCF_METHOD_V2(void, rcf_msgprocess,const std::string &,std::string &);

//rcf msgprocess for download picture
RCF_METHOD_R1(RCF::ByteBuffer, DownloadPictureMain, int);
RCF_METHOD_R0(int, DownloadExit);

RCF_END(MyService);

class MyServiceImpl
{
	public:
		MyServiceImpl();
		~MyServiceImpl();

		//rcf msgprocess for deal cJSON
		void rcf_msgprocess(const std::string &v,std::string &str_ret);

		//rcf msgprocess for download picture
		RCF::ByteBuffer DownloadPictureMain(int);
		int DownloadExit(void);

};

#endif // ! INCLUDE_MYSERVICE_HPP
