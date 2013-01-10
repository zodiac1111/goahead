<html>
<HEAD>
<title>Arm Home</title>
<!- Copyright (c) Echon., 2006. All Rights Reserved. ->
<meta http-equiv="Pragma" content="no-cache" charset=gb2312>
<link rel="stylesheet" href="/style/normal_ws.css" type="text/css"></link>
<link rel="stylesheet" href="/style/sys.css" type="text/css"></link>
<link href="../style/menuExpandable3.css" rel="stylesheet" type="text/css"></link>
<script src="../style/wwyfunc.js" language='javascript' type='text/JavaScript'>
	
</script>
<script type="text/javascript" src="/style/jquery-1.8.3.min.js"></script>
<script type="text/javascript" src="/style/clone_tableheader.js"></script>
<script type="text/javascript">
	//页面初始化 加载 JS函数	
	$(document).ready(cloneTableHeader_Width);

	//调整 浏览器 表格的显示宽度  以及 调用  锁定 表头和列  的JS函数。
	function cloneTableHeader_Width() {
		//document.body.clientWidth获得客户区域(浏览器窗口,不包括菜单栏和状态栏,就是内容窗口)的宽度 - 35像素的滚动条宽度。

		var myTable_Width = (document.body.clientWidth -35);
		var myTable_Heigh = (document.body.clientHeight - 100);
		//alert(myTable_Width + "*" + myTable_Heigh);//测试屏幕宽度
		if ((document.body.clientWidth - 35) < 855) {
			myTable_Width = 855; //宽度
		}
		//调用 锁定表头和 列 的JS函数
		$(document).ready(function() {
			FixTable("MyTable", 3, myTable_Width, myTable_Heigh);
		});
	}
</script>
</HEAD>
<body>
	<h1 align="center"><img src="/graphics/logo32.png" height="45"></h1>
  <form action="/goform/formTest" method=post ID="paraform" name="paraform"> 
    <table width="1200" border="1" cellpadding="0" cellspacing="0" id="MyTable" style="border-bottom-color: black; border-top-color: black; width: 1300px; color: #000000; border-right-color: black; font-size: medium; border-left-color: black">
      <thead>
        <tr style="background-color: #eeeeee; margin: 0px; line-height: 20px; font-weight: bold; padding: 0px 0px 0px 0px;">
          <th width="31" class="mtrparamTableHead">表号</th>
          <th width="48" class="mtrparamTableHead">
            有效
            <br>
            <input type=checkbox name=iv_all value=iv_all onclick="iv_all_click(event);">
          </th>
          <th width="48" class="mtrparamTableHead">线路名称</th>
          <th width="48" class="mtrparamTableHead">表计地址</th>
          <th width="48" class="mtrparamTableHead">表计口令</th>
          <th width="48" class="mtrparamTableHead">使用端口</th>
          <th width="74" class="mtrparamTableHead">
            串口方案
            <br>
            <% sioplan(); %>
          </th>
          <th width="74" class="mtrparamTableHead">
            表计规约
            <br>
            <% mtr_protocol(); %>
          </th>
          <th width="74" class="mtrparamTableHead">
            生产厂家
            <br>
            <% factory(); %>
          </th>
          <th width="74" class="mtrparamTableHead">
            电表类型
            <br>
            <% ph_wire2(); %>
          </th>
          <th width="74" class="mtrparamTableHead">
            电量小数
            <br>
            <input class=ntx type=text size=1 maxlength=1 name=all_it_dot value="0" onchange="all_it_dot_changed(event);">
          </th>
          <th width="74" class="mtrparamTableHead">
            需量小数
            <br>
            <input class=ntx type=text size=1 maxlength=1 name=all_xl_dot value="0" onchange="all_xl_dot_changed(event);">
          </th>
          <th width="74" class="mtrparamTableHead">
            电压小数
            <br>
            <input class=ntx type=text size=1 maxlength=1 name=all_v_dot value="0" onchange="all_v_dot_changed(event);">
          </th>
          <th width="74" class="mtrparamTableHead">
            电流小数
            <br>
            <input class=ntx type=text size=1 maxlength=1 name=all_i_dot value="0" onchange="all_i_dot_changed(event);">
          </th>
          <th width="74" class="mtrparamTableHead">
            有功小数
            <br>
            <input class=ntx type=text size=1 maxlength=1 name=all_p_dot value="0" onchange="all_p_dot_changed(event);">
          </th>
          <th width="74" class="mtrparamTableHead">
            无功小数
            <br>
            <input class=ntx type=text size=1 maxlength=1 name=all_q_dot value="0" onchange="all_q_dot_changed(event);">
          </th>
          <th width="74" class="mtrparamTableHead">
            额定电压
            <br>
            <input class=ntx type=text size=1 name=all_ue value="0" onchange="all_ue_changed(event);">
          </th>
          <th width="74" class="mtrparamTableHead">
            额定电流
            <br>
            <input class=ntx type=text size=1 name=all_ie value="0" onchange="all_ie_changed(event);">
        </tr>
      </thead>
      <tbody><% load_all_mtr_param(); %>
      </tbody>
    </table>
    <p ALIGN="center" id=subbtns>
      <input type=button name="Update" value="设置" ID="Update" OnClick="db_update();">
      <!-- @TODO 添加删除暂时注销,等完善后再开放
					<input type=button name=bDelItem value="删除" ID="bDelItem" OnClick="db_del();">
					<input type=button name=bAddItem value="添加" ID="bAddItem" OnClick="db_add();" >
					-->
      <input type=button name=Refresh value="刷新" ID="Refresh" OnClick="return RefreshWin();">
      <!- 隐藏的输入,用于提交命令类型 ->
      <input class=hideinp type=text name=OpType value="" id="optype">
      <!- 提交操作类型 更新,还是其他->
      <input class=hideinp type=text name=RowNo value="" id="indexno">
      <input class=hideinp type=text name="AllSelFlag" value="0" id="AllSelFlag">
    </p>
  </form>
</body>
</html>
