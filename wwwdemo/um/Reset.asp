<html>
<head>
<title>Arm Home</title>
<!--   Copyright (c) Echon., 2006. All Rights Reserved. -->
<meta http-equiv="Pragma" content="no-cache" charset=gb2312>
<link rel="stylesheet" href="/style/normal_ws.css" type="text/css"></link>
<link rel="stylesheet" href="/style/sys.css" type="text/css"></link>
<link href="/style/menuExpandable3.css" rel="stylesheet" type="text/css"></link>
<script src="/style/wwyfunc.js" language='javascript' type='text/JavaScript'></script>
<!-- 基于jquery的日期时间控件所需要的文件 -开始 -->
<link rel="stylesheet" media="all" type="text/css" href="/style/jquery-ui.css" />
<link rel="stylesheet" media="all" type="text/css" href="/style/jquery-ui-timepicker-addon.css" />
<script type="text/javascript" src="/style/jquery-1.8.3.min.js"></script>
<script type="text/javascript" src="/style/jquery-ui.min.js"></script>
<script type="text/javascript" src="/style/jquery-ui.min-zh-CN-GB2312.js"></script>
<script type="text/javascript" src="/style/jquery-ui-timepicker-addon.js"></script>
<script type="text/javascript" src="/style/jquery-ui-timepicker-zh-CN-GB2312.js"></script>
<script type="text/javascript" src="/style/jquery-ui-sliderAccess.js"></script>
<!-- 基于jquery的日期时间控件所需要的文件 -结束 -->
<script type="text/javascript">
	var stime = document.getElementById("stime_stamp");
	var etime = document.getElementById("etime_stamp");
	var mtr_no = document.getElementById("mtr_no");
	$(document).ready(
			function() {
				$("#btn").click(function() {
					$.get("/um/compara1.asp", function(result) {
						$("#myDiv").html(result);
					});
				});
				$("#btnPost").click(
						function() {
							//显示
							$("#msgbox_wait").show("fade", {}, 1);
							$.post('/goform/get_tou', $("#history_tou")
									.serialize(), function(result) {// 动态加载完的页面才可以接收鼠标悬停等事件
								$("#tr_dat").html(result);
								$("#history_tou tr").mouseover(function() {
									$(this).addClass("over");
									var rows = $(this).attr('relrow');
								});
								$("#history_tou tr").mouseout(function() {
									$(this).removeClass("over");
								});
								//完成之后隐藏
								$("#msgbox_wait").hide("fade", {}, 1);
								$("#msgbox_ok").show("fade", {}, 1);
								$("#msgbox_ok").hide("fade", {}, 1000);
							});
						});
			});
