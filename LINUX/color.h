/**
 * @file color.h
 * 定义终端控制颜色,用于更醒目的显示调试/错误等信息
 * 只要在要显示颜色的字符串两头添加<颜色><_COLOR>即可
 * 如:
 * 	printf("这是"RED"红色"_COLOR"显示");
 */
#ifndef _COLOR_H_
#define RED "\e[31m"
#define GREEN "\e[32m"
#define YELLOW "\e[33m"
#define _COLOR "\e[0m"
#define _COLOR_H
#endif
