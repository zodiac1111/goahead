错误解决 {#err}
=======

操作系统相关错误

* 1001 `-sh: webs: Permission denied`<br>
webs文件没有执行权限,参考[安装](@ref mainpage)的步骤2,为 `webs` 程序添加执行权限.

webs服务器相关错误

* 2001 `open file goahead.conf: No such file or directory`:<br>
检查终端"/mnt/nor/conf/goahead.conf"文件是否存在.检查[配置](@ref mainpage)的步骤1.

* 2002 `Open Monitor Port name file ../conf/monparam_name.conf err`:<br>
检查终端"/mnt/nor/conf/monparam_name.conf"文件是否存在.检查[配置](@ref mainpage)的步骤2.
