/*修改系统参数接口类。各类参数定义，结构根据Initconfig.cpp文件得到。
  格式与系统主程序/规约参数修改 有强烈耦合关系。
  实际为操作文件。
*/
#include "sysparam.h"
#include <stdio.h>
#include <iostream>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#define PERFIX "[class sysParam]"

/* 初始化配置文件目录 如 ./ 或者 /path/to/para/ 等
*/
sysparam::sysparam(const std::string path)
{
	this->path=path;
}
sysparam::sysparam(void)
{
	this->path="./";
}

/* 从文件中读取参数。
  in:	 p 读取参数类型 枚举型
  in:	len 参数的长度(字节),比如表计数目占1字节
  out:	retval[len] 参数的值通过该数组返回
  in:	number(可选) 同类参数组的序号,比如串口1,2
  返回:	 0:读取正确
	 非0:读取失败	*/
int sysparam::read(eParam p,int len, void* const retval,int number) const
{
	FILE* fp;
	int ret;
	fp=openfile(p,"r",number);
	if(fp==NULL) {
		printf("open file err \n");
		perror("openfile");
		return -1;
	}
	ret=fread(retval,1,len,fp);
	if(ret!=len) {
		printf("fread err line %d  ret=%d len=%d \n",__LINE__,ret,len);
		fclose(fp);
		return -2;
	}
	fclose(fp);
	return 0;
}
int sysparam::read(eParam p,void *const retval)const
{
	FILE* fp;
	int len=this->len(p);
	int ret;
	if(isindexparam(p)) {
		printf("To read this paramter need an index \n");
		return -3;
	}
	fp=openfile(p,"r",0);
	if(fp==NULL) {
		printf("open file err \n");
		perror("openfile");
		return -1;
	}
	ret=fread(retval,1,len,fp);
	if(ret!=len) {
		printf("fread err line %d  ret=%d len=%d \n",__LINE__,ret,len);
		fclose(fp);
		return -2;
	}
	fclose(fp);
	return len;
}
//adjust a paramter is need a index to read/write
bool sysparam::isindexparam(eParam p)const
{
	if(  p==meter_num ||
	     p==sioports_num ||
	     p== netports_num ||
	     p== pulse_num ||
	     p== monitor_port_num ||
	     p==control_port_num ||
	     p== sioplan_num ) {
		return false;
	} else {
		return true;
	}
}

/* 将参数写入到参数文件中。
  in:	 p 读取参数类型 枚举型
  in:	len 参数的长度(字节),比如表计数目占1字节
  in:	retval[len] 参数的值通过该数组提供
  返回:	 0:读取正确
	 非0:读取失败	*/
int sysparam::write(eParam p,int len, void* const retval,int number ) const
{
	FILE* fp;
	int ret;
	fp=openfile(p,"r+",number);
	if(fp==NULL) {
		//printf("open file to write err line:%d \n",__LINE__);
		perror("openfile");
		return -1;
	}
	ret=fwrite(retval,1,len,fp);
	if(ret!=len) {
		printf("fwrite err %d \n",__LINE__);
		fclose(fp);
		return -2;
	}
	fclose(fp);
	//修改相关文件,比如修改了端口个数,端口配置文件也应该变化.
	modify_file( p,	 retval);
	return 0;
}

