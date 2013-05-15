/**
 * @file tou.c 电量历史数据文件,=>所有历史数据,电量,瞬时量,需量
 * @param mtr_no
 * @param range
 * @param tou
 * @return
 */
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <errno.h>
#include "../uemf.h"
#include "../wsIntrn.h"
#include "tou.h"
#include "web_err.h"
#include "conf.h"
#include "define.h"
///仅在本文件中使用的静态函数定义.
static int webRece_history_data(webs_t wp);
//
static int add_mtr_tou(jsObj *oMtr, int mtrNo,
        char const *abTou, TimeRange const range);
static int add_mtr_qr(jsObj *oMtr, int mtrNo,
        char const *abQr, TimeRange const range);
static int add_mtr_instant(jsObj *oMtr, int mtrNo,
        char const *abInstant, TimeRange const range);
static int add_mtr_maxn(jsObj *oMtr, int mtrNo,
        char const *abMaxn, TimeRange const range);
//
static int search_records_tou(jsObj *oMtr, uint32_t mtr_no,
        TimeRange const range, const char* enable);
static int search_records_instant(jsObj *aInstant, uint32_t mtr_no,
        TimeRange const range, const char *enable);
static int search_records_maxn(jsObj *oMtr, uint32_t mtr_no,
        TimeRange const range, const char* abMaxn);
static int search_records_qr(jsObj *oMtr, uint32_t mtr_no,
        TimeRange const range, const char *enable);
//
static int mkOneTouDataRecord(jsObj *OneTimeTou, time_t t2,
        const stTou tou, int i, int mtr_no, const char *en);
static int mkOneInstantDataRecord(jsObj *aInstantData, time_t t2,
        const stInstant instant, int i, int mtr_no, const char *en);
static int mkOneMaxnDataRecord(jsObj *OneTimeTou, time_t t2,
        const stMaxn maxn, int i, int mtr_no, const char *en);
static int mkOneQrDataRecord(jsObj *OneTimeTou, time_t t2,
        const stQr qr, int i, int mtr_no, const char *en);
//
static int mkTouDataArray(jsObj *a, const stTou tou, const char *en);
static int mkQrDataArray(jsObj *a, const stQr qr, const char *en);
static int mkInstantDataArray(jsObj *a, const stInstant instant, const char *en);
static int mkMaxnDataArray(jsObj *a, const stMaxn manx, const char *en);
//
static int mkOneTouTi(jsObj *a, const touTi_Category ti, const char *en);
static int mkOneQrTi(jsObj *a, const touTi_Category ti, const char *en);
static int mkOneInstantTi(jsObj *a, int isPQ, const Ti* ti, const char *en);
static int mkOneMaxnTI(jsObj *a, const stMaxn_Ti_Category ti, const char *en);

//
static int isRightDate(const stTouFilehead filehead, struct tm t);
static int isRightDate_Qr(const stQrFilehead filehead, struct tm t);
static int isRightDate_Instant(const stInstantFilehead filehead, struct tm t);
static int isRightDate_Maxn(const stMaxnFilehead filehead, struct tm t);
//
static char * float2string(uint8_t const float_array[4], char * strval);
static void timeToNextDayMorning(struct tm *stTime, time_t *time_t);
static int check_mtrnum(int mtrnum);
#if 0
/**
 * 提交表单,历史电量数据.操作:获取.参数:时间范围,表号.
 * @param wp
 * @param path
 * @param query
 */
void form_history_tou(webs_t wp, char_t *path, char_t *query)
{
	PRINT_FORM_INFO;
	char * strmtr_no = websGetVar(wp, T("mtr_no"), T("0"));
	char * stime_t = websGetVar(wp, T("stime_stamp"), T("0"));
	char * etime_t = websGetVar(wp, T("etime_stamp"), T("0"));
	char * strtz = websGetVar(wp, T("timezone"), T("0"));
	int tz = 0;
	//printf("时间戳范围:%s~%s\n", stime_t, etime_t);
	TimeRange tr;
	int ret;
	int mtr_no = 0;
	stTou tou;
	memset(&tou, 0x00, sizeof(stTou));
	ret = sscanf(strmtr_no, "%d", &mtr_no);
	if (ret!=1) {
		web_err_proc(EL);
	}
	ret = sscanf(stime_t, "%ld", &tr.s);
	if (ret!=1) {
		web_err_proc(EL);
	}
	ret = sscanf(etime_t, "%ld", &tr.e);
	if (ret!=1) {
		web_err_proc(EL);
	}
	ret = sscanf(strtz, "%d", &tz);
	if (ret!=1) {
		web_err_proc(EL);
	}
	tr.s += (tz*60);
	tr.e += (tz*60);
	//printf("时间戳 (数值) 范围:%ld~%ld 表号:%d\n", tr.s, tr.e, mtr_no);
	websHeader_pure(wp);
	ret = search_records_tou(mtr_no, tr, &tou, wp);
	if (ret==ERR) {
		web_err_proc(EL);
	}
	websDone(wp, 200);
	return;
}
#else
void form_history_tou(webs_t wp, char_t *path, char_t *query)
{
	PRINT_FORM_INFO;
	websHeader_pure(wp);
	char * action = websGetVar(wp, T("action"), T("null"));
	if (strcmp(action, "get")==0) {
		webRece_history_data(wp);
	} else {
		web_err_proc(EL);
	}
	websDone(wp, 200);
	return;
}
#endif
int makeTimeTange(TimeRange *tr, webs_t wp)
{
	int tz = 0;
	int ret;
	char * stime_t = websGetVar(wp, T("stime_stamp"), T("0"));
	char * etime_t = websGetVar(wp, T("etime_stamp"), T("0"));
	char * strtz = websGetVar(wp, T("timezone"), T("0"));
	ret = sscanf(stime_t, "%ld", &tr->s);
	if (ret!=1) {
		web_err_proc(EL);
	}
	ret = sscanf(etime_t, "%ld", &tr->e);
	if (ret!=1) {
		web_err_proc(EL);
	}
	ret = sscanf(strtz, "%d", &tz);
	if (ret!=1) {
		web_err_proc(EL);
	}
	tr->s += (tz);
	tr->e += (tz);
	return 0;
}
static int webRece_history_data(webs_t wp)
{
	int i;
	char * abMtr = websGetVar(wp, T("mtr"), T(""));
	char * datatType = websGetVar(wp, T("type"), T(""));
	int mtrnum = strlen(abMtr);
	if (check_mtrnum(mtrnum)<0) {
		web_err_proc(EL);
	}
	TimeRange tr;
	makeTimeTange(&tr, wp);
	jsObj oHistoryData = jsonNew();
	jsObj aMtr = jsonNewArray();
	jsObj oMtr = jsonNew();
	//回显查询信息,增强可靠性
	jsonAdd(&oHistoryData, "rtu_info",
	                "this is rtu info,like version etc..");
	jsonAdd(&oHistoryData, "type", datatType);
	jsonAdd(&oHistoryData, "abMtr", abMtr);     //将参数回传,保证不出错
	//根据请求,仅发送一类数据,电量/无功/瞬时量/需量
	if (strcmp(datatType, "tou")==0) {
		char * abTou = websGetVar(wp, T("tou"), T(""));
		jsonAdd(&oHistoryData, "abTou", abTou);
		for (i = 0; i<mtrnum; i++) {     //遍历所有表
			if (abMtr[i]!='1')
				continue;
			add_mtr_tou(&oMtr, i, abTou, tr);
			jsonAdd(&aMtr, NULL, oMtr);
			jsonClean(&oMtr);
		}
	} else if (strcmp(datatType, "qr")==0) {
		char * abQr = websGetVar(wp, T("qr"), T(""));
		jsonAdd(&oHistoryData, "abQr", abQr);
		for (i = 0; i<mtrnum; i++) {     //遍历所有表
			if (abMtr[i]!='1')
				continue;
			add_mtr_qr(&oMtr, i, abQr, tr);
			jsonAdd(&aMtr, NULL, oMtr);
			jsonClean(&oMtr);
		}
	} else if (strcmp(datatType, "instant")==0) {
		char * abV = websGetVar(wp, T("v"), T(""));
		char * abI = websGetVar(wp, T("i"), T(""));
		char * abP = websGetVar(wp, T("p"), T(""));
		char * abQ = websGetVar(wp, T("q"), T(""));
		char * abPf = websGetVar(wp, T("pf"), T(""));
		char * abF = websGetVar(wp, T("f"), T(""));     //频率貌似暂时没用
		jsonAdd(&oHistoryData, "abV", abV);
		jsonAdd(&oHistoryData, "abI", abI);
		jsonAdd(&oHistoryData, "abP", abP);
		jsonAdd(&oHistoryData, "abQ", abQ);
		jsonAdd(&oHistoryData, "abPf", abPf);
		jsonAdd(&oHistoryData, "abF", abF);
		char abInstant[128];
		sprintf(abInstant, "%s%s%s%s%s%s",
		                abV, abI, abP, abQ, abPf, abF);
		for (i = 0; i<mtrnum; i++) {     //遍历所有表
			if (abMtr[i]!='1')
				continue;
			add_mtr_instant(&oMtr, i, abInstant, tr);
			jsonAdd(&aMtr, NULL, oMtr);
			jsonClean(&oMtr);
		}
	} else if (strcmp(datatType, "maxn")==0) {
		char * abMaxn = websGetVar(wp, T("maxn"), T(""));
		jsonAdd(&oHistoryData, "abMaxn", abMaxn);
		for (i = 0; i<mtrnum; i++) {     //遍历所有表
			if (abMtr[i]!='1')
				continue;
			add_mtr_maxn(&oMtr, i, abMaxn, tr);
			jsonAdd(&aMtr, NULL, oMtr);
			jsonClean(&oMtr);
		}
	}
	jsonAdd(&oHistoryData, "mtr", aMtr);     //添加整个表对象
#if DEBUG_PRINT_HISTORY_DAT
	                printf(WEBS_DBG"历史数据:%s\n", oHistoryData);
#endif
	wpsend(wp, oHistoryData);
	jsonFree(&oMtr);
	jsonFree(&aMtr);
	jsonFree(&oHistoryData);
	return 0;
}


