/**
 * @file rtit.c real time 电量,实时电量
 */
#include <string.h>
#include <time.h>
#include "define.h"
#include "rtit.h"
#include "conf.h"
#include "web_err.h"
#include "wpsend.h"
#include "json.h"
static int webRece_realtime_tou(webs_t wp);
static int check_mtrnum(int mtrnum);
static int add_mtr_tou(jsObj *oMtr, struct stMeter_Run_data const mtr, char const *abTou);
static int add_mtr_qr(jsObj *oMtr, struct stMeter_Run_data const mtr, char const *abQr);
static int add_mtr_v(jsObj *oMtr, struct stMeter_Run_data const mtr, char const *abV);
static int add_mtr_i(jsObj *oMtr, struct stMeter_Run_data const mtr, char const *abI);
static int add_mtr_p(jsObj *oMtr, struct stMeter_Run_data const mtr, char const *abP);
static int add_mtr_q(jsObj *oMtr, struct stMeter_Run_data const mtr, char const *abQ);
static int add_mtr_pf(jsObj *oMtr, struct stMeter_Run_data const mtr, char const *abPf);
static int add_mtr_maxn(jsObj *oMtr, struct stMeter_Run_data const mtr, char const *abMaxn);
static time_t toUnixTimestamp(unsigned int rtMaxnTime);
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
/**
 * 实时电量查询,json格式参考doc/json-date-example.md 实时数据-电量
 * 发送数据到客户端
 */
static int
webRece_realtime_tou(webs_t wp)
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
	char * abF = websGetVar(wp, T("f"), T(""));     //频率貌似暂时没用
	char * abMaxn = websGetVar(wp, T("maxn"), T(""));
	int mtrnum = strlen(abMtr);
	if (check_mtrnum(mtrnum)<0) {
		web_err_proc(EL);
	}
	mtr = GetMeterAll_p();
	if (mtr==NULL ) {
		web_err_proc(EL);
	}
	jsObj oRealTimeData = jsonNew();     //{[{表1},{表2}],终端属性=xx,终端属性2=yy}
	jsObj aMtr = jsonNewArray();     //{[表0,表1...]}
	jsObj oMtr = jsonNew();     //{[it],表属性}
	jsonAdd(&oRealTimeData, "rtu_info",
	                "this is rtu info,like version etc..");
	jsonAdd(&oRealTimeData, "abMtr", abMtr);     //将参数回传,保证不出错
	jsonAdd(&oRealTimeData, "abTou", abTou);
	jsonAdd(&oRealTimeData, "abQr", abQr);
	jsonAdd(&oRealTimeData, "abV", abV);
	jsonAdd(&oRealTimeData, "abI", abI);
	jsonAdd(&oRealTimeData, "abP", abP);
	jsonAdd(&oRealTimeData, "abQ", abQ);
	jsonAdd(&oRealTimeData, "abPf", abPf);
	jsonAdd(&oRealTimeData, "abF", abF);
	jsonAdd(&oRealTimeData, "abMaxn", abMaxn);
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
		add_mtr_p(&oMtr, mtr[i], abP);
		add_mtr_q(&oMtr, mtr[i], abQ);
		add_mtr_pf(&oMtr, mtr[i], abPf);
		add_mtr_maxn(&oMtr, mtr[i], abMaxn);
		//
		jsonAdd(&aMtr, NULL, oMtr);
		jsonClear(&oMtr);
	}
	jsonAdd(&oRealTimeData, "mtr", aMtr);     //添加整个表对象
#if DEBUG_PRINT_REALTIME_TOU_DAT
	printf(WEBS_DBG"oRealTimeData itemnum %s\n",oRealTimeData);
