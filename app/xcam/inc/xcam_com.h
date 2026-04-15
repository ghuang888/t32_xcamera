#ifndef _XCAM_COM_H_
#define _XCAM_COM_H_

enum {
    XMOD_ID_VSTREAM_0,
    XMOD_ID_VSTREAM_1,
    XMOD_ID_VSTREAM_2,
    XMOD_ID_VSTREAM_3,
#if((defined GB28181)||(defined GB35114))
	XMOD_ID_GB_MANAGE,
#endif
    XMOD_ID_RTSP_0,
    XMOD_ID_RTSP_1,
    XMOD_ID_RTSP_2,
    XMOD_ID_RTSP_3,
    XMOD_ID_TFCARD_SAVE,
};

enum {
    MESG_ID_STREAM0,
    MESG_ID_STREAM1,
    MESG_ID_STREAM2,
    MESG_ID_STREAM3,
};

#define SENSOR_RESOLUTION_WIDTH_MAIN 2880
#define SENSOR_RESOLUTION_HEIGHT_MAIN 1620
#define SENSOR_RESOLUTION_WIDTH_SLAVE 640
#define SENSOR_RESOLUTION_HEIGHT_SLAVE 384

typedef struct {
	char osdtitle[20];
	int awb;
	int snapChnSel;
	int snapTimSel;
	int timestampshow;
	int fpsshow;
	int titleshow;
	int ch0_qp;
	int ch1_qp;
	int gamma;
	int filllight;
	int visualangle;
	int backlight;
	int nightmode;
	int snapmanual;
	int titlepos;
}extra_web_param;

#endif