//一个表(mtrNo)在一段时间范围(range)内特定电量项目(abTou)保存到oMtr json数组中.
static int add_mtr_tou(jsObj *oMtr, int mtrNo,
	char const *abTou, TimeRange const range)
{
	if (strlen(abTou)!=TOUNUM) {
		web_err_proc(EL);
		return -100;
	}
	jsObj aOneDate = jsonNewArray();     //一个电量数据=[电量,有效位...]
	jsObj aTou = jsonNewArray();     //[正有,反有,正无,反无]
	search_records_tou(&aTou, mtrNo, range, abTou);
	jsonAdd(oMtr, "tou", aTou);
#if DEBUG_PRINT_REALTIME_TOU_DAT
	printf(WEBS_DBG"tou:%s\n",*oMtr);
#endif
	jsonFree(&aOneDate);
	jsonFree(&aTou);
	return 0;
}
static int add_mtr_qr(jsObj *oMtr, int mtrNo,
	char const *abQr, TimeRange const range)
{
	if (strlen(abQr)!=TOUNUM) {
		web_err_proc(EL);
		return -100;
	}
	jsObj aOneDate = jsonNewArray();     //一个电量数据=[电量,有效位...]
	jsObj aTou = jsonNewArray();     //[正有,反有,正无,反无]
	search_records_qr(&aTou, mtrNo, range, abQr);
	jsonAdd(oMtr, "qr", aTou);
#if DEBUG_PRINT_REALTIME_TOU_DAT
	printf(WEBS_DBG"qr:%s\n",*oMtr);
#endif
	jsonFree(&aOneDate);
	jsonFree(&aTou);
	return 0;
}
static int add_mtr_instant(jsObj *oMtr, int mtrNo,
	char const *abInstant,TimeRange const range)
{
	printf(WEBS_DBG"enable %s", abInstant);
	if (strlen(abInstant)!=PHASENUM+PHASENUM+PQCNUM+PQCNUM+PQCNUM) {
		web_err_proc(EL);
	}
	jsObj aInstant = jsonNewArray();     //[正有,反有,正无,反无]
	search_records_instant(&aInstant, mtrNo, range, abInstant);
	jsonAdd(oMtr, "instant", aInstant);
#if 0
	printf(WEBS_DBG"instant:%s\n",*oMtr);
#endif
	jsonFree(&aInstant);
	return 0;
}
static int add_mtr_maxn(jsObj *oMtr, int mtrNo,
	char const *abMaxn,TimeRange const range)
{
	if (strlen(abMaxn)!=TOUNUM) {
		web_err_proc(EL);
		return -100;
	}
	jsObj aOneDate = jsonNewArray();     //一个电量数据=[电量,有效位...]
	jsObj aMaxn = jsonNewArray();     //[正有,反有,正无,反无]
	search_records_maxn(&aMaxn, mtrNo, range, abMaxn);
#if DEBUG_PRINT_HISTORY_DAT && DEBUG_PRINT_HISTORY_DAT_MAXN
	printf(WEBS_DBG"maxn_array:%s\n",aMaxn);
#endif
	jsonAdd(oMtr, "maxn", aMaxn);
#if DEBUG_PRINT_HISTORY_DAT && DEBUG_PRINT_HISTORY_DAT_MAXN
	printf(WEBS_DBG"maxn:%s\n",*oMtr);
#endif
	jsonFree(&aOneDate);
	jsonFree(&aMaxn);
	return 0;
}
/**
 * 读取一个电表的一段时间段的电量数据.
 * 这个函数思路十分难以理解.
 * 主要是按照时间戳,以一分钟为最小刻度依次从开始时刻到结束时刻遍历一遍.
 * 获取采样周期,如5分钟,则可以以5分钟为最小刻度遍历,速度更快.
 * 遍历过程中有一种比较特殊的情况:
 * 1. 开始时刻不是周期的整数倍.
 * 	如01月01日00时01分开始,读取文件mtr0010101之后发现周期是5分钟.
 * 	那个需要将起始时刻向上园整到5分钟的整数倍,即从01月01日00时05分这个时刻开始.
 * 	之后就可以按照5分钟递增了.
 * 2. 开始时刻为一日较晚时刻,采样跨天.
 * 	如01月01日23时59分开始,读取文件mtr0010101之后得到采样周期5分钟.
 * 	那么23:59加5分钟就到了次日.所以应该返回打开下一个文件mtr0010102.
 * 	然后从01月02日00时00分开始,以5分钟为一个周期开始读取.
 * 3. 在不同的天中采样周期有所变更.
 * 	程序始终以min{当前周期,文件中读取的周期}为步距递增.所以不同天不同周期没问题.
 * 4. 在同一天中变更采样周期. @bug
 * 	无法判断,因为一天就保存一个周期到文件头中.而且文件的程度计算必然出错.
 * @param[in] range
 * @param[in] mtr_no
 * @param[in] ptou
 * @param[out] wp 写入到这个页面
 * @return
 * @todo 分解,流程较为复杂
 */
