/**
 * @file rtit.c real time 电量,实时电量
 */
#include "rtit.h"
#include "conf.h"
#include "web_err.h"
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
	parse_item(bTou,bQR,wp);
	struct stMeter_Run_data *mtr;
	mtr =GetMeterAll_p();
	char tmp[10];
	wpsend(wp,toStr(tmp,"%g",mtr[0].m_iTOU[0]));
	return 0;
}

static int parse_item(uint8_t *bTou,uint8_t *bQR,webs_t wp)
{
	char * action = websGetVar(wp, T("mtr"), T("null"));
	return 0;
}
