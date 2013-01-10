<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" 
	"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=ISO-639-1">
<link href="C:\Documents and Settings\zhang\桌面/goahead.css" rel="stylesheet" type="text/css" media="screen" />
<link rel="shortcut icon" href="favicon.ico">

<base target="websframe"/>
<title>杭州华立电力</title>
</head>

<body bgcolor="#F0F8FF" > 
<br/>
<p align="center">电表参数设置</p>


<form action=/goform/formTest method=POST>
<table  width="100%" height="400" border="0" background="">
<tr  align="center">
	
	<td align="left">表号:</td>
	<td align="left"><input type=text name=a size=12 value=""></td>
	<td align="left">线路名称:</td>
	<td align="left"><input type=text name=b size=12 value=""></td>
	<td align="left">表计地址:</td>
	<td align="left"><input type=text name=c size=12 value=""></td>
</tr>

<tr  align="left">
	<td align="left">表计口令:</td>
	<td align="left"><input type=text name=d size=12 value=""></td>
	<td align="left">电量小数:</td>
	<td align="left"><input type=text name=e size=12 value=""></td>
	<td  align="left">电压小数:</td>
	<td  align="left"><input type=text name=h size=12 value=""></td>
</tr>
  
<tr  align="left">
	<td align="left">有功功率小数:</td>
	<td align="left"><input type=text name=de size=12 value=""></td> 
	<td align="left">无功功率小数:</td>
	<td align="left"><input type=text name=sd size=12 value=""></td>
  <td align="left">需量小数:</td>
	<td align="left"><input type=text name=ed size=12 value=""></td>
</tr>
<tr  align="left">    
	<td align="left">需量小数:</td>
	<td align="left"><input type=text name=ed size=12 value=""></td>
	<td align="left">额定电压:</td>
  <td align="left"><input type=text name=ed size=12 value=""></td>
  <td align="left">额定电流:</td>
  <td align="left"><input type=text name=ed size=12 value=""></td>
</tr>

<tr  align="left">
	<td align="left">使用端口:</td>
	<td align="left"><select name="g">
      <option size=12 value="2">com3</option>
      <option size=12 value="3">com4</option>
      <option size=12 value="4">com5</option>
      <option size=12 value="5">com6</option>
      <option size=12 value="6">com7</option>
      <option size=12 value="7">com8</option>      
      </select></td>
  <td align="left">串口方案:&nbsp; &nbsp;</td>
  <td align="left"><select name="j">
      <option value="0"selected="selected">方案0</option>
      <option value="1">方案1</option>
      <option value="2">方案2</option>
      <option value="3">方案3</option>
      <option value="4">方案4</option>
      <option value="5">方案5</option>
      <option value="6">方案6</option>
      </select> </td>
  <td align="left">表计规约:</td>
	<td align="left"><select name="as">
      <option value="0">645-07</option>
      <option value="1">645-97</option>
      <option value="2">方案2</option>
      <option value="3">方案3</option>
      <option value="3">方案4</option>
      <option value="3">方案5</option>
      <option value="3">方案6</option>
      </select></td>
</tr>

<tr  align="left">
	<td align="left">电表类型:</td>
	<td align="left"><select name="k">
      <option value="0" selected="selected">3相3线</option>
      <option value="1">3相4线</option>
      </select></td>
  <td align="left">生产厂家: &nbsp;</td>
	<td align="left"><select name="f">
      <option value="0"selected="selected">华立</option>
      <option value="1">威胜</option>
      <option value="2"></option>
      <option value="3">3</option>
      <option value="3">4</option>
      </select></td>
  <td align="left">有效标识:</td>
	<td align="left"><select name="ds">
      <option value="0"selected="selected">有效</option>
      <option value="1">无效</option>
      </select></td>
  </tr>
 
  <tr align="left">
      <td></td>    
      <td ALIGN="left"> 
        <input type=submit name=ok value="设置参数"> 
      </td>
      <td></td>
      <td ALIGN="left">
      <input type=submit name=ok value="读取参数"> 
    </td>
    <td></td>
    <td ALIGN="left">
    <input type=reset name=ok value="重新输入">
    </td>
  </tr>
</table>
</form>
<br clear=all><br/><br/>

</body>
</html>