static int
search_records_tou(jsObj *oMtr, uint32_t mtr_no,
        TimeRange const range, const char *enable)
{
	stTouFilehead filehead;
	if (range.e<range.s) {
		web_errno = tou_timerange_err;
		return ERR;
	}
	if (range.e==0) {
		web_errno = tou_stime_err;
		return ERR;
	}
	if (range.e==0) {
		web_errno = tou_etime_err;
		return ERR;
	}
	char file[256] = { 0 };
	struct tm stTime;
	struct tm stToday_0;     //今日凌晨00点00分
	time_t today_0_t;
	time_t start_t = range.s;     //开始时刻
	time_t end_t = range.e;     //结束时刻
	time_t t2;     //时刻
	time_t minCycle_t = 0;
	stTou tou;
	memset(&tou, 0x0, sizeof(stTou));
	FILE*fp;
	int flen;
	int i = 0;
	//从开始时刻到结束时刻,按分钟遍历,步距为周期,可变.[start,end]两边闭区间
	for (t2 = start_t; t2<=end_t; /*t2 += (mincycle * 60)*/) {
		Start:
		#if __arm__ ==2
		gmtime_r(&t2,&stTime);
		gmtime_r(&t2,&stToday_0);
//		printf("gmtime_r %02d-%02d %02d:%02d %s stTime.tm_gmtoff=%d \n",
//				t.tm_mon+1,stTime.tm_mday,stTime.tm_hour,stTime.tm_min,
//				stTime.tm_zone,stTime.tm_gmtoff);
#else
		localtime_r(&t2, &stTime);
		localtime_r(&t2, &stToday_0);
#endif
		sprintf(file, "%s/mtr%03d%02d%02d.%s", TOU_DAT_DIR, mtr_no, 0,
		                stTime.tm_mday, TOU_DAT_SUFFIX);
		fp = fopen(file, "r");
		if (fp==NULL ) {     //这一天没有数据,直接跳到次日零点,这不是错误
			printf(WEBS_INF"%d:%04d-%02d-%02d没有数据文件\n",
			                mtr_no, stTime.tm_year+1900, stTime.tm_mon+1
			                                , stTime.tm_mday);
			web_errno = open_tou_file;
			//到下一天的凌晨,即下一个文件.
			timeToNextDayMorning(&stTime, &t2);
			continue;
		}
		fseek(fp, 0, SEEK_END);
		flen = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		int n = fread(&filehead, sizeof(stTouFilehead), 1, fp);
		if (n!=1) {
			web_errno = read_tou_file_filehead;
			fclose(fp);
			timeToNextDayMorning(&stTime, &t2);
			continue;
		}
		///@note 检查文件头中是否和请求的日期相一致.
		if (isRightDate(filehead, stTime)==0) {     //这也不算错误,最多算信息.
			fclose(fp);
			timeToNextDayMorning(&stTime, &t2);
			continue;
		}
		int cycle = (filehead.save_cycle_hi*256)
		                +filehead.save_cycle_lo;
		minCycle_t = cycle;
		//stTou at[24 * 60 / cycle];
		int t_mod = t2%(minCycle_t*60);     //向上园整至采样周期.
		if (t_mod!=0) {     //需要园整
			t2 += (minCycle_t*60-t_mod);
		}
		//}
		/**@note 判断开始时间+周期是否跨度到了第二天,如果跨度到第二天则需要
		 打开另一个(下一天)数据文件.
		 */
		stToday_0.tm_hour = 0;
		stToday_0.tm_min = 0;
		stToday_0.tm_sec = 0;
		today_0_t = mktime(&stToday_0);
		if (t2-today_0_t>=(60*60*24)) {     //t2已经时间跨过本日了.次日则文件等等需要重新打开.
			fclose(fp);
			goto Start;
		}
		///移动文件指针,指向开始的数据结构.
		int DeltaSec = t2-today_0_t;     //本采样时刻举今日凌晨几秒
		int NumCycle = DeltaSec/(minCycle_t*60);     //从凌晨开始向后偏移几个采样周期
		int offset = sizeof(stTou)*NumCycle;     //每个样本长度*采样个数
		fseek(fp, offset, SEEK_CUR);     ///当前位置为除去文件头的第一个数据体.

		if (ftell(fp)>=flen) {
			printf(WEBS_WAR"本日的数据不够.filesize=%d,fseek=%ld:%s\n", flen,
			                ftell(fp), file);
			t2 += (minCycle_t*60);
			fclose(fp);
			continue;
		}
		while (ftell(fp)<flen&&t2<=end_t) {
			memset(&tou, 0x0, sizeof(stTou));
			//t_cur += cycle * 60;
			int n = fread(&tou, sizeof(stTou), 1, fp);
			if (n!=1) {
				web_errno = read_tou_file_dat;
				return ERR;
			}
			//成功
			mkOneTouDataRecord(oMtr, t2, tou, i, mtr_no, enable);
			i++;
			t2 += (minCycle_t*60);
		}     // end while 在一个文件中
		fclose(fp);
	}     // end for
	return 0;
}
static int
search_records_qr(jsObj *oMtr, uint32_t mtr_no,
        TimeRange const range, const char *enable)
{
	stQrFilehead filehead;
	if (range.e<range.s) {
		web_errno = tou_timerange_err;
		return ERR;
	}
	if (range.e==0) {
		web_errno = tou_stime_err;
		return ERR;
	}
	if (range.e==0) {
		web_errno = tou_etime_err;
		return ERR;
	}
	char file[256] = { 0 };
	struct tm stTime;
	struct tm stToday_0;     //今日凌晨00点00分
	time_t today_0_t;
	time_t start_t = range.s;     //开始时刻
	time_t end_t = range.e;     //结束时刻
	time_t t2;     //时刻
	time_t minCycle_t = 0;
	stQr qr;
	memset(&qr, 0x0, sizeof(stQr));
	FILE*fp;
	int flen;
	int i = 0;
	//从开始时刻到结束时刻,按分钟遍历,步距为周期,可变.[start,end]两边闭区间
	for (t2 = start_t; t2<=end_t; /*t2 += (mincycle * 60)*/) {
		Start:
		#if __arm__ ==2
		gmtime_r(&t2,&stTime);
		gmtime_r(&t2,&stToday_0);
//		printf("gmtime_r %02d-%02d %02d:%02d %s stTime.tm_gmtoff=%d \n",
//				t.tm_mon+1,stTime.tm_mday,stTime.tm_hour,stTime.tm_min,
//				stTime.tm_zone,stTime.tm_gmtoff);
#else
		localtime_r(&t2, &stTime);
		localtime_r(&t2, &stToday_0);
#endif
		sprintf(file, "%s/mtr%03d%02d%02d.%s", TOU_DAT_DIR, mtr_no, 0,
		                stTime.tm_mday, QR_DAT_SUFFIX);
		fp = fopen(file, "r");
		if (fp==NULL ) {     //这一天没有数据,直接跳到次日零点,这不是错误
			printf(WEBS_INF"%d:%04d-%02d-%02d没有数据文件\n",
			                mtr_no, stTime.tm_year+1900, stTime.tm_mon+1
			                                , stTime.tm_mday);
			web_errno = open_tou_file;
			//到下一天的凌晨,即下一个文件.
			timeToNextDayMorning(&stTime, &t2);
			continue;
		}
		fseek(fp, 0, SEEK_END);
		flen = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		int n = fread(&filehead, sizeof(stQrFilehead), 1, fp);
		if (n!=1) {
			web_errno = read_tou_file_filehead;
			fclose(fp);
			timeToNextDayMorning(&stTime, &t2);
			continue;
		}
		///@note 检查文件头中是否和请求的日期相一致.
		if (isRightDate_Qr(filehead, stTime)==0) {     //这也不算错误,最多算信息.
			fclose(fp);
			timeToNextDayMorning(&stTime, &t2);
			continue;
		}
		int cycle = (filehead.save_cycle_hi*256)
		                +filehead.save_cycle_lo;
		minCycle_t = cycle;
		//stTou at[24 * 60 / cycle];
		int t_mod = t2%(minCycle_t*60);     //向上园整至采样周期.
		if (t_mod!=0) {     //需要园整
			t2 += (minCycle_t*60-t_mod);
		}
		//}
		/**@note 判断开始时间+周期是否跨度到了第二天,如果跨度到第二天则需要
		 打开另一个(下一天)数据文件.
		 */
		stToday_0.tm_hour = 0;
		stToday_0.tm_min = 0;
		stToday_0.tm_sec = 0;
		today_0_t = mktime(&stToday_0);
		if (t2-today_0_t>=(60*60*24)) {     //t2已经时间跨过本日了.次日则文件等等需要重新打开.
			fclose(fp);
			goto Start;
		}
		///移动文件指针,指向开始的数据结构.
		int DeltaSec = t2-today_0_t;     //本采样时刻举今日凌晨几秒
		int NumCycle = DeltaSec/(minCycle_t*60);     //从凌晨开始向后偏移几个采样周期
		int offset = sizeof(stQr)*NumCycle;     //每个样本长度*采样个数
		fseek(fp, offset, SEEK_CUR);     ///当前位置为除去文件头的第一个数据体.

		if (ftell(fp)>=flen) {
			printf(WEBS_WAR"本日的数据不够.filesize=%d,fseek=%ld:%s\n", flen,
			                ftell(fp), file);
			t2 += (minCycle_t*60);
			fclose(fp);
			continue;
		}
		while (ftell(fp)<flen&&t2<=end_t) {
			memset(&qr, 0x0, sizeof(stQr));
			//t_cur += cycle * 60;
			int n = fread(&qr, sizeof(stQr), 1, fp);
			if (n!=1) {
				web_errno = read_tou_file_dat;
				return ERR;
			}
			//成功
			mkOneQrDataRecord(oMtr, t2, qr, i, mtr_no, enable);
			i++;
			t2 += (minCycle_t*60);
		}     // end while 在一个文件中
		fclose(fp);
	}     // end for
	return 0;
}
/**
 * 查找文件,找到所有n条记录
 * @param aInstant
 * @param mtr_no
 * @param range
 * @param enable
 * @return
 */
