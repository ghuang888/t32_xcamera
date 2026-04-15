/*
 * sample-audio.c
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <xcam_general.h>
#include <imp/imp_audio.h>
#include <imp/imp_log.h>
#include "xcam_audio_stream.h"
#define TAG "Sample-Audio"

#define AEC_SAMPLE_RATE 8000
#define AEC_SAMPLE_TIME 10

#define IMP_AUDIO_BUF_SIZE (5 * (AEC_SAMPLE_RATE * sizeof(short) * AEC_SAMPLE_TIME / 1000))
#define IMP_AUDIO_RECORD_NUM 500


/* My G711 Encoder */
#define SIGN_BIT    (0x80)      /* Sign bit for a A-law byte. */
#define QUANT_MASK  (0xf)       /* Quantization field mask. */
#define NSEGS       (8)         /* Number of A-law segments. */
#define SEG_SHIFT   (4)         /* Left shift for segment number. */
#define SEG_MASK    (0x70)      /* Segment field mask. */
#define BIAS        (0x84)      /* Bias for linear code. */

static xcam_audio_stream audiostream;
static short seg_end[8] = {0xFF, 0x1FF, 0x3FF, 0x7FF,
	0xFFF, 0x1FFF, 0x3FFF, 0x7FFF};

static int search(int val, short	*table, int	size)
{
	int	i;

	for (i = 0; i < size; i++) {
		if (val <= *table++)
			return (i);
	}
	return (size);
}

/*static int alaw2linear( unsigned char a_val )
{
	int	t;
	int	seg;

	a_val ^= 0x55;

	t = (a_val & QUANT_MASK) << 4;
	seg = ( (unsigned)a_val & SEG_MASK ) >> SEG_SHIFT;
	switch (seg)
	{
		case 0:
			t += 8;
			break;
		case 1:
			t += 0x108;
			break;
		default:
			t += 0x108;
			t <<= seg - 1;
	}
	return ((a_val & SIGN_BIT) ? t : -t);
} for warning*/

static int ulaw2linear(unsigned char u_val)
{
	int	t;

	u_val = ~u_val;

	t = ((u_val & QUANT_MASK) << 3) + BIAS;
	t <<= ((unsigned)u_val & SEG_MASK) >> SEG_SHIFT;

	return ((u_val & SIGN_BIT) ? (BIAS - t) : (t - BIAS));
}

static unsigned char linear2alaw(int pcm_val)
{
	int		mask;
	int		seg;
	unsigned char	aval;

	if (pcm_val >= 0) {
		mask = 0xD5;		/* sign (7th) bit = 1 */
	} else {
		mask = 0x55;		/* sign bit = 0 */
		pcm_val = -pcm_val - 8;
	}

	seg = search(pcm_val, seg_end, 8);
	if (seg >= 8)		/* out of range, return maximum value. */
		return (0x7F ^ mask);
	else {
		aval = seg << SEG_SHIFT;
		if (seg < 2)
			aval |= (pcm_val >> 4) & QUANT_MASK;
		else
			aval |= (pcm_val >> (seg + 3)) & QUANT_MASK;
		return (aval ^ mask);
	}
}

static unsigned char linear2ulaw(int pcm_val)
{
	int		mask;
	int		seg;
	unsigned char	uval;

	if (pcm_val < 0) {
		pcm_val = BIAS - pcm_val;
		mask = 0x7F;
	} else {
		pcm_val += BIAS;
		mask = 0xFF;
	}

	seg = search(pcm_val, seg_end, 8);
	if (seg >= 8)		/* out of range, return maximum value. */
		return (0x7F ^ mask);
	else {
		uval = (seg << 4) | ((pcm_val >> (seg + 3)) & 0xF);
		return (uval ^ mask);
	}
}

/*static int my_g711a_decode( short amp[], const unsigned char g711a_data[], int g711a_bytes )
{
	int i;
	int samples;
	unsigned char code;
	int sl;

	for ( samples = i = 0; ; )
	{
		if (i >= g711a_bytes)
			break;
		code = g711a_data[i++];

		sl = alaw2linear( code );

		amp[samples++] = (short) sl;
	}
	return samples*2;
} for warning*/

