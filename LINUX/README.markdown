# 说明 

goahead web服务器,运行在Linux下.经测试可在x86和arm架构下运行.
在x86架构Linux系统下交叉编译.
## 文件
描述一些文件的意义.
### webs
可执行应用程序文件.服务器程序. 后端程序.
服务器二进制文件仅和配置文件 goahead.conf 相关联. 其指示了服务器www跟目录的位置.如:/mnt/nor/wwwdemo

### wwwdemo
wwwdemo,前端代码,css html js.跟目录写在goahead.conf文件中.默认/mnt/nor/wwwdemo.
已经单独分出来一个项目仓库用于编译前端相关的业务逻辑.查看wwwdemo项目.

### err.log
服务器错误日志,自动生成于<del>webs</del> www根目录下目录下.不超过指定大小.

### 特别的文件编码
*Chinese_string.h*文件是GB2312编码的中文字符串,其他文件默认都是UTF-8编码的.

为了在网页(GB2312)中正确显示中文的文字信息.包括一般填充的如"方案"等文字和错误信息.
其他文件均UTF8,方便web服务器后台打印中文的调试/错误信息.
##编译/安装
###编译

编译本机版本:make
编译arm版本:make CC=arm-linux-gcc

###安装
将webs放到任意目录.
goahead.conf配置文件放在/mnt/nor/conf目录下.
文件内写明www根目录的绝对路径,如:/mnt/nor/wwwdemo等.

###运行

执行webs可执行文件.
访问该ip的8080(默认)端口.通过http协议.浏览器中输入

	http://<IP>:8080

ip替换成webs服务器运行的计算机ip.

