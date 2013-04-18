/**
 * @file param.h
 * 参数设置相关
 *  Created on: 2012-12-19
 *      Author: lee
 */
#ifndef PARAM_H_
#define PARAM_H_
#include <stdint.h>
//#pragma pack(1) 尽量少设置原则,使用__attribute__((packed, aligned(1)))指定
// gcc 3.3.2不支持 pack(1)
#include "conf.h"
#define MAX_MON_PORT_NUM 64U ///<监视参数端口描述字符串最大数量.如COM1 ETH1 等
#define IPV4_LEN 12U ///<ipv4占用4个字节,表示成12个字符
#define LINE_LEN 6U //线路号长度
#define ADDR_LEN 12U
#define PWD_LEN 8U
///<储存周期项目,目前(6项):
///1. 总电量 2. 分时电量	3.象限无功电能 4.最大需量 5.实时遥测量 6.失压断相
#define SAVE_CYCLE_ITEM (6)
///采集周期项目,6相
///1. 总电量 2. 分时电量	3.象限无功电能 4.最大需量 5.瞬时量 6.失压断相
#define COLLECT_CYCLE_ITEM (6)
extern const char *PW[2];
extern const char *PROTOROL[2];
extern const char *UART_P[3];
extern const char *UART_DAT_BIT[3];
extern const char *UART_STOP[2];
extern const char *UART_BAUD[7];
extern const char *UART_COMM_TYPE[2];
extern const char *SAVE_CYCLE[13];
extern const char *COLLECT_CYCLE[13];
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
	uint8_t meter_num;
	uint8_t sioports_num;
	uint8_t netports_num;
	uint8_t pulse_num;
	uint8_t monitor_ports;
	uint8_t control_ports;
	uint8_t sioplan_num;
} __attribute__((packed, aligned(1))) stSysParam;
/**
 * 程序中操作的表参数结构体,具体size大小不重要,只要能容纳就行
 * 存进文件时转化成为_File类型.从文件读出时转化为本结构体类型
 */