static int my_g711u_decode(short amp[], const unsigned char g711u_data[], int g711u_bytes)
{
	int i;
	int samples;
	unsigned char code;
	int sl;

	for (samples = i = 0;;)
	{
		if (i >= g711u_bytes)
			break;
		code = g711u_data[i++];

		sl = ulaw2linear(code);

		amp[samples++] = (short) sl;
	}
	return samples*2;
}
static int my_g711a_encode(unsigned char g711_data[], const short amp[], int len)
{
	int i;

	for (i = 0;  i < len;  i++) {
		g711_data[i] = linear2alaw(amp[i]);
	}

	return len;
}

static int my_g711u_encode(unsigned char g711_data[], const short amp[], int len)
{
	int i;

	for (i = 0;  i < len;  i++) {
		g711_data[i] = linear2ulaw(amp[i]);
	}

	return len;
}

static int MY_Audio_PCM2G711a(char *InAudioData, char *OutAudioData, int DataLen)
{
	int Retaen = 0;

	if((NULL == InAudioData) && (NULL == OutAudioData) && (0 == DataLen)) {
		IMP_LOG_ERR(TAG, "Error, empty data or transmit failed, exit !\n");
		return XCAM_ERROR;
	}

	Retaen = my_g711a_encode((unsigned char *)OutAudioData, (short*)InAudioData, DataLen/2);

	return Retaen;
}

static int MY_Audio_PCM2G711u(char *InAudioData, char *OutAudioData, int DataLen)
{
	int Retuen = 0;

	if((NULL == InAudioData) && (NULL == OutAudioData) && (0 == DataLen)) {
		IMP_LOG_ERR(TAG, "Error, empty data or transmit failed, exit !\n");
		return XCAM_ERROR;
	}

	Retuen = my_g711u_encode((unsigned char *)OutAudioData, (short*)InAudioData, DataLen/2);

	return Retuen;
}

/*static int MY_Audio_G711a2PCM(char *InAudioData, char *OutAudioData, int DataLen)
{
	int Retade = 0;

	if((NULL == InAudioData) && (NULL == OutAudioData) && (0 == DataLen)) {
		IMP_LOG_ERR(TAG, "Error, empty data or transmit failed, exit !\n");
		return XCAM_ERROR;
	}

	Retade = my_g711a_decode((short*)OutAudioData, (unsigned char *)InAudioData, DataLen);

	return Retade;
} for warning*/

static int MY_Audio_G711u2PCM(char *InAudioData, char *OutAudioData, int DataLen)
{
	int Retude = 0;

	if((NULL == InAudioData) && (NULL == OutAudioData) && (0 == DataLen)) {
		IMP_LOG_ERR(TAG, "Error, empty data or transmit failed, exit !\n");
		return XCAM_ERROR;
	}

	Retude = my_g711u_decode((short*)OutAudioData, (unsigned char *)InAudioData, DataLen);

	return Retude;
}

static int MY_G711A_Encode_Frm(void *encoder, IMPAudioFrame *data, unsigned char *outbuf,int *outLen)
{
	*outLen = MY_Audio_PCM2G711a((char *)data->virAddr, (char *)outbuf, data->len);
	return 0;
}

static int MY_G711U_Encode_Frm(void *encoder, IMPAudioFrame *data, unsigned char *outbuf,int *outLen)
{
	*outLen = MY_Audio_PCM2G711u((char *)data->virAddr, (char *)outbuf, data->len);
	return 0;
}

/*static int MY_G711A_Decode_Frm(void *decoder, unsigned char
		*inbuf,int inLen, unsigned short *outbuf,int
		*outLen,int *chns)
{
	*outLen = MY_Audio_G711a2PCM((char *)inbuf, (char *)outbuf, inLen);
	return 0;
}  for warning*/
static int MY_G711U_Decode_Frm(void *decoder, unsigned char
		*inbuf,int inLen, unsigned short *outbuf,int
		*outLen,int *chns)
{
	*outLen = MY_Audio_G711u2PCM((char *)inbuf, (char *)outbuf, inLen);
	return 0;
}

