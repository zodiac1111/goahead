/** @file json.c
 * 简单的json对象生成库,仅用于生成json对象,不能分解json对象.
 * 主要用于服务端将数据构造成json格式,传递给客户端;
 * 客户端解析json对象成为js对象(这很简单).实现只传递数据,与样式(css)和行为(js)分离.
 */
#include "json.h"
#include <malloc.h>
#include <string.h>
#include "color.h"
#define DEBUG_JSON_CLEAR 0 //调试选项
#define JSON_DEMO GREEN"JSON DEMO"_COLOR">"
#define JSON_DBG YELLOW"JSON ERR"_COLOR">"
#define JSON_ERR RED"JSON ERR"_COLOR">"
/**
 * 创建一个空的json对象"{}",用完注意释放 free
 * @return
 */
jsObj jsonNew(void)
{
	jsObj obj=malloc(strlen("{}")+1);
	if(obj!=NULL){
		strcpy(obj,"{}");
	}
	return obj;
}
///穿件空数组"[]" ,注意释放 free
jsObj jsonNewArray(void)
{
	jsObj obj=malloc(strlen("[]")+1);
	if(obj!=NULL){
		strcpy(obj,"[]");
	}
	return obj;
}
///清空对象/数组内的内容.不是删除/释放对象!
jsObj jsonClear(jsObj* pobj)
{
	if(pobj==NULL){
		return NULL;
	}
	char s ;
	char e ;//区分是像对象还是向数组添加名/值对.
	s=(*pobj)[0];
	e=(*pobj)[strlen(*pobj)-1];
#if DEBUG_JSON_CLEAR ==1
	printf(JSON_DBG"%s s=%c e=%c \n",*pobj,s,e);
#endif
	if((s!='{' && s!='[') //只能是{ 或者 [开头
			||(e!=s+2)){ //同时要求括号匹配
		perror("jsonClear-not obj or array");
		return NULL;
	}
	*pobj=realloc(*pobj,2+1);
	if(pobj==NULL){
		perror("jsonClear-realloc");
		return NULL;
	}
	(*pobj)[0]=s;
	(*pobj)[1]=e;
	(*pobj)[2]='\0';
	return *pobj;
}
///删除对象/数组 (free掉堆内存)
int jsonFree(jsObj*pobj)
{
	if(*pobj!=NULL){
		free(*pobj);
		*pobj=NULL;
		return 0;
	}
	return -1;
}
///自定义的sprintf,与系统相比就多一个返回字符串.
/// @note str需要足够大的空间放置格式化后的字符串.
char* toStr(char *str,const char*format,...)
{
	va_list ap;
	va_start(ap,format);
	vsprintf( str,format, ap);
	va_end(ap);
	return str;
}
/**向str中按照format格式打印value,并且放回str的指针.
 * 参考系统的sprintf,多了返回指针的功能.
 * 多了限制:str必须已经分配足够的内存用于打印.
 * @note 已经废弃,正在从代码中剔除,使用toStr()函数代替
 * */
char* u8toa(char* str, const char*format, uint8_t value)
{
	sprintf(str, format, value);
	return str;
}
char* ftoa(char* str, const char*format, double value)
{
	sprintf(str, format, value);
	return str;
}
/**
 * 向json对象/数组最末尾中添加一个名/值对或对象.
 * 能过简单的自动区分是向(对象/数组)内添加(对象/数组/名值对).通过判断"{"
 * 原始对象可以是一个空对象: {}   => {"n2":"v2"}
 * 也可以是个有内容的对象 : {"n1":"v1"}   => {"n1":"v1","n2":"v2"}
 * (注意逗号分割多个名/值对)
 * @param dobj 对象或者数组.(名值对被加到这个对象)
 * @param name 名值对时的名字,或者数组时取NULL,数组不需要名字
 * @param value 名值对中的值,或者其他json对象/数组
 * @retval 返回添加到的对象的地址,方便链式调用.
 * @retval NULL 错误.
 */
