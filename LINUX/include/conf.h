/**
 * @file conf.h
 * 配置服务器一些硬编码的参数,端口,配置文件,日志等 调试开关 打印信息 版本信息
 *  Created on: 2013-1-24
 *      Author: lee
 */
#ifndef CONF_H_
#define CONF_H_
#include "../version.h"
#include "color.h"
///webs服务器配置参数.开始时加载到内存,结束时释放
typedef struct cfg{
	char *main_version_string;//主程序版本字符串
	char *appname;//应用程序全路径
	char *port;//默认端口
	int default_port; //若使用了硬编码的端口,在webs_free()时不需要释放内存.
	char *sslport;//ssl默认端口(ssl未启用)
	int default_sslport; //同上上
	//   错误日志绝对路径
	char *errlog;
	int default_errlog;
	//   para目录(用于计算下面的绝对路径)
	char *paradir;
	char *syspara; //系统参数文件绝对路径
	char *mtrspara; //表计参数文件绝对路径
	char *sioplan; //串口方案文件绝对路径
	char *netpara;//网络参数
	char *monpara; //监视参数
	char *retranTable; //转发表
	char *stspara; //储存周期
	char *ctspara; //采集周期
	//   conf目录(用于计算下面的绝对路径
	char *confdir;
	char *protocol; //规约配置
	char *monparam_name;//监视参数名称
}stCfg;
extern stCfg webs_cfg;

//调试信息开关.用于打印一些信息. 1 表示开启此项调试信息;0 表示关闭.
#define DEBUG_INFO_FORM 1 ///<表单提交函数的信息,每个/次表单提交打印一次提交的内容
#define DEBUG_PARSE_CONF_FILE 0 ///<解析配置文件(goahead.conf)时的调试信息,
#define DEBUG_SIOPLAN_INFO 0 ///<打印串口方案相关调试信息
#define DEBUG_PRINT_NETPARAM 0 ///<打印网口参数相关调试信息
#define DEBUG_PRINT_MONPARAM 0 ///<打印监视参数相关调试信息
#define DEBUG_PRINT_MTRPARAM 0 ///<打印表计参数相关调试信息
#define DEBUG_PRINT_SAVE_CYCLE 0 ///<打印储存周期调试信息
#define DEBUG_PRINT_COLLECT_CYCLE 0 ///<采集周期调试信息
#define DEBUG_PRINT_REALTIME_TOU_DAT 0///实时电量数据打印调试.
#define DEBUG_CONF_FULLPATH 0 ///<解析配置文件合并成为完整的绝对路径(打印调试信息)
#define DEBUG_JSON_DEMO 0 ///<json生成器(不包括解析器)的使用demo示例

///@note 端口相保存在配置文件中,当文件读取失败时使用硬编码的默认值.
#define WEBS_DEFAULT_HOME	T("index.html") /* Default home page */
//遗留的goahead2.5.0版本的端口宏定义,现在放到配置文件中实现
//保留用于读取配置文件错误时使用
#define WEBS_DEFAULT_PORT	"8080" //默认HTTP端口,仅root有权限访问1000以下端口
#define WEBS_DEFAULT_SSL_PORT	"4433" // Default HTTPS port

//一些限制定义宏
//页面一次发送最大长度,超长则分多次发送.否则会截断.wpsend()函数调用
#define WP_MAX_LEN (512)
#define MAIN_PROGRAM_VERSION_STRING_MAX_LENGTH (128) //主程序版本字符串最大长度
//服务器程序自动检查升级
#define  AUTO_UPDATE 1
#if AUTO_UPDATE
  #define UP_SUFFIX ".update"//更新文件后缀
  #if __arm__ ==1
    #define PROG_NAME "/mnt/nor/bin/webs"
    #define UPDATE_FILE_NAME  PROG_NAME UP_SUFFIX
  #else
    #define PROG_NAME "/home/zodiac1111/workspace/goahead/LINUX/webs"
    #define UPDATE_FILE_NAME  PROG_NAME UP_SUFFIX
  #endif
