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