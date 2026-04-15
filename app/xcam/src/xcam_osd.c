#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <imp/imp_framesource.h>
#include <imp/imp_encoder.h>
#include "xcam_log.h"
#include "xcam_thread.h"
#include "bgramapinfo_osd.h"
#include "xcam_osd.h"
#include "xcam_conf_video.h"
#include "xcam_general.h"
#include "xcam_video.h"
#include "xcam_com.h"

#define LOG_TAG "OSD"
#define CHN_TITLE_LEN 20
#define CHN_TITLE_CHN0_POS_X 60
#define CHN_TITLE_CHN0_POS_Y (SENSOR_RESOLUTION_HEIGHT_MAIN - 100)
#define CHN_TITLE_CHN1_POS_X 10
#define CHN_TITLE_CHN1_POS_Y (SENSOR_RESOLUTION_HEIGHT_SLAVE - 80)

extern video_config_info_t stream_attr;
static char* time_token_name[4] = {"timestr0", "timestr1", "", "timestr3"};
static char* picdata_token_name[4] = {"datastr0", "datastr1", "", "datastr3"};
static double _fps[4] = {0.0};
static double _kbr[4] = {0.0};
char  chnTitle[4][CHN_TITLE_LEN]={"title0", "title1", "", "title3"};
static char* title_token_name[4] = {"titlestr0", "titlestr1", "", "titlestr3"};
enum OSD{
	TIMESTAMP,
	FPS,
	TITLE
};
static int osd_index = 0;
static int show_osd[4]={1, 1, 1, 1};
static int titlepos = 0;
#define THREAD_NAME_MAX_LEN (20)
#define THREAD_NUM_MAX (50)
typedef struct thread_info {
    char valid;
    char name[THREAD_NAME_MAX_LEN];
    pthread_t handle_thread;
} thread_info_t;
static int g_title_width = SENSOR_RESOLUTION_WIDTH_MAIN;
static int g_title_height = SENSOR_RESOLUTION_HEIGHT_MAIN;

xcam_osd_t osd_attr;

//初始化链表
void _xcam_osd_link_init()
{
	pthread_mutex_lock(&osd_attr.osd_mutex);
	osd_attr.osd_link_head[0].next = NULL;
	osd_attr.osd_link_head[0].pre = NULL;
	osd_attr.osd_link_head[1].next = NULL;
	osd_attr.osd_link_head[1].pre = NULL;
#ifdef XCAM_DOUBLE_SENSOR
	osd_attr.osd_link_head[2].next = NULL;
	osd_attr.osd_link_head[2].pre = NULL;
	osd_attr.osd_link_head[3].next = NULL;
	osd_attr.osd_link_head[3].pre = NULL;
#endif
	pthread_mutex_unlock(&osd_attr.osd_mutex);

	return;
}

//添加链表节点
int _xcam_osd_link_add_node(int stream_num, xcam_link_t *add_node)
{
	xcam_link_t *temp = NULL;

	pthread_mutex_lock(&osd_attr.osd_mutex);
	temp = &osd_attr.osd_link_head[stream_num];
	while(1) {
		if (temp->next == NULL) {
			temp->next = add_node;
			add_node->pre = temp;
			add_node->next =NULL;
			break;
		}

		temp = temp->next;
	}
	pthread_mutex_unlock(&osd_attr.osd_mutex);

	return XCAM_SUCCESS;
}

//删除链表节点
int _xcam_osd_link_del_node(int stream_num, char* token)
{
	xcam_link_t *temp = NULL;
	osd_rgn_config_t *temptag = NULL;

	pthread_mutex_lock(&osd_attr.osd_mutex);
	temp = osd_attr.osd_link_head[stream_num].next;
	while(1) {
		if (temp ==NULL) {
			LOG_INF(LOG_TAG,"Xcam osd link is NULL.\n");
			pthread_mutex_unlock(&osd_attr.osd_mutex);
			return 0;
		}

		temptag = (osd_rgn_config_t *)temp;
		if (strcmp(token,temptag->osd_rgn_token) == 0) {
			temp->pre->next = temp->next;
			temp->next->pre = temp->pre;
			if(temptag->data != NULL) {
				free(temptag->data);
				temptag->data = NULL;
			}
			free(temptag);
			break;
		}
		temp = temp->next;
	}

	pthread_mutex_unlock(&osd_attr.osd_mutex);
	return XCAM_SUCCESS;
}

//查找链表节点
xcam_link_t* _xcam_osd_link_look_node(int stream_num, char* osd_token)
{
	xcam_link_t *temp = NULL;
	osd_rgn_config_t *temptag = NULL;

	pthread_mutex_lock(&osd_attr.osd_mutex);
	temp = osd_attr.osd_link_head[stream_num].next;
	while(1) {
		if (temp == NULL) {
			LOG_INF(LOG_TAG,"Xcam osd link is NULL.\n");
			break;
		}

		if ((osd_attr.osd_link_head[stream_num].next != NULL) && (temp == NULL)) {
			LOG_INF(LOG_TAG,"The specified node could not be found.\n");
			break;
		}

		temptag = (osd_rgn_config_t *)temp;
		if (strcmp(osd_token,temptag->osd_rgn_token) == 0) {
			pthread_mutex_unlock(&osd_attr.osd_mutex);
			return temp;
		}

		temp = temp->next;
	}

	pthread_mutex_unlock(&osd_attr.osd_mutex);
	return NULL;
}

//申请链表节点空间
osd_rgn_config_t*  _xcam_osd_malloc_node_init_para(int streamnum, int osdhandle, int x0, int y0, int maxlen, char* token, int showflag, char* data, char *text)
{
	assert(token != NULL);

	osd_rgn_config_t *osdnode = NULL;
	osdnode = (osd_rgn_config_t *)malloc(sizeof(osd_rgn_config_t));
	if (osdnode == NULL) {
		LOG_ERR(LOG_TAG,"add osdnode malloc fail.\n");
		return NULL;
	}

	memset(osdnode, 0, sizeof(osd_rgn_config_t));
	memcpy(osdnode->osd_rgn_token, token, XCAM_OSD_TOKEN_MAX_LEN);
	osdnode->streamnum = streamnum;
	osdnode->osdrgnhandle = osdhandle;
	osdnode->maxlen = maxlen;
	osdnode->pos0_x = x0;
	osdnode->pos0_y = y0;
	osdnode->picHeight = stream_attr.stream_config[0].fs_attr.picHeight;
	osdnode->picHeight = stream_attr.stream_config[0].fs_attr.picWidth;
	osdnode->showflag = showflag;
	osdnode->data = data;
	osdnode->type = OSD_REG_PIC;
	if (text != NULL)
		strlcpy(osdnode->text, text, XCAM_OSD_RGN_TEXT_MAX_LEN);

	return osdnode;
}

//释放链表节点空间
void _xcam_osd_free_node_del_info(osd_rgn_config_t* osdnode)
{
	if (osdnode->data != NULL) {
		free(osdnode->data);
	}

	free(osdnode);
}

