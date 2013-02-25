串口方案文件 {#sio_format}
===========
串口方案文件默认路径 /mnt/nor/para/sioplan.cfg

一条方案由一下结构组成:

	typedef struct
	{
		u8 parity; ///<校验位 0无校验 1,偶校验 2,奇校验
		u8 data; ///<数据位 7:7位;8:8位数据位
		u8 stop; ///<停止位 1:1位停止位 2:2位停止位
		u8 baud; ///<波特率 300*2^b 0~5=300~9600
		u8 Commtype; ///<端口类型 0 异步 1 同步
	} stUart_plan;

本文件中保存N个串口方案结构体,N由系统参数的串口方案数确定.
