/**
 * @file wpsend.h
 * 简单的将json数据拆分发送到页面,对websWrite的简单封装.
 */

#ifndef WPSEND_H_
#define WPSEND_H_
#include "uemf.h"
#include "wsIntrn.h"
#include "json.h"
extern int wpsend(webs_t wp,jsObj oJson);
#endif /* WPSEND_H_ */
