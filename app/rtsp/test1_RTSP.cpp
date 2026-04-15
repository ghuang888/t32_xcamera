#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <iostream>

#include "LiveRTSP.hh"
#include "c_liveRTSP.h"

using namespace std;

#define FRAMEBUFSIZE (10*1024*1024)
#define BUFSIZE 8192

class H264FileNalRead {
	private:
		char *tmpbuf;
		int fd_tmp;
		int start;
		int left;
		int file_end;
		//int add_header = 0;
		int last;
		int stat;
		int first;

	public:
	H264FileNalRead() {
		fd_tmp = 0;
		start = 0;
		left = 0;
		file_end = 0;
		//add_header = 0;
		last = 0;
		stat = 0;
		first = 0;
		tmpbuf = (char*)malloc(BUFSIZE);
		if (NULL == tmpbuf) {
			printf("malloc failed\n");
		}
	}
	int readRestart() {
		fd_tmp = 0;
		start = 0;
		left = 0;
		file_end = 0;
		//add_header = 0;
		last = 0;
		stat = 0;
		first = 0;
		return 0;
	}

#define BUFSIZE 8192
	int readAndGetOneNal(int fd, char* buf, int bufsize, int *framesize) {

		if (fd_tmp != fd) {
			start = 0;
			left = 0;
			file_end = 0;
			//add_header = 0;

			last = 0;

			stat = 0;
			first = 0;
			lseek(fd, 0, SEEK_SET);

			fd_tmp = fd;
		}

		char *framebuf = buf;
		char *pbuf = NULL;

#if 0
		if (1 == add_header) {
			add_nal_header(framebuf, add_header);
			framebuf+=3;
		} else if (2 == add_header) {
			add_nal_header(framebuf, add_header);
			framebuf+=4;
		}
		add_header = 0;
#endif

		int end = 0;
		while (1) {
			if (0 == left) {
				int len = 0;
				if (last) {
					last = 0;
					end = 1;
					break;
				}
				len = read(fd, tmpbuf, BUFSIZE);
				left = len;
				if (len != BUFSIZE) {
					//printf("file end\n");
					file_end = 1;
					last = 1;
				}
				start = 0;
			}
			if ((file_end)&&(0 == left)) {
				printf("file end\n");
				return -1;
			}
			for (;left > 0; left--) {
				pbuf = tmpbuf+start;

				char data = *pbuf;
				if (0 == first) {
					switch(stat) {
					case 0:
						if (0x00 != data) {
							;
						} else
							stat = 1;
						break;
					case 1:
						if (0x00 != data) {
							stat = 0;
						} else
							stat = 2;
						break;
					case 2:
						if (0x01 == data) {
#if 0
							add_nal_header(framebuf, 1);
							framebuf+=3;
#endif
							first = 1;
							stat = 0;
						} else if (0x00 == data)
							stat = 3;
						else
							stat = 0;
						break;
					case 3:
						if (0x01 == data) {
#if 0
							add_nal_header(framebuf, 2);
							framebuf+=4;
#endif
							first = 1;
							stat = 0;
						} else
							stat = 0;
						break;
					}
				} else {
					switch(stat) {
					case 0:
						if (0x00 != data) {
							*framebuf = data; framebuf++;
						} else
							stat = 1;
						break;
					case 1:
						if (0x00 != data) {
							*framebuf = 0x00; framebuf++;
							*framebuf = data; framebuf++;
							stat = 0;
						} else
							stat = 2;
						break;
					case 2:
						if (0x01 == data) {
							//add_header = 1;
							end = 1;
							stat = 0;
						} else if (0x00 == data)
							stat = 3;
						else {
							*framebuf = 0x00; framebuf++;
							*framebuf = 0x00; framebuf++;
							*framebuf = data; framebuf++;
							stat = 0;
						}
						break;
					case 3:
						if (0x01 == data) {
							//add_header = 2;
							end = 1;
							stat = 0;
						} else {
							*framebuf = 0x00; framebuf++;
							*framebuf = 0x00; framebuf++;
							*framebuf = 0x00; framebuf++;
							*framebuf = data; framebuf++;
							stat = 0;
						}
						break;
					}
				}
				start++;
				if (end) {
					left--;
					break;
				}

			}
			if (end) {
				break;
			}

		}
		if (end) {
			*framesize = framebuf-buf;
		}
		return 0;
	}
};

