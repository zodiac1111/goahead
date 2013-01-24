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

#endif /* CONF_H_ */