/* 修改相关文件,比如修改了端口号的个数,那么端口配置文件也应该相应修改(实现数据库关系)
  修改[前项]必须同时更新(修改)后项:
	表计个数->表配置(线路,表号等等)
	网口个数->网口配置
	监视端口个数-> 监视参数
	控制端口个数-> 控制参数
	端口数量->端口配置文件
*/
int sysparam::modify_file( eParam p,const  void * const retval)const
{
	std::string s=this->path;
	FILE *fp;
	int fd;
	int ret;
	unsigned char n=*(unsigned char *)retval;
	switch(p) {
	case meter_num://
		s.append("mtrspara.cfg");
		fp=fopen(s.c_str(),"r+");
		fd = fileno(fp);
		ret=ftruncate(fd,43*n);
		break;
		//	case sioports_num:
		//		printf("[串口数量]参数已经被[串口方案数量]参数代替\n");
		//		break;
	case netports_num:
		s.append("netpara.cfg");
		fp=fopen(s.c_str(),"r+");
		fd = fileno(fp);
		ret=ftruncate(fd,42*n);
		break;
		//	case pulse_num:
		//		printf("pulse conf file is coding...\n");
		//		break;
	case monitor_port_num:
		s.append("monpara.cfg");
		fp=fopen(s.c_str(),"r+");
		fd = fileno(fp);
		ret=ftruncate(fd,15*n);
		break;
	case control_port_num:
		s.append("ctlpara.cfg");
		fp=fopen(s.c_str(),"r+");
		fd = fileno(fp);
		ret=ftruncate(fd,1*n);
		break;
	case sioplan_num:
		s.append("sioplan.cfg");
		fp=fopen(s.c_str(),"r+");
		fd = fileno(fp);
		ret=ftruncate(fd,5*n);
		break;
	default:
		return 0;
	}
	if(ret==-1) {
		perror("ftruncate:");
		return -1;
	}
	fclose(fp);
	return 0;
}
/*按照特定的参数(枚举型)打开指定的参数文件,并将游标指向指定位置,返回文件指针
*/
FILE* sysparam::openfile(eParam p,const char * mod,int number)const
{
	FILE *fp;
	std::string s=this->path;
	switch(p) {
	case meter_num://
		s.append("sysspara.cfg");
		fp=fopen(s.c_str(),mod);
		fseek(fp,0,SEEK_SET);
		break;
	case sioports_num:
		s.append("sysspara.cfg");
		fp=fopen(s.c_str(),mod);
		fseek(fp,1,SEEK_SET);
		break;
	case netports_num:
		s.append("sysspara.cfg");
		fp=fopen(s.c_str(),mod);
		fseek(fp,2,SEEK_SET);
		break;
	case pulse_num:
		s.append("sysspara.cfg");
		fp=fopen(s.c_str(),mod);
		fseek(fp,3,SEEK_SET);
		break;
	case monitor_port_num:
		s.append("sysspara.cfg");
		fp=fopen(s.c_str(),mod);
		fseek(fp,4,SEEK_SET);
		break;
	case control_port_num:
		s.append("sysspara.cfg");
		fp=fopen(s.c_str(),mod);
		fseek(fp,5,SEEK_SET);
		break;
	case sioplan_num:
		s.append("sysspara.cfg");
		fp=fopen(s.c_str(),mod);
		fseek(fp,6,SEEK_SET);
		break;
	case sioplanConfig://
		s.append("sioplan.cfg");
		fp=fopen(s.c_str(),mod);
		fseek(fp,number*5+0,SEEK_SET);
		break;
	case meter_Linename://
		s.append("mtrspara.cfg");
		fp=fopen(s.c_str(),mod);
		fseek(fp,number*43+0,SEEK_SET);
		break;
	case meter_addr:
		s.append("mtrspara.cfg");
		fp=fopen(s.c_str(),mod);
		fseek(fp,number*43+6,SEEK_SET);
		break;
	case meter_pwd:
		s.append("mtrspara.cfg");
		fp=fopen(s.c_str(),mod);
		fseek(fp,number*43+18,SEEK_SET);
		break;
	case meter_commPort:
		s.append("mtrspara.cfg");
		fp=fopen(s.c_str(),mod);
		fseek(fp,number*43+26,SEEK_SET);
		break;
	case meter_commPlan:
		s.append("mtrspara.cfg");
		fp=fopen(s.c_str(),mod);
		fseek(fp,number*43+27,SEEK_SET);
		break;
	case MeterType:
		s.append("mtrspara.cfg");
		fp=fopen(s.c_str(),mod);
		fseek(fp,number*43+28,SEEK_SET);
		break;
	case MeterProt:
		s.append("mtrspara.cfg");
		fp=fopen(s.c_str(),mod);
		fseek(fp,number*43+29,SEEK_SET);
		break;
	case DLDot:
		s.append("mtrspara.cfg");
		fp=fopen(s.c_str(),mod);
		fseek(fp,number*43+30,SEEK_SET);
		break;
	case XLDot:
		s.append("mtrspara.cfg");
		fp=fopen(s.c_str(),mod);
		fseek(fp,number*43+31,SEEK_SET);
		break;
	case UDot:
		s.append("mtrspara.cfg");
		fp=fopen(s.c_str(),mod);
		fseek(fp,number*43+32,SEEK_SET);
		break;
	case IDot:
		s.append("mtrspara.cfg");
		fp=fopen(s.c_str(),mod);
		fseek(fp,number*43+33,SEEK_SET);
		break;
	case PDot:
		s.append("mtrspara.cfg");
		fp=fopen(s.c_str(),mod);
		fseek(fp,number*43+34,SEEK_SET);
		break;
	case QDot:
		s.append("mtrspara.cfg");
		fp=fopen(s.c_str(),mod);
		fseek(fp,number*43+35,SEEK_SET);
		break;
	case P3L4:
		s.append("mtrspara.cfg");
		fp=fopen(s.c_str(),mod);
		fseek(fp,number*43+36,SEEK_SET);
		break;
	case Ue:
		s.append("mtrspara.cfg");
		fp=fopen(s.c_str(),mod);
		fseek(fp,number*43+37,SEEK_SET);
		break;
	case Ie:
		s.append("mtrspara.cfg");
		fp=fopen(s.c_str(),mod);
		fseek(fp,number*43+40,SEEK_SET);
		break;
	case Validflag:
		s.append("mtrspara.cfg");
		fp=fopen(s.c_str(),mod);
		fseek(fp,number*43+42,SEEK_SET);
		break;
	case netport:
		s.append("netpara.cfg");
		fp=fopen(s.c_str(),mod);
		fseek(fp,number*42+0,SEEK_SET);
		break;
	case ipaddr:
		s.append("netpara.cfg");
		fp=fopen(s.c_str(),mod);
		fseek(fp,number*42+1,SEEK_SET);
		break;
	case netMask:
		s.append("netpara.cfg");
		fp=fopen(s.c_str(),mod);
		fseek(fp,number*42+13,SEEK_SET);
		break;
	case Gateway:
		s.append("netpara.cfg");
		fp=fopen(s.c_str(),mod);
		fseek(fp,number*42+25,SEEK_SET);
		break;
	case mon_commPort:
		s.append("monpara.cfg");
		fp=fopen(s.c_str(),mod);
		fseek(fp,number*15+0,SEEK_SET);
		break;
	case mon_listenPort:
		s.append("monpara.cfg");
		fp=fopen(s.c_str(),mod);
		fseek(fp,number*15+1,SEEK_SET);
		break;
	case mon_commPlan:
		s.append("monpara.cfg");
		fp=fopen(s.c_str(),mod);
		fseek(fp,number*15+6,SEEK_SET);
		break;
	case mom_protType:
		s.append("monpara.cfg");
		fp=fopen(s.c_str(),mod);
		fseek(fp,number*15+7,SEEK_SET);
		break;
	case mon_portAddr:
		s.append("monpara.cfg");
		fp=fopen(s.c_str(),mod);
		fseek(fp,number*15+8,SEEK_SET);
		break;
	case mon_check_time_Valiflag:
		s.append("monpara.cfg");
		fp=fopen(s.c_str(),mod);
		fseek(fp,number*15+12,SEEK_SET);
		break;
	case mon_retransmit_flag:
		s.append("monpara.cfg");
		fp=fopen(s.c_str(),mod);
		fseek(fp,number*15+13,SEEK_SET);
		break;
	case mon_retransmit_mtrnum:
		s.append("monpara.cfg");
		fp=fopen(s.c_str(),mod);
		fseek(fp,number*15+14,SEEK_SET);
		break;
	case ctl_commPort:
		s.append("ctlpara.cfg");
		fp=fopen(s.c_str(),mod);
		fseek(fp,number*1+0,SEEK_SET);
		break;
	default:
		printf(PERFIX" undefine param number: %d line:%d\n",p,__LINE__);
		return NULL;
	}
	return fp;
}

