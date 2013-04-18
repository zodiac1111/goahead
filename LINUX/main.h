/**
 * @file main.h
 * 头文件.
 *  Created on: 2012-12-19
 *      Author: lee
 */
#ifndef MAIN_H_
#define MAIN_H_
#include "../uemf.h"
#include "../wsIntrn.h"
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdint.h>
#include "tou.h"
#include "conf.h"
#include "json.h"
#include "web_err.h"
#include "autoUpdate.h"
#define MTR_ADD 1 //增删查改?...
#define MTR_DEL  2
#define MTR_SEARCH  3
#define MTR_UPDATE  4
#define MAX_MTR_NUM 128 ///<最大表计个数
//提交表单函数定义
void form_server_time(webs_t wp, char_t *path, char_t *query);
void form_sysparam(webs_t wp, char_t *path, char_t *query);
void form_mtrparams(webs_t wp, char_t *path, char_t *query);
void form_mtr_items(webs_t wp, char_t *path, char_t *query);
void form_sioplans(webs_t wp, char_t *path, char_t *query);
void form_netparas(webs_t wp, char_t *path, char_t *query);
void form_monparas(webs_t wp, char_t *path, char_t *query);
void form_savecycle(webs_t wp, char_t *path, char_t *query);
void form_collect_cycle(webs_t wp, char_t *path, char_t *query);
void form_history_tou(webs_t wp, char_t *path, char_t *query);
void form_sysFunction(webs_t wp, char_t *path, char_t *query);
void form_msg(webs_t wp, char_t *path, char_t *query);
void form_msg_stop(webs_t wp, char_t *path, char_t *query);
void form_load_procotol_cfgfile(webs_t wp, char_t *path, char_t *query);
void form_save_procotol_cfgfile(webs_t wp, char_t *path, char_t *query);
void form_save_log(webs_t wp, char_t *path, char_t *query);
void form_load_log(webs_t wp, char_t *path, char_t *query);
void form_load_monport_cfgfile(webs_t wp, char_t *path, char_t *query);
void form_save_monport_cfgfile(webs_t wp, char_t *path, char_t *query);
void form_info(webs_t wp, char_t *path, char_t *query);
void form_upload_file(webs_t wp, char_t *path, char_t *query);
//各类大相
int webRece_syspara(webs_t wp,stSysParam* sysparam);
int webSend_syspara(webs_t wp);
int webRece_syntime(webs_t wp);
int webSend_mtr_sioplan(webs_t wp,stSysParam sysparam);
int webSend_mtr_procotol(webs_t wp);
int webSend_mtr_factory(webs_t wp);
//int webSend_mtr_type(webs_t wp);
int webSend_savecycle(webs_t wp);
int webSend_collect_cycle(webs_t wp);
int webRece_collect_cycle(webs_t wp);
int webRece_savecycle(webs_t wp);
int webRece_mtrparams(webs_t wp);
int webSend_mtrparams(webs_t wp, int mtrnum);
int webSend_sioplans(webs_t wp, stSysParam sp);
int webRece_sioplans(webs_t wp);
int webSend_netparas(webs_t wp, int netParamNum);
int webRece_netparas(webs_t wp);
int webSend_monparas(webs_t wp, stSysParam sysparam);
int webRece_monparas(webs_t wp);
int webSend_info(webs_t wp);
int webSend_txtfile(webs_t wp, const char*file);
int webRece_txtfile(webs_t wp, char_t *query, const char* file);

static int getmtrparams(stMtr amtr[MAX_MTR_NUM], webs_t wp, uint32_t e[MAX_MTR_NUM]);
//串口方案项目
//网口参数
static int webWrite_ip(webs_t wp, char *name, uint8_t* value);
//监视端口参数
char* webrite_listen_port(char* tmp, const stMonparam monport);
char* webWrite_rtu_addr(char* tmp, stMonparam monport);
//存储周期
char *addItem(char **oItem,stSave_cycle sav);
//采集周期
char *addCollectItem(char **oItem, stCollect_cycle sav);
///其他
static int split(char **ret, char* in);
static int is_all_equ(int n[], int num);
int portstr2u8(const char * str, uint8_t* val);
int listen_port_str2array(const char* str, uint8_t a[5]);
int rtu_addr_str2array(const char* str, uint8_t a[4]);
int reflash_this_wp(webs_t wp, const char *page);
char * point2next(char** s, const char split);
////杂类函数,全系统相关
int load_webs_conf_info(void);

void init_semun(void);
char* a2jsObj(char *tmp, uint8_t * array,int n);
char* getconf(const char * const name,char** value);
void webs_free(void);
void response_ok(webs_t wp);
int printf_webs_app_dir(void);
int load_web_root_dir(char* webdir);
char *trim(char in[], int len);
int load_web_root_dir(char* webdir);
static int initWebs(void);
static int websHomePageHandler(webs_t wp, char_t *urlPrefix, char_t *webDir,
        int arg, char_t *url, char_t *path, char_t *query);
static void sigintHandler(int);
int jsonSavCycle(webs_t wp,const char* name,const stSave_cycle sav);
union semun
{
	int val;
	struct semid_ds *buf;
	unsigned short int *array;
	struct seminfo *__buf;
};
#endif /* MAIN_H_ */
