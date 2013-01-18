/**
 * main.c -- Main program for the GoAhead WebServer (LINUX version)
 * Copyright (c) GoAhead Software Inc., 1995-2010. All Rights Reserved.
 * See the file "license.txt" for usage and redistribution license requirements
 */

/**
 *	Main program for for the GoAhead WebServer.
 */
#include "../uemf.h"
#include "../wsIntrn.h"
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "param.h"
#include "main.h"
#include "Chinese_string.h"
#include "web_err.h"
#include "tou.h"
#ifdef WEBS_SSL_SUPPORT
#include	"../websSSL.h"
#endif
#ifdef USER_MANAGEMENT_SUPPORT
#include	"../um.h"
void formDefineUserMgmt(void);
#endif

/*
 *	Change configuration here
 */
static char_t *rootWeb = T("www"); /* Root web directory */
static char_t *demoWeb = T("wwwdemo");/* Root web directory */
static char_t *password = T(""); /* Security password */
static int port = WEBS_DEFAULT_PORT;/* Server port */
static int retries = 5; /* Server port retries */
static int finished = 0; /* Finished flag */
#ifdef B_STATS
static void printMemStats(int handle, char_t *fmt, ...);
static void memLeaks();
#endif
///保存当前操作的表在内存中,方便修改表参数的操作.当前表号不存文件,仅仅运行时有用
int g_cur_mtr_no = 0;
long maddr = 0;
stSysParam sysparam = { 0 };
static char *procotol_name[MAX_PROCOTOL_NUM];     ///<规约文件中的规约名称.
static int procotol_num = MAX_PROCOTOL_NUM;     ///<规约文件中的实际规约数,初始化为最大
static char *mon_port_name[MAX_MON_PORT_NUM];     ///<规约文件中的规约名称.
static int mon_port_num = MAX_MON_PORT_NUM;     ///<规约文件中的实际规约数,初始化为最大
int is_monmsg = 1;
union semun
{
	int val;
	struct semid_ds *buf;
	unsigned short int *array;
	struct seminfo *__buf;
};
struct sembuf sb;     //信号量操作
union semun sem;     ///<用于控制报文监视停止的信号量.0停止监视程序,1运行监视程序
int semid;     ///<信号量id
void init_semun(void)
{
	///申请信号量组，包含1个信号量
	semid = semget(1000, 1, 0666|IPC_CREAT);
	sem.val = 1;
	///初始化0号信号量为1默认有一个进程可以使用
	semctl(semid, 0, SETVAL, sem);
}
int main(int argc, char** argv)
{
	init_semun();
	int i, demo = 1;
	for (i = 1; i<argc; i++) {
		if (strcmp(argv[i], "-demo")==0) {
			demo++;
		}
	}
	/*
	 *	Initialize the memory allocator. Allow use of malloc and start
	 *	with a 60K heap.  For each page request approx 8KB is allocated.
	 *	60KB allows for several concurrent page requests.  If more space
	 *	is required, malloc will be used for the overflow.
	 */
	bopen(NULL, (60*1024), B_USE_MALLOC);
	signal(SIGPIPE, SIG_IGN);
	signal(SIGINT, sigintHandler);
	signal(SIGTERM, sigintHandler);
	/*
	 *	Initialize the web server
	 */
	if (initWebs(demo)<0) {
		return -1;
	}

#ifdef WEBS_SSL_SUPPORT
	websSSLOpen();
	/*	websRequireSSL("/"); *//* Require all files be served via https */
#endif

	/*
	 * Basic event loop. SocketReady returns true when a socket is ready for
	 * service. SocketSelect will block until an event occurs. SocketProcess
	 * will actually do the servicing.
	 */
	finished = 0;
	while (!finished) {
		if (socketReady(-1)||socketSelect(-1, 1000)) {
			socketProcess(-1);
		}
		websCgiCleanup();
		emfSchedProcess();
	}

#ifdef WEBS_SSL_SUPPORT
	websSSLClose();
#endif

#ifdef USER_MANAGEMENT_SUPPORT
	umClose();
#endif

	/*
	 *	Close the socket module, report memory leaks and close the memory allocator
	 */
	websCloseServer();
	socketClose();
#ifdef B_STATS
	memLeaks();
#endif
	bclose();
	return 0;
}

/**
 * Exit cleanly on interrupt
 */
static void sigintHandler(int unused)
{
	finished = 1;
}

/**
 * Initialize the web server.
 * #
 * # 注册asp和form函数
 * # 改变程序当前目录.
 * @param demo
 * @param argv
 * @return
 */
static int initWebs(int demo)
{
	struct hostent *hp;
	struct in_addr intaddr;
	char host[128], dir[128] = { 0 }, webdir[128];
	char *cp;
	char_t wbuf[128];
	//Initialize the socket subsystem
	socketOpen();
#ifdef USER_MANAGEMENT_SUPPORT
	/*
	 *	Initialize the User Management database
	 */
	umOpen();
	umRestore(T("umconfig.txt"));
#endif
	/*
	 *	Define the local Ip address, host name, default home page and the
	 *	root web directory.
	 */
	if (gethostname(host, sizeof(host))<0) {
		error(E_L, E_LOG, T("Can't get hostname"));
		return -1;
	}
	if ((hp = gethostbyname(host))==NULL) {
		error(E_L, E_LOG, T("Can't get host address"));
		return -1;
	}
	memcpy((char *) &intaddr, (char *) hp->h_addr_list[0],
	                (size_t) hp->h_length);

	/*
	 *	Set ../web as the root web. Modify this to suit your needs
	 *	A "-demo" option to the command webWrite_line will set a webdemo root
	 */
	//getcwd(dir, sizeof(dir));///当前路径作为应用程序目录,终端不适合
	/*
	 * -----/XXX/webs   <-服务器程序所在路径,即argv[0]=/..../XXX/webs
	 *     `/www(demo)  <-网站根目录
	 */
	//读取程序自身路径,用于计算www root,(依赖 webs和www的相对位置.不健壮)
	///@todo 像其他服务器一样使用配置文件配置web服务器的根目录.
	int count = readlink("/proc/self/exe", dir, 128);
	if (count<0||count>128) {
		PRINT_RET(count);
		web_err_proc(EL);
	}
	printf("app dir is %s\n", dir);
	if ((cp = strrchr(dir, '/'))) {     ///向上回2级父目录
		*cp = '\0';
	}
	if ((cp = strrchr(dir, '/'))) {
		*cp = '\0';
	}
	if (demo) {
		sprintf(webdir, "%s/%s", dir, demoWeb);
	} else {
		sprintf(webdir, "%s/%s", dir, rootWeb);
	}
	///@todo 使用配置文件读取路径,如果文件读取错误才由程序硬编码决定.
#if __i386 == 1
	//本机调试(家里)
	//sprintf(webdir, "%s","/home/zodiac1111/Aptana Studio 3 Workspace/wwwdemo");
	//host上调试
	sprintf(webdir, "%s","/home/lee/Aptana Studio 3 Workspace/wwwdemo");
#endif
	printf("change web root dir to \"%s\"\n", webdir);
	///改变程序的当前目录,所有相对路径都是相对当前目录的.当前目录为www(demo)目录
	///必须使用绝对路径启动程序,传入argv[0]的是/mnt/nor/bin/webs这样的路径
	///因为web根目录需要
	chdir(webdir);
	/*
	 *	Configure the web server options before opening the web server
	 */
	websSetDefaultDir(webdir);
	cp = inet_ntoa(intaddr);
	ascToUni(wbuf, cp, min(strlen(cp) + 1, sizeof(wbuf)));
	websSetIpaddr(wbuf);
	ascToUni(wbuf, host, min(strlen(host) + 1, sizeof(wbuf)));
	websSetHost(wbuf);

	/*
	 * Configure the web server options before opening the web server
	 */
	websSetDefaultPage(T("default.asp"));
	websSetPassword(password);

	/*
	 * Open the web server on the given port. If that port is taken, try
	 * the next sequential port for up to "retries" attempts.
	 */
	websOpenServer(port, retries);

	/*
	 * First create the URL handlers. Note: handlers are called in sorted order
	 * with the longest path handler examined first. Here we define the security
	 * handler, forms handler and the default web page handler.
	 */
	websUrlHandlerDefine(T(""), NULL, 0, websSecurityHandler,
	                WEBS_HANDLER_FIRST);
	websUrlHandlerDefine(T("/goform"), NULL, 0, websFormHandler, 0);
	websUrlHandlerDefine(T("/cgi-bin"), NULL, 0, websCgiHandler, 0);
	websUrlHandlerDefine(T(""), NULL, 0, websDefaultHandler,
	                WEBS_HANDLER_LAST);
	//printf("监视端口文件:%s", MON_PORT_NAME_FILE);
	//载入配置文件
	if (-1==init_monparam_port_name(mon_port_name, &mon_port_num,
	                MON_PORT_NAME_FILE)) {
		web_err_proc(EL);
	} else {
		printf("init_monparam_port_name:OK\n");
	}
	if (-1==read_protocol_file(procotol_name, &procotol_num, PORC_FILE)) {
		web_err_proc(EL);
	} else {
		printf("read_protocol_file:OK\n");
	}
	/*
	 *	Now define two test procedures. Replace these with your application
	 *	relevant ASP script procedures and form functions.
	 */
	/**
	 * 注册asp函数,给予asp调用
	 */
	///系统参数
	websAspDefine(T("meter_num"), asp_get_meter_num);
	websAspDefine(T("sioports_num"), asp_get_sioports_num);
	websAspDefine(T("netports_num"), asp_netports_num);
	websAspDefine(T("pulse_num"), asp_pulse_num);
	websAspDefine(T("monitor_ports"), asp_monitor_ports);
	websAspDefine(T("control_ports"), control_ports);
	websAspDefine(T("sioplan_num"), sioplan_num);
	///表计参数
	websAspDefine(T("load_mtr_param"), asp_load_mtr_param);     ///加载表参数
	websAspDefine(T("read_mtr_no"), read_mtr_no);     ///读取表号
	///asp define
	websAspDefine(T("init_sysparam"), asp_load_sysparam);
	websAspDefine(T("load_all_mtr_param"), asp_load_all_mtr_param);
	websAspDefine(T("get_netparams"), asp_load_netparams);
	websAspDefine(T("load_monparams"), asp_load_monparams);
	websAspDefine(T("mtr_protocol"), asp_list_mtr_protocol);
	websAspDefine(T("savecycle"), asp_load_savecycle);
	websAspDefine(T("server_time"), asp_server_time);
	//websAspDefine(T("web_show_log"),asp_show_log);
	websAspDefine(T("ph_wire2"), ph_wire2);
	websAspDefine(T("sioplan"), asp_list_sioplan);
	websAspDefine(T("factory"), asp_factory);
	///form define
	websFormDefine(T("formTest"), form_set_mtrparams);
	websFormDefine(T("sysparam"), form_set_sysparam);
	websFormDefine(T("sioplan"), form_sioplans);
	websFormDefine(T("netpara"), form_netparas);
	websFormDefine(T("monparas"), form_set_monparas);
	websFormDefine(T("savecycle"), form_set_savecycle);
	websFormDefine(T("reset"), form_reset);
	websFormDefine(T("get_tou"), form_history_tou);
	websFormDefine(T("save_log"), form_save_log);
	websFormDefine(T("load_log"), form_load_log);
	websFormDefine(T("load_monport_cfg"), form_load_monport_cfgfile);
	websFormDefine(T("save_monport_cfg"), form_save_monport_cfgfile);
	websFormDefine(T("load_procotol_cfg"), form_load_procotol_cfgfile);
	websFormDefine(T("save_procotol_cfg"), form_save_procotol_cfgfile);
	websFormDefine(T("msg"), form_msg);
	websFormDefine(T("msg_stop"), form_msg_stop);
	//websFormDefine(T("form_set_mtrparam"), myformTest);

	/*
	 *	Create the Form handlers for the User Management pages
	 */
#ifdef USER_MANAGEMENT_SUPPORT
	formDefineUserMgmt();
#endif

	/*
	 *	Create a handler for the default home page
	 */
	websUrlHandlerDefine(T("/"), NULL, 0, websHomePageHandler, 0);
	return 0;
}
///加载系统参数
static int asp_load_sysparam(int eid, webs_t wp, int argc, char_t **argv)
{
	int ret = load_sysparam(&sysparam, CFG_SYS);
	if (ret==-1) {
		PRINT_HERE
		websWrite(wp, T("[File:%s Line:%d] Fun:%s .\n"), __FILE__,
		                __LINE__, __FUNCTION__);
		g_cur_mtr_no = -1;
		web_err_proc(EL);
		return 0;
	}
	printf("读取 系统参数 表计个数 %d\n", sysparam.meter_num);
	return 0;
}
///加载储存周期
static int asp_load_savecycle(int eid, webs_t wp, int argc, char_t **argv)
{
	printf("读取存储周期.\n");
	stSave_cycle sav[SAVE_CYCLE_ITEM];
	int i = 0;
	int ret = load_savecycle(sav, CFG_SAVE_CYCLE);
	if (ret==-1) {
		websWrite(wp, T("[File:%s Line:%d] Fun:%s .\n"), __FILE__,
		                __LINE__, __FUNCTION__);
		web_err_proc(EL);
		return 0;
	}
	//第一行:有效标识
	websWrite(wp, T("<tr>\n"));
	websWrite(wp, T("<td>\n"));
	websWrite(wp, T("%s"), CSTR_SAVECYCLE_FLAG);
	websWrite(wp, T("</td>\n"));
	for (i = 0; i<SAVE_CYCLE_ITEM; i++) {
		websWrite(wp, T("<td>\n"));
		websWrite(wp, T("<input type=checkbox "
				"name=chk_save_enable value=%d %s %s>\n"),
		                sav[i].enable&0x01,
		                (sav[i].enable==1) ? "checked" : "",
		                CHKBOX_ONCLICK);
		///post不能传递没有被选中的复选框的值,通过text传递 class=hideinp
		websWrite(wp, T("<input %s type=text "
				"size=1 name=flag value=%d >\n"), HIDE_CLASS,
		                sav[i].enable);
		websWrite(wp, T("</td>\n"));
	}
	//第二行:储存周期
	websWrite(wp, T("<tr>\n"));
	websWrite(wp, T("<td>\n"));
	websWrite(wp, T("%s"), CSTR_SAVECYCLE_CYCLE);
	websWrite(wp, T("</td>\n"));
	for (i = 0; i<SAVE_CYCLE_ITEM; i++) {
		int j;
		websWrite(wp, T("<td>\n"));
		websWrite(wp, T("<select name=cycle >\n"));
		for (j = 0; j<sizeof(SAVE_CYCLE)/sizeof(SAVE_CYCLE[0]);
		                j++) {
			websWrite(wp, T("<option value=%d %s>%s</option>\n"), j,
			                (j==sav[i].cycle) ? "selected" : "",
			                SAVE_CYCLE[j]);
		}
		websWrite(wp, T("</td>\n"));
	}
	return 0;
}
/// 表计数目(个)
static int asp_get_meter_num(int eid, webs_t wp, int argc, char_t **argv)
{
	printf("read 表计数目:%d \n", sysparam.meter_num);
	return websWrite(wp, T("%u"), sysparam.meter_num);
}
/// 串口数目
static int asp_get_sioports_num(int eid, webs_t wp, int argc, char_t **argv)
{
	printf("read 串口数目:%d \n", sysparam.sioports_num);
	return websWrite(wp, T("%u"), sysparam.sioports_num);
}
///网口数目
static int asp_netports_num(int eid, webs_t wp, int argc, char_t **argv)
{
	printf("read 网口数目:%d \n", sysparam.netports_num);
	return websWrite(wp, T("%u"), sysparam.netports_num);
}
///脉冲数目
static int asp_pulse_num(int eid, webs_t wp, int argc, char_t **argv)
{
	printf("read 脉冲数目:%d \n", sysparam.pulse_num);
	return websWrite(wp, T("%u"), sysparam.pulse_num);
}
///监视端口数目
static int asp_monitor_ports(int eid, webs_t wp, int argc, char_t **argv)
{
	printf("read 监视端口数目:%d \n", sysparam.monitor_ports);
	return websWrite(wp, T("%u"), sysparam.monitor_ports);
}
///服务端时间
static int asp_server_time(int eid, webs_t wp, int argc, char_t **argv)
{
	printf("读取服务器时间\n");
	char strtime[128] = { 0 };
	time_t timer = time(NULL);
	struct tm * t = localtime(&timer);
	sprintf(strtime, "%04d-%02d-%02d %02d:%02d:%02d", t->tm_year+1900,
	                t->tm_mon+1, t->tm_mday, t->tm_hour, t->tm_min,
	                t->tm_sec);
	return websWrite(wp, T("%s"), strtime);
}
///控制端口数目
static int control_ports(int eid, webs_t wp, int argc, char_t **argv)
{
	printf("read 控制端口数目:%d \n", sysparam.control_ports);
	return websWrite(wp, T("%u"), sysparam.control_ports);
}
///串口方案数目
static int sioplan_num(int eid, webs_t wp, int argc, char_t **argv)
{
	printf("read 串口方案数目:%d \n", sysparam.sioplan_num);
	return websWrite(wp, T("%u"), sysparam.sioplan_num);
}
/**
 * asp:调用:根据全局表号变量,加载这一个表的各种参数.
 * 页面刷新加载一次.保存在内存mtr结构体中,暂时无用.
 * @param eid
 * @param wp
 * @param argc
 * @param argv
 * @return
 */
