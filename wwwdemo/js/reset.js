/**
 * 重启页面用到的脚本
 */
var stime = document.getElementById("stime_stamp");
var etime = document.getElementById("etime_stamp");
var mtr_no = document.getElementById("mtr_no");
$(document).ready(function() {
	$("#btn").click(function() {
		$.get("/um/compara1.asp", function(result) {
			$("#log").html(result);
		});
	});
	$("#btnPost").click(function() {
		// 显示
		$("#msgbox_wait").show("fade", {}, 1);
		$.post('/goform/get_tou',
			$("#history_tou").serialize(),
			function(result) {
				$("#tr_dat").html(result);
				// 动态加载完的页面才可以接收鼠标悬停等事件
				$("#history_tou tr").mouseover(function() {
					$(this).addClass("over");
					var rows = $(this).attr('relrow');
				});
				$("#history_tou tr").mouseout(function() {
					$(this).removeClass("over");
				});
				// 完成之后隐藏
				$("#msgbox_wait").hide("fade", {}, 1);
				$("#msgbox_ok").show("fade", {}, 1);
				$("#msgbox_ok").hide("fade", {}, 1000);
			});
	});
	$("#showLog").click(function() {
		// 显示
		$("#msgbox_wait").show("fade", {}, 1);
		$.post('/goform/get_tou',
			$("#history_tou").serialize(),
			function(result) {
				$("#tr_dat").html(result);
				// 动态加载完的页面才可以接收鼠标悬停等事件
				$("#history_tou tr").mouseover(function() {
					$(this).addClass("over");
					var rows = $(this).attr('relrow');
				});
				$("#history_tou tr").mouseout(function() {
					$(this).removeClass("over");
				});
				// 完成之后隐藏
				$("#msgbox_wait").hide("fade", {}, 1);
				$("#msgbox_ok").show("fade", {}, 1);
				$("#msgbox_ok").hide("fade", {}, 1000);
			});
	});
	$("#save_log").click(function (e) {
		$.post('/goform/get_tou',
			$("#log_text").val(),
			function(result) {
				$("#tr_dat").html(result);
				// 动态加载完的页面才可以接收鼠标悬停等事件
				$("#history_tou tr").mouseover(function() {
					$(this).addClass("over");
					var rows = $(this).attr('relrow');
				});
				$("#history_tou tr").mouseout(function() {
					$(this).removeClass("over");
				});
				// 完成之后隐藏
				$("#msgbox_wait").hide("fade", {}, 1);
				$("#msgbox_ok").show("fade", {}, 1);
				$("#msgbox_ok").hide("fade", {}, 1000);
			});
		//e.preventDefault();  //stop the browser from following
		//show.document.execCommand("SaveAs");
		//window.location.href = "/err.log";//文件位置
	});

});
