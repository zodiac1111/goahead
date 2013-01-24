/**
 * @file param.c
 *  Created on: 2012-12-19
 *      Author: lee
 */
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <malloc.h>
#include "param.h"
#include "Chinese_string.h"
#include "web_err.h"
//电表类型 几相几线制
const char *PW[2] = { [0]=CSTR_3P3W,
[1]=CSTR_3P4W
};
const char *UART_P[3] = { [0]=CSTR_NO_PARITY,
[1]=CSTR_EVEN_PARITY,
[2]=CSTR_ODD_PARITY
};
const char *UART_DAT_BIT[] = { [0]="7", [1]="8", [2]="9" };
const char *UART_STOP[] = { [0]="0", [1]="1" };
const char *UART_BAUD[] = { [0]="300", [1]="600", [2]="1200", [3]="2400", [4
		]="4800", [5]="9600" };
//串口通讯方式
const char *UART_COMM_TYPE[] = { [0]=CSTR_UART_SYN, [1]=CSTR_UART_ASYN,};
///@todo  硬编码!可以使用配置文件读取/写入
const char* SAVE_CYCLE[]={
		[0]="1"CSTR_MIN,
		[1]="5"CSTR_MIN,
		[2]="10"CSTR_MIN,
		[3]="15"CSTR_MIN,
		[4]="30"CSTR_MIN,
		[5]="60"CSTR_MIN,
		[6]="120"CSTR_MIN,
		[7]="180"CSTR_MIN,
		[8]="360"CSTR_MIN,
		[9]="480"CSTR_MIN,
		[10]="720"CSTR_MIN,
		[11]="1440"CSTR_MIN,
};
/**
 * 从文本格式的配置文件中读取规约名称字符串.
 * 传入protocol_names数组后为其赋值,并传出.
 * 传入num最大限制规约数,判断是否超限,传出实际文件中有多少条规约.
 * @param[out] protocol_names char*型数组,每个元素指向一个规约字符串
 * @param[in] file 规约文件
 * @param[out] max 最大规约数目[输入]/实际规约数目[输出]
 * @retval 0 成功,其他错误.
 */
int read_protocol_file(char *protocol_names[], int *max, const char* file)
{
	FILE* fp;
	int i = 0;
	int len = 0;
	int strnum = 0;
	char protocolname[256];
	char line[256];
	fp = fopen(file, "rb");
	if (fp == NULL ) {
		perror("open");
		PRINT_HERE
		web_errno = open_protocolfile_err;
		return -1;
	}
	fseek(fp, 0, SEEK_END);
	int flen = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	while (ftell(fp) < flen) {
		//最多多少个规约,如果比预计的多了,返回错误.
		if (i >= (*max)) {
			web_errno = toomany_protocol_err;
			return -1;
		}
		fgets(line, 255, fp);
		strnum = sscanf(line, "%255s\n", protocolname);
		if (strnum == -1) { //忽略空行
			continue;
		}
		//printf("%d: %s\n", i, protocolname);
		len = strlen(protocolname) + 1; // '\0'
		if (protocolname[0] == '#') {		//允许注释
			continue;
		}
		protocol_names[i] = (char*) malloc(sizeof(char) * (len));
		memcpy(protocol_names[i], protocolname, len);
		i++;
	}
	*max = i;
	//查看一下
	//printf("read_protocol_file: num=%d\n", i);
	for (i = 0; i < *max; i++) {
		//printf("\t[%d]: %s\n", i, protocol_names[i]);
	}
	return 0;
}
/**
 * 从file(默认位置/mnt/nor/conf/monparam_config.txt)文本文件中读取监视端口描述,
 * 如 COM1,ETH1,Master-0 等等.
 * 如果打开默认文件错误,则从代码中的硬编码数组MON_PORT[]读入做为默认值.
 * @param[out] port_name 端口名称字符串数组.
 * @param[out] num 输入实际端口数量,输入最大端口数量
 * @param[in] file 配置文件
 * @return 0 成功, 非0 失败
 */
