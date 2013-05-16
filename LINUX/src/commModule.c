/**
 * @file commModule.c
 * 通讯模块后端代码,与前台页面对应
 */
#include "commModule.h"
#define CONF_LINE_MAX_CHAR 256 //一行最大字符
#define HB_STR_CONTER 5 //心跳类型个数
#ifdef __cplusplus
extern "C" {
#endif
const char * const hb_str[HB_STR_CONTER] = { "IEC102 Heart", "Zj Heart",
                "Gw Heart", "Gw Heart", "Gx Heart" };
static int sentParam(webs_t wp);
static int reciParam(webs_t wp);
static int addApnList(jsObj* oCommModule);
static int addHbcyList(jsObj* oCommModule);
static int addItems(jsObj* oCommModule);
static int addStatus(jsObj* oCommModule);
static int saveItems(webs_t wp);
static char* toStatusStr(char *tmp, uint8_t Status);
void form_commModule(webs_t wp, char_t *path, char_t *query)
{
	PRINT_FORM_INFO;
	websHeader_pure(wp);
	char * action = websGetVar(wp, T("action"), T("null"));
	if (strcmp(action, "get")==0) {
		sentParam(wp);
	} else if (strcmp(action, "set")==0) {
		reciParam(wp);
	} else {
		web_err_proc(EL);
	}
	websDone(wp, 200);
	return;
}
/**
 * 回应前端请求通信模块参数设置.
 * 从文件中读取参数 +
 * 从externCShm库中调用信号和状态 =>
 * 发送参数给前端(通过json格式数据)
 * @param wp
 * @return
 */
static int sentParam(webs_t wp)
{
	jsObj oCommModule = jsonNew();
	addApnList(&oCommModule);
	addHbcyList(&oCommModule);
	addItems(&oCommModule);
	addStatus(&oCommModule);
	wpsend(wp, oCommModule);
	jsonFree(&oCommModule);
	return 0;
}
/**
 * 从前端接受要设置的参数,保存到终端
 * @param wp
 * @return
 */
static int reciParam(webs_t wp)
{
	jsObj oCommModule = jsonNew();
	if (saveItems(wp)==0) {
		jsonAdd(&oCommModule, "ret", "0");
	} else {
		jsonAdd(&oCommModule, "ret", "-1");
		jsonAdd(&oCommModule, "ret_str", "失败");
	}
	wpsend(wp, oCommModule);
	jsonFree(&oCommModule);
	return 0;
}
/**
 * apn列表
 * @param oCommModule
 * @return
 */
static int addApnList(jsObj* oCommModule)
{
	jsObj aApnList = jsonNewArray();
	char line[CONF_LINE_MAX_CHAR];
	int strnum;
	char n[CONF_LINE_MAX_CHAR] = { 0 };
	FILE* fp = fopen(webs_cfg.apnList, "r");
	if (fp==NULL ) {
		web_err_procEx(EL, "读取apn列表 filename=%s", webs_cfg.apnList);
		jsonAdd(&aApnList, NULL, "没有Apns.txt文件");
		goto LOAD_DEFAULT_PARAM;
	}
	while (!feof(fp)) {
		fgets(line, CONF_LINE_MAX_CHAR-1, fp);     //得到一行
		strnum = sscanf(line, "%[^#\r\n]", n);     //去掉换行符
		if (strnum!=1) {
			web_err_procEx(EL, "读取一行apn strnum=%d", strnum);
			continue;
		}
		jsonAdd(&aApnList, NULL, n);
	}
	fclose(fp);
	LOAD_DEFAULT_PARAM:
	jsonAdd(oCommModule, "apn_list", aApnList);
	jsonFree(&aApnList);
	return 0;
}
/**
 * 心跳类型列表
 * @param oCommModule
 * @return
 */
static int addHbcyList(jsObj* oCommModule)
{
	jsObj aHbcy = jsonNewArray();
	int i = 0;
	for (i = 0; i<HB_STR_CONTER; i++) {
		jsonAdd(&aHbcy, NULL, hb_str[i]);
	}
	jsonAdd(oCommModule, "hbcy_list", aHbcy);
	jsonFree(&aHbcy);
	return 0;
}
/**
 * 项目
 * @param oCommModule
 * @return
 */
static int addItems(jsObj* oCommModule)
{
	char line[CONF_LINE_MAX_CHAR];
	int strnum;
	char n[CONF_LINE_MAX_CHAR] = { 0 };
	char v[CONF_LINE_MAX_CHAR] = { 0 };
	FILE* fp = fopen(webs_cfg.commModule, "r");
	if (fp==NULL ) {
		web_err_procEx(EL, "读取通信模块项目 filename=%s", webs_cfg.commModule);
		//使用默认设置
		jsonAdd(oCommModule, "simc", "##默认参数##");
		jsonAdd(oCommModule, "mode", "client");
		jsonAdd(oCommModule, "apn", "CMNET");
		jsonAdd(oCommModule, "pitc", "0");
		jsonAdd(oCommModule, "hbcy", "5");
		return 0;
	}
	while (!feof(fp)) {
		fgets(line, CONF_LINE_MAX_CHAR-1, fp);     //得到一行
		//去掉换行符,分解成为 name:value 的形式
		strnum = sscanf(line, "%[^#:]:%[^#\r\n]", n, v);
		if (strnum!=2) {
			continue;
		}
		jsonAdd(oCommModule, n, v);
	}
	fclose(fp);
	return 0;
}
/**
 * 返回状态
 * @param oCommModule
 * @return
 */
static int addStatus(jsObj* oCommModule)
{
	jsObj oStatus = jsonNew();
	char tmp[32];
	unsigned char* ret;
	union ip {
		uint32_t val;
		uint8_t byte[4];
	} Ip;
	///1 gprs状态
	ret = GetGrpsStatus();
#if 0
	printf("GetGrpsStatus ret=%d\n", *ret);
#endif
	jsonAdd(&oStatus, "stat", toStr(tmp, "0x%02X", *ret));
	jsonAdd(&oStatus, "stat_str", toStatusStr(tmp, *ret));
	///2 信号强度
	ret = GetGprsSig();
#if 0
	printf("GetGprsSig ret=%d\n", *ret);
#endif
	jsonAdd(&oStatus, "sig", toStr(tmp, "%d", *ret));
	jsonAdd(&oStatus, "sig_str", toStr(tmp, "%d%%", (*ret+1)/32*100));
	///3 ip address @bug @note ONLY ipv4
	Ip.val = GetInterfaceIpC("ppp0");
#if 0
	printf(WEBS_DBG"ip=%X\n", Ip.val);
	printf(WEBS_DBG"%d %d %d %d \n",Ip.byte[0],Ip.byte[1],Ip.byte[2],Ip.byte[3]);
#endif
	jsonAdd(&oStatus, "ip", toStr(tmp, "%d.%d.%d.%d"
	                , Ip.byte[0], Ip.byte[1], Ip.byte[2], Ip.byte[3]));
	///完成 结束
	jsonAdd(oCommModule, "status", oStatus);
	jsonFree(&oStatus);
	return 0;
}
/// 状态量=>状态文字说明
static char* toStatusStr(char *tmp, uint8_t Status)
{
	switch (Status&0xf)
	{
	case 0:
		sprintf(tmp, "%s", "off ");
		break;
	case 2:
		sprintf(tmp, "%s", "Sea ");
		break;
	case 3:
		sprintf(tmp, "%s", "SIM ");
		break;
	case 5:
		sprintf(tmp, "%s", "Reg ");
		break;
	case 6:
		sprintf(tmp, "%s", "Dial ");
		break;
	default:
		sprintf(tmp, "%s", " ");
		break;
	}
	if (Status&0x80) {
		strcat(tmp, "Fail");
	} else {
		strcat(tmp, "Succ");
	}
	//明确的2种状态,其他状态都是中间(过渡)状态
	if (Status==6) {
		sprintf(tmp, "%s", "在线");
	} else if (Status==0) {
		sprintf(tmp, "%s", "离线");
	}else{
		strcat(tmp, " 离线");
	}
	/*
	 if (Status&0x80) {
	 sprintf(tmp, "%s", "失败");
	 } else {
	 sprintf(tmp, "%s", "成功");
	 }*/
	return tmp;
}
static int saveItems(webs_t wp)
{
	//char tmp[8];
	FILE* fp = fopen(webs_cfg.commModule, "w");		//全文件重写
	if (fp==NULL ) {
		web_err_procEx(EL, "写入通信模块项目 filename=%s", webs_cfg.commModule);
		return -1;
	}
	char *simc = websGetVar(wp, T("simc"), T("null"));
	char *mode = websGetVar(wp, T("mode"), T("null"));
	char *apn = websGetVar(wp, T("apn"), T("null"));
	char *pitc = websGetVar(wp, T("pitc"), T("null"));
	char *hbcy = websGetVar(wp, T("hbcy"), T("null"));
	if (fprintf(fp, "%s:%s\n", "simc", simc)<0) {
		web_err_proc(EL);
	}
	if (fprintf(fp, "%s:%s\n", "mode", mode)<0) {
		web_err_proc(EL);
	}
	if (fprintf(fp, "%s:%s\n", "apn", apn)<0) {
		web_err_proc(EL);
	}
	if (fprintf(fp, "%s:%d\n", "pitc", atoi(pitc))<0) {
		web_err_proc(EL);
	}
	if (fprintf(fp, "%s:%d\n", "hbcy", atoi(hbcy))<0) {
		web_err_proc(EL);
	}
	fclose(fp);
	return 0;
}
#ifdef __cplusplus
} /* End of 'C' functions       */
#endif
