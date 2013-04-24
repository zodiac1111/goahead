/**
 * @file main.c
 * main.c -- Main program for the GoAhead WebServer (LINUX version)
 * Copyright (c) GoAhead Software Inc., 1995-2010. All Rights Reserved.
 * See the file "license.txt" for usage and redistribution license requirements
 * @note 尽量只操作处理数据,样式和行为应该交给前端控制.[历史电量数据部分未完成]
 * @note 给前端发送的数据可以使用JSON.接收前端的数据就还是使用goahead的getVal吧.
 * 	后端仅进行一些必要的数值合法性验证.
 */
#define DLINK_TEST 0
#include "../uemf.h"
#include "../wsIntrn.h"
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/time.h>
#include <signal.h>
#include <dlfcn.h>
//#include "sys_utl.h"

#if DLINK_TEST
#include "struct.h"
#endif

#include "param.h"
#include "main.h"
#include "Chinese_string.h"
#ifdef WEBS_SSL_SUPPORT
#include	"../websSSL.h"
#endif
#ifdef USER_MANAGEMENT_SUPPORT
#include	"../um.h"
void formDefineUserMgmt(void);
#endif
//Change configuration here and conf.h file
static char_t *password = T(""); /* Security password */
static int retries = 5; /* Server port retries */
static int finished = 0; /* Finished flag */
#ifdef B_STATS
static void printMemStats(int handle, char_t *fmt, ...);
static void memLeaks();
#endif
///内存中的系统参数结构体.全局使用.
stSysParam sysparam = { 0,0,0,0,0,0,0 };
///规约文件中的规约名称.
static char *procotol_name[MAX_PROCOTOL_NUM];
///规约文件中的实际规约数,初始化为最大
static int procotol_num = MAX_PROCOTOL_NUM;
///规约文件中的规约名称.
static char *mon_port_name[MAX_MON_PORT_NUM];
///规约文件中的实际规约数,初始化为最大
static int mon_port_num = MAX_MON_PORT_NUM;
struct sembuf sb;     ///<信号量操作,该功能完善中.
union semun sem;     ///<用于控制报文监视停止的信号量.0停止监视程序,1运行监视程序
int semid;     ///<信号量id
stCfg webs_cfg;
char * webdir;
#define JSON 1
//#pragma  GCC diagnostic warning  "-Wunused-parameter"
/**
 * webs主函数,所有业务逻辑在此实现.
 */
