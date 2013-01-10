<html>
<head>
<title>时间显示</title>
<meta http-equiv="Content-Type" content="text/html; charset=gb2312">
<script type="text/javascript" src="/style/jquery-1.8.3.min.js"></script>
<script type="text/javascript" src="/style/jquery-ui.min.js"></script>
<script type="text/javascript" src="/style/jquery-ui-sliderAccess.js"></script>
<script type="text/javascript" src="/style/jquery-ui-timepicker-addon.js"></script>
<script type="text/javascript" src="/style/jquery-ui-timepicker-zh-CN.js"></script>
<style type="text/css">
<!--
.style1 {color: #00FFFF}
-->
</style>
</head> 
<body bgcolor="778899" >
<SCRIPT language=javascript >
function Year_Month(){ 
    var now = new Date(); 
    var yy = now.getFullYear(); 
    var mm = now.getMonth(); 
    var mmm=new Array();
    mmm[0]="1月";
    mmm[1]="2月";
    mmm[2]="3月";
    mmm[3]="4月";
    mmm[4]="5月";
    mmm[5]="6月";
    mmm[6]="7月";
    mmm[7]="8月";
    mmm[8]="9月";
    mmm[9]="10月";
    mmm[10]="11月";
    mmm[11]="12月";
    mm=mmm[mm];
    return(mm ); 
}
    
function thisYear(){ 
    var now = new Date(); 
    var yy = now.getFullYear(); 
    return(yy); }
    
function Date_of_Today(){ 
    var now = new Date(); 
    return(now.getDate() ); }
    
function Date_of_week(){
	  var now=new Date(); 
	  var w; 
if(now.getDay()==0)  w="星期日";
if(now.getDay()==1)  w="星期一";
if(now.getDay()==2)  w="星期二";
if(now.getDay()==3)  w="星期三";
if(now.getDay()==4)  w="星期四";
if(now.getDay()==5)  w="星期五";
if(now.getDay()==6)  w="星期六";  
	  return(w);}    
    
function CurentTime(){ 
    var now = new Date(); 
    var hh = now.getHours(); 
    var mm = now.getMinutes(); 
    var ss = now.getTime() % 60000; 
    ss = (ss - (ss % 1000)) / 1000; 
    var clock = hh+':'; 
    if (mm < 10) clock += '0'; 
    clock += mm+':'; 
    if (ss < 10) clock += '0'; 
    clock += ss; 
    return(clock); } 

function CurenDate(){
	var now=new Date();
	var year=thisYear();
	var month=Year_Month();
	var Today=Date_of_Today();
	var week=Date_of_week();
	var day='电脑时间：'+year+'年'+month+Today+'日  '+week;
	return(day);	
	}
    
function refreshCalendarClock(){ 
document.all.calendarClock1.innerHTML = CurenDate();
document.all.calendarClock2.innerHTML = CurentTime(); 
}

document.write('<span id="calendarClock1" class="style1"> </span>&nbsp;');
document.write('<span id="calendarClock2" class="style1"> </span>');
setInterval('refreshCalendarClock()',1000);
</script>
<span class="style1">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;服务器时间:
<% server_time(); %></span>



</body>
</html>