/*得到指定参数的长度(字节),若参数不存在 返回-1*/
int sysparam::len(eParam p) const
{
	int ret=0;
	switch(p) {
	case meter_num:
		ret= 1;
		break;
	case sioports_num:
		ret= 1;
		break;
	case netports_num:
		ret= 1;
		break;
	case pulse_num:
		ret= 1;
		break;
	case monitor_port_num:
		ret= 1;
		break;
	case control_port_num:
		ret= 1;
		break;
	case sioplan_num://
		ret= 1;
		break;
	case sioplanConfig://
		ret= 5;
		break;
	case meter_Linename://
		ret= 6;
		break;
	case meter_addr:
		ret= 12;
		break;
	case meter_pwd:
		ret= 8;
		break;
	case meter_commPort:
		ret= 1;
		break;
	case meter_commPlan:
		ret= 1;
		break;
	case MeterType:
		ret= 1;
		break;
	case MeterProt:
		ret= 1;
		break;
	case DLDot:
		ret= 1;
		break;
	case XLDot:
		ret= 1;
		break;
	case UDot:
		ret= 1;
		break;
	case IDot:
		ret= 1;
		break;
	case PDot:
		ret= 1;
		break;
	case QDot:
		ret= 1;
		break;
	case P3L4:
		ret= 1;
		break;
	case Ue:
		ret= 3;
		break;
	case Ie:
		ret= 2;
		break;
	case Validflag:
		ret= 1;
		break;
	case netport:
		ret=1;
		break;
	case ipaddr:
		ret=12;
		break;
	case netMask:
		ret=12;
		break;
	case Gateway:
		ret=12;
		break;
	case mon_commPort:
		ret=1;
		break;
	case mon_listenPort:
		ret=5;
		break;
	case mon_commPlan:
		ret=1;
		break;
	case mom_protType:
		ret=1;
		break;
	case mon_portAddr:
		ret=4;
		break;
	case mon_check_time_Valiflag:
		ret=1;
		break;
	case mon_retransmit_flag:
		ret=1;
		break;
	case mon_retransmit_mtrnum:
		ret=1;
		break;
	case ctl_commPort:
		ret=1;
		break;
	default:
		printf(PERFIX" undefine param number: %d line:%d\n",p,__LINE__);
		ret=-1;
	}
	return ret;
}
