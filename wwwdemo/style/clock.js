
function Year_Month(){ powered by 25175.net
    var now = new Date(); 
    var yy = now.getYear(); 
    var mm = now.getMonth(); 
    var mmm=new Array();
    mmm[0]="January";
    mmm[1]="February ";
    mmm[2]="March";
    mmm[3]="April";
    mmm[4]="May";
    mmm[5]="June";
    mmm[6]="July";
    mmm[7]="August";
    mmm[8]="September";
    mmm[9]="October";
    mmm[10]="November";
    mmm[11]="December";
    mm=mmm[mm];
    return(mm ); }
function thisYear(){ 
    var now = new Date(); 
    var yy = now.getYear(); 
    return(yy ); }
function Date_of_Today(){ 
    var now = new Date(); 
    return(now.getDate() ); }
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
function refreshCalendarClock(){ 
document.all.calendarClock1.innerHTML = Year_Month(); 
document.all.calendarClock2.innerHTML = Date_of_Today(); 
document.all.calendarClock3.innerHTML =thisYear(); 
document.all.calendarClock4.innerHTML = CurentTime(); }
document.write('<font id="calendarClock1" > </font>&nbsp;');
document.write('<font id="calendarClock2" > </font>,');
document.write('<font id="calendarClock3" > </font>&nbsp;');
document.write('<font id="calendarClock4" > </font>');
setInterval('refreshCalendarClock()',1000);