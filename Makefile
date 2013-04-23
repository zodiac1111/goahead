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
#以下用于发布，必须有git软件支持，否则版本号不会更新
#生成版本号	
version:
	./makeversion.sh

log:
	echo "生成开发日志 dev.log "
	git log --pretty=format:'[%h] %ad %s' --date=short  > dev.log
