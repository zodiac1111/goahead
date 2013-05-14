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
static int addApnList(jsObj* oCommModule);
static int addHbcyList(jsObj* oCommModule);
static int addItems(jsObj* oCommModule);
static int addStatus(jsObj* oCommModule);
static char* toStatusStr(char *tmp, uint8_t Status);
void form_commModule(webs_t wp, char_t *path, char_t *query)
{
	PRINT_FORM_INFO;
	websHeader_pure(wp);
	char * action = websGetVar(wp, T("action"), T("null"));
	if (strcmp(action, "get")==0) {
		sentParam(wp);
	} else if (strcmp(action, "set")==0) {
		//reciParam(wp);
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
static int addApnList(jsObj* oCommModule)
{
	jsObj aApnList = jsonNewArray();
	char line[CONF_LINE_MAX_CHAR];
	int strnum;
	char n[CONF_LINE_MAX_CHAR] = { 0 };
	FILE* fp = fopen(webs_cfg.apnList, "r");
	if (fp==NULL ) {
		web_err_procEx(EL, "读取apn列表 filename=%s", webs_cfg.apnList);
		return -1;
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
static int addItems(jsObj* oCommModule)
{
	char line[CONF_LINE_MAX_CHAR];
	int strnum;
	char n[CONF_LINE_MAX_CHAR] = { 0 };
	char v[CONF_LINE_MAX_CHAR] = { 0 };
	FILE* fp = fopen(webs_cfg.commModule, "r");
	if (fp==NULL ) {
		web_err_procEx(EL, "读取通信模块项目 filename=%s", webs_cfg.commModule);
		return -1;
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
static int addStatus(jsObj* oCommModule)
{

	jsObj oStatus = jsonNew();
	char tmp[32];
	unsigned char* ret;
	uint32_t ip = GetInterfaceIpC("ppp0");
	jsonAdd(&oStatus, "ip", toStr(tmp, "%d.%d.%d.%d"
			, (ip&0xf000)>>24,(ip&0x0f00)>>16,(ip&0x00f0)>>8,(ip&0x000f)>>0));
	ret = GetGprsSig();
	printf("GetGprsSig ret=%d\n", *ret);
	jsonAdd(&oStatus, "sig", toStr(tmp, "%d", *ret));
	jsonAdd(&oStatus, "sig_str", toStr(tmp, "%d%%", (*ret+1)/32*100));
	ret = GetGrpsStatus();
	printf("GetGrpsStatus ret=%d\n", *ret);
	jsonAdd(&oStatus, "stat", toStr(tmp, "%d", *ret));
	jsonAdd(&oStatus, "stat_str", toStatusStr(tmp, *ret));
	jsonAdd(oCommModule, "status", oStatus);
	jsonFree(&oStatus);
	return 0;
}
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
		sprintf(tmp, "%s", "SIM");
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
	if (Status==6){
		sprintf(tmp, "%s", "成功");
	}
	/*
	if (Status&0x80) {
		sprintf(tmp, "%s", "失败");
	} else {
		sprintf(tmp, "%s", "成功");
	}*/
	return tmp;
}
#ifdef __cplusplus
} /* End of 'C' functions       */
#endif
