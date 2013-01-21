/**
 * @file main.h
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
#define MTR_ADD 1 //增删查改?...
#define MTR_DEL  2
#define MTR_SEARCH  3
#define MTR_UPDATE  4
#define MAX_MTR_NUM 256 ///<最大表计个数
#define PAGE_SYSTEM_PARAMETER "/um/syspara1.asp"///<系统参数页面
#define PAGE_METER_PARAMETER "/um/meterpara1.asp" ///<表计参数页面
#define PAGE_MONITOR_PARAMETER "/um/monpara1.asp"///<监视端口参数页面
#define PAGE_NET_PARAMETER "/um/netpara1.asp"///<网络参数页面
#define PAGE_COM_PARAMETER "/um/compara1.asp"///<串口方案参数页面
#define PAGE_SAVECYCLE_PARAMETER "/um/savcirc0.asp"///<储存周期页面
#define PAGE_HISTORY_TOU "/um/data1.asp"///<储存周期页面
#define PAGE_RESET "/um/Reset.asp"///重启页面
#define TH_CLASS " class= "
#define TD_CLASS " class=sysTDNcLItemStyle "
#define INPUT_CLASS " class=ntx "
#define CHKBOX_ONCLICK " onclick=\"chk_change(event);\" "
#define HIDE_CLASS "" //" class=hideinp " //隐藏的用于POST的text类,
//读取函数
//系统参数页面(file sysspara.cfg)
//小项目
static int asp_list_mtr_protocol(int eid, webs_t wp, int argc, char_t **argv);
static int asp_list_sioplan(int eid, webs_t wp, int argc, char_t **argv);
static int asp_factory(int eid, webs_t wp, int argc, char_t **argv);
static int ph_wire2(int eid, webs_t wp, int argc, char_t **argv);
static int asp_load_mtr_param(int eid, webs_t wp, int argc, char_t **argv);
static int asp_server_time(int eid, webs_t wp, int argc, char_t **argv);
//
///提交表单
//系统参数
void form_sysparam(webs_t wp, char_t *path, char_t *query);
int webGet_syspara(webs_t wp);
int webSet_syspara(webs_t wp);
static void form_set_mtrparams(webs_t wp, char_t *path, char_t *query);
void form_sioplans(webs_t wp, char_t *path, char_t *query);
void form_netparas(webs_t wp, char_t *path, char_t *query);
void form_monparas(webs_t wp, char_t *path, char_t *query);
 void form_savecycle(webs_t wp, char_t *path, char_t *query);
static void form_reset(webs_t wp, char_t *path, char_t *query);
static void form_history_tou(webs_t wp, char_t *path, char_t *query);
void form_msg(webs_t wp, char_t *path, char_t *query);
void form_msg_stop(webs_t wp, char_t *path, char_t *query);
void form_load_procotol_cfgfile(webs_t wp, char_t *path, char_t *query);
void form_save_procotol_cfgfile(webs_t wp, char_t *path, char_t *query);

//表计参数页面的.(file mtrspara.cfg)

static int webWrite_mtrno(webs_t wp, int no);
static int read_mtr_no(int eid, webs_t wp, int argc, char_t **argv);
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
int webSet_sioplans(webs_t wp, stSysParam sp);
int webGet_sioplans(webs_t wp);
static int webWrite_plan_no(webs_t wp, int no, stUart_plan plan);
static int webWrite_parity(webs_t wp, int no, stUart_plan plan);
static int webWrite_dat_bit(webs_t wp, int no, stUart_plan plan);
static int webWrite_stop_bit(webs_t wp, int no, stUart_plan plan);
static int webWrite_baud(webs_t wp, int no, stUart_plan plan);
static int webWrite_commtype(webs_t wp, int no, stUart_plan plan);
//网口参数
int webSet_netparas(webs_t wp, stSysParam sysparam);
int webGet_netparas(webs_t wp);
static int webWrite_net_no(webs_t wp, int no, stNetparam netparam);
static int webWrite_eth(webs_t wp, int net_num, stNetparam netparam);
static int webWrite_ip(webs_t wp, int no, stNetparam netparam);
static int webWrite_mask(webs_t wp, int no, stNetparam netparam);
static int webWrite_gateway(webs_t wp, int no, stNetparam netparam);
//监视端口参数
int webSet_monparas(webs_t wp, stSysParam sysparam);
int webGet_monparas(webs_t wp, stSysParam sysparam);
static int webWrite_mon_no(webs_t wp, int no, stMonparam monport);
static int webWrite_commport(webs_t wp, int no, stMonparam monport);
static int webWrite_listen_port(webs_t wp, int no, stMonparam monport);
static int webWrite_portplan(webs_t wp, int sioplan_num, stMonparam monport);
static int webWrite_porttype(webs_t wp, stMonparam monport);
static int webWrite_rtu_addr(webs_t wp, int no, stMonparam monport);
static int webWrite_timesyn(webs_t wp, int no, stMonparam monport);
static int webWrite_forward_enable(webs_t wp, int no, stMonparam monport);
static int webWrite_forward_mtr_num(webs_t wp, int no, stMonparam monport);
///其他
static int split(char **ret, char* in);
static int is_all_equ(int n[], int num);
int portstr2u8(const char * str, u8* val);
int listen_port_str2array(const char* str, u8 a[5]);
int rtu_addr_str2array(const char* str, u8 a[4]);
static int reflash_this_wp(webs_t wp, const char *page);
char * point2next(char** s, const char split);
static void form_save_log(webs_t wp, char_t *path, char_t *query);
static void form_load_log(webs_t wp, char_t *path, char_t *query);
static void form_load_monport_cfgfile(webs_t wp, char_t *path, char_t *query);
static void form_save_monport_cfgfile(webs_t wp, char_t *path, char_t *query);
//
void webSet_txtfile(webs_t wp, char_t *path, char_t *query, const char*file);
void webGet_txtfile(webs_t wp, char_t *path, char_t *query, const char* file);
////设置函数
static int initWebs(int demo);
static int websHomePageHandler(webs_t wp, char_t *urlPrefix, char_t *webDir,
        int arg, char_t *url, char_t *path, char_t *query);
static void sigintHandler(int);
#endif /* MAIN_H_ */
