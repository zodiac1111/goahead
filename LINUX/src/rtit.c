/**
 * @file rtit.c real time 电量,实时电量
 */
#include <string.h>
#include "define.h"
#include "rtit.h"
#include "conf.h"
#include "web_err.h"
#include "wpsend.h"
#include "json.h"
static int webRece_realtime_tou(webs_t wp);
static int check_mtrnum(int mtrnum);
static int check_itemnum(int itemnum);
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
//向一个表(mtr)中添加it对象,指示这个表中所有tou实时电量数据.
static int add_mtr_tou(jsObj *oMtr, struct stMeter_Run_data const mtr, char const *abTou)
{
	int j;
	char tmpstr[128];
	uint8_t iv;
	jsObj aOneDate = jsonNewArray();     //一个电量数据=[电量,有效位...]
	jsObj aTou = jsonNewArray();     //[正有,反有,正无,反无]
	for (j = 0; j<TOUNUM; j++) {     //电量 总尖峰平谷
		if (abTou[j]=='1') {
			jsonAdd(&aOneDate, NULL,
			                toStr(tmpstr, "%g", mtr.m_iTOU[j]));
			iv = (mtr.Flag_TOU&(0x1<<j))>>j;
			jsonAdd(&aOneDate, NULL,
			                toStr(tmpstr, "%u", iv));
			jsonAdd(&aTou, NULL, aOneDate);     //电量
			jsonClear(&aOneDate);
		}
	}
	jsonAdd(oMtr, "tou", aTou);
	//printf(WEBS_DBG"tou:%s\n",*oMtr);
	jsonFree(&aOneDate);
	jsonFree(&aTou);
	return 0;
}
//向一个表(mtr)中添加qr四象限无功电量
static int add_mtr_qr(jsObj *oMtr, struct stMeter_Run_data const mtr, char const *abQr)
{
	int j;
	char tmpstr[128];
	uint8_t iv;
	jsObj aOneDate = jsonNewArray();     //一个电量数据=[电量,有效位...]
	jsObj aTou = jsonNewArray();     //[正有,反有,正无,反无]
	for (j = 0; j<TOUNUM; j++) {     //电量 总尖峰平谷
		if (abQr[j]=='1') {
			jsonAdd(&aOneDate, NULL,
			                toStr(tmpstr, "%g", mtr.m_iQR[j]));
			iv = (mtr.Flag_QR&(0x1<<j))>>j;
			jsonAdd(&aOneDate, NULL,
			                toStr(tmpstr, "%u", iv));
			jsonAdd(&aTou, NULL, aOneDate);     //电量
			jsonClear(&aOneDate);
		}
	}
	jsonAdd(oMtr, "qr", aTou);
#if DEBUG_PRINT_REALTIME_TOU_DAT
	printf(WEBS_DBG"qr:%s\n",*oMtr);
#endif
	jsonFree(&aOneDate);
	jsonFree(&aTou);
	return 0;
}

static int add_mtr_v(jsObj *oMtr, struct stMeter_Run_data const mtr, char const *abV)
{
	int i;
	char tmpstr[128];
	uint8_t iv;
	jsObj aOneDate = jsonNewArray();
	jsObj aV = jsonNewArray();
	for (i = 0; i<PHASENUM; i++) {
		if (abV[i]=='1') {
			jsonAdd(&aOneDate, NULL,
			                toStr(tmpstr, "%g", mtr.m_wU[i]));
			iv = (mtr.FLag_TA&(0x1<<i))>>i;
			jsonAdd(&aOneDate, NULL,
			                toStr(tmpstr, "%u", iv));
			jsonAdd(&aV, NULL, aOneDate);     //电量
			jsonClear(&aOneDate);
		}
	}
	jsonAdd(oMtr, "v", aV);
#if DEBUG_PRINT_REALTIME_TOU_DAT
	printf(WEBS_DBG"v:%s\n",*oMtr);
#endif
	jsonFree(&aOneDate);
	jsonFree(&aV);
	return 0;
}
static int add_mtr_i(jsObj *oMtr, struct stMeter_Run_data const mtr, char const *abI)
{
	int i;
	char tmpstr[128];
	uint8_t iv;
	jsObj aOneDate = jsonNewArray();     //一个电量数据=[电量,有效位...]
	jsObj aI = jsonNewArray();     //[正有,反有,正无,反无]
	for (i = 0; i<PHASENUM; i++) {     //电量 总尖峰平谷
		if (abI[i]=='1') {
			jsonAdd(&aOneDate, NULL,
			                toStr(tmpstr, "%g", mtr.m_wI[i]));
			iv = (mtr.FLag_TA&(0x1<<i))>>i;
			jsonAdd(&aOneDate, NULL,
			                toStr(tmpstr, "%u", iv));
			jsonAdd(&aI, NULL, aOneDate);     //电量
			jsonClear(&aOneDate);
		}
	}
	jsonAdd(oMtr, "i", aI);
#if DEBUG_PRINT_REALTIME_TOU_DAT
	printf(WEBS_DBG"i:%s\n",*oMtr);
#endif
	jsonFree(&aOneDate);
	jsonFree(&aI);
	return 0;
}
/**
 * 实时电量查询,json格式参考doc/json-date-example.md 实时数据-电量
 * 发送数据到客户端
 */
