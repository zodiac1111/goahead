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
static int parse_item(uint8_t* bMtr, uint8_t* bTou,webs_t wp);
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
///实时电量查询,json格式参考doc/json-date-example.md 实时数据-电量
static int webRece_realtime_tou(webs_t wp)
{
	uint8_t bMtr[MAXMETER];
	uint8_t bTou[TOUNUM];
	struct stMeter_Run_data *mtr;
	char tmpstr[128];
	int i,j;
	parse_item(bMtr,bTou,wp);
	char * mtrArray = websGetVar(wp, T("mtrArray"), T("0"));
	char * itemArray = websGetVar(wp, T("itemArray"), T("0"));
	int mtrnum=strlen(mtrArray);
	int itemnum=strlen(itemArray);
	check_mtrnum(mtrnum);
	check_itemnum(itemnum);
	mtr =GetMeterAll_p();
	if(mtr==NULL){
		web_err_proc(EL);
	}
	jsObj oTou=jsonNew();//{[{表1},{表2}],终端属性=xx,终端属性2=yy}
	jsObj aMtr=jsonNewArray();//{[表0,表1...],表属性1=aa,表属性2=bb}
	jsObj oMtr=jsonNew(); //{[it],表属性}
	jsObj aIt=jsonNewArray(); //[正有,反有...四象限无功]
	jsObj aOneDate=jsonNewArray();//一个电量数据=[电量,有效位...]
	uint8_t iv;
	for(i=0;i<mtrnum;i++){//遍历所有表
		if(mtrArray[i]!='1'){
			continue;
		}
		printf("-无效标识tou:0x%04X qr 0x%04X 抄表时间:%ld,\n"
			,mtr[i].Flag_TOU,mtr[i].Flag_QR,mtr[i].Meter_ReadTime);
		for(j=0;j<TOUNUM;j++){//电量
			if(itemArray[j]=='1'){
				jsonAdd(&aOneDate,NULL,
					toStr(tmpstr,"%g",mtr[i].m_iTOU[j]));
				iv = (mtr[i].Flag_TOU&(0x1<<j))>>j;
				jsonAdd(&aOneDate,NULL,
					toStr(tmpstr,"%u",iv));
				jsonAdd(&aIt,NULL,aOneDate);//电量
				jsonClear(&aOneDate);
#if DEBUG_PRINT_REALTIME_TOU_DAT
				//printf("--aOneDate(TOUNUM) %s\n",aOneDate);
				//printf("--aOneMtr(TOUNUM) %s\n",oOneMtr);
#endif
			}
		}
		for(j=TOUNUM;j<itemnum;j++){//象限无功
			if(itemArray[j]=='1'){
				int index=j-TOUNUM;
				jsonAdd(&aOneDate,NULL,
					toStr(tmpstr,"%g",mtr[i].m_iQR[index]));
				iv = (mtr[i].Flag_QR&(0x1<<index))>>index;
				jsonAdd(&aOneDate,NULL,
					toStr(tmpstr,"%u",iv));
				jsonAdd(&aIt,NULL,aOneDate);
				jsonClear(&aOneDate);
#if DEBUG_PRINT_REALTIME_TOU_DAT
				//printf("--aOneDate(itemnum) %s\n",aOneDate);
				//printf("--aOneMtr(itemnum) %s\n",oOneMtr);
#endif
			}
		}
		jsonAdd(&oMtr,"it",aIt);
		//抄表时刻
		jsonAdd(&oMtr,"Meter_ReadTime"
			,toStr(tmpstr,"%ld",mtr[i].Meter_ReadTime));
		jsonAdd(&aMtr,NULL,oMtr);
		jsonClear(&aIt);//循环利用
		jsonClear(&oMtr);
	}
	jsonAdd(&oTou,"rtu-info","this is rtu info,like version etc..");
	jsonAdd(&oTou,"mtr_selected",mtrArray);
	jsonAdd(&oTou,"item_selected",itemArray);
	jsonAdd(&oTou,"mtr",aMtr);
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
		web_err_proc(EL);
		return -1000;
	}
	if(mtrnum>MAXMETER){
		web_errno=eno_realtime_tou_mtrnum_too_big;
		web_err_proc(EL);
		return -1002;
	}
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
static int parse_item(uint8_t* bMtr, uint8_t* bTou,webs_t wp)
{
	char * mtr = websGetVar(wp, T("mtrArray"), T("null"));
	char * tou = websGetVar(wp, T("itemArray"), T("null"));
	return 0;
}