static int
search_records_instant(jsObj *aInstant, uint32_t mtr_no,
        TimeRange const range, const char *enable)
{
	stInstantFilehead filehead;
	if (range.e<range.s) {
		web_errno = tou_timerange_err;
		return ERR;
	}
	if (range.e==0) {
		web_errno = tou_stime_err;
		return ERR;
	}
	if (range.e==0) {
		web_errno = tou_etime_err;
		return ERR;
	}
	char file[256] = { 0 };
	struct tm stTime;
	struct tm stToday_0;     //今日凌晨00点00分
	time_t today_0_t;
	time_t start_t = range.s;     //开始时刻
	time_t end_t = range.e;     //结束时刻
	time_t t2;     //时刻
	time_t minCycle_t = 0;
	stInstant instant;
	memset(&instant, 0x0, sizeof(stInstant));
	FILE*fp;
	int flen;
	int i = 0;
	//从开始时刻到结束时刻,按分钟遍历,步距为周期,可变.[start,end]两边闭区间
	for (t2 = start_t; t2<=end_t; /*t2 += (mincycle * 60)*/) {
		Start:
		#if __arm__ ==2
		gmtime_r(&t2,&stTime);
		gmtime_r(&t2,&stToday_0);
		//		printf("gmtime_r %02d-%02d %02d:%02d %s stTime.tm_gmtoff=%d \n",
		//				t.tm_mon+1,stTime.tm_mday,stTime.tm_hour,stTime.tm_min,
		//				stTime.tm_zone,stTime.tm_gmtoff);
#else
		localtime_r(&t2, &stTime);
		localtime_r(&t2, &stToday_0);
#endif
		sprintf(file, "%s/mtr%03d%02d%02d.%s", TOU_DAT_DIR, mtr_no, 0,
		                stTime.tm_mday, INSTANT_DAT_SUFFIX);
		fp = fopen(file, "r");
		if (fp==NULL ) {     //这一天没有数据,直接跳到次日零点,这不是错误
			printf(WEBS_INF"%d:%04d-%02d-%02d没有数据文件\n",
			                mtr_no, stTime.tm_year+1900, stTime.tm_mon+1
			                                , stTime.tm_mday);
			web_errno = open_tou_file;
			//到下一天的凌晨,即下一个文件.
			timeToNextDayMorning(&stTime, &t2);
			continue;
		}
		fseek(fp, 0, SEEK_END);
		flen = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		int n = fread(&filehead, sizeof(stInstantFilehead), 1, fp);
		if (n!=1) {
			web_errno = read_tou_file_filehead;
			fclose(fp);
			timeToNextDayMorning(&stTime, &t2);
			continue;
		}
		///@note 检查文件头中是否和请求的日期相一致.
		if (isRightDate_Instant(filehead, stTime)==0) {     //这也不算错误,最多算信息.
			fclose(fp);
			timeToNextDayMorning(&stTime, &t2);
			continue;
		}
		int cycle = (filehead.save_cycle_hi*256)
		                +filehead.save_cycle_lo;
		minCycle_t = cycle;
		//stTou at[24 * 60 / cycle];
		int t_mod = t2%(minCycle_t*60);     //向上园整至采样周期.
		if (t_mod!=0) {     //需要园整
			t2 += (minCycle_t*60-t_mod);
		}
		//}
		/**@note 判断开始时间+周期是否跨度到了第二天,如果跨度到第二天则需要
		 打开另一个(下一天)数据文件.
		 */
		stToday_0.tm_hour = 0;
		stToday_0.tm_min = 0;
		stToday_0.tm_sec = 0;
		today_0_t = mktime(&stToday_0);
		if (t2-today_0_t>=(60*60*24)) {     //t2已经时间跨过本日了.次日则文件等等需要重新打开.
			fclose(fp);
			goto Start;
		}
		///移动文件指针,指向开始的数据结构.
		int DeltaSec = t2-today_0_t;     //本采样时刻举今日凌晨几秒
		int NumCycle = DeltaSec/(minCycle_t*60);     //从凌晨开始向后偏移几个采样周期
		int offset = sizeof(stInstant)*NumCycle;     //每个样本长度*采样个数
		fseek(fp, offset, SEEK_CUR);     ///当前位置为除去文件头的第一个数据体.

		if (ftell(fp)>=flen) {
			printf(WEBS_WAR"本日的数据不够.filesize=%d,fseek=%ld:%s\n",
			                flen, ftell(fp), file);
			t2 += (minCycle_t*60);
			fclose(fp);
			continue;
		}
		while (ftell(fp)<flen&&t2<=end_t) {
			memset(&instant, 0x0, sizeof(stInstant));
			//t_cur += cycle * 60;
			int n = fread(&instant, sizeof(stInstant), 1, fp);
			if (n!=1) {
				web_errno = read_tou_file_dat;
				return ERR;
			}
			//成功
			mkOneInstantDataRecord(aInstant, t2, instant, i, mtr_no, enable);
			i++;
			t2 += (minCycle_t*60);
		}     // end while 在一个文件中
		fclose(fp);
	}     // end for
	return 0;
	return 0;
}