static int asp_load_mtr_param(int eid, webs_t wp, int argc, char_t **argv)
{
	stMtr mtr = { 0, { 0 }, { 0 }, { 0 }, 0 };
	int ret = load_mtrparam(&mtr, CFG_MTR, g_cur_mtr_no);
	if (ret==-1) {
		PRINT_HERE
		return websWrite(wp, T("<font color=red font-size:120%>"
				"<b>ERR %s,l:%d,fn:%s:%s</b></font>"), __FILE__,
		                __LINE__, __FUNCTION__, myweberrstr[ret]);
		web_err_proc(EL);
	}
	return 0;
}
/**
 * asp调用:加载所有表计的表计参数.表格的N行.
 * @param eid
 * @param wp
 * @param argc
 * @param argv
 * @return
 */
static int asp_load_all_mtr_param(int eid, webs_t wp, int argc, char_t **argv)
{
	int no;
	stMtr mtr = { 0, { 0 }, { 0 }, { 0 }, 0 };
	printf("%s\n", __FUNCTION__);
	for (no = 0; no<sysparam.meter_num; no++) {
		if (-1==load_mtrparam(&mtr, CFG_MTR, no)) {
			web_err_proc(EL);
			continue;
		}
		websWrite(wp, T("<tr>\n"));     //一行
		(void) webWrite_mtrno(wp, no);
		(void) webWrite_iv(wp, mtr);
		(void) webWrite_line(wp, mtr);
		(void) webWrite_mtraddr(wp, mtr);
		(void) webWrite_pwd(wp, mtr);
		(void) webWrite_uartport(wp, mtr);
		(void) webWrite_uartPlan(wp, mtr);
		(void) webWrite_mtr_protocol(wp, mtr);
		(void) webWrite_factory(wp, mtr);
		(void) webWrite_ph_wire(wp, mtr);
		(void) webWrite_it_dot(wp, mtr);
		(void) webWrite_xl_dot(wp, mtr);
		(void) webWrite_v_dot(wp, mtr);
		(void) webWrite_i_dot(wp, mtr);
		(void) webWrite_p_dot(wp, mtr);
		(void) webWrite_q_dot(wp, mtr);
		(void) webWrite_ue(wp, mtr);
		(void) webWrite_ie(wp, mtr);
		websWrite(wp, T("</tr>\n"));
	}
	return 0;
}
/**
 * asp调用 加载所有网络端口参数
 * @param eid
 * @param wp
 * @param argc
 * @param argv
 * @retval 0:正确
 */
static int asp_load_netparams(int eid, webs_t wp, int argc, char_t **argv)
{
	int no;
	stNetparam netparam;
	for (no = 0; no<sysparam.netports_num; no++) {
		if (-1==load_netparam(&netparam, CFG_NET, no)) {
			web_err_proc(EL);
			continue;
		}
		(void) websWrite(wp, T("<tr>\n"));
		(void) webWrite_net_no(wp, no, netparam);
		(void) webWrite_eth(wp, sysparam.netports_num, netparam);
		(void) webWrite_ip(wp, no, netparam);
		(void) webWrite_mask(wp, no, netparam);
		(void) webWrite_gateway(wp, no, netparam);
		(void) websWrite(wp, T("</tr>\n"));
	}
	return 0;
}
/**
 * asp调用 加载所有监视端口参数
 * @param eid
 * @param wp
 * @param argc
 * @param argv
 * @return
 */
static int asp_load_monparams(int eid, webs_t wp, int argc, char_t **argv)
{
	int no;
	stMonparam monpara;
	for (no = 0; no<sysparam.monitor_ports; no++) {
		if (-1==load_monparam(&monpara, CFG_MON_PARAM, no)) {
			web_err_proc(EL);
			continue;
		}
		(void) websWrite(wp, T("<tr>\n"));
		(void) webWrite_mon_no(wp, no, monpara);
		(void) webWrite_commport(wp, no, monpara);
		(void) webWrite_listen_port(wp, no, monpara);
		(void) webWrite_portplan(wp, sysparam.sioplan_num, monpara);
		(void) webWrite_porttype(wp, monpara);
		(void) webWrite_rtu_addr(wp, no, monpara);
		(void) webWrite_timesyn(wp, no, monpara);
		(void) webWrite_forward_enable(wp, no, monpara);
		(void) webWrite_forward_mtr_num(wp, no, monpara);
		(void) websWrite(wp, T("</tr>\n"));
	}
	return 0;
}
/**
 * 向页面写转发表个数单元格
 * @param wp
 * @param no
 * @param monport
 * @return
 */
static int webWrite_forward_mtr_num(webs_t wp, int no, stMonparam monport)
{
	printf("监视参数-转发表计数目:%d \n", monport.forward_mtr_num);
	websWrite(wp, T("<td>\n"));
	websWrite(wp, T("<input class=ntx type=text size=3 maxlength=3 "
			" onchange=\"verify_forward_mtr_num(event);\" "
			" name=forward_mtr_num value=\""));
	websWrite(wp, T("%d\"> "), monport.forward_mtr_num);
	websWrite(wp, T("</td>\n"));
	return 0;
}
static int webWrite_forward_enable(webs_t wp, int no, stMonparam monport)
{
	printf("监视参数-转发标志:%d\n", monport.forward_enable);
	websWrite(wp, T("<td>\n"));
	websWrite(wp, T("<input type=checkbox name=forward_chk "
			"value=%d %s %s>\n"), monport.forward_enable,
	                (monport.forward_enable) ? "checked" : "",
	                CHKBOX_ONCLICK);
	///post不能传递没有被选中的复选框的值,通过text传递
	websWrite(wp, T("<input %s type=\"text\""
			"size=1 readonly name=forward value=%d>\n"), HIDE_CLASS,
	                monport.forward_enable);
	websWrite(wp, T("</td>\n"));
	return 0;
}
static int webWrite_timesyn(webs_t wp, int no, stMonparam monport)
{
	printf("监视参数-时间同步标志:%d\n", monport.chktime_valid_flag);
	websWrite(wp, T("<td>\n"));
	websWrite(wp, T("<input type=checkbox name=time_syn_chk "
			" value=%d %s %s>\n "), monport.chktime_valid_flag,
	                (monport.chktime_valid_flag) ? "checked" : "",
	                CHKBOX_ONCLICK);
	///post不能传递没有被选中的复选框的值,通过text传递
	websWrite(wp, T("<input %s type=text "
			" size=1 readonly name=time_syn value=%d>\n"),
	                HIDE_CLASS, monport.chktime_valid_flag);
	websWrite(wp, T("</td>\n"));
	return 0;
}
/**
 * 写终端地址4个数字字符到页面.
 * @param wp
 * @param no
 * @param monport
 * @return
 */
