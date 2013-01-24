# 网络端口参数文件格式

默认路径/mnt/nor/para/netpara.cfg

	typedef struct
	{
		u8 no; ///<序号,表示在文件中的次序.
		u8 ip[IPV4_LEN]; ///<ip地址,字符型,一个字符一个字节,共3*4=12字节.
		u8 mask[IPV4_LEN];
		u8 gateway[IPV4_LEN];
		u8 port[5];
	} stNetparam;

本文件包含N个这样的网络端口参数,N由系统参数的网络端口个数确定.
