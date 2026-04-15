PLATFORM = "MIPS"
ifeq ($(PLATFORM), "MIPS")
C_COMPILER =			mips-linux-gnu-gcc
CPLUSPLUS_COMPILER =	mips-linux-gnu-g++
LINK =					mips-linux-gnu-g++ -o
else
C_COMPILER =			gcc
CPLUSPLUS_COMPILER =	g++
LINK =					g++ -o
endif


ifeq ($(PLATFORM), "MIPS")
LIVE555 = _install_live555_mips_uclibc/
else
LIVE555 = _install_live555_x86_gdb/
endif

INCLUDES = \
		   -I$(LIVE555)include/BasicUsageEnvironment \
		   -I$(LIVE555)include/groupsock \
		   -I$(LIVE555)include/liveMedia \
		   -I$(LIVE555)include/UsageEnvironment 
LIBS = \
	   $(LIVE555)lib/libliveMedia.a \
	   $(LIVE555)lib/libgroupsock.a \
	   $(LIVE555)lib/libBasicUsageEnvironment.a \
	   $(LIVE555)lib/libUsageEnvironment.a

ifeq ($(PLATFORM), "MIPS")
COMPILE_OPTS =		$(INCLUDES) -I. -g -pthread -muclibc -DSOCKLEN_T=socklen_t -D_LARGEFILE_SOURCE=1 -DNEWLOCALE_NOT_USED=1 -DALLOW_RTSP_SERVER_PORT_REUSE=1
LINK_OPTS =		-L. $(LDFLAGS) -muclibc -pthread
else
#COMPILE_OPTS =		$(INCLUDES) -std=c++11 -I. -g -pthread -DSOCKLEN_T=socklen_t -D_LARGEFILE_SOURCE=1 -DNEWLOCALE_NOT_USED=1 -DALLOW_RTSP_SERVER_PORT_REUSE=1
COMPILE_OPTS =		$(INCLUDES) -std=c++0x -I. -g -pthread -DSOCKLEN_T=socklen_t -D_LARGEFILE_SOURCE=1 -DNEWLOCALE_NOT_USED=1 -DALLOW_RTSP_SERVER_PORT_REUSE=1
LINK_OPTS =		-L. $(LDFLAGS) -pthread
endif

C_FLAGS =			$(COMPILE_OPTS) $(CPPFLAGS) $(CFLAGS) -Wall
CPLUSPLUS_FLAGS =	$(COMPILE_OPTS) -Wall $(CPPFLAGS) $(CXXFLAGS)


all: test1_RTSP

#C_OBJS = test1_RTSP.o
CPP_OBJS = G711AudioStreamServerMediaSubsession.o  G711AudioStreamSource.o  H264VideoServerMediaSubsession.o H264VideoStreamSource.o H265VideoServerMediaSubsession.o H265VideoStreamSource.o LiveRTSP.o StreamSource.o test1_RTSP.o   

test1_RTSP:$(C_OBJS) $(CPP_OBJS) $(LIBS)
	$(LINK)$@ $(LINK_OPTS) $(C_OBJS) $(CPP_OBJS) -Wl,--start-group $(LIBS) -Wl,--end-group

$(C_OBJS):%.o:%.c
	$(C_COMPILER) -c $(C_FLAGS) -o $@ $<
$(CPP_OBJS):%.o:%.cpp
	$(CPLUSPLUS_COMPILER) -c $(CPLUSPLUS_FLAGS) -o $@ $<

.PHONY: clean distclean
clean:
	-rm -f $(C_OBJS) $(CPP_OBJS)

distclean: clean
