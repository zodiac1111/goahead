INCS    =       -I../include -I../../../../include
LIB	= ../../../../lib
LIBS    =
hostCXX	= g++
CXX	= arm-linux-g++
NAME    =       sysparam
SOURCE  =       ./sysparam.cpp ./main.cpp
#预处理 选项
CPPFLAGS = 
#编译器 选项
CXXFLAGS = -freg-struct-return -pipe -g
#连接器 选项
LDFLAGS =
#警告
wflag= -Wall -Wextra

debug:$(NAME)-host
$(NAME)-host:$(SOURCE)
	$(hostCXX)  $(MARCO) $(INCS) $^ -o $@ $(wflag) $(CXXFLAGS)

all:$(NAME)
$(NAME):$(SOURCE)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) $(wflag) $^ -o $@


