/**
 * @file conf.h
 * 配置服务器一些硬编码的参数
 *  Created on: 2013-1-24
 *      Author: lee
 */
#ifndef CONF_H_
#define CONF_H_

///打印表单提交函数的信息
#define DEBUG_INFO_FORM 1
///webs服务器应用程序配置文件
#if __arm__  	//运行在终端平台arm上.
#define CONF_FILE "/mnt/nor/conf/goahead.conf"
#else 		//运行在i368,host调试平台上.
#define CONF_FILE "/home/lee/workspace/goahead/conf/goahead.conf"
#endif
#define ERR_LOG "./err.log" ///错误日志文件
#define MAX_ERR_LOG_LINE_LENTH 1024 ///<日志文件单行最大长度(字节)
#define MAX_ERR_LOG_FILE_SIZE (100*1024) ///<错误日志文件最大尺寸(字节)
#define PREFIX "[webs]" ///<应用程序信息前缀
#define PREFIX_INF PREFIX"info:" ///<一般信息
#define PREFIX_WAR PREFIX"warning:" ///<警告信息
#define PREFIX_ERR PREFIX"error:" ///<错误信息
#endif /* CONF_H_ */
