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
int add_mtr_tou(jsObj *oMtr,struct stMeter_Run_data const mtr,char const *abTou)
{
	int j;
	char tmpstr[128];
	uint8_t iv;
	jsObj aOneDate=jsonNewArray();//一个电量数据=[电量,有效位...]
	jsObj aTou=jsonNewArray(); //[正有,反有,正无,反无]
	for(j=0;j<TOUNUM;j++){//电量 总尖峰平谷
		if(abTou[j]=='1'){
			jsonAdd(&aOneDate,NULL,
				toStr(tmpstr,"%g",mtr.m_iTOU[j]));
			iv = (mtr.Flag_TOU&(0x1<<j))>>j;
			jsonAdd(&aOneDate,NULL,
				toStr(tmpstr,"%u",iv));
			jsonAdd(&aTou,NULL,aOneDate);//电量
			jsonClear(&aOneDate);
#if DEBUG_PRINT_REALTIME_TOU_DAT
				printf("--aOneDate(TOUNUM) %s\n",aOneDate);
				printf("--aOneMtr(TOUNUM) %s\n",aIt);
#endif
		}
	}
	jsonAdd(oMtr,"tou",aTou);
	printf(WEBS_DBG"tou:%s\n",*oMtr);
	jsonFree(&aOneDate);
	jsonFree(&aTou);
	return 0;
}
//向一个表(mtr)中添加it对象,指示这个表中所有tou实时电量数据.
int add_mtr_qr(jsObj *oMtr,struct stMeter_Run_data const mtr,char const *abTou)
{
	int j;
	char tmpstr[128];
	uint8_t iv;
	jsObj aOneDate=jsonNewArray();//一个电量数据=[电量,有效位...]
	jsObj aTou=jsonNewArray(); //[正有,反有,正无,反无]
	for(j=0;j<TOUNUM;j++){//电量 总尖峰平谷
		if(abTou[j]=='1'){
			jsonAdd(&aOneDate,NULL,
				toStr(tmpstr,"%g",mtr.m_iQR[j]));
			iv = (mtr.Flag_QR&(0x1<<j))>>j;
			jsonAdd(&aOneDate,NULL,
				toStr(tmpstr,"%u",iv));
			jsonAdd(&aTou,NULL,aOneDate);//电量
			jsonClear(&aOneDate);
#if DEBUG_PRINT_REALTIME_TOU_DAT
				printf("--aOneDate(TOUNUM) %s\n",aOneDate);
				printf("--aOneMtr(TOUNUM) %s\n",aIt);
#endif
		}
	}
	jsonAdd(oMtr,"qr",aTou);
	printf(WEBS_DBG"qr:%s\n",*oMtr);
	jsonFree(&aOneDate);
	jsonFree(&aTou);
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
	char * abMtr = websGetVar(wp, T("abMtr"), T(""));
	char * abTou = websGetVar(wp, T("tou"), T(""));
	char * abQr = websGetVar(wp, T("qr"), T(""));
	char * abV = websGetVar(wp, T("v"), T(""));
	char * abI = websGetVar(wp, T("i"), T(""));
	char * abP = websGetVar(wp, T("p"), T(""));
	char * abQ = websGetVar(wp, T("q"), T(""));
	char * abPF = websGetVar(wp, T("pf"), T(""));
	char * abF = websGetVar(wp, T("f"), T(""));
	int mtrnum=strlen(abMtr);
	int itemnum=strlen(abTou);
	if(check_mtrnum(mtrnum)<0){
		web_err_proc(EL);
	}
	check_itemnum(itemnum);
	mtr =GetMeterAll_p();
	if(mtr==NULL){
		web_err_proc(EL);
	}
	jsObj oTou=jsonNew();//{[{表1},{表2}],终端属性=xx,终端属性2=yy}
	jsObj aMtr=jsonNewArray();//{[表0,表1...]}
	jsObj oMtr=jsonNew(); //{[it],表属性}
	jsObj aIt=jsonNewArray(); //[正有,反有...四象限无功]
	jsObj aOneDate=jsonNewArray();//一个电量数据=[电量,有效位...]
	jsonAdd(&oTou,"rtu-info","this is rtu info,like version etc..");
	jsonAdd(&oTou,"abMtr",abMtr);
	jsonAdd(&oTou,"abTou",abTou);
	jsonAdd(&oTou,"abQr",abQr);
	jsonAdd(&oTou,"abV",abV);
	jsonAdd(&oTou,"abI",abI);
	jsonAdd(&oTou,"abP",abP);
	jsonAdd(&oTou,"abQ",abQ);
	jsonAdd(&oTou,"abPF",abPF);
	jsonAdd(&oTou,"abF",abF);
	for(i=0;i<mtrnum;i++){//遍历所有表
		if(abMtr[i]!='1'){
			continue;
		}
#if 0
#if DEBUG_PRINT_REALTIME_TOU_DAT
		printf("-无效标识tou:0x%04X qr 0x%04X 抄表时间:%ld,\n"
			,mtr[i].Flag_TOU,mtr[i].Flag_QR,mtr[i].Meter_ReadTime);
#endif
		for(j=0;j<TOUNUM;j++){//电量
			if(abTou[j]=='1'){
				jsonAdd(&aOneDate,NULL,
					toStr(tmpstr,"%g",mtr[i].m_iTOU[j]));
				iv = (mtr[i].Flag_TOU&(0x1<<j))>>j;
				jsonAdd(&aOneDate,NULL,
					toStr(tmpstr,"%u",iv));
				jsonAdd(&aIt,NULL,aOneDate);//电量
				jsonClear(&aOneDate);
#if DEBUG_PRINT_REALTIME_TOU_DAT
				printf("--aOneDate(TOUNUM) %s\n",aOneDate);
				printf("--aOneMtr(TOUNUM) %s\n",aIt);
#endif
			}
		}

		for(j=TOUNUM;j<itemnum;j++){//象限无功
			if(abTou[j]=='1'){
				int index=j-TOUNUM;
				jsonAdd(&aOneDate,NULL,
					toStr(tmpstr,"%g",mtr[i].m_iQR[index]));
				iv = (mtr[i].Flag_QR&(0x1<<index))>>index;
				jsonAdd(&aOneDate,NULL,
					toStr(tmpstr,"%u",iv));
				jsonAdd(&aIt,NULL,aOneDate);
				jsonClear(&aOneDate);
#if DEBUG_PRINT_REALTIME_TOU_DAT
				printf("--aOneDate(itemnum) %s\n",aOneDate);
				printf("--aOneMtr(itemnum) %s\n",aIt);
#endif
			}
		}
#else
		add_mtr_tou(&oMtr,mtr[i],abTou);
		add_mtr_qr(&oMtr,mtr[i],abQr);
#endif
		//jsonAdd(&oMtr,"it",aIt);
		//抄表时刻
		jsonAdd(&oMtr,"Meter_ReadTime"
			,toStr(tmpstr,"%ld",mtr[i].Meter_ReadTime));
		jsonAdd(&aMtr,NULL,oMtr);
		jsonClear(&aIt);//循环利用
		jsonClear(&oMtr);
	}

	jsonAdd(&oTou,"mtr",aMtr); //回传输入的参数,备用
#if DEBUG_PRINT_REALTIME_TOU_DAT
		printf("-oTou itemnum %s\n",oTou);
#endif
	wpsend(wp,oTou);
	jsonFree(&aOneDate);
	jsonFree(&aIt);
	jsonFree(&aMtr);
	jsonFree(&oTou);
	return 0;
}
///简单检查项目数量 应该为[4(象限)+4(正反有无)]*5(总尖峰平谷),共40个项目(0或1)
static int check_itemnum(int itemnum)
{
	if(itemnum!=(TOUNUM+TOUNUM)){
		web_err_proc(EL);
		return -1;
	}
	return 0;
}
///简单检查表数量合法性
static int check_mtrnum(int mtrnum)
{
	if(mtrnum<=0){
		web_errno=eno_realtime_tou_mtrnum_too_small;
		//web_err_proc(EL);
		return -1000;
	}
	if(mtrnum>MAXMETER){
		web_errno=eno_realtime_tou_mtrnum_too_big;
		//web_err_proc(EL);
		return -1002;
	}
	return 0;
}
