使用说明 {#mainpage}
========
goahead web服务器(后端).和前端一起实现通过PC端浏览器管理和维护终端的功能.

####步骤表

在已经完成的步骤前面打勾 ☑ .
任何时候发生错误都可以参考[错误解决](@ref err)来解决错误.

* ☐ [生成并且查看文档](#doc)(生成可略过)
* ☐ [编译webs服务器](#compile)(可略过)
* ☐ [安装webs](#install)
* ☐ [配置webs](#conf)
* ☐ [上传wwwdemo目录](#www)
* ☐ [启动webs](#run)
* ☐ 将用于维护的个人电脑与终端连接.使之能 `ping` 通终端的IP
* ☐ [在浏览器输入地址,进入管理界面](#use)

<a name="doc"></a>
### 文档 
<b>如果您现在正在浏览器中查看本文档,那么就可以直接跳转到下一步[编译](#compile)了.</b><

通过 `doxygen` 指令可以生成 `LINUX/html/index.html` (web形式的说明文档).执行

	make doc

指令生成文档(需要 `doxygen` 软件支持).

如果已经存在,则不必再次生成.

<a name="compile"></a>
### 编译

<b>若 `LINUX` 目录下已经存在 `webs` 应用程序文件,则可以直接跳转到下一步[安装](#install)</b></br>

如果确实需要重新编译,前运行

	make clean

以清除之前残留的临时文件

若不存在 `LINUX/webs` 文件,则必须在Linux(PC)下交叉编译需要(arm-linux-gcc),执行:

	make all

在 `LINUX` 目录得到 `webs` 应用程序.

<a name="install"></a>
### 安装

1.将 `webs` 通过ftp工具上传到终端 `/mnt/nor/bin` 目录下.如果 `webs` 正在运行则必须先运行 `killall webs` 以结束进程.

2.在终端 `/mnt/nor/bin` 目录下执行: `chmod +x webs` 给予应用程序执行权限.

<a name="conf"></a>
### 配置

1.将 `conf/goahead.conf` goahead webs服务器配置文件通过ftp工具上传到终端的 `/mnt/nor/conf` 目录下.

2.将 `conf/monparam_name.conf` 网络监视端口名称配置信息文件上传到终端 `/mnt/nor/conf` 目录下.如果有必要,打开并按照文件内容指示修改内容使之与终端端口名称相符.

<a name="www"></a>
3.将整个 `wwwdemo` 网页目录通过ftp工具上传到终端 `/mnt/nor` 目录下.(这个目录位于 *另外一个项目* 中,是webs的前端页面).

4.修改看门狗配置文件.(可选)

<a name="run"></a>
### 运行服务器

在终端执行 `webs` .(运行前务必先执行配置的操作3:[上传网页](#www).)

<img src="../img/webs_start.png"  alt="webs启动" width=80% />

如上图所示即表示已经运行.

此时使用PC机通过网络(路由器)连接终端.

<a name="use"></a>
### 使用

打开浏览器输入地址 `http://192.168.1.189:8080` 来访问和维护终端(如下图).其中:
* 192.168.1.189为终端对应网口的IP地址.
* 8080 为默认维护端口.

<img src="../img/web_mainpage.png"  alt="web维护首页" width=80% />

此时终端服务器能看到一些提示信息:

<img src="../img/webs_run.png"  alt="服务器响应" width=80% />




## 开发者说明
请生成并且查阅:[开发说明](@ref dev).务必使用等宽字体查看代码.

