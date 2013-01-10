#ifndef _SYSPARAM_H_
#define _SYSPARAM_H_
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#define SHOW_WHERE {					\
	printf("\n\n[File:%s Line:%d] Fun:%s\n\n",	\
	__FILE__, __LINE__, __FUNCTION__);	\
	}
enum eParam {//参数,使用这些还需要了解其结构
    //详细参见主程序对参数数据结构的定义。
    // *** in sysspara.cfg ***
    meter_num,//表数量
    sioports_num,//串口数量
    netports_num,//网口数量
    pulse_num,//脉冲数
    monitor_port_num,//监视端口数量
    control_port_num,//控制端口数量
    sioplan_num,//串口方案数量
    // *** in file sioplan.cfg ***
    sioplanConfig,//各个串口配置方案 数据位 校验位 停止位 波特率等
    // *** in file mtrspara.cfg ***
    meter_Linename,//表计线路名称
    meter_addr,//表地址
    meter_pwd,//表密码
    meter_commPort,//表端口
    meter_commPlan,//表串口方案
    MeterType,
    MeterProt,
    DLDot,
    XLDot,
    UDot,
    IDot,
    PDot,
    QDot,
    P3L4,
    Ue,
    Ie,
    Validflag,
    // *** in file netpara.cfg ***
    netport,//网络端口编号 类似数据库主键
    ipaddr,//
    netMask,//子网掩码
    Gateway,//网关地址
    // *** in file monpara.cfg ***
    mon_commPort,//like 主键
    mon_listenPort,//监听端口号
    mon_commPlan,
    mom_protType,
    mon_portAddr,//监视端口地址
    mon_check_time_Valiflag,
    mon_retransmit_flag,
    mon_retransmit_mtrnum,
    // *** in file ctlpara.cfg ***
    ctl_commPort,
    //
    __Retention__
};

class sysparam
{
public:
	sysparam(const std::string path="./para");
	sysparam(void);
	int len(eParam p)const;
	int read(eParam p,int len,void *const retval, int number)const;
	int read(eParam p,void *const retval)const;
	int write(eParam p,int len,void *const retval,int number=0)const;
private:
	std::string path;
private:
	bool isindexparam(eParam p)const;
	FILE* openfile(eParam p, const char *mod, int number=0)const;
	int modify_file( eParam p, const void *const retval)const;
};

#endif
