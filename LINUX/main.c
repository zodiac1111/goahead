/**
 * @file main.c
 * main.c -- Main program for the GoAhead WebServer (LINUX version)
 * Copyright (c) GoAhead Software Inc., 1995-2010. All Rights Reserved.
 * See the file "license.txt" for usage and redistribution license requirements
 * @todo 尽量只操作处理数据,样式和行为应该交给前端控制.[表计参数和储存周期部分未完成]
 * @todo 给前端发送的数据可以使用JSON.接收前端的数据就还是使用goahead的getVal吧.
 * 	后端仅进行一些必要的数值合法性验证.
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
#include <netdb.h>
#include "param.h"
#include "main.h"
#include "Chinese_string.h"
#include "web_err.h"
#include "tou.h"
#include "conf.h"
#ifdef WEBS_SSL_SUPPORT
#include	"../websSSL.h"
#endif
#ifdef USER_MANAGEMENT_SUPPORT
#include	"../um.h"
void formDefineUserMgmt(void);
#endif
//Change configuration here
static char_t *password = T(""); /* Security password */
static int port = WEBS_DEFAULT_PORT;/* Server port */
static int retries = 5; /* Server port retries */
static int finished = 0; /* Finished flag */
#ifdef B_STATS
static void printMemStats(int handle, char_t *fmt, ...);
static void memLeaks();
#endif
///内存中的系统参数结构体.全局使用.
stSysParam sysparam = { 0 };
///规约文件中的规约名称.
static char *procotol_name[MAX_PROCOTOL_NUM];
///规约文件中的实际规约数,初始化为最大
static int procotol_num = MAX_PROCOTOL_NUM;
///规约文件中的规约名称.
static char *mon_port_name[MAX_MON_PORT_NUM];
///规约文件中的实际规约数,初始化为最大
static int mon_port_num = MAX_MON_PORT_NUM;
struct sembuf sb;     ///<信号量操作
union semun sem;     ///<用于控制报文监视停止的信号量.0停止监视程序,1运行监视程序
int semid;     ///<信号量id
static char* errlog=NULL; ///配置项(错误日志文件路径)
#define JSON 1
//#pragma  GCC diagnostic warning  "-Wunused-parameter"
/**
 * webs主函数,所有业务逻辑在此实现.
 */
