/**
 * @file master.h
 * 主站参数部分
 */
#ifndef _MASTER_PARAM_
#define _MASTER_PARAM_
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "conf.h"
#include "json.h"
#include "web_err.h"
#include "wpsend.h"
extern void form_master(webs_t wp, char_t *path, char_t *query);

#endif /* _MASTER_PARAM_ */
