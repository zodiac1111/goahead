/**
 * @file sysfunction.c
 * 对应界面系统参数功能部分
 */
#include "sysfunction.h"
static int webSend_txtfile(webs_t wp, const char*file);
static int webRece_txtfile(webs_t wp, char_t *query, const char* file);
static int webSend_info(webs_t wp);
struct sembuf sb;     ///<信号量操作,该功能完善中.
int semid;     ///<信号量id
char * webdir;
union semun sem;
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
/**
 * 读取文本文件,写到页面中.
 * @param[out] wp
 * @param[in] file
 */
static int webSend_txtfile(webs_t wp, const char*file)
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
 * 从页面中获取文本,保存到本地文本文件中.
 * @note 暂时不知道长度的限制,200k以下(左右)的文本文件是经过测试的
 * @param wp
 * @param query
 * @param file
 */
static int webRece_txtfile(webs_t wp, char_t *query, const char* file)
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
 * 向页面发送
 * 1. webs的配置信息,版本号,各种目录
 * 2. 主程序(hl3104_com)版本信息
 * @param[out] wp
 * @retval 0
 */
static int webSend_info(webs_t wp)
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