static int
search_records_maxn(jsObj *oMtr, uint32_t mtr_no,
        TimeRange const range, const char *abMaxn)
{
	stMaxnFilehead filehead;
	if (range.e<range.s) {
		web_errno = tou_timerange_err;
		return ERR;
	}
	if (range.e==0) {
		web_errno = tou_stime_err;
		return ERR;
	}
	if (range.e==0) {
		web_errno = tou_etime_err;
		return ERR;
	}
	char file[256] = { 0 };
	struct tm stTime;
	struct tm stToday_0;     //今日凌晨00点00分
	time_t today_0_t;
	time_t start_t = range.s;     //开始时刻
	time_t end_t = range.e;     //结束时刻
	time_t t2;     //时刻
	time_t minCycle_t = 0;
	stMaxn maxn;
	memset(&maxn, 0x0, sizeof(stMaxn));
	FILE*fp;
	int flen;
	int i = 0;
	//从开始时刻到结束时刻,按分钟遍历,步距为周期,可变.[start,end]两边闭区间
	for (t2 = start_t; t2<=end_t; /*t2 += (mincycle * 60)*/) {
		Start:
		#if __arm__ ==2
		gmtime_r(&t2,&stTime);
		gmtime_r(&t2,&stToday_0);
//		printf("gmtime_r %02d-%02d %02d:%02d %s stTime.tm_gmtoff=%d \n",
//				t.tm_mon+1,stTime.tm_mday,stTime.tm_hour,stTime.tm_min,
//				stTime.tm_zone,stTime.tm_gmtoff);
#else
		localtime_r(&t2, &stTime);
		localtime_r(&t2, &stToday_0);
#endif
		sprintf(file, "%s/mtr%03d%02d%02d.%s", TOU_DAT_DIR, mtr_no, 0,
		                stTime.tm_mday, MAXN_DAT_SUFFIX);
		fp = fopen(file, "r");
		if (fp==NULL ) {     //这一天没有数据,直接跳到次日零点,这不是错误
			printf(WEBS_INF"%d:%04d-%02d-%02d没有数据文件\n",
			                mtr_no, stTime.tm_year+1900, stTime.tm_mon+1
			                                , stTime.tm_mday);
			web_errno = open_tou_file;
			//到下一天的凌晨,即下一个文件.
			timeToNextDayMorning(&stTime, &t2);
			continue;
		}
		fseek(fp, 0, SEEK_END);
		flen = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		int n = fread(&filehead, sizeof(stMaxnFilehead), 1, fp);
		if (n!=1) {
			web_errno = read_tou_file_filehead;
			fclose(fp);
			timeToNextDayMorning(&stTime, &t2);
			continue;
		}
		///@note 检查文件头中是否和请求的日期相一致.
		if (isRightDate_Maxn(filehead, stTime)==0) {     //这也不算错误,最多算信息.
			fclose(fp);
			timeToNextDayMorning(&stTime, &t2);
			continue;
		}
		int cycle = (filehead.save_cycle_hi*256)
		                +filehead.save_cycle_lo;
		minCycle_t = cycle;
		//stTou at[24 * 60 / cycle];
		int t_mod = t2%(minCycle_t*60);     //向上园整至采样周期.
		if (t_mod!=0) {     //需要园整
			t2 += (minCycle_t*60-t_mod);
		}
		//}
		/**@note 判断开始时间+周期是否跨度到了第二天,如果跨度到第二天则需要
		 打开另一个(下一天)数据文件.
		 */
		stToday_0.tm_hour = 0;
		stToday_0.tm_min = 0;
		stToday_0.tm_sec = 0;
		today_0_t = mktime(&stToday_0);
		if (t2-today_0_t>=(60*60*24)) {     //t2已经时间跨过本日了.次日则文件等等需要重新打开.
			fclose(fp);
			goto Start;
		}
		///移动文件指针,指向开始的数据结构.
		int DeltaSec = t2-today_0_t;     //本采样时刻举今日凌晨几秒
		int NumCycle = DeltaSec/(minCycle_t*60);     //从凌晨开始向后偏移几个采样周期
		int offset = sizeof(stMaxn_ti)*NumCycle;     //每个样本长度*采样个数
		fseek(fp, offset, SEEK_CUR);     ///当前位置为除去文件头的第一个数据体.

		if (ftell(fp)>=flen) {
			printf(WEBS_WAR"本日的数据不够.filesize=%d,fseek=%ld:%s\n", flen,
			                ftell(fp), file);
			t2 += (minCycle_t*60);
			fclose(fp);
			continue;
		}
		while (ftell(fp)<flen&&t2<=end_t) {
			memset(&maxn, 0x0, sizeof(stMaxn));
			int n = fread(&maxn, sizeof(stMaxn), 1, fp);
			if (n!=1) {
				web_errno = read_tou_file_dat;
				return ERR;
			}
			//成功
			mkOneMaxnDataRecord(oMtr, t2, maxn, i, mtr_no, abMaxn);
			i++;
			t2 += (minCycle_t*60);
		}     // end while 在一个文件中
		fclose(fp);
	}     // end for
	return 0;
}