//叠加字符串需要的bitmap
static char* _xcam_osd_show_text(IMPRgnHandle osdrgnhandle,int maxlen, char *text, int streamnum)
{
	bitmapinfo_t_2 *picmap;
	picmap = gBgramap;
	char *tag = NULL;

	if(osdrgnhandle == INVHANDLE)
		return NULL;

	char content[maxlen+1];
	memset(content, 0, maxlen+1);
	memcpy(content, text, maxlen+1);
	int OSD_STR_HEIGHT = 0;
	int OSD_STR_WIDTH = 0;
	IMPOSDRgnAttrData rAttrData;
	int i, j;
	int penpos_t = 0;
	int fontadv = 0;
	char *data = NULL;
	memset(&rAttrData, 0, sizeof(IMPOSDRgnAttrData));

	if (streamnum) {
		OSD_STR_HEIGHT = 18;
		OSD_STR_WIDTH = 8;
	} else {
		OSD_STR_HEIGHT = 34;
		OSD_STR_WIDTH = 16;
	}

#ifdef SUPPORT_RGB555LE
	data = malloc(maxlen*OSD_STR_HEIGHT*OSD_STR_WIDTH*sizeof(int16_t));
#else
	data = malloc(maxlen*OSD_STR_HEIGHT*OSD_STR_WIDTH*sizeof(int32_t));
#endif

	if (data == NULL) {
		LOG_ERR(LOG_TAG,"Malloc fail.\n");
		return NULL;
	}

	for (i = 0; content[i] != '\0'; i++) {
		switch(content[i]) {
			case '0' ... '9':
				tag = (void *)picmap[content[i]- '0'].pdata;
				fontadv = picmap[content[i] - '0'].width;
				break;
			case '-':
				tag = (void *)picmap[10].pdata;
				fontadv = picmap[10].width;
				break;
			case ' ':
				tag = (void *)picmap[11].pdata;
				fontadv = picmap[11].width;
				break;
			case ':':
				tag = (void *)picmap[12].pdata;
				fontadv = picmap[12].width;
				break;
			case '.':
				tag = (void *)picmap[13].pdata;
				fontadv = picmap[13].width;
				break;
			case 'A' ... 'Z':
				tag = (void *)picmap[content[i]- '3'].pdata;
				fontadv = picmap[content[i] - '3'].width;
				break;
			case 'a' ... 'z':
				tag = (void *)picmap[content[i]- '9'].pdata;
				fontadv = picmap[content[i] - '9'].width;
				break;
			default:break;
		}

#ifdef SUPPORT_RGB555LE
		for (j = 0; j < OSD_STR_HEIGHT; j++) {
			memcpy((void *)((uint16_t *)data + j*OSD_STR_WIDTH*maxlen + penpos_t),
					(void *)((uint16_t *)tag + j*fontadv), fontadv*sizeof(uint16_t));
		}
#else
		for (j = 0; j < OSD_STR_HEIGHT; j++) {
			memcpy((void *)((uint32_t *)data + j*OSD_STR_WIDTH*maxlen + penpos_t),
					(void *)((uint32_t *)tag + j*fontadv), fontadv*sizeof(uint32_t));
		}
#endif
		penpos_t = penpos_t + fontadv;
	}

	rAttrData.picData.pData = data;
	IMP_OSD_UpdateRgnAttrData(osdrgnhandle, &rAttrData);

	return data;
}

//创建cover类型的OSD的Rgn
IMPRgnHandle _xcam_Create_IMP_OSDRgn_Cover(int streamnum,IMPOSDRgnAttr *prAttrCover,int pos_x,int pos_y,int maxlen)
{
	int ret = XCAM_SUCCESS;
	int OSD_STR_WIDTH = 0;
	int OSD_STR_HEIGHT = 0;
	IMPRgnHandle coverhandle = INVHANDLE;

	if (streamnum) {
		OSD_STR_HEIGHT = 18;
		OSD_STR_WIDTH = 8;
	} else {
		OSD_STR_HEIGHT = 34;
		OSD_STR_WIDTH = 16;
	}

	coverhandle = IMP_OSD_CreateRgn(NULL);
	if (coverhandle == INVHANDLE) {
		LOG_ERR(LOG_TAG, "IMP_OSD_CreateRgn Cover error !\n");
		return XCAM_ERROR;
	}

	ret = IMP_OSD_RegisterRgn(coverhandle,streamnum, NULL);
	if (ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG, "IMP_OSD_RegisterRgn failed\n");
		return ret;
	}

	IMPOSDRgnAttr rAttrCover;
	memset(&rAttrCover, 0, sizeof(IMPOSDRgnAttr));
	rAttrCover.type = OSD_REG_COVER;
	rAttrCover.rect.p0.x = pos_x;
	rAttrCover.rect.p0.y = pos_y;
	rAttrCover.rect.p1.x = rAttrCover.rect.p0.x + maxlen * OSD_STR_WIDTH - 1;
	rAttrCover.rect.p1.y = rAttrCover.rect.p0.y + OSD_STR_HEIGHT - 1;
	rAttrCover.fmt = PIX_FMT_BGRA;
	rAttrCover.data.coverData.color = OSD_IPU_BLACK;
	ret = IMP_OSD_SetRgnAttr(coverhandle, &rAttrCover);
	if (ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG, "IMP_OSD_SetRgnAttr Cover error !\n");
		return ret;
	}

	IMPOSDGrpRgnAttr grAttrCover;
	if (IMP_OSD_GetGrpRgnAttr(coverhandle, streamnum, &grAttrCover) < 0) {
		LOG_ERR(LOG_TAG, "IMP_OSD_GetGrpRgnAttr Cover error !\n");
		return XCAM_ERROR;
	}

	memset(&grAttrCover, 0, sizeof(IMPOSDGrpRgnAttr));
	grAttrCover.show = 0;
	grAttrCover.gAlphaEn = 1;
	grAttrCover.fgAlhpa = 0x5f;
	grAttrCover.layer = 2;
	if (IMP_OSD_SetGrpRgnAttr(coverhandle, streamnum, &grAttrCover) < 0) {
		LOG_ERR(LOG_TAG, "IMP_OSD_SetGrpRgnAttr Cover error !\n");
		return XCAM_ERROR;
	}

	ret = IMP_OSD_ShowRgn(coverhandle, streamnum, 1);
	if (ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG, "IMP_OSD_ShowRgn() Cover error\n");
		return ret;
	}

	memcpy(prAttrCover, &rAttrCover, sizeof(IMPOSDRgnAttr));

	return coverhandle;
}

