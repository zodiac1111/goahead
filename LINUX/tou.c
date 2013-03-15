/**
 * @file tou.c 电量历史数据文件
 * @param mtr_no
 * @param range
 * @param tou
 * @return
 */
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <errno.h>
#include "../uemf.h"
#include "../wsIntrn.h"
#include "tou.h"
#include "web_err.h"
#include "type.h"
#include "conf.h"
/**
 * 将时间推至次日凌晨0点,用于检索到下一个文件
 * @param stTime 时间结构体
 * @param time_t 时间戳(32位!bug Y2038)
 */
void timeToNextDayMorning(struct tm *stTime,time_t *time_t)
{
	stTime->tm_hour = 0;
	stTime->tm_min = 0;
	stTime->tm_sec = 0;
	*time_t = mktime(stTime);
	*time_t += (60*60*24);
}
/**
 * 读取一个电表的一段时间段的电量数据.
 * 这个函数思路十分难以理解.
 * 主要是按照时间戳,以一分钟为最小刻度依次从开始时刻到结束时刻遍历一遍.
 * 获取采样周期,如5分钟,则可以以5分钟为最小刻度遍历,速度更快.
 * 遍历过程中有一种比较特殊的情况:
 * 1. 开始时刻不是周期的整数倍.
 * 	如01月01日00时01分开始,读取文件mtr0010101之后发现周期是5分钟.
 * 	那个需要将起始时刻向上园整到5分钟的整数倍,即从01月01日00时05分这个时刻开始.
 * 	之后就可以按照5分钟递增了.
 * 2. 开始时刻为一日较晚时刻,采样跨天.
 * 	如01月01日23时59分开始,读取文件mtr0010101之后得到采样周期5分钟.
 * 	那么23:59加5分钟就到了次日.所以应该返回打开下一个文件mtr0010102.
 * 	然后从01月02日00时00分开始,以5分钟为一个周期开始读取.
 * 3. 在不同的天中采样周期有所变更.
 * 	程序始终以min{当前周期,文件中读取的周期}为步距递增.所以不同天不同周期没问题.
 * 4. 在同一天中变更采样周期. @bug
 * 	无法判断,因为一天就保存一个周期到文件头中.而且文件的程度计算必然出错.
 * @param[in] range
 * @param[in] mtr_no
 * @param[in] ptou
 * @param[out] wp 写入到这个页面
 * @return
 * @todo 分解,流程较为复杂
 */