#if 0
int add_nal_header(char* buf, int type)
{
	if (1 == type) {
		*buf = 0x00; buf++;
		*buf = 0x00; buf++;
		*buf = 0x01; buf++;
	} else if (2 == type) {
		*buf = 0x00; buf++;
		*buf = 0x00; buf++;
		*buf = 0x00; buf++;
		*buf = 0x01; buf++;
	}
	return 0;
}
#endif
int read_and_get_one_nal(int fd, char* buf, int bufsize, int *framesize)
{

#define BUFSIZE 8192

	static char *tmpbuf = NULL;
	if (NULL == tmpbuf) {
		tmpbuf = (char*)malloc(BUFSIZE);
		if (NULL == tmpbuf) {
			printf("malloc failed\n");
			return -1;
		}
	}
	static int fd_tmp = 0;

	static int start = 0;
	static int left = 0;
	static int file_end = 0;
	//static int add_header = 0;

	static int last = 0;

	static int stat = 0;
	static int first = 0;

	if (fd_tmp != fd) {
		start = 0;
		left = 0;
		file_end = 0;
		//add_header = 0;

		last = 0;

		stat = 0;
		first = 0;
		lseek(fd, 0, SEEK_SET);

		fd_tmp = fd;
	}

	char *framebuf = buf;
	char *pbuf = NULL;

#if 0
	if (1 == add_header) {
		add_nal_header(framebuf, add_header);
		framebuf+=3;
	} else if (2 == add_header) {
		add_nal_header(framebuf, add_header);
		framebuf+=4;
	}
	add_header = 0;
#endif

	int end = 0;
	while (1) {
		if (0 == left) {
			int len = 0;
			if (last) {
				last = 0;
				end = 1;
				break;
			}
			len = read(fd, tmpbuf, BUFSIZE);
			left = len;
			if (len != BUFSIZE) {
				//printf("file end\n");
				file_end = 1;
				last = 1;
			}
			start = 0;
		}
		if ((file_end)&&(0 == left)) {
			printf("file end\n");
			return -1;
		}
		for (;left > 0; left--) {
			pbuf = tmpbuf+start;

			char data = *pbuf;
			if (0 == first) {
				switch(stat) {
				case 0:
					if (0x00 != data) {
						;
					} else
						stat = 1;
					break;
				case 1:
					if (0x00 != data) {
						stat = 0;
					} else
						stat = 2;
					break;
				case 2:
					if (0x01 == data) {
#if 0
						add_nal_header(framebuf, 1);
						framebuf+=3;
#endif
						first = 1;
						stat = 0;
					} else if (0x00 == data)
						stat = 3;
					else
						stat = 0;
					break;
				case 3:
					if (0x01 == data) {
#if 0
						add_nal_header(framebuf, 2);
						framebuf+=4;
#endif
						first = 1;
						stat = 0;
					} else
						stat = 0;
					break;
				}
			} else {
				switch(stat) {
				case 0:
					if (0x00 != data) {
						*framebuf = data; framebuf++;
					} else
						stat = 1;
					break;
				case 1:
					if (0x00 != data) {
						*framebuf = 0x00; framebuf++;
						*framebuf = data; framebuf++;
						stat = 0;
					} else
						stat = 2;
					break;
				case 2:
					if (0x01 == data) {
						//add_header = 1;
						end = 1;
						stat = 0;
					} else if (0x00 == data)
						stat = 3;
					else {
						*framebuf = 0x00; framebuf++;
						*framebuf = 0x00; framebuf++;
						*framebuf = data; framebuf++;
						stat = 0;
					}
					break;
				case 3:
					if (0x01 == data) {
						//add_header = 2;
						end = 1;
						stat = 0;
					} else {
						*framebuf = 0x00; framebuf++;
						*framebuf = 0x00; framebuf++;
						*framebuf = 0x00; framebuf++;
						*framebuf = data; framebuf++;
						stat = 0;
					}
					break;
				}
			}
			start++;
			if (end) {
				left--;
				break;
			}
		}
		if (end) {
			break;
		}
	}
	if (end) {
		*framesize = framebuf-buf;
	}
	return 0;
}


