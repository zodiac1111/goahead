/**
 * @file conf.h
 * 配置服务器一些硬编码的参数
 *  Created on: 2013-1-24
 *      Author: lee
 */
#ifndef CONF_H_
#define CONF_H_
#include <version.h>
typedef struct cfg{
	//   错误日志绝对路径
	char *errlog;
	//   para目录(用于计算下面的绝对路径)
	char *paradir;
	char *syspara; //系统参数文件绝对路径
	char *mtrspara; //表计参数文件绝对路径
	char *sioplan; //串口方案文件绝对路径
	char *netpara;//网络参数
	char *monpara; //监视参数
	char *retranTable; //转发表
	char *stspara; //储存周期
	//   conf目录(用于计算下面的绝对路径
	char *confdir;
	char *protocol; //规约配置
	char *monparam_name;//监视参数名称
}stCfg;
extern stCfg webs_cfg;
//调试信息开关.用于打印一些信息. 1 表示开启此项调试信息;0 表示关闭.
#define DEBUG_INFO_FORM 1 ///<表单提交函数的信息,每个/次表单提交打印一次提交的内容
#define DEBUG_PARSE_CONF_FILE 0 ///<解析配置文件时的调试信息(goahead.conf),
#define DEBUG_SIOPLAN_INFO 0 ///<打印串口方案相关调试信息
#define DEBUG_PRINT_NETPARAM 0 ///<打印网口参数相关调试信息
#define DEBUG_PRINT_MONPARAM 0 ///<打印监视参数相关调试信息
#define DEBUG_PRINT_MTRPARAM 0 ///<打印表计参数相关调试信息
#define DEBUG_PRINT_SAVE_CYCLE 0 ///<打印储存周期调试信息
///@todo 独立成为配置文件项
#define WEBS_DEFAULT_HOME	T("index.html") /* Default home page */
#define WEBS_DEFAULT_PORT	8080 //默认 HTTP 端口,只有root有权限访问1000以下端口
#define WEBS_DEFAULT_SSL_PORT	4433 /* Default HTTPS port */
//信息字符
#define PREFIX "[webs]" ///<应用程序信息前缀
#define WEBS_DBG PREFIX"\e[33mDebug\e[0m>" ///<调试信息
#define WEBS_INF PREFIX"\e[32mInfo\e[0m>" ///<一般信息
#define WEBS_WAR PREFIX"\e[33mWarning\e[0m>" ///<警告信息
#define WEBS_ERR PREFIX"\e[31mError\e[0m>" ///<错误信息
//各种文件和路径,所有的相对路径都是相对webroot目录(www/wwwdemo)的相对路径.
//而 webroot 则在下面的配置文件中指明.
//webs服务器应用程序配置文件
#if __arm__  	//运行在终端平台arm上.
#define CONF_FILE "/mnt/nor/conf/goahead.conf"
#else 		//运行在i368,host调试平台上.
#define CONF_FILE "/home/lee/workspace/goahead/conf/goahead-host.conf"
#endif
///@todo 这些项目可以放到配置文件中.程序中需要一些修改
//webs服务器错误文件相关
#define ERR_LOG "./err.log" ///<错误日志文件
#define MAX_ERR_LOG_LINE_LENTH 1024 ///<日志文件单行最大长度(字节)
#define MAX_ERR_LOG_FILE_SIZE (100*1024) ///<错误日志文件最大尺寸(字节)
//conf配置相关文件
#define CFG_DIR "../para/"///<参数路径
#define FILE_SYSPARA "sysspara.cfg"///<系统参数文件,关系到下面的其他参数
#define CFG_MTR "mtrspara.cfg"///<表计参数配置文件
#define CFG_SIOPALN "sioplan.cfg"///<串口方案配置文件
#define CFG_NET "netpara.cfg"///<网络參數配置文件
#define CFG_MON_PARAM "monpara.cfg"///<监视参数数配置文件
#define CFG_FORWARD_TABLE "retranTable.cfg"///<转发表信息文件
#define CFG_SAVE_CYCLE "stspara.cfg"///<采集保存周期和项目配置文件
//para参数相关文件.
#define PROC_CFG_DIR "../conf/" ///<规约文本文件目录
#define PORC_FILE "protocol_config.txt"///<主站规约配置文件
#define MAX_PROCOTOL_NUM 128 ///<规约文件中最大规约条目开始时分配这么多
#define MON_PORT_NAME_FILE "monparam_name.conf"///<监视端口描述文件

//信息
#define PRINT_WELCOME {					\
	printf("\t\tWelcome to use "		\
	"\e[31mHolley web services\e[0m\t\t\n");	\
	}
//打印版本信息
#define PRINT_VERSION { 					\
		printf("\tVersion:\t\e[32m%d.%d.%d\e[0m"	\
			" [\e[31m%s\e[0m]\n", 	 \
		MAJOR, MINOR, PATCHLEVEL,GIT_VERSION); 		\
	}
//打印编译构建的日期和时间，类似：Dec  3 2012 09:59:57
#define PRINT_BUILD_TIME {					\
		printf("\tBuild time:\t\e[32m%s %s\e[0m\n",	\
		 __DATE__, __TIME__);		\
	}

#endif /* CONF_H_ */
