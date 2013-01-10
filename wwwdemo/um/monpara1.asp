<html>
	<HEAD>
		<title>Arm Home</title><!- Copyright (c) Echon., 2006. All Rights Reserved. ->
		<meta http-equiv="Pragma" content="no-cache" charset=gb2312>
		<link rel="stylesheet" href="/style/normal_ws.css" type="text/css"></link>
		<link rel="stylesheet" href="/style/sys.css" type="text/css"></link>
		<link href="/style/menuExpandable3.css" rel="stylesheet" type="text/css"></link>
		<script src="/style/wwyfunc.js" language='javascript' type='text/JavaScript' > </script>
		<% init_sysparam(); //初始化系统参数,获得例如表计个数,串口个数,串口方案个数等参数 %>
	</HEAD>
	<body> 
		<br/>
	<h1  align="center"><img src="/graphics/logo52.png" height="45"></h1>
		<form action="/goform/monparas"  method=post  ID="paraform" name="paraform">
			<table class="monparamTable"  border="1" cellspacing="1" cellpadding="1" >
				<tr>  
					<th width="5%" class="monparamTableHead">序号</th>
					<th width="6%" class="monparamTableHead">使用端口</th>
					<th width="4%" class="monparamTableHead">服务端口</th>
					<th width="6%" class="monparamTableHead">串口方案</th>
					<th width="6%" class="monparamTableHead">使用规约</th>
					<th width="5%" class="monparamTableHead">终端地址</th>
					<th width="6%" class="monparamTableHead">对时有效</th>
					<th width="6%" class="monparamTableHead">转发有效</th>
					<th width="5%" class="monparamTableHead">转发数目</th>
					<th width="5%" class="monparamTableHead">序号1</th>
					<th width="6%" class="monparamTableHead">序号2</th> 
					<th width="5%" class="monparamTableHead">序号3</th>
					<th width="5%" class="monparamTableHead">序号4</th>
					<th width="5%" class="monparamTableHead">序号5</th>
					<th width="5%" class="monparamTableHead">序号6</th>
					<th width="4%" class="monparamTableHead">序号7</th> 
					<th width="5%" class="monparamTableHead">序号8</th>
					<th width="5%" class="monparamTableHead">序号9</th>
				</tr>
				<% load_monparams(); %>
				<!--
				<tr>
					<td class=sysTDNcLItemStyle ><input class=ntx type=text size=2 name=no0 value="0" ></td>
					<td class=sysTDNcLItemStyle><SELECT name=type0 >
							<OPTION selected value=0>ETH1</OPTION>
							<OPTION  value=1>ETH2</OPTION>
							<OPTION  value=2>ETH3</OPTION>
							<OPTION  value=2>ETH4</OPTION>
							<OPTION  value=2>COM1</OPTION>
							<OPTION  value=2>COM2</OPTION>
						</SELECT>
					</td>
					<td class=sysTDNcLItemStyle ><input class=ntx type=text size=5 name=v_iedNo0 id=v_iedNo0 value=0></td>
					<td class=sysTDNcLItemStyle><SELECT name=type0 >
							<OPTION selected value=0>方案0</OPTION>
							<OPTION  value=1>方案1</OPTION>
							<OPTION  value=2>方案2</OPTION>
							<OPTION  value=2>方案3</OPTION>
							<OPTION  value=2>方案4</OPTION>
							<OPTION  value=2>方案5</OPTION>
						</SELECT>
					</td>
					<td class=sysTDNcLItemStyle><SELECT name=type0 >
							<OPTION selected value=0>sx102</OPTION>
							<OPTION  value=1>gx102</OPTION>
						</SELECT>
					</td> 
					<td class=sysTDNcLItemStyle><input class=ntx type=text size=5 name=no0 value="000001" ></td>
					<td class=sysTDNcLItemStyle><input class=ntx type=checkbox  size=12 name=name0 >有效</td>
					<td class=sysTDNcLItemStyle><input class=ntx type=checkbox size=5 name=maxAiNum0 >有效</td>				 	      
					<td class=sysTDNcLItemStyle><input class=ntx type=text name=maxDiNum0 size=2 value="4"></td>
					<td class=sysTDNcLItemStyle><input class=ntx type=text name=maxEiNum0 size=2 value="2"></td>
					<td class=sysTDNcLItemStyle><input class=ntx type=text name=syncTime0 size=2 value="2"></td>
					<td class=sysTDNcLItemStyle><input class=ntx type=text name=deadTime0 size=2 value="2"></td>
					<td class=sysTDNcLItemStyle><input class=ntx type=text name=deadTime0 size=2 value="2"></td>
					<td class=sysTDNcLItemStyle><input class=ntx type=text name=deadTime0 size=2 value="2"></td>
					<td class=sysTDNcLItemStyle><input class=ntx type=text name=deadTime0 size=2 value="2"></td>
					<td class=sysTDNcLItemStyle><input class=ntx type=text name=deadTime0 size=2 value="2"></td>
					<td class=sysTDNcLItemStyle><input class=ntx type=text name=deadTime0 size=2 value="2"></td>
					<td class=sysTDNcLItemStyle><input class=ntx type=text name=deadTime0 size=2 value="2"></td>
				</tr>
				-->
			</table>
			<p ALIGN="center" height=25> 
			<input type=button name="Update" value="设置" ID="Update" OnClick="db_update();">
			<!--
			<input type=button name=bDelItem value="删除" ID="bDelItem"  onclick="return DelSubmit();">
			<input type=button  name=bAddItem value="添加" ID="bAddItem" OnClick="return Redirect('AddRoutePara.asp');" >
			-->
			<input type=button  name=Refresh value="读取" ID="Refresh" OnClick="return RefreshWin();" > 
			<!- 隐藏的文本框:用于提交操作类型 更新,还是其他->
			<input class=hideinp  type=text name=OpType value=""  id="optype" > 
			</p>
		</form>
	</body>
</html>
