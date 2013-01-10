/*
 * menuExpandable3.js - implements an expandable menu based on a HTML list
 * Author: Dave Lindquist (http://www.gazingus.org)
 */

if (!document.getElementById)
    document.getElementById = function() { return null; }

// actuatorId  触发menuId的状态变化
function initializeMenu(menuId, actuatorId) {
    var menu = document.getElementById(menuId);
    var actuator = document.getElementById(actuatorId);

    if (menu == null || actuator == null) return;

    //if (window.opera) return; // I'm too tired

    actuator.parentNode.style.backgroundImage = "url(/images/minus.gif)";
    actuator.onclick = function() {
        var display = menu.style.display;
        this.parentNode.style.backgroundImage =
            (display == "block") ? "url(/images/plus.gif)" : "url(/images/minus.gif)";
        menu.style.display = (display == "block") ? "none" : "block";//变化显示状态
		 
        return false;
    }
}

function  CheckIt()
{
	alert('check it'); 
	return false;  
}

//--------------右页面切换函数-----------------------  
 
	function ResetStatus(strInfo)
	{
		window.status=""; return true;	
	}
	
	function OnHrefClick(mainID,subID)	//主菜单和子菜单index
	{
		alert("href it");
		if(mainID==null)
			mainID='A';
		if(subID==null)
			subID=1;
		if('K'==mainID)
			window.open('/frames/tp-link.htm','','width=680,height=520,top=130,left=130,scrollbars=yes');
		var MenuHtml ='/userRpm/MenuRpm.htm' + '?MainID=' + mainID + '&SubID=' + subID;
		
		//window.location.href =MenuHtml;
		window.location.href = 'about.htm' ;
		 
		return false;
	}
	
//var MainFrameHtml  ='/userRpm/VirtualServerRpm.htm';
//parent.mainFrame.location.href=MainFrameHtml;
 
