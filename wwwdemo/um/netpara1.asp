<html>
<HEAD>
<title>网络参数设置</title>
<!-- Copyright (c) Echon., 2006. All Rights Reserved. -->
<meta http-equiv="Pragma" content="no-cache" charset=gb2312>
<link rel="stylesheet" href="/style/normal_ws.css" type="text/css"></link>
<link rel="stylesheet" href="/style/sys.css" type="text/css"></link>
<link href="/style/menuExpandable3.css" rel="stylesheet" type="text/css"></link>
<script src="/style/wwyfunc.js" language='javascript' type='text/JavaScript'>
	
</script>
<% init_sysparam(); //初始化系统参数,获得例如表计个数,串口个数,串口方案个数等参数 %>
</HEAD>
<body>
  <br />
  <h1 align="center">
    <img src="/graphics/logo42.png" height="45">
  </h1>
  <form action="/goform/netpara" method=post name="paraform">
    <table class="sioplanTable" border="1">
      <tr>
        <th>网口号</th>
        <th>使用网口</th>
        <th>IP地址</th>
        <th>子网掩码</th>
        <th>网关</th>
      </tr>
      <% get_netparams(); %>
    </table>
    <p ALIGN="center">
      <input type=button name="Update" value="设置" ID="Update" OnClick="db_update();">
      <!--<input type=button name=bDelItem value="删除" ID="bDelItem"  onclick="return DelSubmit();">
	<input type=button  name=bAddItem value="添加" ID="bAddItem" OnClick="return Redirect('AddRoutePara.asp');" >
      -->
      <input type=button name=Refresh value="读取" ID="Refresh" OnClick="return RefreshWin();">
      <!-- 提交操作类型 更新,还是其他 -->
      <input class=hideinp type=text name=OpType value="" id="optype">
    </p>
  </form>
</body>
</html>