int load_tou_dat(u32 mtr_no, TimeRange const range, stTou* ptou, webs_t wp)
{
	stTouFilehead filehead;
	if (range.e<range.s) {
		web_errno = tou_timerange_err;
		return ERR;
	}
	if(range.e==0){
		web_errno = tou_stime_err;
		return ERR;
	}
	if(range.e==0){
		web_errno = tou_etime_err;
		return ERR;
	}
	char file[256] = { 0 };
	struct tm stTime;
	struct tm stToday_0;//今日凌晨00点00分
	time_t today_0_t;
	time_t start_t = range.s;     //开始时刻
	time_t end_t = range.e;     //结束时刻
	time_t t2;     //时刻
	time_t minCycle_t = 0;
	stTou tou;
	memset(&tou, 0x0, sizeof(stTou));
	FILE*fp;
	int flen;
	int i = 0;
	//从开始时刻到结束时刻,按分钟遍历,步距为周期,可变.[start,end]两边闭区间
	for (t2 = start_t; t2<=end_t; /*t2 += (mincycle * 60)*/) {
		Start:
		#if __arm__ ==2
		gmtime_r(&t2,&stTime);
		gmtime_r(&t2,&stToday_0);
//		printf("gmtime_r %02d-%02d %02d:%02d %s stTime.tm_gmtoff=%d \n",
//				t.tm_mon+1,stTime.tm_mday,stTime.tm_hour,stTime.tm_min,
//				stTime.tm_zone,stTime.tm_gmtoff);
#else
		localtime_r(&t2, &stTime);
		localtime_r(&t2, &stToday_0);
#endif
		sprintf(file, "%s/mtr%03d%02d%02d.%s", TOU_DAT_DIR, mtr_no, 0,
		                stTime.tm_mday, TOU_DAT_SUFFIX);
		fp = fopen(file, "r");
		if (fp==NULL) {	//这一天没有数据,直接跳到次日零点,这不是错误
			printf(WEBS_INF"%d:%04d-%02d-%02d没有数据文件\n",
			                mtr_no, stTime.tm_year+1900, stTime.tm_mon+1
			                                , stTime.tm_mday);
			web_errno = open_tou_file;
			//到下一天的凌晨,即下一个文件.
			timeToNextDayMorning(&stTime,&t2);
			continue;
		}
		fseek(fp, 0, SEEK_END);
		flen = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		int n = fread(&filehead, sizeof(stTouFilehead), 1, fp);
		if (n!=1) {
			web_errno = read_tou_file_filehead;
			fclose(fp);
			timeToNextDayMorning(&stTime,&t2);
			continue;
		}
		///@note 检查文件头中是否和请求的日期相一致.
		if (isRightDate(filehead, stTime)==0) { //这也不算错误,最多算信息.
			fclose(fp);
			timeToNextDayMorning(&stTime,&t2);
			continue;
		}
		int cycle = (filehead.save_cycle_hi*256)
		                +filehead.save_cycle_lo;
		minCycle_t = cycle;
		//stTou at[24 * 60 / cycle];
		int t_mod = t2%(minCycle_t*60);     //向上园整至采样周期.
		if (t_mod!=0) {     //需要园整
			t2 += (minCycle_t*60-t_mod);
		}
		//}
		/**@note 判断开始时间+周期是否跨度到了第二天,如果跨度到第二天则需要
		 打开另一个(下一天)数据文件.
		 */
		stToday_0.tm_hour = 0;
		stToday_0.tm_min = 0;
		stToday_0.tm_sec = 0;
		today_0_t = mktime(&stToday_0);
		if (t2-today_0_t>=(60*60*24)) {     //t2已经时间跨过本日了.次日则文件等等需要重新打开.
			fclose(fp);
			goto Start;
		}
		///移动文件指针,指向开始的数据结构.
		int DeltaSec = t2-today_0_t;     //本采样时刻举今日凌晨几秒
		int NumCycle = DeltaSec/(minCycle_t*60);     //从凌晨开始向后偏移几个采样周期
		int offset = sizeof(stTou)*NumCycle;     //每个样本长度*采样个数
		fseek(fp, offset, SEEK_CUR);     ///当前位置为除去文件头的第一个数据体.

		if (ftell(fp)>=flen) {
			printf(WEBS_WAR"本日的数据不够.filesize=%d,fseek=%ld:%s\n", flen,
			                ftell(fp), file);
			t2 += (minCycle_t*60);
			fclose(fp);
			continue;
		}
		while (ftell(fp)<flen&&t2<=end_t) {
			memset(&tou, 0x0, sizeof(stTou));
			//t_cur += cycle * 60;
			int n = fread(&tou, sizeof(stTou), 1, fp);
			if (n!=1) {
				web_errno = read_tou_file_dat;
				return ERR;
			}
			//成功
			webWrite_toudata(t2, wp, tou, i, mtr_no);
			i++;
			t2 += (minCycle_t*60);
		}// end while 在一个文件中
		fclose(fp);
	}// end for
	return 0;
}
/**
 * 像web页面写东西
 * @param t2
 * @param wp
 * @param tou
 * @param i
 * @param mtr_no
 * @return
 */
