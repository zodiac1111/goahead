#!/bin/sh
# 快速打包发布脚本,被Makefile中的publish目标所使用
# 注意
# 1. /samba_folder 生成的包保存的路径
# 2. '/home/lee/Aptana Studio 3 Workspace/' 前端项目根目录
# 3. '/home/lee/workspace' 后端项目根目录
# 4. '/home/lee/README.txt' 说明文档
# 最后打包成如下结构
# ./goahead/		<-目录
# ./wwwdemo/		<-目录
# ./README.txt		<-文件
major=`cat version.h |grep -i major|awk -F ' ' '{print $3}'`
minor=`cat version.h |grep -i minor|awk -F ' ' '{print $3}'`
patchlevel=`cat version.h |grep -i patchlevel|awk -F ' ' '{print $3}'`
#前端根目录 www
www='/home/zodiac1111/Aptana Studio 3 Workspace/www/'
#gohead  root dir a 
goahead_dir='/home/zodiac1111/workspace/goahead'
#output dir
out='/samba_folder'
#project name : webs
pro='webs'

#cd "$output_dir"
echo "www dir = \"$www \""
echo "goahead_dir = \"$goahead_dir\""


echo "* Go into \"$goahead_dir\" and make update..."
cd "$www" && ./makeversion.sh 

if [ r"$1" = r"dev" ] ; then
	#用于软件开发的版本
	filename=$pro-$major.$minor.$patchlevel-dev.tar.gz 
	echo "Filename:"$filename "正在打包[dev版本](packing)..."
	cd $out \
	&& tar -zcf  $filename \
	       	-C "$www/.." www \
		-C "$goahead_dir/.." goahead \
		-C "$goahead_dir/.." README.txt \
	&& echo -e \
		"\e[32m[成功]\e[0m 生成文件:\e[32m`pwd`/$filename\e[0m"
else
	#用于安装使用的版本
	#使用类似 --exclude="*.[ch]" \ 的语句排除不需要打开的文件.
	filename=$pro-$major.$minor.$patchlevel.tar.gz 
	echo "Filename:"$filename "正在打包[发布版本](packing)..."
	cd $out \
	&& tar --exclude=".*" \
		--exclude="*~" \
		--exclude="*.o" \
		--exclude="*.a" \
		--exclude="*.log" \
	       	-zcf  $filename \
	       	-C "$www/.." www \
		-C "$goahead_dir/.." goahead \
		-C "$goahead_dir/.." README.txt \
	&& echo -e \
		"\e[32m[成功]\e[0m 生成文件:\e[32m`pwd`/$filename\e[0m"
fi
exit 0