//创建picture类型的OSD的Rgn
int _xcam_osd_create_osdrgn_pic(int streamnum, int pos_x, int pos_y, int maxlen)
{
	int ret = XCAM_SUCCESS;
	int osd_str_width = 0;
	int osd_str_height = 0;

	int pichandle = INVHANDLE;

	if (!streamnum) {
		osd_str_width = 16;
		osd_str_height = 34;
		if (pos_x < 0 || pos_y < 0 || pos_x > (SENSOR_RESOLUTION_WIDTH_MAIN - maxlen * osd_str_width) || pos_y > (SENSOR_RESOLUTION_HEIGHT_MAIN - osd_str_height)) {
			LOG_ERR(LOG_TAG, "osd pos beyond camera max scope\n");
			return XCAM_ERROR;
		}
	} else {
		osd_str_width = 8;
		osd_str_height = 18;
		if (pos_x < 0 || pos_y < 0 || pos_x > (SENSOR_RESOLUTION_WIDTH_SLAVE - maxlen * osd_str_width) || pos_y > (SENSOR_RESOLUTION_HEIGHT_SLAVE - osd_str_height)) {
			LOG_ERR(LOG_TAG, "osd pos beyond camera max scope\n");
			return XCAM_ERROR;
		}
	}

	if(!streamnum) {
		pichandle = IMP_ISP_Tuning_CreateOsdRgn(streamnum, NULL);
		if (pichandle == INVHANDLE) {
			LOG_ERR(LOG_TAG, "IMP_ISP_Tuning_CreateOsdRgn error !\n");
			return XCAM_ERROR;
		}
	} else {
		int grpNum = streamnum;
		pichandle = IMP_OSD_CreateRgn(NULL);
		if (pichandle == INVHANDLE) {
			IMP_LOG_ERR(LOG_TAG, "IMP_OSD_CreateRgn TimeStamp error !\n");
			return XCAM_ERROR;
		}

		//query osd rgn create status
		IMPOSDRgnCreateStat stStatus;
		memset(&stStatus, 0x0, sizeof(IMPOSDRgnCreateStat));
		ret = IMP_OSD_RgnCreate_Query(pichandle, &stStatus);
		if(ret < 0){
			IMP_LOG_ERR(LOG_TAG, "IMP_OSD_RgnCreate_Query error !\n");
			return XCAM_ERROR;
		}

		ret = IMP_OSD_RegisterRgn(pichandle, grpNum, NULL);
		if (ret < 0) {
			IMP_LOG_ERR(LOG_TAG, "IVS IMP_OSD_RegisterRgn failed\n");
			return XCAM_ERROR;
		}

		IMPOSDRgnAttr rAttrFont;
		memset(&rAttrFont, 0, sizeof(IMPOSDRgnAttr));
		rAttrFont.type = OSD_REG_PIC;
		rAttrFont.rect.p0.x = pos_x;
		rAttrFont.rect.p0.y = pos_y;
		rAttrFont.rect.p1.x = rAttrFont.rect.p0.x + maxlen * osd_str_width - 1;   //p0 is start，and p1 well be epual p0+width(or heigth)-1
		rAttrFont.rect.p1.y = rAttrFont.rect.p0.y + osd_str_height - 1;
		rAttrFont.fmt = PIX_FMT_BGRA;
		rAttrFont.data.picData.pData = NULL;

		ret = IMP_OSD_SetRgnAttr(pichandle, &rAttrFont);
		if (ret < 0) {
			IMP_LOG_ERR(LOG_TAG, "IMP_OSD_SetRgnAttr TimeStamp error !\n");
			return XCAM_ERROR;
		}

		IMPOSDGrpRgnAttr grAttrFont;
		if (IMP_OSD_GetGrpRgnAttr(pichandle, grpNum, &grAttrFont) < 0) {
			IMP_LOG_ERR(LOG_TAG, "IMP_OSD_GetGrpRgnAttr Logo error !\n");
			return XCAM_ERROR;
		}

		/* Disable Font global alpha, only use pixel alpha. */
		memset(&grAttrFont, 0, sizeof(IMPOSDGrpRgnAttr));
		grAttrFont.show = 1;
		grAttrFont.gAlphaEn = 1;
		grAttrFont.fgAlhpa = 0xff;
		grAttrFont.layer = 3;
		if (IMP_OSD_SetGrpRgnAttr(pichandle, grpNum, &grAttrFont) < 0) {
			IMP_LOG_ERR(LOG_TAG, "IMP_OSD_SetGrpRgnAttr Logo error !\n");
			return XCAM_ERROR;
		}

		ret = IMP_OSD_Start(grpNum);
		if (ret < 0) {
			IMP_LOG_ERR(LOG_TAG, "IMP_OSD_Start TimeStamp error !\n");
			return XCAM_ERROR;
		}

		//实际上自己show配置的就是grAttrFont.show的值
		ret = IMP_OSD_ShowRgn(pichandle, grpNum, 1);
		if (ret != 0) {
			IMP_LOG_ERR(LOG_TAG, "IMP_OSD_ShowRgn() timeStamp error\n");
			return XCAM_ERROR;
		}
	}

	return pichandle;
}

/*图像分辨率改动后，将osd的绝对位置转换为相对位置*/
void _xcam_osd_rgn_position_change(osd_rgn_config_t *osdrgn,int picWidth,int picHeight)
{
	int pos_x = 0;
	int pos_y = 0;

	pthread_mutex_lock(&osd_attr.osd_mutex);
	pos_x = osdrgn->pos0_x*picWidth/osdrgn->picWidth;
	pos_y = osdrgn->pos0_y*picHeight/osdrgn->picHeight;

	osdrgn->pos0_x = pos_x;
	osdrgn->pos0_y = pos_y;
	osdrgn->picHeight = picHeight;
	osdrgn->picWidth = picWidth;
	pthread_mutex_unlock(&osd_attr.osd_mutex);
	return ;
}

int _xcam_osd_restart_text(osd_rgn_config_t* osdrgn)
{
	IMPRgnHandle texthandle = INVHANDLE;
	IMPOSDRgnAttr rAttrStr;
	memset(&rAttrStr, 0, sizeof(IMPOSDRgnAttr));
	char* data = NULL;

	/*释放之前申请的空间*/
	if (osdrgn->data != NULL) {
		free(data);
	}

	texthandle = _xcam_osd_create_osdrgn_pic(osdrgn->streamnum, osdrgn->pos0_x, osdrgn->pos0_y, osdrgn->maxlen);
	if (texthandle < XCAM_SUCCESS) {
		return XCAM_ERROR;
	}

	data = _xcam_osd_show_text(texthandle, osdrgn->maxlen, osdrgn->text, osdrgn->streamnum);
	if (data == NULL) {
		LOG_ERR(LOG_TAG,"Call _xcam_osd_show_text fail.\n");
		return XCAM_ERROR;
	}

	osdrgn->osdrgnhandle = texthandle;
	osdrgn->showflag = 1;

	return XCAM_SUCCESS;
}

int _xcam_osd_restart_fps_bps_stamp(osd_rgn_config_t* osdrgn, int picWidth, int picHeight)
{
	int streamnum = osdrgn->streamnum;
	int pos_x = osdrgn->pos0_x;
	int pos_y = osdrgn->pos0_y;
	int maxlen = osdrgn->maxlen;
	int osd_str_height = 0;
	int osd_str_width = 0;
	IMPOSDRgnAttr rAttrStr;
	IMPRgnHandle osdrgnhandle = INVHANDLE;

	memset(&rAttrStr, 0, sizeof(IMPOSDRgnAttr));

	if (!streamnum) {
		osd_str_height = 34;
		osd_str_width = 16;
		if (pos_x < 0 || pos_y < 0 || pos_x > (picWidth - maxlen * osd_str_width) || pos_y > (picHeight - osd_str_height)) {
			LOG_ERR(LOG_TAG, "Osd position beyond camera max scope.\n");
			return XCAM_ERROR;
		}
	} else {
		osd_str_height = 18;
		osd_str_width = 8;
		if (pos_x < 0 || pos_y < 0 || pos_x > (picWidth - maxlen * osd_str_width) || pos_y > (picHeight - osd_str_height)) {
			LOG_ERR(LOG_TAG, "Osd position beyond camera max scope.\n");
			return XCAM_ERROR;
		}
	}

	osdrgnhandle = _xcam_osd_create_osdrgn_pic(streamnum, pos_x, pos_y, maxlen);
	if (osdrgnhandle == INVHANDLE) {
		LOG_ERR(LOG_TAG,"Call _xcam_osd_create_osdrgn_pic fail.\n");
		return XCAM_ERROR;
	}

	osdrgn->osdrgnhandle = osdrgnhandle;
	osdrgn->showflag = 1;
	osdrgn->pos0_x = pos_x;
	osdrgn->pos0_y = pos_y;
	osdrgn->picWidth = picWidth;
	osdrgn->picHeight = picHeight;

	return XCAM_SUCCESS;
}

