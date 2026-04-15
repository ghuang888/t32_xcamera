CMAKE_MINIMUM_REQUIRED(VERSION 3.0)
PROJECT (XCAM)

SET(CMAKE_SYSTEM_NAME Linux)
SET(CMAKE_C_COMPILER "mips-linux-gnu-gcc")
SET(CMAKE_CXX_COMPILER "mips-linux-gnu-g++")
SET(CMAKE_C_FLAGS " -Wall -march=mips32r2 -DNO_OPENSSL=1 -g -rdynamic -lc")
SET(CMAKE_CXX_FLAGS " -Wall -march=mips32r2 -std=c++11 -DNO_OPENSSL=1 -g -rdynamic -lc")

include (Config.cmake)

if((${CONFIG_DPLATFORM} STREQUAL "T20" OR ${CONFIG_DPLATFORM} STREQUAL "T21") AND ${CONFIG_ENCODE} STREQUAL "H265")
	MESSAGE(FATAL_ERROR "T20 and T21 don't support H265")
endif()

if(${CONFIG_DPLATFORM} STREQUAL "T41")
	ADD_DEFINITIONS(-DT41)
endif()

if(${CONFIG_DPLATFORM} STREQUAL "T32")
	ADD_DEFINITIONS(-DT32)
endif()

if(${CONFIG_ENCODE} STREQUAL "H264")
	ADD_DEFINITIONS(-DH264_encode)
elseif(${CONFIG_ENCODE} STREQUAL "H265")
	ADD_DEFINITIONS(-DH265_encode)
endif()

if(${CONFIG_GB_VERSION} STREQUAL "GB28181")
	ADD_DEFINITIONS(-DGB28181)
elseif(${CONFIG_GB_VERSION} STREQUAL "GB35114")
	ADD_DEFINITIONS(-DGB35114)
endif()

if(${CONFIG_DL_ENABLE} STREQUAL "DL_ENABLE")
	ADD_DEFINITIONS(-DDL_ENABLE)
endif()

if(${CONFIG_MOVEDET_ENABLE} STREQUAL "MOVEDET_ENABLE")
	ADD_DEFINITIONS(-DMOVEDET_ENABLE)
endif()

if(${CONFIG_PLATEREC_ENABLE} STREQUAL "PLATEREC_ENABLE")
	ADD_DEFINITIONS(-DPLATEREC_ENABLE)
endif()

if(${CONFIG_FACEDET_ENABLE} STREQUAL "FACEDET_ENABLE")
	ADD_DEFINITIONS(-DFACEDET_ENABLE)
endif()

if(${CONFIG_PERVEHPETDET_ENABLE} STREQUAL "PERVEHPETDET_ENABLE")
	ADD_DEFINITIONS(-DPERVEHPETDET_ENABLE)
endif()

if(${CONFIG_FIREWORKSDET_ENABLE} STREQUAL "FIREWORKSDET_ENABLE")
	ADD_DEFINITIONS(-DFIREWORKSDET_ENABLE)
endif()

if(${CONFIG_GEKKO_ENABLE} STREQUAL "GEKKO_ENABLE")
	ADD_DEFINITIONS(-DAIISP)
endif()

MESSAGE("Using ${CONFIG_DPLATFORM} xcamera ${CONFIG_ENCODE} encode and ${CONFIG_GB_VERSION}")

INCLUDE_DIRECTORIES(./)
INCLUDE_DIRECTORIES(af/include/)
INCLUDE_DIRECTORIES(xcam/inc)
INCLUDE_DIRECTORIES(onvif/inc)
INCLUDE_DIRECTORIES(onvif/src_soap)
INCLUDE_DIRECTORIES(cJSON/)
INCLUDE_DIRECTORIES(src)
INCLUDE_DIRECTORIES(conf/inc)
INCLUDE_DIRECTORIES(rcf/inc)
INCLUDE_DIRECTORIES(rcf/RCF/include)
INCLUDE_DIRECTORIES(af/include/)

