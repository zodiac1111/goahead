/**
 * @file wpsend.c
 *
 */
#include <string.h>
#include "conf.h"
#include "wpsend.h"
#include "web_err.h"
///向页面发送json对象,字符串过长需要分段发送
int wpsend(webs_t wp, jsObj oJson)
{
	int i;
	char buff[WP_MAX_LEN] = { 0 };
	int t = strlen(oJson)/(WP_MAX_LEN-1);     //商
	int mod = strlen(oJson)%(WP_MAX_LEN-1);     //余,最后部分大小可能不是正好,即不整
	//int len=0;//实际拷贝长度
	for (i = 0; i<=t; i++) {
		if (i==t) {	//最后一次循环/可能不整
			if(memcpy(buff, oJson+i*(WP_MAX_LEN-1), mod)==NULL){
				web_err_proc(EL);
			}
		} else {
			if(memcpy(buff, oJson+i*(WP_MAX_LEN-1), WP_MAX_LEN-1)==NULL){
				web_err_proc(EL);
			}
		}
		//printf(WEBS_DBG"%s\n",buff);
		websWrite(wp, T("%s"), buff);
		if(memset(buff, 0x0, WP_MAX_LEN)==NULL){
			web_err_proc(EL);
		}
	}
	return 0;
}
