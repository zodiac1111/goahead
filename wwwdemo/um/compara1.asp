<html>
<HEAD>
<title>Arm Home</title>
<!--  Copyright (c) Echon., 2006. All Rights Reserved. -->
<meta http-equiv="Pragma" content="no-cache" charset=gb2312>
<link rel="stylesheet" href="/style/normal_ws.css" type="text/css"></link>
<link rel="stylesheet" href="/style/sys.css" type="text/css"></link>
<link href="/style/menuExpandable3.css" rel="stylesheet" type="text/css"></link>
<script src="/style/wwyfunc.js" language='javascript' type='text/JavaScript'>
	
</script>
<!-- jquery -开始-->
<link rel="stylesheet" media="all" type="text/css" href="/style/jquery-ui.css" />
<link rel="stylesheet" media="all" type="text/css" href="/style/jquery-ui-timepicker-addon.css" />
<script type="text/javascript" src="/style/jquery-1.8.3.min.js"></script>
<script type="text/javascript" src="/style/jquery-ui.min.js"></script>
<script type="text/javascript" src="/style/jquery-ui.min-zh-CN-GB2312.js"></script>
<!-- jquery的 -结束 -->
<% init_sysparam(); //初始化系统参数,获得例如表计个数,串口个数,串口方案个数等参数 %>
<script type="text/javascript">
	$(document).ready(
			function() {
				$.post('/goform/sioplan', $("#paraform").serialize()
						+ "&init=1", function(result) {
					$("#tbody_dat").html(result);
					$("#tbody_dat tr").mouseover(function() {
						$(this).addClass("over");
						var rows = $(this).attr('relrow');
					});
					$("#tbody_dat tr").mouseout(function() {
						$(this).removeClass("over");
					});
					//alert("OK");
					//$("#tip").html("完成");
				});
				$("#btnsio").click(function() {
					$.post('/goform/sioplan',
					//$.param($("#paraform")),
					$("#paraform").serialize(), function(result) {
						$("#tbody_dat").html(result);
						$("#tbody_dat tr").mouseover(function() {
							$(this).addClass("over");
							var rows = $(this).attr('relrow');
						});
						$("#tbody_dat tr").mouseout(function() {
							$(this).removeClass("over");
						});
						alert("OK");
						//$("#tip").html("完成");
					});
				});
			});
</script>
</head>
<body>
  <h1 align="center">
    <img src="/graphics/logo32.png" height="45">
  </h1>
  <form action="/goform/sioplan" method=post ID="paraform" name="paraform">
    <table ID="Table1" border="1" cellspacing="1" cellpadding="1" class="sioplanTable">
      <thead>
        <tr>
          <th>串口方案号</th>
          <th>校验位</th>
          <th>数据位</th>
          <th>停止位</th>
          <th>波特率</th>
          <th>通讯方式</th>
        </tr>
      </thead>
      <!-- 循环依次添加所有串口方案 一列一条  -->
      <tbody id=tbody_dat>
      </tbody>
    </table>
  </form>
  <p align="center" height=25>
    <input type=button name="Update" value="设置" ID="Update" OnClick="db_update();">
    <!--
		<input type=button name=bDelItem value="删除" ID="bDelItem"  onclick="return DelSubmit();">
		<input type=button  name=bAddItem value="添加" ID="bAddItem" OnClick="return Redirect('AddRoutePara.asp');" >
		-->
    <input type=button name=Refresh value="读取" ID="Refresh" OnClick="return RefreshWin();">
    <!- 提交操作类型 更新,还是其他->
    <input class=hideinp type=text name=OpType value="" id="optype">
    <button id=btnsio>更新</button>
  </p>
</body>
</html>