static int webWrite_rtu_addr(webs_t wp, int no, stMonparam monport)
{
	printf("监视参数-终端地址:%d%d%d%d\n", monport.prot_addr[0],
	                monport.prot_addr[1], monport.prot_addr[2],
	                monport.prot_addr[3]);
	int i;
	websWrite(wp, T("<td>\n"));
	websWrite(wp, T(" <input class=ntx type=text size=4 maxlength=4 "
			" onchange=\"verify_rtu_addr(event);\" "
			" name=rtu_addr value=\""));
	for (i = 0; i<4; i++) {
		websWrite(wp, T("%1d"), monport.prot_addr[i]);
	}
	websWrite(wp, T("\"> </td>\n"));
	return 0;
}
///主站规约类型
static int webWrite_porttype(webs_t wp, stMonparam monport)
{
	printf("监视参数-端口类型?:%x\n", monport.sioplan);
	int i;
	websWrite(wp, T("<td>\n"));
	websWrite(wp, T(" <select name=protocol>\n"));
	for (i = 0; i<procotol_num; i++) {
		websWrite(wp, T("  <option value=\"%d\" %s >%s"
				"</option>\n"), i,
		                (i==monport.port_type) ? "selected" : "",
		                procotol_name[i]);
	}
	websWrite(wp, T("<select>\n"));
	websWrite(wp, T("</td>\n"));
	return 0;

}
static int webWrite_portplan(webs_t wp, int sioplan_num, stMonparam monport)
{
	printf("监视参数-串口方案:%x\n", monport.sioplan);
	int i;
	websWrite(wp, T("<td>\n"));
	websWrite(wp, T(" <select name=sioplan>\n"));
	for (i = 0; i<sioplan_num; i++) {
		websWrite(wp, T("  <option value=\"%d\" %s >"CSTR_PLAN
				"%d</option>\n"), i,
		                (i==monport.sioplan) ? "selected" : "", i);
	}
	websWrite(wp, T("<select>\n"));
	websWrite(wp, T("</td>\n"));
	return 0;

}
static int webWrite_listen_port(webs_t wp, int no, stMonparam monport)
{
	int i;
	printf("监视参数-监听端口:");
	for (i = 0; i<5; i++) {
		printf("%d", monport.listen_port[i]);
	}
	printf("\n");
	websWrite(wp, T("<td>\n"));
	websWrite(wp, T(" <input type=text name=listenport %s"
			" onchange=\"verify_port(event);\" "
			" size=5  maxlength=5 value="), INPUT_CLASS);
	for (i = 0; i<5; i++) {
		websWrite(wp, T("%d"), monport.listen_port[i]);
	}
	websWrite(wp, T(">\n"));
	websWrite(wp, T("</td>\n"));
	return 0;
}
static int webWrite_mon_no(webs_t wp, int no, stMonparam monport)
{
	printf("监视参数-监视参数序号:%x\n", no);
	websWrite(wp, T("<td>\n"));
	websWrite(wp, T(" <input type=text name=mon_no %s"
			" readonly=readonly size=1 value=%d>\n"), INPUT_CLASS,
	                no);
	websWrite(wp, T("</td>\n"));
	return 0;
}
static int webWrite_commport(webs_t wp, int no, stMonparam monport)
{
	printf("监视参数-使用端口:%x\n", monport.comm_port);
	int i;
	websWrite(wp, T("<td>\n"));
	websWrite(wp, T(" <select name=commport >\n"));
	for (i = 0; i<mon_port_num; i++) {
		websWrite(wp, T("  <option value=\"%d\" %s >%s"
				"</option>\n"), i,
		                (i==monport.comm_port) ? "selected" : "",
		                mon_port_name[i]);
	}
	websWrite(wp, T(" <select>\n"));
	websWrite(wp, T("</td>\n"));
	return 0;

}
static int webWrite_net_no(webs_t wp, int no, stNetparam netparam)
{
	printf("网口参数-序号:%x\n", no);
	websWrite(wp, T("<td>\n"));
	websWrite(wp, T(" <input %s type=text name=net_no "
			" readonly=readonly size=1 value=%d>\n"), INPUT_CLASS,
	                no);
	websWrite(wp, T("</td>\n"));
	return 0;

}
/**
 * 向网页写一个数据单元格,内容为网口标识,ETH1~ETH4等.
 * @param[out] wp 网页
 * @param[in] net_num 系统参数的网口数目.
 * @param[in] netparam
 * @return
 */
static int webWrite_eth(webs_t wp, int net_num, stNetparam netparam)
{
	printf("网口参数-网口:%x\n", netparam.no);
	int i;
	websWrite(wp, T("<td>\n"));
	websWrite(wp, T(" <select name=eth >\n"));
	for (i = 0; i<net_num; i++) {
		websWrite(wp, T("  <option value=\"%d\" %s >ETH%d</option>\n"),
		                i, (i==netparam.no) ? "selected" : "", i+1);
	}
	websWrite(wp, T(" <select>\n</td>\n"));
	return 0;
}
static int webWrite_ip(webs_t wp, int no, stNetparam netparam)
{
	printf("网口参数-IP\n");
	int i;
	websWrite(wp, T("<td>\n"
			" <input %s type=text size=15 maxlength=15 "
			" onchange=\"isIPv4(event);\" "
			" name=ip value=\""), INPUT_CLASS);

	for (i = 0; i<IPV4_LEN; i++) {
		websWrite(wp, T("%1d"), netparam.ip[i]);
		if (((i+1)%3==0)&&(i!=11)) {     //aaa.bbb.ccc.ddd
			websWrite(wp, T("."));
		}
	}
	websWrite(wp, T("\"> </td>\n"));
	return 0;
}
static int webWrite_mask(webs_t wp, int no, stNetparam netparam)
{
	printf("网口参数-掩码(mask)\n");
	int i;
	websWrite(wp, T("<td>\n"
			" <input %s type=text size=15 maxlength=15 "
			" onchange=\"isIPv4(event);\" "
			" name=mask value=\""), INPUT_CLASS);

	for (i = 0; i<IPV4_LEN; i++) {
		websWrite(wp, T("%1d"), netparam.mask[i]);
		if (((i+1)%3==0)&&(i!=11)) {     //aaa.bbb.ccc.ddd
			websWrite(wp, T("."));
		}
	}
	websWrite(wp, T("\"> </td>\n"));
	return 0;
}
static int webWrite_gateway(webs_t wp, int no, stNetparam netparam)
{
	printf("网口参数-网关(gateway)\n");
	int i;
	websWrite(wp, T("<td>\n"
			" <input %s type=text size=15 maxlength=15 "
			" onchange=\"isIPv4(event);\" "
			" name=gateway value="), INPUT_CLASS);

	for (i = 0; i<IPV4_LEN; i++) {
		websWrite(wp, T("%1d"), netparam.gateway[i]);
		if (((i+1)%3==0)&&(i!=11)) {     //aaa.bbb.ccc.ddd
			websWrite(wp, T("."));
		}
	}
	websWrite(wp, T("> </td>\n"));
	return 0;
}
///write 向web页面写串口方案号
static int webWrite_plan_no(webs_t wp, int no, stUart_plan plan)
{
	//printf("串口方案-序号:%d\n", no);
	return websWrite(wp, T("<td>\n"
			" <input type=text %s name=sioplanno "
			" readonly=readonly size=1 value=%d>\n"
			"</td>\n"), INPUT_CLASS, no);
}
///write 向web页面写检验位,一个table单元格 table data
static int webWrite_parity(webs_t wp, int no, stUart_plan plan)
{
	//printf("串口方案-校验位:%x\n", plan.parity);
	int i;
	websWrite(wp, T("<td>\n"
			" <select name=parity >\n"));
	for (i = 0; i<sizeof(UART_P)/sizeof(UART_P[0]); i++) {
		websWrite(wp, T("  <option value=\"%d\" %s >%s</option>\n"), i,
		                (i==plan.parity) ?
		                                   "selected=\"selected\"" :
		                                   "",
		                UART_P[i]);
	}
	websWrite(wp, T(" <select>\n</td>\n"));
	return 0;
}
///向页面也一个 串口方案-数据位 单元格,
static int webWrite_dat_bit(webs_t wp, int no, stUart_plan plan)
{
	//printf("串口方案-数据位:%x\n", plan.data);
	int i;
	websWrite(wp, T("<td>\n"
			" <select name=data >\n"));
	for (i = 0; i<sizeof(UART_DAT_BIT)/sizeof(UART_DAT_BIT[0]); i++) {
		websWrite(wp, T("  <option value=\"%d\" %s >%s</option>\n"), i,
		                (i+7==plan.data) ?
		                                   "selected=\"selected\"" :
		                                   "",
		                UART_DAT_BIT[i]);
	}
	websWrite(wp, T(" <select>\n</td>\n"));
	return 0;
}
///向页面也一个 串口方案-停止位 单元格,
static int webWrite_stop_bit(webs_t wp, int no, stUart_plan plan)
{
	//printf("串口方案-停止位:%x\n", plan.stop);
	int i;
	websWrite(wp, T("<td>\n"
			" <select name=stop >\n"));
	for (i = 0; i<sizeof(UART_STOP)/sizeof(UART_STOP[0]); i++) {
		websWrite(wp, T("  <option value=\"%d\" %s >%s</option>\n"), i,
		                (i==plan.stop) ? "selected" : "",
		                UART_STOP[i]);
	}
	websWrite(wp, T(" <select>\n</td>\n"));
	return 0;
}
///向页面也一个 串口方案-波特率 单元格,
static int webWrite_baud(webs_t wp, int no, stUart_plan plan)
{
	//printf("串口方案-波特率(300*2^BaudByte=Baud):%x  \n", plan.baud);
	int i;
	websWrite(wp, T("<td>\n"
			" <select name=baud >\n"));
	for (i = 0; i<sizeof(UART_BAUD)/sizeof(UART_BAUD[0]); i++) {
		websWrite(wp, T("  <option value=\"%d\" %s >%s</option>\n"), i,
		                (i==plan.baud) ? "selected=\"selected\"" : "",
		                UART_BAUD[i]);
	}
	websWrite(wp, T(" <select>\n</td>\n"));
	return 0;
}
///向页面也一个 串口方案-通讯类型 单元格,
static int webWrite_commtype(webs_t wp, int no, stUart_plan plan)
{
	//printf("串口方案-通讯方式(0-异步,1同步): %x \n", plan.Commtype);
	int i;
	websWrite(wp, T("<td>\n<select name=comm_type >\n"));
	for (i = 0; i<sizeof(UART_COMM_TYPE)/sizeof(UART_COMM_TYPE[0]);
	                i++) {
		websWrite(wp, T("  <option value=\"%d\" %s >%s</option>\n"), i,
		                (i==plan.Commtype) ?
		                                     "selected=\"selected\"" :
		                                     "",
		                UART_COMM_TYPE[i]);
	}
	websWrite(wp, T(" <select>\n</td>\n"));
	return 0;
}
/**
 * 读取表号,表号只能设定,根据设定的表号读取参数
 */
static int read_mtr_no(int eid, webs_t wp, int argc, char_t **argv)
{
	int i;
	for (i = 0; i<sysparam.meter_num; i++) {
		websWrite(wp, T("<option value=\"%d\" %s >%d</option>\n"), i,
		                (i==g_cur_mtr_no) ?
		                                    "selected=\"selected\"" :
		                                    "",
		                i);
	}
	//printf("asp read_mtr_no:%d \n", g_cur_mtr_no);
	return websWrite(wp, T("%d"), g_cur_mtr_no);
}
///线路名称
static int webWrite_line(webs_t wp, stMtr mtr)
{
	websWrite(wp, T("<td>\n"
			"<input class=ntx type=text size=6 maxlength=6 "
			"onchange=\"line_changed(event);\""
			"name=line value=\""));
	int i;
	for (i = 0; i<LINE_LEN; i++) {
		//if (mtr.webWrite_line[i]!=0)
		websWrite(wp, T("%1d"), mtr.line[i]);
	}
	websWrite(wp, T("\" >\n</td>\n"));
	return 0;
}
/// 表计地址
static int webWrite_mtraddr(webs_t wp, stMtr mtr)
{
	int i;
	websWrite(wp, T("<td>\n"
			"<input class=ntx type=text size=12 maxlength=12 "
			"onchange=\"addr_changed(event);\""
			"name=addr value=\""));
	for (i = 0; i<ADDR_LEN; i++) {
		//if (mtr.addr[i]!=0)
		websWrite(wp, T("%1d"), mtr.addr[i]);
	}
	websWrite(wp, T("\" >\n</td>\n"));
	return 0;
}
/// 表计口令
static int webWrite_pwd(webs_t wp, stMtr mtr)
{
	int i;
	websWrite(wp, T("<td>\n"
			"<input class=ntx type=text size=8 maxlength=8 "
			"onchange=\"pwd_changed(event);\""
			"name=pwd value=\""));
	for (i = 0; i<PWD_LEN; i++) {
		//if (mtr.webWrite_pwd[i]!=0)
		websWrite(wp, T("%1d"), mtr.pwd[i]);
	}
	websWrite(wp, T("\" >\n</td>\n"));
	return 0;
}
/// 电量小数位数
static int webWrite_it_dot(webs_t wp, stMtr mtr)
{
	return websWrite(wp, T("<td>\n"
			" <input class=ntx type=text size=1 maxlength=1 "
			" onchange=\"dot_changed(event);\" "
			" name=it_dot value=%u>\n</td>\n "), mtr.it_dot);
}
/// 电压小数位数
static int webWrite_v_dot(webs_t wp, stMtr mtr)
{
	return websWrite(wp, T("<td>\n"
			" <input class=ntx type=text size=1 maxlength=1  "
			" onchange=\"dot_changed(event);\" "
			" name=v_dot value=%u >\n</td>\n "), mtr.v_dot);

}
/// 电流小数位数
static int webWrite_i_dot(webs_t wp, stMtr mtr)
{
	return websWrite(wp, T("<td>\n"
			"<input class=ntx type=text size=1 maxlength=1  "
			" onchange=\"dot_changed(event);\" "
			"name=i_dot value=%u>\n</td>\n "), mtr.i_dot);

}
/// 有功功率小数位数
static int webWrite_p_dot(webs_t wp, stMtr mtr)
{
	return websWrite(wp, T("<td>\n"
			"<input class=ntx type=text size=1 maxlength=1 "
			" onchange=\"dot_changed(event);\" "
			"name=p_dot value=%u>\n</td>\n "), mtr.p_dot);
}
/// 无功功率小数位数
static int webWrite_q_dot(webs_t wp, stMtr mtr)
{
	return websWrite(wp, T("<td>\n"
			" <input class=ntx type=text size=1 maxlength=1 "
			" onchange=\"dot_changed(event);\" "
			" name=q_dot value=%u>\n</td>\n "), mtr.q_dot);
}
/// 需量小数位数
static int webWrite_xl_dot(webs_t wp, stMtr mtr)
{
	return websWrite(wp, T("<td>\n"
			" <input class=ntx type=text size=1 maxlength=1 "
			" onchange=\"dot_changed(event);\" "
			" name=xl_dot value=%u>\n</td>\n "), mtr.xl_dot);

}
/// 额定电压
static int webWrite_ue(webs_t wp, stMtr mtr)
{
	return websWrite(wp, T("<td>\n"
			" <input class=ntx type=text size=4 "
			" onchange=\"ue_changed(event);\" "
			" name=ue value=%u>\n</td>\n "), mtr.ue);
}
/// 额定电流
static int webWrite_ie(webs_t wp, stMtr mtr)
{
	return websWrite(wp, T("<td>\n"
			" <input class=ntx type=text size=4 "
			" onchange=\"ie_changed(event);\" "
			" name=ie value=%u>\n</td>\n "), mtr.ie);
}
/// 使用的串口号
static int webWrite_uartport(webs_t wp, stMtr mtr)
{
	int i;
	printf("表计参数-使用串口号:%d\n", mtr.port);
	websWrite(wp, T("<td>\n"
			"<select name=port >\n"));
	for (i = 0; i<sysparam.sioports_num; i++) {
		websWrite(wp, T("<option value=\"%d\" %s >com%d</option>\n"), i,
		                (i==mtr.port) ? "selected=\"selected\"" : "",
		                i+1);
	}
	websWrite(wp, T("</td>\n"));
	return 0;
}
/// 串口方案号
static int webWrite_uartPlan(webs_t wp, stMtr mtr)
{
	int i;
	printf("表计参数-串口方案号:%d 串口数 %d\n", mtr.portplan, sysparam.sioplan_num);
	websWrite(wp, T("<td>\n"
			"<select name=portplan >\n"));
	for (i = 0; i<sysparam.sioplan_num; i++) {
		websWrite(
		                wp,
		                T("<option value=\"%d\" %s >"CSTR_PLAN"%d</option>\n"),
		                i,
		                (i==mtr.portplan) ?
		                                    "selected=\"selected\"" :
		                                    "",
		                i);
	}
	websWrite(wp, T("</td>\n"));
	return 0;
}
/// 表计规约
static int webWrite_mtr_protocol(webs_t wp, stMtr mtr)
{
	int i;
	printf("表计参数-表计规约:%d\n", mtr.protocol);
	websWrite(wp, T("<td>\n"
			"<select name=protocol >\n"));
	for (i = 0; i<procotol_num; i++) {
		websWrite(wp, T("<option value=\"%d\" %s >%s</option>\n"), i,
		                (i==mtr.protocol) ?
		                                    "selected=\"selected\"" :
		                                    "",
		                procotol_name[i]);
	}
	websWrite(wp, T("</td>\n"));
	return 0;
}
/**
 * asp 调用函数,仅列出所有的规约.供选择,不是针对某一个表的那个规约.
 * 用户设置所有的表计的表计规约列表框.
 * @param eid
 * @param wp
 * @param argc
 * @param argv
 * @return
 */