typedef struct
{
	uint8_t mtrno; ///<表号,这是文件中没有的,文件中顺序存放,结构体为了方便使用,增加表号
	uint8_t line[LINE_LEN];
	uint8_t addr[ADDR_LEN]; ///<12位的十进制数,需要8个字节长度的long long 整形
	uint8_t pwd[PWD_LEN]; ///<8位十进制数....
	uint8_t port;///<串口
	uint8_t portplan;///<串口方案号 0开始
	uint8_t fact;///<厂商表号
	uint8_t protocol;///<规约号
	uint8_t it_dot; ///<电量小数位数
	uint8_t xl_dot; ///<需量小数位数
	uint8_t v_dot; ///<电压
	uint8_t i_dot;///<电流小数位数
	uint8_t p_dot;///<有功功率小数位数
	uint8_t q_dot;///<无功功率小数位数
	uint8_t p3w4; ///<电表类型:三相三线-0 三相四线制-1;
	uint32_t ue; ///<电压 百十个位 (000~999)  乘以 10V
	uint32_t ie; ///< 千,百位 00~99 乘以 100mA
	uint8_t iv;    ///有效标志位:位7= 1-有效 0 无效,位[0,6]保留为0
} __attribute__((packed, aligned(1))) stMtr;
/// mtrspara.cfg 文件中储存的表参数格式结构体,
typedef struct
{
	uint8_t line[LINE_LEN]; ///<0-5 共6 线路名 字节流
	uint8_t addr[ADDR_LEN]; ///<表地址,一字节一个十进制数,每位范围[0,9]超过一定是错误的
	uint8_t pwd[PWD_LEN]; ///<密码
	uint8_t port; ///<串口号 com0 com1 等
	uint8_t portplan; ///<串口方案号,第几种串口方案,和串口方案相关联起来
	uint8_t type; ///<表计类型?什么类型?0x00
	uint8_t protocol; ///<表计规约
	uint8_t it_dot; ///<电量小数位数
	uint8_t xl_dot; ///<需量小数位数
	uint8_t v_dot; ///<电压
	uint8_t i_dot;///<电压小数位数
	uint8_t p_dot;///<有功功率小数位数
	uint8_t q_dot;///<无功功率小数位数
	uint8_t p3w4; ///<电表类型,三相三线-0;三项四线-1;
	uint8_t ue[3]; ///<电压 百十个位
	uint8_t ie[2]; ///< 千,百位
	uint8_t iv; ///<有效标志位:位7= 1-有效 0 无效,位[0,6]保留为0
} __attribute__((packed, aligned(1))) stMtr_File;
///串口方案配置信息 等同配置文件 sioplan.cfg 结构
typedef struct
{
	uint8_t parity; ///<校验位 0无校验 1,偶校验 2,奇校验
	uint8_t data; ///<数据位 7:7位;8:8位数据位
	uint8_t stop; ///<停止位 1:1位停止位 2:2位停止位
	uint8_t baud; ///<波特率 300*2^b 0~5=300~9600
	uint8_t Commtype; ///<端口类型 0 异步 1 同步
} __attribute__((packed, aligned(1))) stUart_plan;
///网络参数
typedef struct
{
	uint8_t no; ///<序号,跟参数的位置有冗余.
	uint8_t ip[IPV4_LEN]; ///<ip地址,字符型,一个字符一个字节,共3*4=12字节.
	uint8_t mask[IPV4_LEN];
	uint8_t gateway[IPV4_LEN];
	uint8_t port[5];
} __attribute__((packed, aligned(1))) stNetparam;
/// 监视参数文件结构
typedef struct{
	uint8_t comm_port;//端口类型COM ETH Master等
	uint8_t listen_port[5];
	uint8_t sioplan;
	uint8_t port_type;
	uint8_t prot_addr[4];
	uint8_t bTimeSyn;
	uint8_t bForward;
	uint8_t forwardNum;
}__attribute__((packed, aligned(1)))stMonparam;
/// 存储周期参数文件结构
typedef struct{
	uint8_t enable;///<0无效,1有效
	uint8_t cycle;///<存储周期
} __attribute__((packed, aligned(1))) stSave_cycle;

/// 采集周期参数文件结构
typedef struct{
	uint8_t enable;///<0无效,1有效
	uint8_t cycle;///<存储周期
} __attribute__((packed, aligned(1))) stCollect_cycle;

//加载(一条)参数
extern int load_sysparam(stSysParam * param, const char * file);
extern int load_mtrparam(stMtr* pmtr, const char * file, int no);
extern int load_sioplan(stUart_plan * plan, const char * file, int no);
extern int load_netparam(stNetparam * netparam, const char * file, int no);
extern int load_monparam(stMonparam * monparam, const char * file, int no);
extern int load_savecycle(stSave_cycle sav[], const char * file);//全部储存周期
extern int load_collect_cycle(stCollect_cycle collect[], const char * file);//采集周期
//保存(一条)参数
extern int save_sysparam(const stSysParam * param, const char * file);
extern int save_mtrparam(const stMtr * mtr, const char * file, int no);
extern int save_sioplan(const stUart_plan * plan, const char * file, int no);
extern int save_netport(const stNetparam * net, const char * file, int no);
extern int save_monparam(const stMonparam * mon, const char * file, int no);
extern int save_savecycle(const stSave_cycle sav[], const char * file);//全部储存周期
extern int save_collect_cycle(const stCollect_cycle sav[], const char * file);//采集周期
///初始化规约,从规约文件中读取所有表计/主站规约
extern int read_protocol_file
	(char *protocol_names[],int *num, const char* file);
extern int init_monparam_port_name
	(char *port_name[],int *num, const char* file);

//

#pragma pack()
#endif /* PARAM_H_ */