#endif

//信息字符
#define PREFIX "[webs]" ///<应用程序信息前缀
#define WEBS_DBG PREFIX YELLOW"Debug>"_COLOR ///<调试信息
#define WEBS_INF PREFIX GREEN"Info>"_COLOR ///<一般信息
#define WEBS_WAR PREFIX YELLOW"Warning>"_COLOR ///<警告信息
#define WEBS_ERR PREFIX RED BOLD"Error>"_COLOR///<错误信息
#define TIME_ZONE (8) //定义时区东8区 :+8
#define TIME_ZONE_SEC (TIME_ZONE*60*60) //时区对应的秒数
//各种文件和路径,所有的相对路径都是相对webroot目录(www/wwwdemo)的相对路径.
//而 webroot 则在下面的配置文件中指明.
//webs服务器应用程序配置文件
#if __arm__  	//运行在终端平台arm上.
	#define CONF_FILE "/mnt/nor/conf/goahead.conf"
#else 		//运行在i368,host调试平台上.
	#define CONF_FILE "/home/zodiac1111/workspace/goahead/conf-host/goahead.conf"
#endif

//webs服务器错误文件相关
//日志路径在配置文件goahead.conf中定义
//如果在配置文件中查找errlog失败启用以下路径作为备用的错误处理文件路径.
#if __arm__
	#define BACKUP_ERR_FILE "/mnt/nor/goahead-backup.log"
#else
	#define BACKUP_ERR_FILE "/home/zodiac1111/workspace/goahead/goahead-backup.log"
#endif
#define MAX_ERR_LOG_LINE_LENTH 1024 ///<日志文件单行最大长度(字节)
#define MAX_ERR_LOG_FILE_SIZE (200*1024) ///<错误日志文件最大尺寸(字节)

//conf配置相关文件
//#define CFG_DIR "../para/"///<参数路径
#define FILE_SYSPARA "sysspara.cfg"///<系统参数文件,关系到下面的其他参数
#define CFG_MTR "mtrspara.cfg"///<表计参数配置文件
#define CFG_SIOPALN "sioplan.cfg"///<串口方案配置文件
#define CFG_NET "netpara.cfg"///<网络參數配置文件
#define CFG_MON_PARAM "monpara.cfg"///<监视参数数配置文件
#define CFG_FORWARD_TABLE "retranTable.cfg"///<转发表信息文件
#define CFG_SAVE_CYCLE "stspara.cfg"///<保存周期和项目配置文件
#define CFG_COLLECT_CYCLE "ctspara.cfg"///<采集周期和项目配置文件

//para参数相关文件.
//#define PROC_CFG_DIR "../conf/" ///<规约文本文件目录
#define PORC_FILE "protocol_config.txt"///<主站规约配置文件
#define MAX_PROCOTOL_NUM 128 ///<规约文件中最大规约条目开始时分配这么多
#define MON_PORT_NAME_FILE "monparam_name.conf"///<监视端口描述文件

//打印欢迎信息
#define PRINT_WELCOME {					\
	printf("\t\tWelcome to use "		\
	RED"Holley web services"_COLOR"\t\t\n");	\
	}
//打印版本信息
#define PRINT_VERSION { 					\
		printf("\tVersion:\t"GREEN"%d.%d.%d"_COLOR	\
			" ["RED"%s"_COLOR"]\n", 	 \
		MAJOR, MINOR, PATCHLEVEL,GIT_VERSION); 		\
	}
//打印编译构建的日期和时间，类似：Dec  3 2012 09:59:57
#define PRINT_BUILD_TIME {					\
		printf("\tBuild time:\t"GREEN"%s %s"_COLOR"\n",\
		 __DATE__, __TIME__);		\
	}

#if DEBUG_INFO_FORM
///打印form表单提交函数的入口信息:函数名,传递进来的队列
	#define	PRINT_FORM_INFO printf(WEBS_INF"%s:%s\n", __FUNCTION__,query)
#else
	#define	PRINT_FORM_INFO
#endif

#endif /* CONF_H_ */
