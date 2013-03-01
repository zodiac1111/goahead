/**
 * @file color.h
 * 定义终端控制颜色/特效,用于更醒目的显示调试/错误等信息
 * 只要在要显示颜色的字符串两头添加<颜色><_COLOR>即可
 * 如:
 * 	printf("这是"RED"红色"_COLOR"显示");
 * 	printf("这是"UNDERLINE"下划线"_COLOR"特效");
 * 	printf("这是"RED UNDERLINE"红色下划线"_COLOR"特效显示");
 * 参考:
 * 	<http://basiccoder.com/output-colorful-words-in-terminal.html>
 */
#ifndef _COLOR_H_
#define RED "\e[31m" //颜色
#define GREEN "\e[32m"
#define YELLOW "\e[33m"
#define UNDERLINE "\033[4m" //下划线效果
#define _COLOR "\e[0m" //一次性关闭所有颜色和特效 类似xml的标签闭合.不需嵌套
#define _COLOR_H
#endif