</script>
<% init_sysparam(); //加载系统参数 %>
</head>
<body>
  <br />
  <form action="/goform/reset" method=post ID="resetform" name="resetform">
    <table class="sioplanTable" id=tbl_sysReset border="1" cellspacing="1" cellpadding="1">
      <thead>
        <tr>
          <th>序号</th>
          <th>说明</th>
          <th>操作</th>
        </tr>
      </thead>
      <tbody>
        <tr class="sysTDNcLItemStyle">
          <td>1</td>
          <td>服务器端重新加载规约文本.</td>
          <td>
            <input type=image src="/graphics/Reset12.png" onmouseover="this.src='/graphics/Reset121.png';" onmouseout="this.src='/graphics/Reset12.png';" alt="Submit" OnClick="reset_procotol();">
          </td>
        </tr>
        <tr class="sysTDNcLItemStyle">
          <td>2</td>
          <td>重启web服务器.</td>
          <td>
            <input type=image src="/graphics/Reset11.png" onmouseover="this.src='/graphics/Reset111.png';" onmouseout="this.src='/graphics/Reset11.png';" alt="Submit" OnClick="reset_web();">
          </td>
        </tr>
        <tr class="sysTDNcLItemStyle">
          <td>3</td>
          <td>重启抄表程序</td>
          <td>
            <input type=image src="/graphics/Reset13.png" onmouseover="this.src='/graphics/Reset131.png';" onmouseout="this.src='/graphics/Reset13.png';" alt="Submit" OnClick="reset_sample();">
          </td>
        </tr>
        <tr class="sysTDNcLItemStyle">
          <td>4</td>
          <td>重启终端操作系统</td>
          <td>
            <input type=image src="/graphics/Reset11.png" onmouseover="this.src='/graphics/Reset111.png';" onmouseout="this.src='/graphics/Reset11.png';" alt="Submit" OnClick="reset_web();">
          </td>
        </tr>
      </tbody>
    </table>
    <p ALIGN="CENTER">
      <!--隐藏的输入框,用于提交form命令类型 -->
      <input class=hideinp type=text name=OpType value="" id="optype">
    </p>
  </form>
  
  <form id="history_tou" name="history_tou">
    <p>
    <h1>获取和设置日期时间:</h1>
    <br>
    <label>
      表号:
      <input type="text" name="mtr_no" id="mtr_no" size=3 value="0">
    </label>
    <label>
      时间范围:
      <input type="text" name="stime" id="stime" readonly>
    </label>
    <label>
      ~
      <input type="text" name="etime" id="etime" readonly>
    </label>
    <br>
    <label>
      时间戳(秒):
      <input type="text" name="stime_stamp" id="stime_stamp" value="0" readonly>
    </label>
    <label>
      ~
      <input type="text" name="etime_stamp" id="etime_stamp" value="0" readonly>
    </label>
    <br>
    <label>
      时区偏移(分钟)
      <input type="text" name="timezone" id="timezone" value="0" readonly>
      <input type="text" name="timezone2" id="timezone2" value="0" readonly>
    </label>
    <br>
  </form>
  <script language=javascript>
			var startDateTextBox = $('#stime');
			var endDateTextBox = $('#etime');
			var stime_stamp = document.getElementById("stime_stamp");
			var etime_stamp = document.getElementById("etime_stamp");
			var tz = document.getElementById("timezone");
			var tz2 = document.getElementById("timezone2");
			//开始时间控件的属性设置
			startDateTextBox.datetimepicker({
				maxDate : 0,
				controlType : 'select', //选择方式选时刻
				dateFormat : "yy-mm-dd", //日期格式
				//showSecond: true,	//显示秒
				timeFormat : 'HH:mm z', //时刻格式
				separator : ' ', //日期时刻分割字符(串)
				//showTimezone: true,	//显示时区
				//timezone: '+0800',		//默认时区
				//点击关闭(完成)按钮事件 
				//如果开始时间晚于结束时间,则将开始时间设置为结束时间 
				onClose : function(dateText, inst) {
					if (endDateTextBox.val() != '') {
						var testStartDate = startDateTextBox
								.datetimepicker('getDate');
						var testEndDate = endDateTextBox
								.datetimepicker('getDate');
						if (testStartDate > testEndDate)
							endDateTextBox.datetimepicker('setDate',
									testStartDate);
					} else {
						endDateTextBox.val(dateText);
					}
					var testStartDate = startDateTextBox
							.datetimepicker('getDate');
					etime_stamp.value = testStartDate.getTime() / 1000;
					//Date.parse(endDateTextBox.datetimepicker('getDate'))/1000;
				},
				onSelect : function(selectedDateTime) {//选择时限定开始时间必须早于结束时间
					endDateTextBox.datetimepicker('option', 'minDate',
							startDateTextBox.datetimepicker('getDate'));
					//tz.value=startDateTextBox.datetimepicker.timezone;
					var testStartDate = startDateTextBox
							.datetimepicker('getDate');
					tz.value = testStartDate.getTimezoneOffset();
					//这里转换成位标准时间,没有时区信息了.
					stime_stamp.value = testStartDate.getTime() / 1000;
					//Date.parse(startDateTextBox.datetimepicker('getDate'))/1000;
				}
			});
			//结束时间控件的属性设置
			endDateTextBox.datetimepicker({
				maxDate : 0,//不能选择未来
				controlType : 'select', //选择方式选时刻
				dateFormat : "yy-mm-dd", //日期格式
				//showSecond: true,	//显示秒
				timeFormat : 'HH:mm z', //时刻格式
				separator : ' ', //日期时刻分割字符(串)
				//showTimezone: true,	//显示时区
				//timezone: '+0800',		//默认时区
				onClose : function(dateText, inst) {//关闭时候判断
					if (startDateTextBox.val() != '') {
						var testStartDate = startDateTextBox
								.datetimepicker('getDate');
						var testEndDate = endDateTextBox
								.datetimepicker('getDate');
						if (testStartDate > testEndDate)
							startDateTextBox.datetimepicker('setDate',
									testEndDate);
					} else {
						startDateTextBox.val(dateText);
					}
					var testEndDate = endDateTextBox.datetimepicker('getDate');
					etime_stamp.value = testEndDate.getTime() / 1000;
					//stime_stamp.value=
					//	Date.parse(startDateTextBox.datetimepicker('getDate'))/1000;
				},
				onSelect : function(selectedDateTime) {//选择时候防止误选
					startDateTextBox.datetimepicker('option', 'maxDate',
							endDateTextBox.datetimepicker('getDate'));

					var testEndDate = endDateTextBox.datetimepicker('getDate');
					tz2.value = testEndDate.getTimezoneOffset();
					etime_stamp.value = testEndDate.getTime() / 1000;
				}
			});
		</script>
  <button id="btn">子页面加载</button>
  <button id="btnPost">查询</button>
  <button id="showLog">查看日志</button>
  <div id="msgbox_wait" style="display: none;" class=msgbox_wait>
    <input type=image src="/style/images/wait.gif">
  </div>
  <div id="msgbox_ok" style="display: none;" class=msgbox_ok>完成</div>
  <table class="sioplanTable" ID="history_tou" border="1" cellspacing="1" cellpadding="1">
    <thead>
      <tr>
        <th rowspan=2>表号</th>
        <th rowspan=2>序号</th>
        <th rowspan=2>时刻</th>
        <th colspan=5>正向有功</th>
        <th colspan=5>反向有功</th>
        <th colspan=5>正向无功</th>
        <th colspan=5>反向无功</th>
      </tr>
      <tr>
        <th>总</th>
        <th>尖</th>
        <th>峰</th>
        <th>平</th>
        <th>谷</th>
        <th>总</th>
        <th>尖</th>
        <th>峰</th>
        <th>平</th>
        <th>谷</th>
        <th>总</th>
        <th>尖</th>
        <th>峰</th>
        <th>平</th>
        <th>谷</th>
        <th>总</th>
        <th>尖</th>
        <th>峰</th>
        <th>平</th>
        <th>谷</th>
      </tr>
    </thead>
    <tbody id=tr_dat>
  </table>
  <p><% show_log(); %></p>
</body>
</html>
