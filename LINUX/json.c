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
 * 用于开发参考 :)
 */
void jsonDemo(void)
{
	printf(JSON_DEMO"Online JSON Editor: http://jsoneditoronline.org/");

	char* o1=jsonNew();
	printf(JSON_DEMO"Create a void Object.O1 "
		RED"%s"_COLOR"\n",o1);

	o1=jsonAddValue(&o1,"name1","value1");
	printf(JSON_DEMO"Add a name/value to O1 "
		RED"%s"_COLOR"\n",o1);

	o1=jsonAddValue(&o1,"name2","value2");
	printf(JSON_DEMO"Add second name/value to O1 "
		RED"%s"_COLOR"\n",o1);

	jsonFree(&o1);
	printf(JSON_DEMO"Free O1 "
		RED"%s"_COLOR"\n",o1);

	char* o2=jsonNew();
	printf(JSON_DEMO"Create a void Object.O2 "
		RED"%s"_COLOR"\n",o2);

	char* o3=jsonNew();
	printf(JSON_DEMO"Create a void Object.O3 "
		RED"%s"_COLOR"\n",o2);

	jsonAddValue(&o2,"name3","value3");
	printf(JSON_DEMO"Add a name/value to O2 "
		RED"%s"_COLOR"\n",o2);

	jsonAddObj(&o3,"name4",o2);
	printf(JSON_DEMO"Add a name/value(o2) to O3 "
		RED"%s"_COLOR"\n",o3);

	jsonAddValue(&o3,"name5","value5");
	printf(JSON_DEMO"Add a name/value to O2 "
		RED"%s"_COLOR"\n",o3);

	jsonFree(&o2);
	printf(JSON_DEMO"Free O2 "
		RED"%s"_COLOR"\n",o2);

	jsonFree(&o3);
	printf(JSON_DEMO"Free O3 "
		RED"%s"_COLOR"\n",o3);

}