int main(int argc __attribute__ ((unused)),
        char** argv __attribute__ ((unused)))
{
	print_ip();
	PRINT_WELCOME
	PRINT_VERSION
	PRINT_BUILD_TIME
	init_semun();     //初始化信号量,用于控制,未完善.
	/*
	 * Initialize the memory allocator. Allow use of malloc and start
	 * with a 60K heap.  For each page request approx 8KB is allocated.
	 * 60KB allows for several concurrent page requests.  If more space
	 * is required, malloc will be used for the overflow.
	 * 初始化并分配内存,如果内存不足可以在这里多分配一些.
	 */
	bopen(NULL, (60*1024), B_USE_MALLOC);
	signal(SIGPIPE, SIG_IGN );
	signal(SIGINT, sigintHandler);
	signal(SIGTERM, sigintHandler);

	//Initialize the web server 初始化web服务器
	if (initWebs()<0) {
		printf(WEBS_ERR"init Webs.\n");
														return -1;
	}
#ifdef WEBS_SSL_SUPPORT
	printf(WEBS_INF"SSL support\n");
	websSSLOpen();
	/* websRequireSSL("/"); *//* Require all files be served via https */
#endif
	/* 基本事件循环.
	 * Basic event loop. SocketReady returns true when a socket is ready for
	 * service. SocketSelect will block until an event occurs. SocketProcess
	 * will actually do the servicing.
	 */
	finished = 0;
	printf(WEBS_INF"Initialization is complete.\t[\e[32mOK\e[0m]\n");
	printf(WEBS_INF"All configure is OK.\t[\e[32mOK\e[0m]\n");
	printf(WEBS_INF"Now access \e[32m\033[4mhttp://<IP>:%d\e[0m"
		"with Browser.\n",WEBS_DEFAULT_PORT);
	while (!finished) {
		//PRINT_HERE
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
	//Close the socket module,
	//report memory leaks and close the memory allocator
	websCloseServer();
	socketClose();
#ifdef B_STATS
	memLeaks();
#endif
	bclose();
	webs_free();
	return 0;
}
/**
 * 系统参数设置表单提交触发的函数.
 * 判断数据合法,写入到sysspara.cfg文件中.一共一项,大小7字节,脉冲数目已经废弃.
 * @param wp
 * @param path
 * @param query
 */
void form_sysparam(webs_t wp, char_t *path, char_t *query)
{
	PRINT_FORM_INFO;
	websHeader_pure(wp);
	char * action = websGetVar(wp, T("action"), T("null"));
	if (strcmp(action,"init")==0) {
		webSend_syspara(wp);
	} else {
		webRece_syspara(wp, &sysparam);
	}
	websDone(wp, 200);
	return;
}
/**
 * 客户端请求获取服务端时间.
 * 只要是post发送到这个函数就同步时间,不管query
 * @bug arm32处理器 t最大32位,将于2038年01月19日03时14分07秒 溢出
 * @param wp
 * @param path
 * @param query
 */
void form_server_time(webs_t wp, char_t *path, char_t *query)
{
	PRINT_FORM_INFO;
	time_t t = time(NULL );
	printf("\t\t@%s", ctime(&t));
	websHeader_pure(wp);
	char * action = websGetVar(wp, T("action"), T("null"));
	if(strcmp(action,"get")==0){
		///@todo 时间处理的函数
	}
	websWrite(wp, T("{\"timestamp\":\"%d\"}"), t);
	websDone(wp, 200);
	return;
}
/**
 * 串口方案表单提交触发函数
 * @param wp
 * @param path
 * @param query
 */
void form_sioplans(webs_t wp, char_t *path, char_t *query)
{
	PRINT_FORM_INFO;
	websHeader_pure(wp);
	char * action = websGetVar(wp, T("action"), T("null"));
	if (strcmp(action,"init")==0) {
		webSend_sioplans(wp, sysparam);
	} else if(strcmp(action,"set")==0) {
		webRece_sioplans(wp);
	} else {
		web_err_proc(EL);
	}
	websDone(wp, 200);
	return;
}
/**
 * 网口参数(多个)表单提交处理函数
 * @param wp
 * @param path
 * @param query
 */
void form_netparas(webs_t wp, char_t *path, char_t *query)
{
	PRINT_FORM_INFO;
	websHeader_pure(wp);
	char * init = websGetVar(wp, T("init"), T("null"));
	if (*init=='1') {
		webSend_netparas(wp, sysparam.netports_num);
	} else {
		webRece_netparas(wp);
	}
	websDone(wp, 200);
	return;
}
/**
 * 表计参数项目表头表单提交函数.
 * 列举:
 *  所有的规约名称(用于规约选择)
 *  所有的串口方案名称(用于选择串口方案)
 *  所有的生产厂家名称(用于选择表计的厂家)
 * @todo 数据格式化
 * @param wp
 * @param path
 * @param query
 */
void form_mtr_items(webs_t wp, char_t *path, char_t *query)
{
	PRINT_FORM_INFO;
	websHeader_pure(wp);
	char * item = websGetVar(wp, T("item"), T("null"));
	if (strcmp(item, "sioplan")==0) {
		webSend_mtr_sioplan(wp, sysparam);
	} else if (strcmp(item, "procotol")==0) {
		webSend_mtr_procotol(wp);
	} else if (strcmp(item, "factory")==0) {
		webSend_mtr_factory(wp);
	} else if (strcmp(item, "type")==0) {
		webSend_mtr_type(wp);
	}/* 在这里添加其他需要的项目类型 */
	websDone(wp, 200);
	return;
}
/**
 * 表计参数设置表单提交触发事件,由meterpara.asp页面触发
 * @param wp 页面
 * @param path 路径
 * @param query 提交POST的字符串值
 */
void form_mtrparams(webs_t wp, char_t *path, char_t *query)
{
	PRINT_FORM_INFO;
	websHeader_pure(wp);
	char * init = websGetVar(wp, T("init"), T("null"));
	if (*init=='1') {
		webSend_mtrparams(wp, sysparam.meter_num);
	} else {
		webRece_mtrparams(wp);
	}
	websDone(wp, 200);
	return;
}
/**
 * 监视端口 表单提交触发函数
 * @param wp
 * @param path
 * @param query
 */
void form_monparas(webs_t wp, char_t *path, char_t *query)
{
	PRINT_FORM_INFO;
	websHeader_pure(wp);
	char * init = websGetVar(wp, T("init"), T("null"));
	if (*init=='1') {
		webSend_monparas(wp, sysparam);
	} else {
		webRece_monparas(wp);
	}
	websDone(wp, 200);
	return;
}
/**
 * 表单提交函数,储存周期.
 * @param wp
 * @param path
 * @param query
 */
void form_savecycle(webs_t wp, char_t *path, char_t *query)
{
	PRINT_FORM_INFO;
	websHeader_pure(wp);
	char * init = websGetVar(wp, T("init"), T("null"));
	if (*init=='1') {
		(void) webSend_savecycle(wp);
	} else {
		(void) webRece_savecycle(wp);
	}
	websDone(wp, 200);
	return;
}
/**
 * 客户端发送重启表单,分类重启.
 * @todo 配合的更好一些,参考路由器的页面行为设计.前端一个更舒适的行为反馈.
 * @param wp
 * @param path
 * @param query
 */
void form_reset(webs_t wp, char_t *path, char_t *query)
{
#define REINIT_PROTOCOL_FILE 1
#define RET_WEB 2
#define RET_SAMPLE_PROC 3
#define RET_RTU 4
#define RET_TEST 10
	PRINT_FORM_INFO;
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
			web_err_proc(EL);
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
			execl(app, app, NULL );
			exit(0);
		}
		if (pid>0) {

		}
		break;
	case RET_SAMPLE_PROC:///@待定
		system("killall hl3104_com ");
		break;
	case RET_RTU:
		reflash_this_wp(wp, PAGE_RESET);
#if __i386 == 1
		//调试不要重启PC系统...
		system("echo \"reboot ok\"");
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
 * 提交表单,历史电量数据.操作:获取.参数:时间范围,表号.
 * @param wp
 * @param path
 * @param query
 */
void form_history_tou(webs_t wp, char_t *path, char_t *query)
{
	PRINT_FORM_INFO;
	char * strmtr_no = websGetVar(wp, T("mtr_no"), T("0"));
	char * stime_t = websGetVar(wp, T("stime_stamp"), T("0"));
	char * etime_t = websGetVar(wp, T("etime_stamp"), T("0"));
	//printf("时间戳范围:%s~%s\n", stime_t, etime_t);
	TimeRange tr;
	int ret;
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
	//printf("时间戳 (数值) 范围:%ld~%ld 表号:%d\n", tr.s, tr.e, mtr_no);
	websHeader_pure(wp);
	ret = load_tou_dat(mtr_no, tr, &tou, wp);
	if (ret==ERR) {
		web_err_proc(EL);
	}
	websDone(wp, 200);
	return;
}

/// 接收客户端的日志文件
void form_save_log(webs_t wp, char_t *path, char_t *query)
{
	PRINT_FORM_INFO;
	webRece_txtfile(wp, query, ERR_LOG);
	return;
}
void form_save_monport_cfgfile(webs_t wp, char_t *path, char_t *query)
{
	PRINT_FORM_INFO;
	webRece_txtfile(wp, query, MON_PORT_NAME_FILE);
	return;
}
void form_save_procotol_cfgfile(webs_t wp, char_t *path, char_t *query)
{
	PRINT_FORM_INFO;
	webRece_txtfile(wp, query, PORC_FILE);
	return;
}
/**
 * 加载日志文件到客户端
 * @param wp
 * @param path
 * @param query
 */
void form_load_log(webs_t wp, char_t *path, char_t *query)
{
	PRINT_FORM_INFO;
	webSend_txtfile(wp, ERR_LOG);
	return;
}
void form_load_procotol_cfgfile(webs_t wp, char_t *path, char_t *query)
{
	PRINT_FORM_INFO;
	webSend_txtfile(wp, PORC_FILE);
	return;
}
void form_load_monport_cfgfile(webs_t wp, char_t *path, char_t *query)
{
	PRINT_FORM_INFO;
	webSend_txtfile(wp, MON_PORT_NAME_FILE);
	return;
}
/**
 * 报文监视(执行指令) 表单提交处理函数.
 * @todo:未实现,执行命令的输出还不能读取.前后端可能需要频繁交互.
 * @param wp
 * @param path
 * @param query
 */
void form_msg(webs_t wp, char_t *path, char_t *query)
{
	PRINT_FORM_INFO;
	printf(WEBS_WAR"This function is being debugged.\n");
	pid_t pid;
	if ((pid = fork())==0) {
		websHeader_pure(wp);
		FILE* pf;
		char line[256] = { 0 };
		pf = popen(query, "r");
		if (pf==NULL ) {
			perror("open ping:");
			return;
		}
		printf("信号量 s =%d\n", semctl(semid, 0, GETVAL, 0));
		while (fgets(line, 256-1, pf)
		                &&(semctl(semid, 0, GETVAL, 0))) {

			printf("%s", line);
			websWrite(wp, T("%s"), line);
		}
		///如果是点击停止使之退出的,那么信号量现在是0,
		///为了下次使用,加1.如果自然结束退出,信号量还是1不变.
		if (semctl(semid, 0, GETVAL, 0)==0) {
			printf("点击停止\n");
			sb.sem_num = 0;
			sb.sem_op = 1;
			sb.sem_flg = sb.sem_flg&~IPC_NOWAIT;
			semop(semid, &sb, 1);
		}
		websDone(wp, 200);
		pclose(pf);
	}
}
/**
 * 监控报文(执行指令)停止函数.使用信号量控制.
 * @todo: 未实现,中止进程的方法待考虑.
 * @param wp
 * @param path
 * @param query
 */
void form_msg_stop(webs_t wp, char_t *path, char_t *query)
{
	PRINT_FORM_INFO;
	printf(WEBS_WAR"This function is being debugged.\n");
	websHeader_pure(wp);
	sb.sem_num = 0;     //将0号信号量
	sb.sem_op = -1;     //减1
	sb.sem_flg = sb.sem_flg&~IPC_NOWAIT;
	semop(semid, &sb, 1);     //操作信号量
	printf("信号量 s :%d \n", semctl(semid, 0, GETVAL, 0));
	websWrite(wp, T("ok"));
	websDone(wp, 200);
}
/**
 * 打印服务器应用程序运行路径,依赖proc文件系统.
 * @return
 */
int printf_webs_app_dir(void)
{
	char dir[128] = { 0 };
	///@todo 像其他服务器一样使用配置文件配置web服务器的根目录.
	int count = readlink("/proc/self/exe", dir, 128);
	if (count<0||count>128) {
		PRINT_RET(count);
		printf(WEBS_ERR"%s\n", __FUNCTION__);
	}
	printf(WEBS_INF"App dir:\e[32m%s\e[0m\n", dir);
	return 0;
}

/** 通用读取配置文件项函数.
 * 读取类似 name = value 这样的配置项
 * 输入一个项目名称字符串,输出这个项目的值(字符串)
 * 暂时配有被使用,因为需要修改很多关联的地方.
 * 用完要free!
 * 例子:
 *	char* val = getconf("paradir");
 *	if (val==NULL) {
 *		printf("paradir is not set.");
 *	}
 * @param name 名字,const
 * @param value 值的指针,函数内部修改
 * @retval NULL 没有这个项,或则其他错误情况
 * @retval 其他 指向一个字符串的指针.这个字符串即项的值
 * @return
 */
char* getconf(const char const* name,char** value)
{
	//配置文件定义为 一行一条, 以 变量名=变量值的形式
	/** @bug 为了方便起见在栈上分配固定大小内存用于存储配置字符串.
	 * 限制了配置项的长度,且过长可能溢出.更好的方式是使用malloc在堆上分配,
	 * realloc动态调整大小,能够使适用范围更广.
	 */
	//char* value=NULL;
	char line[256] = { 0 };
	char n[256] = { 0 };
	char v[256] = { 0 };
	char *pname = NULL;
	char *pvalue = NULL;
	int strnum = 0;

	FILE* fp = fopen(CONF_FILE, "r");
	if (fp==NULL ) {
		perror(WEBS_ERR"open file goahead.conf");
		return NULL;
	}
	while (!feof(fp)) {
		memset(&line, 0x00, 256);
		memset(&n, 0x00, 256);
		memset(&v, 0x00, 256);
		fgets(line, 255, fp);     //得到一行
		//得到这一行的字符串,根据表达式截断
		strnum = sscanf(line, "%[^#=]=%[^#\r\n]", n, v);
		if (strnum!=2) {
			continue;
		}
		pname = trim(n, strlen(n));//去除前导和后导空白符
		pvalue = trim(v, strlen(v));
		if (strcmp(pname, name)==0) {
			*value=(char*)malloc(strlen(pvalue)+1);
			strcpy(*value, pvalue);
		}
	}
	fclose(fp);
	printf(WEBS_INF"Item \e[33m%s\e[0m = \e[32m%s\e[0m\n"
		, name,*value);
	return *value;
}
/**
 * Initialize the web server.
 * 初始化web服务的一些操作:
 * * 配置套接字
 * * 修改运行目录和配置www根目录
 * * 注册asp和form函数
 * @todo:(doing)分解成较短的函数
 * @return
 */
static int initWebs(void)
{
	struct hostent *hp;
	struct in_addr intaddr;
	char host[128];
	//char webdir[128];
	char * webdir;
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
	if ((hp = gethostbyname(host))==NULL ) {
		error(E_L, E_LOG, T("Can't get host address"));
		return -1;
	}
	memcpy((char *) &intaddr, (char *) hp->h_addr_list[0],
	                (size_t) hp->h_length);

	(void) printf_webs_app_dir();
	if(getconf("wwwroot",&webdir)==NULL){
		return -2;
	}
	if(getconf("errlog",&errlog)==NULL){
			return -2;
	}
	//(void) load_web_root_dir(webdir);	//获取根目录
	///改变程序的当前目录,所有相对路径都是相对当前目录的.当前目录为www(demo)目录
	///必须使用绝对路径启动程序,传入argv[0]的是/mnt/nor/bin/webs这样的路径
	///因为web根目录需要
	chdir(webdir);
	//Configure the web server options before opening the web server
	websSetDefaultDir(webdir);
	if(webdir!=NULL)free(webdir);
	cp = inet_ntoa(intaddr);
	ascToUni(wbuf, cp, min(strlen(cp) + 1, sizeof(wbuf)));
	websSetIpaddr(wbuf);
	ascToUni(wbuf, host, min(strlen(host) + 1, sizeof(wbuf)));
	websSetHost(wbuf);
	//Configure the web server options before opening the web server
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
	}
	if (-1==read_protocol_file(procotol_name, &procotol_num, PORC_FILE)) {
		web_err_proc(EL);
	}
	/**
	 * 注册asp函数,给予asp调用
	 */
	///form define/用于post
	websFormDefine(T("srv_time"), form_server_time);
	websFormDefine(T("mtrparams"), form_mtrparams);
	///@todo 表计参数的数据项,若使用json可以自描述就不需要描述项了
	websFormDefine(T("mtr_items"), form_mtr_items);
	websFormDefine(T("sysparam"), form_sysparam);
	websFormDefine(T("sioplan"), form_sioplans);
	websFormDefine(T("netpara"), form_netparas);
	websFormDefine(T("monparas"), form_monparas);
	websFormDefine(T("savecycle"), form_savecycle);
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
#ifdef USER_MANAGEMENT_SUPPORT
	//Create the Form handlers for the User Management pages
	formDefineUserMgmt();
#endif
	//Create a handler for the default home page
	websUrlHandlerDefine(T("/"), NULL, 0, websHomePageHandler, 0);
	///加载系统参数
	if (-1==load_sysparam(&sysparam, CFG_SYS)) {
		web_err_proc(EL);
		//return -1;
	}
	return 0;
}
/**
 * 写终端地址4个数字字符到页面.
 * @param wp
 * @param monport
 * @return
 */