static int asp_list_mtr_protocol(int eid, webs_t wp, int argc, char_t **argv)
{
	int i;
	websWrite(wp, T("<select name=all_protocol "
			"onchange=\"changeall_mtr_protocol(event);\">\n"));
	for (i = 0; i<procotol_num; i++) {
		websWrite(wp, T(" <option value=\"%d\" >%s</option>"), i,
		                procotol_name[i]);
	}
	websWrite(wp, T("</td>\n"));
	return 0;
}
/**
 * 列表框:列举出所有的串口方案.
 * 以串口方案号作为列举内容.用于配置所有表的串口方案选择列表框.
 * @param eid
 * @param wp
 * @param argc
 * @param argv
 * @return
 */
static int asp_list_sioplan(int eid, webs_t wp, int argc, char_t **argv)
{
	printf("加载所有串口方案,共%d个\n", sysparam.sioplan_num);
	int i;
	websWrite(wp, T("<select name=all_portplan "
			"onchange=\"changeall_sioplan(event);\">\n"));
	for (i = 0; i<sysparam.sioplan_num; i++) {
		websWrite(
		                wp,
		                T(" <option value=\"%d\" >"CSTR_PLAN"%d</option>"),
		                i,
		                i);
	}
	return 0;
}
///@todo 表计厂家使用配置文件
/**
 * asp调用:加载所有电表厂家字符串
 * @param eid
 * @param wp
 * @param argc
 * @param argv
 * @return
 */
static int asp_factory(int eid, webs_t wp, int argc, char_t **argv)
{
//PRINT_HERE;
	int i;
	char *fact[] = { HOLLEY, WEI_SHENG, LAN_JI_ER, HONG_XIANG, "other" };
	printf("加载所有生产厂家:共%d个\n", sizeof(fact)/sizeof(fact[0]));
	websWrite(wp, T("<select name=all_factory "));
	websWrite(wp, T("onchange=\"setall_factory(event);\">\n"));
	for (i = 0; i<sizeof(fact)/sizeof(fact[0]); i++) {
		websWrite(wp, T("<option value=\"%d\">%s</option>"), i,
		                fact[i]);
	}
	return 0;
}
/**
 * asp调用函数 列举出所有的几相几线制类型,不针对具体的哪块表.
 * @param eid
 * @param wp
 * @param argc
 * @param argv
 * @return
 */
static int ph_wire2(int eid, webs_t wp, int argc, char_t **argv)
{
	int i;
	//就一个选择列表框.
	websWrite(wp, T("<select name=ph_wire_all "
			"onchange=\"type_all_changed(event);\">\n"));
	for (i = 0; i<sizeof(PW)/sizeof(PW[0]); i++) {
		websWrite(wp, T("<option value=\"%d\" >%s</option>"), i,
		                PW[i]);
	}
	return 0;
}
/// 几相几线制,就两种情况 0-3相3线 1-3相4线
static int webWrite_ph_wire(webs_t wp, stMtr mtr)
{
//PRINT_HERE;
	printf("表计参数-几相几线:%d\n", mtr.p3w4);

	int i;
	websWrite(wp, T("<td>\n"
			"<select name=ph_wire >\n"));
	for (i = 0; i<sizeof(PW)/sizeof(PW[0]); i++) {
		websWrite(wp, T("<option value=\"%d\" %s >%s</option>\n"), i,
		                (i==mtr.p3w4) ? "selected=\"selected\"" : "",
		                PW[i]);
	}
	websWrite(wp, T("</td>\n"));
	return 0;
}
/// 生产厂家
static int webWrite_factory(webs_t wp, stMtr mtr)
{
//PRINT_HERE;
	int i;
	char *fact[] = { HOLLEY, WEI_SHENG, LAN_JI_ER, HONG_XIANG, "other" };
	printf("表计参数-生产厂家:%d\n", mtr.fact);
	websWrite(wp, T("<td>\n"
			"<select name=factory >\n"));
	for (i = 0; i<sizeof(fact)/sizeof(fact[0]); i++) {
		websWrite(wp, T("<option value=\"%d\" %s >%s</option>\n"), i,
		                (i==mtr.fact) ? "selected=\"selected\"" : "",
		                fact[i]);
	}
	websWrite(wp, T("</td>\n"));
	return 0;
}
/// 有效标志
static int webWrite_iv(webs_t wp, stMtr mtr)
{
	//PRINT_HERE;
	printf("表计参数-有效标志:%x\n", mtr.iv);
	websWrite(wp, T("<td>\n"
			"<input type=checkbox  name=iv_check value=\"%d\" %s "
			" id=ivchk onclick=\"chk_change(event);\" >"
			"\n"), mtr.iv&0x01, (mtr.iv&0x01) ? "checked" : "");
	///post不能传递没有被选中的复选框的值,通过text传递
	websWrite(wp, T("<input class=\"hideinp\" type=\"text\""
			"size=1 name=\"iv\" value=%d >"), mtr.iv&0x01);
	websWrite(wp, T("</td>\n"));
	return 0;
}
/**
 * 向页面写"表计参数-表号"单元格
 * @param wp
 * @param no
 * @return
 */
static int webWrite_mtrno(webs_t wp, int no)
{
	printf("表计参数-表号:%d\n", no);
	return websWrite(wp, T("<td>"
			"<input type=text class=ntx name=mtrno align=center "
			" readonly=readonly size=1 value=%d>"
			"</td>\n"), no);
}

/**
 * 分割字符串
 * @param ret
 * @param in
 * @return
 */
static int split(char **ret, char* in)
{
//PRINT_HERE
	int i = 0;
//printf("in %s \n", in);
	char* token = strtok(in, " ");
//PRINT_HERE
	while (token!=NULL) {
		//PRINT_HERE
		//printf("%s ", token);
		//PRINT_HERE
		ret[i] = token;
		//printf("%p \n", ret[i]);
		token = strtok(NULL, " ");
		i++;
	}
//PRINT_HERE
	return i;
}
/**
 * 判断数组内数是否全部相等.
 * @param n
 * @param num
 * @return
 */
static int is_all_equ(int n[], int num)
{
	int i;
	for (i = 1; i<num; i++) {
		if (n[i]!=n[0]) {
			printf("POST数量[%d]=%d,表数目=%d\n", i, n[i], n[0]);
			return -2000;
		}
	}
	return 0;
}
/**
 * 从客户端post来的字符串 分解出n组表计参数.
 * 返回值表示有多少组.小于0 错误.
 *
 * @param[out] amtr 表计参数数组.下标即序号(表号)
 * @param[in] wp 页面
 * @param[in] query post来的表计参数值字符串,(所有)
 * @param[out] e 输出错误数组,每个元素表示一个表,每以位表示这个表的某一相参数的错误,1错误,0正确.
 * @retval 大于0整数:参数数组个数
 * @retval 小于0 :错误代码
 */