#endif
	wpsend(wp, oRealTimeData);
	jsonFree(&oMtr);
	jsonFree(&aMtr);
	jsonFree(&oRealTimeData);
	return 0;
}
//向一个表(mtr)中添加it对象,指示这个表中所有tou实时电量数据.
static int
add_mtr_tou(jsObj *oMtr, struct stMeter_Run_data const mtr, char const *abTou)
{
	if (strlen(abTou)!=TOUNUM) {
		web_err_proc(EL);
		return -100;
	}
	int j;
	char tmpstr[128];
	uint8_t iv;
	jsObj aOneDate = jsonNewArray();     //一个电量数据=[电量,有效位...]
	jsObj aTou = jsonNewArray();     //[正有,反有,正无,反无]
	for (j = 0; j<TOUNUM; j++) {     //电量 总尖峰平谷
		if (abTou[j]!='1') {
			continue;
		}
		jsonAdd(&aOneDate, NULL, toStr(tmpstr, "%g", mtr.m_iTOU[j]));
		iv = (mtr.Flag_TOU&(0x1<<j))>>j;
		jsonAdd(&aOneDate, NULL, toStr(tmpstr, "%u", iv));
		jsonAdd(&aTou, NULL, aOneDate);     //电量
		jsonClear(&aOneDate);
	}
	jsonAdd(oMtr, "tou", aTou);
#if DEBUG_PRINT_REALTIME_TOU_DAT
	printf(WEBS_DBG"tou:%s\n",*oMtr);
#endif
	jsonFree(&aOneDate);
	jsonFree(&aTou);
	return 0;
}
//向一个表(mtr)中添加qr四象限无功电量
static int
add_mtr_qr(jsObj *oMtr, struct stMeter_Run_data const mtr, char const *abQr)
{
	if (strlen(abQr)!=TOUNUM) {
		web_err_proc(EL);
		return -100;
	}
	int j;
	char tmpstr[128];
	uint8_t iv;
	jsObj aOneDate = jsonNewArray();
	jsObj aQr = jsonNewArray();
	for (j = 0; j<TOUNUM; j++) {
		if (abQr[j]!='1') {
			continue;
		}
		jsonAdd(&aOneDate, NULL, toStr(tmpstr, "%g", mtr.m_iQR[j]));
		iv = (mtr.Flag_QR&(0x1<<j))>>j;
		jsonAdd(&aOneDate, NULL,toStr(tmpstr, "%u", iv));
		jsonAdd(&aQr, NULL, aOneDate);
		jsonClear(&aOneDate);
	}
	jsonAdd(oMtr, "qr", aQr);
#if DEBUG_PRINT_REALTIME_TOU_DAT
	printf(WEBS_DBG"qr:%s\n",*oMtr);
#endif
	jsonFree(&aOneDate);
	jsonFree(&aQr);
	return 0;
}