/**
 * 生成一条电量记录,包括表号序号和电量数据数组.
 * @param OneTimeTou
 * @param t2
 * @param tou
 * @param i
 * @param mtr_no
 * @param enable
 * @return
 */
static int
mkOneTouDataRecord(jsObj *OneTimeTou, time_t t2, const stTou tou,
        int i, int mtr_no, const char *en)
{
	struct tm t;
#if __arm__ ==2
	gmtime_r(&t2,&t);
#else
	localtime_r(&t2, &t);
#endif
	jsObj oneRecord = jsonNewArray();     //一条记录(某个表一个时刻所有电量)
	char tmp[128];
	jsonAdd(&oneRecord, NULL, toStr(tmp, "%d", mtr_no));     //表号
	jsonAdd(&oneRecord, NULL,
	                toStr(tmp, "%04d-%02d-%02d %02d:%02d:%02d",
	                                t.tm_year+1900,
	                                t.tm_mon+1, t.tm_mday, t.tm_hour,
	                                t.tm_min, t.tm_sec, t.tm_zone));
	mkTouDataArray(&oneRecord, tou, en);
	jsonAdd(OneTimeTou, NULL, oneRecord);
	jsonClean(&oneRecord);
	return 0;
}
static int mkOneQrDataRecord(jsObj *OneTimeTou, time_t t2,
        const stQr qr, int i, int mtr_no, const char *en)
{
	struct tm t;
#if __arm__ ==2
	gmtime_r(&t2,&t);
#else
	localtime_r(&t2, &t);
#endif
	jsObj oneRecord = jsonNewArray();     //一条记录(某个表一个时刻所有电量)
	char tmp[128];
	jsonAdd(&oneRecord, NULL, toStr(tmp, "%d", mtr_no));     //表号
	jsonAdd(&oneRecord, NULL,
	                toStr(tmp, "%04d-%02d-%02d %02d:%02d:%02d",
	                                t.tm_year+1900,
	                                t.tm_mon+1, t.tm_mday, t.tm_hour,
	                                t.tm_min, t.tm_sec, t.tm_zone));
	mkQrDataArray(&oneRecord, qr, en);
	jsonAdd(OneTimeTou, NULL, oneRecord);
	jsonClean(&oneRecord);
	return 0;
}
static int mkOneInstantDataRecord(jsObj *aInstantData, time_t t2,
        const stInstant instant, int i, int mtr_no, const char *en)
{
	struct tm t;
#if __arm__ ==2
	gmtime_r(&t2,&t);
#else
	localtime_r(&t2, &t);
#endif
	jsObj oneRecord = jsonNewArray();     //一条记录(某个表一个时刻所有电量)
	char tmp[128];
	jsonAdd(&oneRecord, NULL, toStr(tmp, "%d", mtr_no));     //表号
	jsonAdd(&oneRecord, NULL,
	                toStr(tmp, "%04d-%02d-%02d %02d:%02d:%02d",
	                                t.tm_year+1900,
	                                t.tm_mon+1, t.tm_mday, t.tm_hour,
	                                t.tm_min, t.tm_sec, t.tm_zone));
	mkInstantDataArray(&oneRecord, instant, en);
	jsonAdd(aInstantData, NULL, oneRecord);
	jsonClean(&oneRecord);
	return 0;
}
static int mkOneMaxnDataRecord(jsObj *OneTimeTou, time_t t2,
        const stMaxn maxn, int i, int mtr_no, const char *en)
{
	struct tm t;
#if __arm__ ==2
	gmtime_r(&t2,&t);
#else
	localtime_r(&t2, &t);
#endif
	jsObj oneRecord = jsonNewArray();     //一条记录(某个表一个时刻所有电量)
	char tmp[128];
	jsonAdd(&oneRecord, NULL, toStr(tmp, "%d", mtr_no));     //表号
	jsonAdd(&oneRecord, NULL,
	                toStr(tmp, "%04d-%02d-%02d %02d:%02d:%02d",
	                                t.tm_year+1900,
	                                t.tm_mon+1, t.tm_mday, t.tm_hour,
	                                t.tm_min, t.tm_sec));
	mkMaxnDataArray(&oneRecord, maxn, en);
	jsonAdd(OneTimeTou, NULL, oneRecord);
	jsonClean(&oneRecord);
	return 0;
}
/**
 * 生成一条记录中的所有电量数据数组
 * @param[out] a json格式4*5个电量数据数组
 * @param[in] tou 文件中读取到的数据
 * @param[in] en 根据请求只大发送指定(使能)的某些数据,如正有功总等等
 * @return
 */