static int getmtrparams(stMtr amtr[MAX_MTR_NUM], webs_t wp, char_t *query,
        u32 e[MAX_MTR_NUM])
{
	int n[20] = { 0 };     //一共的记录条数
	int i = 0;
	char * errstr = NULL;
	char *no[MAX_MTR_NUM];
	char *line[MAX_MTR_NUM];
	char *addr[MAX_MTR_NUM];
	char *pwd[MAX_MTR_NUM];
	char * it_dot[MAX_MTR_NUM];
	char * v_dot[MAX_MTR_NUM];
	char * p_dot[MAX_MTR_NUM];
	char * q_dot[MAX_MTR_NUM];
	char * i_dot[MAX_MTR_NUM];
	char * xl_dot[MAX_MTR_NUM];
	char * ue[MAX_MTR_NUM];
	char * ie[MAX_MTR_NUM];
	char * port[MAX_MTR_NUM];
	char * portplan[MAX_MTR_NUM];
	char * protocol[MAX_MTR_NUM];
	char * ph_wire[MAX_MTR_NUM];
	char * factory[MAX_MTR_NUM];
	char * iv[MAX_MTR_NUM] = { 0 };
	if (websTestVar(wp, T("mtrno"))) {
		n[0] = split(no, websGetVar(wp, T("mtrno"), T("null")));
	} else {
		PRINT_HERE
		return -1000;
	}
	if (websTestVar(wp, T("line"))) {
		n[1] = split(line, websGetVar(wp, T("line"), T("null")));
	} else {
		PRINT_HERE
		return -1010;
	}
	if (websTestVar(wp, T("addr"))) {
		n[2] = split(addr, websGetVar(wp, T("addr"), T("null")));
	} else {
		PRINT_HERE
		return -1020;
	}
	if (websTestVar(wp, T("pwd"))) {
		n[3] = split(pwd, websGetVar(wp, T("pwd"), T("null")));
	} else {
		PRINT_HERE
		return -1030;
	}
	if (websTestVar(wp, T("it_dot"))) {
		n[4] = split(it_dot, websGetVar(wp, T("it_dot"), T("null")));
	} else {
		PRINT_HERE
		return -1040;
	}
	if (websTestVar(wp, T("v_dot"))) {
		n[5] = split(v_dot, websGetVar(wp, T("v_dot"), T("null")));
	} else {
		PRINT_HERE
		return -1050;
	}
	if (websTestVar(wp, T("p_dot"))) {
		n[6] = split(p_dot, websGetVar(wp, T("p_dot"), T("null")));
	} else {
		PRINT_HERE
		return -1060;
	}
	if (websTestVar(wp, T("q_dot"))) {
		n[7] = split(q_dot, websGetVar(wp, T("q_dot"), T("null")));
	} else {
		PRINT_HERE
		return -1070;
	}
	if (websTestVar(wp, T("i_dot"))) {
		n[8] = split(i_dot, websGetVar(wp, T("i_dot"), T("null")));
	} else {
		PRINT_HERE
		return -1080;
	}
	if (websTestVar(wp, T("xl_dot"))) {
		n[9] = split(xl_dot, websGetVar(wp, T("xl_dot"), T("null")));
	} else {
		PRINT_HERE
		return -1090;
	}
	if (websTestVar(wp, T("ue"))) {
		n[10] = split(ue, websGetVar(wp, T("ue"), T("null")));
	} else {
		PRINT_HERE
		return -1090;
	}
	if (websTestVar(wp, T("ie"))) {
		n[11] = split(ie, websGetVar(wp, T("ie"), T("null")));
	} else {
		PRINT_HERE
		return -1100;
	}
	if (websTestVar(wp, T("port"))) {
		n[12] = split(port, websGetVar(wp, T("port"), T("null")));
	} else {
		PRINT_HERE
		return -1110;
	}
	if (websTestVar(wp, T("portplan"))) {
		n[13] = split(portplan,
		                websGetVar(wp, T("portplan"), T("null")));
	} else {
		PRINT_HERE
		return -1120;
	}
	if (websTestVar(wp, T("protocol"))) {
		n[14] = split(protocol,
		                websGetVar(wp, T("protocol"), T("null")));
	} else {
		PRINT_HERE
		return -1130;
	}
	if (websTestVar(wp, T("ph_wire"))) {
		n[15] = split(ph_wire, websGetVar(wp, T("ph_wire"), T("null")));
	} else {
		PRINT_HERE
		return -1140;
	}
	if (websTestVar(wp, T("factory"))) {
		n[16] = split(factory, websGetVar(wp, T("factory"), T("null")));
	} else {
		PRINT_HERE
		return -1150;
	}
	printf("iv= %s\n", websGetVar(wp, T("iv"), T("0")));
	if (websTestVar(wp, T("iv"))) {
		n[17] = split(iv, websGetVar(wp, T("iv"), T("0")));
	} else {
		PRINT_HERE
		return -1160;
	}
///n[0]~n[18]应该完全相等,不然肯定是某一项目少传了
	int ret = is_all_equ(n, 18);
	if (ret!=0) {
		return ret;
	}
//PRINT_HERE
	printf("record num[0]=%d\n", n[0]);
	for (i = 0; i<n[0]; i++) {
		printf("接收循环[%d]\n", i);
		amtr[i].mtrno = strtoul(no[i], &errstr, 10);
		if (*errstr!='\0') {
			e[i] |= 0b1;
		}
		(void) strtoull(line[i], &errstr, 10);
		if (*errstr!='\0') {
			e[i] |= 0b10;
		}
		(void) strtoull(addr[i], &errstr, 10);
		if (*errstr!='\0') {
			e[i] |= 0b100;
		}
		//PRINT_HERE
		(void) strtoul(pwd[i], &errstr, 10);     //32位无符号,最多4,294,967,295能存所有9位十进制数
		if (*errstr!='\0') {
			e[i] |= 0b1000;
		}
		amtr[i].it_dot = strtoul(it_dot[i], &errstr, 10);
		if (*errstr!='\0') {
			e[i] |= 0b10000;
		}
		amtr[i].v_dot = strtol(v_dot[i], &errstr, 10);
		if (*errstr!='\0') {

			e[i] |= 0b100000;
		}
		amtr[i].p_dot = strtol(p_dot[i], &errstr, 10);
		if (*errstr!='\0') {
			e[i] |= 0b1000000;
		}
		amtr[i].q_dot = strtol(q_dot[i], &errstr, 10);
		if (*errstr!='\0') {
			e[i] |= 0b10000000;
		}
		amtr[i].i_dot = strtol(i_dot[i], &errstr, 10);
		if (*errstr!='\0') {
			e[i] |= 0b100000000;
		}
		//PRINT_HERE
		amtr[i].xl_dot = strtol(xl_dot[i], &errstr, 10);
		if (*errstr!='\0') {
			e[i] |= 0b1000000000;
		}
		amtr[i].ue = strtol(ue[i], &errstr, 10);
		if (*errstr!='\0') {
			e[i] |= 0b10000000000;
		}
		amtr[i].ie = strtol(ie[i], &errstr, 10);
		if (*errstr!='\0') {
			e[i] |= 0b100000000000;
		}
		amtr[i].port = strtol(port[i], &errstr, 10);
		if (*errstr!='\0') {
			e[i] |= 0b1000000000000;
		}
		amtr[i].portplan = strtol(portplan[i], &errstr, 10);
		if (*errstr!='\0') {
			e[i] |= 0b10000000000000;
		}
		amtr[i].protocol = strtol(protocol[i], &errstr, 10);
		if (*errstr!='\0') {
			e[i] |= 0b100000000000000;
		}
		//PRINT_HERE
		amtr[i].p3w4 = strtol(ph_wire[i], &errstr, 10);
		if (*errstr!='\0') {
			e[i] |= 0b1000000000000000;
		}
		amtr[i].fact = strtol(factory[i], &errstr, 10);
		if (*errstr!='\0') {
			e[i] |= 0b10000000000000000;
		}
		amtr[i].iv = strtol(iv[i], &errstr, 10);
		if (*errstr!='\0') {
			e[i] |= 0b100000000000000000;
		}
		//PRINT_HERE
		memset(amtr[i].line, '0', LINE_LEN);
		memset(amtr[i].addr, '0', ADDR_LEN);
		memset(amtr[i].pwd, '0', PWD_LEN);
		u8 l;
		int delta;
		//PRINT_HERE
		l = strlen(line[i]);
		//PRINT_HERE
		delta = LINE_LEN-l;
		if (delta>=0) {     //输入的位数少,前面补0
			memcpy(&amtr[i].line[0]+delta, line[i], l);
		} else {     //输入的位数多,后面截断
			memcpy(&amtr[i].line[0], line[i], LINE_LEN);
		}
		l = strlen(addr[i]);
		//PRINT_HERE
		delta = ADDR_LEN-l;
		if (delta>=0) {
			memcpy(&amtr[i].addr[0]+delta, addr[i], l);
		} else {
			memcpy(&amtr[i].addr[0], addr[i], ADDR_LEN);
		}
		l = strlen(pwd[i]);
		delta = PWD_LEN-l;
		if (delta>=0) {
			memcpy(&amtr[i].pwd[0]+delta, pwd[i], l);
		} else {
			memcpy(&amtr[i].pwd[0], pwd[i], PWD_LEN);
		}
		//PRINT_HERE
		int j = 0;
		for (j = 0; j<LINE_LEN; j++) {
			amtr[i].line[j] -= 0x30;
		}
		for (j = 0; j<ADDR_LEN; j++) {
			amtr[i].addr[j] -= 0x30;
		}
		for (j = 0; j<PWD_LEN; j++) {
			amtr[i].pwd[j] -= 0x30;
		}
		//PRINT_HERE
		//print_array(webWrite_line[i], LINE_LEN);
		//print_array(amtr[i].webWrite_line, LINE_LEN);

	}
//PRINT_HERE
	return n[0];
}
/**
 * 服务端刷新,重定向到这个页面
 * @param[in] wp 本页面
 * @param[in] page 重定向到的页面 char *
 * @return
 */
static int reflash_this_wp(webs_t wp, const char *page)
{
	websHeader(wp);
	websWrite(wp, T("<meta http-equiv=refresh content=\"0.01;"
			"url=%s\">\n"), page);
	websFooter(wp);
	websDone(wp, 200);
	return 0;
}
/**
 * 表计参数form接收打印测试函数.打印接收到的所有字符串,调试使用
 * @param wp
 * @return
 */
int mtr_param_print_item(webs_t wp)
{
	printf("OpType=%s\n", websGetVar(wp, T("OpType"), T("null")));
	printf("mtrno=%s\n", websGetVar(wp, T("mtrno"), T("null")));
	printf("line=%s\n", websGetVar(wp, T("line"), T("null")));
	printf("addr=%s\n", websGetVar(wp, T("addr"), T("null")));
	printf("pwd=%s\n", websGetVar(wp, T("pwd"), T("null")));
	printf("it_dot=%s\n", websGetVar(wp, T("it_dot"), T("null")));
	printf("v_dot=%s\n", websGetVar(wp, T("v_dot"), T("null")));
	printf("p_dot=%s\n", websGetVar(wp, T("p_dot"), T("null")));
	printf("q_dot=%s\n", websGetVar(wp, T("q_dot"), T("null")));
	printf("i_dot=%s\n", websGetVar(wp, T("i_dot"), T("null")));
	printf("xl_dot=%s\n", websGetVar(wp, T("xl_dot"), T("null")));
	printf("ue=%s\n", websGetVar(wp, T("ue"), T("null")));
	printf("ie=%s\n", websGetVar(wp, T("ie"), T("null")));
	printf("port=%s\n", websGetVar(wp, T("port"), T("null")));
	printf("portplan=%s\n", websGetVar(wp, T("portplan"), T("null")));
	printf("protocol=%s\n", websGetVar(wp, T("protocol"), T("null")));
	printf("ph_wire=%s\n", websGetVar(wp, T("ph_wire"), T("null")));
	printf("factory=%s\n", websGetVar(wp, T("factory"), T("null")));
	printf("iv= %s\n", websGetVar(wp, T("iv"), T("0")));
	return 0;
}
/**
 * 串口方案表单提交触发函数
 * @param wp
 * @param path
 * @param query
 */
void form_sioplans(webs_t wp, char_t *path, char_t *query)
{
	printf("%s:\n", __FUNCTION__);
	printf("query:%s\n", query);
	websHeader(wp);
	char * init = websGetVar(wp, T("init"), T("null"));
	if (*init=='1') {
		webSet_sioplans(wp, sysparam);
	} else {
		webGet_sioplans(wp);
	}
	websDone(wp, 200);
	return;
}
/**
 * 串口方案:页面->文件
 * @param wp
 * @return
 */
int webGet_sioplans(webs_t wp)
{
	int n = 0;
	unsigned char no;
	stUart_plan plan;
	char * sioplanno = websGetVar(wp, T("sioplanno"), T("null"));
	char * parity = websGetVar(wp, T("parity"), T("null"));
	char * data = websGetVar(wp, T("data"), T("null"));
	char * stop = websGetVar(wp, T("stop"), T("null"));
	char * baud = websGetVar(wp, T("baud"), T("null"));
	char * comm_type = websGetVar(wp, T("comm_type"), T("null"));
	/**
	 * 得到的数组字符串,类似如下:都是一个字节大小的
	 * 方方方方
	 * 案案案案
	 * 0 1 2 3 (方案序号[不存到文件])
	 * 1 1 1 0 (校验位)
	 * 1 1 1 1 (数据位)
	 * 1 1 1 1 (停止位)
	 * 2 3 5 5 (波特率)
	 * 0 0 0 0 (通讯类型)
	 * 一行为项结构体成员,一列为一个方案数组.
	 */
	while (1) {
		//串口方案序号
		n = sscanf(sioplanno, "%hhu", &no);
		if (n!=1) {     //正常完结
			break;
		}
		sioplanno = point2next(&sioplanno, ' ');
		//校验位
		n = sscanf(parity, "%hhu", &plan.parity);
		if (n!=1) {
			web_err_proc(EL);
			break;
		}
		parity = point2next(&parity, ' ');
		//数据位
		n = sscanf(data, "%hhu", &plan.data);
		if (n!=1) {
			web_err_proc(EL);
			break;
		}
		data = point2next(&data, ' ');
		plan.data += 7;
		//停止位
		n = sscanf(stop, "%hhu", &plan.stop);
		if (n!=1) {
			web_err_proc(EL);
			break;
		}
		stop = point2next(&stop, ' ');
		//波特率
		n = sscanf(baud, "%hhu", &plan.baud);
		if (n!=1) {
			web_err_proc(EL);
			break;
		}
		baud = point2next(&baud, ' ');
		//通讯类型 同步异步
		n = sscanf(comm_type, "%hhu", &plan.Commtype);
		if (n!=1) {
			web_err_proc(EL);
			break;
		}
		comm_type = point2next(&comm_type, ' ');
		//最后:保存
		save_sioplan(&plan, CFG_SIOPALN, no);
	}
	return 0;
}
/**
 * 从文件中读取串口方案,向页面写串口方案.
 * @param wp
 * @param plan 串口方案
 * @param sp 系统参数
 */
int webSet_sioplans(webs_t wp, stSysParam sp)
{
	int no;
	stUart_plan plan;
	printf("%s\n", __FUNCTION__);
	for (no = 0; no<sp.sioplan_num; no++) {
		if (-1==load_sioplan(&plan, CFG_SIOPALN, no)) {
			web_err_proc(EL);
			continue;
		}
		(void) websWrite(wp, T("<tr>\n"));
		(void) webWrite_plan_no(wp, no, plan);
		(void) webWrite_parity(wp, no, plan);
		(void) webWrite_dat_bit(wp, no, plan);
		(void) webWrite_stop_bit(wp, no, plan);
		(void) webWrite_baud(wp, no, plan);
		(void) webWrite_commtype(wp, no, plan);
		(void) websWrite(wp, T("</tr>\n"));
	}
	return 0;
}
/**
 * 把ip地址字符串如"192.168.0.1"转化成为"192.168.000.001"的12字节文件标准格式.
 * 只转换最开始一个ip地址,完成即停止,返回使用过的ipstr字符的个数.
 * 比如输入"192.168.0.1 192.168.111.2",则转换192.168.0.1为"192.168.000.001",
 * 并且返回 11.
 * @param[in] ipstr 字符串,输入输出!
 * @param ipfile
 * @return
 */