int init_monparam_port_name(char *port_name[], int *num, const char* file)
{
	FILE* fp;
	int i = 0; //int j=0;
	int len = 0;
	int strnum = 0;
	char line[256];
	char strname[256];
	fp = fopen(file, "rb");
	if (fp == NULL ) {
		web_errno = open_monitor_name_file_err;
		return -1;
	}
	fseek(fp, 0, SEEK_END);
	int flen = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	while (ftell(fp) < flen) {
		if (i >= (*num)) { //大于最大监视端口描述数量.
			return -3;
		}
		fgets(line, 255, fp); //得到一行
		strnum = sscanf(line, "%255s\n", strname); //得到这一行的字符串
		if (strnum == -1) { //忽略空行
			continue;
		}
		len = strlen(strname) + 1; // '\0'
		if (strname[0] == '#') { //允许注释
			continue;
		}
		port_name[i] = (char*) malloc(sizeof(char) * (len));
		memcpy(port_name[i], strname, len);
		i++;
	}
	*num = i;
	//printf("init_monparam_port_name: fact_num=%d\n", i);
	for (i = 0; i < *num; i++) {
		//printf("\t[%d]: %s\n", i, port_name[i]);
	}
	return 0;
}
/**
 * 从文件中读取表计参数到一个结构体.
 * @param[out] pmtr
 * @param[in] file
 * @param[in] no
 * @retval 0-成功
 * @retval -1-失败,同时设置web_errno错误号.
 */
int load_mtrparam(stMtr* pmtr, const char * file, int no)
{
	stMtr_File mtr_file;
	FILE* fp;
	int ret = 0;
	fp = fopen(file, "rb");
	if (fp == NULL ) {
		perror("open");
		PRINT_HERE
		web_errno = open_mtrcfgfile_err;
		return -1;
	}
	fseek(fp, 0, SEEK_END);
	long unsigned int flen = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	//printf("sizeof struct=%d\n",sizeof(stMtr_File));
	if (flen < sizeof(stMtr_File) * (no + 1)) {
		PRINT_HERE
		printf("mtrparam num no=%d\n", no);
		web_errno = no_this_mtrparam;
		return -1;
	}
	fseek(fp, sizeof(stMtr_File) * no, SEEK_SET);
	ret = fread(&mtr_file, sizeof(mtr_file), 1, fp);
	if (ret != 1) {
		perror("read");
		PRINT_HERE
		web_errno = read_mtrcfgfile_err;
		return -1;
	}
	fclose(fp);
	//转化
	ret = mtr_file2men(pmtr, &mtr_file);
	if (ret != 0) {
		PRINT_HERE
		web_errno = mtr_file2men_err;
		return -1;
	}
	return 0;
}
/**
 * 将某个表参数写入到文件.
 * @param[in] mtr  表参数结构体
 * @param[in] file 表参数文件路径
 * @param[in] no 表号 base 0
 * @return
 */
int save_mtrparam(const stMtr * mtr, const char * file, int const no)
{
	printf("save_mtrparam %x %x %x %x %x %x\n", mtr->line[0], mtr->line[1],
			mtr->line[2], mtr->line[3], mtr->line[4], mtr->line[5]);
	stMtr_File mtr_file;
	FILE* fp;
	int ret = 0;
	fp = fopen(file, "r+");
	if (fp == NULL ) {
		perror("open");
		PRINT_HERE
		web_errno = open_mtrcfgfile_err;
		return -1;
	}
	fseek(fp, 0, SEEK_END);
	long  int flen = ftell(fp);
	printf("save_mtrparam 文件长度:%ld \n", flen);
	fseek(fp, 0, SEEK_SET);
	//printf("sizeof struct=%d\n",sizeof(stMtr_File));
	if (flen < (long int)sizeof(stMtr_File) * (no + 1)) {
		int fd = fileno(fp);
		///确保文件的大小,大了截断,小了添0,这个文件应该是正好sizeof(stSysParam)大小,
		///不应该小一个字节或者大一个字节.
		ret = ftruncate(fd, sizeof(stMtr_File) * (no + 1));
	}
	fseek(fp, 0, SEEK_SET);
	fseek(fp, sizeof(stMtr_File) * no, SEEK_SET);		//指向特定的表参数
	mtr_men2file(&mtr_file, mtr);		//转化
	ret = fwrite(&mtr_file, sizeof(mtr_file), 1, fp);		//写入
	if (ret != 1) {
		perror("write");
		PRINT_HERE
		web_errno = write_mtrcfgfile_err;
		return -1;
	}
	fclose(fp);
	return 0;
}
/**
 * 表计参数的从文件中的表计参数结构体转化为程序中的结构体,方便计算和使用
 * @param[out] pmtr
 * @param[in] pmtr_file
 * @return
 */
