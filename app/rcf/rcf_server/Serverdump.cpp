#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
#include <iterator>
#include <RCF/RCF.hpp>

#include "MyService.hpp"
extern "C" {
	#include "xcam_ptz.h"
}

using namespace std;

#define PicturePATH "/tmp/snap.yuv"
int fdPicture;
int fdRecord;
RCF::ByteBuffer *bytebuffer;

// Step 1: DownloadPictureStart
/* Init picture format, snap frame in picture snap.yuv */
static int DownloadPictureStart(int channel)
{
	int ret = -1;
	ret = xcam_snap_nv12(channel);
	return ret;
}

// Step 2: DownloadInit
/* Set picture width and height */
static int DownloadInit(std::size_t bandwidth)
{
	if(bandwidth > 640 * 360 * 3 / 2) {
		printf("bandwidth is too big !\n");
		return -1;
	}

	bytebuffer = new RCF::ByteBuffer(bandwidth);
	if(bytebuffer == NULL) {
		printf("bytebuffer new error !\n");
		return -1;
	}

	return 0;
}

// Step 3-1: Get DownloadPictureGetinfo
/* Get picture data length */
static long int DownloadPictureGetinfo(void)
{
	int fd;
	long int length;

	fd = open(PicturePATH, O_RDONLY);
	if(fd < 0) {
		printf("open error !\n");
		return -1;
	}

	length = (long int)lseek(fd, 0, SEEK_END);

	close(fd);

	return length;
}

// Step 3-2: DownloadPictureOpen
/* open picture */
static int DownloadPictureOpen(void)
{
	fdPicture = open(PicturePATH, O_RDONLY);
	if(fdPicture < 0) {
		printf("open error !\n");
		return -1;
	}

	return 0;
}

// Step 3-3: DownloadPicture
/* Write image data to buf */
static RCF::ByteBuffer DownloadPicture(int length)
{
	int i;
	char buf[length];

	read(fdPicture, buf, length);
	for (i = 0; i < length; i++)
	{
		bytebuffer->getPtr()[i] = buf[i];
	}

	return *bytebuffer;
}

// Step 3-4: DownloadPictureClose
/* Close picture */
static int DownloadPictureClose(void)
{
	close(fdPicture);

	return 0;
}

// Step 4: DownloadExit
/* End of transmission, clear buf */
 int MyServiceImpl::DownloadExit(void )
{
	if(bytebuffer == NULL)
		return 0;

	delete bytebuffer;

	return 0;
}

RCF::ByteBuffer MyServiceImpl::DownloadPictureMain(int channel)
{
	int bytewidth = 640 * 360 * 3 / 2;

	DownloadPictureStart(channel);
	DownloadInit(bytewidth);

	try {
		long int size;
		RCF::ByteBuffer bytebuffer;
		size = DownloadPictureGetinfo();
		DownloadPictureOpen();
		bytebuffer = DownloadPicture(size);
		DownloadPictureClose();
	}
	catch (const RCF::Exception & e) {
		printf("%s() error: %s", __func__, e.getError().getErrorString().data());
	}

	return *bytebuffer;
}

