错误解决 {#err}
=======

操作系统相关错误

* 1001 终端显示`-sh: webs: Permission denied`<br>
webs文件没有执行权限,参考[安装指南](./index.html)的[安装-1](./index.html#install_1),为 `webs` 程序添加执行权限.

webs服务器相关错误

* 2001 终端显示`open file goahead.conf: No such file or directory`:<br>
检查终端"/mnt/nor/conf/goahead.conf"文件是否存在.检查[安装指南](./index.html)的[配置-1](./index.html#conf_1).

* 2002 终端显示`Open Monitor Port name file ../conf/monparam_name.conf err`:<br>
检查终端"/mnt/nor/conf/monparam_name.conf"文件是否存在.检查[安装指南](./index.html)的[配置-2](./index.html#conf_2).
