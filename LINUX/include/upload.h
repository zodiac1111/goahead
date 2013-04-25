/**
 * @file upload.h
 * 客户端上传文件功能模块
 */

#ifndef UPLOAD_H_
#define UPLOAD_H_
#include <stdio.h>
#include "../uemf.h"
#include "../wsIntrn.h"
#include "conf.h"
#include "json.h"
#include "wpsend.h"
#include "web_err.h"
extern void form_upload_file(webs_t wp, char_t *path, char_t *query);

#endif /* UPLOAD_H_ */
