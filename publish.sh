#!/bin/sh
# 快速打包发布脚本,被Makefile中的publish目标所使用
# 注意
# 1. /samba_folder 生成的包保存的路径
# 2. '/home/lee/Aptana Studio 3 Workspace/' 前端项目根目录
# 3. '/home/lee/workspace' 后端项目根目录
# 4. '/home/lee/文档/说明.txt' 说明文档
# 最后打包成如下结构
# ./goahead/		<-目录
# ./wwwdemo/		<-目录
# ./README.txt		<-文件
major=`cat version.h |grep -i major|awk -F ' ' '{print $3}'`
minor=`cat version.h |grep -i minor|awk -F ' ' '{print $3}'`
patchlevel=`cat version.h |grep -i patchlevel|awk -F ' ' '{print $3}'`
#
#echo $1

if [ r"$1" = r"dev" ] ; then
#用于软件开发的版本
	filename=webs-dev-$major.$minor.$patchlevel.tar.bz2 
	echo "Filename:"$filename "正在打包[dev版本](packing)..."
	cd /samba_folder \
	&& tar -jvcf  $filename \
       		-C '/home/lee/Aptana Studio 3 Workspace/' wwwdemo  \
		-C '/home/lee/workspace' goahead \
		-C '/home/lee/workspace' README.txt \
	&& echo -e \
		"\e[32m[成功]\e[0m 生成文件:\e[31m`pwd`/$filename\e[0m"
else
#用于安装使用的版本
#使用类似 --exclude="*.[ch]" \ 的语句排除不需要打开的文件.
	filename=webs-$major.$minor.$patchlevel.tar.bz2 
	echo "Filename:"$filename "正在打包[发布版本](packing)..."
	cd /samba_folder \
	&& tar --exclude=".*" \
		--exclude="*~" \
		--exclude="*.o" \
		--exclude="*.a" \
		--exclude="*.log" \
	       	-jvcf  $filename \
	       	-C '/home/lee/Aptana Studio 3 Workspace/' wwwdemo \
		-C '/home/lee/workspace' goahead \
		-C '/home/lee/workspace' README.txt \
	&& echo -e \
		"\e[32m[成功]\e[0m 生成文件:\e[31m`pwd`/$filename\e[0m"
fi
exit 0
#下面的暂时保留,但是不使用.
echo "Filename:"$filename "正在打包(packing)..."
cd /samba_folder \
&& tar --exclude .git* -jcf  $filename \
       	-C '/home/lee/Aptana Studio 3 Workspace/' wwwdemo  \
	-C '/home/lee/workspace' goahead \
	-C '/home/lee/workspace' README.txt index.html \
&& echo -e \
"\e[32m[成功]\e[0m 生成文件:\e[31m`pwd`/$filename\e[0m"