int _xcam_osd_restart_timestamp(osd_rgn_config_t* osdrgn,int picWidth, int picHeight)
{
	int streamnum = osdrgn->streamnum;
	int pos_x = osdrgn->pos0_x;
	int pos_y = osdrgn->pos0_y;
	int maxlen = osdrgn->maxlen;
	int OSD_STR_HEIGHT = 0;
	int OSD_STR_WIDTH = 0;
	IMPOSDRgnAttr rAttrStr;
	IMPRgnHandle osdrgnhandle = INVHANDLE;

	memset(&rAttrStr, 0, sizeof(IMPOSDRgnAttr));

	if (!streamnum) {
		OSD_STR_HEIGHT = 34;
		OSD_STR_WIDTH = 16;
		if (pos_x < 0 || pos_y < 0 || pos_x > (picWidth - maxlen*OSD_STR_WIDTH) || pos_y > (picHeight - OSD_STR_HEIGHT)) {
			LOG_ERR(LOG_TAG, "Osd position beyond camera max scope.\n");
			return XCAM_ERROR;
		}
	} else {
		OSD_STR_HEIGHT = 18;
		OSD_STR_WIDTH = 8;
		if (pos_x < 0 || pos_y < 0 || pos_x > (picWidth - maxlen*OSD_STR_WIDTH) || pos_y > (picHeight - OSD_STR_HEIGHT)) {
			LOG_ERR(LOG_TAG, "Osd position beyond camera max scope.\n");
			return XCAM_ERROR;
		}
	}

	osdrgnhandle = _xcam_osd_create_osdrgn_pic(streamnum, pos_x, pos_y, maxlen);
	if (osdrgnhandle == INVHANDLE) {
		LOG_ERR(LOG_TAG,"Call _xcam_osd_create_osdrgn_pic fail.\n");
		return XCAM_ERROR;
	}

	osdrgn->osdrgnhandle = osdrgnhandle;
	osdrgn->showflag = 1;
	osdrgn->picWidth = picWidth;
	osdrgn->picHeight = picHeight;

	return XCAM_SUCCESS;
}

int _xcam_osd_restart_cover(osd_rgn_config_t *osdrgn)
{
	IMPOSDRgnAttr rAttrCover;
	IMPRgnHandle osdrgnhandle = INVHANDLE;
	memset(&rAttrCover, 0, sizeof(IMPOSDRgnAttr));

	osdrgnhandle = _xcam_Create_IMP_OSDRgn_Cover(osdrgn->streamnum, &rAttrCover, osdrgn->pos0_x, osdrgn->pos0_y, osdrgn->maxlen);
	if (osdrgnhandle ==  INVHANDLE) {
		LOG_ERR(LOG_TAG,"Create cover OSDRGN fail.\n");
		return XCAM_ERROR;
	}

	osdrgn->osdrgnhandle = osdrgnhandle;
	osdrgn->showflag = 1;

	return XCAM_SUCCESS;
}
void xcam_osd_update_stream_info(int streamnum, int picWidth, int picHeight){
	g_title_width  = picWidth;
	g_title_height = picHeight;
}

//重启OSD,恢复链表中所有的osd的配置
void xcam_osd_all_osdrgn_restart(int streamnum, int picWidth, int picHeight)
{
	int ret = XCAM_SUCCESS;
	osd_rgn_config_t *osdtemp = NULL;
	xcam_link_t* nodetemp = osd_attr.osd_link_head[streamnum].next;

	while (nodetemp != NULL) {
		osdtemp = (osd_rgn_config_t*)nodetemp;
		if(osdtemp->type == OSD_REG_COVER){
			_xcam_osd_rgn_position_change(osdtemp, picWidth, picHeight);
			ret = _xcam_osd_restart_cover(osdtemp);
			if (ret < XCAM_SUCCESS) {
				LOG_ERR(LOG_TAG,"restart osd cover fail.\n");
				break;
			}
		} else if (osdtemp->type == OSD_REG_PIC){
			if (strcmp(osdtemp->osd_rgn_token,time_token_name[streamnum]) == 0) {
				ret = _xcam_osd_restart_timestamp(osdtemp, picWidth, picHeight);
				if (ret < XCAM_SUCCESS) {
					LOG_ERR(LOG_TAG,"restart osd timetemp fail.\n");
					break;
				}
			} else if (strcmp(osdtemp->osd_rgn_token,picdata_token_name[streamnum]) == 0) {
				ret = _xcam_osd_restart_fps_bps_stamp(osdtemp, picWidth, picHeight);
				if (ret < XCAM_SUCCESS) {
					LOG_ERR(LOG_TAG,"restart osd fpsbpsstamp fail.\n");
					break;
				}
			} else {
				_xcam_osd_rgn_position_change(osdtemp, picWidth, picHeight);
				ret = _xcam_osd_restart_text(osdtemp);
				if (ret < XCAM_SUCCESS) {
					LOG_ERR(LOG_TAG,"restart osd textstamp  fail.\n");
					break;
				}
			}
		} else {
			LOG_ERR(LOG_TAG,"this osd type error.\n");
			return;
		}

		nodetemp = nodetemp->next;
	}

	return;
}

//停止并销毁指定流通道的所有OSD
int xcam_osd_all_osdrgn_destroy(int iFsChnNum)
{
	int ret = XCAM_SUCCESS;
	osd_rgn_config_t *osdtemp = NULL;
	xcam_link_t* nodetemp = osd_attr.osd_link_head[iFsChnNum].next;

	pthread_mutex_lock(&osd_attr.osd_mutex);
	while (nodetemp != NULL) {
		osdtemp = (osd_rgn_config_t*)nodetemp;
		LOG_ERR(LOG_TAG, "IMP_OSD_ShowRgn close %d error\n", osdtemp->osdrgnhandle);
		if(iFsChnNum) {
			if(osdtemp->osdrgnhandle != -1) {
				/*注销osdrgn*/
				ret = IMP_OSD_ShowRgn(osdtemp->osdrgnhandle, iFsChnNum, 0);
				if (ret < XCAM_SUCCESS) {
					LOG_ERR(LOG_TAG, "IMP_OSD_ShowRgn close %d error\n", iFsChnNum);
					return ret;
				}

				IMP_OSD_UnRegisterRgn(osdtemp->osdrgnhandle, iFsChnNum);
				if (ret < XCAM_SUCCESS) {
					LOG_ERR(LOG_TAG, "IMP_OSD_UnRegisterRgn %d error\n", iFsChnNum);
					return ret;
				}

				IMP_OSD_DestroyRgn(osdtemp->osdrgnhandle);

				osdtemp->showflag = 0;
				osdtemp->osdrgnhandle = -1;
				nodetemp = nodetemp->next;
				LOG_ERR(LOG_TAG, "IMP_OSD_ShowRgn close %x error\n", nodetemp);
			}
		} else {
			if(osdtemp->osdrgnhandle != -1){
				ret = IMP_ISP_Tuning_ShowOsdRgn(iFsChnNum, osdtemp->osdrgnhandle, 0);
				if (ret < XCAM_SUCCESS) {
					LOG_ERR(LOG_TAG, "IMP_OSD_ShowRgn close rHandleStr error\n");
					return ret;
				}
				ret = IMP_ISP_Tuning_DestroyOsdRgn(iFsChnNum, osdtemp->osdrgnhandle);
				if (ret < XCAM_SUCCESS) {
					LOG_ERR(LOG_TAG, "IMP_ISP_Tuning_DestroyOsdRgn error\n");
					return ret;
				}

				osdtemp->showflag = 0;
				osdtemp->osdrgnhandle = -1;
				nodetemp = nodetemp->next;
			}
		}
	}
	pthread_mutex_unlock(&osd_attr.osd_mutex);

	return XCAM_SUCCESS;
}

