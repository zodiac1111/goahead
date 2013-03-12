/**
 * @file param.h
 * 参数设置相关
 *  Created on: 2012-12-19
 *      Author: lee
 */
#ifndef PARAM_H_
#define PARAM_H_
#include "type.h"
#pragma pack(1)
#include "conf.h"
#define MAX_MON_PORT_NUM 64U ///<监视参数端口描述字符串最大数量.如COM1 ETH1 等
#define IPV4_LEN 12U ///<ipv4占用4个字节,表示成12个字符
#define LINE_LEN 6U //线路号长度
#define ADDR_LEN 12U
#define PWD_LEN 8U
///<储存周期项目,目前(6项):
///1. 总电量 2. 分时电量	3.相线无功电能 4.最大需量 5.实时遥测量 6.失压断相
#define SAVE_CYCLE_ITEM 6U
extern const char *PW[2];
extern const char *PROTOROL[2];
extern const char *UART_P[3];
extern const char *UART_DAT_BIT[3];
extern const char *UART_STOP[2];
extern const char *UART_BAUD[7];
extern const char *UART_COMM_TYPE[2];
extern const char* SAVE_CYCLE[12];
/**
 *  打印输出当前位置的文件,函数,代码行数
 */
#define PRINT_HERE {						\
	printf(WEBS_ERR"[File:%s Line:%d] Fun:%s .\n",	\
	__FILE__, __LINE__, __FUNCTION__);			\
	}
#define PRINT_RET(ret) {						\
	printf(WEBS_ERR"[File:%s Line:%d] Fun:%s ret %d .\n",	\
	__FILE__, __LINE__, __FUNCTION__,(ret));		 	\
	}
/// sysspara.cfg 文件/内存储存形式.
typedef struct
{
	u8 meter_num;
	u8 sioports_num;
	u8 netports_num;
	u8 pulse_num;
	u8 monitor_ports;
	u8 control_ports;
	u8 sioplan_num;
} stSysParam;
/**
 * 程序中操作的表参数结构体,具体size大小不重要,只要能容纳就行
 * 存进文件时转化成为_File类型.从文件读出时转化为本结构体类型
 */
typedef struct
{
	u8 mtrno; ///<表号,这是文件中没有的,文件中顺序存放,结构体为了方便使用,增加表号
	u8 line[LINE_LEN];
	u8 addr[ADDR_LEN]; ///<12位的十进制数,需要8个字节长度的long long 整形
	u8 pwd[PWD_LEN]; ///<8位十进制数....
	u8 port;///<串口
	u8 portplan;///<串口方案号 0开始
	u8 fact;///<厂商表号
	u8 protocol;///<规约号
	u8 it_dot; ///<电量小数位数
	u8 xl_dot; ///<需量小数位数
	u8 v_dot; ///<电压
	u8 i_dot;///<电流小数位数
	u8 p_dot;///<有功功率小数位数
	u8 q_dot;///<无功功率小数位数
	u8 p3w4; ///<电表类型:三相三线-0 三相四线制-1;
	u32 ue; ///<电压 百十个位 (000~999) * 10V
	u32 ie; ///< 千,百位 00~99 *100mA
	//TODO 有效标志位
	u8 iv;    ///有效标志位:位7= 1-有效 0 无效,位[0,6]保留为0
} stMtr;
/// mtrspara.cfg 文件中储存的表参数格式结构体,
typedef struct
{
	u8 line[LINE_LEN]; ///<0-5 共6 线路名 字节流
	u8 addr[ADDR_LEN]; ///<表地址,一字节一个十进制数,每位范围[0,9]超过一定是错误的
	u8 pwd[PWD_LEN]; ///<密码
	u8 port; ///<串口号 com0 com1 等
	u8 portplan; ///<串口方案号,第几种串口方案,和串口方案相关联起来
	u8 type; ///<表计类型?什么类型?0x00
	u8 protocol; ///<表计规约
	u8 it_dot; ///<电量小数位数
	u8 xl_dot; ///<需量小数位数
	u8 v_dot; ///<电压
	u8 i_dot;///<电压小数位数
	u8 p_dot;///<有功功率小数位数
	u8 q_dot;///<无功功率小数位数
	u8 p3w4; ///<电表类型,三相三线-0;三项四线-1;
	u8 ue[3]; ///<电压 百十个位
	u8 ie[2]; ///< 千,百位
	u8 iv; ///<有效标志位:位7= 1-有效 0 无效,位[0,6]保留为0
} stMtr_File;
///串口方案配置信息 等同配置文件 sioplan.cfg 结构
typedef struct
{
	u8 parity; ///<校验位 0无校验 1,偶校验 2,奇校验
	u8 data; ///<数据位 7:7位;8:8位数据位
	u8 stop; ///<停止位 1:1位停止位 2:2位停止位
	u8 baud; ///<波特率 300*2^b 0~5=300~9600
	u8 Commtype; ///<端口类型 0 异步 1 同步
} stUart_plan;
///网络参数
typedef struct
{
	u8 no; ///<序号,跟参数的位置有冗余.
	u8 ip[IPV4_LEN]; ///<ip地址,字符型,一个字符一个字节,共3*4=12字节.
	u8 mask[IPV4_LEN];
	u8 gateway[IPV4_LEN];
	u8 port[5];
} stNetparam;
/// 监视参数文件结构
typedef struct{
	u8 comm_port;//端口类型COM ETH Master等
	u8 listen_port[5];
	u8 sioplan;
	u8 port_type;
	u8 prot_addr[4];
	u8 bTimeSyn;
	u8 bForward;
	u8 forwardNum;
}stMonparam;
/// 存储周期参数文件结构
typedef struct{
	u8 enable;///<0无效,1有效
	u8 cycle;///<存储周期
}stSave_cycle;

//加载(一条)参数
int load_sysparam(stSysParam * param, const char * file);
int load_mtrparam(stMtr* pmtr, const char * file, int no);
int load_sioplan(stUart_plan * plan, const char * file, int no);
int load_netparam(stNetparam * netparam, const char * file, int no);
int load_monparam(stMonparam * monparam, const char * file, int no);
int load_savecycle(stSave_cycle sav[], const char * file);//全部储存周期
//保存(一条)参数
int save_sysparam(const stSysParam * param, const char * file);
int save_mtrparam(const stMtr * mtr, const char * file, int no);
int save_sioplan(const stUart_plan * plan, const char * file, int no);
int save_netport(const stNetparam * net, const char * file, int no);
int save_monparam(const stMonparam * mon, const char * file, int no);
int save_savecycle(const stSave_cycle sav[], const char * file);//全部储存周期
//由系统参数触发的其他参数的修改
int update_mtrfile(const stSysParam param);
int update_siofile(const stSysParam param);
int update_netparamfile(const stSysParam param);
int update_monparamfile(const stSysParam param);
//表计参数转换
int mtr_file2men(stMtr* pmtr, const stMtr_File * pmtr_file);
int mtr_men2file(stMtr_File * pmtr_file, const stMtr* pmtr);
///初始化规约,从规约文件中读取所有表计/主站规约
int read_protocol_file
	(char *protocol_names[],int *num, const char* file);
int init_monparam_port_name
	(char *port_name[],int *num, const char* file);

//
void TransBcdArray2BinArray(u8* srcbuf, u8* desbuf, u8 flag);
void TransBinArray2BcdArray(u8* srcbuf, u8* desbuf, u8 flag);
#pragma pack()
#endif /* PARAM_H_ */