if(${CONFIG_DL_ENABLE} STREQUAL "DL_ENABLE")
INCLUDE_DIRECTORIES(persondet_ivs/IVS/include/)
endif()
if(${CONFIG_MOVEDET_ENABLE} STREQUAL "MOVEDET_ENABLE")
INCLUDE_DIRECTORIES(movedet_ivs/IVS/include/)
endif()
if(${CONFIG_PLATEREC_ENABLE} STREQUAL "PLATEREC_ENABLE")
INCLUDE_DIRECTORIES(platerec_ivs/IVS/include/)
endif()
if(${CONFIG_FACEDET_ENABLE} STREQUAL "FACEDET_ENABLE")
INCLUDE_DIRECTORIES(facedet_ivs/IVS/include/)
endif()
if(${CONFIG_PERVEHPETDET_ENABLE} STREQUAL "PERVEHPETDET_ENABLE")
INCLUDE_DIRECTORIES(personvehiclepetdet_ivs/IVS/include/)
endif()
if(${CONFIG_FIREWORKSDET_ENABLE} STREQUAL "FIREWORKSDET_ENABLE")
INCLUDE_DIRECTORIES(fireworksdet_ivs/IVS/include/)
endif()
if(${CONFIG_GEKKO_ENABLE} STREQUAL "GEKKO_ENABLE")
INCLUDE_DIRECTORIES(gekko_sdk/install/include)
endif()

if(${CONFIG_DPLATFORM} STREQUAL "T41")
	INCLUDE_DIRECTORIES(../platform/ingenic/t41/sdk/include)
endif()
if(${CONFIG_DPLATFORM} STREQUAL "T32")
	INCLUDE_DIRECTORIES(../platform/ingenic/t32/sdk/include)
endif()

INCLUDE_DIRECTORIES(rtsp_client/_install_live555_mips_uclibc/include/BasicUsageEnvironment)
INCLUDE_DIRECTORIES(rtsp_client/_install_live555_mips_uclibc/include/groupsock)
INCLUDE_DIRECTORIES(rtsp_client/_install_live555_mips_uclibc/include/liveMedia)
INCLUDE_DIRECTORIES(rtsp_client/_install_live555_mips_uclibc/include/UsageEnvironment)

if(${CONFIG_GB_VERSION} STREQUAL "GB28181" OR ${CONFIG_GB_VERSION} STREQUAL "GB35114")
	INCLUDE_DIRECTORIES(gbmanage/include)
endif()

ADD_DEFINITIONS(-lpthread)

SET(SRC_XCAM
	xcam/src/xcam_thread.c
	xcam/src/xcam_log.c
	xcam/src/xcam_module.c
	xcam/src/xcam_msg.c
	xcam/src/xcam_rtsp.c
	xcam/src/xcam_osd.c
	xcam/src/xcam_onvif.c
	xcam/src/xcam_stream.c
	xcam/src/xcam_simple_list.c
	xcam/src/xcam_main.c
	xcam/src/xcam_osd.c
	xcam/src/xcam_video.c
	xcam/src/xcam_extra.c
	xcam/src/xcam_audio_stream.c
	xcam/src/xcam_conf.c
	xcam/src/xcam_conf_sys.c
	xcam/src/xcam_conf_video.c
	xcam/src/xcam_conf_json.c
	xcam/src/xcam_conf_process.c
	xcam/src/xcam_web_process.c
	xcam/src/xcam_network.c
	xcam/src/xcam_conf_network.c
	xcam/src/xcam_cli_options.c
	xcam/src/xcam_system.c
	xcam/src/xcam_message_queue.c
	xcam/src/xcam_ptz.c
	)

SET(SRC_ONVIF
#	onvif/src_soap/soapC.c
#	onvif/src_soap/soapServer.c
#	onvif/src_soap/stdsoap2.c
	onvif/src/func_network.c
	onvif/src/func_video.c
	onvif/src/func_log.c
	onvif/src/onvif_func.c
	onvif/src/onvif.c
	onvif/src/func_osd.c
	)


SET(SRC_CJSON
	cJSON/cJSON.c
	)

SET(SRC_SOCKET_TCP
	socket/socket_server_tcp.c
	)

SET(SRC_RCFSERVER
    rcf/rcf_server/Server.cpp
	rcf/rcf_server/Serverdump.cpp
    )

SET(SRC_CONF
    conf/src/conf_process.c
    conf/src/conf_msg_process.c
    )

