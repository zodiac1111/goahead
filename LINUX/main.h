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
#include "type.h"
#include "tou.h"
#include "conf.h"

#if DEBUG_INFO_FORM
///打印form表单提交函数的入口信息:函数名,传递进来的队列
#define PRINT_FORM_INFO printf(PREFIX_INF"%s:%s\n", __FUNCTION__,query)
#else
#define PRINT_FORM_INFO
#endif
#define MTR_ADD 1 //增删查改?...
#define MTR_DEL  2
#define MTR_SEARCH  3
#define MTR_UPDATE  4
#define MAX_MTR_NUM 256 ///<最大表计个数
#define PAGE_RESET "/um/Reset.asp"///重启页面
//#define INPUT_CLASS " class=ntx "
#define CHKBOX_ONCLICK " onclick=\"chk_change(event);\" "
//提交表单函数定义
void form_server_time(webs_t wp, char_t *path, char_t *query);
void form_sysparam(webs_t wp, char_t *path, char_t *query);
void form_mtrparams(webs_t wp, char_t *path, char_t *query);
void form_mtr_items(webs_t wp, char_t *path, char_t *query);
void form_sioplans(webs_t wp, char_t *path, char_t *query);
void form_netparas(webs_t wp, char_t *path, char_t *query);
void form_monparas(webs_t wp, char_t *path, char_t *query);
void form_savecycle(webs_t wp, char_t *path, char_t *query);
void form_reset(webs_t wp, char_t *path, char_t *query);
void form_history_tou(webs_t wp, char_t *path, char_t *query);
void form_msg(webs_t wp, char_t *path, char_t *query);
void form_msg_stop(webs_t wp, char_t *path, char_t *query);
void form_load_procotol_cfgfile(webs_t wp, char_t *path, char_t *query);
void form_save_procotol_cfgfile(webs_t wp, char_t *path, char_t *query);
void form_save_log(webs_t wp, char_t *path, char_t *query);
void form_load_log(webs_t wp, char_t *path, char_t *query);
void form_load_monport_cfgfile(webs_t wp, char_t *path, char_t *query);
void form_save_monport_cfgfile(webs_t wp, char_t *path, char_t *query);
//各类大相
int webRece_syspara(webs_t wp,stSysParam* sysparam);
int webSend_syspara(webs_t wp);
int webSend_mtr_sioplan(webs_t wp,stSysParam sysparam);
int webSend_mtr_procotol(webs_t wp);
int webSend_mtr_factory(webs_t wp);
int webSend_mtr_type(webs_t wp);
int webSend_savecycle(webs_t wp);
int webRece_savecycle(webs_t wp);
int webRece_mtrparams(webs_t wp);
int webSend_mtrparams(webs_t wp, int mtrnum);
int webSend_sioplans(webs_t wp, stSysParam sp);
int webRece_sioplans(webs_t wp);
int webSend_netparas(webs_t wp, int netParamNum);
int webRece_netparas(webs_t wp);
int webSend_monparas(webs_t wp, stSysParam sysparam);
int webRece_monparas(webs_t wp);
int webSend_txtfile(webs_t wp, const char*file);
int webRece_txtfile(webs_t wp, char_t *query, const char* file);
//表计参数页面 小项目
///@todo 表计参数所有数据传输方式都需要使用json格式
static int webWrite_mtrno(webs_t wp, int no);
static int webWrite_line(webs_t wp, stMtr mtr);
static int webWrite_mtraddr(webs_t wp, stMtr mtr);
static int webWrite_pwd(webs_t wp, stMtr mtr);
static int webWrite_it_dot(webs_t wp, stMtr mtr);
static int webWrite_v_dot(webs_t wp, stMtr mtr);
static int webWrite_i_dot(webs_t wp, stMtr mtr);
static int webWrite_p_dot(webs_t wp, stMtr mtr);
static int webWrite_q_dot(webs_t wp, stMtr mtr);
static int webWrite_xl_dot(webs_t wp, stMtr mtr);
static int webWrite_ue(webs_t wp, stMtr mtr);
static int webWrite_ie(webs_t wp, stMtr mtr);
static int webWrite_uartport(webs_t wp, stMtr mtr);
static int webWrite_uartPlan(webs_t wp, stMtr mtr);
static int webWrite_mtr_protocol(webs_t wp, stMtr mtr);
static int webWrite_ph_wire(webs_t wp, stMtr mtr);
static int webWrite_factory(webs_t wp, stMtr mtr);
static int webWrite_iv(webs_t wp, stMtr mtr);
static int getmtrparams(stMtr amtr[MAX_MTR_NUM], webs_t wp, u32 e[MAX_MTR_NUM]);
//串口方案项目
//网口参数
static int webWrite_ip(webs_t wp, char *name, u8* value);
//监视端口参数
static int webWrite_commportList(webs_t wp);
static int webWrite_listen_port(webs_t wp, stMonparam monport);
static int webWrite_porttype(webs_t wp);
static int webWrite_rtu_addr(webs_t wp, stMonparam monport);
///其他
static int split(char **ret, char* in);
static int is_all_equ(int n[], int num);
int portstr2u8(const char * str, u8* val);
int listen_port_str2array(const char* str, u8 a[5]);
int rtu_addr_str2array(const char* str, u8 a[4]);
static int reflash_this_wp(webs_t wp, const char *page);
char * point2next(char** s, const char split);

//

////设置函数
int printf_webs_app_dir(void);
int load_web_root_dir(char* webdir);
char *trim(char in[], int len);
int load_web_root_dir(char* webdir);
static int initWebs(void);
static int websHomePageHandler(webs_t wp, char_t *urlPrefix, char_t *webDir,
        int arg, char_t *url, char_t *path, char_t *query);
static void sigintHandler(int);
#endif /* MAIN_H_ */
