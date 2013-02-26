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
# ./说明.txt		<-文件
major=`cat version.h |grep -i major|awk -F ' ' '{print $3}'`
minor=`cat version.h |grep -i minor|awk -F ' ' '{print $3}'`
patchlevel=`cat version.h |grep -i patchlevel|awk -F ' ' '{print $3}'`
#
filename=web-release-$major.$minor.$patchlevel.tar.bz2 
echo "filename:"$filename
#exit -1
cd /samba_folder \
&& tar --exclude .git* -jcvf  $filename \
       	-C '/home/lee/Aptana Studio 3 Workspace/' wwwdemo  \
	-C '/home/lee/workspace' goahead \
	-C '/home/lee/文档' 说明.txt \
&& echo -e \
"\e[32m[成功]\e[0m 生成文件:\e[31m`pwd`/$filename\e[0m"
