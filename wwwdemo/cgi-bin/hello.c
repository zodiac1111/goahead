#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
char **getCGIvars(void);
char *gogetenv(char *varname);
int strcmpci(char *s1, char *s2);
void unescape_url(char *url);
char x2c(char *what);
int MeterNo=0;
int Decimal=0;
int main(int argc,char* argv[])
{
	char	**pVars;//指向一个变量名和一个变量值的组合字符数组.
	char	**p;
	int i=0;//一共有几组(个)变量
	pVars = getCGIvars();//得到变量和值 
	p	=pVars;
/*	
 *	Print the HTML header to STDOUT.

    printf("<html>\n");
    printf("<head><title>CGI Output</title></head>\n");
    printf("<body>\n"); */
	if (pVars) {
		while (*pVars&&*(pVars+1)) { //v[0]变量,v[0+1],值
			//printf("<p>%s = <b>%s</b></p>\n", *pVars, *(pVars+1));
			pVars += 2;
			i++;
		}
	} else {
		//printf("<P>No query variables found</P>\n");
	}
MeterNo = atoi(p[1]);
Decimal=atoi(p[3]);
long line_name = atoi(p[5]);
int mtr_addr=atoi(p[7]);
int v_deci = atoi(p[9]);
int fact =atoi(p[11]);
   printf(
"<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \n"
"	\"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">\n"
"<html xmlns=\"http://www.w3.org/1999/xhtml\">\n"
"<head>\n"
"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=ISO-639-1\">\n"
"<link href=\"/style/goahead.css\" rel=\"stylesheet\" type=\"text/css\" media=\"screen\" />\n"
"<link rel=\"shortcut icon\" href=\"favicon.ico\">\n"
"\n"
"<base target=\"websframe\"/>\n"
"<title>杭州华立电力</title>\n"
"</head>\n"
"\n"
"<body > \n"
"<img src=\"/graphics/topbluebar.gif\" width=\"100%\" height=\"20\"><br/><br/>\n"
"<p class=\"secondmenu\" >表计参数维护界面</p>\n"
"<p class=\"topmenu\">PeerSec Networks MatrixSSL&trade; Enabled</p> \n"
"\n"
"<form action=/cgi-bin/a.out method=POST>\n"
"<table  width=\"100%\" height=\"380\" border=\"0\" background=\"graphics/7.gif\">\n"
"<tr>\n"
"	<td>表号:</td>\n"
"	<td><input type=text name=name size=10 value=\"%d\"></td>\n"
"	<td>电量小数:</td>\n"
"	<td><input type=text name=name size=10 value=\"%d\">\n"
"  	</td>\n"
"	<td>线路名称:</td>\n"
"	<td><input type=text name=address size=10 value=\"%d\"></td>\n"
"</tr>\n"
"<tr>\n"
"	<td>表计地址:</td>\n"
"	<td><input type=text name=address size=12 value=\"%d\"></td>\n"
"	<td>电压小数:</td>\n"
"	<td><input type=text name=name size=20 value=\"%d\">\n"
"  </td>\n"
"	<td>生产厂家:</td>\n"
"	<td><select name=\"生产厂家\">\n"
"      <option value=\"0\"selected=\"selected\">华立</option>\n"
"      <option value=\"1\">威胜</option>\n"
"      <option value=\"2\"></option>\n"
"      <option value=\"3\">3</option>\n"
"      <option value=\"4\">4</option>\n"
"      </select></td>\n"
"</tr>\n"
"<tr>\n"
"	<td>使用端口:</td>\n"
"	<td><select name=\"使用端口\">\n"
"      <option value=\"2\"selected=\"selected\">com3</option>\n"
"      <option value=\"3\">com4</option>\n"
"      <option value=\"4\">com5</option>\n"
"      <option value=\"5\">com6</option>\n"
"      <option value=\"6\">com7</option>\n"
"      <option value=\"7\">com8</option>      \n"
"      </select>\n"
"  </td>\n"
"	<td>电流小数:</td>\n"
"	<td><input type=text name=name size=20 value=\"\">\n"
"  </td>\n"
"	<td>表计口令:</td>\n"
"	<td><input type=text name=address size=20 value=\"\"></td>\n"
"</tr>\n"
"<tr>\n"
"	<td>串口方案:</td>\n"
"	<td><select name=\"串口方案\">\n"
"      <option value=\"0\"selected=\"selected\">方案0</option>\n"
"      <option value=\"1\">方案1</option>\n"
"      <option value=\"2\">方案2</option>\n"
"      <option value=\"3\">方案3</option>\n"
"      <option value=\"3\">方案4</option>\n"
"      <option value=\"3\">方案5</option>\n"
"      <option value=\"3\">方案6</option>\n"
"      </select>\n"
"  </td>\n"
"	<td>有功功率小数:</td>\n"
"	<td><input type=text name=name size=20 value=\"\">\n"
"	</td>  \n"
"	<td>电表类型:</td>\n"
"	<td><select name=\"电表类型\">\n"
"      <option value=\"0\" selected=\"selected\">3相3线</option>\n"
"      <option value=\"1\">3相4线</option>\n"
"      </select>\n"
"  </td>\n"
"</tr>\n"
"<tr>\n"
"	<td>表计规约:</td>\n"
"	<td><select name=\"标记规约\">\n"
"      <option value=\"0\">DLT-07</option>\n"
"      <option value=\"1\">DLT-97</option>\n"
"      <option value=\"2\">方案2</option>\n"
"      <option value=\"3\">方案3</option>\n"
"      <option value=\"3\">方案4</option>\n"
"      <option value=\"3\">方案5</option>\n"
"      <option value=\"3\">方案6</option>\n"
"      </select>\n"
"	</td>\n"
"	<td>无功功率小数:</td>\n"
"	<td><input type=text name=name size=20 value=\"\">\n"
"  </td>\n"
"	<td>额定电压:</td>\n"
"	<td><select name=\"额定电压\">\n"
"      <option value=\"0\">57.7伏</option>\n"
"      <option value=\"1\" selected=\"selected\">100伏</option>\n"
"      </select>\n"
"  </td>\n"
"  </tr>\n"
"  <tr>\n"
"	<td>有效标识:</td>\n"
"	<td><select name=\"有效标识\">\n"
"      <option value=\"0\"selected=\"selected\">有效</option>\n"
"      <option value=\"1\">无效</option>\n"
"      </select>\n"
"	</td>\n"
"	<td>需量小数:</td>\n"
"	<td><input type=text name=name size=20 value=\"\"></td>\n"
"	<td>额定电流:</td>\n"
"	<td><select name=\"额定电流\">\n"
"      <option value=\"0\"selected=\"selected\">1安</option>\n"
"      <option value=\"1\">5安</option>\n"
"      </select>\n"
"  </td>\n"
"  </tr>\n"
"  <tr>\n"
"       <td>\n"
"       </td>\n"
"         <td>\n"
"       </td>\n"
"      <td ALIGN=\"CENTER\"> \n"
"        <input type=submit name=ok value=\"设置参数\"> <input type=submit name=ok value=\"读取参数\"> <input type=submit name=ok value=\"取消\">\n"
"      </td>\n"
"  </tr>\n"
""
"\n"
"</form>\n"
"<br clear=all>\n"
"<img src=\"graphics/topbluebar.gif\" width=\"100%\" height=\"20\"> <br/>\n"
"</body>\n"
"</html>\n",MeterNo,Decimal,line_name,mtr_addr,v_deci,fact
);

}
char **getCGIvars(void)
{
	register int i;
	char *request_method, *cgiinput, *nvpair, *eqpos;
	int content_length, paircount;
	char **cgivars, **pairlist;

	/*
	 *	Depending on the request method, read all CGI input into cgiinput
	 *	(really should produce HTML error messages, instead of exit()ing)
	 */
	request_method = gogetenv("REQUEST_METHOD");

	if (!request_method) {
		request_method = "NOT-CGI";
	}

	if (!strcmp(request_method, "GET")||!strcmp(request_method, "HEAD")) {
		cgiinput = strdup(gogetenv("QUERY_STRING"));
	} else if (!strcmp(request_method, "POST")) {
		if (strcmpci(gogetenv("CONTENT_TYPE"), "application/x-www-form-urlencoded")) {
			printf("getcgivars(): Unsupported Content-Type.\n");
			return NULL ;
		}

		if (!(content_length = atoi(gogetenv("CONTENT_LENGTH")))) {
			printf("getcgivars(): No Content-Length was sent with the POST request.\n");
			return NULL ;
		}

		if (!(cgiinput = (char *) malloc(content_length+1))) {
			printf("getcgivars(): Could not malloc for cgiinput.\n");
			return NULL ;
		}

		if (!fread(cgiinput, content_length, 1, stdin)) {
			printf("Couldn't read CGI input from STDIN.\n");
			return NULL ;
		}

		cgiinput[content_length] = '\0';
	} else {
		printf("getcgivars(): unsupported REQUEST_METHOD\n");
		return NULL ;
	}

	/*
	 *	Change all plusses back to spaces
	 */
	for (i = 0; cgiinput[i]; i++) {
		if (cgiinput[i]=='+') {
			cgiinput[i] = ' ';
		}
	}

	/*
	 *	First, split on "&" to extract the name-value pairs into pairlist
	 */
	pairlist = (char **) malloc(256*sizeof(char **));
	paircount = 0;
	nvpair = strtok(cgiinput, "&");

	while (nvpair) {
		pairlist[paircount++ ] = strdup(nvpair);
		if (!(paircount%256)) {
			pairlist = (char **) realloc(pairlist, (paircount+256)
			                *sizeof(char **));
		}

		nvpair = strtok(NULL, "&");
	}

	/*
	 *	Terminate the pair list with NULL
	 */
	pairlist[paircount] = 0;

	/*
	 *	Then, from the list of pairs, extract the names and values
	 */
	cgivars = (char **) malloc((paircount*2+1)*sizeof(char **));
	for (i = 0; i<paircount; i++) {
		if (eqpos = strchr(pairlist[i], '=')) {
			*eqpos = '\0';
			unescape_url(cgivars[i*2+1] = strdup(eqpos+1));
		} else {
			unescape_url(cgivars[i*2+1] = strdup(""));
		}
		unescape_url(cgivars[i*2] = strdup(pairlist[i]));
	}

	/*
	 *	Terminate the cgi var list with NULL
	 */
	cgivars[paircount*2] = 0;

	/*
	 *	Free anything that needs to be freed
	 */
	free(cgiinput);

	for (i = 0; pairlist[i]; i++) {
		free(pairlist[i]);
	}

	free(pairlist);

	/*
	 *	Return the list of name-value strings
	 */
	return cgivars;

}
char *gogetenv(char *varname)
{
	char *result;

	if ((result = getenv(varname))==NULL ) {
		result = "(NULL)";
	}
	return result;
}
int strcmpci(char *s1, char *s2)
{
	int rc;

	if (s1==NULL ||s2==NULL ) {
		return 0;
	}

	if (s1==s2) {
		return 0;
	}

	do {
		rc = tolower(*s1)-tolower(*s2);
		if (*s1=='\0') {
			break;
		}
		s1++;
		s2++;
	} while (rc==0);

	return rc;
}
void unescape_url(char *url)
{
	register int i, j;

	for (i = 0, j = 0; url[j]; ++i, ++j) {
		if ((url[i] = url[j])=='%') {
			url[i] = x2c(&url[j+1]);
			j += 2;
		}
	}

	url[i] = '\0';
}

char x2c(char *what)
{
	register char digit;

	digit = (what[0]>='A' ? ((what[0]&0xdf)-'A')+10 : (what[0]-'0'));
	digit *= 16;
	digit += (what[1]>='A' ? ((what[1]&0xdf)-'A')+10 : (what[1]-'0'));

	return digit;
}