int ipstr2ipfile(char *ipstr, u8 ipfile[12])
{
	int sub = 0;
	int j = 0;
	//字符串转化为数值数组
	int val[4] = { 0, 0, 0, 0 };
	while (*(ipstr+j)!=' '&&*(ipstr+j)!='\0') {
		//分割各组数据
		if (*(ipstr+j)=='.') {
			sub++;
		} else {
			//服务端验证数字
			if (*(ipstr+j)<'0'||*(ipstr+j)>'9') {
				return -2;
			}
			//转化成为数值型
			val[sub] = val[sub]*10+(*(ipstr+j)-'0');
		}
		j++;
	}
	if (sub!=3) {		///一共应该有4项 aaa.bbb.ccc.ddd
		return -1;
	}
	ipfile[0] = (val[0]/100)%10;
	ipfile[1] = (val[0]/10)%10;
	ipfile[2] = (val[0]/1)%10;
	ipfile[3] = (val[1]/100)%10;
	ipfile[4] = (val[1]/10)%10;
	ipfile[5] = (val[1]/1)%10;
	ipfile[6] = (val[2]/100)%10;
	ipfile[7] = (val[2]/10)%10;
	ipfile[8] = (val[2]/1)%10;
	ipfile[9] = (val[3]/100)%10;
	ipfile[10] = (val[3]/10)%10;
	ipfile[11] = (val[3]/1)%10;
	return j+1;
}
/**
 * 客户端发送重启表单,分类重启.
 * @param wp
 * @param path
 * @param query
 */
static void form_reset(webs_t wp, char_t *path, char_t *query)
{
#define REINIT_PROTOCOL_FILE 1
#define RET_WEB 2
#define RET_SAMPLE_PROC 3
#define RET_RTU 4
#define RET_TEST 10
	printf("form_reset :");
	printf("query:%s\n", query);
	char app[128] = { 0 };
	int typ = 0;
	int ret = -1;
	pid_t pid;
	char * str_typ = websGetVar(wp, T("OpType"), T("null"));
	typ = atoi(str_typ);
	printf("type=%d\n", typ);
	switch (typ) {
	case REINIT_PROTOCOL_FILE:
		ret = read_protocol_file(procotol_name, &procotol_num,
		                PORC_FILE);
		if (ret!=0) {
			fprintf(stderr, "%s", myweberrstr[ret]);
			PRINT_HERE
		}
		break;
	case RET_WEB:
		readlink("/proc/self/exe", app, 128);
		pid = fork();
		if (pid==-1) {
			fprintf(stderr, "fork() error.errno:%d error:%s\n",
			                errno, strerror(errno));
			break;
		}
		if (pid==0) {		//子进程.
			system("killall webs");
			execl(app, app, NULL);
			exit(0);
		}

		if (pid>0) {

		}
		break;
	case RET_SAMPLE_PROC:
		system("killall hl3104_com ");
		break;
	case RET_RTU:
		reflash_this_wp(wp, PAGE_RESET);
#if __i386 == 1
		system("ls");
#else
		system("reboot");
#endif

		return;
		break;
	case RET_TEST:
		system("ls");
		break;
	default:
		reflash_this_wp(wp, PAGE_RESET);
		return;
		break;
	}
	reflash_this_wp(wp, PAGE_RESET);
}
/**
 * 监视端口 表单提交触发函数
 * @param wp
 * @param path
 * @param query
 */
static void form_set_monparas(webs_t wp, char_t *path, char_t *query)
{
	printf("form_set_monparas :");
	printf("query:%s\n", query);
	stMonparam monparam;
	int n;
	memset(&monparam, 0x0, sizeof(stMonparam));
	char * mon_no = websGetVar(wp, T("mon_no"), T("null"));
	char * commport = websGetVar(wp, T("commport"), T("null"));
	char * listenport = websGetVar(wp, T("listenport"), T("null"));
	char * sioplan = websGetVar(wp, T("sioplan"), T("null"));
	char * protocol = websGetVar(wp, T("protocol"), T("null"));
	char * rtu_addr = websGetVar(wp, T("rtu_addr"), T("null"));
	char * time_syn = websGetVar(wp, T("time_syn"), T("null"));
	char * forward = websGetVar(wp, T("forward"), T("null"));
	char * forward_mtr_num = websGetVar(wp, T("forward_mtr_num"),
	                T("null"));
	printf("val: \n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n", mon_no, commport,
	                listenport, sioplan, protocol, rtu_addr, time_syn,
	                forward, forward_mtr_num);

	int param_no = 0;		///参数序号,即数据库的主键,base 0.没有物理意义
	while (1) {
		//监视参数序号
		n = sscanf(mon_no, "%d", &param_no);
		if (n!=1) {		//正常完结
			break;
		}
		mon_no = point2next(&mon_no, ' ');
		//端口类型,COM ETH Master等
		n = sscanf(commport, "%hhu", &monparam.comm_port);
		if (n!=1) {
			web_err_proc(EL);
			break;
		}
		commport = point2next(&commport, ' ');
		//监听端口
		listen_port_str2array(listenport, &monparam.listen_port[0]);
		listenport = point2next(&listenport, ' ');
		//串口方案号
		n = sscanf(sioplan, "%hhu", &monparam.sioplan);
		if (n!=1) {
			web_err_proc(EL);
			break;
		}
		sioplan = point2next(&sioplan, ' ');
		//规约号
		n = sscanf(protocol, "%hhu", &monparam.port_type);
		if (n!=1) {
			web_err_proc(EL);
			break;
		}
		protocol = point2next(&protocol, ' ');
		//终端地址
		rtu_addr_str2array(rtu_addr, &monparam.prot_addr[0]);
		rtu_addr = point2next(&rtu_addr, ' ');
		//是否对时
		n = sscanf(time_syn, "%hhu", &monparam.chktime_valid_flag);
		if (n!=1) {
			web_err_proc(EL);
			break;
		}
		time_syn = point2next(&time_syn, ' ');
		//是否转发
		n = sscanf(forward, "%hhu", &monparam.forward_enable);
		if (n!=1) {
			web_err_proc(EL);
			break;
		}
		forward = point2next(&forward, ' ');
		//转发数量
		n = sscanf(forward_mtr_num, "%hhu", &monparam.forward_mtr_num);
		if (n!=1) {
			web_err_proc(EL);
			break;
		}
		forward_mtr_num = point2next(&forward_mtr_num, ' ');
		save_monparam(&monparam, CFG_MON_PARAM, param_no);
	}
	//回复(刷新)网页
	reflash_this_wp(wp, PAGE_MONITOR_PARAMETER);
}
/**
 * 将"255","1"这样的字符串转化成 "0255","0001"这样的字符数组.终端地址.
 * @param str
 * @param a
 * @return
 */
int rtu_addr_str2array(const char* str, u8 a[4])
{
	int i = 0;		//查找的个数,
	int val = 0;
	while (*str!='\0'&&*str!=' ') {
		if (*str<'0'||*str>'9') {
			return -2;
		}
		val = val*10+(*str-'0');
		str++;
		i++;
	}
	a[0] = (val/1000)%10;
	a[1] = (val/100)%10;
	a[2] = (val/10)%10;
	a[3] = (val/1)%10;
	return i+1;
}
/**
 * 将"10000 10002 9999"这样的字符串中5位的端口号,不足前面补0,返回原指针偏移数
 * @param str
 * @param a
 * @return
 */
int listen_port_str2array(const char* str, u8 a[5])
{
	int i = 0;		//查找的个数,
	int val = 0;
	while (*str!='\0'&&*str!=' ') {
		if (*str<'0'||*str>'9') {
			return -2;
		}
		val = val*10+(*str-'0');
		str++;
		i++;
	}
	a[0] = (val/10000)%10;
	a[1] = (val/1000)%10;
	a[2] = (val/100)%10;
	a[3] = (val/10)%10;
	a[4] = (val/1)%10;
	return i+1;
}
/**
 * "1"或"001"或"255"这样的字符串转化成u8类型,返回转化一个字节原字符串指针向后移动的
 * 字符个数.
 * @param str
 * @param val
 * @return
 */
int portstr2u8(const char * str, u8* val)
{
	int i = 0;		//查找的个数,
	int tmp = 0;
	while (*str!='\0'&&*str!=' ') {
		if (*str<'0'||*str>'9') {
			return -2;
		}
		tmp = tmp*10+(*str-'0');
		str++;
		i++;
	}
	*val = tmp;
	return i+1;
}
/**
 * 显示网口参数(所有): 文件->页面
 * @param wp
 * @param sysparam
 * @return
 */
int webSet_netparas(webs_t wp, stSysParam sysparam)
{
	int no;
	stNetparam netparam;
	for (no = 0; no<sysparam.netports_num; no++) {
		if (-1==load_netparam(&netparam, CFG_NET, no)) {
			web_err_proc(EL);
			continue;
		}
		(void) websWrite(wp, T("<tr>\n"));
		(void) webWrite_net_no(wp, no, netparam);
		(void) webWrite_eth(wp, sysparam.netports_num, netparam);
		(void) webWrite_ip(wp, no, netparam);
		(void) webWrite_mask(wp, no, netparam);
		(void) webWrite_gateway(wp, no, netparam);
		(void) websWrite(wp, T("</tr>\n"));
	}

	return 0;
}
/**
 * 从页面获取所有网络参数,保存到文件中.
 * @param wp
 * @return
 */
int webGet_netparas(webs_t wp)
{
	stNetparam netparam;
	int n;
	int param_no = 0;		///参数序号,即数据库的主键,base 0.没有物理意义
	char * net_no = websGetVar(wp, T("net_no"), T("null"));
	char * eth = websGetVar(wp, T("eth"), T("null"));
	char * ip = websGetVar(wp, T("ip"), T("null"));
	char * mask = websGetVar(wp, T("mask"), T("null"));
	char * gateway = websGetVar(wp, T("gateway"), T("null"));
	printf("val: \n%s\n%s\n%s\n%s\n%s\n", net_no, eth, ip, mask, gateway);
	while (1) {
		//网口参数序号
		n = sscanf(net_no, "%d", &param_no);
		if (n!=1) {		//正常完结
			break;
		}
		net_no = point2next(&net_no, ' ');
		//网口号
		n = sscanf(eth, "%hhu", &netparam.no);
		if (n!=1) {
			web_err_proc(EL);
			break;
		}
		eth = point2next(&eth, ' ');
		//ip
		ipstr2ipfile(ip, netparam.ip);
		ip = point2next(&ip, ' ');
		//mask
		ipstr2ipfile(mask, netparam.mask);
		mask = point2next(&mask, ' ');
		//gateway
		ipstr2ipfile(gateway, netparam.gateway);
		gateway = point2next(&gateway, ' ');
		save_netport(&netparam, CFG_NET, param_no);
	}
	return 0;
}
/**
 * 网口参数(多个)表单提交处理函数
 * @param wp
 * @param path
 * @param query
 */
void form_netparas(webs_t wp, char_t *path, char_t *query)
{
	printf("form_set_netparas :");
	printf("query:%s\n", query);
	websHeader(wp);
	char * init = websGetVar(wp, T("init"), T("null"));
	if (*init=='1') {
		webSet_netparas(wp, sysparam);
	} else {
		webGet_netparas(wp);
	}
	websDone(wp, 200);
	return;
}
/**
 * 从页面获取系统参数，保存到文件．同时更新全局变量．
 * @param wp
 * @return
 */