SET(SRC_RTSP
	rtsp/H265VideoServerMediaSubsession.cpp
	rtsp/H265VideoStreamSource.cpp
	rtsp/H264VideoServerMediaSubsession.cpp
	rtsp/G711AudioStreamServerMediaSubsession.cpp
	rtsp/G711AudioStreamSource.cpp
	rtsp/H264VideoStreamSource.cpp
	rtsp/StreamSource.cpp
	rtsp/LiveRTSP.cpp
	)

LINK_DIRECTORIES(rtsp_client/_install_live555_mips_uclibc/lib)
LINK_DIRECTORIES(onvif/src_soap/)
LINK_DIRECTORIES(rcf/lib)
LINK_DIRECTORIES(af/lib)
if(${CONFIG_GEKKO_ENABLE} STREQUAL "GEKKO_ENABLE")
LINK_DIRECTORIES(gekko_sdk/install/lib/uclibc)
LINK_DIRECTORIES(gekko_sdk/install/lib/gclibc)
endif()
if(${CONFIG_FIREWORKSDET_ENABLE} STREQUAL "FIREWORKSDET_ENABLE")
LINK_DIRECTORIES(fireworksdet_ivs/IVS/lib/uclibc)
LINK_DIRECTORIES(fireworksdet_ivs/MXU/lib/uclibc)
endif()
if(${CONFIG_PERVEHPETDET_ENABLE} STREQUAL "PERVEHPETDET_ENABLE")
LINK_DIRECTORIES(personvehiclepetdet_ivs/IVS/lib/uclibc)
LINK_DIRECTORIES(personvehiclepetdet_ivs/MXU/lib/uclibc)
endif()
if(${CONFIG_DL_ENABLE} STREQUAL "DL_ENABLE")
LINK_DIRECTORIES(persondet_ivs/IVS/lib/uclibc)
LINK_DIRECTORIES(persondet_ivs/MXU/lib/uclibc)
endif()
if(${CONFIG_MOVEDET_ENABLE} STREQUAL "MOVEDET_ENABLE")
LINK_DIRECTORIES(movedet_ivs/IVS/lib/uclibc)
LINK_DIRECTORIES(movedet_ivs/MXU/lib/uclibc)
endif()
if(${CONFIG_PLATEREC_ENABLE} STREQUAL "PLATEREC_ENABLE")
LINK_DIRECTORIES(platerec_ivs/IVS/lib/uclibc)
LINK_DIRECTORIES(platerec_ivs/MXU/lib/uclibc)
endif()
if(${CONFIG_FACEDET_ENABLE} STREQUAL "FACEDET_ENABLE")
LINK_DIRECTORIES(facedet_ivs/IVS/lib/uclibc)
LINK_DIRECTORIES(facedet_ivs/MXU/lib/uclibc)
endif()

if(${CONFIG_GB_VERSION} STREQUAL "GB28181" OR ${CONFIG_GB_VERSION} STREQUAL "GB35114")
	LINK_DIRECTORIES(gbmanage/lib)
endif()

option(DIRECT_MODE "DIRECT_MODE select" OFF)
 
if(DIRECT_MODE)
	ADD_DEFINITIONS(-DDIRECT_MODE)
	MESSAGE("you select DIRECT_MODE")
else()
	MESSAGE("you dont't select DIRECT_MODE")
endif()

option(DOUBLE_SENSOR "DOUBLE_SENSOR select" OFF)

if(DOUBLE_SENSOR)
	ADD_DEFINITIONS(-DXCAM_DOUBLE_SENSOR)
	MESSAGE("you select DOUBLE_SENSOR")
else()
	MESSAGE("you dont't select DOUBLE_SENSOR")
endif()

if(${CONFIG_DPLATFORM} STREQUAL "T41")
	LINK_DIRECTORIES(../platform/ingenic/t41/sdk/lib/uclibc)
endif()
if(${CONFIG_DPLATFORM} STREQUAL "T32")
	LINK_DIRECTORIES(../platform/ingenic/t32/sdk/lib/uclibc)
endif()
string(TIMESTAMP DATE %Y%m%d%H%M)
if(${CONFIG_FIRST_SNESOR_NAME} STREQUAL "")
      MESSAGE(FATAL_ERROR "Please set sensor name in Config.cmake")
else()
    string(TOUPPER ${CONFIG_FIRST_SNESOR_NAME} CONFIG_FIRST_SNESOR_NAME)
    ADD_DEFINITIONS(-D${CONFIG_FIRST_SNESOR_NAME})
