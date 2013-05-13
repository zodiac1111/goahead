/**
 * @file master.c
 * 主站参数,与前端页面对应
 */
#include "master.h"
static int webRece_master(webs_t wp);
static int webSend_master(webs_t wp);
static int addInterFaceList(jsObj *oMaster);
static void print_array(const char *a, const int len);
char *arrayTo2Array(char * a1, int* from, int *to);
int print_stMaster(struct stMasterConfig m_Master);
void form_master(webs_t wp, char_t *path, char_t *query)
{
	PRINT_FORM_INFO;
	websHeader_pure(wp);
	char * action = websGetVar(wp, T("action"), T("null"));
	if (strcmp(action, "get")==0) {
		webRece_master(wp);
	} else if (strcmp(action, "set")==0) {
		webSend_master(wp);
	} else {
		web_err_proc(EL);
	}
	websDone(wp, 200);
	return;

}
static int webRece_master(webs_t wp)
{
	struct stMasterConfig m_Master;
	int i;
	int ret;
	char tmp[128];
	jsObj oMaster = jsonNew();
	jsObj aMasterList = jsonNewArray();
	jsObj oOneParam = jsonNew();
	//web_err_procEx(EL,"读取主站参数");
	//web_err_procEx(EL,"读取主站参数 ret=%d",1);
	addInterFaceList(&oMaster);
	for (i = 0; i<MASTER_INTERFACE_NUM; i++) {
		ret = GetMaster(&m_Master, i);
#if 0
		printf(WEBS_DBG"ret=%d\n", ret);
#endif
		if (ret<0) {
			web_err_proc(EL);
			continue;
		}
#if 0
		printf(WEBS_DBG"interface:%s\n",m_Master.m_interface);
		printf(WEBS_DBG"host:%s\n",m_Master.m_host);
		printf(WEBS_DBG"port:%s\n",m_Master.m_port);
		printf(WEBS_DBG"m_addr:%d\n", m_Master.m_addr);
		printf(WEBS_DBG"m_zone:%d\n", m_Master.m_zone);
		printf(WEBS_DBG"m_hb:%d\n", m_Master.m_hb);
#endif
		jsonAdd(&oOneParam, "interface", m_Master.m_interface);
		jsonAdd(&oOneParam, "host", m_Master.m_host);
		jsonAdd(&oOneParam, "port", m_Master.m_port);
		jsonAdd(&oOneParam, "zone", toStr(tmp, "%d", m_Master.m_zone));
		jsonAdd(&oOneParam, "addr", toStr(tmp, "%d", m_Master.m_addr));
		jsonAdd(&oOneParam, "hb", toStr(tmp, "%d", m_Master.m_hb));
		jsonAdd(&aMasterList, NULL, oOneParam);
		jsonClean(&oOneParam);
	}
	jsonAdd(&oMaster, "master", aMasterList);
	wpsend(wp, oMaster);
	jsonFree(&aMasterList);
	jsonFree(&oOneParam);
	jsonFree(&oMaster);
	return 0;
}
int print_stMaster(struct stMasterConfig m_Master)
{
	printf(WEBS_DBG"结构体 interface=%s host=%s port=%s zone=%d addr=%d hb=%d\n"
	                , m_Master.m_interface, m_Master.m_host
	                , m_Master.m_port, m_Master.m_zone
	                , m_Master.m_addr, m_Master.m_hb);
	return 0;
}
/**
 *  从页面接受参数,调用SetMaster保存
 * @param wp
 * @return
 */
static int webSend_master(webs_t wp)
{
	struct stMasterConfig m_Master;
	int ret, i;
	char tmp_name[32];
	char *insterface;
	char *host;
	char *port;
	char *zone;
	char *addr;
	char *hb;
	for (i = 0; i<MASTER_INTERFACE_NUM; i++) {
		insterface = websGetVar(wp,
		                T(toStr(tmp_name, "%s%d", "interface", i)), T("null"));
		host = websGetVar(wp,
		                T(toStr(tmp_name, "%s%d", "host", i)), T("null"));
		port = websGetVar(wp,
		                T(toStr(tmp_name, "%s%d", "port", i)), T("null"));
		zone = websGetVar(wp,
		                T(toStr(tmp_name, "%s%d", "zone", i)), T("null"));
		addr = websGetVar(wp,
		                T(toStr(tmp_name,"%s%d", "addr", i)), T("null"));
		hb = websGetVar(wp,
		                T(toStr(tmp_name,"%s%d","hb", i)), T("null"));
		strcpy(m_Master.m_interface, insterface);
		strcpy(m_Master.m_host, host);
		strcpy(m_Master.m_port, port);
		m_Master.m_zone = atoi(zone);
		m_Master.m_addr = atoi(addr);
		m_Master.m_hb = atoi(hb);
#if 0
		print_stMaster(m_Master);
#endif
		ret = SetMaster(&m_Master, i);
		if (ret<0) {
			web_err_procEx(EL, "保存主站参数 ret=%d", ret);
			continue;
		}
	}
	return 0;
}
/**
 * 向主站参数对象中添加接口列表,用于网页上选择接口(interface)字符串
 * @param oMaster
 * @return
 */
static int addInterFaceList(jsObj *oMaster)
{
	jsObj aList = jsonNewArray();
	char tmp[1024];
	strcpy(tmp, webs_cfg.master_interface);
	int f = 0, t = 0;
	char *interface = NULL;
	int l = strlen(tmp);
	while ((interface = arrayTo2Array(tmp, &f, &t))!=NULL ) {
		printf(WEBS_DBG"f=%d t=%d\n", f, t);
		f = t;
		jsonAdd(&aList, NULL, interface);
		if (t>=l) {
			break;
		}
	}
	jsonAdd(oMaster, "interface", aList);
	jsonFree(&aList);
	return 0;
}
/**
 * 字符串分割,分割符为空格/逗号/分号
 * @param a1
 * @param from
 * @param to
 * @return
 */
char *arrayTo2Array(char * a1, int* from, int *to)
{
	if (a1==NULL ) {
		return a1;
	}
	int i;
	for (i = *from;; i++) {
		if (a1[i]==','||a1[i]==' '||a1[i]==';'||a1[i]=='\0') {
			*to = i+1;
			a1[i] = '\0';
			return a1+*from;
		}
	}
	return NULL ;
}
///打印数组
static void print_array(const char *a, const int len)
{
	int i;
	printf("[%d] ", len);
	for (i = 0; i<len; i++) {
		printf("%02X ", a[i]);
	}
	printf("\n");
	return;
}
