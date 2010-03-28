#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <sstream>
#include <algorithm>
#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>
#include <cassert>
#include <curl/curl.h>
#include <cstdlib>
#include <ctime>
#include "dfs_func.h"
#include "ylemClient/YlemClient.h"

#ifdef DEBUG
#define LOG(msg) cerr<<msg<<endl
#else
#define LOG(msg)
#endif

#define TRACKER_HOST "track01.ylem.cn3.yahoo.com"
#define TRACKER_PORT 7001
#define TEST_DOMAIN "test"
#define TEST_CLASS ""
#define BLKSIZE (1024*1024)

using namespace std;
using namespace ylem;
using namespace boost;

const regex e("(\\d+.\\d+.\\d+.\\d+)");

static size_t dummy_curl_cb(void *ptr, size_t size, size_t nmemb, void *data)
{
	int *total=(int*)data;
	int bsize=size*nmemb;
	LOG("Read "<<size*nmemb<<" bytes");
	*total+=bsize;
	return bsize;
}

static int curl_put(const string &url, const char *buf, int buf_len, int total_len, int offset)
{
	// 使用CURL上传文件数据
	CURL *curl;
	CURLcode res;
	struct curl_slist *slist=NULL;

	curl=curl_easy_init();
	if(!curl) {
		LOG("Failed to initialize CURL easy api");
		return -1;
	}
	
//	curl_easy_setopt(curl,CURLOPT_VERBOSE,1);
	curl_easy_setopt(curl,CURLOPT_URL,url.c_str());
	curl_easy_setopt(curl,CURLOPT_CUSTOMREQUEST,"PUT");
	curl_easy_setopt(curl,CURLOPT_POSTFIELDS,buf);
	curl_easy_setopt(curl,CURLOPT_POSTFIELDSIZE,buf_len);

	ostringstream oss1,oss2;
	oss1<<"Content-Range: bytes "<<offset<<"-"<<offset+buf_len-1<<"/"<<total_len;
	oss2<<"Content-Length: "<<buf_len;
	slist=curl_slist_append(slist,oss1.str().c_str());
	slist=curl_slist_append(slist,oss2.str().c_str());
	slist=curl_slist_append(slist,"Content-Type: application/octet-stream");
	// 删除lighttpd无法识别的Expect头
	slist=curl_slist_append(slist,"Expect:");
	curl_easy_setopt(curl,CURLOPT_HTTPHEADER,slist);

	res=curl_easy_perform(curl);
	if(res!=CURLE_OK) {
		LOG("curl_easy_perform() failed: "<<res<<", "<<curl_easy_strerror(res));
	}

	curl_slist_free_all(slist);
	curl_easy_cleanup(curl);

	if(res!=CURLE_OK) {
		return -1;
	} else {
		return 0;
	}
}

int dfs_init()
{
	LOG("In dfs_init()");
	CURLcode res=curl_global_init(CURL_GLOBAL_ALL);
	if(res!=CURLE_OK) {
		LOG("Curl global initialization failed");
		return -1;
	}
	return 0;
}

void dfs_destroy()
{
	LOG("In dfs_destroy()");
	curl_global_cleanup();
}

/*
* upload file to the storage server
* param file_buff the file content
* param file_size the file size (bytes)
* param file_id return the file id (max length 63)
* param storage_ip return the storage server ip address (max length 15)
* return 0 if success, none zero for error
*/
int upload_file(const char *file_buff, const int file_size, char *file_id, char *storage_ip)
{
	LOG("In upload_file()");
	YlemClient cli(TRACKER_HOST,TRACKER_PORT,TEST_DOMAIN,TEST_CLASS);
	if(!cli.ylem_connect()) {
		LOG("Failed to connect to tracker");
		return -1;
	}

	// 用当前UNIX时戳+随机数作为上传文件名
	snprintf(file_id,63,"%010lu%010lu",(unsigned long)time(NULL),(unsigned long)random());
	LOG("Generated file id: "<<file_id);

	// 发送创建文件命令
	YlemFile info;
	YlemState rc=cli.ylem_createopen(info,file_id);
	if(rc!=YLEM_OK) {
		LOG("Failed to execute CREATE_OPEN command");
		return -1;
	}

	string url=info.get_path(0);
	info.select_dev(0);

	smatch what;
	if(regex_search(url,what,e)) {
		string tmps(what[1].first,what[1].second);
		strcpy(storage_ip,tmps.c_str());
	}
	LOG("Upload URL: "<<url);

	// 通过CURL接口分段上传文件数据
	int offset=0;
	int res;
	int blk_sz;
	while(offset<file_size) {
		blk_sz=file_size-offset;
		if(blk_sz>BLKSIZE) {
			blk_sz=BLKSIZE;
		}
		res=curl_put(url,file_buff+offset,blk_sz,file_size,offset);
		if(res) {
			LOG("Failed to upload block: offset="<<offset<<", blocksize="<<blk_sz);
			break;
		}
		offset+=blk_sz;
	}

	// 发送关闭文件命令
	rc=cli.ylem_createclose(info);
	if(rc!=YLEM_OK) {
		LOG("Failed to execute CREATE_CLOSE command");
		return -1;
	}

	return 0;
}