static int
add_mtr_v(jsObj *oMtr, struct stMeter_Run_data const mtr, char const *abV)
{
	if (strlen(abV)!=PHASENUM) {
		web_err_proc(EL);
		return -100;
	}
	int i;
	uint8_t offset = 0;     //有效标志在FLag_TA32位中的偏移
	char tmpstr[128];
	uint8_t iv;
	jsObj aOneDate = jsonNewArray();
	jsObj aV = jsonNewArray();
	for (i = 0; i<PHASENUM; i++) {
		if (abV[i]!='1') {
			continue;
		}
		jsonAdd(&aOneDate, NULL, toStr(tmpstr, "%g", mtr.m_wU[i]));
		iv = (mtr.FLag_TA&(0x1<<(i+offset)))>>(i+offset);
		jsonAdd(&aOneDate, NULL, toStr(tmpstr, "%u", iv));
		jsonAdd(&aV, NULL, aOneDate);
		jsonClear(&aOneDate);
	}
	jsonAdd(oMtr, "v", aV);
#if DEBUG_PRINT_REALTIME_TOU_DAT
	printf(WEBS_DBG"v:%s\n",*oMtr);
#endif
	jsonFree(&aOneDate);
	jsonFree(&aV);
	return 0;
}
static int
add_mtr_i(jsObj *oMtr, struct stMeter_Run_data const mtr, char const *abI)
{
	if (strlen(abI)!=PHASENUM) {
		web_err_proc(EL);
		return -100;
	}
	int i;
	uint8_t offset = PHASENUM;
	char tmpstr[128];
	uint8_t iv;
	jsObj aOneDate = jsonNewArray();
	jsObj aI = jsonNewArray();
	for (i = 0; i<PHASENUM; i++) {
		if (abI[i]!='1') {
			continue;
		}
		jsonAdd(&aOneDate, NULL, toStr(tmpstr, "%g", mtr.m_wI[i]));
		iv = (mtr.FLag_TA&(0x1<<(i+offset)))>>(i+offset);
		jsonAdd(&aOneDate, NULL, toStr(tmpstr, "%u", iv));
		jsonAdd(&aI, NULL, aOneDate);
		jsonClear(&aOneDate);
	}
	jsonAdd(oMtr, "i", aI);
#if DEBUG_PRINT_REALTIME_TOU_DAT
	printf(WEBS_DBG"i:%s\n",*oMtr);
#endif
	jsonFree(&aOneDate);
	jsonFree(&aI);
	return 0;
}
static int add_mtr_p(jsObj *oMtr, struct stMeter_Run_data const mtr, char const *abP)
{
	if (strlen(abP)!=PQCNUM) {
		web_err_proc(EL);
		return -100;
	}
	int i;
	uint8_t offset = PHASENUM+PHASENUM;
	char tmpstr[128];
	uint8_t iv;
	jsObj aOneDate = jsonNewArray();
	jsObj aP = jsonNewArray();
	for (i = 0; i<PQCNUM; i++) {
		if (abP[i]!='1') {
			continue;
		}
		jsonAdd(&aOneDate, NULL, toStr(tmpstr, "%g", mtr.m_iP[i]));
		iv = (mtr.FLag_TA&(0x1<<(i+offset)))>>(i+offset);
		jsonAdd(&aOneDate, NULL, toStr(tmpstr, "%u", iv));
		jsonAdd(&aP, NULL, aOneDate);
		jsonClear(&aOneDate);
	}
	jsonAdd(oMtr, "p", aP);
#if DEBUG_PRINT_REALTIME_TOU_DAT
	printf(WEBS_DBG"p:%s\n",*oMtr);
#endif
	jsonFree(&aOneDate);
	jsonFree(&aP);
	return 0;
}
static int add_mtr_q(jsObj *oMtr, struct stMeter_Run_data const mtr, char const *abQ)
{
	if (strlen(abQ)!=PQCNUM) {
		web_err_proc(EL);
		return -100;
	}
	int i;
	uint8_t offset = PHASENUM+PHASENUM+PQCNUM;
	char tmpstr[128];
	uint8_t iv;
	jsObj aOneDate = jsonNewArray();
	jsObj aQ = jsonNewArray();
	for (i = 0; i<PQCNUM; i++) {
		if (abQ[i]!='1') {
			continue;
		}
		jsonAdd(&aOneDate, NULL, toStr(tmpstr, "%g", mtr.m_wQ[i]));
		iv = (mtr.FLag_TA&(0x1<<(i+offset)))>>(i+offset);
		jsonAdd(&aOneDate, NULL, toStr(tmpstr, "%u", iv));
		jsonAdd(&aQ, NULL, aOneDate);
		jsonClear(&aOneDate);
	}
	jsonAdd(oMtr, "q", aQ);
#if DEBUG_PRINT_REALTIME_TOU_DAT
	printf(WEBS_DBG"q:%s\n",*oMtr);
#endif
	jsonFree(&aOneDate);
	jsonFree(&aQ);
	return 0;
}
static int
add_mtr_pf(jsObj *oMtr, struct stMeter_Run_data const mtr, char const *abPf)
{
	if (strlen(abPf)!=PQCNUM) {
		web_err_proc(EL);
		return -100;
	}
	int i;
	uint8_t offset = PHASENUM+PHASENUM+PQCNUM+PQCNUM;
	char tmpstr[128];
	uint8_t iv;
	jsObj aOneDate = jsonNewArray();
	jsObj aPf = jsonNewArray();
	for (i = 0; i<PQCNUM; i++) {
		if (abPf[i]!='1') {
			continue;
		}
		jsonAdd(&aOneDate, NULL,toStr(tmpstr, "%g", mtr.m_wPF[i]));
		iv = (mtr.FLag_TA&(0x1<<(i+offset)))>>(i+offset);
		jsonAdd(&aOneDate, NULL, toStr(tmpstr, "%u", iv));
		jsonAdd(&aPf, NULL, aOneDate);
		jsonClear(&aOneDate);
	}
	jsonAdd(oMtr, "pf", aPf);
#if DEBUG_PRINT_REALTIME_TOU_DAT
	printf(WEBS_DBG"pf:%s\n",*oMtr);
#endif
	jsonFree(&aOneDate);
	jsonFree(&aPf);
	return 0;
}
static int
add_mtr_maxn(jsObj *oMtr, struct stMeter_Run_data const mtr, char const *abMaxn)
{
	if (strlen(abMaxn)!=TOUNUM) {
		web_err_proc(EL);
		return -100;
	}
	int i;
	char tmpstr[128];
	uint8_t iv;
	jsObj aOneDate = jsonNewArray();
	jsObj aMaxn = jsonNewArray();
	for (i = 0; i<TOUNUM; i++) {
		if (abMaxn[i]!='1') {
			continue;
		}
		jsonAdd(&aOneDate, NULL,toStr(tmpstr, "%g", mtr.m_iMaxN[i]));
		iv = (mtr.Flag_MNT&(0x1<<i))>>i;
		jsonAdd(&aOneDate, NULL, toStr(tmpstr, "%u", iv));
		jsonAdd(&aOneDate, NULL,
		                toStr(tmpstr, "%ld",
		                                toUnixTimestamp(mtr.m_iMaxNT[i])));
		jsonAdd(&aMaxn, NULL, aOneDate);
		jsonClear(&aOneDate);

	}
	jsonAdd(oMtr, "maxn", aMaxn);
#if DEBUG_PRINT_REALTIME_TOU_DAT
	printf(WEBS_DBG"maxn:%s\n",*oMtr);
#endif
	jsonFree(&aOneDate);
	jsonFree(&aMaxn);
	return 0;
}
///简单检查表数量合法性
static int check_mtrnum(int mtrnum)
{
	if (mtrnum<=0) {
		web_errno = eno_realtime_tou_mtrnum_too_small;
		return -1000;
	}
	if (mtrnum>MAXMETER) {
		web_errno = eno_realtime_tou_mtrnum_too_big;
		return -1002;
	}
	return 0;
}
/**
 * 将规约形式的时间类型4字节,见下方 stRealTimeMaxNeedTime 类型.
 * @param rtMaxnTime
 * @return
 */