int mtr_file2men(stMtr* pmtr, const stMtr_File * pmtr_file)
{
	memcpy((void*) pmtr->line, (void*) pmtr_file->line, LINE_LEN);
	memcpy((void*) pmtr->addr, (void*) pmtr_file->addr, ADDR_LEN);
	memcpy((void*) pmtr->pwd, (void*) pmtr_file->pwd, PWD_LEN);
	pmtr->port = pmtr_file->port;
	pmtr->portplan = pmtr_file->portplan;
	pmtr->fact = pmtr_file->type;
	pmtr->protocol = pmtr_file->protocol;
	pmtr->it_dot = pmtr_file->it_dot;
	pmtr->xl_dot = pmtr_file->xl_dot;
	pmtr->v_dot = pmtr_file->v_dot;
	pmtr->i_dot = pmtr_file->i_dot;
	pmtr->p_dot = pmtr_file->p_dot;
	pmtr->q_dot = pmtr_file->q_dot;
	pmtr->p3w4 = pmtr_file->p3w4;
	pmtr->ue = pmtr_file->ue[2] * 1 + pmtr_file->ue[1] * 10
			+ pmtr_file->ue[0] * 100;
	pmtr->ie = pmtr_file->ie[1] * 100 + pmtr_file->ie[0] * 1000;
	pmtr->iv = pmtr_file->iv;

	return 0;
}
/**
 * 将表计参数结构体转化成为保存文件的格式.
 * @param[out] pmtr_file
 * @param[in] pmtr
 * @retval 0 正确
 * @retval !0 错误
 */
int mtr_men2file(stMtr_File * pmtr_file, const stMtr* pmtr)
{
	memcpy(pmtr_file->line, pmtr->line, LINE_LEN);
	printf("save_mtrparam[%d] %x %x %x %x %x %x\n", pmtr->mtrno,
			pmtr->line[0], pmtr->line[1], pmtr->line[2],
			pmtr->line[3], pmtr->line[4], pmtr->line[5]);
	memcpy(pmtr_file->addr, pmtr->addr, ADDR_LEN);
	memcpy(pmtr_file->pwd, pmtr->pwd, PWD_LEN);
	pmtr_file->port = pmtr->port;
	pmtr_file->portplan = pmtr->portplan;
	pmtr_file->type = pmtr->fact;
	pmtr_file->protocol = pmtr->protocol;
	pmtr_file->it_dot = pmtr->it_dot;
	pmtr_file->xl_dot = pmtr->xl_dot;
	pmtr_file->v_dot = pmtr->v_dot;
	pmtr_file->i_dot = pmtr->i_dot;
	pmtr_file->p_dot = pmtr->p_dot;
	pmtr_file->q_dot = pmtr->q_dot;
	pmtr_file->p3w4 = pmtr->p3w4;
	pmtr_file->ue[0] = (pmtr->ue / 100) % 10;
	pmtr_file->ue[1] = (pmtr->ue / 10) % 10;
	pmtr_file->ue[2] = (pmtr->ue / 1) % 10;
	pmtr_file->ie[0] = (pmtr->ie / 1000) % 10;
	pmtr_file->ie[1] = (pmtr->ie / 100) % 10;
	pmtr_file->iv = pmtr->iv;
	return 0;
}
/**
 * 从文件中读取系统参数到结构体
 * @param[out] param 读出的系统数据结构体
 * @param[in] file 系统参数文件路径
 * @retval 0:成功
 * @retval -1:失败 同时设置web_errno 错误号
 */