jsObj jsonAdd(jsObj* dobj, const char*name,const char*value)
{
	if(dobj==NULL){
		perror("jsonAdd-null dobj");
		return NULL;
	}
	int isArray=((*dobj)[0]=='[')?1:0;//是否是向数组内添加.
	if(name==NULL && !isArray){ //添加数组元素不需要name.设置成空.
		perror("jsonAdd-null name");
		return NULL;
	}
	if(value==NULL){
		perror("jsonAdd- null value");
		return NULL;
	}
	int dlen=strlen(*dobj);
	int isObj=(value[0]=='{'||value[0]=='[')?1:0;
	int isFirst=((dlen==2)?1:0); //原始对象为空则不需要加逗号.
	char end ;//区分是像对象还是向数组添加名/值对.
	end=(*dobj)[0]+2;
	*dobj=realloc(*dobj,
			strlen(*dobj) //原来对象长度
			+(isFirst?0:1) //可能存在的逗号.如果空对象,则新增的名/值对不需要逗号
			+(isArray?0:strlen("\"\":"))//名/值对的可格式 "名":
			+(isObj?0:2) //如果不是对象则要加一对引号
			+(isArray?0:strlen(name)) //数组不需要name字段.
			+strlen(value)// 名/值对的长度
			+1);// \0
	if(*dobj==NULL){
		perror("jsonAddValue-realloc");
		return *dobj;
	}
	///@note 用三个bit分别表示,若gcc版本过低不能识别0b开头的二进制表达方式.
	/// 换成0x开头的十六进制表示就可以了.这里用二进制表示更形象 XD /
	///较低版本的gcc不支持二进制
#if defined (__GNUC__) && \
	defined (__GNUC_MINOR__) && \
	__GNUC__ >= 4 && __GNUC_MINOR__ >= 3
	int cs=isArray*0b100+isFirst*0b10+isObj*0b1;
#else
	int cs=isArray*0x04+isFirst*0x02+isObj*0x01;
#endif
	switch(cs){
	//添加到不是数组的对象中
	case 0x00/*0b000*/:	//不是首个元素(需要逗号),不是对象(需要引号).
		sprintf(*dobj+dlen-1,",\"%s\":\"%s\"%c",name,value,end);
		break;
	case 0x01/*0b001*/:	//不是首个元素(需要逗号),是对象(不需要引号)
		sprintf(*dobj+dlen-1,",\"%s\":%s%c",name,value,end);
		break;
	case 0x02/*0b010*/:	//是首个元素(不需要逗号),不是对象(需要引号).
		sprintf(*dobj+dlen-1,"\"%s\":\"%s\"%c",name,value,end);
		break;
	case 0x03/*0b011*/:	//是首个元素(不需要逗号),是对象(不需要引号).
		sprintf(*dobj+dlen-1,"\"%s\":%s%c",name,value,end);
		break;
	//添加到一个数组对象中,不需要name
	case 0x04/*0b100*/:	//不是首个元素(需要逗号),不是对象(需要引号).
		sprintf(*dobj+dlen-1,",\"%s\"%c",value,end);
		break;
	case 0x05/*0b101*/:	//不是首个元素(需要逗号),是对象(不需要引号)
		sprintf(*dobj+dlen-1,",%s%c",value,end);
		break;
	case 0x06/*0b110*/:	//是首个元素(不需要逗号),不是对象(需要引号).
		sprintf(*dobj+dlen-1,"\"%s\"%c",value,end);
		break;
	case 0x07/*0b111*/:	//是首个元素(不需要逗号),是对象(不需要引号).
		sprintf(*dobj+dlen-1,"%s%c",value,end);
		break;
	default:
		printf(JSON_ERR"in switch case cs=%d\n",cs);
		break;
	}
	return *dobj;
}
/**
 * json使用示例函数,演示创建,添加,销毁一个json对象的全部过程.
 * 以一张名片为例.
 * 名片:
 * 	姓名:Bob [基本的名/值对]
 * 	电话:89300405,133****8628 [数组的使用]
 * 	地址:浙江省 杭州市 A路810号 [以对象为值]
 * 用于开发参考,通过 conf.h 中 DEBUG_JSON_DEMO 宏可以开/关.
 * 在线解析:http://jsoneditoronline.org/
 */