static int xcam_config_audio_encode(void){
    int ret;
    /* my G711A encoder Register */
	audiostream.handle_g711a = 0;
	IMPAudioEncEncoder my_encoder;
	my_encoder.maxFrmLen = 1024;
	sprintf(my_encoder.name, "%s", "MY_G711A");
	my_encoder.openEncoder = NULL;
	my_encoder.encoderFrm = MY_G711A_Encode_Frm;
	my_encoder.closeEncoder = NULL;
	ret = IMP_AENC_RegisterEncoder(&audiostream.handle_g711a , &my_encoder);
	if(ret != 0) {
		IMP_LOG_ERR(TAG, "IMP_AENC_RegisterEncoder failed\n");
		return XCAM_ERROR;
	}

	/* my G711U encoder Register */
	audiostream.handle_g711u = 0;
	memset(&my_encoder, 0x0, sizeof(my_encoder));
	my_encoder.maxFrmLen = 1024;
	sprintf(my_encoder.name, "%s", "MY_G711U");
	my_encoder.openEncoder = NULL;
	my_encoder.encoderFrm = MY_G711U_Encode_Frm;
	my_encoder.closeEncoder = NULL;
	ret = IMP_AENC_RegisterEncoder(&audiostream.handle_g711u, &my_encoder);
	if(ret != 0) {
		IMP_LOG_ERR(TAG, "IMP_AENC_RegisterEncoder failed\n");
		return XCAM_ERROR;
	}

    audiostream.AeChn = 0;
	IMPAudioEncChnAttr attr;
	attr.type = audiostream.handle_g711a; /* Use the My method to encoder. if use the system method is attr.type = PT_G711A; */
	attr.bufSize = 20;
	ret = IMP_AENC_CreateChn(audiostream.AeChn, &attr);
	if(ret != 0) {
		IMP_LOG_ERR(TAG, "imp audio encode create channel failed\n");
		return XCAM_ERROR;
	}
    return ret; 
}

static int xcam_config_audio_stream(void){
    int ret;

	/* Step 1: set public attribute of AI device. */
	audiostream.devID = 1;
	IMPAudioIOAttr attr;
	attr.samplerate = AUDIO_SAMPLE_RATE_8000;
	attr.bitwidth = AUDIO_BIT_WIDTH_16;
	attr.soundmode = AUDIO_SOUND_MODE_MONO;
	attr.frmNum = 20;
	attr.numPerFrm = 400;
	attr.chnCnt = 1;
	ret = IMP_AI_SetPubAttr(audiostream.devID, &attr);
	if(ret != 0) {
		IMP_LOG_ERR(TAG, "set ai %d attr err: %d\n", audiostream.devID, ret);
		return XCAM_ERROR;
	}
	/* Step 2: enable AI device. */
	ret = IMP_AI_Enable(audiostream.devID);
	if(ret != 0) {
		IMP_LOG_ERR(TAG, "enable ai %d err\n", audiostream.devID);
		return XCAM_ERROR;
	}

	/* Step 3: set audio channel attribute of AI device. */
	audiostream.chnID = 0;
	IMPAudioIChnParam chnParam;
	chnParam.usrFrmDepth = 20;
	chnParam.aecChn = 0;
	ret = IMP_AI_SetChnParam(audiostream.devID, audiostream.chnID , &chnParam);
	if(ret != 0) {
		IMP_LOG_ERR(TAG, "set ai %d channel %d attr err: %d\n", audiostream.devID, audiostream.chnID, ret);
		return XCAM_ERROR;
	}

	/* Step 4: enable AI channel. */
	ret = IMP_AI_EnableChn(audiostream.devID, audiostream.chnID );
	if(ret != 0) {
		IMP_LOG_ERR(TAG, "Audio Record enable channel failed\n");
		return XCAM_ERROR;
	}

	// /* Step.5 enable Aec */
	// ret = IMP_AI_EnableAec(audiostream.devID, audiostream.chnID, 0, 0);
	// if(ret != 0) {
	// 	IMP_LOG_ERR(TAG, "Audio Record enable channel failed\n");
	// 	return XCAM_ERROR;
	// }

	/* Step 5: Set audio channel volume. */
	int chnVol = 60;
	ret = IMP_AI_SetVol(audiostream.devID, audiostream.chnID , chnVol);
	if(ret != 0) {
		IMP_LOG_ERR(TAG, "Audio Record set volume failed\n");
		return XCAM_ERROR;
	}


	int aigain = 28;
	ret = IMP_AI_SetGain(audiostream.devID, audiostream.chnID , aigain);
	if(ret != 0) {
		IMP_LOG_ERR(TAG, "Audio Record Set Gain failed\n");
		return XCAM_ERROR;
	}
    return ret;
}

