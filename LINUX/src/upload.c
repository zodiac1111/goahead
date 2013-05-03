/**
 * @file upload.c
 * 1. 实现客户端上传文件功能,主要用于更新程序.
 *  在更新时非常实用.使用了goahead上传文件补丁
 * 2. 实现配置文件的导入导出操作.
 */
/** 文件上传补丁由 http://velep.com/archives/321.html 下载.
 * 仅适用goahead2.5和2.1.1(老版本)现在该公司已经被收购,维护需要靠自己.
 * for test html upload file to web server
 * add by gyr 2011.10.15
 */
#include "upload.h"
#include "autoUpdate.h"
static int webExport_conf(webs_t wp);
void form_upload_file(webs_t wp, char_t *path, char_t *query)
{
	//PRINT_FORM_INFO;
	printf(WEBS_INF"%s\n", __FUNCTION__);
	FILE * fp;
	char_t * fn;
	char_t * bn = NULL;
	int locWrite;
	int numLeft;
	int numWrite;
	char tmp[128] = { 0 };
	jsObj oUpdate = jsonNew();
	char full_fname[1024] = { 0 };     //全路径的压缩包名
	char tmpfname[1024] = { 0 };     //保存到服务器的临时文件名
	char cmd[1024] = { 0 };
	a_assert(websValid(wp));
	websHeader_pure(wp);
	fn = websGetVar(wp, T("filename"), T(""));
	if (fn!=NULL &&*fn!='\0') {
		if ((int) (bn = gstrrchr(fn, '/')+1)==1) {
			if ((int) (bn = gstrrchr(fn, '\\')+1)==1) {
				bn = fn;
			}
		}
	}
	printf(WEBS_INF"file update fn=%s, bn=%s\n", fn, bn);
	websWrite(wp, T("file update fn=%s, bn=%s<br>"), fn, bn);
	//简单验证试图更新文件的正确性
	if (bn==NULL ) {
		jsonAdd(&oUpdate, "ret", "filename must no-void");
		web_err_proc(EL);
		goto SEND_ERROR;
	}
	if (strcmp(bn, "webs-binary.tar.gz")==0) {
		web_err_proc(EL);
		jsonAdd(&oUpdate, "ret", "filename must be webs-binary.tar.gz");
		goto SEND_ERROR;
	}
	websWrite(wp, "文件名正确<br>");
	jsonAdd(&oUpdate, "filename", bn);
	jsonAdd(&oUpdate, "size", toStr(tmp, "%d", wp->lenPostData));
	sprintf(full_fname, "/tmp/%s", bn);
	sprintf(tmpfname, "%s%s", full_fname, ".tmp");
	jsonAdd(&oUpdate, "savename", tmpfname);
	//开始将文件保存到服务器,打开本地文件
	websWrite(wp, T("上传到服务器..<br>"));
	if ((fp = fopen(tmpfname, "w+b"))==NULL ) {
		jsonAdd(&oUpdate, "ret", "open file failed");
		web_err_proc(EL);
		goto SEND_ERROR;
	}
	//写临时文件 .tmp
	locWrite = 0;
	numLeft = wp->lenPostData;
	while (numLeft>0) {
		numWrite = fwrite(
		                &(wp->postData[locWrite]),
		                sizeof(*(wp->postData)),
		                numLeft,
		                fp);
		if (numWrite<numLeft) {
			break;
		}
		locWrite += numWrite;
		numLeft -= numWrite;
	}
	//检查 临时文件
	if (numLeft!=0) {
		jsonAdd(&oUpdate, "ret", toStr(
		                tmp,
		                "不完整 numLeft=%d locWrite=%d Size=%d bytes",
		                numLeft,
		                locWrite,
		                wp->lenPostData));
		fclose(fp);
		web_err_proc(EL);
		goto SEND_ERROR;

	}
	websWrite(wp, "上传完成<br>");
	//关闭文件,此时会刷新缓冲区到文件,真正的保存成为文件.(*.tmp)
	if (fclose(fp)!=0) {
		jsonAdd(&oUpdate, "ret", "close file failed");
		web_err_proc(EL);
		goto SEND_ERROR;
	}
	//完成传输
	websWrite(wp, "复制文件到临时目录<br>");
	//去掉tmp后缀
	printf(WEBS_INF"full_fname : %s\n", full_fname);
	printf(WEBS_INF"tmpfname : %s\n", tmpfname);
	toStr(cmd, "mv %s %s", tmpfname, full_fname);
	printf(WEBS_INF"rm(rename) cmd : %s\n", cmd);
	system(cmd);
	//解压文件
	websWrite(wp, "正在解压文件(可能需要1~2分钟),请稍候...<br>");
	toStr(cmd, " gzip -dc %s | tar -xvf - -C / && rm %s -f && echo \"ok\"",
	                full_fname, full_fname);
	printf(WEBS_INF"tar cmd : %s\n", cmd);
	if (system(cmd)<0) {
		websWrite(wp, "文件解压(%s)失败<br>", cmd);
		web_err_proc(EL);
		goto SEND_ERROR;
	}
	websWrite(wp, "文件解压成功<br>");
	jsonAdd(&oUpdate, "ret", "ok");
	websWrite(wp, "回传信息:");
	wpsend(wp, oUpdate);
	jsonFree(&oUpdate);
	websWrite(wp, "<br>重启服务器...<br>");
	websWrite(wp, T("<font color=green>更新完成</font>.请<b>清空浏览器缓存</b>并<b>刷新</b>页面.<br>"));
	autoUpdate();     //自动升级
	websDone(wp, 200);
	return;
	SEND_ERROR:
	websWrite(wp, "<font color=red>升级失败<font>");
	jsonFree(&oUpdate);
	websDone(wp, 200);
	return;
}
/**
 * 配置文件的操作,涉及倒入导出配置文件
 * @todo 实现配置文件的倒入导出功能
 * @param wp
 * @param path
 * @param query
 */
void form_conf_file(webs_t wp, char_t *path, char_t *query)
{
	PRINT_FORM_INFO;
	websHeader_pure(wp);
	char * action = websGetVar(wp, T("action"), T("null"));
	if (strcmp(action, "import")==0) {
		//todo
		printf(WEBS_WAR"未实现:配置文件导入\n");
	} else if (strcmp(action, "export")==0) {
		printf(WEBS_INF"配置文件导出\n");
		webExport_conf(wp);
	} else {
		web_err_proc(EL);
	}
	websDone(wp, 200);
	return;
}
/**
 * 打包配置文件
 * @param wp
 * @return
 */
static int
webExport_conf(webs_t wp)
{
	char cmd[4096];
	char *items = websGetVar(wp, T("items"), T("null"));
	//保存到www根目录
	toStr(cmd, "tar cvf /mnt/nor/www/backup.tar %s ",
		items);
	printf(WEBS_INF"备份命令:%s\n",cmd);
	if(system(cmd)<0){
		web_err_proc(EL);
	}
	return 0;
}
