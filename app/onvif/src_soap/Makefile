PLATFORM = "INGENIC"

ifeq ($(PLATFORM),"INGENIC")

C_COMPILER =			mips-linux-gnu-gcc
CPLUSPLUS_COMPILER =	mips-linux-gnu-g++
#CPP =	                mips-linux-uclibc-gnu-g++
LINK =					mips-linux-gnu-g++ -o

#ONVIF_FLAGS := -g  -Wall -ldl -pthread -lm -muclibc -DDEBUG -DWITH_NONAMESPACES -DWITH_NOIDREF
ONVIF_FLAGS :=  -Wall -ldl -pthread -lm -muclibc -DDEBUG -DWITH_NONAMESPACES -DWITH_NOIDREF

else

C_COMPILER =			gcc
CPLUSPLUS_COMPILER =	g++
LINK =					g++ -o

ONVIF_FLAGS := -g -Wall -ldl -pthread -lm -DDEBUG -DWITH_NONAMESPACES  -DWITH_NOIDREF -DPLATFORM_PC
endif

ONVIFDIR = $(PWD)/../



ONVIF_INCLUDES := -I $(ONVIFDIR)/inc

all:
#	mips-linux-gnu-gcc $(ONVIF_FLAGS) $(ONVIF_INCLUDES) -o soapC.o -c soapC.c
#	mips-linux-gnu-gcc $(ONVIF_FLAGS) $(ONVIF_INCLUDES) -o soapServer.o -c soapServer.c
#	mips-linux-gnu-gcc $(ONVIF_FLAGS) $(ONVIF_INCLUDES) -o stdsoap2.o -c stdsoap2.c
#	mips-linux-uclibc-gnu-ar -r libonvif_uclibc.a soapC.o soapServer.o stdsoap2.o
	mips-linux-gnu-gcc -Wall -ldl -pthread -lm -O2  -DWITH_NONAMESPACES -DWITH_NOIDREF $(ONVIF_INCLUDES) -o soapC_glibc.o -c soapC.c
	mips-linux-gnu-gcc -Wall -ldl -pthread -lm  -O2 -DWITH_NONAMESPACES -DWITH_NOIDREF $(ONVIF_INCLUDES) -o soapServer_glibc.o -c soapServer.c
	mips-linux-gnu-gcc -Wall -ldl -pthread -lm  -O2 -DWITH_NONAMESPACES -DWITH_NOIDREF $(ONVIF_INCLUDES) -o stdsoap2_glibc.o -c stdsoap2.c
	mips-linux-gnu-ar -r libonvif_glibc.a soapC_glibc.o soapServer_glibc.o stdsoap2_glibc.o


.PHONY: all clean distclean
clean:
	-rm -f $(C_OBJS) $(CPP_OBJS)

distclean: clean