int xcam_destroy_audio(void){
    int ret;
	ret = IMP_AENC_UnRegisterEncoder(&audiostream.handle_g711a);
	if(ret != 0) {
		IMP_LOG_ERR(TAG, "IMP_AENC_UnRegisterEncoder failed\n");
		return XCAM_ERROR;
	}

	ret = IMP_AENC_UnRegisterEncoder(&audiostream.handle_g711u);
	if(ret != 0) {
		IMP_LOG_ERR(TAG, "IMP_AENC_UnRegisterEncoder failed\n");
		return XCAM_ERROR;
	}

	/* destroy the encode channel. */
	ret = IMP_AENC_DestroyChn(audiostream.AeChn);
	if(ret != 0) {
		IMP_LOG_ERR(TAG, "imp audio encode destroy channel failed\n");
		return XCAM_ERROR;
	}

	// ret = IMP_AI_DisableAec(audiostream.devID, audiostream.chnID);
	// if(ret != 0) {
	// 	IMP_LOG_ERR(TAG, "IMP_AI_DisableAecRefFrame\n");
	// 	return XCAM_ERROR;
	// }

    /* Step 9: disable the audio channel. */
	ret = IMP_AI_DisableChn(audiostream.devID, audiostream.chnID);
	if(ret != 0) {
		IMP_LOG_ERR(TAG, "Audio channel disable error\n");
		return XCAM_ERROR;
	}

	/* Step 10: disable the audio devices. */
	ret = IMP_AI_Disable(audiostream.devID);
	if(ret != 0) {
		IMP_LOG_ERR(TAG, "Audio device disable error\n");
		return XCAM_ERROR;
	}
	return XCAM_SUCCESS;
}

int xcam_release_audio_stream(IMPAudioStream *stream){
    	/* release stream. */
	int ret = IMP_AENC_ReleaseStream(audiostream.AeChn, stream);
	if(ret != 0) {
		IMP_LOG_ERR(TAG, "imp audio encode release stream failed\n");
		return XCAM_ERROR;
	}
    return 0;
}

int xcam_record_audio_stream(IMPAudioStream *stream)
{
    int ret = XCAM_ERROR;
	/* Step 6: get audio record frame. */
	ret = IMP_AI_PollingFrame(audiostream.devID, audiostream.chnID, 1000);
	if (ret != 0 ) {
		IMP_LOG_ERR(TAG, "Audio Polling Frame Data error\n");
	}

	ret = IMP_AI_GetFrame(audiostream.devID, audiostream.chnID, &(audiostream.frm), BLOCK);
	if(ret != 0) {
		IMP_LOG_ERR(TAG, "Audio Get Frame Data error\n");
		return XCAM_ERROR;
	}
//    printf("%s[%d]  xxxx msgid=%d sqp=%d\n", __func__, __LINE__,  audiostream.frm.len, audiostream.frm.timeStamp);
	ret = IMP_AENC_SendFrame(audiostream.AeChn, &(audiostream.frm));
	if(ret != 0) {
		IMP_LOG_ERR(TAG, "imp audio encode send frame failed\n");
		return XCAM_ERROR;
	}
	/* get audio encode frame. */

	ret = IMP_AENC_PollingStream(audiostream.AeChn, 1000);
	if (ret != 0) {
		IMP_LOG_ERR(TAG, "imp audio encode polling stream failed\n");
	}

	ret = IMP_AENC_GetStream(audiostream.AeChn, stream, BLOCK);
	if(ret != 0) {
		IMP_LOG_ERR(TAG, "imp audio encode get stream failed\n");
		return XCAM_ERROR;
	}
	// printf("%s[%d]  xxxx msgid=%d sqp=%d\n", __func__, __LINE__,  stream->len, stream->timeStamp);

	ret = IMP_AI_ReleaseFrame(audiostream.devID, audiostream.chnID, &(audiostream.frm));
	if(ret != 0) {
		IMP_LOG_ERR(TAG, "Audio release frame data error\n");
		return XCAM_ERROR;
	}
    return 0;
}