static int mkTouDataArray(jsObj *a, const stTou tou, const char *en)
{
	mkOneTouTi(a, tou.FA, &en[0]);     ///正向有功
	mkOneTouTi(a, tou.RA, &en[5]);     ///正向无功
	mkOneTouTi(a, tou.FR, &en[10]);     ///反向有功
	mkOneTouTi(a, tou.RR, &en[15]);     ///反向无功
	return 0;
}
static int mkQrDataArray(jsObj *a, const stQr qr, const char *en)
{
	mkOneQrTi(a, qr.ph1, &en[0]);
	mkOneQrTi(a, qr.ph2, &en[5]);
	mkOneQrTi(a, qr.ph3, &en[10]);
	mkOneQrTi(a, qr.ph4, &en[15]);
	return 0;
}
///生成一条记录中的所有瞬时量数据数组
static int mkInstantDataArray(jsObj *a, const stInstant instant, const char *en)
{
	const int isVI = 0;
	const int isPQ = 1;
	int offset = 0;
	jsObj oitem = jsonNew();
	jsObj aitem = jsonNewArray();
	//v
	mkOneInstantTi(&aitem, isVI, instant.v, en+offset);
	jsonAdd(&oitem, "v", aitem);
	jsonClean(&aitem);
	offset += PHASENUM;
	//i
	mkOneInstantTi(&aitem, isVI, instant.i, en+offset);
	jsonAdd(&oitem, "i", aitem);
	jsonClean(&aitem);
	offset += PHASENUM;
	//p
	mkOneInstantTi(&aitem, isPQ, instant.p, en+offset);
	jsonAdd(&oitem, "p", aitem);
	jsonClean(&aitem);
	offset += PQCNUM;
	//q
	mkOneInstantTi(&aitem, isPQ, instant.q, en+offset);
	jsonAdd(&oitem, "q", aitem);
	jsonClean(&aitem);
	offset += PQCNUM;
	//pf
	mkOneInstantTi(&aitem, isPQ, instant.pf, en+offset);
	jsonAdd(&oitem, "pf", aitem);
	jsonClean(&aitem);
	offset += PQCNUM;
	//ok
	jsonAdd(a, NULL, oitem);
	jsonFree(&oitem);
	jsonFree(&aitem);
	return 0;
}

static int mkMaxnDataArray(jsObj *a, const stMaxn manx, const char *en)
{
	mkOneMaxnTI(a, manx.FA, &en[0]);     ///正向有功
	mkOneMaxnTI(a, manx.RA, &en[5]);     ///正向无功
	mkOneMaxnTI(a, manx.FR, &en[10]);     ///反向有功
	mkOneMaxnTI(a, manx.RR, &en[15]);     ///反向无功
	return 0;
}
time_t ToUnixTimestarmp(stTime st)
{

	if (st.year<=0||st.month<=0||st.day<=0) {
		return 0;
	}
	time_t t;
	struct tm tm;
	tm.tm_sec = 0;     //不是实际秒.和表有关,前端忽略
	tm.tm_min = st.min;
	tm.tm_hour = st.hour;
	tm.tm_mday = st.day;     //标准日 1-31
	tm.tm_mon = st.month-1;     //unix 标准月0~11
	///@note 表计的年可能与表计有关,目前仅忽略之 可能会是@bug !
	tm.tm_year = st.year+100;     //645-97表没有年,
#if 0
	                printf(WEBS_DBG"sys: %04d-%02d-%02d %02d:%02d:%02d \n"
			                , st.year+2000, st.month, st.day
			                , st.hour, st.min, 0);
#endif
	t = mktime(&tm);
#if 0
	printf(WEBS_DBG"sys time(struct): %s\n",asctime(&tm));
	printf(WEBS_DBG"sys time(t): %s\n",ctime(&t));
#endif
	return t;
}
/**
 * 将由4个字节型数组组成的浮点型转化为最短的字符输出.
 * 应为websWrite仅实现了%d和%s,所以必须转化成字符串.
 * @param float_array
 * @param strval
 * @return
 */
static char * float2string(uint8_t const float_array[4], char * strval)
{
#if __arm__ ==1
	/*arm-linux-gcc会优化掉下面的 #else 的方式. - -
	 * 而且是优化掉第二次调用,第一次调用得出的数值还是对的
	 * 所以只能使用这种不优雅的方式防止优化掉强制类型转换
	 */
	typedef union kf {
		char tmp[4];
		float fot;
	}stFloat;
	stFloat f;
	f.tmp[0]=float_array[0];
	f.tmp[1]=float_array[1];
	f.tmp[2]=float_array[2];
	f.tmp[3]=float_array[3];
	sprintf(strval, "%g", f.fot);
#else
	sprintf(strval, "%g", *(float*) (&float_array[0]));
#endif
	return strval;
}
/**
 *
 * @param o
 * @param ti
 * @param enable
 * @return
 */
