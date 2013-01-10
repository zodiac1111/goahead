<html>
<head>
<title>终端数据浏览</title>
<meta http-equiv="Content-Type" content="text/html; charset=gb2312">
<link rel="stylesheet" href="/style/emx_nav_left1.css" type="text/css">
<style type="text/css">
img {border-width: 0px 0px 0px 0px}
</style>
		<!- 基于jquery的日期时间控件所需要的文件 -开始->
		<link rel="stylesheet" media="all" type="text/css" href="/style/jquery-ui.css"/> 
		<link rel="stylesheet" media="all" type="text/css" href="/style/jquery-ui-timepicker-addon.css"/>
		<script type="text/javascript" src="/style/jquery-1.8.3.min.js"></script>
		<script type="text/javascript" src="/style/jquery-ui.min.js"></script>
		<script type="text/javascript" src="/style/jquery-ui-timepicker-addon.js"></script>
		<script type="text/javascript" src="/style/jquery-ui-timepicker-zh-CN-GB2312.js"></script>
		<script type="text/javascript" src="/style/jquery-ui-sliderAccess.js"></script>
		<!- 基于jquery的日期时间控件所需要的文件 -结束->
</head>
<body >
<table width="101%" border="0" cellspacing="0" cellpadding="4">
  <tr> 
    <td bgcolor="#ffffff">      <table width="100%" border="0" cellspacing="1" cellpadding="2">
        <tr> 
          <td align="center" width="33%"><a href="/um/data1.asp"><img src="/graphics/logo80.png" onmouseover="this.src='/graphics/logo81.png';" onmouseout="this.src='/graphics/logo80.png';" height="40"/></a></td>
          <td align="center" width="33%"><a href="/um/data2.asp"><img src="/graphics/logo90.png" onmouseover="this.src='/graphics/logo91.png';" onmouseout="this.src='/graphics/logo90.png';" height="40"/></a></td>
          <td align="center" width="33%"><a href="/um/data3.asp"><img src="/graphics/logo100.png" onmouseover="this.src='/graphics/logo101.png';" onmouseout="this.src='/graphics/logo100.png';" height="40"/></a></td>
        </tr>
      </table></td>
  </tr>
</table>
<table width="101%" border="0" cellspacing="0" cellpadding="2" bgcolor="#76A8D9">
  <tr> 
    <td> 
      &#149; <a href="#">历史瞬时量数据</a></td>
    <td align="right">
     <a >起始时间</a> &#149; <a ><input type="text" name="rest_example_4_start" id="rest_example_4_start" value="" /> </a>&nbsp;<a>结束时间</a>     &#149; <a > <input type="text" name="rest_example_4_end" id="rest_example_4_end" value="" /></a>    
      <a href="">search</a> 
<script language=javascript>
var startDateTextBox = $('#rest_example_4_start');
var endDateTextBox = $('#rest_example_4_end');
startDateTextBox.datetimepicker({ 
	onClose: function(dateText, inst) {
		if (endDateTextBox.val() != '') {
			var testStartDate = startDateTextBox.datetimepicker('getDate');
			var testEndDate = endDateTextBox.datetimepicker('getDate');
			if (testStartDate > testEndDate)
				endDateTextBox.datetimepicker('setDate', testStartDate);
		}
		else {
			endDateTextBox.val(dateText);
		}
	},
	onSelect: function (selectedDateTime){
		endDateTextBox.datetimepicker('option', 'minDate', startDateTextBox.datetimepicker('getDate') );
	}
});
endDateTextBox.datetimepicker({ 
	onClose: function(dateText, inst) {
		if (startDateTextBox.val() != '') {
			var testStartDate = startDateTextBox.datetimepicker('getDate');
			var testEndDate = endDateTextBox.datetimepicker('getDate');
			if (testStartDate > testEndDate)
				startDateTextBox.datetimepicker('setDate', testEndDate);
		}
		else {
			startDateTextBox.val(dateText);
		}
	},
	onSelect: function (selectedDateTime){
		startDateTextBox.datetimepicker('option', 'maxDate', endDateTextBox.datetimepicker('getDate') );
	}
});
</script>
  </td>
  </tr>
</table>
<table>
<div>
</div>
</table>
</body>
</html>
