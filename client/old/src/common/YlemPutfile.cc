/*
 * =====================================================================================
 *
 *       Filename:  YlemPutfile.cc
 *    Description:  
 *        Version:  1.0
 *        Created:  2008年03月24日 13时45分15秒 CST
 *       Revision:  none *
 *         Author:  changjin (changjing.xu@alibaba-inc.com)
 *        Company:  
 *
 * =====================================================================================
 */

#ifndef YLEMPUTFILE_H_
#include "YlemPutfile.h"
#endif

#ifdef USENEON
#include <neon/ne_session.h>
#include <neon/ne_basic.h>
#endif
#include <curl/curl.h>
#ifndef YLEMURI_H_
#include "YlemUri.h"
#endif

#include <netdb.h>
#include <sys/types.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <boost/lexical_cast.hpp>

using namespace::boost;

namespace ylem{

static log4cpp::Category& logger=log4cpp::Category::getInstance("ylem");

#ifdef USESENDFILE
// 分段发送文件
static int sectsendfile(int filefd,struct hostent *he,off_t *offset,YlemUri& url,int sendsize,const string& strRange);
#endif

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  YlemPutfile
 *  Description:  construction function
 * =====================================================================================
 */
YlemPutfile::YlemPutfile()
{
	DBG(logger) << "YlemPutfile::Ylemputfile()";
}


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  ~YlemPutfile
 *  Description:  destruction function
 * =====================================================================================
 */
YlemPutfile::~YlemPutfile()
{
	DBG(logger) << "YlemPutfile::~Ylemputfile()";
}

YlemState YlemPutfile::putfile(const string &webPath, const string &key,
		const char *buf, uint32_t len, int &filesize) const
{
	/* declare variables  */
	DBG(logger) << "YlemPutfile::putfile(buf)";
	YlemState state=YLEM_WEBDAV_ERR;
	YlemUri url;
	filesize = 0;
	
	/* variables not == NULL  */
	if(webPath.length() ==0 || key.length() ==0 
			|| len ==0)
	{
		ERROR(logger) << "params error, webPath:" << webPath <<
			" key:" << key << " len:" << len;
		state = YLEM_ERR;
		return state;
	}

	/* parse url */
	if(url.ylem_uri_parse(webPath.c_str()) == 0)
	{
//=============use sendfile=======================
//#ifdef USERSENDFILE
#if defined(USESENDFILE)||defined(USENEON)||defined(USECURL)
			DBG(logger) << "use curl";	
			string leninfo="Content-Length: "+lexical_cast<string>(len);
			int http_code = _usecurlput(webPath.c_str(),buf,len,leninfo.c_str(),"");
			DBG(logger) << "http_code: " << http_code;
			if(http_code == -1 || http_code == 404) {
				DBG(logger) << "put file fail";
				state=YLEM_ERR;
			} else {
				DBG(logger) << "put file OK";
				state=YLEM_OK;
			}
//=============use curl==============================
#else
//=============socket write==========================
			DBG(logger) << "socket write ";
			struct hostent *he;
			if((he=gethostbyname(url.host.c_str()))==NULL) {
				ERROR(logger) <<"gethostbyname() error " << url.host;
				state = YLEM_ERR;
				close(fileid);
				return state;
			}

			int sockfd;
			struct sockaddr_in serv_addr;
        
			if((sockfd=socket(PF_INET,SOCK_STREAM,0))==-1) {
				ERROR(logger) <<"socket() error";
				close(fileid);
				state = YLEM_SOCKET_ERR;
				return state;
			}
        
			serv_addr.sin_family=AF_INET;
			serv_addr.sin_port=htons(url.port);
			serv_addr.sin_addr=*((struct in_addr*)he->h_addr);
			bzero( &(serv_addr.sin_zero),8);
 
			if(connect(sockfd,(struct sockaddr*)&serv_addr,sizeof(serv_addr))==-1) {
				ERROR(logger)<<"connect() error";
				close(sockfd);
				close(fileid);
				state = YLEM_SOCKET_ERR;
				return state;
			}

			// write HTTP PUT request header first
			string put_req="PUT "+url.path+" HTTP/1.1\r\nHost: "+url.host+"\r\nContent-Length: "+lexical_cast<string>(len)+"\r\n\r\n";

			write(sockfd,put_req.data(),put_req.size());
			DBG(logger) << put_req;
			
			int sent;
			// send buffer data
			sent=write(sockfd,buf,len);

			int nb;
			char tmpbuf[MAXREADFILE];
			while((nb=recv(sockfd,tmpbuf,MAXREADFILE,0)) > 0) {
				DBG(logger) << "recv: " << string(tmpbuf,nb);
				break;
			}
			
			if(sent == len){
				DBG(logger) << "put file OK";
				state = YLEM_OK;
			}
			else{
				state = YLEM_ERR;
				DBG(logger) << "put file fail";
			}

			close(sockfd);
//=============secket write==========================
#endif
	}else{
		ERROR(logger) << "webPath error: "<<webPath;
		state = YLEM_WEBDAV_ERR;
	}

//	state = YLEM_OK;
	return state;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  putfile
 *  Description:  上传文件
 * =====================================================================================
 */
YlemState YlemPutfile::putfile(const string& webPath,
		const string& key, const string& filePath, int& filesize) const
{
	/* declare variables  */
	DBG(logger) << "YlemPutfile::putfile";
	YlemState state=YLEM_WEBDAV_ERR;
	YlemUri url;
	filesize = 0;
	
	/* variables not == NULL  */
	if(webPath.length() ==0 || key.length() ==0 
			|| filePath.length() ==0)
	{
		ERROR(logger) << "params error, webPath:" << webPath <<
			" key:" << key << " filePath:" << filePath;
		state = YLEM_ERR;
		return state;
	}

	/* parse url */
	if(url.ylem_uri_parse(webPath.c_str()) == 0)
	{
		/* open file */
		int fileid=-1;
		if((fileid=open(filePath.c_str(),O_RDONLY)) !=-1)
		{
			/* 获取文件大小 */
			struct stat sb;
			fstat(fileid,&sb);
			filesize = sb.st_size;
			DBG(logger) << "filesize is: " << filesize;
			
//=============use sendfile=======================
//#ifdef USERSENDFILE
#ifdef USESENDFILE
			struct hostent *he;
			if((he=gethostbyname(url.host.c_str()))==NULL) {
				ERROR(logger) <<"gethostbyname() error " << url.host;
				state = YLEM_ERR;
				close(fileid);
				return state;
			}

			off_t offset=0;
			int sz=0;
			int iTemp = filesize;
			int iCurrent=0;
			
			do{
				int sendsize = MAXFILEBLOCK;
				int loopNum = 1;
				if(iTemp < MAXFILEBLOCK)
				{
					sendsize = iTemp;
				}
				ostringstream oss;
				oss << iCurrent << "-" << iCurrent+sendsize-1 << "/" << filesize; 
				iCurrent+=sendsize;
				int isz = sectsendfile(fileid,he,&offset,url,sendsize,oss.str());
				while(isz < 0 && loopNum < LOOPNUMBER)
				{
					++loopNum;
					isz = sectsendfile(fileid,he,&offset,url,sendsize,oss.str());
					DBG(logger) << "send error, No." << loopNum << " send ";
				}
				if(iTemp != LOOPNUMBER){
					sz += isz;
				}else{
					ERROR(logger) << "put file error";
					sz =-1;
					iTemp = -1;
				}
			}while((iTemp-=MAXFILEBLOCK)>0);

			if(sz <0){
				ERROR(logger) << "put file fail, file name is " << filePath;
				state = YLEM_ERR;
			}else{
				DBG(logger) << "put file OK";
				state = YLEM_OK;
			}
			DBG(logger) << "send size " << sz << "| file size " << sb.st_size;

			close(fileid);
//=============use sendfile=======================
//=============use neon libary=======================
#elif USENEON
			ne_session *sess=NULL;
			// create session 
			sess = ne_session_create(url.scheme.c_str(),url.host.c_str(),url.port);
			if(sess == NULL)
			{
				DBG(logger) << "ne_create_seesion error";
				state = YLEM_WEBDAV_ERR;
				close(fileid);
				return state;
			}
	
			// put file 
			int ret = ne_put(sess,url.path.c_str(),fileid);
			close(fileid);
			if(ret != 0)
			{
				state = YLEM_WEBDAV_ERR;
				ERROR(logger) << "ne_put error: " << ne_get_error(sess);
			}
			ne_session_destroy(sess);
			DBG(logger) << "put file OK";
			state = YLEM_OK;
//=============use neon libary=======================
#elif USECURL
//=============use curl==============================
			DBG(logger) << "use curl";	
			ssize_t sz;
			char buf[MAXREADFILE];
			int total=0;
			while((sz=read(fileid,buf,MAXREADFILE))>0) {
				ostringstream oss;
				oss << "Content-Range: bytes " <<total << "-" << total+sz-1 << "/" << sb.st_size; 
				string leninfo="Content-Length: "+lexical_cast<string>(sz);
				string strRange = oss.str();
				int http_code = _usecurlput(webPath.c_str(),
						(const char*)(&buf),sz,leninfo.c_str(),strRange.c_str());
				DBG(logger) << "http_code: " << http_code;
				if(http_code == -1 || http_code == 404)
					break;
				total+=sz;
			}

			DBG(logger) << "send size " << total << "| file size " << sb.st_size;

			if(total == sb.st_size){
				DBG(logger) << "put file OK";
				state = YLEM_OK;
			}
			else{
				state = YLEM_ERR;
				DBG(logger) << "put file fail";
			}

			close(fileid);

//=============use curl==============================
#else
//=============socket write==========================
			DBG(logger) << "socket write ";
			struct hostent *he;
			if((he=gethostbyname(url.host.c_str()))==NULL) {
				ERROR(logger) <<"gethostbyname() error " << url.host;
				state = YLEM_ERR;
				close(fileid);
				return state;
			}

			int sockfd;
			struct sockaddr_in serv_addr;
        
			if((sockfd=socket(PF_INET,SOCK_STREAM,0))==-1) {
				ERROR(logger) <<"socket() error";
				close(fileid);
				state = YLEM_SOCKET_ERR;
				return state;
			}
        
			serv_addr.sin_family=AF_INET;
			serv_addr.sin_port=htons(url.port);
			serv_addr.sin_addr=*((struct in_addr*)he->h_addr);
			bzero( &(serv_addr.sin_zero),8);
 
			if(connect(sockfd,(struct sockaddr*)&serv_addr,sizeof(serv_addr))==-1) {
				ERROR(logger)<<"connect() error";
				close(sockfd);
				close(fileid);
				state = YLEM_SOCKET_ERR;
				return state;
			}

			// write HTTP PUT request header first
			string put_req="PUT "+url.path+" HTTP/1.1\r\nHost: "+url.host+"\r\nContent-Length: "+lexical_cast<string>(sb.st_size)+"\r\n\r\n";

			write(sockfd,put_req.data(),put_req.size());
			DBG(logger) << put_req;
			
			ssize_t sz;
			char buf[MAXREADFILE];
			int total=0;
			while((sz=read(fileid,buf,MAXREADFILE))>0) {
				write(sockfd,buf,sz);
				total+=sz;
			}

			int nb;
			while((nb=recv(sockfd,buf,MAXREADFILE,0)) > 0) {
				DBG(logger) << "recv: " << string(buf,nb);
				break;
			}
			
			DBG(logger) << "send size " << total << "| file size " << sb.st_size;

			if(total == sb.st_size){
				DBG(logger) << "put file OK";
				state = YLEM_OK;
			}
			else{
				state = YLEM_ERR;
				DBG(logger) << "put file fail";
			}

			close(sockfd);
			close(fileid);

//=============secket write==========================
#endif
		}else{
			ERROR(logger) << "file not found:" << filePath;
			state = YLEM_OPENFILE_ERR;
		}
	}else{
		ERROR(logger) << "webPath error: "<<webPath;
		state = YLEM_WEBDAV_ERR;
	}

//	state = YLEM_OK;
	return state;
}


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  
 *  Description:  
 * =====================================================================================
 */
int YlemPutfile::_usecurlput(const char* url, const char* data, ssize_t len,
					const char* leninfo, const char* rangeinfo) const 
{
	DBG(logger) << "_usecurlput file: " << url;
	DBG(logger) << leninfo;
	DBG(logger) << rangeinfo;
	CURL *ec;
	CURLcode rc;
	int http_code=-1;
	struct curl_slist *slist=NULL;
        
	ec=curl_easy_init();
	if(ec == NULL)
		return http_code;
    
	// for debugging
//	curl_easy_setopt(ec,CURLOPT_VERBOSE,1);

	curl_easy_setopt(ec,CURLOPT_URL,url);
	curl_easy_setopt(ec,CURLOPT_CUSTOMREQUEST,"PUT");
	curl_easy_setopt(ec,CURLOPT_POSTFIELDS,data);
	curl_easy_setopt(ec,CURLOPT_POSTFIELDSIZE,len);
        
	// 更改http头
	// Content-Type:ation/x-www-form-urlencoded 修改为 ªapplication/octet-stream
	slist=curl_slist_append(slist,"Content-Type: application/octet-stream");
	// 删除lighttpd无法识别的Expect头
	slist=curl_slist_append(slist,"Expect:");
	slist=curl_slist_append(slist,leninfo);
	if(rangeinfo&&strlen(rangeinfo)>0) {
		slist=curl_slist_append(slist,rangeinfo);
	}
	curl_easy_setopt(ec,CURLOPT_HTTPHEADER,slist);

	rc=curl_easy_perform(ec);
	if(rc!=CURLE_OK) {
		ERROR(logger) << "curl_easy_perform() failed: " << rc << curl_easy_strerror(rc);
		curl_slist_free_all(slist);
		curl_easy_cleanup(ec);
		return http_code;
	}

	rc=curl_easy_getinfo(ec,CURLINFO_RESPONSE_CODE,&http_code);
	if(rc!=CURLE_OK) {
		ERROR(logger) << "curl_easy_getinfo() failed: " << rc << curl_easy_strerror(rc);
		http_code=-1;
		curl_slist_free_all(slist);
		curl_easy_cleanup(ec);
		return http_code;
	}

	curl_slist_free_all(slist);
	curl_easy_cleanup(ec);

	return http_code;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  sectsendfile
 *  Description:  分段发送文件
 * =====================================================================================
 */
#ifdef USESENDFILE
int sectsendfile(int fileid,struct hostent *he,
		off_t *offset,YlemUri& url,int sendsize,const string& strRange)
{
	DBG(logger) << "static sectsendfile";
	int sockfd;
	struct sockaddr_in serv_addr;
        
	if((sockfd=socket(PF_INET,SOCK_STREAM,0))==-1) {
		ERROR(logger) <<"socket() error";
		return -1;
	}
        
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_port=htons(url.port);
	serv_addr.sin_addr=*((struct in_addr*)he->h_addr);
	bzero( &(serv_addr.sin_zero),8);
 
	if(connect(sockfd,(struct sockaddr*)&serv_addr,sizeof(serv_addr))==-1) {
		ERROR(logger)<<"connect() error";
		close(sockfd);
		return -1;
	}

	// write HTTP PUT request header first
	string put_req="PUT "+url.path+" HTTP/1.1\r\nHost: "+url.host+"\r\nContent-Length: "+lexical_cast<string>(sendsize)+"\r\nContent-Range: bytes " +strRange+"\r\n\r\n";

	write(sockfd,put_req.data(),put_req.size());
			
	// use kernel call 'sendfile' to send remaining file data 
	ssize_t sz;
	// 每次发送数量
	sz=sendfile(sockfd,fileid,offset,sendsize);
	DBG(logger) << put_req;

	if(sendsize != MAXFILEBLOCK)
	{
		int nb;
		char buf[MAXREADFILE];
		DBG(logger) << "start recv";
		while((nb=recv(sockfd,buf,MAXREADFILE,0)) > 0) {
			DBG(logger) << "recv: " << string(buf,nb);
			break;
		}
		DBG(logger) << "end recv " << nb;
	}

	DBG(logger) << "start close(sockfd)";
	close(sockfd);
	DBG(logger) << "end close(sockfd)";
	DBG(logger) << "send file size: " << sz;

	return sz;
}
#endif
} // =============end namespace ylem=========
