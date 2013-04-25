/**
 * @file sysfunction.h
 * 系统参数部分功能实现
 */

#ifndef SYSFUNCTION_H_
#define SYSFUNCTION_H_
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include "../uemf.h"
#include "../wsIntrn.h"
#include "conf.h"
#include "json.h"
#include "wpsend.h"
#include "web_err.h"
#include "param.h"
#include "autoUpdate.h"
//这两个目前在主函数定义 TODO 分离
extern char *procotol_name[MAX_PROCOTOL_NUM];
extern int procotol_num ;

extern char * webdir;
//web shell 部分功能需要信号量
extern struct sembuf sb;
extern int semid;     ///<信号量id
extern union semun
{
	int val;
	struct semid_ds *buf;
	unsigned short int *array;
	struct seminfo *__buf;
}sem;     ///<用于控制报文监视停止的信号量.0停止监视程序,1运行监视程序
extern void form_sysFunction(webs_t wp, char_t *path, char_t *query);
extern void form_msg(webs_t wp, char_t *path, char_t *query);
extern void form_msg_stop(webs_t wp, char_t *path, char_t *query);
extern void form_load_procotol_cfgfile(webs_t wp, char_t *path, char_t *query);
extern void form_save_procotol_cfgfile(webs_t wp, char_t *path, char_t *query);
extern void form_save_log(webs_t wp, char_t *path, char_t *query);
extern void form_load_log(webs_t wp, char_t *path, char_t *query);
extern void form_load_monport_cfgfile(webs_t wp, char_t *path, char_t *query);
extern void form_save_monport_cfgfile(webs_t wp, char_t *path, char_t *query);
extern void form_info(webs_t wp, char_t *path, char_t *query);
extern void init_semun(void);
#endif /* SYSFUNCTION_H_ */
