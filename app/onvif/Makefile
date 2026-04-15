PLATFORM = "INGENIC"

ifeq ($(PLATFORM),"INGENIC")

C_COMPILER =			mips-linux-gnu-gcc
CPLUSPLUS_COMPILER =	mips-linux-gnu-g++
#CPP =	                mips-linux-uclibc-gnu-g++
LINK =					mips-linux-gnu-g++ -o

ONVIF_FLAGS := -g  -Wall -ldl -pthread -lm -muclibc -DDEBUG -DWITH_NONAMESPACES -DWITH_NOIDREF 

else

C_COMPILER =			gcc
CPLUSPLUS_COMPILER =	g++
LINK =					g++ -o

ONVIF_FLAGS := -g -Wall -ldl -pthread -lm -DDEBUG -DWITH_NONAMESPACES  -DWITH_NOIDREF -DPLATFORM_PC 
endif

ONVIFDIR = $(PWD)

ONVIF=onvif

all:${ONVIF}
	echo ${ONVIF}

#onvif

ONVIF_INCLUDES := -I$(ONVIFDIR)/src_soap \
	-I $(ONVIFDIR)/inc

ONVIF_SRC_OBJS := $(ONVIFDIR)/src_soap/soapC.o \
	$(ONVIFDIR)/src_soap/soapServer.o \
	$(ONVIFDIR)/src_soap/stdsoap2.o


ONVIF_OBJS = $(ONVIFDIR)/src/onvif_func.o \
	$(ONVIFDIR)/src/func_network.o \
	$(ONVIFDIR)/src/onvif.o


$(ONVIF):$(ONVIF_OBJS) $(ONVIF_SRC_OBJS)
	$(C_COMPILER) $(ONVIF_OBJS) $(ONVIF_SRC_OBJS) $(ONVIF_FLAGS) -o $@
$(ONVIF_OBJS):%.o:%.c
	$(C_COMPILER) $(ONVIF_FLAGS) $(ONVIF_INCLUDES) -o $@ -c $^

.PHONY: all clean distclean
clean:
	-rm -f $(C_OBJS) $(CPP_OBJS) $(ONVIF_OBJS) $(ONVIF)

distclean: clean
