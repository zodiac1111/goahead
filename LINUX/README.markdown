@mainpage
# 说明 
goahead web服务器.实现通过PC端浏览器管理和维护终端的功能.

通过`doxygen`指令可以生成`goahead/html/index.html`(web形式的说明文档).

# 编译安装
###编译
在Linux下交叉编译,执行:
	
	make CC=arm-linux-gcc

在`goahead/LINUX`目录得到`webs`应用程序.

###安装

将`webs`通过ftp工具上传到终端`/mnt/nor/bin`目录下.

在终端`/mnt/nor/bin`目录下执行:`chmod +x webs`给予应用程序执行权限.

###配置

将`goahead/conf/goahead.conf`goahead webs服务器配置文件通过ftp工具上传到终端的`/mnt/nor/conf`目录下.

将整个`wwwdemo`网页目录通过ftp工具上传到终端`/mnt/nor`目录下.(这个目录位于另外一个项目中,是webs的前端页面)

(可选)修改看门狗配置文件.

###运行

在终端执行`webs`.

此时连接终端的计算机即可通过地址`http://192.168.1.189:8080`来访问和维护终端.其中192.168.1.189为终端对应网口的IP地址

#开发者说明

