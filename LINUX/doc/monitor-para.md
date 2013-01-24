# 监视端口参数文件格式

默认路径 /mnt/nor/para/monpara.cfg

一条记录结构如下:

	typedef struct{
		u8 comm_port;
		u8 listen_port[5];
		u8 sioplan;
		u8 port_type;
		u8 prot_addr[4];
		u8 chktime_valid_flag;
		u8 forward_enable;
		u8 forward_mtr_num;
	}stMonparam;

本文件包含N个这样的监视端口参数,N由系统参数的监视端口个数确定.