int xcam_playaudio_stream(uint8_t *buf_g711, int len){


		/* Send a frame to decoder. */
		IMPAudioStream stream_in;
		stream_in.stream = (uint8_t *)buf_g711;
		stream_in.len = len;
		int ret = IMP_ADEC_SendStream(audiostream.adChn, &stream_in, BLOCK);
		if(ret != 0) {
			IMP_LOG_ERR(TAG, "imp audio encode send frame failed\n");
			return XCAM_ERROR;
		}

		/* get audio decoder frame. */
		IMPAudioStream stream_out;
		ret = IMP_ADEC_PollingStream(audiostream.adChn, 1000);
		if(ret != 0) {
			IMP_LOG_ERR(TAG, "imp audio encode polling stream failed\n");
		}

		ret = IMP_ADEC_GetStream(audiostream.adChn, &stream_out, BLOCK);
		if(ret != 0) {
			IMP_LOG_ERR(TAG, "imp audio decoder get stream failed\n");
			return XCAM_ERROR;
		}

		/* save the decoder data to the file. */
		// fwrite(stream_out.stream, 1, stream_out.len, file_pcm);

		/* release stream. */
		ret = IMP_ADEC_ReleaseStream(audiostream.adChn, &stream_out);
		if(ret != 0) {
			IMP_LOG_ERR(TAG, "imp audio decoder release stream failed\n");
			return XCAM_ERROR;
		}

		/* Step 5: send frame data. */
		IMPAudioFrame frm;
		// frm.virAddr = (uint32_t *)buf;
		// frm.len = size;
		ret = IMP_AO_SendFrame(audiostream.playdevID, audiostream.playchnID, &frm, BLOCK);
		if(ret != 0) {
			IMP_LOG_ERR(TAG, "send Frame Data error\n");
			return XCAM_ERROR;
		}

		IMPAudioOChnState play_status;
		ret = IMP_AO_QueryChnStat(audiostream.playdevID, audiostream.playchnID, &play_status);
		if(ret != 0) {
			IMP_LOG_ERR(TAG, "IMP_AO_QueryChnStat error\n");
			return XCAM_ERROR;
		}

		IMP_LOG_INFO(TAG, "Play: TotalNum %d, FreeNum %d, BusyNum %d\n",
				play_status.chnTotalNum, play_status.chnFreeNum, play_status.chnBusyNum);

		// if(++i == 100) {
		// 	ret = IMP_AO_PauseChn(devID, chnID);
		// 	if(ret != 0) {
		// 		IMP_LOG_ERR(TAG, "IMP_AO_PauseChn error\n");
		// 		return XCAM_ERROR;
		// 	}

		// 	printf("[INFO] Test : Audio Play Pause test.\n");
		// 	printf("[INFO]      : Please input any key to continue.\n");
		// 	getchar();

		// 	ret = IMP_AO_ClearChnBuf(devID, chnID);
		// 	if(ret != 0) {
		// 		IMP_LOG_ERR(TAG, "IMP_AO_ClearChnBuf error\n");
		// 		return XCAM_ERROR;
		// 	}

		// 	ret = IMP_AO_ResumeChn(audiostream.playdevID, chnID);
		// 	if(ret != 0) {
		// 		IMP_LOG_ERR(TAG, "IMP_AO_ResumeChn error\n");
		// 		return XCAM_ERROR;
		// 	}
		// }
		return XCAM_SUCCESS;
}

int xcam_destroy_playaudio(void){
	int ret = IMP_AO_FlushChnBuf(audiostream.playdevID, audiostream.playchnID);
	if (ret != 0) {
		IMP_LOG_ERR(TAG, "IMP_AO_FlushChnBuf error\n");
		return XCAM_ERROR;
	}
	/* Step 6: disable the audio channel. */
	ret = IMP_AO_DisableChn(audiostream.playdevID, audiostream.playchnID);
	if(ret != 0) {
		IMP_LOG_ERR(TAG, "Audio channel disable error\n");
		return XCAM_ERROR;
	}

	/* Step 7: disable the audio devices. */
	ret = IMP_AO_Disable(audiostream.playdevID);
	if(ret != 0) {
		IMP_LOG_ERR(TAG, "Audio device disable error\n");
		return XCAM_ERROR;
	}
	return XCAM_SUCCESS;
}