int webWrite_toudata(time_t t2, webs_t wp, const stTou tou, int i, int mtr_no)
{
	struct tm t;
#if __arm__ ==2
	gmtime_r(&t2,&t);
#else
	localtime_r(&t2, &t);
#endif
	websWrite(wp, T("<tr>"));
	websWrite(wp, T("<td>%d</td>"), i);     //记录序号
	websWrite(wp, T("<td>%d</td>"), mtr_no);     //表号
	websWrite(wp,
	                T("<td>%04d-%02d-%02d %02d:%02d:%02d %s</td>"),
	                t.tm_year+1900,
	                t.tm_mon+1, t.tm_mday, t.tm_hour,
	                t.tm_min, t.tm_sec, t.tm_zone);
	webWrite1Tou(wp, tou);
	websWrite(wp, T("</tr>\n"));
	return 0;
}
/**
 * 将由4个字节型数组组成的浮点型转化为最短的字符输出.
 * 应为websWrite仅实现了%d和%s,所以必须转化成字符串.
 * @param float_array
 * @param strval
 * @return
 */
char * float2string( u8 const float_array[4], char * strval)
{
#if __arm__ ==1
	/*arm-linux-gcc会优化掉下面的 #else 的方式. - -
	 * 而且是优化掉第二次调用,第一次调用得出的数值还是对的
	 * 所以只能使用这种不优雅的方式防止优化掉强制类型转换
	*/
	typedef union kf{
	 char tmp[4];
	 float fot;
	}stFloat;
	stFloat f;
	f.tmp[0]=float_array[0];
	f.tmp[1]=float_array[1];
	f.tmp[2]=float_array[2];
	f.tmp[3]=float_array[3];
	sprintf(strval, "%g", f.fot);
	//float f2=*(float*) (&float_array[0]);
	//sprintf(strval, "%g", f2);
#else
	sprintf(strval, "%g", *(float*) (&float_array[0]));
#endif
	return strval;
}
/**
 * 写一条正向有功或者正向无功之类的电量的"总尖峰平谷"5个数值
 * @todo 使用json传输数据,不要传输样式和行为!
 * @param wp
 * @param ti
 * @return
 */
int webWriteOneTI(webs_t wp, Ti_Category ti)
{
	char strval[32];
	const char *iv = " class=\"iv\" ";
	const char *valid = " class=\"valid\" ";
	///保存是否无效字串的数组
	const char *isInvalid[2] = {
	                [0]=valid,     ///<有效
	                [1]=iv,     ///<无效
	                };
	//总 尖 峰 平 谷
	websWrite(wp, T("<td %s>%s</td>"),
	                isInvalid[ti.total.iv],
	                float2string(ti.total.fake_float_val, strval));
	websWrite(wp, T("<td %s>%s</td>"),
	                isInvalid[ti.tip.iv],
	                float2string(ti.tip.fake_float_val, strval));
	websWrite(wp, T("<td %s>%s</td>"),
	                isInvalid[ti.peak.iv],
	                float2string(ti.peak.fake_float_val, strval));
	websWrite(wp, T("<td %s>%s</td>"),
	                isInvalid[ti.flat.iv],
	                float2string(ti.flat.fake_float_val, strval));
	websWrite(wp, T("<td %s>%s</td>"),
	                isInvalid[ti.valley.iv],
	                float2string(ti.valley.fake_float_val, strval));
	return 0;
}
//写一条电量Tou数据
int webWrite1Tou(webs_t wp, const stTou tou)
{
	webWriteOneTI(wp, tou.FA);     ///正向有功
	webWriteOneTI(wp, tou.RA);     ///正向无功
	webWriteOneTI(wp, tou.FR);     ///反向有功
	webWriteOneTI(wp, tou.RR);     ///反向无功
	return 0;
}
/**
 * 通过比较文件头中的日期字节和请求的日期,判断是否是正确的日期,没有差几个月
 * @param filehead
 * @param t
 * @return
 */
int isRightDate(const stTouFilehead filehead, struct tm t)
{
	if (filehead.month!=t.tm_mon+1) {
		return 0;
	}
	if(filehead.year+2000!=t.tm_year+1900){
		return 0;
	}
	return 1;
}
