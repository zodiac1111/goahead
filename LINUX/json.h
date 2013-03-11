#ifndef _JSON_H_
#define _JSON_H_
#include <stdint.h>
//json对象,其实就是一串字符串XD
typedef  char* jsObj;
void jsonDemo(void);
jsObj jsonNew(void);
jsObj jsonNewArray(void);
jsObj jsonClear(jsObj* obj);
int jsonFree(jsObj *obj);
jsObj u8toa(jsObj str, const char*format, uint8_t value);
char* ftoa(char* str, const char*format, double value);
jsObj jsonAdd(jsObj*dobj,const char*name,const char*value);
jsObj jsonAddObj(jsObj*dobj,char*name,char*obj);
#endif
