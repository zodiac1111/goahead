<html>
<head>
<title>Arm Home</title>
<!--  Copyright (c) Echon., 2006. All Rights Reserved. -->
<meta http-equiv="Pragma" content="no-cache" charset=gb2312>
<link rel="stylesheet" href="/style/normal_ws.css" type="text/css"></link>
<link rel="stylesheet" href="/style/sys.css" type="text/css"></link>
<link href="/style/menuExpandable3.css" rel="stylesheet" type="text/css"></link>
<script src="/style/wwyfunc.js" language='javascript' type='text/JavaScript'>
	
</script>
</head>
<body>
  <br />
  <h1 align="center">
    <img src="/graphics/logo72.png" height="45">
  </h1>
  <form action="/goform/savecycle" method=post ID="paraform" name="paraform">
    <table align=center ID="Table1" border="1" cellspacing="1" cellpadding="1" class="sioplanTableHead" >
      <tr>
        <td width="2%" class="sysTDCLItemStyle">存储周期</td>
        <td width="5%" class="sysTDCLItemStyle">总电量</td>
        <td width="5%" class="sysTDCLItemStyle">分时电量</td>
        <td width="5%" class="sysTDCLItemStyle">相线无功电能</td>
        <td width="5%" class="sysTDCLItemStyle">最大需量</td>
        <td width="5%" class="sysTDCLItemStyle">实时遥测量</td>
        <td width="5%" class="sysTDCLItemStyle">失压断相</td>
      </tr>
      <% savecycle(); %>
    </table>
    <p align="center" >
      <input type=button name="Update" value="设置" ID="Update" OnClick="db_update();">
      <input type=button name=Refresh value="读取" ID="Refresh" OnClick="return RefreshWin();">
      <!-- 提交操作类型 更新,还是其他-->
      <input class=hideinp type=text name=optype value="" id="optype">
    </p>
  </form>
</body>
</html>
