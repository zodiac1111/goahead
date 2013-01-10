var MTR_ADD = 1;// 增删查改?...
var MTR_DEL = 2;
var MTR_SEARCH = 3;
var MTR_UPDATE = 4;
// var errobj=null;//当前有错误的配置项
function verifyInput(input) {
	if (input.nullable == "no") {
		if (isnull(input.value)) {
			alert(input.namefs + "不能为空");
			input.focus();
			return false;
		}
	}
	if (input.nullable == "yes") {verify_rtu_addr
		if (isnull(input.value)) {
			if (input.datatype == "number") {
				input.value = '0';
				return true;
			}
		}
	}
	if (input.nullable == "yesnoz") {
		if (isnull(input.value)) {
			if (input.datatype == "number") {
				input.value = '';
				return true;
			}
		}
	}
	switch (input.datatype) {
	case "number": {
		if (!isnumber(input.value)) {
			alert(input.namefs + "必须为数字!");
			input.value = '';
			input.focus();
			return false;
		}
	}
		break;
	case "string":
		break;
	default:
		break;
	}
	if (input.value.length > input.maxsize) {
		alert(input.namefs + "输入长度太长!");
		input.focus();
		return false;
	}
	return true;
}

/*--
 * type--0:插入一行  1:删除一行  2:更新  3:换页 4:删除选中的 5:搜索 6:下拉框换rtu号
 * noOfRow--当type==3, 5 ,6 时, 无实际意义
 */
function fdex(type, noOfRow) {
	document.paraform.optype.value = type;
	// alert(document.paraform.optype.value);
	if (3 != type && 5 != type && 6 != type) {
		document.paraform.indexno.value = noOfRow; // 传行号
	}
	return true;
}

// 刷新页面
function RefreshWin() {
	// alert('refresh');
	window.location = window.location;
	return true;
}
// 获得地址栏目输入参数值
function Request(strName) {
	var strHref = window.location.href;
	var intPos = strHref.indexOf("?");
	var strRight = strHref.substr(intPos + 1);

	var arrTmp = strRight.split("&");
	for ( var i = 0; i < arrTmp.length; i++) {
		var arrTemp = arrTmp[i].split("=");

		if (arrTemp[0].toUpperCase() == strName.toUpperCase())
			return arrTemp[1];
	}
	return "";
}

/* 检测字符串是否为空 */
function isnull(str) {
	var i;
	for (i = 0; i < str.length; i++) {
		if (str.charAt(i) != ' ')
			return false;
	}
	return true;
}

/* 检测字符串是否全为数字--开头可以是'-' */
function isnumber(str) {
	var number_chars = "1234567890";
	var i;
	if (number_chars.indexOf(str.charAt(0)) == -1 && str.charAt(0) != '-') {
		return false;
	}
	if (str.length == 1 && str.charAt(0) == '-') {
		return false;
	}
	for (i = 1; i < str.length; i++) {
		if (number_chars.indexOf(str.charAt(i)) == -1)
			return false;
	}
	return true;
}
// 到前页或后页 -1:前
function GoForNPage(ward) {
	var curpagevalue = document.paraform.pageselect.value;
	var PageCount = 1;
	if (ward == -1) {
		document.paraform.pageselect.value = curpagevalue
				- (curpagevalue == 0 ? 0 : 1);
		// 往 向server端传页码的控件写页号(0,1...)
		document.paraform.pageindex.value = document.paraform.pageselect.value;
		// alert("to pageIndex"+document.paraform.pageselect.value);
	} else if (ward == 1) {
		PageCount = document.paraform.pageselect.count;
		if (PageCount - 1 > (document.paraform.pageselect.value)) {
			document.paraform.pageselect.value++;
		}
		// 往 向server端传页码的控件写页号(0,1...)
		document.paraform.pageindex.value = document.paraform.pageselect.value;
		// alert("to pageIndex"+document.paraform.pageselect.value);
	}
	if (null != document.paraform.pageindex.value
			&& document.paraform.pageindex.value != "") {
		fdex(3, -1); // 准备提交换页命令
		document.paraform.submit();
	}

}

function DelSubmit() {
	fdex(4, -1);
	document.paraform.submit();
}

function Redirect(page) {
	window.location = page;
}

function PageChange() {
	document.paraform.pageindex.value = document.paraform.pageselect.value;
	fdex(3, -1);
	document.paraform.submit();
}