static int mkOneTouTi(jsObj *a, const touTi_Category ti, const char *en)
{
	char strval[32];
	char tmp[32];
	jsObj aTi = jsonNewArray();
	//printf(WEBS_DBG"使能:%s\n", enable);
	if (en[0]=='1') {
		//printf(WEBS_DBG"总采集\n");
		jsonAdd(&aTi, NULL, float2string(ti.total.fake_float_val, strval));
		jsonAdd(&aTi, NULL, toStr(tmp, "%d", ti.total.iv));
		jsonAdd(a, NULL, aTi);
		jsonClean(&aTi);
	}
	if (en[1]=='1') {
		//printf(WEBS_DBG"尖采集\n");
		jsonAdd(&aTi, NULL, float2string(ti.tip.fake_float_val, strval));
		jsonAdd(&aTi, NULL, toStr(tmp, "%d", ti.tip.iv));
		jsonAdd(a, NULL, aTi);
		jsonClean(&aTi);
	}
	if (en[2]=='1') {
		//printf(WEBS_DBG"峰采集\n");
		jsonAdd(&aTi, NULL, float2string(ti.peak.fake_float_val, strval));
		jsonAdd(&aTi, NULL, toStr(tmp, "%d", ti.peak.iv));
		jsonAdd(a, NULL, aTi);
		jsonClean(&aTi);
	}
	if (en[3]=='1') {
		//printf(WEBS_DBG"平采集\n");
		jsonAdd(&aTi, NULL, float2string(ti.flat.fake_float_val, strval));
		jsonAdd(&aTi, NULL, toStr(tmp, "%d", ti.flat.iv));
		jsonAdd(a, NULL, aTi);
		jsonClean(&aTi);
	}
	if (en[4]=='1') {
		//printf(WEBS_DBG"谷采集\n");
		jsonAdd(&aTi, NULL, float2string(ti.valley.fake_float_val, strval));
		jsonAdd(&aTi, NULL, toStr(tmp, "%d", ti.valley.iv));
		jsonAdd(a, NULL, aTi);
		jsonClean(&aTi);
	}
	jsonFree(&aTi);
	return 0;
}
static int mkOneQrTi(jsObj *a, const touTi_Category ti, const char *en)
{
	char strval[32];
	char tmp[32];
	jsObj aTi = jsonNewArray();
	//printf(WEBS_DBG"使能:%s\n", en);
	if (en[0]=='1') {
		//printf(WEBS_DBG"总采集\n");
		jsonAdd(&aTi, NULL, float2string(ti.total.fake_float_val, strval));
		jsonAdd(&aTi, NULL, toStr(tmp, "%d", ti.total.iv));
		jsonAdd(a, NULL, aTi);
		jsonClean(&aTi);
	}
	if (en[1]=='1') {
		//printf(WEBS_DBG"尖采集\n");
		jsonAdd(&aTi, NULL, float2string(ti.tip.fake_float_val, strval));
		jsonAdd(&aTi, NULL, toStr(tmp, "%d", ti.tip.iv));
		jsonAdd(a, NULL, aTi);
		jsonClean(&aTi);
	}
	if (en[2]=='1') {
		//printf(WEBS_DBG"峰采集\n");
		jsonAdd(&aTi, NULL, float2string(ti.peak.fake_float_val, strval));
		jsonAdd(&aTi, NULL, toStr(tmp, "%d", ti.peak.iv));
		jsonAdd(a, NULL, aTi);
		jsonClean(&aTi);
	}
	if (en[3]=='1') {
		//printf(WEBS_DBG"平采集\n");
		jsonAdd(&aTi, NULL, float2string(ti.flat.fake_float_val, strval));
		jsonAdd(&aTi, NULL, toStr(tmp, "%d", ti.flat.iv));
		jsonAdd(a, NULL, aTi);
		jsonClean(&aTi);
	}
	if (en[4]=='1') {
		//printf(WEBS_DBG"谷采集\n");
		jsonAdd(&aTi, NULL, float2string(ti.valley.fake_float_val, strval));
		jsonAdd(&aTi, NULL, toStr(tmp, "%d", ti.valley.iv));
		jsonAdd(a, NULL, aTi);
		jsonClean(&aTi);
	}
	jsonFree(&aTi);
	return 0;
}
static int mkOneMaxnTI(jsObj *a, const stMaxn_Ti_Category ti, const char *en)
{
	char tmp[32];
	jsObj aTi = jsonNewArray();
	//printf(WEBS_DBG"使能:%s\n", en);
	if (en[0]=='1') {
		//printf(WEBS_DBG"总采集\n");
		jsonAdd(&aTi, NULL,
		                float2string(ti.total.fake_float_val, tmp));
		jsonAdd(&aTi, NULL, toStr(tmp, "%d", ti.total.iv));
		jsonAdd(&aTi, NULL,
		                toStr(tmp, "%d", ToUnixTimestarmp(ti.total.time)));
		jsonAdd(a, NULL, aTi);
		jsonClean(&aTi);
	}
	if (en[1]=='1') {
		//printf(WEBS_DBG"尖采集\n");
		jsonAdd(&aTi, NULL,
		                float2string(ti.tip.fake_float_val, tmp));
		jsonAdd(&aTi, NULL, toStr(tmp, "%d", ti.tip.iv));
		jsonAdd(&aTi, NULL,
		                toStr(tmp, "%d", ToUnixTimestarmp(ti.tip.time)));
		jsonAdd(a, NULL, aTi);
		jsonClean(&aTi);
	}
	if (en[2]=='1') {
		//printf(WEBS_DBG"峰采集\n");
		jsonAdd(&aTi, NULL,
		                float2string(ti.peak.fake_float_val, tmp));
		jsonAdd(&aTi, NULL, toStr(tmp, "%d", ti.peak.iv));
		jsonAdd(&aTi, NULL,
		                toStr(tmp, "%d", ToUnixTimestarmp(ti.peak.time)));
		jsonAdd(a, NULL, aTi);
		jsonClean(&aTi);
	}
	if (en[3]=='1') {
		//printf(WEBS_DBG"平采集\n");
		jsonAdd(&aTi, NULL,
		                float2string(ti.flat.fake_float_val, tmp));
		jsonAdd(&aTi, NULL, toStr(tmp, "%d", ti.flat.iv));
		jsonAdd(&aTi, NULL,
		                toStr(tmp, "%d", ToUnixTimestarmp(ti.flat.time)));
		jsonAdd(a, NULL, aTi);
		jsonClean(&aTi);
	}
	if (en[4]=='1') {
		//printf(WEBS_DBG"谷采集\n");
		jsonAdd(&aTi, NULL,
		                float2string(ti.valley.fake_float_val, tmp));
		jsonAdd(&aTi, NULL, toStr(tmp, "%d", ti.valley.iv));
		jsonAdd(&aTi, NULL,
		                toStr(tmp, "%d", ToUnixTimestarmp(ti.valley.time)));
		jsonAdd(a, NULL, aTi);
		jsonClean(&aTi);
	}
	jsonFree(&aTi);
	return 0;
}
static int mkOneInstantTi(jsObj *a, int isPQ,
        const Ti* ti, const char *en)
{
	char strval[32];
	char tmp[32];
	jsObj aTi = jsonNewArray();
	//printf(WEBS_DBG"使能:%s\n", enable);
	int i;
	int arraylen = (isPQ==1) ? 4 : 3;
	for (i = 0; i<arraylen; i++) {
		if (en[i]=='1') {
			jsonAdd(&aTi, NULL, float2string(ti[i].fake_float_val, strval));
			jsonAdd(&aTi, NULL, toStr(tmp, "%d", ti[i].iv));
			jsonAdd(a, NULL, aTi);
			jsonClean(&aTi);
		}
	}
	jsonFree(&aTi);
	return 0;
}

/**
 * 通过比较文件头中的日期字节和请求的日期,判断是否是正确的日期,没有差几个月
 * @param filehead
 * @param t
 * @return
 */
static int isRightDate(const stTouFilehead filehead, struct tm t)
{
	if (filehead.month!=t.tm_mon+1) {
		return 0;
	}
	if (filehead.year+2000!=t.tm_year+1900) {
		return 0;
	}
	return 1;
}
static int isRightDate_Qr(const stQrFilehead filehead, struct tm t)
{
	if (filehead.month!=t.tm_mon+1) {
		return 0;
	}
	if (filehead.year+2000!=t.tm_year+1900) {
		return 0;
	}
	return 1;
}
/**
 * 通过比较文件头中的日期字节和请求的日期,判断是否是正确的日期,没有差几个月
 * @param filehead
 * @param t
 * @return
 */
static int isRightDate_Instant(const stInstantFilehead filehead, struct tm t)
{
	if (filehead.month!=t.tm_mon+1) {
		return 0;
	}
	if (filehead.year+2000!=t.tm_year+1900) {
		return 0;
	}
	return 1;
}
static int isRightDate_Maxn(const stMaxnFilehead filehead, struct tm t)
{
	if (filehead.month!=t.tm_mon+1) {
		return 0;
	}
	if (filehead.year+2000!=t.tm_year+1900) {
		return 0;
	}
	return 1;
}
/**
 * 将时间推至次日凌晨0点,用于检索到下一个文件
 * @param stTime 时间结构体
 * @param time_t 时间戳(32位!bug Y2038)
 */
static void timeToNextDayMorning(struct tm *stTime, time_t *time_t)
{
	stTime->tm_hour = 0;
	stTime->tm_min = 0;
	stTime->tm_sec = 0;
	*time_t = mktime(stTime);
	*time_t += (60*60*24);
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
