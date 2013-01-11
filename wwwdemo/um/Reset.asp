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
<script type="text/javascript" src="/js/jquery-1.8.3.js"></script>
<script type="text/javascript" src="/style/jquery-ui.min.js"></script>
<script type="text/javascript" src="/style/jquery-ui.min-zh-CN-GB2312.js"></script>
<script type="text/javascript" src="/style/jquery-ui-timepicker-addon.js"></script>
<script type="text/javascript" src="/style/jquery-ui-timepicker-zh-CN-GB2312.js"></script>
<script type="text/javascript" src="/style/jquery-ui-sliderAccess.js"></script>
<script type="text/javascript" src="/js/reset.js"></script>
<!-- 基于jquery的日期时间控件所需要的文件 -结束 -->
<% init_sysparam(); //加载系统参数 %>
</head>
<body>
   <br />
   <h1>功能操作</h1>
   <div id="tabs">
      <ul>
         <li><a href="#tabs_func">系统操作</a></li>
         <li><a href="#tabs-log">日志</a></li>
         <li><a href="#tabs-monport-cfg">监视端口配置</a></li>
         <li><a href="#tabs-tou">历史电量</a></li>
      </ul>
      <div id="tabs_func">
         <table class="sioplanTable" id=tbl_sysReset border="1" cellspacing="1" cellpadding="1">
            <thead>
               <tr>
                  <th>序号</th>
                  <th>说明</th>
                  <th>操作</th>
               </tr>
            </thead>
            <tbody>
               <tr>
                  <td>1</td>
                  <td>服务器端重新加载规约文本.</td>
                  <td>
                     <button id=btnResetPro class=reboot></button>
                  </td>
               </tr>
               <tr>
                  <td>2</td>
                  <td>重启web服务器.</td>
                  <td>
                     <button id=btnResetWeb class=reboot></button>
                  </td>
               </tr>
               <tr>
                  <td>3</td>
                  <td>重启抄表程序</td>
                  <td>
                     <button id=btnResetSample class=reboot></button>
                  </td>
               </tr>
               <tr>
                  <td>4</td>
                  <td>重启终端操作系统</td>
                  <td>
                     <button id=btnResetRtu class=reboot></button>
                  </td>
               </tr>
            </tbody>
         </table>
      </div>
      <!-- ###标签2 ### -->
      <div id="tabs-log">
         <textarea id=log_text class=log_txt></textarea>
         <p ALIGN="center">
            <!--  
        <button id="import_log">导入</button>
        <button id="export_log">导出</button>
      
        _______-->
            <button id="load_log" title="从终端读取日志文件.">读取</button>
            <button id="save_log" title="将文本保存到终端日志文件中.">保存</button>
            <img class="waiticon" id=log_wait></img>
         </p>
         <!-- 保存确认对话框1 -->
         <div id="dialog-confirm" class=dialog title="确认将修改保存到终端中?">
            <span class="ui-icon ui-icon-alert" style="float: left; margin: 0 7px 20px 0;"></span>
            这个操作会将文本框中的内容保存到终端日志文件中,不能撤销,请自留备份.是否保存到终端?
         </div>
      </div>
      <div id="tabs-monport-cfg">
         <textarea id=monport_text class=log_txt></textarea>
         <p ALIGN="center">
            <button id="import_monprot" title="调试中...">导入</button>
            <button id="export_monprot" title="调试中...">导出</button>
            <button id="load_monport" title="从终端读取监视端口配置文件(文本格式)">读取</button>
            <button id="save_monport" title="将文本保存到终端监视端口文件">保存</button>
            <img class="waiticon" id=monprot_wait></img>
         </p>
         <!-- 保存确认对话框1 -->
         <div id="dialog-confirm-monport" class=dialog title="确认将修改保存到终端中?">
            <span class="ui-icon ui-icon-alert" style="float: left; margin: 0 7px 20px 0;"></span>
            <p>
               这个操作会将文本框中的内容保存到终端监视端口文件中.
               <br>
               终端重启后生效.该操作不能撤销,请自留备份.
               <br>
               是否保存到终端?
            </p>
         </div>
      </div>
      <div id="tabs-tou">
         <p>
         <h1>获取和设置日期时间:</h1>
         <label> 表号: <input type="text" name="mtr_no" id="mtr_no" size=3 value="0"></label> <label> 时间范围: <input type="text" name="stime" id="stime" readonly class="date " title="选择开始时刻"></label> <label> ~ <input type="text"
               name="etime" id="etime" readonly class="date " title="选择截止时刻">
         </label>
         <button class="ui-button" id="btnPost" title="查询选中时段和指定表计的电量数据">查询</button>
         <img class="waiticon" id=msgbox_wait />
         <div class=hide>
            <form id="history_tou" name="history_tou">
               <label> 时间戳(秒): <input type="text" name="stime_stamp" id="stime_stamp" value="0" readonly>
               </label> <label> ~ <input type="text" name="etime_stamp" id="etime_stamp" value="0" readonly>
               </label>
               <br>
               <label> 时区偏移(分钟) <input type="text" name="timezone" id="timezone" value="0" readonly> <input type="text" name="timezone2" id="timezone2" value="0" readonly>
               </label>
            </form>
         </div>
         <script language=javascript>
										var startDateTextBox = $('#stime');
										var endDateTextBox = $('#etime');
										var stime_stamp = document
												.getElementById("stime_stamp");
										var etime_stamp = document
												.getElementById("etime_stamp");
										var tz = document
												.getElementById("timezone");
										var tz2 = document
												.getElementById("timezone2");
										//开始时间控件的属性设置
										startDateTextBox
												.datetimepicker({
													maxDate : 0,
													controlType : 'select', //选择方式选时刻
													dateFormat : "yy-mm-dd", //日期格式
													//showSecond: true, //显示秒
													timeFormat : 'HH:mm', //时刻格式
													separator : ' ', //日期时刻分割字符(串)
													//showTimezone: true, //显示时区
													//timezone: '+0800',    //默认时区
													//点击关闭(完成)按钮事件 
													//如果开始时间晚于结束时间,则将开始时间设置为结束时间 
													onClose : function(
															dateText, inst) {
														if (endDateTextBox
																.val() != '') {
															var testStartDate = startDateTextBox
																	.datetimepicker('getDate');
															var testEndDate = endDateTextBox
																	.datetimepicker('getDate');
															if (testStartDate > testEndDate)
																endDateTextBox
																		.datetimepicker(
																				'setDate',
																				testStartDate);
														} else {
															endDateTextBox
																	.val(dateText);
														}
														var testStartDate = startDateTextBox
																.datetimepicker('getDate');
														if (testStartDate != null)
															stime_stamp.value = testStartDate
																	.getTime() / 1000;
														//Date.parse(endDateTextBox.datetimepicker('getDate'))/1000;
													},
													onSelect : function(
															selectedDateTime) {//选择时限定开始时间必须早于结束时间
														endDateTextBox
																.datetimepicker(
																		'option',
																		'minDate',
																		startDateTextBox
																				.datetimepicker('getDate'));
														//tz.value=startDateTextBox.datetimepicker.timezone;
														var testStartDate = startDateTextBox
																.datetimepicker('getDate');
														tz.value = testStartDate
																.getTimezoneOffset();
														//这里转换成位标准时间,没有时区信息了.
														stime_stamp.value = testStartDate
																.getTime() / 1000;
														//Date.parse(startDateTextBox.datetimepicker('getDate'))/1000;
													}
												});
										//结束时间控件的属性设置
										endDateTextBox
												.datetimepicker({
													maxDate : 0,//不能选择未来
													controlType : 'select', //选择方式选时刻
													dateFormat : "yy-mm-dd", //日期格式
													//showSecond: true, //显示秒
													timeFormat : 'HH:mm', //时刻格式
													separator : ' ', //日期时刻分割字符(串)
													//showTimezone: true, //显示时区
													//timezone: '+0800',    //默认时区
													onClose : function(
															dateText, inst) {//关闭时候判断
														if (startDateTextBox
																.val() != '') {
															var testStartDate = startDateTextBox
																	.datetimepicker('getDate');
															var testEndDate = endDateTextBox
																	.datetimepicker('getDate');
															if (testStartDate > testEndDate)
																startDateTextBox
																		.datetimepicker(
																				'setDate',
																				testEndDate);
														} else {
															startDateTextBox
																	.val(dateText);
														}
														var testEndDate = endDateTextBox
																.datetimepicker('getDate');
														if (testStartDate != null)
															etime_stamp.value = testEndDate
																	.getTime() / 1000;
														//stime_stamp.value=
														//  Date.parse(startDateTextBox.datetimepicker('getDate'))/1000;
													},
													onSelect : function(
															selectedDateTime) {//选择时候防止误选
														startDateTextBox
																.datetimepicker(
																		'option',
																		'maxDate',
																		endDateTextBox
																				.datetimepicker('getDate'));

														var testEndDate = endDateTextBox
																.datetimepicker('getDate');
														tz2.value = testEndDate
																.getTimezoneOffset();
														etime_stamp.value = testEndDate
																.getTime() / 1000;
													}
												});
									</script>
         <!-- <button class="ui-button" id="btn" title="测试中...">子页面加载</button> -->
         <!-- <button class="ui-button" id="showLog" title="从服务器加载日志">查看日志</button> -->
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
            <tbody id=tr_dat></tbody>
         </table>
      </div>
   </div>
</body>
</html>