int load_sysparam(stSysParam * param, const char * file)
{
	//stSysParam mtr_file;
	FILE* fp;
	int ret = 0;
	fp = fopen(file, "rb");
	if (fp == NULL ) {
		perror("open sysparam.cfg");
		PRINT_HERE
		web_errno = open_sysparam_file;
		return -1;
	}
	fseek(fp, 0, SEEK_END);
	long  int  flen = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	//printf("sizeof struct=%d\n",sizeof(stMtr_File));
	if (flen < (long int)sizeof(stSysParam)) {
		PRINT_HERE
		web_errno = sysfile_size_err;
		return -1;
	}
	ret = fread(param, sizeof(stSysParam), 1, fp);
	if (ret != 1) {
		perror("read");
		PRINT_HERE
		web_errno = read_sysfile_err;
		return -1;
	}
	fclose(fp);
	return 0;
}
/**
 * 采集周期.
 * @param param
 * @param file
 * @retval 0:成功
 * @retval -1:失败 同时设置web_errno 错误号
 */
int load_stsparam(stSysParam * param, const char * file)
{
	//stSysParam mtr_file;
	FILE* fp;
	int ret = 0;
	fp = fopen(file, "rb");
	if (fp == NULL ) {
		perror("open sysparam.cfg");
		PRINT_HERE
		web_errno = open_sysparam_file;
		return -1;
	}
	fseek(fp, 0, SEEK_END);
	long unsigned int flen = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	//printf("sizeof struct=%d\n",sizeof(stMtr_File));
	if (flen < sizeof(stSysParam)) {
		PRINT_HERE
		web_errno = sysfile_size_err;
		return -1;
	}
	ret = fread(param, sizeof(stSysParam), 1, fp);
	if (ret != 1) {
		perror("read");
		PRINT_HERE
		web_errno = read_sysfile_err;
		return -1;
	}
	fclose(fp);
	return 0;
}
/**
 * 保存系统参数到文件.
 * @param[in] param 系统参数结构体
 * @param[in] file 系统参数文件路径
 * @retval 0:成功
 * @retval -1:失败 同时设置web_errno错误号
 */
int save_sysparam(const stSysParam * param, const char * file)
{
	//stSysParam mtr_file;
	FILE* fp;
	int ret = 0;
	fp = fopen(file, "wb");
	if (fp == NULL ) {
		perror("open sysparam.cfg");
		PRINT_HERE
		web_errno = open_sysparam_file;
		return -1;
	}
	int fd = fileno(fp);
	///确保文件的大小,大了截断,小了添0,这个文件应该是正好sizeof(stSysParam)大小,
	///不应该小一个字节或者大一个字节.
	ret = ftruncate(fd, sizeof(stSysParam));
	if (ret != 0) {
		perror("ftruncate");
		PRINT_HERE
		web_errno = write_sysfile_size_err;
		return -1;
	}
	ret = fwrite(param, sizeof(stSysParam), 1, fp);
	if (ret != 1) {
		perror("write");
		PRINT_HERE
		web_errno = write_sysfile_err;
		return -1;
	}
	fclose(fp);
	/**
	 * 由于系统参数更改引起其他文件的更改
	 */
	ret = update_mtrfile(*param);  ///更新表计个数
	if (ret != 0) {
		web_errno = update_mtr_file_err;
		return -1;
	}
	ret = update_siofile(*param);  ///串口方案个数
	if (ret != 0) {
		web_errno = update_sio_file_err;
		return -1;
	}
	ret = update_netparamfile(*param);  ///网口个数
	if (ret != 0) {
		web_errno = update_netparam_file_err;
		return -1;
	}
	ret = update_monparamfile(*param);  ///监视端口数
	if (ret != 0) {
		web_errno = update_monparam_file_err;
		return -1;
	}
	return 0;
}
/**
 * 加载一条串口方案
 * @param[out] plan 串口方案结构体
 * @param[in] file 串口方案文件
 * @param[in] no 串口方案序号 base 0
 * @retval 0 成功
 * @retval -1:失败 同时设置web_errno错误号
 */
int load_sioplan(stUart_plan * plan, const char * file, int no)
{
	FILE* fp;
	int ret = 0;
	fp = fopen(file, "rb");
	if (fp == NULL ) {
		perror("open");
		PRINT_HERE
		web_errno = open_sioplan_cfgfile_err;
		return -1;
	}
	fseek(fp, 0, SEEK_END);
	long int flen = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	if (flen < sizeof(stUart_plan) * (no + 1)) {
		PRINT_HERE
		printf("sioplan no=%d\n", no);
		web_errno = no_this_sioplan;
		return -1;
	}
	fseek(fp, sizeof(stUart_plan) * no, SEEK_SET);
	ret = fread(plan, sizeof(stUart_plan), 1, fp);
	if (ret != 1) {
		perror("read");
		PRINT_HERE
		web_errno = read_sioplan_cfgfile_err;
		return -1;
	}
	fclose(fp);
	return 0;
}
/**
 * 将某一条串口方案写入到文件.
 * @param[in] plan  表参数结构体
 * @param[in] file 表参数文件路径
 * @param[in] no 表号 base 0
 * @retval 0 成功
 * @retval 非0 失败
 */
