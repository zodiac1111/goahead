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
char* jsonNew(void)
{
	char* obj=malloc(strlen("{}")+1);
	if(obj!=NULL){
		strcpy(obj,"{}");
	}
	return obj;
}
///穿件空数组"[]" ,注意释放 free
char* jsonNewArray(void)
{
	char* obj=malloc(strlen("[]")+1);
	if(obj!=NULL){
		strcpy(obj,"[]");
	}
	return obj;
}
///清空对象/数组内的内容.不是删除!
char* jsonClear(char** obj)
{
	if(obj==NULL){
		return NULL;
	}
	char s ;
	char e ;//区分是像对象还是向数组添加名/值对.
	s=(*obj)[0];
	e=(*obj)[strlen(*obj)-1];
#if DEBUG_JSON_CLEAR ==1
	printf(JSON_DBG"%s s=%c e=%c \n",*obj,s,e);
#endif
	if((s!='{' && s!='[') //只能是{ 或者 [开头
			||(e!=s+2)){ //同时要求括号匹配
		perror("jsonClear-not obj or array");
		return NULL;
	}
	*obj=realloc(*obj,2+1);
	if(obj==NULL){
		perror("jsonClear-realloc");
		return NULL;
	}
	(*obj)[0]=s;
	(*obj)[1]=e;
	(*obj)[2]='\0';
	return *obj;
}
///删除对象/数组 (free掉堆内存)
int jsonFree(char**obj)
{
	if(*obj!=NULL){
		free(*obj);
		*obj=NULL;
		return 0;
	}
	return -1;
}

/**
 * 向json对象/数组最末尾中添加一个名/值对或对象.
 * 能过简单的自动区分是向(对象/数组)内添加(对象/数组/名值对).通过判断"{"
 * 原始对象可以是一个空对象: {}   => {"n2":"v2"}
 * 也可以是个有内容的对象 : {"n1":"v1"}   => {"n1":"v1","n2":"v2"}
 * (注意逗号分割多个名/值对)
 * @param dobj 对象或者数组.
 * @param name
 * @param value
 * @return
 */
char* jsonAdd(char**dobj, const char*name,const char*value)
{
	if(dobj==NULL){
		perror("jsonAdd-null dobj");
		return NULL;
	}
	int isArray=((*dobj)[0]=='[')?1:0;//是否是向数组内添加.
	if(name==NULL && !isArray){ //添加数组原书不需要name.设置成空.
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
			+(isObj?0:2) //图过不是对象则要加一对引号
			+(isArray?0:strlen(name)) //数组不需要name字段.
			+strlen(value)// 名/值对的长度
			+1);// \0
	if(*dobj==NULL){
		perror("jsonAddValue-realloc");
		return *dobj;
	}
	///@note 用三个bit分别表示,若gcc版本过低不能识别0b开头的二进制表达方式.
	/// 换成0x开头的十六进制表示就可以了.这里用二进制表示更形象 XD
	int cs=isArray*0b100+isFirst*0b10+isObj*0b1;
	switch(cs){
	//添加到不是数组的对象中
	case 0b000:	//不是首个元素(需要逗号),不是对象(需要引号).
		sprintf(*dobj+dlen-1,",\"%s\":\"%s\"%c",name,value,end);
		break;
	case 0b001:	//不是首个元素(需要逗号),是对象(不需要引号)
		sprintf(*dobj+dlen-1,",\"%s\":%s%c",name,value,end);
		break;
	case 0b010:	//是首个元素(不需要逗号),不是对象(需要引号).
		sprintf(*dobj+dlen-1,"\"%s\":\"%s\"%c",name,value,end);
		break;
	case 0b011:	//是首个元素(不需要逗号),是对象(不需要引号).
		sprintf(*dobj+dlen-1,"\"%s\":%s%c",name,value,end);
		break;
	//添加到一个数组对象中,不需要name
	case 0b100:	//不是首个元素(需要逗号),不是对象(需要引号).
		sprintf(*dobj+dlen-1,",\"%s\"%c",value,end);
		break;
	case 0b101:	//不是首个元素(需要逗号),是对象(不需要引号)
		sprintf(*dobj+dlen-1,",%s%c",value,end);
		break;
	case 0b110:	//是首个元素(不需要逗号),不是对象(需要引号).
		sprintf(*dobj+dlen-1,"\"%s\"%c",value,end);
		break;
	case 0b111:	//是首个元素(不需要逗号),是对象(不需要引号).
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
 * 	姓名:Bob
 * 	电话:85070110
 * 	地址:浙江省 杭州市 A路810号
 * 用于开发参考 :)
 * 在线解析:http://jsoneditoronline.org/
 */
void jsonDemo(void)
{
	printf(JSON_DEMO"在线解析 Online JSON Editor: "
			GREEN"http://jsoneditoronline.org/"_COLOR"\n");
	printf(JSON_DEMO"  ********  Example  ********\n"
		JSON_DEMO"  * Name   :Bob             *\n"
		JSON_DEMO"  * Tel    :85070110        *\n"
		JSON_DEMO"  * Address:                *\n"
		JSON_DEMO"  *   Province:Zhejiang     *\n"
		JSON_DEMO"  *   City    :Hanzhou      *\n"
		JSON_DEMO"  *   No.     :810 A Road   *\n"
		JSON_DEMO"  ***************************\n");
	char* oCard=jsonNew();
	printf(JSON_DEMO"1. Create a void Object. oCard "
		RED"%s"_COLOR"\n",oCard);

	oCard=jsonAdd(&oCard,"name","Bob");
	printf(JSON_DEMO"2. Add a field(name) to oCard "
		RED"%s"_COLOR"\n",oCard);

	oCard=jsonAdd(&oCard,"Tel","85070110");
	printf(JSON_DEMO"3. Add a field(Tel) to oCard "
		RED"%s"_COLOR"\n",oCard);

	char* oAddr=jsonNew();
	printf(JSON_DEMO"4. Create a void Object. oAddress "
		RED"%s"_COLOR"\n",oAddr);

	jsonAdd(&oAddr,"Province","Zhejiang");
	printf(JSON_DEMO"5. Add a field(Province) to oAddress "
		RED"%s"_COLOR"\n",oAddr);

	jsonAdd(&oAddr,"City","Hangzhou");
	printf(JSON_DEMO"6. Add a field(City) to oAddress "
		RED"%s"_COLOR"\n",oAddr);

	jsonAdd(&oAddr,"No","810 A Road");
	printf(JSON_DEMO"7. Add a field(No) to oAddress "
		RED"%s"_COLOR"\n",oAddr);

	jsonAdd(&oCard,"Address",oAddr);
	printf(JSON_DEMO"8. Add a Object(oAddress) to oCard "
		RED"%s"_COLOR"\n",oCard);

	jsonFree(&oAddr);
	printf(JSON_DEMO"9. Free oAddress "
		RED"%s"_COLOR"\n",oAddr);

	jsonFree(&oCard);
	printf(JSON_DEMO"10.Free oCard "
		RED"%s"_COLOR"\n",oCard);
}