static int webRece_realtime_tou(webs_t wp)
{
	struct stMeter_Run_data *mtr;
	char tmpstr[128];
	int i;
	char * abMtr = websGetVar(wp, T("mtr"), T(""));
	char * abTou = websGetVar(wp, T("tou"), T(""));
	char * abQr = websGetVar(wp, T("qr"), T(""));
	char * abV = websGetVar(wp, T("v"), T(""));
	char * abI = websGetVar(wp, T("i"), T(""));
	char * abP = websGetVar(wp, T("p"), T(""));
	char * abQ = websGetVar(wp, T("q"), T(""));
	char * abPf = websGetVar(wp, T("pf"), T(""));
	char * abF = websGetVar(wp, T("f"), T(""));
	int mtrnum = strlen(abMtr);
	int itemnum = strlen(abTou);
	if (check_mtrnum(mtrnum)<0) {
		web_err_proc(EL);
	}
	check_itemnum(itemnum);
	mtr = GetMeterAll_p();
	if (mtr==NULL ) {
		web_err_proc(EL);
	}
	jsObj oRealTimeData = jsonNew();     //{[{表1},{表2}],终端属性=xx,终端属性2=yy}
	jsObj aMtr = jsonNewArray();     //{[表0,表1...]}
	jsObj oMtr = jsonNew();     //{[it],表属性}
	jsonAdd(&oRealTimeData, "rtu_info", "this is rtu info,like version etc..");
	jsonAdd(&oRealTimeData, "abMtr", abMtr);//将参数回传,保证不出错
	jsonAdd(&oRealTimeData, "abTou", abTou);
	jsonAdd(&oRealTimeData, "abQr", abQr);
	jsonAdd(&oRealTimeData, "abV", abV);
	jsonAdd(&oRealTimeData, "abI", abI);
	jsonAdd(&oRealTimeData, "abP", abP);
	jsonAdd(&oRealTimeData, "abQ", abQ);
	jsonAdd(&oRealTimeData, "abPf", abPf);
	jsonAdd(&oRealTimeData, "abF", abF);
	for (i = 0; i<mtrnum; i++) {     //遍历所有表
		if (abMtr[i]!='1') {
			continue;
		}
		jsonAdd(&oMtr, "Meter_ReadTime"
		                , toStr(tmpstr, "%ld", mtr[i].Meter_ReadTime));
		add_mtr_tou(&oMtr, mtr[i], abTou);
		add_mtr_qr(&oMtr, mtr[i], abQr);
		add_mtr_v(&oMtr, mtr[i], abV);
		add_mtr_i(&oMtr, mtr[i], abI);
		jsonAdd(&aMtr, NULL, oMtr);
		jsonClear(&oMtr);
	}
	jsonAdd(&oRealTimeData, "mtr", aMtr);  //添加整个表对象
#if DEBUG_PRINT_REALTIME_TOU_DAT
	printf(WEBS_DBG"oRealTimeData itemnum %s\n",oRealTimeData);
#endif
	wpsend(wp, oRealTimeData);
	jsonFree(&aMtr);
	jsonFree(&oRealTimeData);
	return 0;
}
///简单检查项目数量 应该为[4(象限)+4(正反有无)]*5(总尖峰平谷),共40个项目(0或1)
static int check_itemnum(int itemnum)
{
	if (itemnum!=(TOUNUM+TOUNUM)) {
		web_err_proc(EL);
		return -1;
	}
	return 0;
}
///简单检查表数量合法性
static int check_mtrnum(int mtrnum)
{
	if (mtrnum<=0) {
		web_errno = eno_realtime_tou_mtrnum_too_small;
		//web_err_proc(EL);
		return -1000;
	}
	if (mtrnum>MAXMETER) {
		web_errno = eno_realtime_tou_mtrnum_too_big;
		//web_err_proc(EL);
		return -1002;
	}
	return 0;
}