extern LiveRTSP liveRTSP;



void stream(int channel, int argc, char** argv)
{

	int test_cnt = 0;
	int ch = channel;
	char* test_mem;
	int test_fd_h264;
	int test_fd_h265;
	int ret = 0;
	cout << "test file: " << argv[1] << ", " << argv[2] << "\n";
	cout << "stream " << channel << " start\n";
	test_fd_h264 = open(argv[1], O_RDONLY);
	test_fd_h265 = open(argv[2], O_RDONLY);
	cout << "h264_fd :" << test_fd_h264 << "\n";
	cout << "h265_fd :" << test_fd_h265 << "\n";
	test_mem = (char*)malloc(FRAMEBUFSIZE);
	if (NULL == test_mem) {
		cout << "stream " << channel << "exit\n";
		return;
	}
	//if (channel == 1) usleep(8*1000*1000);
	H264FileNalRead read;
	printf("read ptr : %p\n", &read);
	int framecnt = 0;

	liveRTSP.setStreamType(ch, LIVE_RTSP_STREAM_H264);
	liveRTSP.startRTSP(ch);

loop:
	printf("\n\n\n\n\n-------------------start h264-------------------\n");
	while (1) {
		int fsize = 0;
retry1:
		//ret = read_and_get_one_nal(test_fd_h264, test_mem, FRAMEBUFSIZE, &fsize);
		ret = read.readAndGetOneNal(test_fd_h264, test_mem, FRAMEBUFSIZE, &fsize);
		if (0 != ret) {
			::close(test_fd_h264);
			test_fd_h264 = open(argv[1], O_RDONLY);
			read.readRestart();
			goto retry1;
		}
		liveRTSP.putFrame(ch, test_mem, fsize, 0);
		usleep(40*1000);
		if (800 == ++framecnt) {
			framecnt = 0;
			break;
		}
	}

	printf("\n\n\n\n\n-------------------stop h264-------------------\n");
	//usleep(1*1000*1000);
	liveRTSP.stopRTSP(ch);
	//usleep(1*1000*1000);

	//goto loop;

	test_cnt++;
	printf("\n\n\n\n\n-------------------switch  cnt %d-------------------\n\n\n\n\n", test_cnt);

	printf("-------------------start h265-------------------\n");
	liveRTSP.setStreamTypeAndRun(ch, LIVE_RTSP_STREAM_H265);
	//liveRTSP.startRTSP(ch);
	usleep(10*1000*1000);


	while (1) {
		int fsize = 0;
retry2:
		//ret = read_and_get_one_nal(test_fd_h265, test_mem, FRAMEBUFSIZE, &fsize);
		ret = read.readAndGetOneNal(test_fd_h265, test_mem, FRAMEBUFSIZE, &fsize);
		if (0 != ret) {
			::close(test_fd_h265);
			test_fd_h265 = open(argv[2], O_RDONLY);
			read.readRestart();
			goto retry2;
		}
		liveRTSP.putFrame(ch, test_mem, fsize, 0);
		usleep(40*1000);
		if (800 == ++framecnt) {
			framecnt = 0;
			break;
		}
	}

	test_cnt++;
	printf("\n\n\n\n\n-------------------switch  cnt %d-------------------\n\n\n\n\n", test_cnt);
	printf("-------------------stop h265-------------------\n");
	liveRTSP.setStreamTypeAndRun(ch, LIVE_RTSP_STREAM_H264);
	usleep(10*1000*1000);
	//liveRTSP.stopRTSP(ch);
	//usleep(1*1000*1000);

	goto loop;
}




int main(int argc, char** argv)
{
	if (argc < 3) {
		printf("Usage:%s h264bsfile h265bsfile\n", argv[0]);
		return -1;
	}

	liveRTSP.initEnv();

	thread t0(stream, 0, argc, argv);
	thread t1(stream, 1, argc, argv);

	t0.join();
	t1.join();


	return 0;
}
