/**
 * @file autoUpdate.c
 *
 */
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "web_err.h"
#include "autoUpdate.h"
#include "conf.h"
/**
 * 服务器自动升级
 * 判断 webs.update文件是否存在
 * 移动 webs.update 到 webs,execl新的程序
 * 完成升级
 */
int autoUpdate(void)
{
	int ret = access(UPDATE_FILE_NAME, 0);
	if (ret==0) {
		(void)printf(WEBS_INF" Being upgraded...\n");
		if(system("mv " UPDATE_FILE_NAME " " PROG_NAME)){
			web_err_proc(EL);
		}
		if(system("chmod +x "PROG_NAME)<0){
			web_err_proc(EL);
		}
		printf(WEBS_INF" **********************************\n");
		printf(WEBS_INF" *                                *\n");
		printf(WEBS_INF" *            Update OK.          *\n");
		printf(WEBS_INF" *                                *\n");
		printf(WEBS_INF" **********************************\n");
		//execl直接覆盖本进程/变成其他进程(这里是自己)
		//不知道有什么潜在危险,使用很实用 :)
		if(execl(PROG_NAME, PROG_NAME, NULL )<0){
			web_err_proc(EL);
		}
	} else {
		return 1;
	}
	return 1;
}
