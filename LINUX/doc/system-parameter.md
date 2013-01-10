# 系统参数文件结构

系统参数文件:/mnt/nor/para/system.cfg

共一个结构体,文件总长度不变.

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


结构体各项意义和限制如下:
* meter_num[0,255]: 总表计个数(根据实际设置),一个字节,其中0不符合程序逻辑.
* sioports_num[0,255]: 终端串口个数(根据物理实际设置),一个字节,其中0和10以上物理上没实现.
* netports_num[0,255]: 终端网络端口数量.
* pulse_num:一个字节,已经废弃.
* monitor_ports[0,255]: 监视端口数.
* control_ports[0,255]: 控制端口数,废弃.
* sioplan_num[0,255]: 串口方案数.0表示没有,不符合要求.