int  xcam_osd_str_create(int pos_x, int pos_y, int type, int maxlen, int streamnum, char* token)
{
	assert(token != NULL);

	int ret = XCAM_SUCCESS;
	int osd_str_width = 0;
	int osd_str_height = 0;

	int osdrgnhandle = INVHANDLE;

	// char tokenTemp[XCAM_OSD_TOKEN_MAX_LEN] = {0}; for warning
	osd_rgn_config_t *osdnode = NULL;
	char *data = NULL;

	if (!streamnum) {
		osd_str_width = 16;
		osd_str_height = 34;
		if (pos_x < 0 || pos_y < 0 || pos_x > (SENSOR_RESOLUTION_WIDTH_MAIN - maxlen * osd_str_width) || pos_y > (SENSOR_RESOLUTION_HEIGHT_MAIN - osd_str_height)) {
			LOG_ERR(LOG_TAG, "Osd position beyond camera max scope.\n");
			return XCAM_ERROR;
		}
	} else if (streamnum == 3) {
		osd_str_width = 16;
		osd_str_height = 34;
		if (pos_x < 0 || pos_y < 0 || pos_x > (SENSOR_RESOLUTION_WIDTH_MAIN - maxlen * osd_str_width) || pos_y > (SENSOR_RESOLUTION_HEIGHT_MAIN - osd_str_height)) {
			LOG_ERR(LOG_TAG, "Osd position beyond camera max scope.\n");
			return XCAM_ERROR;
		}
	} else {
		osd_str_width = 8;
		osd_str_height = 18;
		if (pos_x < 0 || pos_y < 0 || pos_x > (SENSOR_RESOLUTION_WIDTH_SLAVE - maxlen * osd_str_width) || pos_y > (SENSOR_RESOLUTION_HEIGHT_SLAVE - osd_str_height)) {
			LOG_ERR(LOG_TAG, "Osd position beyond camera max scope.\n");
			return XCAM_ERROR;
		}
	}

	data = malloc(maxlen * osd_str_width * osd_str_height * sizeof(uint32_t));
	if (data == NULL) {
		LOG_ERR(LOG_TAG,"malloc fail.\n");
		return XCAM_ERROR;
	}

	osdrgnhandle = _xcam_osd_create_osdrgn_pic(streamnum, pos_x, pos_y, maxlen);
	if (osdrgnhandle == INVHANDLE) {
		LOG_ERR(LOG_TAG,"Call _xcam_osd_create_osdrgn_pic fail.\n");
		free(data);
		return XCAM_ERROR;
	}

	// 申请链表节点
	osdnode = _xcam_osd_malloc_node_init_para(streamnum, osdrgnhandle, pos_x, pos_y, maxlen, token, 1, data, NULL);
	if (osdnode == NULL) {
		LOG_ERR(LOG_TAG,"Malloc mode and set info fial.\n");
		free(data);
		return XCAM_ERROR;
	}

	// 将节点加入链表
	ret = _xcam_osd_link_add_node(streamnum, &osdnode->node);
	if (ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG,"Call _xcam_osd_link_add_node fail\n");
		_xcam_osd_free_node_del_info(osdnode);
		return XCAM_ERROR;
	}
	return XCAM_SUCCESS;
}

//创建OSD的Rgn
static int xcam_osd_create_text_process(int streamnum, char *text, int pos_x, int pos_y, int maxlen, char *osdtoken)
{
	int ret = XCAM_SUCCESS;
	IMPRgnHandle texthandle = INVHANDLE;
	IMPOSDRgnAttr rAttrStr;
	osd_rgn_config_t *osdnode = NULL;
	char token[XCAM_OSD_TOKEN_MAX_LEN] = {0};
	char *data = NULL;

	memset(&rAttrStr, 0, sizeof(IMPOSDRgnAttr));

	texthandle = _xcam_osd_create_osdrgn_pic(streamnum, pos_x, pos_y, maxlen);
	if (texthandle < XCAM_SUCCESS) {
		return XCAM_ERROR;
	}

	data = _xcam_osd_show_text(texthandle, maxlen, text, streamnum);
	if (data == NULL) {
		LOG_ERR(LOG_TAG,"Call _xcam_osd_show_text fail.\n");
		return XCAM_ERROR;
	}

	sprintf(token, "osdstream%d_%d",streamnum,texthandle);
	osdnode = _xcam_osd_malloc_node_init_para(streamnum, texthandle, 0, 0, maxlen, token, 1, data, text);
	if (osdnode == NULL) {
		LOG_ERR(LOG_TAG,"Mallloc mode and set info fial.\n");
		return XCAM_ERROR;
	}

	ret = _xcam_osd_link_add_node(streamnum,&osdnode->node);
	if (ret <XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG,"Call xcam_OSD_lint_add_nodefail.\n");
		_xcam_osd_free_node_del_info(osdnode);
		return XCAM_ERROR;
	}

	/*将token返回给调用函数*/
	strlcpy(osdtoken, osdnode->osd_rgn_token, XCAM_OSD_TOKEN_MAX_LEN);

	return XCAM_SUCCESS;
}

//获取某一路码流的OSD的Rgn的个数
int xcam_osd_get_osdrgns_num(int streamnum)
{
	int osdnumber = 0;
	xcam_link_t *node_tag = NULL;

	node_tag = osd_attr.osd_link_head[streamnum].next;

	while (node_tag != NULL) {
		osdnumber++;
		node_tag = node_tag->next;
	}

	return osdnumber;
}

//删除指定的OSD的Rgn
int xcam_osd_delete_osdrgn(int streamnum, char* token)
{
	assert((streamnum == 0) || (streamnum == 1));
	if((token == NULL)|| (strlen(token) >= XCAM_OSD_RGN_TEXT_MAX_LEN)) {
		LOG_ERR(LOG_TAG,"Invalid parameter.\n");
		return XCAM_ERROR;
	}

	int ret = XCAM_SUCCESS;
	xcam_link_t *linknode = NULL;
	osd_rgn_config_t *osdnode = NULL;

	linknode = _xcam_osd_link_look_node(streamnum, token);
	if (linknode == NULL) {
		LOG_INF(LOG_TAG,"The node was not found.\n");
		return XCAM_ERROR;
	}

	osdnode = (osd_rgn_config_t *)linknode;

	ret = IMP_OSD_ShowRgn(osdnode->osdrgnhandle, streamnum, 0);
	if (ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG, "IMP_OSD_ShowRgn close rHandleStr error\n");
		return ret;
	}

	ret = IMP_OSD_UnRegisterRgn(osdnode->osdrgnhandle, streamnum);
	if (ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG, "IMP_OSD_UnRegisterRgn Str error\n");
		return ret;
	}

	IMP_OSD_DestroyRgn(osdnode->osdrgnhandle);

	ret = _xcam_osd_link_del_node(streamnum, token);
	if (ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG,"delete node fail.\n");
		return ret;
	}

	return ret;
}

int xcam_osd_remove_show(int streamnum, char* token)
{
	assert((streamnum == 0) || (streamnum == 1));
	if((token == NULL)|| (strlen(token) >= XCAM_OSD_RGN_TEXT_MAX_LEN)) {
		LOG_ERR(LOG_TAG,"Invalid parameter.\n");
		return XCAM_ERROR;
	}

	int ret = XCAM_SUCCESS;
	xcam_link_t *linknode = NULL;
	osd_rgn_config_t *osdnode = NULL;

	linknode = _xcam_osd_link_look_node(streamnum, token);
	if (linknode == NULL) {
		LOG_INF(LOG_TAG,"The node was not found.\n");
		return XCAM_ERROR;
	}

	osdnode = (osd_rgn_config_t *)linknode;

	ret = IMP_OSD_ShowRgn(osdnode->osdrgnhandle, streamnum, 0);
	if (ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG, "IMP_OSD_ShowRgn close rHandleStr error\n");
		return ret;
	}

	osdnode->showflag = 0;

	return XCAM_SUCCESS;
}

int xcam_osd_get_max_effective_range(int streamnum, int* height, int* width)
{
	if(streamnum < 0 || streamnum > 0 || height == NULL || width == NULL) {
		return XCAM_ERROR;
	}

	*height = stream_attr.stream_config[streamnum].fs_attr.picHeight;
	*width = stream_attr.stream_config[streamnum].fs_attr.picWidth;

	return XCAM_SUCCESS;
}

