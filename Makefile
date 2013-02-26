#主目录的Makefile文件,用于总体掌控,各项实现均在/LINUX下的Makefile实现.
default:
	make -C ./LINUX
all:
	make -C ./LINUX all
debug:
	make -C ./LINUX debug
clean:
	make -C ./LINUX clean
doc:
	make -C ./LINUX doc

distclean:
	make -C ./LINUX distclean
	
#生成版本号	
version:
	./makeversion.sh

#用于打包(发布版 保留一些有git生成的东西)
publish:version  all
	./publish.sh 
#用于打包 开发板.去除全部可以由代码生成的文件.
#使用时需要 git awk sed doxygen 这些工具用于重建.
publish-dev:distclean
	./publish.sh dev
log:
	echo "生成开发日志 dev.log "
	git log --pretty=format:'[%h] %ad %s' --date=short  > dev.log