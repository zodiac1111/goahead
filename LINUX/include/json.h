/**
 * @file json.h  自己写的json对象构造库,头文件.
 */
#ifndef _JSON_H_
#define _JSON_H_
#include <stdint.h>
#include <stdarg.h>
//json对象,其实就是一串字符串:D
typedef  char* jsObj;
void jsonDemo(void);
jsObj jsonNew(void);
jsObj jsonNewArray(void);
jsObj jsonClean(jsObj* pobj);
int jsonFree(jsObj *pobj);
jsObj jsonAdd(jsObj*dobj,const char*name,const char*value);
char* toStr(char *str,const char*format,...);
#endif