// yc, yx,kwh等页 RTU下拉框触发函数
function RtuChange() {
	document.paraform.rtuNo.value = document.paraform.RtuSelect.value;
	fdex(6, -1); // 填操作类型
	document.paraform.submit();
}

/*******************************************************************************
 * 根据text选择下拉框的当前值
 */
// 恢复select下拉列表当前选中的值
function LoadSelectByText(obj, text) {
	for ( var i = 0; i < obj.options.length; i++) {
		if (obj.options[i].text == text) {
			obj.selectedIndex = i;
			break;
		}
	}
}
/*******************************************************************************
 * 根据value选择下拉框的当前值
 */
// 恢复select下拉列表当前选中的值
function LoadSelectByValue(obj, value) {
	for ( var i = 0; i < obj.options.length; i++) {
		if (obj.options[i].value == value) {
			obj.selectedIndex = i;
			break;
		}
	}
}
// 根据名和行号得到对象 (对象id="namerow")
function GetObj(name, row) {
	var obj = document.getElementById(name + row);
	return obj;
}
// 提交登出
function logout() {
	alert('您已经操时,请重新登录!');
	parent.frames[1].document.forms[0].submit();
}

// 判断是否选择了待更新项,若选中就提交,否则返回.11y27
function UpdateTheSelect() {
	var selobj;
	var num = 0;
	for ( var i = 0; i < 17; i++) {
		selobj = GetObj('selCheck', i);
		if (selobj && selobj.checked == true) {
			// alert('selCheck'+i+'checked');
			num++;
		}
	}
	if (num > 0) {
		fdex(2, -1);
		document.paraform.submit();
	} else {
		alert("请务必选中要更新的行!");
		return false;
	}
}

// 全选或全取消函数11y28
function AllSelOrFree() {
	var i = 0;
	var aflag = document.getElementById('AllSelFlag');
	if (aflag && aflag.value == 1) {// 已经全选,做全取消操作
		aflag.value = 0; // 置标
		while ((checkObj = GetObj("selCheck", i++)) != null) {
			checkObj.checked = false;
		}

	} else {
		aflag.value = 1; // 置标
		while ((checkObj = GetObj("selCheck", i++)) != null) {
			checkObj.checked = true;
		}
	}
}
// 数据库操作
function db_add()// 增
{
	document.paraform.optype.value = MTR_ADD;
	document.paraform.submit();
}
function db_del()// 删
{
	document.paraform.optype.value = MTR_DEL;
	document.paraform.submit();
}
function db_update()// 改
{
	var errobj = document.getElementById("errobj");
	if (errobj != null) {
		alert("非法参数");
		return;
	}
	document.paraform.optype.value = MTR_UPDATE;
	document.paraform.submit();
}
/**
 * 联动 表计参数全部有效标识
 */
