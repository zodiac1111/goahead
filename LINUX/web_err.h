/**
 * @file web_err.h
 * web服务器错误相关.
 */
#ifndef _WEB_ERR_H_
#define _WEB_ERR_H_
#include "conf.h"
#define ERR (-1) ///<错误
#define EL T(__FILE__),T(__FUNCTION__),__LINE__
#define EL_ARGS	const char* file,const char* func,int line
extern void web_err_proc(EL_ARGS);
int save_log(const char * errstring,const char*filename);
///错误号
extern int web_errno;
///错误描述
extern const char *myweberrstr[];
///错误描述枚举
enum e_errno {
	no_error=0,
	open_sysparam_file = 1,
	sysfile_size_err,
	write_sysfile_size_err,
	read_sysfile_err,
	write_sysfile_err,
	update_mtr_file_err,
	update_sio_file_err,
	update_netparam_file_err,
	update_monparam_file_err,
	//表计参数方案
	open_mtrcfgfile_err,
	no_this_mtrparam,
	read_mtrcfgfile_err,
	write_mtrcfgfile_err,
	mtr_file2men_err,
	//串口方案
	open_sioplan_cfgfile_err,
	no_this_sioplan,
	read_sioplan_cfgfile_err,
	write_sioplan_cfgfile_err,
	//net parameter
	open_netparam_cfgfile_err,
	no_this_netparam,
	read_netparam_cfgfile_err,
	write_netparam_cfgfile_err,
	ftruncate_netparam_cfgfile_err,
	//monitor parameter
	open_monparam_cfgfile_err,
	no_this_monparam,
	read_monparam_cfgfile_err,
	write_monparam_cfgfile_err,
	//save cycle item:
	open_savecycle_cfgfile_err,
	savecycle_cfgfile_size_err,
	read_savecycle_cfgfile_err,
	write_savecycle_cfgfile_err,
	//initialize protocol
	open_protocolfile_err,
	toomany_protocol_err,
	//initialize monitor port name
	open_monitor_name_file_err,
	//历史数据
	tou_timerange_err,
	tou_stime_err,
	tou_etime_err,
	open_tou_file,
	read_tou_file_filehead,
	read_tou_file_dat,
	//其他系统警告
	use_backup_err_log,
	use_backup_port,
	use_backup_sslport,
};
#endif
