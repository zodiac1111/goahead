<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" 
	"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=ISO-639-1">
<link href="/style/goahead.css" rel="stylesheet" type="text/css" media="screen" />
<link rel="shortcut icon" href="favicon.ico">

<base target="websframe"/>
<title>杭州华立电力</title>
</head>

<body > 
<img src="/graphics/topbluebar.gif" width="100%" height="20"><br/><br/>
<p class="secondmenu"align="center" ></p>^M
<form action=/goform/formTest method=POST>
<% load_mtr_param(); //首先加载默认的表0的参数
//加载系统参数,包括 表计个数,串口数,网口数,
//串口方案个数,等等,需要在设置参数地方给予一些限制,像一共40块表,
//就不应该能够读取表号40的表计参数.
 %>
<table  width="100%" height="400" border="0" background="/graphics/welcome1.jpg">
<tr  align="center">
	<td align="left">表号:</td>
	<td align="left"><select name=mtrno><% read_mtr_no(); %> </select></td>
	<td align="left">线路名称:</td>
	<td align="left"><input type=text name=line size=12 value="<% line(); %>"></td>
	<td align="left">表计地址:</td>
	<td align="left"><input type=text name=addr size=12 value="<% mtraddr(); %>"></td>
</tr>

<tr  align="left">
	<td align="left">表计口令:</td>
	<td align="left"><input type=text name=pwd size=12 value="<% pwd(); %>"></td>
	<td align="left">电量小数:</td>
	<td align="left"><input type=text name=it_dot size=12 value="<% it_dot(); %>"></td>
	<td  align="left">电压小数:</td>
	<td  align="left"><input type=text name=v_dot size=12 value="<% v_dot (); %>"></td>
</tr>
  
<tr  align="left">
	<td align="left">有功功率小数:</td>
	<td align="left"><input type=text name=p_dot size=12 value="<% p_dot (); %>"></td> 
	<td align="left">无功功率小数:</td>
	<td align="left"><input type=text name=q_dot size=12 value="<% q_dot (); %>"></td>
  	<td align="left">电流小数:</td>
	<td align="left"><input type=text name=xl_dot size=12 value="<% i_dot (); %>"></td>
</tr>
<tr  align="left">    
	<td align="left">需量小数:</td>
	<td align="left"><input type=text name=xl_dot size=12 value="<% xl_dot (); %>"></td>
	<td align="left">额定电压:</td>
  	<td align="left"><input type=text name=ue size=12 value="<% ue (); %>"></td>
  	<td align="left">额定电流:</td>
  	<td align="left"><input type=text name=ie size=12 value="<% ie (); %>"></td>
</tr>

<tr  align="left">
	<td align="left">使用端口:</td>
	<td align="left"><select name=port>
	<% port(); %>  
        </select></td>
  <td align="left">串口方案:</td>
  <td align="left"><select name=portplan>
	<% portplan(); %> 
        </select> </td>
  <td align="left">表计规约:</td>
	<td align="left"><select name=protocol>
 	<% protocol(); %> 
        </select></td>
</tr>

<tr  align="left">
	<td align="left">电表类型:</td>
	<td align="left"><select name=ph_wire>
	<% ph_wire(); %> 
      	</select></td>
  <td align="left">生产厂家: &nbsp;</td>
	<td align="left"><select name=factory>
	<% factory(); %> 
      	</select></td>
  <td align="left">有效标识:</td>
	<td align="left">
	<% iv(); %>
	</td>
  </tr>
  
  <tr align="left">
      <td></td>    
      <td ALIGN="CENTER"> 
        <input type=submit name=save value="设置参数"> 
      </td>
      <td></td>
      <td ALIGN="CENTER">
      <input type=submit name=load value="读取参数"> 
    </td>
    <td></td>
    <td ALIGN="CENTER">
    <input type=reset name=reset value="重新输入">
    </td>
  </tr>
</table>
</form>
<br clear=all>

</body>
</html>
