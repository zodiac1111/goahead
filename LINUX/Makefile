# Makefile for the GoAhead web server reference source base
#  for the Linux OS
# Copyright (c) 华立电力

HL3104_HOME= "/home/zodiac1111/hl3104/trunk/hl3104-v2.3"
#配置头文件,库目录和库文件
my_cc=arm-linux-gcc-4.3.5
my_cxx=arm-linux-g++
include_dir=$(HL3104_HOME)/include
lib_dir=$(HL3104_HOME)/lib
lib_obj = -lsys_utl
lib_obj += -lexternCShm

#用于上传到终端的ip,指定ftp的用户名,密码,ftp上传文件夹路径(需要lftp支持,CentOS自带)
#老版本root,linux,ftp服务器根目录需要注意,可能是 / /mnt /mnt/nor 这几种
rtu_ip_for_update=192.168.1.189 
username=anonymous
passwd=holley
update_dir=nor/bin/
build_dir=build
# This is set by the debug target below 设置发布版和开发版区别(现在耦合,实时数据)
ifdef WEBS_DEBUG
DFLAGS  = -g -Wall -Wextra \
	-Winline -Wdisabled-optimization -DDEBUG \
	 -Wno-unused-parameter \
	 -Wunreachable-code  -Wno-empty-body\
	# -Wshadow
else
CC=$(my_cc)
CXX=$(my_cxx)
endif
DFLAGS  = -Os 
DFLAGS  += -Wall -Wno-unused-parameter

default:  gold

all: gold

clean:
	rm -f $(NAME) $(ARCH) $(DEPEND_FILES)
	rm -f $(NAME).update
	rm -f err.log log.txt trace.txt ../trace.txt 
	rm -f ../log.txt 
distclean:clean
	rm -rf html
	rm -rf ../version.h
	rm -rf *~ ../*~
	
#文档(简化的文档,主要给使用人员阅读)	
doc-simple::
	doxygen Doxyfile
#给开发人员的更详细的文档
doc::
	doxygen Doxyfile-dev

ARCH	= libwebs.a
NAME	= webs
# User Management switch
#UMSW	= -DUSER_MANAGEMENT_SUPPORT

# Digest Access switch
DASW	= -DDIGEST_ACCESS_SUPPORT

debug:
	@$(MAKE) compile "WEBS_DEBUG = 1"

gold:
	echo $GCC_VERSION_3_3_2
	@$(MAKE) compile


#   Enable SSL build if MatrixSSL package found in root source directory
#   (if automation is not working, simply define SSLINC, SSLLIB, and SSLSW)
matrixsslDir:=$(shell ls -d ../matrixssl-3-1*/)
ifdef matrixsslDir
SSLINC = $(matrixsslDir)
SSLLIB = $(matrixsslDir)libmatrixssl.a
SSLSW = -DWEBS_SSL_SUPPORT -DPOSIX -I$(SSLINC)
OPT_FILES = ../matrixSSLSocket.o ../websSSL.o
else
OPT_FILES = ../md5c.o
endif

# If-Modified-Support switches (requires math library, libm.a)
# IFMODSW = -DWEBS_IF_MODIFIED_SUPPORT
# IFMODLIB = /usr/lib/libm.a
#本实例需要的依赖
my_obj	=	main.o src/param.o src/web_err.o src/tou.o src/json.o \
	       	src/rtit.o src/wpsend.o src/autoUpdate.o
# Dependencies
DEPEND_FILES	= $(OPT_FILES) ../asp.o ../balloc.o ../mime64.o \
		../cgi.o ../default.o  ../ejlex.o ../ejparse.o ../form.o \
		../h.o ../handler.o ../mime.o ../misc.o ../page.o  \
	 	../ringq.o ../rom.o  ../sock.o ../sockGen.o \
		../security.o ../sym.o ../uemf.o ../url.o ../value.o \
		../um.o ../umui.o ../websda.o ../emfdb.o \
		../webrom.o ../webs.o ../websuemf.o $(my_obj)
	       	
CFLAGS	= -DWEBS -DOS="LINUX" -DLINUX $(UMSW) $(DASW) $(SSLSW) $(IFMODSW) -D_arm_cplusplus
CXXFLAGS	= -DWEBS -DOS="LINUX" -DLINUX $(UMSW) $(DASW) $(SSLSW) $(IFMODSW) -D_arm_cplusplus
OTHERS	= -DB_STATS -DB_FILL 
IFLAGS 	=  -I.. -I. -Iinclude -I$(include_dir)

#链接
LDFLAGS	= $(SSLLIB) $(IFMODLIB) -L$(lib_dir) $(lib_obj)  

compile: $(NAME)

#	Build archive of objects
$(ARCH): $(DEPEND_FILES)
	$(AR) $(ARFLAGS) $(ARCH) $?