int webGet_syspara(webs_t wp)
{
	int ret = -1;
	/** 错误的项目,每一位表示一个项目,1表示此项错误,0表示此项正确.初始全部正确.
	 共8位表示6个项目,位[6,7]保留为0,位0至位5分别对应:
	 get_meter_num ;get_sioports_num ;netports_num;monitor_ports;control_ports
	 sioplan_num;*/
	int erritem = 0b00000000;
	char * str_meter_num = websGetVar(wp, T("meter_num"), T("null"));
	char * str_sioports_num = websGetVar(wp, T("sioports_num"), T("null"));
	char * str_netports_num = websGetVar(wp, T("netports_num"), T("null"));
	char * str_monitor_ports = websGetVar(wp, T("monitor_ports"),
	                T("null"));
	char * str_control_ports = websGetVar(wp, T("control_ports"),
	                T("null"));
	char * str_sioplan_num = websGetVar(wp, T("sioplan_num"), T("null"));
	///检查输入合法性
	char * errstr = NULL;
	int meter_num = strtol(str_meter_num, &errstr, 10);
	if (*errstr!='\0'||meter_num<=0||meter_num>=256) {
		printf("1:%s\n", errstr);
		erritem |= 0b1;
	}
	int sioports_num = strtol(str_sioports_num, &errstr, 10);
	if (*errstr!='\0'||sioports_num<=0||meter_num>=256) {
		printf("2:%s\n", errstr);
		erritem |= 0b10;
	}
	int netports_num = strtol(str_netports_num, &errstr, 10);
	if (*errstr!='\0'||netports_num<=0||meter_num>=256) {
		printf("3:%s\n", errstr);
		erritem |= 0b100;
	}
	int monitor_ports = strtol(str_monitor_ports, &errstr, 10);
	if (*errstr!='\0'||monitor_ports<=0||meter_num>=256) {
		printf("4:%s\n", errstr);
		erritem |= 0b1000;
	}
	int control_ports = strtol(str_control_ports, &errstr, 10);
	if (*errstr!='\0'||control_ports<=0||meter_num>=256) {
		printf("5:%s\n", errstr);
		erritem |= 0b10000;
	}
	int sioplan_num = strtol(str_sioplan_num, &errstr, 10);
	if (*errstr!='\0'||sioplan_num<=0||meter_num>=256) {
		printf("6:%s\n", errstr);
		erritem |= 0b100000;
	}
	if (erritem==0) {     //只有所有输入都合法
		sysparam.meter_num = meter_num;
		sysparam.sioports_num = sioports_num;
		sysparam.netports_num = netports_num;
		//脉冲已经废弃
		sysparam.monitor_ports = monitor_ports;
		sysparam.control_ports = control_ports;
		sysparam.sioplan_num = sioplan_num;
		ret = save_sysparam(&sysparam, CFG_SYS);
	}
	return 0;
}
/**
 * 向页面写系统参数
 * @param wp
 * @return
 */
int webSet_syspara(webs_t wp)
{
	int ret = load_sysparam(&sysparam, CFG_SYS);
	if (ret==-1) {
		web_err_proc(EL);
		return -1;
	}
	websWrite(wp, T("mtrnum=%u&"), sysparam.meter_num);
	websWrite(wp, T("sioports_num=%u&"), sysparam.sioports_num);
	websWrite(wp, T("monitor_ports=%u&"), sysparam.monitor_ports);
	websWrite(wp, T("netports_num=%u&"), sysparam.netports_num);
	websWrite(wp, T("sioports_num=%u&"), sysparam.sioports_num);
	websWrite(wp, T("control_ports=%u"), sysparam.control_ports);
	return 0;
}
/**
 * 系统参数设置表单提交触发的函数.
 * 判断数据合法,写入到sysspara.cfg文件中.一共一项,大小7字节,脉冲数目已经废弃.
 * @param wp
 * @param path
 * @param query
 */
static void form_set_sysparam(webs_t wp, char_t *path, char_t *query)
{
	printf("%s\n:", __FUNCTION__);
	printf("query:%s\n", query);
	websHeader_pure(wp);     //头和尾完成了除head和body标签在内的东西
	char * init = websGetVar(wp, T("init"), T("null"));
	if (*init=='1') {
		webSet_syspara(wp);
	} else {
		webGet_syspara(wp);
	}
	websDone(wp, 200);
	return;
//	websHeader(wp);     //头和尾完成了除head和body标签在内的东西
////设置完成自动跳转回原来的页面.
//	websWrite(wp, T("<head>\n"));
//	websWrite(wp, T("<title>"CSTR_SET_PARAM"</title>\n"));
//	//websWrite(wp, T("<meta http-equiv=\"Content-Type\" content=\"text/html; charset=GB18030\" />"));
//
//	websWrite(wp, T("</head>\n"));
////body start
//	websWrite(wp, T("<body>\n"));
//	websWrite(wp, T("<p>\n"));
//	websWrite(wp, T("%s"), (erritem&0b1) ? Fail : Success);
//	websWrite(wp, T(CSTR_MTR_NUM":\"%s\"</br>"), str_meter_num);
//	websWrite(wp, T("%s"), (erritem&0b10) ? Fail : Success);
//	websWrite(wp, T(CSTR_UART_PORT_NUM":\"%s\"</br>"), str_sioports_num);
//	websWrite(wp, T("%s"), (erritem&0b100) ? Fail : Success);
//	websWrite(wp, T(CSTR_NET_NUM":\"%s\"</br>"), str_netports_num);
//	websWrite(wp, T("%s"), (erritem&0b1000) ? Fail : Success);
//	websWrite(wp, T(CSTR_MONPORT_NUM":\"%s\"</br>"), str_monitor_ports);
//	websWrite(wp, T("%s"), (erritem&0b10000) ? Fail : Success);
//	websWrite(wp, T(CSTR_CTRLPORT_NUM":\"%s\"</br>"), str_control_ports);
//	websWrite(wp, T("%s"), (erritem&0b100000) ? Fail : Success);
//	websWrite(wp, T(CSTR_UART_PLAN_NUM":\"%s\"</br>"), str_sioplan_num);
//	websWrite(wp, T("</p>\n"));
//	websWrite(wp, T("<p>\n"));
//	if (ret==0&&erritem==0) {     //只有在全部没有出错的情况下才自动跳回.
//		websWrite(wp, T("<font color=green font-size:120%><b>"
//				CSTR_SET_OK"</b></font>"));
//		websWrite(wp, T("<meta http-equiv=refresh content=\"2;"
//				"url=%s\">\n"), PAGE_SYSTEM_PARAMETER);
//	} else {	//只有在全部没有出错的情况下才自动跳回.
//		if (erritem!=0) {
//			websWrite(
//			                wp,
//			                T("<font color=red font-size:120%>"
//					                "<b>"CSTR_SET_ERR_FEILD":0x%X</b></font>"),
//			                erritem);
//		} else {
//			websWrite(wp, T("<font color=red font-size:120%>"
//					"<b>"CSTR_SET_ERR_FILE":%d</b></font>"),
//			                ret);
//		}
//	}
//	websWrite(wp, T("<form action=%s method=POST>"), PAGE_SYSTEM_PARAMETER);
//	websWrite(wp, T("<input type=submit name=return value=Return >"));
//	websWrite(wp, T("</form>\n"));
//	websWrite(wp, T("</p>\n"));
////body end
//	websWrite(wp, T("</body>\n"));
////页脚
//	websFooter(wp);
//	websDone(wp, 200);
	return;
}
/**
 * 表计参数设置表单提交触发事件,由meterpara.asp页面触发
 * @param wp 页面
 * @param path 路径
 * @param query 提交POST的字符串值
 */
static void form_set_mtrparams(webs_t wp, char_t *path, char_t *query)
{
//printf("formTest\n");
//printf("path:%s\n", path);
	printf("query:%s\n", query);
	///表计结构体参数数组
	stMtr amtr[256];
	mtr_param_print_item(wp);	///<debug
	int ret = -1;
	int i;
	stMtr mtr = { 0, { 0 }, { 0 }, { 0 }, 0 };
	int saveret = -1;
	u32 erritem = 0b000000000000000000;	//一共十八项
//操作
	char * str_cmd = websGetVar(wp, T("OpType"), T("0"));
	char * errstr = NULL;
	int cmd = strtoul(str_cmd, &errstr, 10);
	if (*errstr!='\0') {
		erritem |= 0b1;
	}
	printf("客户端操作分类:%d\n", cmd);
	switch (cmd) {
	case MTR_UPDATE:     //更新所有记录
		break;
	case MTR_ADD:     //增加1条记录
		load_mtrparam(&mtr, CFG_MTR, sysparam.meter_num);
		sysparam.meter_num += 1;
		ret = save_sysparam(&sysparam, CFG_SYS);
		printf("add one mtr number,ret %d \n", ret);
		ret = save_mtrparam(&mtr, CFG_MTR, sysparam.meter_num);
		printf("add a new mtr param record,ret= %d \n", ret);
		reflash_this_wp(wp, PAGE_METER_PARAMETER);
		return;
		break;
	case MTR_DEL:     //删除最后一条表参数数据
		sysparam.meter_num -= 1;
		ret = save_sysparam(&sysparam, CFG_SYS);
		printf("del(the last) one mtr number,ret %d \n", ret);
		reflash_this_wp(wp, PAGE_METER_PARAMETER);
		return;
		break;
	default:
		PRINT_RET(cmd)
		break;
	}
	//字符串获取完毕m,开始检查和转换成数值
	u32 e[MAX_MTR_NUM] = { 0 };	///<错误项
	int mtr_num = 0;	///<表计数目
	mtr_num = getmtrparams(amtr, wp, query, e);
	printf("get param from clint ret %d \n", mtr_num);
	if (mtr_num>0) {     //只有所有输入都合法
		for (i = 0; i<mtr_num; i++) {
			saveret = save_mtrparam(&amtr[i], CFG_MTR,
			                amtr[i].mtrno);
			printf("1047 i=%d saveret=%d\n", i, saveret);
			printf("amtr[i].mtrno=%d\n", amtr[i].mtrno);
		}
	} else {
		saveret = mtr_num;
	}
	/**
	 * TODO post 返回 测试等待确定
	 */
	websHeader_pure(wp);
	int no;
	//stMtr mtr = { 0, { 0 }, { 0 }, { 0 }, 0 };
	printf("%s\n", __FUNCTION__);
	for (no = 0; no<sysparam.meter_num; no++) {
		if (-1==load_mtrparam(&mtr, CFG_MTR, no)) {
			web_err_proc(EL);
			continue;
		}
		websWrite(wp, T("<tr>\n"));     //一行
		(void) webWrite_mtrno(wp, no);
		(void) webWrite_iv(wp, mtr);
		(void) webWrite_line(wp, mtr);
		(void) webWrite_mtraddr(wp, mtr);
		(void) webWrite_pwd(wp, mtr);
		(void) webWrite_uartport(wp, mtr);
		(void) webWrite_uartPlan(wp, mtr);
		(void) webWrite_mtr_protocol(wp, mtr);
		(void) webWrite_factory(wp, mtr);
		(void) webWrite_ph_wire(wp, mtr);
		(void) webWrite_it_dot(wp, mtr);
		(void) webWrite_xl_dot(wp, mtr);
		(void) webWrite_v_dot(wp, mtr);
		(void) webWrite_i_dot(wp, mtr);
		(void) webWrite_p_dot(wp, mtr);
		(void) webWrite_q_dot(wp, mtr);
		(void) webWrite_ue(wp, mtr);
		(void) webWrite_ie(wp, mtr);
		websWrite(wp, T("</tr>\n"));
	}
	websDone(wp, 200);
	return;
	/**
	 * 测试,等待删除 TODO
	 */
	reflash_this_wp(wp, PAGE_METER_PARAMETER);
	return;
	///@note 是否需要返回给用户信息? 待定
	//写页面
	websHeader(wp);     //头和尾完成了除head和body标签在内的东西
	//设置完成自动跳转回原来的页面.
	websWrite(wp, T("<head>\n"));
	websWrite(wp, T("<title>"CSTR_RETURN"</title>\n"));
	websWrite(wp, T("</head>\n"));
	websWrite(wp, T("<body>\n"));
	//段落1
	websWrite(wp, T("<p>\n"));
	websWrite(wp, T("%s"), (erritem&0b1) ? Fail : Success);
	websWrite(wp, T(CSTR_MTR_NO":\"%d\"</br>"), amtr[0].mtrno);
	websWrite(wp, T("%s"), (erritem&0b10) ? Fail : Success);
	websWrite(wp, T(CSTR_LINE_NAME":\"%s\"</br>"), amtr[0].line);
	websWrite(wp, T("%s"), (erritem&0b100) ? Fail : Success);
	websWrite(wp, T(CSTR_MTR_ADDR":\"%s\"</br>"), amtr[0].addr);
	websWrite(wp, T("%s"), (erritem&0b1000) ? Fail : Success);
	websWrite(wp, T(CSTR_MTR_PWD":\"%s\"</br>"), amtr[0].pwd);
	websWrite(wp, T("%s"), (erritem&0b10000) ? Fail : Success);
	websWrite(wp, T(CSTR_IT_DOT":\"%d\"</br>"), amtr[0].it_dot);
	websWrite(wp, T("%s"), (erritem&0b100000) ? Fail : Success);
	websWrite(wp, T(CSTR_V_DOT":\"%d\"</br>"), amtr[0].v_dot);
	websWrite(wp, T("%s"), (erritem&0b1000000) ? Fail : Success);
	websWrite(wp, T(CSTR_P_DOT":\"%d\"</br>"), amtr[0].p_dot);
	websWrite(wp, T("%s"), (erritem&0b10000000) ? Fail : Success);
	websWrite(wp, T(CSTR_Q_DOT":\"%d\"</br>"), amtr[0].q_dot);
	websWrite(wp, T("%s"), (erritem&0b100000000) ? Fail : Success);
	websWrite(wp, T(CSTR_I_DOT":\"%d\"</br>"), amtr[0].i_dot);
	websWrite(wp, T("%s"), (erritem&0b1000000000) ? Fail : Success);
	websWrite(wp, T(CSTR_XL_DOT":\"%d\"</br>"), amtr[0].xl_dot);
	websWrite(wp, T("%s"), (erritem&0b10000000000) ? Fail : Success);
	websWrite(wp, T(CSTR_UE":\"%d\"</br>"), amtr[0].ue);
	websWrite(wp, T("%s"), (erritem&0b100000000000) ? Fail : Success);
	websWrite(wp, T(CSTR_IE":\"%d\"</br>"), amtr[0].ie);
	websWrite(wp, T("%s"), (erritem&0b1000000000000) ? Fail : Success);
	websWrite(wp, T(CSTR_PROT":\"%d\"</br>"), amtr[0].port);
	websWrite(wp, T("%s"), (erritem&0b10000000000000) ? Fail : Success);
	websWrite(wp, T(CSTR_UART_NO":\"%d\"</br>"), amtr[0].portplan);
	websWrite(wp, T("%s"), (erritem&0b100000000000000) ? Fail : Success);
	websWrite(wp, T(CSTR_MTR_PROTOL":\"%d\"</br>"), amtr[0].protocol);
	websWrite(wp, T("%s"), (erritem&0b1000000000000000) ? Fail : Success);
	websWrite(wp, T(CSTR_MTR_TYPE":\"%d\"</br>"), amtr[0].p3w4);
	websWrite(wp, T("%s"),
	                (erritem&0b10000000000000000) ? Fail : Success);
	websWrite(wp, T(CSTR_FACT":\"%d\"</br>"), amtr[0].fact);
	websWrite(wp, T("%s"),
	                (erritem&0b100000000000000000) ? Fail : Success);
	websWrite(wp, T(CSTR_IV_FLAG":\"%d\"</br>"), amtr[0].iv);
	websWrite(wp, T("</p>\n"));
//段落2
	websWrite(wp, T("<p>\n"));
	if ((saveret==0)&&(erritem==0)) {     //只有在全部没有出错的情况下才自动跳回.
		websWrite(wp, T("<font color=green font-size:120%>"
				"<b>"CSTR_SET_OK"</b></font>"));
		websWrite(wp, T("<meta http-equiv=refresh content=\"1;"
				"url=/um/meterpara1.asp\">\n"));
	}     //只有在全部没有出错的情况下才自动跳回.
	if (erritem!=0) {
		websWrite(wp, T("<font color=red font-size:120%>"
				"<b>"CSTR_SET_ERR_FEILD":0x%X</b></font>"),
		                erritem);
	}
	if ((saveret!=0)) {
		websWrite(wp, T("<font color=red font-size:120%>"
				"<b>"CSTR_SET_ERR_FILE":%d</b></font>"),
		                saveret);
	}
	websWrite(wp, T("<form action=/um/meterpara1.asp method=POST>"));
	websWrite(wp, T("<input type=submit name=return value=Return >"));
	websWrite(wp, T("</form>\n"));
	websWrite(
	                wp,
	                T("<input type=text maxlength=12 name=line size=20 value=\""));
//	for (i = 0; i<LINE_LEN; i++) {
//		if (mtr.webWrite_line[i]!=0)
//			websWrite(wp, T("%1d"), mtr.webWrite_line[i]);
//	}
	websWrite(wp, T("\">"));
	websWrite(wp, T("</p>\n"));
	websWrite(wp, T("</body>\n"));
//页脚
	websFooter(wp);
	websDone(wp, 200);
	return;
}
/**
 *
 * @param wp
 * @param path
 * @param query
 */