/*
* download file from the storage server
* param file_id the file id
* param file_size return the file size (bytes)
* param storage_ip return the storage server ip address (max length 15)
* return 0 if success, none zero for error
*/
int download_file(const char *file_id, int *file_size, char *storage_ip)
{
	LOG("In download_file()");
	YlemClient cli(TRACKER_HOST,TRACKER_PORT,TEST_DOMAIN,TEST_CLASS);
	if(!cli.ylem_connect()) {
		LOG("Failed to connect to tracker");
		return -1;
	}

	map<string,string> kvs;
	string mimetype;
	string url;

	// 获取下载URL
	YlemState rc=cli.ylem_getpaths(kvs,mimetype,file_id);

	if(rc==YLEM_OK) {
		LOG("GET_PATHS succeeded");
		url=kvs["path1"];
		smatch what;
		if(regex_search(url,what,e)) {
			string tmps(what[1].first,what[1].second);
			strcpy(storage_ip,tmps.c_str());
		}
		LOG("Download URL: "<<url);

		// 使用CURL函数下载
		CURL *curl;
		CURLcode res;

		curl=curl_easy_init();
		if(!curl) {
			LOG("Failed to initialize CURL easy api");
			return -1;
		}

//		curl_easy_setopt(curl,CURLOPT_VERBOSE,1);
		curl_easy_setopt(curl,CURLOPT_URL,url.c_str());
		curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,dummy_curl_cb);
		*file_size=0;
		curl_easy_setopt(curl,CURLOPT_WRITEDATA,file_size);

		res=curl_easy_perform(curl);
		if(res!=CURLE_OK) {
			LOG("curl_easy_perform() failed: "<<res<<", "<<curl_easy_strerror(res));
		}

		curl_easy_cleanup(curl);
		return 0;
	} else {
		LOG("GET_PATHS failed");
		return -1;
	}
}

/*
* delete file from the storage server
* param file_id the file id
* param storage_ip return the storage server ip address (max length 15)
* return 0 if success, none zero for error
*/
int delete_file(const char *file_id, char *storage_ip)
{
	LOG("In download_file()");
	YlemClient cli(TRACKER_HOST,TRACKER_PORT,TEST_DOMAIN,TEST_CLASS);
	if(!cli.ylem_connect()) {
		LOG("Failed to connect to tracker");
		return -1;
	}

	YlemState rc=cli.ylem_delete(file_id);
	strncpy(storage_ip,"unknown",7+1);

	if(rc==YLEM_OK) {
		LOG("File deleted successfully");
		return 0;
	} else {
		LOG("Failed to delete file");
		return -1;
	}
}

#ifdef MAIN
int main(int argc,char *argv[])
{
	int ntimes=100;
	vector<string> fids;
	string tstdata(1024*1024*5,'a');
	char file_id[63]={0};
	char st_ip[15]={0};
	int rc;
	int size=0;

	if(argc==2) {
		ntimes=lexical_cast<unsigned int>(argv[1]);
	}

	rc=dfs_init();
	assert(!rc);

	for(int i=0;i<ntimes;++i) {
		rc=upload_file(tstdata.data(),tstdata.size(),file_id,st_ip);
		assert(!rc);
		LOG("File uploaded to storage:"<<st_ip);
		fids.push_back(file_id);
	}

	for(int i=0;i<ntimes;++i) {
		rc=download_file(fids[i].c_str(),&size,st_ip);
		assert(!rc);
		LOG("File size:"<<size<<" bytes, from storage:"<<st_ip);
	}

	for(int i=0;i<ntimes;++i) {
		rc=delete_file(fids[i].c_str(),st_ip);
		assert(!rc);
		LOG("File deleted, from storage:"<<st_ip);
	}

	dfs_destroy();
}
#endif