int save_sioplan(const stUart_plan * plan, const char * file, int no)
{
	FILE* fp;
	int ret = 0;
	fp = fopen(file, "r+");
	if (fp == NULL ) {
		perror("open");
		PRINT_HERE
		web_errno=open_sioplan_cfgfile_err;
		return(-1);
	}
	fseek(fp, 0, SEEK_END);
	long  int  flen = ftell(fp);
	printf("save_sioplan 文件长度:%lu \n", flen);
	fseek(fp, 0, SEEK_SET);
	//printf("sizeof struct=%d\n",sizeof(stMtr_File));
	if (flen < sizeof(stUart_plan) * (no + 1)) {
		int fd = fileno(fp);
		///确保文件的大小,大了截断,小了添0,这个文件应该是正好sizeof(stSysParam)大小,
		///不应该小一个字节或者大一个字节.
		ret = ftruncate(fd, sizeof(stUart_plan) * (no + 1));
	}
	fseek(fp, 0, SEEK_SET);
	fseek(fp, sizeof(stUart_plan) * no, SEEK_SET);		//指向特定的表参数
	ret = fwrite(plan, sizeof(stUart_plan), 1, fp);		//写入
	if (ret != 1) {
		perror("write");
		PRINT_HERE
		web_errno= write_sioplan_cfgfile_err;
		return(-1);
	}
	fclose(fp);
	return 0;
}
/**
 * 保存一条监视参数到文件
 * @param[in] mon 监视参数结构体
 * @param[in] file 监视参数文件
 * @param[in] no 第几条监视参数
 * @return
 */
int save_monparam(const stMonparam * mon, const char * file, int no)
{
	FILE* fp;
	int ret = 0;
	fp = fopen(file, "r+");
	if (fp == NULL ) {
		perror("open");
		PRINT_HERE
		web_errno= open_monparam_cfgfile_err;
		return(-1);
	}
	fseek(fp, 0, SEEK_END);
	long unsigned int  flen = ftell(fp);
	printf("save_monparam 文件长度:%lu \n", flen);
	fseek(fp, 0, SEEK_SET);
	//printf("sizeof struct=%d\n",sizeof(stMtr_File));
	if (flen < sizeof(stMonparam) * (no + 1)) {
		int fd = fileno(fp);
		///确保文件的大小,大了截断,小了添0,这个文件应该是正好sizeof(stSysParam)大小,
		///不应该小一个字节或者大一个字节.
		ret = ftruncate(fd, sizeof(stMonparam) * (no + 1));
	}
	fseek(fp, 0, SEEK_SET);
	fseek(fp, sizeof(stMonparam) * no, SEEK_SET);		//指向特定的表参数
	ret = fwrite(mon, sizeof(stMonparam), 1, fp);		//写入
	if (ret != 1) {
		perror("write");
		PRINT_HERE
		web_errno= write_monparam_cfgfile_err;
		return(-1);
	}
	fclose(fp);
	return 0;
}
/**
 * 保存一条网口信息到文件.
 * @param[in] net 网络端口参数结构体
 * @param[in] file 网口参数文件
 * @param[in] no
 * @return
 */