endif()

ADD_DEFINITIONS("-Wall")
ADD_SUBDIRECTORY(onvif/src/)
ADD_SUBDIRECTORY(rcf/rcf_server/)
# ${SRC_ONVIF}
if(${CONFIG_GB_VERSION} STREQUAL "GB28181" OR ${CONFIG_GB_VERSION} STREQUAL "GB35114")
	if(${CONFIG_DL_ENABLE} STREQUAL "DL_ENABLE")
		ADD_EXECUTABLE(xcamera ${SRC_XCAM}  ${SRC_RTSP} ${SRC_CJSON} ${SRC_SOCKET_TCP} ${SRC_GB} ${SRC_CONF} ${SRC_PERSPNDET}  $<TARGET_OBJECTS:ONVIF>  $<TARGET_OBJECTS:RCFSERVER>)
	else()
		ADD_EXECUTABLE(xcamera ${SRC_XCAM}  ${SRC_RTSP} ${SRC_CJSON} ${SRC_SOCKET_TCP} ${SRC_GB} ${SRC_CONF} $<TARGET_OBJECTS:ONVIF>  $<TARGET_OBJECTS:RCFSERVER>)
	endif()
else()
	if(${CONFIG_DL_ENABLE} STREQUAL "DL_ENABLE")
		ADD_EXECUTABLE(xcamera ${SRC_XCAM}  ${SRC_RTSP} ${SRC_CJSON} ${SRC_SOCKET_TCP} ${SRC_CONF} ${SRC_PERSPNDET} $<TARGET_OBJECTS:ONVIF>  $<TARGET_OBJECTS:RCFSERVER>)
	else()
		ADD_EXECUTABLE(xcamera ${SRC_XCAM}  ${SRC_RTSP} ${SRC_CJSON} ${SRC_SOCKET_TCP} ${SRC_CONF} $<TARGET_OBJECTS:ONVIF>  $<TARGET_OBJECTS:RCFSERVER>)
	endif()
endif()

SET_TARGET_PROPERTIES(xcamera PROPERTIES
	ARCHIVE_OUTPUT_DIRECTORY "out"
	RUNTIME_OUTPUT_DIRECTORY "out"
	LINK_FLAGS "-muclibc"
	COMPILE_FLAGS "-muclibc"
	)

TARGET_COMPILE_DEFINITIONS(xcamera
	PRIVATE
	${XCAM_VER}
	DATE=${DATE}
	)

if(${CONFIG_DPLATFORM} STREQUAL "T41")
	TARGET_LINK_LIBRARIES(xcamera  libonvif_glibc.a dl m rt -pthread RCF_720 c)
elseif (${CONFIG_DPLATFORM} STREQUAL "T32")
	TARGET_LINK_LIBRARIES(xcamera  libonvif_glibc.a dl m rt -pthread RCF_720 c)
else()
	TARGET_LINK_LIBRARIES(xcamera c libonvif_glibc.a dl m rt -pthread RCF Af)
endif()

if(${CONFIG_DPLATFORM} STREQUAL "T41")
	TARGET_LINK_LIBRARIES(xcamera
		libliveMedia.a
		libgroupsock.a
		libBasicUsageEnvironment.a
		libUsageEnvironment.a
		)
	# TARGET_LINK_LIBRARIES(xcamera
	# 	# libimp.a
	# 	# libalog.a
	# 	)
else()
	TARGET_LINK_LIBRARIES(xcamera
		libliveMedia.a
		libgroupsock.a
		libBasicUsageEnvironment.a
		libUsageEnvironment.a
		)
	# TARGET_LINK_LIBRARIES(xcamera
	# 	libimp.a
	# 	libalog.a
	# 	)
endif()
if(${CONFIG_FIREWORKSDET_ENABLE} STREQUAL "FIREWORKSDET_ENABLE")
TARGET_LINK_LIBRARIES(xcamera
	libfireworksDet_inf.so
	)
endif()

if(${CONFIG_FACEDET_ENABLE} STREQUAL "FACEDET_ENABLE")
TARGET_LINK_LIBRARIES(xcamera
	libfaceDet_inf.so
	)
endif()

