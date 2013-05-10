/**
 * @file commModule.h
 * 通讯模块部分
 */
#ifndef COMMMODULE_H_
#define COMMMODULE_H_
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "conf.h"
#include "json.h"
#include "web_err.h"
#include "wpsend.h"
extern void form_commModule(webs_t wp, char_t *path, char_t *query);

#endif /* COMMMODULE_H_ */
