/**
 * @file rtit.h real time 电量,实时电量
 */
#ifndef _RTIT_H_
#define _RTIT_H_
#include <time.h>
#include "../uemf.h"
#include "../wsIntrn.h"
#include "struct.h"
#include "externCShm.h"
#include "define.h"
extern void form_realtime_tou(webs_t wp, char_t *path, char_t *query);
#endif