int save_netport(const stNetparam * net, const char * file, int no)
{
	FILE* fp;
	int ret = 0;
	fp = fopen(file, "r+");
	if (fp == NULL ) {
		perror("open");
		PRINT_HERE
		web_errno= open_netparam_cfgfile_err;
		return (-1);
	}
	fseek(fp, 0, SEEK_END);
	long unsigned int  flen = ftell(fp);
	printf("save_netprot 文件长度:%lu \n", flen);
	fseek(fp, 0, SEEK_SET);
	//printf("sizeof struct=%d\n",sizeof(stMtr_File));
	if (flen < sizeof(stNetparam) * (no + 1)) {
		int fd = fileno(fp);
		///确保文件的大小,大了截断,小了添0,这个文件应该是正好sizeof(stSysParam)大小,
		///不应该小一个字节或者大一个字节.
		ret = ftruncate(fd, sizeof(stNetparam) * (no + 1));
	}
	fseek(fp, 0, SEEK_SET);
	fseek(fp, sizeof(stNetparam) * no, SEEK_SET);		//指向特定的表参数
	ret = fwrite(net, sizeof(stNetparam), 1, fp);		//写入
	if (ret != 1) {
		perror("write");
		PRINT_HERE
		web_errno= write_netparam_cfgfile_err;
		return(-1);
	}
	fclose(fp);
	return 0;
}
/**
 * 根据系统参数中的表计个数更新表计参数,电表个数变化的话,表计参数个数也应该变化
 * @param netparam
 * @param file
 * @param no
 * @return
 */
int load_netparam(stNetparam * netparam, const char * file, int no)
{
	FILE* fp;
	int ret = 0;
	fp = fopen(file, "rb");
	if (fp == NULL ) {
		perror("open");
		PRINT_HERE
		web_errno = open_netparam_cfgfile_err;
		return -1;
	}
	fseek(fp, 0, SEEK_END);
	u32 flen = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	if (flen < sizeof(stNetparam) * (no + 1)) {
		PRINT_HERE
		printf("monparam no=%d\n", no);
		web_errno = no_this_netparam;
		return -1;
	}
	fseek(fp, sizeof(stNetparam) * no, SEEK_SET);
	ret = fread(netparam, sizeof(stNetparam), 1, fp);
	if (ret != 1) {
		perror("read");
		PRINT_HERE
		web_errno = read_netparam_cfgfile_err;
		return -1;
	}
	fclose(fp);
	return 0;

}
/**
 * 加载一条监视参数记录
 * @param[out] monparam
 * @param file
 * @param no
 * @return
 */
int load_monparam(stMonparam * monparam, const char * file, int no)
{
	FILE* fp;
	int ret = 0;
	fp = fopen(file, "rb");
	if (fp == NULL ) {
		perror("open");
		PRINT_HERE
		web_errno = open_monparam_cfgfile_err;
		return -1;
	}
	fseek(fp, 0, SEEK_END);
	long unsigned int  flen = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	if (flen < sizeof(stMonparam) * (no + 1)) {
		PRINT_HERE
		printf("monparam no=%d\n", no);
		web_errno = no_this_monparam;
		return -1;
	}
	fseek(fp, sizeof(stMonparam) * no, SEEK_SET);
	ret = fread(monparam, sizeof(stMonparam), 1, fp);
	if (ret != 1) {
		perror("read");
		PRINT_HERE
		web_errno = read_monparam_cfgfile_err;
		return -1;
	}
	fclose(fp);
	return 0;
}
/**
 * 读取/加载 SAVE_CYCLE_ITEM 条存储周期记录
 * @param[out] sav 存储周期记录,
 * @param[in] file 存储周期配置文件
 * @return
 */
int load_savecycle(stSave_cycle sav[], const char * file)
{
	FILE* fp;
	int ret = 0;
	fp = fopen(file, "rb");
	if (fp == NULL ) {
		perror("open");
		PRINT_HERE
		web_errno = open_savecycle_cfgfile_err;
		return -1;
	}
	fseek(fp, 0, SEEK_END);
	int flen = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	if (flen != sizeof(stSave_cycle) * SAVE_CYCLE_ITEM) {
		PRINT_HERE
		web_errno = savecycle_cfgfile_size_err;
		return -1;
	}
	ret = fread(sav, sizeof(stSave_cycle)* SAVE_CYCLE_ITEM, 1, fp);
	if (ret != 1) {
		perror("read");
		PRINT_HERE
		web_errno = read_savecycle_cfgfile_err;
		return -1;
	}
	fclose(fp);
	return 0;
}
/**
 * 保存"储存周期".
 * @param sav
 * @param file
 * @return
 */
