安装指南 {#mainpage}
========
goahead web服务器(后端).和前端一起实现通过PC端浏览器管理和维护终端的功能.

####步骤表

在已经完成的步骤前面打勾 ☑ 。
任何时候发生错误都可以参考[错误解决](@ref err)来解决错误。

* ☑ [生成并且查看文档](#doc)(可略过)
* ☑ [编译webs服务器](#compile)(可略过)
* ☐ [安装webs](#install)
* ☐ [配置webs](#conf)
* ☐ [上传www目录](#www)
* ☐ [启动webs](#run)
* ☐ 将用于维护的个人电脑与终端连接.使之能 `ping` 通终端的IP
* ☐ [在浏览器输入地址,进入管理界面](#use)

<a name="doc"></a>
### 文档 
<b>如果您现在正在浏览器中查看本文档,那么就可以直接跳转到下一步[编译](#compile)了。</b>

通过 `doxygen` 指令可以生成 `<解压目录>/goahead/LINUX/html/index.html` (web形式的说明文档)。执行

	make doc

指令生成文档(需要 `doxygen` 软件支持)。

如果文档已经存在，则不必再次生成。

<a name="compile"></a>
### 编译

<b>若已经存在 `<解压目录>/goahead/LINUX/webs` 应用程序文件,则可以直接跳转到下一步[安装](#install)。</b><br>

如果确实需要重新编译，在LinuxPC上执行

	make clean

以清除之前残留的临时文件

若不存在 `<解压目录>/gohead/LINUX/webs` 文件，则必须在Linux(PC)下交叉编译需要(arm-linux-gcc)，执行:

	make all

在 `LINUX` 目录得到 `webs` 应用程序。

<a name="install"></a>
### 安装

<a name="install_1"></a>
1.将 `<解压目录>/goahead/LINUX/webs` 应用程序文件通过ftp工具上传到终端 `/mnt/nor/bin` 目录下。

如果 `webs` 正在运行则会上传失败。必须先使用telnet登录到终端,运行 `killall webs` 命令来结束进程。再重新上传即可成功。

<a name="install_2"></a>
2.在终端执行： `chmod +x /mnt/nor/bin/webs` 给予应用程序可执行权限。

<a name="conf"></a>
### 配置

<a name="conf_1"></a>
1.将 `<解压目录>/goahead/conf/goahead.conf` webs服务器配置文件通过ftp工具上传到终端的 `/mnt/nor/conf` 目录下。

<a name="conf_2"></a>
2.1.根据终端类型,将 `<解压目录>/goahead/conf/monparam_name_HA.conf` <b>或</b> `<解压目录>/goahead/conf/monparam_name_JD.conf` <b>重命名</b>为 `conf/monparam_name.conf`。

2.2.将 `<解压目录>/goahead/conf/monparam_name.conf` 网络监视端口名称配置信息文件上传到终端 `/mnt/nor/conf` 目录下。如果有必要，打开并按照文件内容指示修改内容使之与终端端口名称相符。

<a name="www"></a>
3.将整个 `<解压目录>/www` 网页目录通过ftp工具上传到终端 `/mnt/nor` 目录下.(该目录是webs的前端项目目录)。

4.修改看门狗配置文件。修改终端 `/mnt/nor/conf/watchdog.conf` 看门狗配置文件，追加 

	once  /mnt/nor/bin/webs webs

一行。这样每次重启终端后服务器将会自动运行。	

<a name="run"></a>
### 运行服务器

在终端执行(运行前务必正确完成配置的操作3：[上传网页](#www))：

	/mnt/nor/bin/webs 

如下图所示即表示已经成功运行。若发生错误请参考[错误解决](@ref err)页面。

<img src="../img/webs_start.png"  alt="webs启动" width=80% />

此时使用PC机通过网络(路由器)连接终端。确保PC能够ping通终端。否则请联系网络管理员。

<a name="use"></a>
### 使用

打开浏览器输入地址 `http://192.168.1.189:80` 来访问和维护终端(如下图)。其中：
* 192.168.1.189为终端对应网口的IP地址。<b>应改为实际IP</b>
* 80 为默认维护端口。<b>80端口可省略，如果使用其他端口则不可省略</b>

如果维护端口为80，那么端口号在浏览器地址栏中时可以省略的，即只要输入：

	http://192.168.1.189

即可。

在浏览器中显示类似如下界面：

<img src="../img/web_mainpage.png"  alt="web维护首页" width=80% />

同时终端服务器能看到一些提示信息:

<img src="../img/webs_run.png"  alt="服务器响应" width=80% />

即安装完成。


## 开发者说明
请生成并且查阅：[开发说明](@ref dev)。务必使用等宽字体查看代码。

