#!/bin/sh
# 快速打包发布脚本
# 注意
# 1. /samba_folder 生成的包保存的路径
# 2. '/home/lee/Aptana Studio 3 Workspace/' 前端项目根目录
# 3. '/home/lee/workspace' 后端项目根目录
# 4. '/home/lee/文档/说明.txt' 说明文档
# 最后打包成如下结构
# ./goahead/		<-目录
# ./wwwdemo/		<-目录
# ./说明.txt		<-文件
echo $1

cd /samba_folder \
&& tar --exclude .git* -jcvf  webs-release-$1.tar.bz2 \
       	-C '/home/lee/Aptana Studio 3 Workspace/' wwwdemo  \
	-C '/home/lee/workspace' goahead \
	-C '/home/lee/文档' 说明.txt \
&& echo -e "\e[32m[成功]\e[0m 生成文件:\e[31m`pwd`/webs-$1.tar.gz\e[0m"