int xcam_config_playaudio_stream(IMPAudioStream *stream){
	/* Step 1: set public attribute of AO device. */
	int devID = 0;
	IMPAudioIOAttr attr;
	attr.samplerate = AUDIO_SAMPLE_RATE_8000;
	attr.bitwidth = AUDIO_BIT_WIDTH_16;
	attr.soundmode = AUDIO_SOUND_MODE_MONO;
	attr.frmNum = 20;
	attr.numPerFrm = 400;
	attr.chnCnt = 1;
	int ret = IMP_AO_SetPubAttr(audiostream.playdevID, &attr);
	if(ret != 0) {
		IMP_LOG_ERR(TAG, "set ao %d attr err: %d\n", audiostream.playdevID, ret);
		return XCAM_ERROR;
	}

	memset(&attr, 0x0, sizeof(attr));
	ret = IMP_AO_GetPubAttr(audiostream.playdevID, &attr);
	if(ret != 0) {
		IMP_LOG_ERR(TAG, "get ao %d attr err: %d\n", audiostream.playdevID, ret);
		return XCAM_ERROR;
	}

	IMP_LOG_INFO(TAG, "Audio Out GetPubAttr samplerate:%d\n", attr.samplerate);
	IMP_LOG_INFO(TAG, "Audio Out GetPubAttr   bitwidth:%d\n", attr.bitwidth);
	IMP_LOG_INFO(TAG, "Audio Out GetPubAttr  soundmode:%d\n", attr.soundmode);
	IMP_LOG_INFO(TAG, "Audio Out GetPubAttr     frmNum:%d\n", attr.frmNum);
	IMP_LOG_INFO(TAG, "Audio Out GetPubAttr  numPerFrm:%d\n", attr.numPerFrm);
	IMP_LOG_INFO(TAG, "Audio Out GetPubAttr     chnCnt:%d\n", attr.chnCnt);

	/* Step 2: enable AO device. */
	ret = IMP_AO_Enable(audiostream.playdevID);
	if(ret != 0) {
		IMP_LOG_ERR(TAG, "enable ao %d err\n", devID);
		return XCAM_ERROR;
	}

	/* Step 3: enable AI channel. */
	ret = IMP_AO_EnableChn(audiostream.playdevID, audiostream.playchnID);
	if(ret != 0) {
		IMP_LOG_ERR(TAG, "Audio play enable channel failed\n");
		return XCAM_ERROR;
	}

	/* Step 4: Set audio channel volume. */
	int chnVol = 60;
	ret = IMP_AO_SetVol(audiostream.playdevID, audiostream.playchnID, chnVol);
	if(ret != 0) {
		IMP_LOG_ERR(TAG, "Audio Play set volume failed\n");
		return XCAM_ERROR;
	}

	ret = IMP_AO_GetVol(audiostream.playdevID, audiostream.playchnID, &chnVol);
	if(ret != 0) {
		IMP_LOG_ERR(TAG, "Audio Play get volume failed\n");
		return XCAM_ERROR;
	}
	IMP_LOG_INFO(TAG, "Audio Out GetVol    vol:%d\n", chnVol);

	int aogain = 28;
	ret = IMP_AO_SetGain(audiostream.playdevID, audiostream.playchnID, aogain);
	if(ret != 0) {
		IMP_LOG_ERR(TAG, "Audio Record Set Gain failed\n");
		return XCAM_ERROR;
	}

	ret = IMP_AO_GetGain(audiostream.playdevID, audiostream.playchnID, &aogain);
	if(ret != 0) {
		IMP_LOG_ERR(TAG, "Audio Record Get Gain failed\n");
		return XCAM_ERROR;
	}
	IMP_LOG_INFO(TAG, "Audio Out GetGain    gain : %d\n", aogain);
	return XCAM_SUCCESS;
}

