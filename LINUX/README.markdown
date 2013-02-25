使用说明 {#mainpage}
========
goahead web服务器(后端).和前端一起实现通过PC端浏览器管理和维护终端的功能.

### 1.文档
通过 `doxygen` 指令可以生成 `LINUX/html/index.html` (web形式的说明文档).

或者使用

	make doc

生成文档.

### 2.编译

若 `LINUX` 目录下已经存在 `webs` 应用程序文件,则可以直接跳转到下一步(安装)章节.如果缺失需要重新编译,前运行

	make clean

以清除之前残留的临时文件

若不存在 `LINUX/webs` 文件,则在Linux(PC)下交叉编译,执行:
	
	make

或者

	make all

在 `LINUX` 目录得到 `webs` 应用程序.

### 3.安装

1.将 `webs` 通过ftp工具上传到终端 `/mnt/nor/bin` 目录下.

2.在终端 `/mnt/nor/bin` 目录下执行: `chmod +x webs` 给予应用程序执行权限.

### 4.配置

1.将 `conf/goahead.conf` goahead webs服务器配置文件通过ftp工具上传到终端的 `/mnt/nor/conf` 目录下.

2.将 `conf/monparam_name.conf` 网络监视端口名称配置信息文件上传到终端 `/mnt/nor/conf` 目录下.如果有必要,打开并按照文件内容指示修改内容使之与终端端口名称相符.

3.将整个 `wwwdemo` 网页目录通过ftp工具上传到终端 `/mnt/nor` 目录下.(这个目录位于 *另外一个项目* 中,是webs的前端页面).

4.修改看门狗配置文件.(可选)
 
### 运行

在终端执行 `webs` .(运行前务必先执行 4.配置的3操作,上传网页.)

![测试图片](../img/test.png "测试图片")

此时连接终端的计算机即可通过地址 `http://192.168.1.189:8080` 来访问和维护终端.其中192.168.1.189为终端对应网口的IP地址

# 开发者说明
请生成并且查阅: `LINUX/html/index.html` .务必使用等宽字体查看代码.