//设置OSD区域属性
int xcam_osd_set_text(int streamnum, char* osdtoken, int x, int y, char *text)
{
	int ret = 0;
	int pos_x = 0;
	int pos_y = 0;
	xcam_link_t *tag_lint = NULL;
	IMPOSDRgnAttr rAttrStr;
	int maxlen =0;
	char* tag = NULL;
	int OSD_STR_WIDTH = 0;
	int OSD_STR_HEIGHT = 0;
	osd_rgn_config_t *node = NULL;

	memset(&rAttrStr, 0,sizeof(IMPOSDRgnAttr));
	pos_x = x;
	pos_y = y;

	if ( streamnum ) {
		OSD_STR_HEIGHT = 18;
		OSD_STR_WIDTH = 8;
	} else {
		OSD_STR_HEIGHT = 34;
		OSD_STR_WIDTH = 16;
	}

	tag = text;
	while ( tag != NULL ) {
		if(*tag == '\0')
			break;
		maxlen++;
		tag++;
	}

	tag_lint = _xcam_osd_link_look_node(streamnum, osdtoken);
	if ( tag_lint == NULL ) {
		return XCAM_ERROR;
	}
	node = (osd_rgn_config_t *)tag_lint;

	ret = IMP_OSD_GetRgnAttr(node->osdrgnhandle, &rAttrStr);
	if ( ret < 0 ) {
		LOG_ERR(LOG_TAG,"Call IMP_OSD_GetGrpRgnAttr fail.\n");
		return ret;
	}

	rAttrStr.rect.p0.x = pos_x;
	rAttrStr.rect.p0.y = pos_y;
	rAttrStr.rect.p1.x = rAttrStr.rect.p0.x + 19 * OSD_STR_WIDTH - 1;
	rAttrStr.rect.p1.y = rAttrStr.rect.p0.y + OSD_STR_HEIGHT - 1;

	ret = IMP_OSD_SetRgnAttr(node->osdrgnhandle, &rAttrStr);
	if ( ret < 0 ) {
		LOG_ERR(LOG_TAG,"IMP_OSD_SetRgnAttr Str error.\n");
		return ret;
	}

	tag = NULL;
//	tag = xcam_show_OsdRgn_text(node->osdrgnhandle, maxlen, text,streamnum);
	if ( tag !=NULL ) {
		return XCAM_ERROR;
	}

	return ret;
}

//新建OSD的Rgn
int xcam_osd_create_text(int stream_num, int x, int y, char* text, char *osdtoken)
{
	int ret = XCAM_SUCCESS;
	int pos_x = 0;
	int pos_y = 0;
	int maxlen = 0;
	char *tag =NULL;

	if ((text == NULL) || (osdtoken == NULL)) {
		LOG_ERR(LOG_TAG, "xcam_OSD_create_OsdRgn parameter is error.\n");
		return XCAM_ERROR;
	}

	if (strlen(text) >= XCAM_OSD_RGN_TEXT_MAX_LEN) {
		LOG_INF(LOG_TAG, "text too length.\n");
		return XCAM_ERROR;
	}

	pos_x = x;
	pos_y = y;

	tag = text;
	while (tag != NULL) {
		if(*tag == '\0')
			break;
		maxlen++;
		tag++;
	}

	ret = xcam_osd_create_text_process(stream_num, text, pos_x, pos_y, maxlen, osdtoken);
	if (ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG,"Call xcam_osd_create_text_process fail.\n");
		return ret;
	}

	return ret;
}

int xcam_osd_create_timestamp(int stream_num, int x, int y, int timetype, char *osdtoken)
{
	assert((stream_num == 0) || (stream_num == 1));
	assert(osdtoken != NULL);
	int ret = XCAM_SUCCESS;
	int type = 0;
	int maxlen [2] = {22,16};
	xcam_link_t* node = NULL;

	node = _xcam_osd_link_look_node(stream_num, time_token_name[stream_num]);
	if (node != NULL) {
		LOG_INF(LOG_TAG,"info(%s,%d), timetamp already create.\n",__func__,__LINE__);
		return XCAM_SUCCESS;
	}

	ret = xcam_osd_str_create(x, y, type, maxlen[stream_num], stream_num, time_token_name[stream_num]);
    if (ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG, "create timestamp error\n");
		return ret;
	}

	strlcpy(osdtoken, time_token_name[stream_num], XCAM_OSD_TOKEN_MAX_LEN);
	return ret;
}

int xcam_osd_get_timestamp(int streamnum, int* x, int* y, char* osdtoken)
{
	if ((x == NULL) || (y == NULL) || (osdtoken == NULL) || (streamnum < 0) || (streamnum > 1)) {
		return XCAM_ERROR;
	}

	osd_rgn_config_t *osdnode = NULL;
	xcam_link_t* node = NULL;

	node = _xcam_osd_link_look_node(streamnum, time_token_name[streamnum]);
	if (node == NULL) {
		LOG_INF(LOG_TAG,"info(%s,%d), timetamp don't create.\n",__func__,__LINE__);
		return XCAM_ERROR;
	}

	osdnode = (osd_rgn_config_t *)node;
	*x = osdnode->pos0_x;
	*y = osdnode->pos0_y;
	strlcpy(osdtoken, osdnode->osd_rgn_token, XCAM_OSD_TOKEN_MAX_LEN);

	return XCAM_SUCCESS;
}

int xcam_osd_create_fps_bps_stamp(int stream_num, int x, int y, int timetype, char *osdtoken)
{
	assert((stream_num == 0) || (stream_num == 1));
	assert(osdtoken != NULL);
	int ret = XCAM_SUCCESS;
	int type = 0;
	int maxlen [2] = {22,16};
	xcam_link_t* node = NULL;

	node = _xcam_osd_link_look_node(stream_num, picdata_token_name[stream_num]);
	if (node != NULL) {
		LOG_INF(LOG_TAG,"info(%s,%d), timetamp already create.\n",__func__,__LINE__);
		return XCAM_SUCCESS;
	}

	ret = xcam_osd_str_create(x, y, type, maxlen[stream_num], stream_num, picdata_token_name[stream_num]);
    if (ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG, "create fps error.\n");
		return ret;
	}

	/*将token传送给调用函数*/
	strlcpy(osdtoken, time_token_name[stream_num], XCAM_OSD_TOKEN_MAX_LEN);

	return ret;
}

