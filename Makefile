#主目录的Makefile文件,用于总体掌控,各项实现均在/LINUX下的Makefile实现.
all:
	make -C ./LINUX all
install:
	make -C ./LINUX install
update:
	make -C ./LINUX update
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
binary:
	/home/zodiac1111/workspace/package-make/mkpkg.sh
#以下用于发布，必须有git软件支持，否则版本号不会更新
#生成版本号	
version:
	./makeversion.sh

#用于打包(发布版 尽量保留少量东西,文档被简化)
publish:clean version all 
	#make -C ./LINUX doc;
	./publish.sh

#用于打包 开发板.去除全部可以由代码生成的文件.
#使用时需要 git awk sed doxygen 这些工具用于重建.
publish-dev:distclean version 
	#make -C ./LINUX doc;
	./publish.sh dev
log:
	echo "生成开发日志 dev.log "
	git log --pretty=format:'[%h] %ad %s' --date=short  > dev.log
