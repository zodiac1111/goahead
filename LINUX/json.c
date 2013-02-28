#include "json.h"
#include <malloc.h>
#include <string.h>
#define RED "\e[31m"
#define GREEN "\e[32m"
#define _COLOR "\e[0m"
#define JSON_DEMO RED"JSON DEMO"_COLOR">"
char* jsonNew(void)
{
	char* obj=malloc(strlen("{}")+1);
	if(obj!=NULL){
		strcpy(obj,"{}");
	}
	return obj;
}
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
 * 像json对象最末尾中添加一个名/值对.
 * 原始对象可以是一个空对象: {}   => {"n2":"v2"}
 * 也可以是个有内容的对象 : {"n1":"v1"}   => {"n1":"v1","n2":"v2"}
 * (注意逗号分割多个名/值对)
 * @param dobj
 * @param name
 * @param value
 * @return
 */
char* jsonAddValue(char**dobj,char*name,char*value)
{
	int dlen=strlen(*dobj);
	int i=(dlen==2)?0:1; //原始对象为空则不需要加逗号.
	*dobj=realloc(*dobj,
			strlen(*dobj) //原来对象长度
			+i //可能存在的逗号.如果空对象,则新增的名/值对不需要逗号
			+strlen("\"\":\"\"")//名/值对的可格式 "名":"值"
			+strlen(name)+strlen(value)// 名/值对的长度
			+1);// \0
	if(*dobj==NULL)
		return *dobj;
	if(i==0){
	sprintf(*dobj+dlen-1,"\"%s\":\"%s\"}",name,value);
	}else{
		sprintf(*dobj+dlen-1,",\"%s\":\"%s\"}",name,value);
	}
	return *dobj;
}
char* jsonAddObj(char**dobj,char*name,char*obj)
{
	int dlen=strlen(*dobj);
	int i=(dlen==2)?0:1; //原始对象为空则不需要加逗号.
	*dobj=realloc(*dobj,
			strlen(*dobj) //原来对象长度
			+i //可能存在的逗号.如果空对象,则新增的名/值对不需要逗号
			+strlen("\"\":")//名/值对的可格式 "名":"值"
			+strlen(name)+strlen(obj)// 名/值对的长度
			+1);// \0
	if(*dobj==NULL)
		return *dobj;
	if(i==0){
	sprintf(*dobj+dlen-1,"\"%s\":%s}",name,obj);
	}else{
		sprintf(*dobj+dlen-1,",\"%s\":%s}",name,obj);
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

	char* oCard=jsonNew();
	printf(JSON_DEMO"Create a void Object. oCard "
		RED"%s"_COLOR"\n",oCard);

	oCard=jsonAddValue(&oCard,"name","Bob");
	printf(JSON_DEMO"Add a field(name) to oCard "
		RED"%s"_COLOR"\n",oCard);

	oCard=jsonAddValue(&oCard,"Tel","85070110");
	printf(JSON_DEMO"Add a field(Tel) to oCard "
		RED"%s"_COLOR"\n",oCard);

	char* oAddr=jsonNew();
	printf(JSON_DEMO"Create a void Object. oAddress "
		RED"%s"_COLOR"\n",oAddr);

	jsonAddValue(&oAddr,"Province","Zhejiang");
	printf(JSON_DEMO"Add a field(Province) to oAddress "
		RED"%s"_COLOR"\n",oAddr);

	jsonAddValue(&oAddr,"City","Hangzhou");
	printf(JSON_DEMO"Add a field(City) to oAddress "
		RED"%s"_COLOR"\n",oAddr);

	jsonAddValue(&oAddr,"No","810 A Road");
	printf(JSON_DEMO"Add a field(No) to oAddress "
		RED"%s"_COLOR"\n",oAddr);

	jsonAddObj(&oCard,"Address",oAddr);
	printf(JSON_DEMO"Add a Object(oAddress) to oCard "
		RED"%s"_COLOR"\n",oCard);

	jsonFree(&oAddr);
	printf(JSON_DEMO"Free oAddress "
		RED"%s"_COLOR"\n",oAddr);

	jsonFree(&oCard);
	printf(JSON_DEMO"Free oCard "
		RED"%s"_COLOR"\n",oCard);
}