if(${CONFIG_PERVEHPETDET_ENABLE} STREQUAL "PERVEHPETDET_ENABLE")
TARGET_LINK_LIBRARIES(xcamera
	libpersonvehicleDet_inf.so
	)
endif()

if(${CONFIG_PLATEREC_ENABLE} STREQUAL "PLATEREC_ENABLE")
TARGET_LINK_LIBRARIES(xcamera
	libplateRec_inf.so
	)
endif()

if(${CONFIG_MOVEDET_ENABLE} STREQUAL "MOVEDET_ENABLE")
TARGET_LINK_LIBRARIES(xcamera
	libmove_inf.so
	)
endif()

if(${CONFIG_DL_ENABLE} STREQUAL "DL_ENABLE")
TARGET_LINK_LIBRARIES(xcamera
    liburanus_tensor.so
	liburanus_core.so
	liburanus_extend.so
	libaie.so
	libimp.so
	libalog.so
	)
TARGET_LINK_LIBRARIES(xcamera
	libmxu_merge.a
	# libmxu_objdetect.a
	# libmxu_contrib.a
	libmxu_video.a
	libmxu_imgproc.a
	libmxu_core.a
	)
endif()

if(${CONFIG_GEKKO_ENABLE} STREQUAL "GEKKO_ENABLE")
TARGET_LINK_LIBRARIES(xcamera
    libvenus_tensor.so
	libgekko.so
	)
endif()

# ADD_EXECUTABLE(onvif ${SRC_ONVIF})
# if(${CONFIG_DPLATFORM} STREQUAL "T41")
# 	TARGET_LINK_LIBRARIES(onvif  libonvif_glibc.a dl m rt -pthread RCF_720)
# elseif(${CONFIG_DPLATFORM} STREQUAL "T32")
# 	TARGET_LINK_LIBRARIES(xcamera  libonvif_glibc.a dl m rt -pthread RCF_720)
# else()
# 	TARGET_LINK_LIBRARIES(onvif  libonvif_glibc.a dl m rt -pthread RCF Af)
# endif()

# SET_TARGET_PROPERTIES(onvif PROPERTIES
# 	ARCHIVE_OUTPUT_DIRECTORY "out"
# 	RUNTIME_OUTPUT_DIRECTORY "out"
# 	LINK_FLAGS "-muclibc"
# 	COMPILE_FLAGS "-muclibc"
# 	)
# TARGET_COMPILE_DEFINITIONS(onvif
# 	PRIVATE
# 	ONVIF_WITH_CARRIER_SERVER
# 	${ONVIF_VER}
# 	DATE=${DATE}
# 	)

# ADD_EXECUTABLE(onvif_glibc ${SRC_ONVIF})

# if(${CONFIG_DPLATFORM} STREQUAL "T41")
# 	TARGET_LINK_LIBRARIES(onvif_glibc libonvif_glibc.a dl m rt -pthread RCF_720)
# else()
# 	TARGET_LINK_LIBRARIES(onvif_glibc libonvif_glibc.a dl m rt -pthread RCF Af)
# endif()

# SET_TARGET_PROPERTIES(onvif_glibc PROPERTIES
# 	ARCHIVE_OUTPUT_DIRECTORY "out"
# 	RUNTIME_OUTPUT_DIRECTORY "out"
# 	)
# TARGET_COMPILE_DEFINITIONS(onvif_glibc
# 	PRIVATE
# 	ONVIF_WITH_CARRIER_SERVER
# 	${ONVIF_VER}
# 	DATE=${DATE}
# 	)

ADD_CUSTOM_COMMAND(TARGET xcamera
	POST_BUILD
	COMMAND make clean -C ${CMAKE_SOURCE_DIR}/daemon
	COMMAND make -C ${CMAKE_SOURCE_DIR}/daemon
	COMMAND cp ${CMAKE_SOURCE_DIR}/daemon/xcam_daemon ${CMAKE_SOURCE_DIR}/build/out
	)

ADD_CUSTOM_COMMAND(TARGET xcamera
	POST_BUILD
	COMMAND make clean -C ${CMAKE_SOURCE_DIR}/mac_set
	COMMAND make -C ${CMAKE_SOURCE_DIR}/mac_set
	COMMAND cp ${CMAKE_SOURCE_DIR}/mac_set/mac_set ${CMAKE_SOURCE_DIR}/build/out
	)
