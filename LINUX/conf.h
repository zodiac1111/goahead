/**
 * @file conf.h
 * 配置服务器一些硬编码的参数
 *  Created on: 2013-1-24
 *      Author: lee
 */
#ifndef CONF_H_
#define CONF_H_
//调试开关.用于打印一些信息.
#define DEBUG_INFO_FORM 1 ///<打印表单提交函数的信息
#define DEBUG_PARSE_CONF_FILE 0 ///<打印配置文件调式信息
#define DEBUG_SIOPLAN_INFO 0 ///<打印串口方案相关调试信息
#define DEBUG_PRINT_NETPARAM 0 ///<打印网口参数相关调试信息
#define DEBUG_PRINT_MONPARAM 0 ///<打印监视参数相关调试信息
#define DEBUG_PRINT_MTRPARAM 0 ///<打印表计参数相关调试信息
//信息字符
#define PREFIX "[webs]" ///<应用程序信息前缀
#define PREFIX_INF PREFIX"info>" ///<一般信息
#define PREFIX_WAR PREFIX"warning>" ///<警告信息
#define PREFIX_ERR PREFIX"error>" ///<错误信息
//各种文件和路径,所有的相对路径都是相对webroot目录(www/wwwdemo)的相对路径.
//而 webroot 则在下面的配置文件中指明.
//webs服务器应用程序配置文件
#if __arm__  	//运行在终端平台arm上.
#define CONF_FILE "/mnt/nor/conf/goahead.conf"
#else 		//运行在i368,host调试平台上.
#define CONF_FILE "/home/lee/workspace/goahead/conf/goahead.conf"
#endif
//webs服务器错误文件相关
#define ERR_LOG "./err.log" ///<错误日志文件
#define MAX_ERR_LOG_LINE_LENTH 1024 ///<日志文件单行最大长度(字节)
#define MAX_ERR_LOG_FILE_SIZE (100*1024) ///<错误日志文件最大尺寸(字节)
//conf配置相关文件
#define CFG_DIR "../para/"///<参数路径
#define CFG_SYS CFG_DIR"sysspara.cfg"///<系统参数文件,关系到下面的其他参数
#define CFG_MTR CFG_DIR"mtrspara.cfg"///<表计参数配置文件
#define CFG_SIOPALN CFG_DIR"sioplan.cfg"///<串口方案配置文件
#define CFG_NET CFG_DIR"netpara.cfg"///<網路參數配置文件
#define CFG_MON_PARAM CFG_DIR"monpara.cfg"///<监视参数数配置文件
#define CFG_FORWARD_TABLE CFG_DIR"retranTable.cfg"///<转发表信息文件
#define CFG_SAVE_CYCLE CFG_DIR"stspara.cfg"///<采集保存周期和项目配置文件
//para参数相关文件.
#define PROC_CFG_DIR "../conf/" ///<规约文本文件目录
#define PORC_FILE PROC_CFG_DIR"protocol_config.txt"///<主站规约配置文件
#define MAX_PROCOTOL_NUM 128 ///<规约文件中最大规约条目开始时分配这么多
#define MON_PORT_NAME_FILE PROC_CFG_DIR"monprarm_config.txt"///<监视端口描述文件


#endif /* CONF_H_ */
