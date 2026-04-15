CONFIG_UCLIBC_BUILD=y
CUR_DIR=$(shell pwd)

SDK_DIR ?= $(ISVP_ENV_RELEASE_DIR_SDK)
SDK_INC_DIR = $(SDK_DIR)/include

MXU_DIR ?= $(CUR_DIR)/../../../MXU
IVS_DIR ?= $(CUR_DIR)/../../../IVS
IVS_INC_DIR = $(IVS_DIR)/include

CROSS_COMPILE ?= mips-linux-gnu-

CC = $(CROSS_COMPILE)gcc
CPLUSPLUS = $(CROSS_COMPILE)g++
LD = $(CROSS_COMPILE)ld
AR = $(CROSS_COMPILE)ar cr
STRIP = $(CROSS_COMPILE)strip

INCLUDES = -I$(SDK_INC_DIR) -I$(IVS_INC_DIR)
CFLAGS = $(INCLUDES) -O2 -Wall -march=mips32r2
LDFLAG = -Wall -Wl,-gc-sections -lpthread -lm -lrt -lstdc++

ifeq ($(CONFIG_UCLIBC_BUILD), y)
CFLAGS += -muclibc
LDFLAG += -muclibc
endif

ifeq ($(CONFIG_UCLIBC_BUILD), y)
SDK_LIB_DIR = $(SDK_DIR)/lib/uclibc
MXU_LIB_DIR = $(MXU_DIR)/lib/uclibc
IVS_LIB_DIR = $(IVS_DIR)/lib/uclibc
else
SDK_LIB_DIR = $(SDK_DIR)/lib/glibc
MXU_LIB_DIR = $(MXU_DIR)/lib/glibc
IVS_LIB_DIR = $(IVS_DIR)/lib/glibc
endif

CONFIG_STATIC_URANUS=n
ifeq ($(CONFIG_STATIC_URANUS), y)
LDFLAG += -ldl
URANUS_LIB = -Wl,-whole-archive $(IVS_LIB_DIR)/liburanus_tensor.a $(IVS_LIB_DIR)/liburanus_core.a $(IVS_LIB_DIR)/liburanus_extend.a $(IVS_LIB_DIR)/libaie.a -Wl,-no-whole-archive -Wl,-gc-sections -fno-exceptions -Wl,-export-dynamic
else
URANUS_LIB = -L$(IVS_LIB_DIR) -luranus_tensor -luranus_core -luranus_extend -laie
endif

LIBS = $(SDK_LIB_DIR)/libimp.a $(SDK_LIB_DIR)/libalog.a \
       $(MXU_LIB_DIR)/libmxu_merge.a  \
       $(MXU_LIB_DIR)/libmxu_video.a \
       $(MXU_LIB_DIR)/libmxu_imgproc.a \
       $(MXU_LIB_DIR)/libmxu_core.a

SAMPLES = sample-personvehicleDet

all: $(SAMPLES)

sample-personvehicleDet: sample-common.o sample-personvehicleDet.o
	$(CC) -o $@ $^  $(IVS_LIB_DIR)/libpersonvehicleDet_inf.a $(URANUS_LIB) $(LIBS) $(LDFLAG)
	$(STRIP) $@

%.o:%.c sample-common.h
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -f *.o *~

distclean: clean
	rm -f $(SAMPLES)