//更新时间的bitmap
int _xcam_osd_str_update(xcam_link_t* node, char *str, int streamnum)
{
	int ret = XCAM_SUCCESS;
	int osd_str_width;
	int osd_str_height;
	bitmapinfo_t_2 *picmap;
	IMPOSDRgnAttrData rAttrData;
	if (!streamnum) {
		osd_str_width = 16;
		osd_str_height = 34;
		picmap = gBgramap;
	} else {
		osd_str_width = 8;
		osd_str_height = 18;
		picmap = gBgramap_sec;
	}

	osd_rgn_config_t *osd_str_info = (osd_rgn_config_t *)node;
	if (strlen(str) > osd_str_info->maxlen) {
		LOG_ERR(LOG_TAG, "Str length larger than maxlen.\n");
		return XCAM_ERROR;
	}

	int i, j;
	int penpos_t = 0;
	int fontadv = 0;
	void *dateData = NULL;
	char DateStr[osd_str_info->maxlen];
	sprintf(DateStr, str);
	int len = strlen(DateStr);
	for (i = 0; i < len; i++) {
		switch(DateStr[i]) {
			case '0' ... '9':
				dateData = (void *)picmap[DateStr[i]- '0'].pdata;
				fontadv = picmap[DateStr[i] - '0'].width;
				penpos_t += picmap[DateStr[i] - '0'].width;
				break;
			case '-':
				dateData = (void *)picmap[10].pdata;
				fontadv = picmap[10].width;
				penpos_t += picmap[10].width;
				break;
			case ' ':
				dateData = (void *)picmap[11].pdata;
				fontadv = picmap[11].width;
				penpos_t += picmap[11].width;
				break;
			case ':':
				dateData = (void *)picmap[12].pdata;
				fontadv = picmap[12].width;
				penpos_t += picmap[12].width;
				break;
			case '.':
				dateData = (void *)picmap[13].pdata;
				fontadv = picmap[13].width;
				penpos_t += picmap[13].width;
				break;
			case 'A' ... 'Z':
				dateData = (void *)picmap[DateStr[i]- '3'].pdata;
				fontadv = picmap[DateStr[i] - '3'].width;
				penpos_t += picmap[DateStr[i] - '3'].width;
				break;
			case 'a' ... 'z':
				dateData = (void *)picmap[DateStr[i]- '9'].pdata;
				fontadv = picmap[DateStr[i] - '9'].width;
				penpos_t += picmap[DateStr[i] - '9'].width;
				break;
			default:
				break;
		}
		for (j = 0; j < osd_str_height; j++) {
			memcpy((void *)((uint32_t *)osd_str_info->data + j * osd_str_width * osd_str_info->maxlen + penpos_t - picmap[0].width),
					(void *)((uint32_t *)dateData + j*fontadv), fontadv*sizeof(uint32_t));
		}
	}

	if(!streamnum) {
		IMPIspOsdAttrAsm stISPOSDAsm;
		stISPOSDAsm.type = ISP_OSD_REG_PIC;
		stISPOSDAsm.stsinglepicAttr.chnOSDAttr.osd_type = IMP_ISP_PIC_ARGB_8888;
		stISPOSDAsm.stsinglepicAttr.chnOSDAttr.osd_argb_type = IMP_ISP_ARGB_TYPE_BGRA;
		stISPOSDAsm.stsinglepicAttr.chnOSDAttr.osd_pixel_alpha_disable = IMPISP_TUNING_OPS_MODE_DISABLE;
		stISPOSDAsm.stsinglepicAttr.pic.pinum =  osd_str_info->osdrgnhandle;
		stISPOSDAsm.stsinglepicAttr.pic.osd_enable = 1;
		if(osd_index == TITLE){
			if(titlepos == 0){
				stISPOSDAsm.stsinglepicAttr.pic.osd_left = osd_str_info->pos0_x;
				stISPOSDAsm.stsinglepicAttr.pic.osd_top = g_title_height - 50;
			} else if(titlepos == 1){
				stISPOSDAsm.stsinglepicAttr.pic.osd_left = g_title_width > 350 ? g_title_width  -350 : g_title_width;
				stISPOSDAsm.stsinglepicAttr.pic.osd_top = 60;
			} else{
				stISPOSDAsm.stsinglepicAttr.pic.osd_left = g_title_width > 350 ? g_title_width  -350 : g_title_width;
				stISPOSDAsm.stsinglepicAttr.pic.osd_top = g_title_height - 50;
			}
		} else {
			stISPOSDAsm.stsinglepicAttr.pic.osd_left = osd_str_info->pos0_x;
			stISPOSDAsm.stsinglepicAttr.pic.osd_top = osd_str_info->pos0_y;
		}
		stISPOSDAsm.stsinglepicAttr.pic.osd_width =  osd_str_width * osd_str_info->maxlen;
		stISPOSDAsm.stsinglepicAttr.pic.osd_height = osd_str_height;
		stISPOSDAsm.stsinglepicAttr.pic.osd_image = (char*)osd_str_info->data;
		stISPOSDAsm.stsinglepicAttr.pic.osd_stride = osd_str_width * osd_str_info->maxlen * 4;

		ret = IMP_ISP_Tuning_SetOsdRgnAttr(0, osd_str_info->osdrgnhandle, &stISPOSDAsm);
		if(ret < 0) {
			IMP_LOG_ERR(LOG_TAG,"IMP_ISP_SetOSDAttr error\n");
			return XCAM_ERROR;
		}

		ret = IMP_ISP_Tuning_ShowOsdRgn(0, osd_str_info->osdrgnhandle, show_osd[osd_index]);
		if(ret < 0) {
			IMP_LOG_ERR(LOG_TAG,"IMP_OSD_ShowRgn_ISP error\n");
			return XCAM_ERROR;
		}
	} else {
		IMP_OSD_ShowRgn(osd_str_info->osdrgnhandle, streamnum, show_osd[osd_index]);
		if(osd_index == TITLE){
			IMPOSDRgnAttr rAttrFont;
			IMP_OSD_GetRgnAttr(osd_str_info->osdrgnhandle, &rAttrFont);
			if(titlepos == 0){
					rAttrFont.rect.p0.x = 10;
					rAttrFont.rect.p0.y = SENSOR_RESOLUTION_HEIGHT_SLAVE - 35;
			} else if(titlepos == 1){
					rAttrFont.rect.p0.x = SENSOR_RESOLUTION_WIDTH_SLAVE - 160;
					rAttrFont.rect.p0.y = 10;
			} else{
					rAttrFont.rect.p0.x = SENSOR_RESOLUTION_WIDTH_SLAVE - 160;
					rAttrFont.rect.p0.y = SENSOR_RESOLUTION_HEIGHT_SLAVE - 35;
			}
			rAttrFont.rect.p1.x = rAttrFont.rect.p0.x + CHN_TITLE_LEN * osd_str_width - 1;   //p0 is start，and p1 well be epual p0+width(or heigth)-1
			rAttrFont.rect.p1.y = rAttrFont.rect.p0.y + osd_str_height - 1;
			ret = IMP_OSD_SetRgnAttr(osd_str_info->osdrgnhandle, &rAttrFont);
			if (ret < 0) {
				IMP_LOG_ERR(LOG_TAG, "IMP_OSD_SetRgnAttr TimeStamp error !\n");
				return XCAM_ERROR;
			}
		}
		rAttrData.picData.pData = osd_str_info->data;
		ret = IMP_OSD_UpdateRgnAttrData(osd_str_info->osdrgnhandle, &rAttrData);
		if(ret < 0) {
			IMP_LOG_ERR(LOG_TAG,"IMP_OSD_UpdateRgnAttrData error\n");
			return XCAM_ERROR;
		}
	}

	return XCAM_SUCCESS;
}

//时间戳更新处理函数
int _xcam_osd_time_update()
{
	int ret = XCAM_SUCCESS;
	time_t currTime;
	struct tm *currDate;
	int maxlen = 19;
	char timestr[maxlen + 1];
	xcam_link_t* node = NULL;
	osd_rgn_config_t *rgntemp = NULL;

	time(&currTime);
	currDate = localtime(&currTime);
	memset(timestr, 0, maxlen + 1);
	strftime(timestr, maxlen + 1, "%Y-%m-%d %I:%M:%S", currDate);

	node = _xcam_osd_link_look_node(0, time_token_name[0]);
	if (node == NULL) {
		LOG_ERR(LOG_TAG,"info(%s,%d), timetamp0 no create.\n",__func__,__LINE__);
	} else {
		rgntemp = (osd_rgn_config_t *)node;
		if (rgntemp->showflag == 1) {
			osd_index = TIMESTAMP;
			ret = _xcam_osd_str_update(node, timestr, 0);
			if (ret < XCAM_SUCCESS) {
				LOG_ERR(LOG_TAG, "timestamp0 update error\n");
				return XCAM_ERROR;
			}
		}
	}

	node = _xcam_osd_link_look_node(1, time_token_name[1]);
	if (node == NULL) {
		LOG_ERR(LOG_TAG,"info(%s,%d), timetamp1 no create.\n",__func__,__LINE__);
	} else {
		rgntemp = (osd_rgn_config_t *)node;
		if (rgntemp->showflag == 1) {
			osd_index = TIMESTAMP;
			ret = _xcam_osd_str_update(node, timestr, 1);
			if (ret < XCAM_SUCCESS) {
				LOG_ERR(LOG_TAG, "timestamp1 update error\n");
				return XCAM_ERROR;
			}
		}
	}
#ifdef XCAM_DOUBLE_SENSOR
	node = _xcam_osd_link_look_node(3, time_token_name[3]);
	if (node == NULL) {
		LOG_ERR(LOG_TAG,"info(%s,%d), timetamp3 no create.\n",__func__,__LINE__);
	} else {
		rgntemp = (osd_rgn_config_t *)node;
		if (rgntemp->showflag == 1) {
			osd_index = TIMESTAMP;
			ret = _xcam_osd_str_update(node, timestr, 3);
			if (ret < XCAM_SUCCESS) {
				LOG_ERR(LOG_TAG, "timestamp3 update error\n");
				return XCAM_ERROR;
			}
		}
	}
#endif

	return XCAM_SUCCESS;
}

