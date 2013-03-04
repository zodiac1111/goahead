#主目录的Makefile文件,用于总体掌控,各项实现均在/LINUX下的Makefile实现.
default:
	make -C ./LINUX gold
all:
	make -C ./LINUX all
debug:
	make -C ./LINUX debug
clean:
	make -C ./LINUX clean
doc-simple:
	make -C ./LINUX doc-simple
doc:
	make -C ./LINUX doc

distclean:
	make -C ./LINUX distclean
	
#生成版本号	
version:
	./makeversion.sh

#用于打包(发布版 尽量保留少量东西,文档被简化)
publish:clean all doc
	./publish.sh dev
#用于打包 开发板.去除全部可以由代码生成的文件.
#使用时需要 git awk sed doxygen 这些工具用于重建.
publish-dev:distclean doc
	./publish.sh dev
log:
	echo "生成开发日志 dev.log "
	git log --pretty=format:'[%h] %ad %s' --date=short  > dev.log
