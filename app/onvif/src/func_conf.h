#ifndef __FUNC_CONF_H__

#define _STR(s) #s
#define STR(s) _STR(s)

#ifdef T31
#define TDS_DEVICE_INFO_MANUFACTURER "Ingenic"
#define TDS_DEVICE_INFO_MODE "xcam"

#if (!defined DATE)
#error should define DATE in makefile
#endif

//configs for onvif alone
#ifdef ONVIF_WITH_CARRIER_SERVER
#if (!defined ONVIF_VER_MAJOR) || (!defined ONVIF_VER_MINOR)
#error should define ONVIF_VER in makefile
#endif
#define TDS_DEVICE_INFO_FW_VERSION "XCAM-T31-"STR(ONVFI_VER_MAJOR)"."STR(ONVFI_VER_MINOR)"-"STR(DATE)
#define TT_MEDIA_0_NAME "main"
#define TT_MEDIA_1_NAME "second"

//configs for xcam
#else
#if (!defined XCAM_VER_MAJOR) || (!defined XCAM_VER_MINOR)
#error should define XCAM_VER in makefile
#endif
#define TDS_DEVICE_INFO_FW_VERSION "XCAM-T31-"STR(XCAM_VER_MAJOR)"."STR(XCAM_VER_MINOR)"-"STR(DATE)
#define TT_MEDIA_0_NAME "stream0"
#define TT_MEDIA_1_NAME "stream1"
#endif

#elif defined T41
#define TDS_DEVICE_INFO_MANUFACTURER "Ingenic"
#define TDS_DEVICE_INFO_MODE "xcam"

#if (!defined DATE)
#error should define DATE in makefile
#endif

//configs for onvif alone
#ifdef ONVIF_WITH_CARRIER_SERVER
#if (!defined ONVIF_VER_MAJOR) || (!defined ONVIF_VER_MINOR)
#error should define ONVIF_VER in makefile
#endif
#define TDS_DEVICE_INFO_FW_VERSION "XCAM-T41-"STR(ONVFI_VER_MAJOR)"."STR(ONVFI_VER_MINOR)"-"STR(DATE)
#define TT_MEDIA_0_NAME "main"
#define TT_MEDIA_1_NAME "second"

//configs for xcam
#else
#if (!defined XCAM_VER_MAJOR) || (!defined XCAM_VER_MINOR)
#error should define XCAM_VER in makefile
#endif
#define TDS_DEVICE_INFO_FW_VERSION "XCAM-T41-"STR(XCAM_VER_MAJOR)"."STR(XCAM_VER_MINOR)"-"STR(DATE)
#define TT_MEDIA_0_NAME "stream0"
#define TT_MEDIA_1_NAME "stream1"
#endif

#elif defined T30

#else
#define TT_MEDIA_0_NAME "stream0"
#define TT_MEDIA_1_NAME "stream1"
#define TDS_DEVICE_INFO_MANUFACTURER "Ingenic"
#define TDS_DEVICE_INFO_MODE "xcam"
#define TDS_DEVICE_INFO_FW_VERSION "XCAM-T32-"STR(XCAM_VER_MAJOR)"."STR(XCAM_VER_MINOR)"-"STR(DATE)
#endif

#endif
