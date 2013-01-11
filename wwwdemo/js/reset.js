/**
 * 重启页面用到的脚本
 */
var stime = document.getElementById("stime_stamp");
var etime = document.getElementById("etime_stamp");
var mtr_no = document.getElementById("mtr_no");
var form_load_monport ="/goform/load_monport_cfg";
var form_save_monport ="/goform/save_monport_cfg";

$(document).ready(function() { 
	/**
	 * 功能操作
	 */
	$("#btnResetPro").click(function() {
		$.post('/goform/reset',"OpType=1",
				function(result) {
			alert("操作完成");
		});
	});
	$("#btnResetWeb").click(function() {
		$.post('/goform/reset',"OpType=2",
				function(result) {
			alert("操作完成");
		});
	});
	$("#btnResetSample").click(function() {
		$.post('/goform/reset',"OpType=3",
				function(result) {
			alert("操作完成");
		});
	});
	$("#btnResetRtu").click(function() {
		$.post('/goform/reset',"OpType=4",
				function(result) {
			alert("操作完成");
		});
	});
	/**
	 * 对话框
	 */
	var monport_txt=document.getElementById("monport_text");
	$(".dialog").hide();
	$("#btn").click(function() {
		$.get("/um/compara1.asp", function(result) {
			$("#log").html(result);
		});
	});
	/**
	 * 历史电量查询post
	 */
	$("#btnPost").click(function() {
		// 显示
		//$("#tr_dat").html("");
		$("#msgbox_wait").show("fade", {}, 1);
		$.post('/goform/get_tou',
			$("#history_tou").serialize()+"&mtr_no="+$("#mtr_no").val(),
			function(result) {
				$("#tr_dat").html(result);
				// 动态加载完的页面才可以接收鼠标悬停等事件
				$("#history_tou tr").mouseover(function() {
					$(this).addClass("over");
				});
				$("#history_tou tr").mouseout(function() {
					$(this).removeClass("over");
				});
				// 完成之后隐藏
				$("#msgbox_wait").hide("fade", {}, 1000);
			});
	});
	/**
	 * 从服务器加载错误日志文件
	 */
	$("#load_log").click(function() {
		// 显示
		$("#log_wait").show("fade", {}, 1);
		$.post("/goform/load_log",
			"load",
			function(result) {
				$("#log_wait").hide("fade", {}, 1000);
				var b=document.getElementById("log_text");
				b.value=result;			
			});
	});
	/**
	 * 将文本保存到服务器的错误日志文件
	 */
	$("#save_log").click(function () {
		var b=document.getElementById("log_text");
		if (b.value==""){
			alert("文本不能为空");
		}else{
			//确认信息框
			$(function() {
				$( "#dialog-confirm" ).dialog({
					modal: true,
					buttons: {
						"保存": function() {
							$( this ).dialog( "close" );
							//alert("确认保存");
							$("#log_wait").show("fade", {}, 1);
							$.post("/goform/save_log",
								$("#log_text").val(),
								function(result) {
									// 完成之后隐藏
									$("#log_wait").hide("fade", {}, 1000);
								});
						},
						"取消": function() {
							$( this ).dialog( "close" );
							//alert("取消了");
						}
					}
				});
			});
		}
	});
	//   监视端口配置文件
	/**
	 * 从服务器加载 监视端口配置文件(端口文本描述)
	 */
	$("#load_monport").click(function() {
		$("#monprot_wait").show("fade", {}, 1);
		$.post(form_load_monport,
			"load_monport",
			function(result) {
				$("#monprot_wait").hide("fade", {}, 1000);		
				monport_txt.value=result;
				//$("#log_text").html("1231");
				//$("#log_text").html(result);
				// 完成之后隐藏
				
			});
	});
	/**
	 * 将文本保存到服务器的监视端口配置文件
	 */
	$("#save_monport").click(function () {
		if (monport_txt.value==""){
			alert("文本不能为空");
		}else{
			//确认信息框
			$(function() {
				$( "#dialog-confirm-monport" ).dialog({
					modal: true,
					//position: { my: "center", at: "center", of: window },
					buttons: {
						"保存": function() {
							$( this ).dialog( "close" );
							//alert("确认保存");
							$("#monprot_wait").show("fade", {}, 1);
							$.post(form_save_monport,
								$("#monport_text").val(),
								function(result) {
									// 完成之后隐藏
									$("#monprot_wait").hide("fade", {}, 1000);
								});
						},
						"取消": function() {
							$( this ).dialog( "close" );
							//alert("取消了");
							//return;
						}
					}
				});
			});
		}
	});
});
//文本提示信息,鼠标移上 显示提示信息
$(function() {
	$( document ).tooltip();
});
//重启按钮
$(function() {
    $( ".reboot" ).button({
      icons: {
        primary: "ui-icon-power"
      },
      text: false
    });
});
//标签页 标签id tabs
$(function() {
	var tabs = $( "#tabs" ).tabs();
	tabs.find( ".ui-tabs-nav" ).sortable({
		axis: "x",
		stop: function() {
			tabs.tabs( "refresh" );
		}
	});
});