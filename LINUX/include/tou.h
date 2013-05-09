#ifndef _TOU_H_
#define _TOU_H_
#include <time.h>
#include "../uemf.h"
#include "../wsIntrn.h"
#include "json.h"
#include "struct.h"
#include <stdint.h>
#include "wpsend.h"
#define DATA_DIR "../../nand" //数据目录
#if __i386 == 1
#define TOU_DAT_DIR DATA_DIR"/tou" //电量Tou数据目录
#else
#define TOU_DAT_DIR DATA_DIR
#endif
#define TOU_DAT_SUFFIX "tou" //电量数据文件后缀名
#define QR_DAT_SUFFIX "qr" //无功数据文件后缀名
#define INSTANT_DAT_SUFFIX "ta" //瞬时量数据文件后缀名
#define MAXN_DAT_SUFFIX "mnt" //最大需量数据文件后缀名
#define DEBUG_PRINT_HISTORY_DAT_MAXN 1 //最大需量打印测试
typedef enum {
	etou=1,
	eqr,
	einstant,
	emaxn
}eDatetype;
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
}  __attribute__((packed, aligned(1)))stQrFilehead;
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
}  __attribute__((packed, aligned(1)))stInstantFilehead;
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
}  __attribute__((packed, aligned(1))) stMaxnFilehead;
///某单独电量结构,如 总电量 或者 谷电量
typedef struct {
	///以4个字节型组织的假装的浮点型,注意读取时顺序
	uint8_t fake_float_val[4];
	union{
		uint8_t byte;
		struct{
			uint8_t :7;///<保留 取0
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
}  __attribute__((packed, aligned(1))) touTi_Category;
///四种正反,有无功率.
typedef struct {
	 touTi_Category FA;///<正有
	 touTi_Category RA;///<反有
	 touTi_Category FR;///<正无
	 touTi_Category RR;///<反无
}  __attribute__((packed, aligned(1))) stTou;
///四象限无功,注意文件中数据保存的顺序
typedef struct {
	 touTi_Category ph1;///
	 touTi_Category ph4;///
	 touTi_Category ph2;///
	 touTi_Category ph3;///
}  __attribute__((packed, aligned(1))) stQr;
///一个瞬时量文件结构
typedef struct {
	Ti v[PHASENUM];
	Ti i[PHASENUM];
	Ti p[PQCNUM];
	Ti q[PQCNUM];
	Ti pf[PQCNUM];
	Ti f;
}  __attribute__((packed, aligned(1))) stInstant;
typedef struct {
	uint8_t min;
	uint8_t hour;
	uint8_t day;
	uint8_t month;
	uint8_t year;
} __attribute__((packed, aligned(1))) stTime;
//一个最大需量数据结构共10个字节
typedef struct {
	uint8_t fake_float_val[4];
	//float ti;///
	stTime time;
	union{
		uint8_t byte;
		struct{
			uint8_t :7;///<等待主程序使用,暂时未用
			uint8_t iv:1;///<有效标志,1-有效
		} __attribute__((packed, aligned(1)));
	} __attribute__((packed, aligned(1)));
}  __attribute__((packed, aligned(1))) stMaxn_ti;
typedef struct {
	stMaxn_ti total;///<总
	stMaxn_ti tip;///<尖
	stMaxn_ti peak;///<峰
	stMaxn_ti flat;///<平
	stMaxn_ti valley;///<谷
}  __attribute__((packed, aligned(1))) stMaxn_Ti_Category;
typedef struct {
	stMaxn_Ti_Category FA;///<正有
	stMaxn_Ti_Category RA;///<反有
	stMaxn_Ti_Category FR;///<正无
	stMaxn_Ti_Category RR;///<反无
}  __attribute__((packed, aligned(1))) stMaxn;
///时间跨度,开始时间和结束时间
typedef struct{
	time_t s;
	time_t e;
}  __attribute__((packed, aligned(1))) TimeRange;
//
extern void form_history_tou(webs_t wp, char_t *path, char_t *query);





#endif