static int webWrite_rtu_addr(webs_t wp, stMonparam monport)
{
#if DEBUG_PRINT_MONPARAM
	printf("监视参数-终端地址:%d%d%d%d\n", monport.prot_addr[0],
			monport.prot_addr[1], monport.prot_addr[2],
			monport.prot_addr[3]);
#endif
	int i;
	websWrite(wp, T("\"rtu_addr\":\""));
	for (i = 0; i<4; i++) {
		websWrite(wp, T("%1d"), monport.prot_addr[i]);
	}
	websWrite(wp, T("\""));
	return 0;
}
///主站规约类型
static int webWrite_porttype(webs_t wp)
{
#if DEBUG_PRINT_MONPARAM
	printf("监视参数-端口类型?:%x\n", monport.sioplan);
#endif
	int i;
	websWrite(wp, T("\"protocol\":["));
	for (i = 0; i<procotol_num; i++) {
		websWrite(wp, T("\"%s\""), procotol_name[i]);
		if (i!=procotol_num-1) {
			websWrite(wp, T(","));
		}
	}
	websWrite(wp, T("]"));
	return 0;

}
static int webWrite_listen_port(webs_t wp, stMonparam monport)
{
	int i;
#if DEBUG_PRINT_MONPARAM
	printf("监视参数-监听端口:");
	for (i = 0; i<5; i++) {
		printf("%d", monport.listen_port[i]);
	}
	printf("\n");
#endif
	websWrite(wp, T("\"listenport\":\""));
	for (i = 0; i<5; i++) {
		websWrite(wp, T("%1d"), monport.listen_port[i]);
	}
	websWrite(wp, T("\""));
	return 0;
}

