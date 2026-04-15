CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

SET(SRC_ONVIF
#	onvif/src_soap/soapC.c
#	onvif/src_soap/soapServer.c
#	onvif/src_soap/stdsoap2.c
	func_network.c
	func_video.c
	func_log.c
	onvif_func.c
	onvif.c
	func_osd.c
	)

ADD_LIBRARY(ONVIF OBJECT  ${SRC_ONVIF})
SET_TARGET_PROPERTIES(ONVIF PROPERTIES ENABLE_EXPORTS ON)
ADD_DEFINITIONS("-w")
