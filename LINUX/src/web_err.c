/**
 * @file web_err.c
 * 服务器错误字符串输出
 */
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include "conf.h"
#include "color.h"
#include "web_err.h"
#include "param.h"
#include <time.h>
static int
save_log(const char *strtime, const char * errstring, const char*filename);
int web_errno = 0;
const char *myweberrstr[] = {
                [ErrNoError]="No Error.",
                ///系统参数相关
                [ErrOpen_sysparam_file]="Open system confingure file "FILE_SYSPARA" err.",
                [ErrSysfileSize]= "System parameter file size err.",
                [ErrWriteSysParamFileSize]="Save system parameter err,size is error.",
                [ErrReadSysParamFile]="读取系统参数文件"FILE_SYSPARA"时出错",
                [write_sysfile_err]="写入系统参数文件"FILE_SYSPARA"时出错",
                [update_mtr_file_err]="更新表计参数数量"CFG_MTR"时出错",
                [ErrUpdateSioFile]="更新串口方案文件失败",
                [ErrUpdate_netparam_file_err]="更新网络参数文件",
                [ErrUpdate_monparam_file_err]="更新监视参数",
                ///表计参数相关
                [ErrOpenMtrcfgFile]="打开表计参数文件"CFG_MTR"时出错",
                [ErrNoSuchMeterParam] =CFG_MTR"中没有这个表计参数",
                [ErrReadMetercfgFile]="读取表计参数文件"CFG_MTR"时出错",
                [write_mtrcfgfile_err]="Write Meter cfg file Err."
                "写入表计参数文件"CFG_MTR"时出错",
                [mtr_file2men_err]="Err when change the Meter Parameter from "
                "File struct to Data struct. "
                "从表计参数文件结构转化为数据结构时出错",
                ///串口方案参数相关
                [open_sioplan_cfgfile_err]=
                "Open uartplan cfg file "CFG_SIOPALN" err.",
                [no_this_sioplan]=
                "No such com plan record.",
                [read_sioplan_cfgfile_err]=
                "Read uartplan cfg file"CFG_SIOPALN" err.",
                [write_sioplan_cfgfile_err]=
                "Update the uartplan config file "CFG_SIOPALN" err.",
                ///网络参数
                [open_netparam_cfgfile_err]="Open netparam cfg file err:"CFG_NET,
                [no_this_netparam]="没有这个网络参数",
                [read_netparam_cfgfile_err]="Read netparam cfg file err:"CFG_NET,
                [write_netparam_cfgfile_err]="Write netparam cfg file err:"CFG_NET,
                [ftruncate_netparam_cfgfile_err]="Ftruncate netparam cfg "
                "file err:"CFG_NET,
                //监视参数
                [open_monparam_cfgfile_err]="Open Monitor parameter cfg file err:"
                CFG_MON_PARAM".",
                [no_this_monparam]="No such Monitor parameter .",
                [read_monparam_cfgfile_err]=
                "Read Monitor parameter file err:"CFG_MON_PARAM".",
                [write_monparam_cfgfile_err]=
                "Write Monitor Paramter file err:"CFG_MON_PARAM".",
                //储存周期
                [open_savecycle_cfgfile_err]="Err when open the save cycle cfg file.",
                [savecycle_cfgfile_size_err]="Save cycle cfg file size is error.",
                [read_savecycle_cfgfile_err]="Read save cycle config file",
                [write_savecycle_cfgfile_err]="Write save cycle cfg file error.",
                //采集周期
                [open_collect_cycle_cfgfile_err]="Err when open the collect cycle conf file.",
                [collect_cycle_cfgfile_size_err]="Save collect cfg file size is error.",
                [read_collect_cycle_cfgfile_err]="Read collect cycle config file",
                [write_collect_cycle_cfgfile_err]="Write collect cycle cfg file error.",
                //规约初始化
                [open_protocolfile_err]="Open Protocol file err:"PORC_FILE,
                [toomany_protocol_err]="Too many Protocol in "PORC_FILE"."
                "Quantity of Protocol is Greater than MAX_PROCOTOL_NUM"
                "[128].",
                [open_monitor_name_file_err]="Open Monitor Port name file "
                MON_PORT_NAME_FILE" err.",
                //历史数据操作
                [tou_timerange_err]="Data end time is less than start time.",
                [tou_stime_err]="Start time is 1970.",
                [tou_etime_err]="End time is 1970.",
                [open_tou_file]="Open history Tou data err.",
                [read_tou_file_filehead]="Read history Tou file(file head) err.",
                [read_tou_file_dat]="Read history Tou file(data) err.",
                //系统警告
                [use_backup_err_log]="Can't find "YELLOW"errlog"_COLOR
                " in "RED CONF_FILE _COLOR
                ",use file " RED BACKUP_ERR_FILE _COLOR" as a backup.",
                [use_backup_port]="Can't find "YELLOW"port"_COLOR" in "RED CONF_FILE _COLOR
                ",use " RED WEBS_DEFAULT_PORT _COLOR
                " as a backup port number.",
                [use_backup_sslport]="Can't find "YELLOW"sslport"_COLOR
                " in "RED CONF_FILE _COLOR
                ",use " RED WEBS_DEFAULT_SSL_PORT _COLOR
                " as a backup sslport number.",
                [syn_time]="Set system datatime error.",
                [eno_realtime_tou_mtrnum_too_small]="Realtime tou mtrnum too small.",
                [eno_realtime_tou_mtrnum_too_big]="Realtime tou mtrnum too big.",
                [ErrNotSupportHostNameFunction]="1.5版本特殊:hostname函数错误,使用默认的127.0.0.1代替.",
        };
