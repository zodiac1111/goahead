#ifndef _TOU_H_
#define _TOU_H_
#include <time.h>
#include "../uemf.h"
#include "../wsIntrn.h"
#include "type.h"
#pragma pack(1)

#define DATA_DIR "../../nand" //数据目录
#if __i386 == 1
#define TOU_DAT_DIR DATA_DIR"/tou" //电量Tou数据目录
#else
#define TOU_DAT_DIR DATA_DIR
#endif
#define TOU_DAT_SUFFIX "tou" //电量数据文件后缀名
///tou 文件头
typedef struct {
	u8 year; //+2000
	u8 month;
	u8 day;
	u8 save_cycle_lo;///<存储周期[分钟]
	u8 save_cycle_hi;///<存储周期[分钟]
	u8 save_number;
	u8 save_flag1;
	u8 save_flag2;
	u8 save_flag3;
	u8 save_flag4;
}stTouFilehead;
///某单独电量结构,如 总电量 或者 谷电量
typedef struct {
	///以4个字节型组织的假装的浮点型,注意读取时顺序
	u8 fake_float_val[4];
	union{
		u8 byte;
		struct{
			u8 res:7;///<保留 取0
			u8 iv:1;///<有效标志,1-有效
		};
	};
}Ti;
///tou 总尖峰平谷
typedef struct {
	 Ti total;///<总
	 Ti tip;///<尖
	 Ti peak;///<峰
	 Ti flat;///<平
	 Ti valley;///<谷
}Ti_Category;
///四种正反,有无功率.
typedef struct {
	 Ti_Category FA;///<正有
	 Ti_Category RA;///<反有
	 Ti_Category FR;///<正无
	 Ti_Category RR;///<反无
}stTou;
///时间跨度,开始时间和结束时间
typedef struct{
	time_t s;
	time_t e;
}TimeRange;
//
int load_tou_dat(u32 mtr_no,TimeRange const range,stTou* ptou,webs_t wp);
int webWrite1Tou(webs_t wp,const stTou tou);
int webWrite_toudata(time_t t2, webs_t wp, const stTou tou,int i,int mtr_no);
int isRightDate(const stTouFilehead  filehead,  struct  tm   t);
int webWriteOneTI(webs_t wp, Ti_Category ti);
#pragma pack()
#endif