int save_savecycle(const stSave_cycle sav[], const char * file)
{
	FILE* fp;
	int ret = 0;
	fp = fopen(file, "r+");
	if (fp == NULL ) {
		perror("open");
		PRINT_HERE
		web_errno = open_savecycle_cfgfile_err;
		return (-1);
	}
	fseek(fp, 0, SEEK_END);
	int flen = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	if (flen != sizeof(stSave_cycle) * SAVE_CYCLE_ITEM) {
		int fd = fileno(fp);
		ret = ftruncate(fd, sizeof(stSave_cycle) * SAVE_CYCLE_ITEM);
		if(ret!=0){
			return (-1);
		}
	}
	ret = fwrite(sav, sizeof(stSave_cycle)* SAVE_CYCLE_ITEM, 1, fp);		//写入
	if (ret != 1) {
		perror("write");
		PRINT_HERE
		web_errno= write_netparam_cfgfile_err;
		return (-1);
	}
	fclose(fp);
	return 0;
}
/**
 * 由于系统参数更改导致的表计参数文件的长度(表计参数个数)的修改
 * @param[in] param 系统参数
 * @return
 */
int update_mtrfile(const stSysParam param)
{
	int ret = -1;
	FILE* fp = fopen(CFG_MTR, "rb+");
	if (fp == NULL ) {
		web_errno = open_mtrcfgfile_err;
		return -1;
	}
	//伸缩文件长度.变长添0,变短直接截断
	int fd = fileno(fp);
	ret = ftruncate(fd, param.meter_num * sizeof(stMtr_File));
	if (ret != 0) {
		return -1;
	}
	fclose(fp);
	return 0;
}
/**
 * 由于系统参数更改导致的串口方案文件的长度(串口方案个数)的修改
 * @param[in] param 系统参数
 * @retval 0:成功
 * @retval -1:失败 web_errno中保存错误号.
 */
int update_siofile(const stSysParam param)
{
	int ret = -1;
	FILE* fp = fopen(CFG_SIOPALN, "rb+");
	if (fp == NULL ) {
		web_errno = open_sioplan_cfgfile_err;
		return -1;
	}
	//伸缩文件长度.变长添0,变短直接截断
	int fd = fileno(fp);
	ret = ftruncate(fd, param.sioplan_num * sizeof(stUart_plan));
	if (ret != 0) {
		return -1;
	}
	fclose(fp);
	return 0;
}
/**
 * 由于系统参数更改导致的网络端口参数文件的长度(网口个数)的修改
 * @param[in] param 系统参数
 * @return
 */
int update_netparamfile(const stSysParam param)
{
	int ret = -1;
	FILE* fp = fopen(CFG_NET, "rb+");
	if (fp == NULL ) {
		web_errno = open_netparam_cfgfile_err;
		return -1;
	}
	//伸缩文件长度.变长添0,变短直接截断
	int fd = fileno(fp);
	ret = ftruncate(fd, param.netports_num * sizeof(stNetparam));
	if (ret != 0) {
		return -1;
	}
	fclose(fp);
	return 0;
}
/**
 * 由于系统参数更改导致的监视参数文件的长度的修改.
 * 根据系统参数改变监视参数文件的大小.
 * @param[in] param 系统参数
 * @return
 */
int update_monparamfile(const stSysParam param)
{
	int ret = -1;
	FILE* fp = fopen(CFG_MON_PARAM, "rb+");
	if (fp == NULL ) {
		web_errno = open_monparam_cfgfile_err;
		return -1;
	}
	//伸缩文件长度.变长添0,变短直接截断
	int fd = fileno(fp);
	ret = ftruncate(fd, param.monitor_ports * sizeof(stMonparam));
	if (ret != 0) {
		return -1;
	}
	fclose(fp);
	return 0;
}

void TransBcdArray2BinArray(u8* srcbuf, u8* desbuf, u8 flag)
{
	unsigned char i;
	for (i = 0; i < flag; i++) {
		*(desbuf + i) = *(srcbuf + i / 2) & (i % 2 ? 0x0f : 0xf0);
		if (i % 2 == 0)
			*(desbuf + i) >>= 4;
	}
}
void TransBinArray2BcdArray(u8* srcbuf, u8* desbuf, u8 flag)
{
	unsigned char i;
	for (i = 0; i < flag; i++)
		*(desbuf + i) = (*(srcbuf + 2 * i) << 4)
				| (*(srcbuf + 2 * i + 1));
}