static void form_set_savecycle(webs_t wp, char_t *path, char_t *query)
{
	//printf("query:%s\n", query);
	int i;
	int n;
	stSave_cycle sav[SAVE_CYCLE_ITEM];
	char *flags = websGetVar(wp, T("flag"), T("null"));
	char *cycle = websGetVar(wp, T("cycle"), T("null"));
	//printf(" flag:%s\n cycle:%s\n", flags, cycle);
	for (i = 0; i<SAVE_CYCLE_ITEM; i++) {
		n = sscanf(flags, "%hhu", &sav[i].enable);
		if (n!=1) {
			//web_err_proc(EL);
			break;
		}
		flags = point2next(&flags, ' ');
		n = sscanf(cycle, "%hhu", &sav[i].cycle);
		if (n!=1) {
			web_err_proc(EL);
			break;
		}
		cycle = point2next(&cycle, ' ');
	}
	save_savecycle(sav, CFG_SAVE_CYCLE);
	reflash_this_wp(wp, PAGE_SAVECYCLE_PARAMETER);
	return;
}
/**
 * 提交表单,历史电量数据.操作:获取.参数:时间范围,表号.
 * @param wp
 * @param path
 * @param query
 */
static void form_history_tou(webs_t wp, char_t *path, char_t *query)
{
	printf("%s ***query:%s\n", __FUNCTION__, query);
	//PRINT_HERE
	char * strmtr_no = websGetVar(wp, T("mtr_no"), T("0"));
	char * stime_t = websGetVar(wp, T("stime_stamp"), T("0"));
	char * etime_t = websGetVar(wp, T("etime_stamp"), T("0"));
	printf("时间戳范围:%s~%s\n", stime_t, etime_t);
	TimeRange tr;
	int ret;
	//int tou_test=1100;
	int mtr_no = 0;
	stTou tou;
	memset(&tou, 0x00, sizeof(stTou));
	ret = sscanf(strmtr_no, "%d", &mtr_no);
	if (ret!=1) {
		web_err_proc(EL);
	}
	ret = sscanf(stime_t, "%ld", &tr.s);
	if (ret!=1) {
		web_err_proc(EL);
	}
	ret = sscanf(etime_t, "%ld", &tr.e);
	if (ret!=1) {
		web_err_proc(EL);
	}
	//PRINT_HERE
	printf("时间戳 (数值) 范围:%ld~%ld 表号:%d\n", tr.s, tr.e, mtr_no);
#if __arm__ ==2
//	tr.s+=8*60*60;
//	tr.e+=8*60*60;
//	printf("时间戳校正(数值)范围:%ld~%ld\n", tr.s, tr.e);
#endif
	websHeader_pure(wp);
	ret = load_tou_dat(mtr_no, tr, &tou, wp);
	if (ret==ERR) {
		web_err_proc(EL);
	}
	//websFooter(wp);
	websDone(wp, 200);
	return;
}
/**
 * 接收客户端的日志文件
 * @param wp
 * @param path
 * @param query
 */
static void form_save_log(webs_t wp, char_t *path, char_t *query)
{
//	printf("%s:%s\n", __FUNCTION__, query);
//	websWrite(wp, T("HTTP/1.0 200 OK\n"));
//	char * txt = query;
//	FILE*fp = fopen(ERR_LOG, "w");
//	if (fp==NULL) {
//		return;
//	}
//	fwrite(txt, strlen(txt), 1, fp);
//	fclose(fp);
//	websDone(wp, 200);
//	return;
	save_file(wp, path, query, ERR_LOG);
}
static void form_save_monport_cfgfile(webs_t wp, char_t *path, char_t *query)
{
	save_file(wp, path, query, MON_PORT_NAME_FILE);
}
void form_save_procotol_cfgfile(webs_t wp, char_t *path, char_t *query)
{
	save_file(wp, path, query, PORC_FILE);
}
void save_file(webs_t wp, char_t *path, char_t *query, const char* file)
{
	printf("%s:%s\n", __FUNCTION__, query);
	websWrite(wp, T("HTTP/1.0 200 OK\n"));
	char * txt = query;
	FILE*fp = fopen(file, "w");
	if (fp==NULL) {
		return;
	}
	fwrite(txt, strlen(txt), 1, fp);
	fclose(fp);
	websDone(wp, 200);
	return;
}
/**
 * 加载日志文件到客户端
 * @param wp
 * @param path
 * @param query
 */
static void form_load_log(webs_t wp, char_t *path, char_t *query)
{
	//printf("%s:%s\n", __FUNCTION__, query);
	load_file(wp, path, query, ERR_LOG);
	return;
}
void form_load_procotol_cfgfile(webs_t wp, char_t *path, char_t *query)
{
	//printf("%s:%s\n", __FUNCTION__, query);
	load_file(wp, path, query, PORC_FILE);
}
static void form_load_monport_cfgfile(webs_t wp, char_t *path, char_t *query)
{
	//printf("%s:%s\n", __FUNCTION__, query);
	load_file(wp, path, query, MON_PORT_NAME_FILE);
}
void load_file(webs_t wp, char_t *path, char_t *query, const char*file)
{

	//websWrite(wp, T("HTTP/1.0 200 OK\n"));
	websHeader_pure(wp);
	char buf[1024] = { 0 };
	int ret;
	FILE*fp = fopen(file, "r");
	if (fp==NULL) {
		websWrite(wp, T("No info."));
		goto WEB_END;
	}
	while (1) {
		ret = fread(&buf, sizeof(char), 1024, fp);
		if (ret>0) {
			websWrite(wp, T("%s"), buf);
		} else {
			break;
		}
	}
//	while (ftell(fp) < flen) {
	fclose(fp);
	WEB_END:
	//websFooter(wp);
	websDone(wp, 200);
	return;
}
/**
 * 报文监视 表单提交处理函数
 * @param wp
 * @param path
 * @param query
 */
void form_msg(webs_t wp, char_t *path, char_t *query)
{
	printf("%s:%s\n", __FUNCTION__, query);
	pid_t pid;
	is_monmsg = 1;
	if ((pid = fork())==0) {
		websHeader_pure(wp);
		FILE* pf;
		char line[256] = { 0 };
		pf = popen(query, "r");
		if (pf==NULL) {
			perror("open ping:");
			return;
		}
//		is_monmsg = 0;
//		sb.sem_num = 0;
//		sb.sem_op = 0;
		printf("信号量 s =%d\n", semctl(semid, 0, GETVAL, 0));
		while (fgets(line, 256-1, pf)
		                &&(semctl(semid, 0, GETVAL, 0))) {
//			sb.sem_num = 1;     //将1号信号量
//			sb.sem_op = -1;     //减1
//			sb.sem_flg = sb.sem_flg&~IPC_NOWAIT;
//			semop(semid, &sb, 1);
			printf("%s", line);
			websWrite(wp, T("%s"), line);
			//printf("is_monmsg:%d \n", is_monmsg);
			//websDone(wp, 200);
			//websTimeoutCancel(wp);
			//socketSetBlock(wp->sid, 1);
			//socketFlush(wp->sid);
			//socketCloseConnection(wp->sid);
		}
		///如果是点击停止使之退出的,那么信号量现在是0,
		///为了下次使用,加1.如果自然结束退出,信号量还是1不变.
		if (semctl(semid, 0, GETVAL, 0)==0) {
			printf("点击停止\n");
			is_monmsg = 0;
			sb.sem_num = 0;
			sb.sem_op = 1;
			sb.sem_flg = sb.sem_flg&~IPC_NOWAIT;
			semop(semid, &sb, 1);
		}
		websDone(wp, 200);
		pclose(pf);
	}
}
void form_msg_stop(webs_t wp, char_t *path, char_t *query)
{
	websHeader_pure(wp);
	printf("%s:%s\n", __FUNCTION__, query);
	sb.sem_num = 0;     //将0号信号量
	sb.sem_op = -1;     //减1
	sb.sem_flg = sb.sem_flg&~IPC_NOWAIT;
	semop(semid, &sb, 1);     //操作信号量
//	is_monmsg = 0;
//	sb.sem_num = 1;
//	sb.sem_op = 1;
//	sb.sem_flg = sb.sem_flg&~IPC_NOWAIT;
//	semop(semid, &sb, 1);
	printf("信号量 s :%d \n", semctl(semid, 0, GETVAL, 0));
	websWrite(wp, T("ok"));
	websDone(wp, 200);
}
/**
 * 以split为分割字符,指向下一个项.
 * 例如空格为分隔符."1 2 3"输入,"2 3"输出.不能排除多个连续的分隔符.
 * @param[out] s 输入待分割的字符,输出指针指向下一项
 * @param[in] split 以此为分隔符分割字符串.
 * @retval 指向下一相的指针
 */
char * point2next(char** s, const char split)
{
	while (**s!='\0') {
		if (* ++(*s)==split) {
			(*s)++;
			break;
		}
	}
	return *s;
}
/******************************************************************************/
/*
 *	Home page handler
 */

static int websHomePageHandler(webs_t wp, char_t *urlPrefix, char_t *webDir,
        int arg, char_t *url, char_t *path, char_t *query)
{
	/*
	 *	If the empty or "/" URL is invoked, redirect default URLs to the home page
	 */
	if (*url=='\0'||gstrcmp(url, T("/"))==0) {
		websRedirect(wp, WEBS_DEFAULT_HOME);
		return 1;
	}
	return 0;
}
void print_array(const u8 *a, const int len)
{
	int i;
	printf("[%d] ", len);
	for (i = 0; i<len; i++) {
		printf("%02X ", a[i]);
	}
	printf("\n");
	return;
}