static time_t toUnixTimestamp(unsigned int rtMaxnTime)
{
	union stRealTimeMaxNeedTime {
		unsigned int val;
		struct {
			uint32_t sec :6;
			uint32_t min :6;
			uint32_t hour :5;
			uint32_t mday :5;     //1-31
			uint32_t mon :4;     //1-12
			uint32_t year :6;     //从2000年到现在的年数
		};
	} rtTime;
	if (rtMaxnTime<=0) {
		return 0;
	}
	time_t t;
	struct tm tm;
	rtTime.val = rtMaxnTime;
	tm.tm_sec = rtTime.sec;
	tm.tm_min = rtTime.min;
	tm.tm_hour = rtTime.hour;
	tm.tm_mday = rtTime.mday;     //标准日 1-31
	tm.tm_mon = rtTime.mon-1;     //unix 标准月0~11
	tm.tm_year = rtTime.year+100;
#if 0
	printf(WEBS_DBG"day %d,hour:%d \n",rtTime.mday,rtTime.hour);
	printf(WEBS_DBG"day sys %d hour \n",tm.tm_mday);
#endif
	t = mktime(&tm);
#if 0
	printf(WEBS_DBG"sys time(struct): %s\n",asctime(&tm));
	printf(WEBS_DBG"sys time(t): %s\n",ctime(&t));
#endif
	return t;
}