int main(int argc __attribute__ ((unused)),
        char** argv __attribute__ ((unused)))
{
#if AUTO_UPDATE
	autoUpdate();
#endif
#if DEBUG_JSON_DEMO
	jsonDemo();     ///@note json操作示例.对操作不熟悉可以反注释来查看
#endif

#if DLINK_TEST
#define SOFILE "sys_utl.so"
	struct stMeter_Run_data* (*GetData)(void);
	void *dp;
	dp=dlopen(SOFILE,RTLD_LAZY);
	GetData=dlsym(dp,"GetData");
	//struct stMeter_Run_data mtrs;
	struct stMeter_Run_data* mtrs= GetData();
	printf(WEBS_DBG"电压 A相=%f\n",mtrs->m_wU[0]);
	printf(WEBS_DBG"软件版本 %s\n",GetSoftVersion());
	//printf(WEBS_WAR"%X \n",Bcd2Hex_Byte(0x10));
#endif
	memset(&webs_cfg, 0x0, sizeof(stCfg));
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
	bopen(NULL, (600*1024), B_USE_MALLOC);
	signal(SIGPIPE, SIG_IGN );
	signal(SIGINT, sigintHandler);
	signal(SIGTERM, sigintHandler);
	//Initialize the web server 初始化web服务器
	int ret_webs = initWebs();
	if (ret_webs<0) {
		printf(WEBS_ERR"init Webs.ret:[%d]\n", ret_webs);
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
	printf(WEBS_INF"Now access "GREEN UNDERLINE "http://<IP>:%s"_COLOR
	" with Browser.\n", webs_cfg.port);
	while (!finished) {
		//PRINT_HERE
		if (socketReady(-1)||socketSelect(-1, 1000)) {
			socketProcess(-1);
		}
		websCgiCleanup();
		emfSchedProcess();
		//break;//用于调试正确的退出.
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
	if (strcmp(action, "get")==0) {
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
	char * action = websGetVar(wp, T("action"), T("null"));
	printf("\t\t@%s", ctime(&t));
	websHeader_pure(wp);
	if (strcmp(action, "get")==0) {
		websWrite(wp, T("{\"timestamp\":\"%d\"}"), t);
	} else if (strcmp(action, "set")==0) {
		webRece_syntime(wp);
	} else {
		web_err_proc(EL);
	}
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
	if (strcmp(action, "init")==0) {
		webSend_sioplans(wp, sysparam);
	} else if (strcmp(action, "set")==0) {
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
 * 表计参数设置表单提交触发事件,由meterpara.html页面post触发
 * @param[in] wp 页面 输入/输出
 * @param[in] path 路径
 * @param[in] query 提交POST的字符串值
 */
void form_mtrparams(webs_t wp, char_t *path, char_t *query)
{
	PRINT_FORM_INFO;
	//printf(WEBS_DBG" %s\n",wp->postData);
	websHeader_pure(wp);
	char * action = websGetVar(wp, T("action"), T("null"));
	if (strcmp(action, "get")==0) {
		webSend_mtrparams(wp, sysparam.meter_num);
	} else if (strcmp(action, "set")==0) {
		webRece_mtrparams(wp);
	} else {     //其他未知命令一律忽略
	             //web_errno=mtr_form;
	             //web_err_proc(EL);
	}
	websDone(wp, 200);
	return;
}
/**
 * 监视端口 表单提交触发函数
 * @param wp 输入/输出
 * @param path
 * @param query
 */
void form_monparas(webs_t wp, char_t *path, char_t *query)
{
	PRINT_FORM_INFO;
	websHeader_pure(wp);
	char * action = websGetVar(wp, T("action"), T("null"));
	if (strcmp(action, "get")==0) {
		webSend_monparas(wp, sysparam);
	} else if (strcmp(action, "set")==0) {
		webRece_monparas(wp);
	} else {
		web_err_proc(EL);
	}
	websDone(wp, 200);
	return;
}
/**
 * 表单提交函数,储存周期.
 * @param wp 输入/输出
 * @param path
 * @param query
 */
void form_savecycle(webs_t wp, char_t *path, char_t *query)
{
	PRINT_FORM_INFO;
	websHeader_pure(wp);
	char * action = websGetVar(wp, T("action"), T("null"));
	if (strcmp(action, "get")==0) {
		webSend_savecycle(wp);
	} else if (strcmp(action, "set")==0) {
		webRece_savecycle(wp);
	} else {
		web_err_proc(EL);
	}
	websDone(wp, 200);
	return;
}
/**
 * 表单提交函数,采集周期
 * @param wp 输入/输出
 * @param path
 * @param query
 */
void form_collect_cycle(webs_t wp, char_t *path, char_t *query)
{
	PRINT_FORM_INFO;
	websHeader_pure(wp);
	char * action = websGetVar(wp, T("action"), T("null"));
	if (strcmp(action, "get")==0) {
		webSend_collect_cycle(wp);
	} else if (strcmp(action, "set")==0) {
		webRece_collect_cycle(wp);
	} else {
		web_err_proc(EL);
	}
	websDone(wp, 200);
	return;
}
/**
 * 客户端发送重启表单,分类重启,一些系统功能
 * @todo 配合的更好一些,参考路由器的页面行为设计.
 *   前端一个更舒适的行为反馈(终端重启前端交互基本可行).
 *   进行较为科学的分类,待整理.
 * @param wp
 * @param path
 * @param query
 */
void form_sysFunction(webs_t wp, char_t *path, char_t *query)
{
#define REINIT_PROTOCOL_FILE 1
#define RET_WEB 2
#define RET_SAMPLE_PROC 3
#define RET_RTU 4
#define RET_CLEARDATA 5
#define RET_TEST 10
#define WEBS_REQ_TEST 12
	PRINT_FORM_INFO;
	websHeader_pure(wp);
	//char app[256] = { 0 };
	int typ = 0;
	int ret = -1;
	//pid_t pid;
	char * str_typ = websGetVar(wp, T("OpType"), T("null"));
	typ = atoi(str_typ);
	printf("type=%d\n", typ);
	switch (typ) {
	case REINIT_PROTOCOL_FILE:
		ret = read_protocol_file(procotol_name, &procotol_num,
		                webs_cfg.protocol);
		if (ret!=0) {
			web_err_proc(EL);
		}
		break;
	case RET_WEB:
		autoUpdate();
		break;
	case RET_SAMPLE_PROC:		///@待定
		system("killall -9 hl3104_com");
		break;
	case RET_RTU:
		//reflash_this_wp(wp, PAGE_RESET);
#if __i386 == 1
		//调试不要重启PC系统...
		system("echo \"reboot ok\"");
#else
		system("reboot");
		//websDone(wp, 200);
#endif
		break;
	case RET_CLEARDATA:
		#if __i386 == 1
		//调试不要重启PC系统...
		system("echo \"clear data ok\"");
#else
		system("rm /mnt/nand/*.* -f");
		//websDone(wp, 200);
#endif
		break;
	case RET_TEST:
		system("ls");
		//websDone(wp, 200);
		break;
	case WEBS_REQ_TEST:
		//websDone(wp, 200);
		break;
	default:
		//websDone(wp, 200);
		return;
		break;
	}
	websDone(wp, 200);
	//reflash_this_wp(wp, PAGE_RESET);
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
	char * strtz = websGetVar(wp, T("timezone"), T("0"));
	int tz = 0;
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
	ret = sscanf(strtz, "%d", &tz);
	if (ret!=1) {
		web_err_proc(EL);
	}
	tr.s += (tz*60);
	tr.e += (tz*60);
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
	webRece_txtfile(wp, query, webs_cfg.errlog);
	return;
}
void form_save_monport_cfgfile(webs_t wp, char_t *path, char_t *query)
{
	PRINT_FORM_INFO;
	webRece_txtfile(wp, query, webs_cfg.monparam_name);
	return;
}
void form_save_procotol_cfgfile(webs_t wp, char_t *path, char_t *query)
{
	PRINT_FORM_INFO;
	webRece_txtfile(wp, query, webs_cfg.protocol);
	return;
}
/**
 * 加载日志文件到客户端
 * @param[out] wp 发送/写到这个页面
 * @param path
 * @param query
 */
void form_load_log(webs_t wp, char_t *path, char_t *query)
{
	PRINT_FORM_INFO;
	webSend_txtfile(wp, webs_cfg.errlog);
	return;
}
///加载规约文件,同上
void form_load_procotol_cfgfile(webs_t wp, char_t *path, char_t *query)
{
	PRINT_FORM_INFO;
	webSend_txtfile(wp, webs_cfg.protocol);
	return;
}
///加载监视端口名称描述文件.
void form_load_monport_cfgfile(webs_t wp, char_t *path, char_t *query)
{
	PRINT_FORM_INFO;
	webSend_txtfile(wp, webs_cfg.monparam_name);
	return;
}

/**
 * 报文监视(执行指令) 表单提交处理函数.
 * @todo 未实现,执行命令的输出还不能读取.前后端可能需要频繁交互.
 *       创建进程有问题,ps看到很多webs,销毁进程有问题!
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
///客户端读取服务器配置信息
void form_info(webs_t wp, char_t *path, char_t *query)
{
	PRINT_FORM_INFO;
	websHeader_pure(wp);
	char * action = websGetVar(wp, T("action"), T("null"));
	if (strcmp(action, "get")==0) {
		webSend_info(wp);
	} else {
		web_err_proc(EL);
	}
	websDone(wp, 200);
	return;
}
/******************************************************************************/

/**
 * 打印服务器应用程序运行路径,依赖proc文件系统.
 * @return
 */
int printf_webs_app_dir(void)
{
	char dir[256] = { 0 };
	//需要proc文件系统支持
	int count = readlink("/proc/self/exe", dir, 256);
	if (count<0||count>256) {
		PRINT_RET(count);
		printf(WEBS_ERR"%s\n", __FUNCTION__);
	}
	printf(WEBS_INF"App dir\t:"GREEN"%s"_COLOR"\n", dir);
	webs_cfg.appname = (char*) malloc(strlen(dir)+1);
	strcpy(webs_cfg.appname, dir);
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
char* getconf(const char * const name, char** value)
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
		return NULL ;
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
		pname = trim(n, strlen(n));		//去除前导和后导空白符
		pvalue = trim(v, strlen(v));
		if (strcmp(pname, name)==0) {
			*value = (char*) malloc(strlen(pvalue)+1);
			if (*value==NULL ) {
				web_err_proc(EL);
				break;
			}
			strcpy(*value, pvalue);
		}
	}
	fclose(fp);
	printf(WEBS_INF"conf "YELLOW"%s"_COLOR"\t= "GREEN"%s"_COLOR"\n"
	                , name, *value);
	return *value;
}
/**
 * 传入目录和文件构造完整的绝对路径.用完注意free!
 * path 和 name 变成"path/name"的形式
 * @param path
 * @param name
 * @return
 */
char *mkFullPath(const char *path, const char *name)
{
	char *fullpath = NULL;
	int l1 = strlen(path);
	int l2 = strlen(name);
	int len = l1+1+l2;		// "/"中间添加正斜杠
	fullpath = (char*) malloc(len+1);		// \0
	memset(fullpath, 0x0, len+1);
	memcpy(fullpath, path, l1);
	memcpy(fullpath+l1, "/", 1);
	memcpy(fullpath+l1+1, name, l2);
#if DEBUG_CONF_FULLPATH
	printf(WEBS_DBG"FullPath %s\n", fullpath);
#endif
	return fullpath;
}
/**
 * Initialize the web server.
 * 初始化web服务的一些操作:
 *   配置套接字
 *   修改运行目录和配置www根目录
 *   注册asp和form函数
 * @return
 */
static int initWebs(void)
{
	struct hostent *hp;
	struct in_addr intaddr;
	char host[128];

	char *cp;
	char_t wbuf[128];
	//先打开各种配置文件,主要是使能错误日志
	int ret = load_webs_conf_info();
	if (ret<0) {
		return ret;
	}
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
		perror("gethostname");
		error(E_L, E_LOG, T("Can't get hostname"));
		web_err_proc(EL);
		return -1000;
	}
	/// @note gethostbyname系统调佣需要hostname命令执行成功,老版本文件只读
	if ((hp = gethostbyname(host))==NULL ) {
		herror(WEBS_ERR"gethostbyname");
		printf(WEBS_WAR"Try to use the IP 127.0.0.1 instead.");
		error(E_L, E_LOG, T("Can't get host address"));
		web_err_proc(EL);
		memcpy((char *) &intaddr, (char *)"127.0.0.1",
					strlen("127.0.0.1"));
	}else{
		memcpy((char *) &intaddr, (char *) hp->h_addr_list[0],
			                (size_t) hp->h_length);
	}
	///改变程序的当前目录,所有相对路径都是相对当前目录的.当前目录为www(demo)目录
	///除了配置文件(多数)中定义的绝对路径的文件,其他相对路型以webdir为起点.
	chdir(webdir);
	//Configure the web server options before opening the web server
	websSetDefaultDir(webdir);
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
	//websOpenServer(port, retries);
	websOpenServer(atoi(webs_cfg.port), retries);

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
	//载入名称文件,给服务器程序显示用的.
	if (-1==read_protocol_file(procotol_name,
	                &procotol_num, webs_cfg.protocol)) {
		web_err_proc(EL);
	}
	if (-1==init_monparam_port_name(mon_port_name, &mon_port_num,
	                webs_cfg.monparam_name)) {
		printf("* %s\n", webs_cfg.monparam_name);
		web_err_proc(EL);
	}
	/*
	 * 注册asp函数,给予asp调用
	 * [空]
	 */
	//注册表单post函数. form define/用于post
	websFormDefine(T("srv_time"), form_server_time);
	websFormDefine(T("sysparam"), form_sysparam);
	websFormDefine(T("sioplan"), form_sioplans);
	websFormDefine(T("netpara"), form_netparas);
	websFormDefine(T("monparas"), form_monparas);
	websFormDefine(T("savecycle"), form_savecycle);
	websFormDefine(T("collectcycle"), form_collect_cycle);
	websFormDefine(T("mtrparams"), form_mtrparams);
	websFormDefine(T("get_tou"), form_history_tou);
	websFormDefine(T("realtime_tou"), form_realtime_tou);
	websFormDefine(T("reset"), form_sysFunction);
	websFormDefine(T("save_log"), form_save_log);
	websFormDefine(T("load_log"), form_load_log);
	websFormDefine(T("load_monport_cfg"), form_load_monport_cfgfile);
	websFormDefine(T("save_monport_cfg"), form_save_monport_cfgfile);
	websFormDefine(T("load_procotol_cfg"), form_load_procotol_cfgfile);
	websFormDefine(T("save_procotol_cfg"), form_save_procotol_cfgfile);
	websFormDefine(T("msg"), form_msg);
	websFormDefine(T("msg_stop"), form_msg_stop);
	websFormDefine(T("info"), form_info);
	websFormDefine(T("upload_file"), form_upload_file);
#ifdef USER_MANAGEMENT_SUPPORT
	//Create the Form handlers for the User Management pages
	formDefineUserMgmt();
#endif
	//Create a handler for the default home page
	websUrlHandlerDefine(T("/"), NULL, 0, websHomePageHandler, 0);
	///加载系统参数
	if (-1==load_sysparam(&sysparam, webs_cfg.syspara)) {
		web_err_proc(EL);
		//return -1;
	}
	return 0;
}

/**
 * 从配置文件和动态库中加载配置信息
 */
int load_webs_conf_info(void)
{
	(void) printf_webs_app_dir();
	printf(WEBS_INF"Config file\t:"GREEN CONF_FILE _COLOR "\n");
	if (getconf("wwwroot", &webdir)==NULL ) {
		webs_cfg.errlog = BACKUP_ERR_FILE;	//记录到备用文件
		webs_cfg.default_errlog = 1;
		web_err_proc(EL);
		return -2000;
	}
	if (getconf("errlog", &webs_cfg.errlog)==NULL ) {
		webs_cfg.errlog = BACKUP_ERR_FILE;	//记录到备用文件
		webs_cfg.default_errlog = 1;
		web_errno = use_backup_err_log;
		web_err_proc(EL);
		//从这里开始可以记录错误日志到文件了.
	}
	if (getconf("paradir", &webs_cfg.paradir)==NULL ) {
		web_err_proc(EL);
		return -2001;
	}
	if (getconf("confdir", &webs_cfg.confdir)==NULL ) {
		web_err_proc(EL);
		return -2002;
	}
	if (getconf("port", &webs_cfg.port)==NULL ) {
		webs_cfg.port = WEBS_DEFAULT_PORT;
		webs_cfg.default_port = 1;
		web_errno = use_backup_port;
		web_err_proc(EL);
	}
#ifdef WEBS_SSL_SUPPORT
	if (getconf("sslport", &webs_cfg.sslport)==NULL ) {
		webs_cfg.sslport=WEBS_DEFAULT_SSL_PORT;
		webs_cfg.default_sslport=1;
		web_errno=use_backup_sslport;
		web_err_proc(EL);
	}
#endif
	webs_cfg.syspara = mkFullPath(webs_cfg.paradir, FILE_SYSPARA);
	webs_cfg.mtrspara = mkFullPath(webs_cfg.paradir, CFG_MTR);
	webs_cfg.sioplan = mkFullPath(webs_cfg.paradir, CFG_SIOPALN);
	webs_cfg.netpara = mkFullPath(webs_cfg.paradir, CFG_NET);
	webs_cfg.monpara = mkFullPath(webs_cfg.paradir, CFG_MON_PARAM);
	webs_cfg.retranTable = mkFullPath(webs_cfg.paradir, CFG_FORWARD_TABLE);
	webs_cfg.stspara = mkFullPath(webs_cfg.paradir, CFG_SAVE_CYCLE);
	webs_cfg.ctspara = mkFullPath(webs_cfg.paradir, CFG_COLLECT_CYCLE);
	webs_cfg.protocol = mkFullPath(webs_cfg.confdir, PORC_FILE);
	webs_cfg.monparam_name = mkFullPath(
	                webs_cfg.confdir,
	                MON_PORT_NAME_FILE);
	//版本信息字符串
	webs_cfg.main_version_string =
	                (char*) malloc(MAIN_PROGRAM_VERSION_STRING_MAX_LENGTH);
	unsigned char * v = GetSoftVersion();
	sprintf(webs_cfg.main_version_string,
	                "%d.%d %d.%02d.%02d %c%c",
	                v[0], v[1], v[2], v[3], v[4], v[5], v[6]);
	//printf("%s\n",webs_cfg.main_version_string);
	return 0;
}
/**
 * 写终端地址4个数字字符到页面.
 * @param wp
 * @param monport
 * @return
 */
char* webWrite_rtu_addr(char* tmp, stMonparam monport)
{
#if DEBUG_PRINT_MONPARAM
	printf("监视参数-终端地址:%d%d%d%d\n", monport.prot_addr[0],
			monport.prot_addr[1], monport.prot_addr[2],
			monport.prot_addr[3]);
#endif
	int i;
	for (i = 0; i<4; i++) {
		sprintf(tmp+i, "%1X", monport.prot_addr[i]);
	}
	return tmp;
}
/**
 * 构造监听端口,写道tmp数组中,tmp必须已经被分配空间.
 * @param[out] tmp
 * @param[in] monport
 * @return
 */
char* webWrite_listen_port(char* tmp, const stMonparam monport)
{
	int i;
#if DEBUG_PRINT_MONPARAM
	printf("监视参数-监听端口:");
	for (i = 0; i<5; i++) {
		printf("%d", monport.listen_port[i]);
	}
	printf("\n");
#endif
	for (i = 0; i<5; i++) {
		sprintf(tmp+i, "%1d", monport.listen_port[i]);
	}
	return tmp;
}

/**
 * 填写形如 "ip":"111.222.333.444" 的json字符串
 * @param[out] wp 写入到这个页面
 * @param[in] name 变量名,如上面的ip,可取ip,mask,gateway
 * @param[in] value 变量值,如上面的111.222.333.444
 * @retval 0 成功
 */
static int webWrite_ip(webs_t wp, char *name, uint8_t* value)
{
	//printf("网口参数-IP\n");
	uint i;
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
			printf("表计项目序号:%d,项目数量%d,表数目=%d\n"
			                , i, n[i], n[0]);
			web_err_proc(EL);
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
static int getmtrparams(stMtr amtr[MAX_MTR_NUM], webs_t wp, uint32_t e[MAX_MTR_NUM])
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
#if DEBUG_PRINT_MTRPARAM
		printf("mtrno= %s\n", websGetVar(wp, T("mtrno"), T("null")));
#endif
		n[0] = split(no, websGetVar(wp, T("mtrno"), T("null")));
	} else {
		PRINT_HERE
		return -1000;
	}
	if (websTestVar(wp, T("line"))) {
#if DEBUG_PRINT_MTRPARAM
		printf("line= %s\n", websGetVar(wp, T("line"), T("null")));
#endif
		n[1] = split(line, websGetVar(wp, T("line"), T("null")));
	} else {
		PRINT_HERE
		return -1010;
	}
	if (websTestVar(wp, T("addr"))) {
#if DEBUG_PRINT_MTRPARAM
		printf("addr= %s\n", websGetVar(wp, T("addr"), T("null")));
#endif
		n[2] = split(addr, websGetVar(wp, T("addr"), T("null")));
	} else {
		PRINT_HERE
		return -1020;
	}
	if (websTestVar(wp, T("pwd"))) {
#if DEBUG_PRINT_MTRPARAM
		printf("pwd= %s\n", websGetVar(wp, T("pwd"), T("null")));
#endif
		n[3] = split(pwd, websGetVar(wp, T("pwd"), T("null")));
	} else {
		PRINT_HERE
		return -1030;
	}
	if (websTestVar(wp, T("it_dot"))) {
#if DEBUG_PRINT_MTRPARAM
		printf("it_dot= %s\n", websGetVar(wp, T("it_dot"), T("null")));
#endif
		n[4] = split(it_dot, websGetVar(wp, T("it_dot"), T("null")));
	} else {
		PRINT_HERE
		return -1040;
	}
	if (websTestVar(wp, T("v_dot"))) {
#if DEBUG_PRINT_MTRPARAM
		printf("v_dot= %s\n", websGetVar(wp, T("v_dot"), T("null")));
#endif
		n[5] = split(v_dot, websGetVar(wp, T("v_dot"), T("null")));
	} else {
		PRINT_HERE
		return -1050;
	}
	if (websTestVar(wp, T("p_dot"))) {
#if DEBUG_PRINT_MTRPARAM
		printf("p_dot= %s\n", websGetVar(wp, T("p_dot"), T("null")));
#endif
		n[6] = split(p_dot, websGetVar(wp, T("p_dot"), T("null")));
	} else {
		PRINT_HERE
		return -1060;
	}
	if (websTestVar(wp, T("q_dot"))) {
#if DEBUG_PRINT_MTRPARAM
		printf("q_dot= %s\n", websGetVar(wp, T("q_dot"), T("null")));
#endif
		n[7] = split(q_dot, websGetVar(wp, T("q_dot"), T("null")));
	} else {
		PRINT_HERE
		return -1070;
	}
	if (websTestVar(wp, T("i_dot"))) {
#if DEBUG_PRINT_MTRPARAM
		printf("i_dot= %s\n", websGetVar(wp, T("i_dot"), T("null")));
#endif
		n[8] = split(i_dot, websGetVar(wp, T("i_dot"), T("null")));
	} else {
		PRINT_HERE
		return -1080;
	}
	if (websTestVar(wp, T("xl_dot"))) {
#if DEBUG_PRINT_MTRPARAM
		printf("xl_dot= %s\n", websGetVar(wp, T("xl_dot"), T("null")));
#endif
		n[9] = split(xl_dot, websGetVar(wp, T("xl_dot"), T("null")));
	} else {
		PRINT_HERE
		return -1090;
	}
	if (websTestVar(wp, T("ue"))) {
#if DEBUG_PRINT_MTRPARAM
		printf("ue= %s\n", websGetVar(wp, T("ue"), T("null")));
#endif
		n[10] = split(ue, websGetVar(wp, T("ue"), T("null")));
	} else {
		PRINT_HERE
		return -1090;
	}
	if (websTestVar(wp, T("ie"))) {
#if DEBUG_PRINT_MTRPARAM
		printf("ie= %s\n", websGetVar(wp, T("ie"), T("null")));
#endif
		n[11] = split(ie, websGetVar(wp, T("ie"), T("null")));
	} else {
		PRINT_HERE
		return -1100;
	}
	if (websTestVar(wp, T("port"))) {
#if DEBUG_PRINT_MTRPARAM
		printf("port= %s\n", websGetVar(wp, T("port"), T("null")));
#endif
		n[12] = split(port, websGetVar(wp, T("port"), T("null")));
	} else {
		PRINT_HERE
		return -1110;
	}
	if (websTestVar(wp, T("portplan"))) {
#if DEBUG_PRINT_MTRPARAM
		printf(
		                "portplan= %s\n",
		                websGetVar(wp, T("portplan"), T("null")));
#endif
		n[13] = split(portplan,
		                websGetVar(wp, T("portplan"), T("null")));
	} else {
		PRINT_HERE
		return -1120;
	}
	if (websTestVar(wp, T("protocol"))) {
#if DEBUG_PRINT_MTRPARAM
		printf(
		                "protocol= %s\n",
		                websGetVar(wp, T("protocol"), T("null")));
#endif
		n[14] = split(protocol,
		                websGetVar(wp, T("protocol"), T("null")));
	} else {
		PRINT_HERE
		return -1130;
	}
	if (websTestVar(wp, T("ph_wire"))) {
#if DEBUG_PRINT_MTRPARAM
		printf(
		                "ph_wire= %s\n",
		                websGetVar(wp, T("ph_wire"), T("null")));
#endif
		n[15] = split(ph_wire, websGetVar(wp, T("ph_wire"), T("null")));
	} else {
		PRINT_HERE
		return -1140;
	}

	if (websTestVar(wp, T("factory"))) {
#if DEBUG_PRINT_MTRPARAM
		printf(
		                "factory= %s\n",
		                websGetVar(wp, T("factory"), T("null")));
#endif
		n[16] = split(factory, websGetVar(wp, T("factory"), T("null")));
	} else {
		PRINT_HERE
		return -1150;
	}

	if (websTestVar(wp, T("iv"))) {
#if DEBUG_PRINT_MTRPARAM
		printf("iv= %s\n", websGetVar(wp, T("iv"), T("0")));
#endif
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
		amtr[i].ie = 1000*atof(ie[i]);
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
		uint8_t l;
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
		uint j = 0;
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
int reflash_this_wp(webs_t wp, const char *page)
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
 * 串口方案:从页面接收串口方案,保存到服务器(终端)文件中
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
		save_sioplan(&plan, webs_cfg.sioplan, no);
	}
	return 0;
}
/**
 * 从文件中读取串口方案,构造json对象,向页面写串口方案.
 * @param[out] wp 向这个页面输出数据
 * @param[in] sp 系统参数
 */
int webSend_sioplans(webs_t wp, stSysParam sp)
{
	int no;
	char tmp[256] = { 0 };
	stUart_plan plan;
	jsObj oSioPlan = jsonNew();
	jsObj aList = jsonNewArray();
	jsObj aItems = jsonNewArray();
	jsObj oItem = jsonNew();
	jsonAdd(&aList, NULL, "无");
	jsonAdd(&aList, NULL, "偶");
	jsonAdd(&aList, NULL, "奇");
	jsonAdd(&oSioPlan, "parity", aList);
	jsonClear(&aList);
	jsonAdd(&aList, NULL, "7");
	jsonAdd(&aList, NULL, "8");
	jsonAdd(&aList, NULL, "9");
	jsonAdd(&oSioPlan, "data", aList);
	jsonClear(&aList);
	jsonAdd(&aList, NULL, "0");
	jsonAdd(&aList, NULL, "1");
	jsonAdd(&oSioPlan, "stop", aList);
	jsonClear(&aList);
	jsonAdd(&oSioPlan, "baud", "[300,600,1200,2400,4800,9600,19200]");
	jsonClear(&aList);
	jsonAdd(&oSioPlan, "commtype", "[\"异步\",\"同步\"]");
	jsonClear(&aList);
	for (no = 0; no<sp.sioplan_num; no++) {
		if (-1==load_sioplan(&plan, webs_cfg.sioplan, no)) {
			web_err_proc(EL);
			continue;
		}
		jsonAdd(&oItem, "no", u8toa(tmp, "%d", no));
		jsonAdd(&oItem, "parity", u8toa(tmp, "%d", plan.parity));
		jsonAdd(&oItem, "data", u8toa(tmp, "%d", plan.data));
		jsonAdd(&oItem, "stop", u8toa(tmp, "%d", plan.stop));
		jsonAdd(&oItem, "baud", u8toa(tmp, "%d", plan.baud));
		jsonAdd(&oItem, "commtype", u8toa(tmp, "%d", plan.Commtype));
		jsonAdd(&aItems, NULL, oItem);
		jsonClear(&oItem);

	}
	jsonAdd(&oSioPlan, "item", aItems);
	wpsend(wp, oSioPlan);
	jsonFree(&oSioPlan);
	jsonFree(&aList);
	jsonFree(&aItems);
	jsonFree(&oItem);
	return 0;
}
/**
 * 把ip地址字符串如"192.168.0.1"转化成为"192.168.000.001"的12字节文件标准格式.
 * 只转换最开始一个ip地址,完成即停止,返回使用过的ipstr字符的个数(指针后移数目).
 * 比如输入"192.168.0.1 192.168.111.2",则转换192.168.0.1为"192.168.000.001",
 * 并且返回 11.
 * @param[in] ipstr 字符串,输入输出!
 * @param[out] ipfile
 * @return
 */
int ipstr2ipfile(char *ipstr, uint8_t ipfile[12])
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
char* itoa(char* str, const char*format, int value)
{
	sprintf(str, format, value);
	return str;
}

/**
 * 从文件读取监视参数,显示到页面
 * @param[out] wp 写到这个页面
 * @param sysparam
 * @return
 */
int webSend_monparas(webs_t wp, stSysParam sysparam)
{
	int no;
	stMonparam monpara;
	char tmp[256] = { 0 };
	char* oMonPara = jsonNew();
	char* aCommList = jsonNewArray();
	char* aProcList = jsonNewArray();
	char* aItemList = jsonNewArray();
	char* oItem = jsonNew();
	int i;
	jsonAdd(&oMonPara, "sioplan_num",
	                u8toa(tmp, "%d", sysparam.sioplan_num));

	for (i = 0; i<mon_port_num; i++) {
		jsonAdd(&aCommList, NULL, mon_port_name[i]);
	}
	jsonAdd(&oMonPara, "commport", aCommList);

	for (i = 0; i<procotol_num; i++) {
		jsonAdd(&aProcList, NULL, procotol_name[i]);
	}
	jsonAdd(&oMonPara, "protocol", aProcList);

	for (no = 0; no<sysparam.monitor_ports; no++) {
		if (-1==load_monparam(&monpara, webs_cfg.monpara, no)) {
			web_err_proc(EL);
			continue;
		}
		jsonAdd(&oItem, "mon_no", u8toa(tmp, "%d", no));
		jsonAdd(&oItem, "commport",
		                u8toa(tmp, "%d", monpara.comm_port));
		webWrite_listen_port(tmp, monpara);
		jsonAdd(&oItem, "listenport", tmp);
		jsonAdd(&oItem, "sioplan", u8toa(tmp, "%d", monpara.sioplan));
		jsonAdd(&oItem, "protocol",
		                u8toa(tmp, "%d", monpara.port_type));
		jsonAdd(&oItem, "rtu_addr", webWrite_rtu_addr(tmp, monpara));
		jsonAdd(&oItem, "time_syn_chk",
		                u8toa(tmp, "%d", monpara.bTimeSyn));
		jsonAdd(&oItem, "forward_chk",
		                u8toa(tmp, "%d", monpara.bForward));
		jsonAdd(&oItem, "forward_mtr_num",
		                u8toa(tmp, "%d", monpara.forwardNum));
		jsonAdd(&aItemList, NULL, oItem);
		jsonClear(&oItem);
	}
	//printf(WEBS_DBG"%s\n",oMonPara);
	jsonAdd(&oMonPara, "item", aItemList);
	wpsend(wp, oMonPara);
	jsonFree(&oItem);
	jsonFree(&aItemList);
	jsonFree(&aCommList);
	jsonFree(&aProcList);
	jsonFree(&oMonPara);
	return 0;

}
/**
 * 从页面获取监视参数,保存到文件
 * @param[in]wp
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
	int param_no = 0;		///<参数序号,即数据库的主键,base 0.没有物理意义
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
		if (rtu_addr_str2array(rtu_addr, &monparam.prot_addr[0])<0) {
			web_err_proc(EL);
		}
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
		save_monparam(&monparam, webs_cfg.monpara, param_no);
	}
	return 0;
}

/**
 * 将"255","1","ff"这样的字符串转化成 "0255","0001","00ff"这样的字符数组.终端地址.
 *
 * @param[in] str
 * @param[out] a
 * @retval 指针后移的数量
 */
int rtu_addr_str2array(const char* str, uint8_t a[4])
{
	int i = 0;		//查找的个数,
	int val = 0;
	while (*str!='\0'&&*str!=' ') {
		if (*str>='0'&&*str<='9') {
			val = val*16+(*str-'0');
			goto OK;
		}
		if (*str>='A'&&*str<='F') {
			val = val*16+(*str-'A'+10);
			goto OK;
		}
		if (*str>='a'&&*str<='f') {
			val = val*16+(*str-'a'+10);
			goto OK;
		}
		return -2;
		OK:
		str++;
		i++;
	}
	//change int(0~0xFFFF) to a number(0~15)
	a[0] = (val&0xF000)>>(4*3);
	a[1] = (val&0x0F00)>>(4*2);
	a[2] = (val&0x00F0)>>(4*1);
	a[3] = (val&0x000F)>>(4*0);
	printf("0xabcd&0xf000 = %X\n", 0xabcd&0xf000);
	printf("0xabcd&0xf000,>>24 = %X\n", (0xabcd&0xf000)>>24);
	printf("%X %X %X %X [val=%X]\n", a[0], a[1], a[2], a[3], val);
	return i+1;
}
/**
 * 将"10000 10002 9999"这样的字符串中5位的端口号,不足前面补0,返回原指针偏移数
 * @param[in] str
 * @param[out] a
 * @return
 */
int listen_port_str2array(const char* str, uint8_t a[5])
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
int portstr2u8(const char * str, uint8_t* val)
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
 * @note 使用json格式传输数据,使数据与样式无关,
 *   示例格式参见/doc下 @ref json-date-example
 */
int webSend_netparas(webs_t wp, int netParamNum)
{
	int i;
	stNetparam netparam;
	///@note eth_num 或许和下面的数据元素个数冗余,待定
	///@todo 使用库函数构造和wpsend()函数
	websWrite(wp, T("{\"eth_num\":\"%d\","), netParamNum);
	websWrite(wp, T("\"item\":["));
	for (i = 0; i<netParamNum; i++) {
		if (-1==load_netparam(&netparam, webs_cfg.netpara, i)) {
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
 * @param[in] wp
 * @return
 */
int webRece_netparas(webs_t wp)
{
	stNetparam netparam;
	memset(&netparam, 0x0, sizeof(stNetparam));
	int n;
	int param_no = 0;	///参数序号,即数据库的主键,base 0.没有物理意义
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
		save_netport(&netparam, webs_cfg.netpara, param_no);
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
		erritem |= 0x1;
	}
	int sioports_num = strtol(str_sioports_num, &errstr, 10);
	if (*errstr!='\0'||sioports_num<=0||meter_num>=256) {
		printf("2:%s\n", errstr);
		erritem |= 0x2;
	}
	int netports_num = strtol(str_netports_num, &errstr, 10);
	if (*errstr!='\0'||netports_num<=0||meter_num>=256) {
		printf("3:%s\n", errstr);
		erritem |= 0x4;
	}
	int monitor_ports = strtol(str_monitor_ports, &errstr, 10);
	if (*errstr!='\0'||monitor_ports<=0||meter_num>=256) {
		printf("4:%s\n", errstr);
		erritem |= 0x8;
	}
	int control_ports = strtol(str_control_ports, &errstr, 10);
	if (*errstr!='\0'||control_ports<=0||meter_num>=256) {
		printf("5:%s\n", errstr);
		erritem |= 0x10;
	}
	int sioplan_num = strtol(str_sioplan_num, &errstr, 10);
	if (*errstr!='\0'||sioplan_num<=0||meter_num>=256) {
		printf("6:%s\n", errstr);
		erritem |= 0x20;
	}
	if (erritem==0) {     //只有所有输入都合法
		sysparam->meter_num = meter_num;
		sysparam->sioports_num = sioports_num;
		sysparam->netports_num = netports_num;
		//脉冲已经废弃
		sysparam->monitor_ports = monitor_ports;
		sysparam->control_ports = control_ports;
		sysparam->sioplan_num = sioplan_num;
		ret = save_sysparam(sysparam, webs_cfg.syspara);
		if (ret==-1) {
			web_err_proc(EL);
		}
	}
	response_ok(wp);
	return 0;
}
/**
 * 向页面发送
 * 1. webs的配置信息,版本号,各种目录
 * 2. 主程序(hl3104_com)版本信息
 * @param[out] wp
 * @retval 0
 */
int webSend_info(webs_t wp)
{
	char dir[256] = { 0 };
	int count = readlink("/proc/self/exe", dir, 128);
	if (count<0||count>256) {
		PRINT_RET(count);
		printf(WEBS_ERR"%s\n", __FUNCTION__);
	}
	char* oInfo = jsonNew();
	char tmp[256] = { 0 };
	jsonAdd(&oInfo, "main_version_string", webs_cfg.main_version_string);
	jsonAdd(&oInfo, "info_webbin", dir);
	jsonAdd(&oInfo, "info_webconf", CONF_FILE);
	jsonAdd(&oInfo, "info_weblog", webs_cfg.errlog);
	jsonAdd(&oInfo, "info_rtuconf", webs_cfg.confdir);
	jsonAdd(&oInfo, "info_rtupara", webs_cfg.paradir);
	jsonAdd(&oInfo, "info_wwwroot", webdir);
	jsonAdd(&oInfo, "major", toStr(tmp, "%d", MAJOR));
	jsonAdd(&oInfo, "minor", toStr(tmp, "%d", MINOR));
	jsonAdd(&oInfo, "patchlevel", toStr(tmp, "%d", PATCHLEVEL));
	jsonAdd(&oInfo, "git_version", GIT_VERSION);
	jsonAdd(&oInfo, "build_time", __DATE__" " __TIME__);
	wpsend(wp, oInfo);
	jsonFree(&oInfo);
	return 0;
}
/**
 * 向页面写系统参数,各项数据以json对象形式,客户端解析并添加到指定的框中
 * @param wp
 * @return
 */
int webSend_syspara(webs_t wp)
{
	//stSysParam sysparam;
	//读取并更新内存中的 全局变量 sysparam 结构,保持一致
	int ret = load_sysparam(&sysparam, webs_cfg.syspara);
	if (ret==-1) {
		web_err_proc(EL);
		return -1;
	}
	/**JSON 简单使用,将系统参数抽象为一个对象,其有表计参数个数,
	 串口个数等6个名称/值对
	 在线解析网站: http://jsoneditoronline.org/
	 前端使用eval 或者JSON.parse(IE不支持)即可解析.
	 数据格式参考 @ref json-date-example
	 */
	char* oSysPara = jsonNew();
	char tmp[128] = { 0 };
	jsonAdd(&oSysPara, "meter_num", toStr(tmp, "%u", sysparam.meter_num));
	jsonAdd(
	                &oSysPara,
	                "sioplan_num",
	                toStr(tmp, "%u", sysparam.sioplan_num));
	jsonAdd(
	                &oSysPara,
	                "monitor_ports",
	                toStr(tmp, "%u", sysparam.monitor_ports));
	jsonAdd(
	                &oSysPara,
	                "netports_num",
	                toStr(tmp, "%u", sysparam.netports_num));
	jsonAdd(
	                &oSysPara,
	                "sioports_num",
	                toStr(tmp, "%u", sysparam.sioports_num));
	jsonAdd(
	                &oSysPara,
	                "control_ports",
	                toStr(tmp, "%u", sysparam.control_ports));
	wpsend(wp, oSysPara);
	jsonFree(&oSysPara);
	return 0;
}
/**
 * 接收时间戳和时区(暂时不用,未搞懂),修改为系统参数.
 * @param wp
 * @return
 */
int webRece_syntime(webs_t wp)
{
	char *timestamp = websGetVar(wp, T("timestamp"), T("null"));
	char *timezone = websGetVar(wp, T("timezone"), T("null"));
	//char *dsttime = websGetVar(wp, T("dsttime"), T("null"));
	char *oRet = jsonNew();
	//特别定义成64位长度,希望解决Y2038时修改轻松点.
	int64_t t = atoll(timestamp);     //时间戳数值
	//int tz_min = atoi(timezone);     //与标准时间的差值(分钟)
	//int tz_dsttime=atot(dsttime); //日光节约时,夏令
	printf("time=%lld timezone=%s\n", t, timezone);
	struct timeval tv;
	//struct timezone tz;
	//tz.tz_minuteswest=tz_min;
	//tz.tz_dsttime=tz_dsttime;
	tv.tv_sec = t;
	tv.tv_usec = 0;     //毫秒不要了
	//设置系统时间
	if (settimeofday(&tv, (struct timezone *) 0)<0) {
		web_errno = syn_time;
		web_err_proc(EL);
		jsonAdd(&oRet, "ret", "error");
		jsonAdd(&oRet, "strerror", strerror(errno));
	} else {
		//成功之后再设置硬件时间
		if (system("hwclock -w")<0) {
			web_errno = syn_time;
			web_err_proc(EL);
			jsonAdd(&oRet, "ret", "error");
			jsonAdd(&oRet, "strerror", strerror(errno));
		} else {
			printf("Set system datatime successfully!\n");
			jsonAdd(&oRet, "ret", "success");
		}
	}
	websWrite(wp, "%s", oRet);
	jsonFree(&oRet);
	return 0;
}

/**
 * 将所有表计参数发送到页面.
 * @param wp 页面
 * @param mtrnum 一共有的表计个数
 * @return
 */
int webSend_mtrparams(webs_t wp, int mtrnum)
{
	stMtr mtr;
	memset(&mtr, 0x00, sizeof(stMtr));
	int i;
	char tmp[256];
	jsObj oAll = jsonNew();     //所有要传递的数据,包括一些表单名称
	jsObj aList = jsonNewArray();     //所有要传递的列表,如所有表计规约名称/监视端口
	jsObj aMtrParaList = jsonNewArray();     //所有表计参数的集合,是一个数组
	jsObj oMtrPara = jsonNew();     //一个表计参数
	//串口方案
	for (i = 0; i<sysparam.sioplan_num; i++) {
		jsonAdd(&aList, NULL, u8toa(tmp, "%d", i));
	}
	jsonAdd(&oAll, "portplan", aList);
	jsonClear(&aList);
	//监视端口(端口)
	if (mon_port_num<sysparam.sioports_num) {
		printf(WEBS_ERR"mon_port_num is less than sioports_num!\n");
		web_err_proc(EL);
	}
	for (i = 0; i<sysparam.sioports_num; i++) {
		jsonAdd(&aList, NULL, mon_port_name[i]);
	}
	jsonAdd(&oAll, "port", aList);
	jsonClear(&aList);
	//规约名称
	for (i = 0; i<procotol_num; i++) {
		jsonAdd(&aList, NULL, procotol_name[i]);
	}
	jsonAdd(&oAll, "procotol", aList);
	jsonClear(&aList);
	//表计类型 JD/HA不一样
	///@todo 去掉硬编码!
	char *fact_HA[] = { HOLLEY, WEI_SHENG, LAN_JI_ER, HONG_XIANG, "哈表" };
	char *fact_JD[] = { "华立", "威盛", "哈表",
	                "ABB", "浩宁达", "华隆", "红相", "东方", "许继", "龙电" };
	char **fact;
	int len;
	if (0) {
		fact = fact_HA;
		len = sizeof(fact_HA)/sizeof(fact_HA[0]);
	} else {
		fact = fact_JD;
		len = sizeof(fact_JD)/sizeof(fact_JD[0]);
	}
	for (i = 0; i<len; i++) {
		jsonAdd(&aList, NULL, fact[i]);
	}
	jsonAdd(&oAll, "factory", aList);
	jsonClear(&aList);
	//电表类型,几相几线制
	for (i = 0; i<(int) (sizeof(PW)/sizeof(PW[0])); i++) {
		jsonAdd(&aList, NULL, PW[i]);
	}
	jsonAdd(&oAll, "type", aList);
	jsonClear(&aList);
	//表计参数对象数组循环赋值
	for (i = 0; i<mtrnum; i++) {
		if (-1==load_mtrparam(&mtr, webs_cfg.mtrspara, i)) {
			web_err_proc(EL);
			continue;
		}
		jsonAdd(&oMtrPara, "mtrno", toStr(tmp, "%d", i));
		jsonAdd(&oMtrPara, "iv", toStr(tmp, "%d", mtr.iv));
		jsonAdd(&oMtrPara, "line", a2jsObj(tmp, mtr.line, LINE_LEN));
		jsonAdd(&oMtrPara, "addr", a2jsObj(tmp, mtr.addr, ADDR_LEN));
		jsonAdd(&oMtrPara, "pwd", a2jsObj(tmp, mtr.pwd, PWD_LEN));
		jsonAdd(&oMtrPara, "port", toStr(tmp, "%d", mtr.port));
		jsonAdd(&oMtrPara, "portplan", toStr(tmp, "%d", mtr.portplan));
		jsonAdd(&oMtrPara, "protocol", toStr(tmp, "%d", mtr.protocol));
		jsonAdd(&oMtrPara, "factory", toStr(tmp, "%d", mtr.fact));
		jsonAdd(&oMtrPara, "ph_wire", toStr(tmp, "%d", mtr.p3w4));
		jsonAdd(&oMtrPara, "it_dot", toStr(tmp, "%d", mtr.it_dot));
		jsonAdd(&oMtrPara, "xl_dot", toStr(tmp, "%d", mtr.xl_dot));
		jsonAdd(&oMtrPara, "v_dot", toStr(tmp, "%d", mtr.v_dot));
		jsonAdd(&oMtrPara, "i_dot", toStr(tmp, "%d", mtr.i_dot));
		jsonAdd(&oMtrPara, "p_dot", toStr(tmp, "%d", mtr.p_dot));
		jsonAdd(&oMtrPara, "q_dot", toStr(tmp, "%d", mtr.q_dot));
		jsonAdd(&oMtrPara, "ue", toStr(tmp, "%d", mtr.ue));	//单位伏特,没有小数
		jsonAdd(&oMtrPara, "ie", toStr(tmp, "%.1f", mtr.ie/1000.0));
		//添加到数组
		jsonAdd(&aMtrParaList, NULL, oMtrPara);
		jsonClear(&oMtrPara);
	}
	jsonAdd(&oAll, "items", aMtrParaList);
	wpsend(wp, oAll);
	jsonFree(&oAll);
	jsonFree(&oMtrPara);
	jsonFree(&aList);
	jsonFree(&aMtrParaList);
	return 0;
}
/**
 * 将纯数值数组转化为jsObj对象.并返回指向这个对象的指针
 * 一个元素一位0~9.
 * @param tmp
 * @param array
 * @param n
 * @return
 */
jsObj a2jsObj(char *tmp, uint8_t * array, int n)
{
	int i;
	for (i = 0; i<n; i++) {
		sprintf(tmp+i, "%1d", array[i]);
	}
	return tmp;
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
	uint32_t err[MAX_MTR_NUM] = { 0 };
	///表计数目
	int mtr_num = 0;
	mtr_num = getmtrparams(amtr, wp, err);
	printf("get param from clint ret %d \n", mtr_num);
	if (mtr_num>0) {     //只有所有输入都合法
		for (i = 0; i<mtr_num; i++) {
			saveret = save_mtrparam(&amtr[i], webs_cfg.mtrspara,
			                amtr[i].mtrno);
#if 0
			printf("1047 i=%d saveret=%d\n", i, saveret);
			printf("amtr[i].mtrno=%d\n", amtr[i].mtrno);
#endif
		}
	} else {
		saveret = mtr_num;
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
	uint i;
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
	if(save_savecycle(sav, webs_cfg.stspara)<0){
		web_err_proc(EL);
	}
	return 0;
}
/**
 * 从文件读取储存周期,发送(写)到页面 .
 * @param wp
 * @return
 */
int webSend_savecycle(webs_t wp)
{
	stSave_cycle sav[SAVE_CYCLE_ITEM];
	int ret = load_savecycle(sav, webs_cfg.stspara);
	if (ret==-1) {
		web_err_proc(EL);
		return -1;
	}
	char* oSavCycle = jsonNew();
	char* oCycleList = jsonNewArray();
	char* oItem = jsonNew();
	char* oItemArray = jsonNewArray();
	uint j;
	for (j = 0; j<sizeof(SAVE_CYCLE)/sizeof(SAVE_CYCLE[0]); j++) {
		jsonAdd(&oCycleList, NULL, SAVE_CYCLE[j]);
	}
	for (j = 0; j<SAVE_CYCLE_ITEM; j++) {
		jsonAdd(&oItemArray, NULL, addItem(&oItem, sav[j]));
#if DEBUG_PRINT_SAVE_CYCLE
		printf(WEBS_DBG"oItem:%s\n",oItemArray);
#endif
		jsonClear(&oItem);
	}
	jsonAdd(&oSavCycle, "cycle", oCycleList);
	jsonAdd(&oSavCycle, "item", oItemArray);
#if DEBUG_PRINT_SAVE_CYCLE
	printf(WEBS_DBG"oSavCycle:%s\n",oSavCycle);
#endif
	wpsend(wp, oSavCycle);
	jsonFree(&oItemArray);
	jsonFree(&oItem);
	jsonFree(&oCycleList);
	jsonFree(&oSavCycle);
	return 0;
}
/**
 * 从文件读取采集周期,发送(写)到页面 .
 * @param wp
 * @return
 */
int webSend_collect_cycle(webs_t wp)
{
	stCollect_cycle sav[COLLECT_CYCLE_ITEM];
	int ret = load_collect_cycle(sav, webs_cfg.ctspara);
	if (ret==-1) {
		web_err_proc(EL);
		return -1;
	}
	char* oCollectCycle = jsonNew();
	char* oCycleList = jsonNewArray();
	char* oItem = jsonNew();
	char* oItemArray = jsonNewArray();
	uint j;
	for (j = 0; j<sizeof(COLLECT_CYCLE)/sizeof(COLLECT_CYCLE[0]); j++) {
		jsonAdd(&oCycleList, NULL, COLLECT_CYCLE[j]);
	}
	for (j = 0; j<COLLECT_CYCLE_ITEM; j++) {
		jsonAdd(&oItemArray, NULL,addCollectItem(&oItem, sav[j]));
#if DEBUG_PRINT_COLLECT_CYCLE
		printf(WEBS_DBG"oItem:%s\n",oItemArray);
#endif
		jsonClear(&oItem);
	}
	jsonAdd(&oCollectCycle, "cycle", oCycleList);
	jsonAdd(&oCollectCycle, "item", oItemArray);
#if DEBUG_PRINT_COLLECT_CYCLE
	printf(WEBS_DBG"oCollectCycle:%s\n",oCollectCycle);
#endif
	wpsend(wp, oCollectCycle);
	jsonFree(&oItemArray);
	jsonFree(&oItem);
	jsonFree(&oCycleList);
	jsonFree(&oCollectCycle);
	return 0;
}
/**
 * 从页面获取采样周期参数,保存到本地文件中
 * @param wp
 * @return
 */
int webRece_collect_cycle(webs_t wp)
{
	int i;
	int n;
	stCollect_cycle sav[COLLECT_CYCLE_ITEM];
	char *flags = websGetVar(wp, T("flag"), T("null"));
	char *cycle = websGetVar(wp, T("cycle"), T("null"));
	for (i = 0; i<COLLECT_CYCLE_ITEM; i++) {
		printf(WEBS_DBG"i=%d\n",i);
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
	if(save_collect_cycle(sav, webs_cfg.ctspara)<0){
		web_err_proc(EL);
	}
	return 0;
}
/**
 *  存储周期的小项目json名&值对
 * @param oItem
 * @param sav
 * @return
 */
char *addItem(char **oItem, stSave_cycle sav)
{
	char value[256] = { 0 };
	jsonAdd(oItem, "en", toStr(value, "%d", sav.enable));
	jsonAdd(oItem, "t", toStr(value, "%d", sav.cycle));
	return *oItem;
}
/**
 * 采集周期的小项目json名&值对
 * @param oItem
 * @param sav
 * @return
 */
char *addCollectItem(char **oItem, stCollect_cycle sav)
{
	char value[256] = { 0 };
	jsonAdd(oItem, "en", toStr(value, "%d", sav.enable));
	jsonAdd(oItem, "t", toStr(value, "%d", sav.cycle));
	return *oItem;
}

/**
 * 从页面中获取文本,保存到本地文本文件中.
 * @note 暂时不知道长度的限制,200k以下(左右)的文本文件是经过测试的
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
	char buf[256] = { 0 };
	int ret;
	FILE*fp = fopen(file, "r");
	if (fp==NULL ) {
		websWrite(wp, T("No Error."));
		goto WEB_END;
	}
	while (1) {
		ret = fread(&buf, sizeof(char), 255, fp);
		if (ret>0) {
			websWrite(wp, T("%s"), buf);
			memset(buf, 0x0, 256);
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
void print_array(const uint8_t *a, const int len)
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
 * 储存周期,较为简单,
 * @todo 使用json构造库构造,统一方便.
 * @param[out] wp
 * @param name
 * @param sav
 * @return
 */
int jsonSavCycle(webs_t wp, const char* name, const stSave_cycle sav)
{
	websWrite(wp, T("\"%s\":{"), name);
	websWrite(wp, T("\"en\":"));
	websWrite(wp, T("\"%d\","), sav.enable);
	websWrite(wp, T("\"t\":"));
	websWrite(wp, T("\"%d\"}"), sav.cycle);
	return 0;
}
/**
 * 去除输入数组中的前导空白符和后导空白符
 * @param[in] in 待修改的数组,会被修改
 * @param[in] len 数组构成的字符串长度.
 * @retval 指向修改好的字符串首地址指针
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
	if (webdir!=NULL )     //不使用了就尽早释放.
		free(webdir);
	//
	if (webs_cfg.main_version_string!=NULL ) {
		free(webs_cfg.main_version_string);
		webs_cfg.main_version_string = NULL;
	}
	//应用程序名称
	if (webs_cfg.appname!=NULL ) {
		free(webs_cfg.appname);
		webs_cfg.appname = NULL;
	}
	//配置文件项 WEBS_DEFAULT_PORT
	if (webs_cfg.port!=NULL &&
	                //如果使用了备用的端口号,分配在静态区,不能/需要释放.
	                webs_cfg.default_port!=1) {
		free(webs_cfg.port);
		webs_cfg.port = NULL;
	}
	if (webs_cfg.sslport!=NULL &&
	                webs_cfg.default_sslport!=1) {
		free(webs_cfg.sslport);
		webs_cfg.sslport = NULL;
	}
	if (webs_cfg.errlog!=NULL &&
	                //如果使用了备用的错误日志文件,分配在静态区,不能/需要释放.
	                webs_cfg.default_errlog!=1) {
		free(webs_cfg.errlog);
		webs_cfg.errlog = NULL;
	}
	if (webs_cfg.paradir!=NULL ) {
		free(webs_cfg.paradir);
		webs_cfg.paradir = NULL;
	}
	if (webs_cfg.syspara!=NULL ) {
		free(webs_cfg.syspara);
		webs_cfg.syspara = NULL;
	}
	if (webs_cfg.mtrspara!=NULL ) {
		free(webs_cfg.mtrspara);
		webs_cfg.mtrspara = NULL;
	}
	if (webs_cfg.sioplan!=NULL ) {
		free(webs_cfg.sioplan);
		webs_cfg.sioplan = NULL;
	}
	if (webs_cfg.netpara!=NULL ) {
		free(webs_cfg.netpara);
		webs_cfg.netpara = NULL;
	}
	if (webs_cfg.monpara!=NULL ) {
		free(webs_cfg.monpara);
		webs_cfg.monpara = NULL;
	}
	if (webs_cfg.retranTable!=NULL ) {
		free(webs_cfg.retranTable);
		webs_cfg.retranTable = NULL;
	}
	if (webs_cfg.stspara!=NULL ) {
		free(webs_cfg.stspara);
		webs_cfg.stspara = NULL;
	}
	if (webs_cfg.ctspara!=NULL ) {
		free(webs_cfg.ctspara);
		webs_cfg.ctspara = NULL;
	}
	if (webs_cfg.confdir!=NULL ) {
		free(webs_cfg.confdir);
		webs_cfg.confdir = NULL;
	}
	if (webs_cfg.protocol!=NULL ) {
		free(webs_cfg.protocol);
		webs_cfg.protocol = NULL;
	}
	if (webs_cfg.monparam_name!=NULL ) {
		free(webs_cfg.monparam_name);
		webs_cfg.monparam_name = NULL;
	}
	//监视参数端口名称
	for (i = 0; i<mon_port_num; i++) {
		if (mon_port_name[i]!=NULL ) {
			free(mon_port_name[i]);
			mon_port_name[i] = NULL;
		}
	}
	//规约名称
	for (i = 0; i<procotol_num; i++) {
		if (procotol_name[i]!=NULL ) {
			free(procotol_name[i]);
			procotol_name[i] = NULL;
		}
	}
}
/**
 * 对于客户端的更新请求给予响应.如果更新成功则简单的返回这个成功响应
 * @param wp 页面
 */
void response_ok(webs_t wp)
{
	websWrite(wp, T("{\"ret\":\"ok\"}"));
}

//#pragma  GCC diagnostic ignored  "-Wunused-parameter"
