/**
 * @file commModule.c
 * 通讯模块后端代码,与前台页面对应
 */
#include "commModule.h"
#define CONF_LINE_MAX_CHAR 256 //一行最大字符
static int sentParam(webs_t wp);
static int addApnList(jsObj* oCommModule);
static int addItems(jsObj* oCommModule);
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
	addItems(&oCommModule);
	wpsend(wp, oCommModule);
	jsonFree(&oCommModule);
	return 0;
}
static int addApnList(jsObj* oCommModule)
{
	jsObj aApnList = jsonNewArray();
	char line[CONF_LINE_MAX_CHAR];
	int strnum;
	char n[256] = { 0 };

	FILE* fp = fopen(webs_cfg.apnList, "r");
	if (fp==NULL ) {
		web_err_procEx(EL, "读取apn列表");
		return -1;
	}
	while (!feof(fp)) {
		fgets(line, CONF_LINE_MAX_CHAR-1, fp);     //得到一行
		strnum = sscanf(line, "%[^#\\r\\n]", n);     //去掉换行符
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
static int addItems(jsObj* oCommModule)
{
	jsObj aItems = jsonNewArray();
	jsObj oItem = jsonNew();
	char line[CONF_LINE_MAX_CHAR];
	int strnum;
	char n[256] = { 0 };
	char v[256] = { 0 };
	FILE* fp = fopen(webs_cfg.commModule, "r");
	if (fp==NULL ) {
		web_err_procEx(EL, "读取apn列表");
		return -1;
	}
	while (!feof(fp)) {
		fgets(line, CONF_LINE_MAX_CHAR-1, fp);     //得到一行
		//去掉换行符,分解成为 name:value 的形式
		strnum = sscanf(line, "%[^#:]:%[^#\r\n]", n, v);
		if (strnum!=2) {
			continue;
		}
		jsonAdd(&oItem, n, v);
		jsonAdd(&aItems, NULL, oItem);
		jsonClean(&oItem);
	}
	fclose(fp);
	jsonAdd(oCommModule, "items", aItems);
	jsonFree(&aItems);
	jsonFree(&oItem);
	return 0;
}
