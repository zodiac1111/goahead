/*sysparam类的示例程序，使用了基本的读&写功能。参数的格式已经由主程序决定了。*/
#include "sysparam.h"
#include <stdio.h>
#include <string.h>
#define BUF_SIZE 256
//#include <db_cxx.h>


int main(void)
{
	class sysparam para("./para/");//实例化需要提供参数文件的路径
	int retval;
	char ret[BUF_SIZE];
	unsigned char mtrnumber=0;
	unsigned char netport_num=0;
	unsigned char mon_port_num=0;
	unsigned char ios_port_num=0;//串口方案数
	int i;
	int j;
	memset(ret,0,BUF_SIZE);
	mtrnumber=40;
	retval=para.write(meter_num,para.len(meter_num),&mtrnumber);
	retval=para.read(meter_num,&mtrnumber);
	printf("meter_num read retval=%d\n",mtrnumber);
#if 1 //遍历监视端口 信息,端口号,监听地址,端口地址等
	retval=para.read(monitor_port_num,&mon_port_num);
	printf("monitor_port_num %d retval=%d\n",mon_port_num,retval);
	unsigned char a[4][5]= {{1,3,3,3,1},{2,3,3,3,1},{3,3,3,3,3},{4,3,3,3,1}};
	retval=para.write(mon_listenPort,para.len(mon_listenPort), a[1],0);
	for(j=0; j<mon_port_num; j++) {
		retval=para.read(mon_listenPort,para.len(mon_listenPort),ret,j);
		printf("mon_listenPort =%d  retval=%d :",j,retval);
		for(i=0; i<para.len(mon_listenPort); i++) {
			printf("%d ",ret[i]);
		}
		printf("\n");
		retval=para.read(mon_portAddr,para.len(mon_portAddr),ret,j);
		printf("mon_portAddr =%d  retval=%d :",j,retval);
		for(i=0; i<para.len(mon_portAddr); i++) {
			printf("%d ",ret[i]);
		}
		printf("\n");
		printf("\n");
	}
#endif
#if 1 //遍历网卡
	retval=para.read(netports_num,para.len(netports_num),&netport_num,0);
	printf("netport %d retval=%d\n",netport_num,retval);
	for(j=0; j<netport_num; j++) {
		retval=para.read(ipaddr,para.len(ipaddr),ret,j);
		printf("ipaddr =%d  retval=%d :",j,retval);
		for(i=0; i<para.len(ipaddr); i++) {
			printf("%d ",ret[i]);
		}
		printf("\n");
		retval=para.read(netMask,para.len(netMask),ret,j);
		printf("netMask =%d  retval=%d :",j,retval);
		for(i=0; i<para.len(netMask); i++) {
			printf("%d ",ret[i]);
		}
		printf("\n");
		retval=para.read(Gateway,para.len(Gateway),ret,j);
		printf("Gateway =%d  retval=%d :",j,retval);
		for(i=0; i<para.len(Gateway); i++) {
			printf("%d ",ret[i]);
		}
		printf("\n\n");
	}
#endif
#if 1 //遍历线路编号
	retval=para.read(meter_num,para.len(meter_num),&mtrnumber);
	printf("meter_num read retval=%d\n",mtrnumber);
	for(j=0; j<mtrnumber; j++) {
		retval=para.read(meter_Linename,para.len(meter_Linename),ret,j);
		printf("meter_Linename %d  retval=%d ",j,retval);
		for(i=0; i<para.len(meter_Linename); i++) {
			printf("%d ",ret[i]);
		}
		printf("\n");
	}
#if 1 //修改线路编号 （++1）
#define TST_NO 0
	retval=para.read(meter_Linename,para.len(meter_Linename),ret,TST_NO);
	printf("meter_Linename %d  retval=%d ",TST_NO,retval);
	for(i=0; i<para.len(meter_Linename); i++) {
		printf("%d ",ret[i]);
	}
	printf("\n");
	ret[0]=ret[0]+1;
	retval=para.write(meter_Linename,para.len(meter_Linename),ret,TST_NO);
	if(retval<0) {
		printf("write err\n");
	}
	retval=para.read(meter_Linename,para.len(meter_Linename),ret,TST_NO);
	if(retval<0) {
		printf("read err\n");
	}
	printf("meter_Linename %d  retval=%d ",TST_NO,retval);
	for(i=0; i<para.len(meter_Linename); i++) {
		printf("%d ",ret[i]);
	}
	printf("\n");
#endif
#endif
#if 1 //遍历串口方案
	retval=para.read(sioports_num,para.len(sioports_num),&ios_port_num);
	printf("sioports_num %d retval=%d\n",ios_port_num,retval);
	for(j=0; j<5; j++) {
		retval=para.read(sioplanConfig,para.len(sioplanConfig),ret,j);
		printf("sioplanConfig retval=%d :",retval);
		for(i=0; i<5; i++) {
			printf(" %d",ret[i]);
		}
		printf("\n");
	}
#endif
}
