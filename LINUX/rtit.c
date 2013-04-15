/**
 * @file rtit.c real time 电量,实时电量
 */
#include "rtit.h"
#include "conf.h"
#include "web_err.h"
#include "wpsend.h"
static int webRece_realtime_tou(webs_t wp);
static int parse_item(uint8_t* bMtr, uint8_t* bTou,uint8_t* bQR,webs_t wp);
///实时电度量
void form_realtime_tou(webs_t wp, char_t *path, char_t *query)
{
	PRINT_FORM_INFO;
	websHeader_pure(wp);
	char * action = websGetVar(wp, T("action"), T("null"));
	if (strcmp(action, "get")==0) {
		webRece_realtime_tou(wp);
	} else {
		web_err_proc(EL);
	}
	websDone(wp, 200);
	return;
}
//实时电量查询
static int webRece_realtime_tou(webs_t wp)
{
	uint8_t bMtr[MAXMETER];
	uint8_t bTou[TOUNUM];
	uint8_t bQR[TOUNUM];
	parse_item(bMtr,bTou,bQR,wp);
	struct stMeter_Run_data *mtr;
	mtr =GetMeterAll_p();
	char tmp[10];
	wpsend(wp,toStr(tmp,"%g",mtr[0].m_iTOU[0]));
	return 0;
}
/**
 * 从wp中解析出选择的表,需要采集的电量,象限无功,0表示不采集,1表示采集
 * @param bMtr 表
 * @param bTou 电量
 * @param bQR 象限无功
 * @param wp
 * @return
 */
static int parse_item(uint8_t* bMtr, uint8_t* bTou,uint8_t* bQR,webs_t wp)
{
	char * mtr = websGetVar(wp, T("mtr"), T("null"));
	char * tou = websGetVar(wp, T("tou"), T("null"));
	char * qr = websGetVar(wp, T("qr"), T("null"));
	return 0;
}