void jsonDemo(void)
{
	char tmp[233];
	toStr(tmp,"%d%d",1,2);
	printf(JSON_DEMO"在线解析 Online JSON Editor: "
			GREEN"http://jsoneditoronline.org/"_COLOR"\n");
	printf(JSON_DEMO RED" /------------ Card ------------\\\n"_COLOR
		JSON_DEMO RED" | Name   :Bob                  |\n"_COLOR
		JSON_DEMO RED" | Tel    :89300405,133****8628 |\n"_COLOR
		JSON_DEMO RED" | Address:                     |\n"_COLOR
		JSON_DEMO RED" |   Province:Zhejiang          |\n"_COLOR
		JSON_DEMO RED" |   City    :Hangzhou          |\n"_COLOR
		JSON_DEMO RED" |   No.     :810 A Road        |\n"_COLOR
		JSON_DEMO RED" \\------------------------------/\n"_COLOR);
	jsObj oCard=jsonNew();
	printf(JSON_DEMO" Create a Empty Object. oCard "
		RED"%s"_COLOR"\n",oCard);

	oCard=jsonAdd(&oCard,"name","Bob");
	printf(JSON_DEMO" Add a field(name) to oCard "
		RED"%s"_COLOR"\n",oCard);

	jsObj aTel=jsonNewArray();
		printf(JSON_DEMO" Create a Empty Array. aTel "
			RED"%s"_COLOR"\n",aTel);

	jsonAdd(&aTel,NULL,"89300405");
	printf(JSON_DEMO" Add a Element to aTel "
		RED"%s"_COLOR"\n",aTel);

	jsonAdd(&aTel,NULL,"133****8628");
	printf(JSON_DEMO" Add a Element to aTel "
		RED"%s"_COLOR"\n",aTel);

	oCard=jsonAdd(&oCard,"Tel",aTel);
	printf(JSON_DEMO" Add a field(Tel) to oCard "
		RED"%s"_COLOR"\n",oCard);

	jsObj oAddr=jsonNew();
	printf(JSON_DEMO" Create a Empty Object. oAddress "
		RED"%s"_COLOR"\n",oAddr);

	jsonAdd(&oAddr,"Province","Zhejiang");
	printf(JSON_DEMO" Add a field(Province) to oAddress "
		RED"%s"_COLOR"\n",oAddr);

	jsonAdd(&oAddr,"City","Hangzhou");
	printf(JSON_DEMO" Add a field(City) to oAddress "
		RED"%s"_COLOR"\n",oAddr);

	jsonAdd(&oAddr,"No","810 A Road");
	printf(JSON_DEMO" Add a field(No) to oAddress "
		RED"%s"_COLOR"\n",oAddr);

	jsonAdd(&oCard,"Address",oAddr);
	printf(JSON_DEMO" Add a Object(oAddress) to oCard \n "
		RED"%s"_COLOR"\n",oCard);

	jsonFree(&aTel);
	printf(JSON_DEMO" Free array aTel "
		RED"%s"_COLOR"\n",aTel);

	jsonFree(&oAddr);
	printf(JSON_DEMO" Free oAddress "
		RED"%s"_COLOR"\n",oAddr);

	jsonFree(&oCard);
	printf(JSON_DEMO" Free oCard "
		RED"%s"_COLOR"\n",oCard);
	//清空和释放的区别.数组和对象都适用.
	printf(JSON_DEMO" ***** A Example of jsonClear *****\n");
	jsObj aTemp=jsonNewArray();
	printf(JSON_DEMO" Create a Empty Array. aTemp "
		RED"%s"_COLOR"\n",aTemp);
	jsonAdd(&aTemp,NULL,"a");
	printf(JSON_DEMO" Add a Element(a) to aTemp "
		RED"%s"_COLOR"\n",aTemp);
	jsonAdd(&aTemp,NULL,"b");
	printf(JSON_DEMO" Add a Element(b) to aTemp "
		RED"%s"_COLOR"\n",aTemp);
	jsonClear(&aTemp);
	printf(JSON_DEMO" Clear aTemp "
		RED"%s"_COLOR"\n",aTemp);
	jsonAdd(&aTemp,NULL,"c");
	printf(JSON_DEMO" Add a Element(c) to aTemp "
		RED"%s"_COLOR"\n",aTemp);
	jsonClear(&aTemp);
	printf(JSON_DEMO" Clear aTemp "
		RED"%s"_COLOR"\n",aTemp);
	jsonFree(&aTemp);
	printf(JSON_DEMO" Free aTemp "
		RED"%s"_COLOR"\n",aTemp);

}