function iv_all_click(e) {
	// e就是事件源对象,src是arguments[0]是Firefox的方法,window.event是IE的方法 \
	// chrome or ff no ie
	var curobj = e.srcElement || e.target;
	// var curobj=e.target;
	// chrome
	// var curobj=window.event.target
	var chklist = document.getElementsByName("iv_check");
	var chktxt = document.getElementsByName("iv");
	var i = 0;
	var n = chklist.length;
	for (i = 0; i < n; i++) {
		chklist[i].checked = curobj.checked;
		chktxt[i].value = (curobj.checked) ? 1 : 0;
	}
}
// 点击某一个表参数的有效标识,复选框和文本框联动,文本框用于post.
// chkbox控制txt控件值0或1,txt是chkbox的兄弟元素,
// parent--chk_box (子元素1)
// `-input(txt) (子元素2)
function chk_change(e) {
	// 获取被点击的对象,有很多个name为ivchk的chkbox,这样区分
	// var curobj=this.event.target;
	var curobj = e.srcElement || e.target;
	if (curobj.checked) {
		// alert("1");
		// ivchk.checked=false;
		curobj.parentElement.children[1].value = 1;
	} else {
		// alert("2");
		curobj.parentElement.children[1].value = 0;
		// ivchk.checked=true;
	}
}
// 表计类型,几相几线联动
function type_all_changed(e) {
	var curobj = e.srcElement || e.target;
	var typelist = document.getElementsByName("ph_wire");
	var i = 0;
	var n = typelist.length;
	for (i = 0; i < n; i++) {
		typelist[i].value = curobj.value;
	}
}
// 串口方案,所有表计联动.
function changeall_sioplan(e) {
	var curobj = e.srcElement || e.target;
	var typelist = document.getElementsByName("portplan");
	var i = 0;
	var n = typelist.length;
	for (i = 0; i < n; i++) {
		typelist[i].value = curobj.value;
	}
}
// 统一设置所有电表的"表计厂家"参数.
function setall_factory(e) {
	var curobj = e.srcElement || e.target;
	var typelist = document.getElementsByName("factory");
	var i = 0;
	var n = typelist.length;
	for (i = 0; i < n; i++) {
		typelist[i].value = curobj.value;
	}
}
// 表计规约全选联动
function changeall_mtr_protocol(e) {
	var curobj = e.srcElement || e.target;
	var typelist = document.getElementsByName("protocol");
	var i = 0;
	var n = typelist.length;
	for (i = 0; i < n; i++) {
		typelist[i].value = curobj.value;
	}
}
// 所有电量小数 联动
function all_it_dot_changed(e) {
	var curobj = e.srcElement || e.target;
	var it_dotlist = document.getElementsByName("it_dot");
	var rNums = /^[0-9]{1}$/;
	curobj.style.color = "green";// 数值改变了,字体变绿色
	if (!rNums.test(curobj.value)) {// 数值错误:
		curobj.style.backgroundColor = "red";
		curobj.id = "errobj";
		return;
	} else {// 正确则恢复
		curobj.style.backgroundColor = "";
		curobj.id = "";
	}
	var i = 0;
	var n = it_dotlist.length;
	for (i = 0; i < n; i++) {
		it_dotlist[i].value = curobj.value;
		// it_dotlist[i].onchange(e);
	}
}
// 使所有需量小数可以联动
function all_xl_dot_changed(e) {
	var curobj = e.srcElement || e.target;
	var xl_dotlist = document.getElementsByName("xl_dot");
	var rNums = /^[0-9]{1}$/;
	curobj.style.color = "green";// 数值改变了,字体变绿色
	if (!rNums.test(curobj.value)) {// 数值错误:
		curobj.style.backgroundColor = "red";
		curobj.id = "errobj";
		return;
	} else {// 正确则恢复
		curobj.style.backgroundColor = "";
		curobj.id = "";
	}
	var i = 0;
	var n = xl_dotlist.length;
	for (i = 0; i < n; i++) {
		xl_dotlist[i].value = curobj.value;
	}
}
// 使所有电压小数可以联动
function all_v_dot_changed(e) {
	var curobj = e.srcElement || e.target;
	var xl_dotlist = document.getElementsByName("v_dot");
	var rNums = /^[0-9]{1}$/;
	curobj.style.color = "green";// 数值改变了,字体变绿色
	if (!rNums.test(curobj.value)) {// 数值错误:
		curobj.style.backgroundColor = "red";
		curobj.id = "errobj";
		return;
	} else {// 正确则恢复
		curobj.style.backgroundColor = "";
		curobj.id = "";
	}
	var i = 0;
	var n = xl_dotlist.length;
	for (i = 0; i < n; i++) {
		xl_dotlist[i].value = curobj.value;
	}
}
// 设置所有电流小数位
function all_i_dot_changed(e) {
	var curobj = e.srcElement || e.target;
	var xl_dotlist = document.getElementsByName("i_dot");
	var rNums = /^[0-9]{1}$/;
	curobj.style.color = "green";// 数值改变了,字体变绿色
	if (!rNums.test(curobj.value)) {// 数值错误:
		curobj.style.backgroundColor = "red";
		curobj.id = "errobj";
		return;
	} else {// 正确则恢复
		curobj.style.backgroundColor = "";
		curobj.id = "";
	}
	var i = 0;
	var n = xl_dotlist.length;
	for (i = 0; i < n; i++) {
		xl_dotlist[i].value = curobj.value;
	}
}
function all_p_dot_changed(e) {
	var curobj = e.srcElement || e.target;
	var xl_dotlist = document.getElementsByName("p_dot");
	var rNums = /^[0-9]{1}$/;
	curobj.style.color = "green";// 数值改变了,字体变绿色
	if (!rNums.test(curobj.value)) {// 数值错误:
		curobj.style.backgroundColor = "red";
		curobj.id = "errobj";
		return;
	} else {// 正确则恢复
		curobj.style.backgroundColor = "";
		curobj.id = "";
	}
	var i = 0;
	var n = xl_dotlist.length;
	for (i = 0; i < n; i++) {
		xl_dotlist[i].value = curobj.value;
	}
}
function all_q_dot_changed(e) {
	var curobj = e.srcElement || e.target;
	var xl_dotlist = document.getElementsByName("q_dot");
	var rNums = /^[0-9]{1}$/;
	curobj.style.color = "green";// 数值改变了,字体变绿色
	if (!rNums.test(curobj.value)) {// 数值错误:
		curobj.style.backgroundColor = "red";
		curobj.id = "errobj";
		return;
	} else {// 正确则恢复
		curobj.style.backgroundColor = "";
		curobj.id = "";
	}
	var i = 0;
	var n = xl_dotlist.length;
	for (i = 0; i < n; i++) {
		xl_dotlist[i].value = curobj.value;
	}
}
function all_ue_changed(e) {
	var curobj = e.srcElement || e.target;
	var xl_dotlist = document.getElementsByName("ue");
	var rNums = /^[0-9]*$/;
	curobj.style.color = "green";// 数值改变了,字体变绿色
	if (!rNums.test(curobj.value)) {// 数值错误:
		curobj.style.backgroundColor = "red";
		curobj.id = "errobj";
		return;
	} else {// 正确则恢复
		curobj.style.backgroundColor = "";
		curobj.id = "";
	}
	var i = 0;
	var n = xl_dotlist.length;
	for (i = 0; i < n; i++) {
		xl_dotlist[i].value = curobj.value;
	}
}
function all_ie_changed(e) {
	var curobj = e.srcElement || e.target;
	var xl_dotlist = document.getElementsByName("ie");
	var rNums = /^[0-9]*$/;
	curobj.style.color = "green";// 数值改变了,字体变绿色
	if (!rNums.test(curobj.value)) {// 数值错误:
		curobj.style.backgroundColor = "red";
		curobj.id = "errobj";
		return;
	} else {// 正确则恢复
		curobj.style.backgroundColor = "";
		curobj.id = "";
	}
	var i = 0;
	var n = xl_dotlist.length;
	for (i = 0; i < n; i++) {
		xl_dotlist[i].value = curobj.value;
	}
}
// 线路名称
function line_changed(e) {
	var curobj = e.srcElement || e.target;
	var rNums = /^[0-9]{1,6}$/;
	curobj.style.color = "green";// 数值改变了,字体变绿色
	if (!rNums.test(curobj.value)) {// 数值错误:
		curobj.style.backgroundColor = "red";
		curobj.id = "errobj";
	} else {// 正确则恢复
		curobj.style.backgroundColor = "";
		curobj.id = "";
	}
}
// 地址
function addr_changed(e) {
	var curobj = e.srcElement || e.target;
	var rNums = /^[0-9]{1,12}$/;
	curobj.style.color = "green";// 数值改变了,字体变绿色
	if (!rNums.test(curobj.value)) {// 数值错误:
		curobj.style.backgroundColor = "red";
		curobj.id = "errobj";
	} else {// 正确则恢复
		curobj.style.backgroundColor = "";
		curobj.id = "";
	}
}
// 口令
function pwd_changed(e) {
	var curobj = e.srcElement || e.target;
	var rNums = /^[0-9]{1,12}$/;
	curobj.style.color = "green";// 数值改变了,字体变绿色
	if (!rNums.test(curobj.value)) {// 数值错误:
		curobj.style.backgroundColor = "red";
		curobj.id = "errobj";
	} else {// 正确则恢复
		curobj.style.backgroundColor = "";
		curobj.id = "";
	}
}
// 各种物理量的小数位数的改变,检测.小数位数一定是一个数字.
function dot_changed(e) {
	var curobj = e.srcElement || e.target;
	var rNums = /^[0-9]$/;
	curobj.style.color = "green";// 数值改变了,字体变绿色
	if (!rNums.test(curobj.value)) {// 数值错误:
		curobj.style.backgroundColor = "red";
		curobj.id = "errobj";
	} else {// 正确则恢复
		curobj.style.backgroundColor = "";
		curobj.id = "";
	}
}
// 电压
function ue_changed(e) {
	var curobj = e.srcElement || e.target;
	var rNums = /^[0-9]{1,}$/;
	curobj.style.color = "green";// 数值改变了,字体变绿色
	if (!rNums.test(curobj.value)) {// 数值错误:
		curobj.style.backgroundColor = "red";
		curobj.id = "errobj";
	} else {// 正确则恢复
		curobj.style.backgroundColor = "";
		curobj.id = "";
	}
}
// 电流
function ie_changed(e) {
	var curobj = e.srcElement || e.target;
	var rNums = /^[0-9]{1,}$/;
	curobj.style.color = "green";// 数值改变了,字体变绿色
	if (!rNums.test(curobj.value)) {// 数值错误:
		curobj.style.backgroundColor = "red";
		curobj.id = "errobj";
	} else {// 正确则恢复
		curobj.style.backgroundColor = "";
		curobj.id = "";
	}
}
// 数值小于256 0~255,有用是标识表计/串口等的数量,所以0也是不和逻辑的.
function lessthan1byte(e) {
	var curobj = e.srcElement || e.target;
	var rNums = /^[0-9]{1,3}$/;
	curobj.style.color = "green";// 数值改变了,字体变绿色
	// 数值错误: 0也是不合逻辑的
	if (!rNums.test(curobj.value) || curobj.value <= 0 || curobj.value >= 256) {
		curobj.style.backgroundColor = "red";
		curobj.id = "errobj";
	} else {// 正确则恢复
		curobj.style.backgroundColor = "";
		curobj.id = "";
	}
}
// 验证终端地址 [0,255]
function verify_rtu_addr(e) {
	var curobj = e.srcElement || e.target;
	var rNums = /^[0-9]{1,4}$/;
	curobj.style.color = "green";// 数值改变了,字体变绿色
	// 数值错误:
	if (!rNums.test(curobj.value) || curobj.value < 0) {
		curobj.style.backgroundColor = "red";
		curobj.id = "errobj";
	} else {// 正确则恢复
		curobj.style.backgroundColor = "";
		curobj.id = "";
	}
}
// 验证监视参数的 转发数目
function verify_forward_mtr_num(e) {
	var curobj = e.srcElement || e.target;
	var rNums = /^[0-9]{1,3}$/;
	curobj.style.color = "green";// 数值改变了,字体变绿色
	// 数值错误:
	if (!rNums.test(curobj.value) || curobj.value < 0 || curobj.value > 255) {
		curobj.style.backgroundColor = "red";
		curobj.id = "errobj";
	} else {// 正确则恢复
		curobj.style.backgroundColor = "";
		curobj.id = "";
	}
}
// 验证网络端口是否合法 [0,65535] 之间
function verify_port(e) {
	var curobj = e.srcElement || e.target;
	var rNums = /^[0-9]{1,5}$/;
	curobj.style.color = "green";// 数值改变了,字体变绿色
	// 数值错误:
	if (!rNums.test(curobj.value) || curobj.value < 0 || curobj.value > 65535) {
		curobj.id = "errobj";
		curobj.style.backgroundColor = "red";
	} else {// 正确则恢复
		curobj.id = "";
		curobj.style.backgroundColor = "";
	}
}
// 判断是不是IP
function isIPv4(e) {
	var curobj = e.srcElement || e.target;
	var rNums = /^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$/;
	curobj.style.color = "green";// 数值改变了,字体变绿色
	// 数值错误: 0也是不合逻辑的
	if (!rNums.test(curobj.value) || curobj.value <= 0 || curobj.value >= 256) {
		curobj.id = "errobj";
		curobj.style.backgroundColor = "red";
	} else {// 正确则恢复
		curobj.id = "";
		curobj.style.backgroundColor = "";
	}
}
// 重启web服务器
function reset_web() {
	document.paraform.optype.value = 1;
	document.paraform.submit();
}
// 重新初始化规约文件,修改规约文件后可以点击
function reset_procotol() {
	document.paraform.optype.value = 2;
	document.paraform.submit();
}
// 重启采集程序
function reset_sample() {
	document.paraform.optype.value = 3;
	document.paraform.submit();
}
// 重启终端
function reset_rtu() {
	document.paraform.optype.value = 4;
	document.paraform.submit();
}
// 重启程序测试,用于测试服务端
function reset_test() {
	document.paraform.optype.value = 10;
	document.paraform.submit();
}