//码率帧率更新处理函数
int _xcam_osd_fps_update(int channel)
{
	int ret = XCAM_SUCCESS;
	xcam_link_t* node = NULL;
	osd_rgn_config_t *rgntemp = NULL;
	int maxlen = 30;
	char fpsstr[maxlen];
	double fps = _fps[channel];
	double kbr = _kbr[channel];
	sprintf(fpsstr, "FPS: %d%d.%d%d Bitrate: %d%d%d%d.%d%d",
		(int)fps / 10, (int)fps % 10,
		(int)(fps*10) % 10, (int)(fps*100) % 10,
		(int)kbr / 1000, ((int)kbr % 1000) / 100,
		((int)kbr % 100) / 10, (int)kbr % 10,
		(int)(kbr*10) % 10, (int)(kbr*100) % 10);

	node = _xcam_osd_link_look_node(channel, picdata_token_name[channel]);
	if (node == NULL) {
		LOG_INF(LOG_TAG,"info(%s,%d), timetamp no create. %d %d\n",__func__,__LINE__, channel, picdata_token_name[channel]);
	} else {
		rgntemp = (osd_rgn_config_t *)node;
		if (rgntemp->showflag == 1) {
			osd_index = FPS;
			ret = _xcam_osd_str_update(node, fpsstr, channel);
			if (ret < XCAM_SUCCESS) {
				LOG_ERR(LOG_TAG, "fpsbpsstamp update error %d\n", channel);
				return XCAM_ERROR;
			}
		}
	}

	return XCAM_SUCCESS;
}

int _xcam_osd_title_update(int channel)
{
	int ret = XCAM_SUCCESS;
	xcam_link_t* node = NULL;
	osd_rgn_config_t *rgntemp = NULL;
	int maxlen = CHN_TITLE_LEN;
	char titlestr[maxlen];
	memset(titlestr, 0 , CHN_TITLE_LEN);
	sprintf(titlestr, "%s", chnTitle[channel] );
	memset(titlestr + strlen(chnTitle[channel]), 32 , CHN_TITLE_LEN - strlen(chnTitle[channel]) - 1);
	node = _xcam_osd_link_look_node(channel, title_token_name[channel]);
	if (node == NULL) {
		LOG_INF(LOG_TAG,"info(%s,%d), timetamp[%d] no create.\n",__func__,__LINE__, channel);
	} else {
		rgntemp = (osd_rgn_config_t *)node;
		if (rgntemp->showflag == 1 ) {
			osd_index = TITLE;
			ret = _xcam_osd_str_update(node, titlestr, channel);
			if (ret < XCAM_SUCCESS) {
				LOG_ERR(LOG_TAG, "titlestr update error %d\n", channel);
				return XCAM_ERROR;
			}
		}
	}
	return XCAM_SUCCESS;
}

//更新时间戳和码率帧率线程
void* xcam_osd_thread(void * param)
{
	xcam_thread_set_name("xcam_osd");

	while (1) {
		_xcam_osd_time_update();
		_xcam_osd_fps_update(0);
		_xcam_osd_fps_update(1);
#ifdef XCAM_DOUBLE_SENSOR
		_xcam_osd_fps_update(3);
#endif
		_xcam_osd_title_update(0);
		_xcam_osd_title_update(1);
		usleep(1000*1000);
	}

	return NULL;
}

int xcam_osd_set_fps_kbps(int ch, double fps, double kbps)
{
	_fps[ch] = fps;
	_kbr[ch] = kbps;
	return XCAM_SUCCESS;
}

int xcam_osd_set_chnTitles(int chn, char *title, int titleLen, int timestamp_s, int fps_s, int title_s, int titlepo)
{
	if(titleLen > CHN_TITLE_LEN){
		return XCAM_ERROR;
	}
	memcpy(chnTitle[chn], title, titleLen);
	show_osd[TIMESTAMP] = timestamp_s;
	show_osd[FPS] = fps_s;
	show_osd[TITLE] = title_s;
	titlepos  = titlepo;
	return XCAM_SUCCESS;
}

//OSD初始化
int xcam_osd_init()
{
	int ret = XCAM_SUCCESS;

	memset(&osd_attr, 0, sizeof(xcam_osd_t));
	pthread_mutex_init(&osd_attr.osd_mutex, NULL);

	// 初始化OSD链表
	_xcam_osd_link_init();

	//	创建主码流时间戳
	int maxlen = 22;
	ret = xcam_osd_str_create(30, 10, 0, maxlen, 0, time_token_name[0]);
	if (ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG, "init stream0 timestamp create error\n");
		return XCAM_ERROR;
	}

	// 创建主码流fps和bitrate
	maxlen = 30;
	ret = xcam_osd_str_create(30, 60, 0, maxlen, 0, picdata_token_name[0]);
	if (ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG, "int stream0 fpsbpsstamp create error.\n");
		return XCAM_ERROR;
	}
#ifdef XCAM_DOUBLE_SENSOR
	ret = xcam_osd_str_create(30, 10, 0, maxlen, 3, time_token_name[3]);
	if (ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG, "init stream3 timestamp create error\n");
		return XCAM_ERROR;
	}

	// 创建主码流fps和bitrate
	maxlen = 30;
	ret = xcam_osd_str_create(30, 60, 0, maxlen, 3, picdata_token_name[3]);
	if (ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG, "int stream3 fpsbpsstamp create error.\n");
		return XCAM_ERROR;
	}
#endif
	maxlen = 22;
	// 创建次码流时间戳
	ret = xcam_osd_str_create(10, 10, 0, maxlen, 1, time_token_name[1]);
	if (ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG, "init stream1 timestamp create error\n");
		return XCAM_ERROR;
	}

	maxlen = 30;
	// 创建次码流fps和bitrate
	ret = xcam_osd_str_create(10, 30, 0, maxlen, 1, picdata_token_name[1]);
	if (ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG, "init stream1 fpsbpsstamp create error\n");
		return XCAM_ERROR;
	}

	maxlen = CHN_TITLE_LEN;
	// 创建主码流通道标题
	ret = xcam_osd_str_create(CHN_TITLE_CHN0_POS_X, CHN_TITLE_CHN0_POS_Y, 0, maxlen, 0, title_token_name[0]);
	if (ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG, "init stream0 chntitle create error\n");
		return XCAM_ERROR;
	}

	// 创建次码流通道标题
	ret = xcam_osd_str_create(CHN_TITLE_CHN1_POS_X, CHN_TITLE_CHN1_POS_Y, 0, maxlen, 1, title_token_name[1]);
	if (ret < XCAM_SUCCESS) {
		LOG_ERR(LOG_TAG, "init stream1 chntitle create error\n");
		return XCAM_ERROR;
	}
	// 创建OSD更新线程
	xcam_thread_create("xcam_osd", xcam_osd_thread, NULL);

	return XCAM_SUCCESS;
}

