#ifndef _JSON_H_
#define _JSON_H_
#include <stdint.h>
#include <stdarg.h>
//json对象,其实就是一串字符串:D
typedef  char* jsObj;
void jsonDemo(void);
jsObj jsonNew(void);
jsObj jsonNewArray(void);
jsObj jsonClear(jsObj* obj);
int jsonFree(jsObj *obj);
jsObj jsonAdd(jsObj*dobj,const char*name,const char*value);
jsObj jsonAddObj(jsObj*dobj,char*name,char*obj);
//
jsObj u8toa(jsObj str, const char*format, uint8_t value);
char* ftoa(char* str, const char*format, double value);
char* toStr(char *str,const char*format,...);
#endif
