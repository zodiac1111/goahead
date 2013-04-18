#ifndef _TOU_H_
#define _TOU_H_
#include <time.h>
#include "../uemf.h"
#include "../wsIntrn.h"
#include <stdint.h>
#define DATA_DIR "../../nand" //数据目录
#if __i386 == 1
#define TOU_DAT_DIR DATA_DIR"/tou" //电量Tou数据目录
#else
#define TOU_DAT_DIR DATA_DIR
#endif
#define TOU_DAT_SUFFIX "tou" //电量数据文件后缀名
///tou 文件头
typedef struct {
	uint8_t year; //+2000
	uint8_t month;
	uint8_t day;
	uint8_t save_cycle_lo;///<存储周期[分钟]
	uint8_t save_cycle_hi;///<存储周期[分钟]
	uint8_t save_number;
	uint8_t save_flag1;
	uint8_t save_flag2;
	uint8_t save_flag3;
	uint8_t save_flag4;
}  __attribute__((packed, aligned(1))) stTouFilehead;
///某单独电量结构,如 总电量 或者 谷电量
typedef struct {
	///以4个字节型组织的假装的浮点型,注意读取时顺序
	uint8_t fake_float_val[4];
	union{
		uint8_t byte;
		struct{
			uint8_t res:7;///<保留 取0
			uint8_t iv:1;///<有效标志,1-有效
		} __attribute__((packed, aligned(1)));
	} __attribute__((packed, aligned(1)));
}  __attribute__((packed, aligned(1))) Ti;
///tou 总尖峰平谷
typedef struct {
	 Ti total;///<总
	 Ti tip;///<尖
	 Ti peak;///<峰
	 Ti flat;///<平
	 Ti valley;///<谷
}  __attribute__((packed, aligned(1))) Ti_Category;
///四种正反,有无功率.
typedef struct {
	 Ti_Category FA;///<正有
	 Ti_Category RA;///<反有
	 Ti_Category FR;///<正无
	 Ti_Category RR;///<反无
}  __attribute__((packed, aligned(1))) stTou;
///时间跨度,开始时间和结束时间
typedef struct{
	time_t s;
	time_t e;
}  __attribute__((packed, aligned(1))) TimeRange;
//
int load_tou_dat(uint32_t mtr_no,TimeRange const range,stTou* ptou,webs_t wp);
int webWrite1Tou(webs_t wp,const stTou tou);

int isRightDate(const stTouFilehead  filehead,  struct  tm   t);

#endif
