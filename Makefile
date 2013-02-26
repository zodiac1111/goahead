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
version:
	./makeversion.sh
publish:version
	./publish.sh 
log:
	echo "生成开发日志 dev.log "
	git log --pretty=format:'[%h] %ad %s' --date=short  > dev.log