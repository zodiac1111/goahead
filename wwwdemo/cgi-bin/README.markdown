#封装类,通过文件读/写参数.

para文件夹为参数文件夹,实例化类构造函数需要提供这个
文件夹的路径.需要知道参数意义和结构.(和文件保存时一致,耦合)

sysparam.cpp 类的实现
main.cpp是使用这个类的一个示例.

1. `make debug` 生成sysparam-host,在host平台运行的例子
2. `make [all]` 生成sysparam 在target平台上的例子,使用arm-linux-g++编译
	需要相应的para文件在适当的目录.
