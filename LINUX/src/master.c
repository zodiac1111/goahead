/**
 * @file master.c
 * 主站参数,与前端页面对应
 */
#include "master.h"
static int webRece_master(webs_t wp);
static int webSend_master(webs_t wp);
static void print_array(const char *a, const int len);
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
	struct stMasterConfig m_Master ;
	int i;
	int ret;
	char tmp[128];
	jsObj oMaster=jsonNew();
	jsObj aMasterList=jsonNewArray();
	jsObj oOneParam=jsonNew();
	for (i=0;i<4;i++){
		ret = GetMaster(&m_Master, i);
		printf(WEBS_DBG"ret=%d\n", ret);
		if(ret<0){
			web_err_proc(EL);
			continue;
		}
		printf(WEBS_DBG"interface:%s\n",m_Master.m_interface);
		printf(WEBS_DBG"host:%s\n",m_Master.m_host);
		printf(WEBS_DBG"port:%s\n",m_Master.m_port);
		printf(WEBS_DBG"m_addr:%d\n", m_Master.m_addr);
		printf(WEBS_DBG"m_zone:%d\n", m_Master.m_zone);
		printf(WEBS_DBG"m_hb:%d\n", m_Master.m_hb);
		jsonAdd(&oOneParam,"interface",m_Master.m_interface);
		jsonAdd(&oOneParam,"host",m_Master.m_host);
		jsonAdd(&oOneParam,"port",m_Master.m_port);
		jsonAdd(&oOneParam,"addr",toStr(tmp,"%d",m_Master.m_addr));
		jsonAdd(&oOneParam,"zone",toStr(tmp,"%d",m_Master.m_zone));
		jsonAdd(&oOneParam,"hb",toStr(tmp,"%d",m_Master.m_hb));
		jsonAdd(&aMasterList,NULL,oOneParam);
		jsonClean(&oOneParam);
	}
	jsonAdd(&oMaster,"master",aMasterList);
	wpsend(wp,oMaster);
	jsonFree(&aMasterList);
	jsonFree(&oOneParam);
	jsonFree(&oMaster);
	return 0;
}
static int webSend_master(webs_t wp)
{
	return 0;
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