/**
 * 服务器错误处理函数.
 * 处理web_errno错误号,控制台打印,清除web_errno.
 * 以较为简单的形式保存到文件.
 * @param EL_ARGS
 */
void web_err_proc(EL_ARGS)
{
	time_t timer = time(NULL );
	struct tm * t = localtime(&timer);
	//错误时间
	char strtime[25] = { 0 };
	//错误描述
	char errstring[MAX_ERR_LOG_LINE_LENTH] = { 0 };
	//错误等级
	if (web_errno<0) {
		web_errno = 0;
		return;
	}
	sprintf(strtime, "%04d-%02d-%02d %02d:%02d:%02d",
	                t->tm_year+1900, t->tm_mon+1, t->tm_mday,
	                t->tm_hour, t->tm_min, t->tm_sec);
	sprintf(errstring, "%s[%d](%s,%s:%d)%s[%d]",
	                 myweberrstr[web_errno],web_errno,
	                file, func, line, strerror(errno),errno);
	if (myweberrstr[web_errno]!=NULL ) {
		printf(WEBS_ERR"%s %s\n", strtime, errstring);
	}
	// 写入文件
	save_log(strtime, errstring, webs_cfg.errlog);
	web_errno = 0;
	return;
}
/**
 * 扩展的错误处理函数,支持不定个数参数打印,
 * 使用例子:
 * 	web_err_proc_Ex(EL,"发生了错误.");
 * 	web_err_proc_Ex(EL,"发生了错误,返回值:ret=%d.",iRet);
 * 	web_err_proc_Ex(EL,"发生了错误,返回值:ret=%d value=%f v2=%s"
 * 		,iRet,fValue,strSomeString);
 * @param EL_ARGS (宏) EL (宏)
 * @param format 格式化字符串 + ... 不定参数
 */
void web_err_procEx(EL_ARGS,const char*format, ... )
{
	time_t timer = time(NULL );
	struct tm * t = localtime(&timer);
	//错误时间
	char strtime[25] = { 0 };
	//错误描述 RealErrstring = errstring + errstringEx ;
	char errstring[MAX_ERR_LOG_LINE_LENTH] = { 0 };
	char errstringEx[MAX_ERR_LOG_LINE_LENTH] = { 0 };//扩展的描述信息
	char RealErrstring[MAX_ERR_LOG_LINE_LENTH] = { 0 };
	va_list ap;
	if (web_errno<0) {
		web_errno = 0;
		return;
	}
	sprintf(strtime, "%04d-%02d-%02d %02d:%02d:%02d",
	                t->tm_year+1900, t->tm_mon+1, t->tm_mday,
	                t->tm_hour, t->tm_min, t->tm_sec);
	sprintf(errstring, "%s[%d](%s,%s:%d)%s[%d]",
	                 myweberrstr[web_errno],web_errno,
	                file, func, line, strerror(errno),errno);
	// --- 扩展的字符串
	va_start(ap,format);
	vsprintf( errstringEx,format, ap);
	va_end(ap);
	sprintf(RealErrstring,"%s %s",errstring,errstringEx);
	if (myweberrstr[web_errno]!=NULL ) {
		printf(WEBS_ERR"%s %s\n", strtime, RealErrstring);
	}
	// 写入文件
	save_log(strtime, RealErrstring, webs_cfg.errlog);
	web_errno = 0;
	return;
}
/**
 * 保存服务器错误日志.
 * @param[in] errstring
 * @param[in] filename
 * @return
 */
static int save_log(const char *strtime, const char * errstring, const char*filename)
{
	FILE*fp = NULL;
	fp = fopen(filename, "a");
	if (fp==NULL ) {
		fp = fopen(filename, "w");
		if (fp==NULL ) {
			printf("errfile:%s\n", filename);
			perror(WEBS_ERR"create errlog file err");
			return -1;
		}
	}
	fseek(fp, 0, SEEK_END);
	int flen = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	if (flen>=MAX_ERR_LOG_FILE_SIZE) {     //文件过长,简单的截断为0
		int fd = fileno(fp);
		ftruncate(fd, 0);
	}
	fprintf(fp, "%s %s\n", strtime, errstring);
	fclose(fp);
	return 0;
}