int xcam_config_decode_stream(){
	int ret = XCAM_ERROR;

	/* My g711u decoder Register. */
	// int handle_g711u = 0;
	IMPAudioDecDecoder my_decoder;
	memset(&my_decoder, 0x0, sizeof(my_decoder));
	sprintf(my_decoder.name, "%s", "MY_G711U");
	my_decoder.openDecoder = NULL;
	my_decoder.decodeFrm = MY_G711U_Decode_Frm;
	my_decoder.getFrmInfo = NULL;
	my_decoder.closeDecoder = NULL;

	ret = IMP_ADEC_RegisterDecoder(&audiostream.handle_g711ud, &my_decoder);
	if(ret != 0) {
		IMP_LOG_ERR(TAG, "IMP_ADEC_RegisterDecoder failed\n");
		return XCAM_ERROR;
	}

	/* audio decoder create channel. */
	// int adChn = 0;
	IMPAudioDecChnAttr attr;
	attr.type = audiostream.handle_g711ud;
	attr.bufSize = 20;
	attr.mode = ADEC_MODE_PACK;
	ret = IMP_ADEC_CreateChn(audiostream.adChn, &attr);
	if(ret != 0) {
		IMP_LOG_ERR(TAG, "imp audio decoder create channel failed\n");
		return XCAM_ERROR;
	}

	ret = IMP_ADEC_ClearChnBuf(audiostream.adChn);
	if(ret != 0) {
		IMP_LOG_ERR(TAG, "IMP_ADEC_ClearChnBuf failed\n");
		return XCAM_ERROR;
	}
	return XCAM_SUCCESS;
}

int xcam_audio_destroy_decoder(){
	int ret = IMP_ADEC_UnRegisterDecoder(&audiostream.handle_g711ud);
	if(ret != 0) {
		IMP_LOG_ERR(TAG, "IMP_ADEC_UnRegisterDecoder failed\n");
		return XCAM_ERROR;
	}

	/* destroy the decoder channel. */
	ret = IMP_ADEC_DestroyChn(audiostream.adChn);
	if(ret != 0) {
		IMP_LOG_ERR(TAG, "imp audio decoder destroy channel failed\n");
		return XCAM_ERROR;
	}
	return XCAM_SUCCESS;
}

// void* xcam_audiostream_thread(void *param){
// 	//check env
// 	IMPAudioStream audiostream;
// 	int ret;
// 	while(1){
// 		ret = xcam_record_audio_stream(&audiostream);
// 		if(ret < 0){
// 			pthread_mutex_unlock(&vmod->stream_mutex);
// 			continue;
// 		}
// 			msg_data = msg_pool_get_free_msg(vmod->msgpool);
// 			if (NULL == msg_data) {
// 				usleep(500*1000);
// 				LOG_ERR(LOG_TAG, "channel%d get free msg faild!\n", channel);
// 				pthread_mutex_unlock(&vmod->stream_mutex);
// 				continue;
// 			}
// 			msg_data->type = MSG_AUDIO;
// 			msg_data->buf = (char *)&audiostream;
// 			xcam_module_process_msg(vmod->xmod->id, MSG_DATA_TO_MSG(msg_data));
// 	}
// 		// if(channel == 0){
// 		// 	ret = xcam_record_audio_stream(&audiostream);
// 		// 	if(ret < 0){
// 		// 		pthread_mutex_unlock(&vmod->stream_mutex);
// 		// 		continue;
// 		// 	}
// 		// 	msg_data = msg_pool_get_free_msg(vmod->msgpool);
// 		// 	if (NULL == msg_data) {
// 		// 		usleep(500*1000);
// 		// 		LOG_ERR(LOG_TAG, "channel%d get free msg faild!\n", channel);
// 		// 		pthread_mutex_unlock(&vmod->stream_mutex);
// 		// 		continue;
// 		// 	}
// 		// 	msg_data->type = MSG_AUDIO;
// 		// 	msg_data->buf = (char *)&audiostream;
// 		// 	xcam_module_process_msg(vmod->xmod->id, MSG_DATA_TO_MSG(msg_data));
// 		// }
// }
int  xcam_audio_stream_init(void)
{
    if (xcam_config_audio_stream() !=0) {
        IMP_LOG_ERR(TAG, "xcam_config_audio_streamerror\n");
        return XCAM_ERROR;
    }
    IMP_LOG_ERR(TAG, "xcam_config_audio_stream  ok\n");

    if (xcam_config_audio_encode() !=0) {
        IMP_LOG_ERR(TAG, "xcam_config_audio_encode\n");
        return XCAM_ERROR;
    }
    IMP_LOG_ERR(TAG, "xcam_config_audio_encode  ok\n");
	return XCAM_SUCCESS;
	// xcam_thread_create("xcam_audio", xcam_audiostream_thread, NULL);
}

