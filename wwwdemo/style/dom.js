var page_num = 0;
var page = 0;
function nextpage()
{
	if(page_num < page-1)
		page_num +=1;
	document.getElementById("page_num").innerHTML =  page_num;
}

function pastpage()
{
	if(page_num > 0)
		page_num -=1;
	document.getElementById("page_num").innerHTML = page_num;
}

function xmlhttpPostyc() 
{
	var xmlHttpReq = false;
	var self = this;
	var strURL="/cgi-bin/yc.ajax";
    // Mozilla/Safari
	window.setTimeout("xmlhttpPostyc()", 3000);

    if (window.XMLHttpRequest) {
        self.xmlHttpReq = new XMLHttpRequest();
    }
    // IE
    else if (window.ActiveXObject) {
        self.xmlHttpReq = new ActiveXObject("Microsoft.XMLHTTP");
    }
    self.xmlHttpReq.open('POST', strURL, true);
    self.xmlHttpReq.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');
    self.xmlHttpReq.onreadystatechange = function() {
        if (self.xmlHttpReq.readyState == 4) {
            reloadYC(self.xmlHttpReq.responseText);
        }
    }
    self.xmlHttpReq.send("haha");
}

function reloadYC(str)
{
	var xmlDoc = new ActiveXObject("Microsoft.xmldom");
	 
	xmlDoc.async = "false";
	xmlDoc.loadXML(str);
	var mid=xmlDoc.getElementsByTagName("ID");
	var mvalue=xmlDoc.getElementsByTagName("VALUE");
	var mname=xmlDoc.getElementsByTagName("NAME");
	var mmax=xmlDoc.getElementsByTagName("MAX");
	var mmin=xmlDoc.getElementsByTagName("MIN");
	while(document.all.ycobject.firstChild)
	{
		document.all.ycobject.removeChild(document.all.ycobject.firstChild);
	}   
	var	tr=document.createElement('tr');   
	var	td=document.createElement('td');
	var	tablebody = document.createElement("TBODY");
				
	td.innerHTML='序号';
	td.style.border = "#ffffff 2px outset";		
	tr.appendChild(td);
		
	td=document.createElement('td');
	td.innerHTML='遥测名称';
	td.style.border = "#ffffff 2px outset";
	tr.appendChild(td);
		
	td=document.createElement('td');
	td.style.border = "#ffffff 2px outset";
	td.innerHTML='遥测值';
	tr.appendChild(td);
	
	td=document.createElement('td');
	td.innerHTML='值上限';
	td.style.border = "#ffffff 2px outset";
	tr.appendChild(td);
		
	td=document.createElement('td');
	td.innerHTML='值下限';
	td.style.border = "#ffffff 2px outset";
	tr.appendChild(td);
		
	tablebody.appendChild(tr);
	document.all.ycobject.appendChild(tablebody);
	page = parseInt((mid.length + 16) / 17);
	
	document.getElementById("page").innerHTML =  page;
	
	for(var i=page_num*17;(i< mid.length) && ((i-page_num*17) < 17);i++) 
	{

   
		tr=document.createElement('tr');   
		td=document.createElement('td');
		tablebody = document.createElement("TBODY");
		
		
		
		td.innerHTML=mid(i).text;
		td.style.backgroundColor="#FFFFFF";
		td.width = "50"; 
		tr.appendChild(td);
		
		td=document.createElement('td');
		td.innerHTML=mname(i).text;
		td.style.backgroundColor="#FFFFFF";
		td.width = "80";
		tr.appendChild(td);
		
		td=document.createElement('td');
		td.innerHTML=mvalue(i).text;
		td.style.backgroundColor="#FFFFFF";
		td.width = "80";
		tr.appendChild(td);
		
		td=document.createElement('td');
		td.innerHTML=mmax(i).text;
		td.style.backgroundColor="#FFFFFF";
		td.width = "80";
		//td.align="center";
		tr.appendChild(td);
		
		td=document.createElement('td');
		td.innerHTML=mmin(i).text;
		td.style.backgroundColor="#FFFFFF";
		td.width = "80";
		tr.appendChild(td);
		
		tablebody.appendChild(tr);
		//tab.appendChild(mytablebody);
		//var av = "" + "<a>" + i + "</a>" + "     " +"<a>" + mid(i).text + "</a>" + "<a>" + mvalue(i).text + "</a><a>hello</a>";
		//o.innerHTML=av;
		document.all.ycobject.appendChild(tablebody);
	}
}


  function xmlhttpPostyx() {
    var xmlHttpReq = false;
    var self = this;
    var strURL="/cgi-bin/yx.ajax";
    // Mozilla/Safari
    window.setTimeout("xmlhttpPostyx()", 3000);

    if (window.XMLHttpRequest) {
        self.xmlHttpReq = new XMLHttpRequest();
    }
    // IE
    else if (window.ActiveXObject) {
        self.xmlHttpReq = new ActiveXObject("Microsoft.XMLHTTP");
    }
    self.xmlHttpReq.open('POST', strURL, true);
    self.xmlHttpReq.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');
    self.xmlHttpReq.onreadystatechange = function() {
        if (self.xmlHttpReq.readyState == 4) {
            reloadYX(self.xmlHttpReq.responseText);
        }
    }
    self.xmlHttpReq.send("haha");
}
function reloadYX(str)
{
	var i;
	var xmlDoc = new ActiveXObject("Microsoft.xmldom");
	
	xmlDoc.async = "false";
	xmlDoc.loadXML(str);
	var mid=xmlDoc.getElementsByTagName("ID");
	var mvalue=xmlDoc.getElementsByTagName("VALUE");
	var mname=xmlDoc.getElementsByTagName("NAME");
	while(document.all.yxobject.firstChild)
	{
		document.all.yxobject.removeChild(document.all.yxobject.firstChild);
	}   
	var   tr=document.createElement('tr');   
	var   td=document.createElement('td');
	var tablebody = document.createElement("TBODY");
				
	td.innerHTML='序号';
	td.style.border = "#ffffff 2px outset";	
	td.width="5%";	
	tr.appendChild(td);
	
	td=document.createElement('td');
	td.innerHTML='遥信编码';
	td.style.border = "#ffffff 2px outset";
	td.align="center";
	tr.appendChild(td);
		
	td=document.createElement('td');
	td.innerHTML='遥信名称';
	td.style.border = "#ffffff 2px outset";
	td.align="center";
	tr.appendChild(td);
		
	td=document.createElement('td');
	td.style.border = "#ffffff 2px outset";
	td.width="10%";
	td.innerHTML='遥信值';
	td.align="center";
	tr.appendChild(td);
	
	td=document.createElement('td');
	td.innerHTML='取反标志';
	td.style.border = "#ffffff 2px outset";
	td.width="10%";	
	tr.appendChild(td);

		
	tablebody.appendChild(tr);
	document.all.yxobject.appendChild(tablebody);
	page = parseInt((mid.length + 16) / 17);
	
	document.getElementById("page").innerHTML =  page;
	// document.getElementById("x_y").value =  mid.length;
	//document.getElementById("x_y").value =  str;
	for(i=page_num*17;(i< mid.length) && ((i-page_num*17) < 17);i++) 
	{

   
		tr=document.createElement('tr');   
		td=document.createElement('td');
		tablebody = document.createElement("TBODY");
		
		
		
		td.innerHTML=mid(i).text;
		td.style.backgroundColor="#FFFFFF";
		tr.appendChild(td);
		
		td=document.createElement('td');
		td.innerHTML=mname(i).text;
		td.style.backgroundColor="#FFFFFF";
		td.align="center";
		tr.appendChild(td);
		
		td=document.createElement('td');
		td.innerHTML=mname(i).text;
		td.style.backgroundColor="#FFFFFF";
		td.align="center";
		tr.appendChild(td);

		td=document.createElement('td');
		//td.innerHTML=mvalue(i).text;
		if(mvalue(i).text==1)
		{
			font=document.createElement('font');
			font.color='#FFFFFF';
			font.innerHTML='合';
			td.style.backgroundColor='#FF0000';
		}
		else
		{
			font=document.createElement('font');
			font.color='#ffFFff';
			font.innerHTML='分';
			td.style.backgroundColor='#00FF00';
		}
		td.align="center";
		td.appendChild(font);
		
		
		tr.appendChild(td);
		
		td=document.createElement('td');
		td.innerHTML='否';
		td.style.backgroundColor="#FFFFFF";
		
		//td.align="center";
		tr.appendChild(td);
		

		
		tablebody.appendChild(tr);
		//tab.appendChild(mytablebody);
		//var av = "" + "<a>" + i + "</a>" + "     " +"<a>" + mid(i).text + "</a>" + "<a>" + mvalue(i).text + "</a><a>hello</a>";
		//o.innerHTML=av;
		document.all.yxobject.appendChild(tablebody);
	}
}  