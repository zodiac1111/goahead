<html>
 <HEAD>
<title>Arm Home</title><!- Copyright (c) Echon., 2006. All Rights Reserved. ->
<meta http-equiv="Pragma" content="no-cache" charset=gb2312>
<link rel="stylesheet" href="/style/normal_ws.css" type="text/css"></link>
<link rel="stylesheet" href="/style/sys.css" type="text/css"></link>
<link href="style/menuExpandable3.css" rel="stylesheet" type="text/css"></link>
<script src="style/wwyfunc.js" language='javascript' type='text/JavaScript' > </script>
</HEAD>
<body> 
	<form action="/goform/SetRoute"  method=post  ID="paraform" name="paraform">
		<table ID="Table4" border="0" cellspacing="0" cellpadding="0"  class="tbbordclo" width="99%">
			<tr>
				<td height=25 align="center" class="hfont"><font size=4>表计参数</font></td>
			</tr>
      <tr>
	      <td class="lb-top-head"  valign=top  align=center >
		      <table width="100%" border="0" cellspacing="1" cellpadding="2" class="sysBgTable" ID="Table1">
			      <tr>
				      <td class="sysTDHeadCLStyle">
					      <table width="100%" border="0" cellspacing="0" cellpadding="2" ID="Table2">
						     <tr>
							     <td>
								     <a>表计数目</a>
								     <input type=text size=6  id="Text1" name="hao"  maxsize="5" datatype="number" namefs="监视数目" nullable="yesnoz"  value="">
								     <a class=acs onclick="return  Search();">读取</a>
							     </td>
						     </tr>
					      </table>
				      </td>
			      </tr>
		      </table>
	      </td>
      </tr>
      <tr >
	      <td class="lb-body" valign=top  align=center> 
		      <table border="0" width="99%" " style="margin-top:5" ID="Table3">
			      <tr>
				      <td class="lb-bkg" valign="top">
				       <table width="90%" cellspacing="1" cellpadding="2"  border="0" class="sysBgTable" ID="Table4">
						      <tr>
							      <td  colspan="19"  align=center class="sysTDHeadCCStyle">参数列表</td>
						      </tr>
						      <tr>  
							       <td width="5%" class="sysTDCLItemStyle" align="center">表号</td>
							       <td width="5%" class="sysTDCLItemStyle" align="center">有效标识</td>
							       <td width="6%" class="sysTDCLItemStyle" align="center">线路名称</td>
							       <td width="4%" class="sysTDCLItemStyle" align="center">表计地址</td>
							       <td width="6%" class="sysTDCLItemStyle" align="center">表计口令</td>
							       <td width="6%" class="sysTDCLItemStyle" align="center">使用端口</td>
							       <td width="5%" class="sysTDCLItemStyle" align="center">串口方案</td>
							       <td width="5%" class="sysTDCLItemStyle" align="center">表计规约</td>
							       <td width="5%" class="sysTDCLItemStyle" align="center">生产厂家</td>
							       <td width="7%" class="sysTDCLItemStyle" align="center">电表类型</td>							       
							       <td width="5%" class="sysTDCLItemStyle" align="center">电量小数</td>			
							       <td width="5%" class="sysTDCLItemStyle" align="center">需量小数</td>							
							       <td width="6%" class="sysTDCLItemStyle" align="center">电压小数</td> 
							       <td width="7%" class="sysTDCLItemStyle" align="center">电流小数</td>
							       <td width="5%" class="sysTDCLItemStyle" align="center">有功功率小数</td>
							       <td width="5%" class="sysTDCLItemStyle" align="center">无功功率小数</td>
							       <td width="4%" class="sysTDCLItemStyle" align="center">额定电压</td> 
							       <td width="5%" class="sysTDCLItemStyle" align="center">额定电流</td>
							       
						      </tr>
						      <tr>
						      	 <td class=sysTDNcLItemStyle ><input  class=ntx type=text size=2 name=no0 value="0" ></td>
							       <td class=sysTDNcLItemStyle><input type=checkbox name=selCheck0 >有效</td>
							       <input class=hideinp type=text size=4 name=v_iedNo0 id=v_iedNo0 value=0>
							       <td class=sysTDNcLItemStyle ><input  class=ntx type=text size=6 name=no0 value="000001" ></td>
							       <td class=sysTDNcLItemStyle><input  class=ntx type=text name=name0 size=12 value="000000000001"></td>
							       <td class=sysTDNcLItemStyle><input class=ntx type=text name=maxAiNum0 size=8 value="00000001"></td>
							       <td class=sysTDNcLItemStyle><SELECT name=type0 >
							 	      <OPTION selected value=0>com3</OPTION>
							 	      <OPTION  value=1>com4</OPTION>
							 	      <OPTION  value=2>com5</OPTION>
							 	      <OPTION  value=2>com6</OPTION>
							 	      <OPTION  value=2>com7</OPTION>
							 	      <OPTION  value=2>com8</OPTION>
							 	      </SELECT>
							       </td>
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
							 	      <OPTION selected value=0>645-97</OPTION>
							 	      <OPTION  value=1>645-07</OPTION>
							 	      </SELECT>
							       </td>      		       
							       <td class=sysTDNcLItemStyle><SELECT name=type0 >
							 	      <OPTION selected value=0>华立</OPTION>
							 	      <OPTION  value=1>威胜</OPTION>
							 	      <OPTION  value=2>兰吉尔</OPTION>
							 	      <OPTION  value=2>红相</OPTION>
							 	      </SELECT>
							 	      </td>
							 	      <td class=sysTDNcLItemStyle><SELECT name=type0 >
							 	      <OPTION selected value=0>3相3线</OPTION>
							 	      <OPTION  value=1>3相4线</OPTION>
							 	      </SELECT>
							 	      </td>							 	      
							       <td class=sysTDNcLItemStyle align=center><input class=ntx type=text name=maxDiNum0 size=2 value="2"></td>
							       <td class=sysTDNcLItemStyle align=center><input class=ntx type=text name=maxEiNum0 size=2 value="2"></td>
							       <td class=sysTDNcLItemStyle align=center><input class=ntx type=text name=syncTime0 size=2 value="2"></td>
							       <td class=sysTDNcLItemStyle align=center><input class=ntx type=text name=deadTime0 size=2 value="2"></td>
                     <td class=sysTDNcLItemStyle align=center><input class=ntx type=text name=deadTime0 size=2 value="2"></td>
                     <td class=sysTDNcLItemStyle align=center><input class=ntx type=text name=deadTime0 size=2 value="2"></td>
                     <td class=sysTDNcLItemStyle align=center><input class=ntx type=text name=deadTime0 size=2 value="2"></td>
                     <td class=sysTDNcLItemStyle align=center><input class=ntx type=text name=deadTime0 size=2 value="2"></td>
						      </tr> 	 
					     </table>
				      </td>
			      </tr>
		      </table>
	      </td>
      </tr>
	    <tr valign=top>
		    <td>
			    <table width=100% ID="Table5">
				    <tr>
					    <td align=right>
						  <input class="hideinp" type=text id=pageindex name=pageIndex  value="">
						  <a class=acs onclick="return  GoForNPage(-1);">前一页</a>
						  <a class=acs  onclick="return  GoForNPage(1);">后一页</a>
						  <a>当前第</a> 
						  <%RoutePageSelect();%> 
					    </td>
				    </tr>
			    </table>
		    </td>
	    </tr>   
	    <tr>
		     <td ALIGN="center" height=25> 
			   <input type=button name="Update" value="设置" ID="Update" OnClick="UpdateTheSelect();">
			   <input type=button name=bDelItem value="删除" ID="bDelItem"  onclick="return DelSubmit();">
			   <input type=button  name=bAddItem value="添加" ID="bAddItem" OnClick="return Redirect('AddRoutePara.asp');" >
			   <input type=button  name=Refresh value="刷新" ID="Refresh" OnClick="return RefreshWin();" > 
		     </td>
	    </tr>
	    <tr>
		    <td> 
			  <input class=hideinp  type=text name=RowNo  value=""  id="indexno">
			  <input class=hideinp  type=text name=OpType value=""  id="optype" >
			  <input class=hideinp  type=text name="AllSelFlag" value="0"  id="AllSelFlag">
		    </td>
	    </tr> 
		</table>
  </form>
<form action=/goform/ReSetCmu  method=post  ID="Reset" name="Reset"></form>
</body>
</html>