#	Primary link
$(NAME): Makefile $(ARCH)
	$(CC) -o $(NAME) $(DFLAGS) \
		$(CFLAGS) $(IFLAGS) \
		 $(ARCH) $(LDFLAGS) 
	#复制一份用于升级
	cp $(NAME) $(NAME).update 

# 安装即上传到终端指定目录
install:all
	lftp -u $(username),$(passwd) $(rtu_ip_for_update) \
	-e "put $(NAME)  -o $(update_dir);quit" && \
	echo -e "\033[32mSuccess\033[0m"
#升级,原来已经有webs的情况下,上传.update文件可以,之后重启,可以自动升级.
update:all
	lftp -u $(username),$(passwd) $(rtu_ip_for_update) \
	-e "put $(NAME).update  -o $(update_dir);quit" && \
	echo -e "\033[32mSuccess\033[0m"

# 依赖(webs的)
main.o:  main.c ../wsIntrn.h ../webs.h ../ej.h ../ejIntrn.h ../uemf.h \
	include/web_err.h include/conf.h ../version.h
src/param.o: src/param.c include/param.h include/web_err.h include/conf.h
src/autoUpdate.o :src/autoUpdate.c include/autoUpdate.h
src/web_err.o: src/web_err.c include/web_err.h include/conf.h
src/tou.o: src/tou.c include/tou.h include/conf.h
src/json.o: src/json.c include/json.h
src/rtit.o: include/rtit.h src/rtit.c
src/wpsend.o:src/wpsend.c include/wpsend.h
# Dependencies(goahead本身的)
../asp.o:  ../webs.h ../wsIntrn.h ../ej.h ../ejIntrn.h ../uemf.h
../balloc.o: ../balloc.c ../uemf.h
../mime64.o:  ../mime64.c ../webs.h ../wsIntrn.h  ../ej.h ../ejIntrn.h ../uemf.h
../cgi.o:  ../webs.h ../wsIntrn.h ../uemf.h
../default.o:  ../default.c ../webs.h ../wsIntrn.h ../ej.h ../ejIntrn.h ../uemf.h
../ejlex.o:  ../ejlex.c ../ej.h ../ejIntrn.h ../uemf.h
../ejparse.o:  ../ejparse.c ../ej.h ../ejIntrn.h ../uemf.h
../emfdb.o:  ../emfdb.h ../wsIntrn.h ../uemf.h
../form.o:  ../form.c ../webs.h ../wsIntrn.h ../ej.h ../ejIntrn.h ../uemf.h
../h.o:  ../h.c ../uemf.h
../handler.o:  ../handler.c ../webs.h ../wsIntrn.h ../ej.h ../ejIntrn.h ../uemf.h
../md5c.o:  ../md5.h ../wsIntrn.h ../uemf.h
../mime.o:  ../mime.c ../webs.h ../wsIntrn.h ../ej.h ../ejIntrn.h ../uemf.h
../misc.o:  ../misc.c ../uemf.h
../page.o:  ../page.c ../webs.h ../wsIntrn.h ../ej.h ../ejIntrn.h ../uemf.h
../ringq.o:  ../ringq.c ../uemf.h
../rom.o:  ../rom.c ../webs.h ../wsIntrn.h ../ej.h ../ejIntrn.h ../uemf.h
../security.o:  ../security.c ../webs.h ../wsIntrn.h ../ej.h ../ejIntrn.h ../uemf.h
../sock.o:  ../sock.c ../uemf.h
../sockGen.o:  ../sockGen.c ../uemf.h
../sym.o:  ../sym.c ../uemf.h
../uemf.o:  ../uemf.c ../uemf.h
../um.o:  ../webs.h ../wsIntrn.h ../um.h ../uemf.h
../umui.o:  ../webs.h ../wsIntrn.h ../um.h ../uemf.h
../url.o:  ../url.c ../webs.h ../wsIntrn.h ../ej.h ../ejIntrn.h ../uemf.h
../value.o:  ../value.c ../uemf.h
../webrom.o:  ../webrom.c ../webs.h ../wsIntrn.h ../uemf.h
../webs.o:  ../webs.c ../webs.h ../wsIntrn.h ../ej.h ../ejIntrn.h ../uemf.h
../websda.o:  ../webs.h ../wsIntrn.h ../websda.h ../uemf.h
../websuemf.o:  ../websuemf.c ../webs.h ../wsIntrn.h ../ej.h ../ejIntrn.h ../uemf.h
../websSSL.o:  ../websSSL.c ../websSSL.h ../wsIntrn.h ../ej.h ../ejIntrn.h ../uemf.h
../matrixSSLSocket.o:  ../matrixSSLSocket.c ../matrixSSLSocket.h

#	Transition rules (add -o to put object in right directory)
.c.o:
	$(CC) -c -o $@ $(DFLAGS) $(CFLAGS) $(IFLAGS) $<