static int webWrite_commportList(webs_t wp)
{
#if DEBUG_PRINT_MONPARAM
	printf("监视参数-使用端口:%x\n", monport.comm_port);
#endif
	int i;
	websWrite(wp, T("\"commport\":["));
	for (i = 0; i<mon_port_num; i++) {
		websWrite(wp, T("\"%s\""), mon_port_name[i]);
		if (i!=mon_port_num-1) {
			websWrite(wp, T(","));
		}
	}
	websWrite(wp, T("]"));
	return 0;

}
/**
 * 填写形如 "ip":"111.222.333.444" 的json字符串
 * @param[out] wp 写入到这个页面
 * @param[in] name 变量名,如上面的ip,可取ip,mask,gateway
 * @param[in] value 变量值,如上面的111.222.333.444
 * @retval 0 成功
 */
static int webWrite_ip(webs_t wp, char *name, u8* value)
{
	//printf("网口参数-IP\n");
	int i;
	websWrite(wp, T("\"%s\":\""), name);
	for (i = 0; i<IPV4_LEN; i++) {
		websWrite(wp, T("%1d"), value[i]);
		if (((i+1)%3==0)&&(i!=11)) {     //aaa.bbb.ccc.ddd
			websWrite(wp, T("."));
		}
	}
	websWrite(wp, T("\""));
	return 0;
}
///线路名称
static int webWrite_line(webs_t wp, stMtr mtr)
{
#if DEBUG_PRINT_MTRPARAM
	printf("\t线路名称\n");
#endif
	websWrite(wp, T("<td>\n"
			"<input type=text  maxlength=6 "
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
#if DEBUG_PRINT_MTRPARAM
	printf("\t表计地址\n");
#endif
	int i;
	websWrite(wp, T("<td>\n"
			"<input type=text  maxlength=12 "
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
#if DEBUG_PRINT_MTRPARAM
	printf("\t表计口令\n");
#endif
	int i;
	websWrite(wp, T("<td>\n"
			"<input type=text maxlength=8 "
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
#if DEBUG_PRINT_MTRPARAM
	printf("\t电量小数位数:%u\n", mtr.it_dot);
#endif
	return websWrite(wp, T("<td>\n"
			" <input type=text  maxlength=1 "
			" onchange=\"dot_changed(event);\" "
			" name=it_dot value=%u>\n</td>\n "), mtr.it_dot);
}
/// 电压小数位数
static int webWrite_v_dot(webs_t wp, stMtr mtr)
{
#if DEBUG_PRINT_MTRPARAM
	printf("\t电压小数位数:%u\n", mtr.v_dot);
#endif
	return websWrite(wp, T("<td>\n"
			" <input type=text  maxlength=1  "
			" onchange=\"dot_changed(event);\" "
			" name=v_dot value=%u >\n</td>\n "), mtr.v_dot);

}
/// 电流小数位数
static int webWrite_i_dot(webs_t wp, stMtr mtr)
{
#if DEBUG_PRINT_MTRPARAM
	printf("\t电流小数位数:%u\n", mtr.i_dot);
#endif
	return websWrite(wp, T("<td>\n"
			"<input type=text maxlength=1  "
			" onchange=\"dot_changed(event);\" "
			"name=i_dot value=%u>\n</td>\n "), mtr.i_dot);
}
/// 有功功率小数位数
static int webWrite_p_dot(webs_t wp, stMtr mtr)
{
#if DEBUG_PRINT_MTRPARAM
	printf("\t有功功率小数:%u\n", mtr.p_dot);
#endif
	return websWrite(wp, T("<td>\n"
			"<input  type=text  maxlength=1 "
			" onchange=\"dot_changed(event);\" "
			"name=p_dot value=%u>\n</td>\n "), mtr.p_dot);
}
/// 无功功率小数位数
static int webWrite_q_dot(webs_t wp, stMtr mtr)
{
#if DEBUG_PRINT_MTRPARAM
	printf("\t无功功率小数:%u\n", mtr.q_dot);
#endif
	return websWrite(wp, T("<td>\n"
			" <input  type=text  maxlength=1 "
			" onchange=\"dot_changed(event);\" "
			" name=q_dot value=%u>\n</td>\n "), mtr.q_dot);
}
/// 需量小数位数
static int webWrite_xl_dot(webs_t wp, stMtr mtr)
{
#if DEBUG_PRINT_MTRPARAM
	printf("\t需量小数:%u\n", mtr.xl_dot);
#endif
	return websWrite(wp, T("<td>\n"
			" <input type=text  maxlength=1 "
			" onchange=\"dot_changed(event);\" "
			" name=xl_dot value=%u>\n</td>\n "), mtr.xl_dot);

}
/// 额定电压
static int webWrite_ue(webs_t wp, stMtr mtr)
{
#if DEBUG_PRINT_MTRPARAM
	printf("\t额定电压:%u\n", mtr.ue);
#endif
	return websWrite(wp, T("<td>\n"
			" <input type=text  "
			" onchange=\"ue_changed(event);\" "
			" name=ue value=%u>\n</td>\n "), mtr.ue);
}
/// 额定电流
static int webWrite_ie(webs_t wp, stMtr mtr)
{
#if DEBUG_PRINT_MTRPARAM
	printf("\t额定电压:%u\n", mtr.ie);
#endif
	return websWrite(wp, T("<td>\n"
			" <input type=text size=4 "
			" onchange=\"ie_changed(event);\" "
			" name=ie value=%u>\n</td>\n "), mtr.ie);
}
/// 使用的串口号
static int webWrite_uartport(webs_t wp, stMtr mtr)
{
	int i;
#if DEBUG_PRINT_MTRPARAM
	printf("\t使用串口号:%d\n", mtr.port);
#endif
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
#if DEBUG_PRINT_MTRPARAM
	printf("\t串口方案号:%d,总串口个数%d个\n", mtr.portplan, sysparam.sioplan_num);
#endif
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
#if DEBUG_PRINT_MTRPARAM
	printf("\t表计规约:%d\n", mtr.protocol);
#endif
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
/// 几相几线制,就两种情况 0-3相3线 1-3相4线
static int webWrite_ph_wire(webs_t wp, stMtr mtr)
{
#if DEBUG_PRINT_MTRPARAM
	printf("\t几相几线:%d\n", mtr.p3w4);
#endif
	u32 i;
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
/// 生产厂家 @todo: 现在是硬编码,期待使用文件配置.
static int webWrite_factory(webs_t wp, stMtr mtr)
{
	u32 i;
	char *fact[] = { HOLLEY, WEI_SHENG, LAN_JI_ER, HONG_XIANG, "other" };
#if DEBUG_PRINT_MTRPARAM
	printf("\t生产厂家:%d\n", mtr.fact);
#endif
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
#if DEBUG_PRINT_MTRPARAM
	printf("\t有效标志:%x\n", mtr.iv);
#endif
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
#if DEBUG_PRINT_MTRPARAM
	printf("表计参数:表号:%d\n", no);
#endif
	return websWrite(wp, T("<td>"
			"<input type=text name=mtrno "
			" readonly=readonly  value=%d>"
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
	char* token = strtok(in, " ");
	while (token!=NULL ) {
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
 * @param[out] amtr 表计参数数组.下标即序号(表号)
 * @param[in] wp 页面
 * @param[out] e 输出错误数组,每个元素表示一个表,每以位表示这个表的某一相参数的错误,1错误,0正确.
 * @retval 大于0整数:参数数组个数
 * @retval 小于0 :错误代码
 */
static int getmtrparams(stMtr amtr[MAX_MTR_NUM], webs_t wp, u32 e[MAX_MTR_NUM])
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
#if DEBUG_PRINT_MTRPARAM
	printf("record num[0]=%d\n", n[0]);
#endif
	for (i = 0; i<n[0]; i++) {
		//printf("接收循环[%d]\n", i);
		amtr[i].mtrno = strtoul(no[i], &errstr, 10);
		if (*errstr!='\0') {
			e[i] |= 0x1;
		}
		(void) strtoull(line[i], &errstr, 10);
		if (*errstr!='\0') {
			e[i] |= 0x2;
		}
		(void) strtoull(addr[i], &errstr, 10);
		if (*errstr!='\0') {
			e[i] |= 0x4;
		}
		//PRINT_HERE
		(void) strtoul(pwd[i], &errstr, 10);     //32位无符号,最多4,294,967,295能存所有9位十进制数
		if (*errstr!='\0') {
			e[i] |= 0x8;
		}
		amtr[i].it_dot = strtoul(it_dot[i], &errstr, 10);
		if (*errstr!='\0') {
			e[i] |= 0x10;
		}
		amtr[i].v_dot = strtol(v_dot[i], &errstr, 10);
		if (*errstr!='\0') {

			e[i] |= 0x20;
		}
		amtr[i].p_dot = strtol(p_dot[i], &errstr, 10);
		if (*errstr!='\0') {
			e[i] |= 0x40;
		}
		amtr[i].q_dot = strtol(q_dot[i], &errstr, 10);
		if (*errstr!='\0') {
			e[i] |= 0x80;
		}
		amtr[i].i_dot = strtol(i_dot[i], &errstr, 10);
		if (*errstr!='\0') {
			e[i] |= 0x100;
		}
		//PRINT_HERE
		amtr[i].xl_dot = strtol(xl_dot[i], &errstr, 10);
		if (*errstr!='\0') {
			e[i] |= 0x200;
		}
		amtr[i].ue = strtol(ue[i], &errstr, 10);
		if (*errstr!='\0') {
			e[i] |= 0x400;
		}
		amtr[i].ie = strtol(ie[i], &errstr, 10);
		if (*errstr!='\0') {
			e[i] |= 0x800;
		}
		amtr[i].port = strtol(port[i], &errstr, 10);
		if (*errstr!='\0') {
			e[i] |= 0x1000;
		}
		amtr[i].portplan = strtol(portplan[i], &errstr, 10);
		if (*errstr!='\0') {
			e[i] |= 0x2000;
		}
		amtr[i].protocol = strtol(protocol[i], &errstr, 10);
		if (*errstr!='\0') {
			e[i] |= 0x4000;
		}
		//PRINT_HERE
		amtr[i].p3w4 = strtol(ph_wire[i], &errstr, 10);
		if (*errstr!='\0') {
			e[i] |= 0x8000;
		}
		amtr[i].fact = strtol(factory[i], &errstr, 10);
		if (*errstr!='\0') {
			e[i] |= 0x100000;
		}
		amtr[i].iv = strtol(iv[i], &errstr, 10);
		if (*errstr!='\0') {
			e[i] |= 0x200000;
		}
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
		//print_array(webWrite_line[i], LINE_LEN);
		//print_array(amtr[i].webWrite_line, LINE_LEN);
	}
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
 * 串口方案:从页面接收串口方案,保存到服务器文件中
 * @param wp
 * @return
 */
int webRece_sioplans(webs_t wp)
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
 * @param[out] wp 向这个页面输出数据
 * @param[in] sp 系统参数
 */
int webSend_sioplans(webs_t wp, stSysParam sp)
{
	int no;
	stUart_plan plan;
	websWrite(wp, T("{"));
	websWrite(wp, T("\"parity\":[\"无\",\"奇\",\"偶\"],"));     //3种奇偶校验方式
	websWrite(wp, T("\"data\":[7,8,9],"));     //三种数据位
	websWrite(wp, T("\"stop\":[0,1],"));     //2中停止位 0 1
	websWrite(wp, T("\"baud\":[300,600,1200,2400,4800,9600],"));
	websWrite(wp, T("\"commtype\":[\"异步\",\"同步\"],"));
	websWrite(wp, T("\"item\":["));     //下面是串口数组,每个元素为一个串口配置
	for (no = 0; no<sp.sioplan_num; no++) {
		if (-1==load_sioplan(&plan, CFG_SIOPALN, no)) {
			web_err_proc(EL);
			continue;
		}
		websWrite(wp, T("{"));
		websWrite(wp, T("\"no\":\"%d\""), no);
		websWrite(wp, T(","));
		websWrite(wp, T("\"parity\":\"%u\""), plan.parity);
		websWrite(wp, T(","));
		websWrite(wp, T("\"data\":\"%u\""), plan.data);
		websWrite(wp, T(","));
		websWrite(wp, T("\"stop\":\"%d\""), plan.stop);
		websWrite(wp, T(","));
		websWrite(wp, T("\"baud\":\"%d\""), plan.baud);
		websWrite(wp, T(","));
		websWrite(wp, T("\"commtype\":\"%d\""), plan.Commtype);
		websWrite(wp, T("}"));
		if (no!=sp.sioplan_num-1) {     //末尾的对象不用加逗号
			websWrite(wp, T(","));
		}
	}
	websWrite(wp, T("]}"));
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
 * 从文件读取监视参数,显示到页面
 * @param wp
 * @param sysparam
 * @return
 */
int webSend_monparas(webs_t wp, stSysParam sysparam)
{
	int no;
	stMonparam monpara;
	websWrite(wp, T("{"));
	webWrite_commportList(wp);
	websWrite(wp, T(","));
	webWrite_porttype(wp);
	websWrite(wp, T(",\"sioplan_num\":\"%d\","), sysparam.sioplan_num);
	websWrite(wp, T("\"item\":["));
	for (no = 0; no<sysparam.monitor_ports; no++) {
		if (-1==load_monparam(&monpara, CFG_MON_PARAM, no)) {
			web_err_proc(EL);
			continue;
		}
		websWrite(wp, T("{"));
		websWrite(wp, T("\"mon_no\":\"%d\","), no);
		websWrite(wp, T("\"commport\":\"%d\","), monpara.comm_port);
		webWrite_listen_port(wp, monpara);
		websWrite(wp, T(",\"sioplan\":\"%d\","), monpara.sioplan);
		;
		websWrite(wp, T("\"protocol\":\"%d\","), monpara.port_type);
		webWrite_rtu_addr(wp, monpara);
		websWrite(wp, T(",\"time_syn_chk\":\"%d\","), monpara.bTimeSyn);
		websWrite(wp, T("\"forward_chk\":\"%d\","), monpara.bForward);
		websWrite(
		                wp,
		                T("\"forward_mtr_num\":\"%d\""),
		                monpara.forwardNum);
		websWrite(wp, T("}"));
		if (no!=sysparam.monitor_ports-1) {
			websWrite(wp, T(","));
		}
	}
	websWrite(wp, T("]}"));
	return 0;
}
/**
 * 从页面获取监视参数,保存到文件
 * @param wp
 * @return
 */
int webRece_monparas(webs_t wp)
{
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
		n = sscanf(time_syn, "%hhu", &monparam.bTimeSyn);
		if (n!=1) {
			web_err_proc(EL);
			break;
		}
		time_syn = point2next(&time_syn, ' ');
		//是否转发
		n = sscanf(forward, "%hhu", &monparam.bForward);
		if (n!=1) {
			web_err_proc(EL);
			break;
		}
		forward = point2next(&forward, ' ');
		//转发数量
		n = sscanf(forward_mtr_num, "%hhu", &monparam.forwardNum);
		if (n!=1) {
			web_err_proc(EL);
			break;
		}
		forward_mtr_num = point2next(&forward_mtr_num, ' ');
		save_monparam(&monparam, CFG_MON_PARAM, param_no);
	}
	return 0;
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
 * 将所有网口参数数据:
 * 1.从文件中读取到内存中
 * 2.以json的格式发送的客户端页面(wp)
 * @param[out] wp 客户端页面结构体
 * @param[in] netParamNum 网口的个数
 * @return
 * @note 使用json格式传输数据,使数据与样式无关,示例格式参见/doc下 @ref json-date-example
 */
int webSend_netparas(webs_t wp, int netParamNum)
{
	int i;
	stNetparam netparam;
	//使用json,仅传输数据.样式和行为交由前端控制,易于扩展
	///@note eth_num 或许和下面的数据元素个数冗余,待定
	websWrite(wp, T("{\"eth_num\":\"%d\","), netParamNum);
	websWrite(wp, T("\"item\":["));
	for (i = 0; i<netParamNum; i++) {
		if (-1==load_netparam(&netparam, CFG_NET, i)) {
			web_err_proc(EL);
			continue;
		}
		websWrite(wp, T("{"));
		websWrite(wp, T("\"no\":\"%d\","), i);
		websWrite(wp, T("\"eth\":\"%d\","), netparam.no);
		webWrite_ip(wp, "ip", netparam.ip);
		websWrite(wp, T(","));
		webWrite_ip(wp, "mask", netparam.mask);
		websWrite(wp, T(","));
		webWrite_ip(wp, "gateway", netparam.gateway);
		websWrite(wp, T("}"));
		if (i!=netParamNum-1) {
			websWrite(wp, T(","));
		}
	}
	websWrite(wp, T("]}"));
	return 0;
}
/**
 * 从页面获取所有网络参数,保存到文件中.
 * @param wp
 * @return
 */
int webRece_netparas(webs_t wp)
{
	stNetparam netparam;
	int n;
	int param_no = 0;		///参数序号,即数据库的主键,base 0.没有物理意义
	char * net_no = websGetVar(wp, T("net_no"), T("null"));
	char * eth = websGetVar(wp, T("eth"), T("null"));
	char * ip = websGetVar(wp, T("ip"), T("null"));
	char * mask = websGetVar(wp, T("mask"), T("null"));
	char * gateway = websGetVar(wp, T("gateway"), T("null"));
#if DEBUG_PRINT_NETPARAM
	printf("val: \n%s\n%s\n%s\n%s\n%s\n", net_no, eth, ip, mask, gateway);
#endif
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
 * 从页面获取系统参数，保存到文件．同时更新全局变量．
 * @param[in] wp
 * @param[out] sysparam
 * @return
 */
int webRece_syspara(webs_t wp, stSysParam * sysparam)
{
	int ret = -1;
	/** 错误的项目,每一位表示一个项目,1表示此项错误,0表示此项正确.初始全部正确.
	 共8位表示6个项目,位[6,7]保留为0,位0至位5分别对应:
	 get_meter_num ;get_sioports_num ;netports_num;monitor_ports;control_ports
	 sioplan_num;*/
	int erritem = 0;
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
		erritem|=0x1;
	}
	int sioports_num = strtol(str_sioports_num, &errstr, 10);
	if (*errstr!='\0'||sioports_num<=0||meter_num>=256) {
		printf("2:%s\n", errstr);
		erritem|=0x2;
	}
	int netports_num = strtol(str_netports_num, &errstr, 10);
	if (*errstr!='\0'||netports_num<=0||meter_num>=256) {
		printf("3:%s\n", errstr);
		erritem|=0x4;
	}
	int monitor_ports = strtol(str_monitor_ports, &errstr, 10);
	if (*errstr!='\0'||monitor_ports<=0||meter_num>=256) {
		printf("4:%s\n", errstr);
		erritem|=0x8;
	}
	int control_ports = strtol(str_control_ports, &errstr, 10);
	if (*errstr!='\0'||control_ports<=0||meter_num>=256) {
		printf("5:%s\n", errstr);
		erritem|=0x10;
	}
	int sioplan_num = strtol(str_sioplan_num, &errstr, 10);
	if (*errstr!='\0'||sioplan_num<=0||meter_num>=256) {
		printf("6:%s\n", errstr);
		erritem|=0x20;
	}
	if (erritem==0) {     //只有所有输入都合法
		sysparam->meter_num = meter_num;
		sysparam->sioports_num = sioports_num;
		sysparam->netports_num = netports_num;
		//脉冲已经废弃
		sysparam->monitor_ports = monitor_ports;
		sysparam->control_ports = control_ports;
		sysparam->sioplan_num = sioplan_num;
		ret = save_sysparam(sysparam, CFG_SYS);
		if (ret==-1) {
			web_err_proc(EL);
		}
	}
	response_ok(wp);
	return 0;
}
/**
 * 向页面写系统参数,各项数据以json对象形式,客户端解析并添加到指定的框中
 * @param wp
 * @return
 */
int webSend_syspara(webs_t wp)
{
	stSysParam sysparam;
	int ret = load_sysparam(&sysparam, CFG_SYS);
	if (ret==-1) {
		web_err_proc(EL);
		return -1;
	}
	/*JSON 简单使用,将系统参数抽象为一个对象,其有表计参数个数,串口个数等6个名称/值对
	 在线解析网站: http://jsoneditoronline.org/
	 前端使用eval 或者JSON.parse即可解析.传递类似下面的文本
	 {
	 "meter_num": 1 ,
	 "sioplan_num": 2 ,
	 "monitor_ports": 3 ,
	 "netports_num": 4 ,
	 "sioports_num": 5 ,
	 "control_ports": 6
	 }
	 */
	websWrite(wp, T("{"
			"\"meter_num\":%u,"
			"\"sioplan_num\":%u,"
			"\"monitor_ports\":%u,"
			"\"netports_num\":%u,"
			"\"sioports_num\":%u,"
			"\"control_ports\":%u"
			"}"),
	                sysparam.meter_num,
	                sysparam.sioplan_num,
	                sysparam.monitor_ports,
	                sysparam.netports_num,
	                sysparam.sioports_num,
	                sysparam.control_ports
	                );
	return 0;
}

/**
 * 将所有表计参数发送到页面.
 * @param wp 页面
 * @param mtrnum 一共有的表计个数
 * @return
 * @todo 数据格式修改,只发送数据,不发送格式(样式)和行为(js函数).
 */
int webSend_mtrparams(webs_t wp, int mtrnum)
{
	int no;
	stMtr mtr;
	memset(&mtr, 0x00, sizeof(stMtr));
	///@todo 使用JSON传递数据可以使格式更明确,前端处理字符串不难.
	for (no = 0; no<mtrnum; no++) {
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
 * 从页面接收所有表计参数,保存到服务器端文件中
 * @param wp
 * @return
 */
int webRece_mtrparams(webs_t wp)
{
	stMtr amtr[256];
	memset(amtr, 0x00, sizeof(stMtr));
	int saveret = -1;
	int i = 0;
	///错误项
	u32 e[MAX_MTR_NUM] = { 0 };
	///表计数目
	int mtr_num = 0;
	mtr_num = getmtrparams(amtr, wp, e);
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
	return 0;
}

int webSend_mtr_type(webs_t wp)
{
	u32 i;     //就一个选择列表框.
	websWrite(wp, T("<select name=ph_wire_all "
			"onchange=\"type_all_changed(event);\">\n"));
	for (i = 0; i<sizeof(PW)/sizeof(PW[0]); i++) {
		websWrite(wp, T("<option value=\"%d\" >%s</option>"), i,
		                PW[i]);
	}
	return 0;
}
int webSend_mtr_factory(webs_t wp)
{
	u32 i;
	char *fact[] = { HOLLEY, WEI_SHENG, LAN_JI_ER, HONG_XIANG, "other" };
	//printf("加载所有生产厂家:共%d个\n", sizeof(fact)/sizeof(fact[0]));
	websWrite(wp, T("<select name=all_factory "));
	websWrite(wp, T("onchange=\"setall_factory(event);\">\n"));
	for (i = 0; i<sizeof(fact)/sizeof(fact[0]); i++) {
		websWrite(wp, T("<option value=\"%d\">%s</option>"), i,
		                fact[i]);
	}
	return 0;
}
int webSend_mtr_procotol(webs_t wp)
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
 * 向客户端页面发送串口数据
 * @param[out] wp
 * @param[in] sysparam
 * @return
 */
int webSend_mtr_sioplan(webs_t wp, stSysParam sysparam)
{
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

/**
 * 从页面获取储存周期参数,保存到本地文件中
 * @param wp
 * @return
 */
int webRece_savecycle(webs_t wp)
{
	int i;
	int n;
	stSave_cycle sav[SAVE_CYCLE_ITEM];
	char *flags = websGetVar(wp, T("flag"), T("null"));
	char *cycle = websGetVar(wp, T("cycle"), T("null"));
	for (i = 0; i<SAVE_CYCLE_ITEM; i++) {
		n = sscanf(flags, "%hhu", &sav[i].enable);
		if (n!=1) {
			web_err_proc(EL);
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
	return 0;
}
/**
 * 从文件读取储存周期,发送(写)到页面 .
 * @param wp
 * @return
 * @todo 发送的数据格式修改,需要与前端配置修改格式.
 */
int webSend_savecycle(webs_t wp)
{
	//printf("读取存储周期.\n");
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
		websWrite(wp, T("<input type=text "
				"size=1 name=flag value=%d >\n"),
		                sav[i].enable);
		websWrite(wp, T("</td>\n"));
	}
	//第二行:储存周期
	websWrite(wp, T("<tr>\n"));
	websWrite(wp, T("<td>\n"));
	websWrite(wp, T("%s"), CSTR_SAVECYCLE_CYCLE);
	websWrite(wp, T("</td>\n"));
	for (i = 0; i<SAVE_CYCLE_ITEM; i++) {
		u32 j;
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

/**
 * 从页面中获取文本,保存到本地文本文件中.
 * @param wp
 * @param query
 * @param file
 */
int webRece_txtfile(webs_t wp, char_t *query, const char* file)
{
	websHeader_pure(wp);
	char * txt = query;
	FILE*fp = fopen(file, "w");
	if (fp==NULL ) {
		return -1;
	}
	fwrite(txt, strlen(txt), 1, fp);
	fclose(fp);
	websDone(wp, 200);
	return 0;
}
/**
 * 读取文本文件,写到页面中.
 * @param[out] wp
 * @param[in] file
 */
int webSend_txtfile(webs_t wp, const char*file)
{
	websHeader_pure(wp);
	char buf[1024] = { 0 };
	int ret;
	FILE*fp = fopen(file, "r");
	if (fp==NULL ) {
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
	fclose(fp);
	WEB_END:
	websDone(wp, 200);
	return 0;
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
///Home page handler,首页句柄
static int websHomePageHandler(webs_t wp,
        char_t *urlPrefix __attribute__ ((unused)),
        char_t *webDir __attribute__ ((unused)),
        int arg __attribute__ ((unused)), char_t *url,
        char_t *path __attribute__ ((unused)),
        char_t *query __attribute__ ((unused)))
{
	/** If the empty or "/" URL is invoked,
	 * 例如 "http:192.168.1.189:8080" 或者 "http:192.168.1.189:8080/"
	 * redirect default URLs to the home page */
	if (*url=='\0'||gstrcmp(url, T("/"))==0) {
		websRedirect(wp, WEBS_DEFAULT_HOME);
		return 1;
	}
	return 0;
}
///打印数组
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
/**
 * Exit cleanly on interrupt
 */
static void sigintHandler(int unused __attribute__ ((unused)))
{
	finished = 1;
}
/**
 * 去除输入数组中的前导空白符和后导空白符
 * @param[in] in 待修改的数组,会被修改
 * @param[in] len 数组构成的字符串长度.
 * @return 指向修改好的字符串首地址指针
 */
char *trim(char in[], int len)
{
	int i = 0;
	int tail = len-1;
	//找到倒数第一个不为空白符的字符,截断之.
	while (tail>=0) {
		if (strchr(" \t\r\n\\/", in[tail])==NULL ) {
			break;
		}
		tail--;
	}
	in[tail+1] = '\0';
	//找到第一个不为空白符的字符,字符串头指向它.
	while (i<=len) {
		if (strchr(" \t\r\n", in[i])==NULL ) {
			break;
		}
		i++;
	}
	in += i;
	return in;
}
/**
 * 初始化信号量,用于进程间的控制,目前用于监视报文的启动和停止.
 */
void init_semun(void)
{
	///申请信号量组，包含1个信号量
	semid = semget(1000, 1, 0666|IPC_CREAT);
	sem.val = 1;
	///初始化0号信号量为1,起始便有一个进程可以使用.
	semctl(semid, 0, SETVAL, sem);
}
/**
 * 释放手动分配在堆上的内存.
 * 主要包括:
 * 1. 配置文件项占用的
 * 2. 监视参数端口名称占用的.
 * 3. 规约名称(如sx102这样的文本)所占用的.
 */
void webs_free(void)
{
	int i;
	//配置文件项
	if(errlog!=NULL){
		free(errlog);
		errlog=NULL;
	}
	//监视参数端口名称
	for(i=0;i<mon_port_num;i++){
		if(mon_port_name[i]!=NULL){
			free(mon_port_name[i]);
			mon_port_name[i]=NULL;
		}
	}
	//规约名称
	for(i=0;i<procotol_num;i++){
		if(procotol_name[i]!=NULL){
			free(procotol_name[i]);
			procotol_name[i]=NULL;
		}
	}
}
/**
 * 对于客户端的更新请求给予响应.如果更新成功则简单的返回这个成功响应
 * @param wp 页面
 */
void response_ok(webs_t wp){
	websWrite(wp, T("{\"ret\":\"ok\"}"));
}
#include <stdio.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
int print_ip(void)
{
        struct hostent *he;
        char hostname[20] = {0};
        gethostname(hostname,sizeof(hostname));
        he = gethostbyname(hostname);
        printf("hostname=%s\n",hostname);
        printf("%s\n",inet_ntoa(*(struct in_addr*)(he->h_addr)));
}
//#pragma  GCC diagnostic ignored  "-Wunused-parameter"
