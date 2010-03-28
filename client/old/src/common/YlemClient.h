/*
 * =====================================================================================
 * 
 *       Filename:  YlemClient.h 
 *    Description:  
 *        Version:  1.0
 *        Created:  2008年03月24日 13时43分51秒 CST
 *       Revision:  none * 
 *         Author:  changjin (changjing.xu@alibaba-inc.com)
 *        Company:  
 * 
 * =====================================================================================
 */

#ifndef  YLEMCLIENT_H_
#define  YLEMCLIENT_H_

#ifndef BASE_H_
#include "Base.h"
#endif
#ifndef YLEMTRACKER_H_
#include "YlemTracker.h"
#endif
#ifndef YLEMFILE_H_
#include "YlemFile.h"
#endif
#include <boost/shared_ptr.hpp>
using boost::shared_ptr;

/*
 * =====================================================================================
 *        Class: YlemClient
 *  Description: 连接ylem系统的客户端类
 * =====================================================================================
 */

namespace ylem{

	class YlemClient{
	public:
		YlemClient();
		YlemClient(const string& host, int port,
				const string& domain,const string& classname);
		void set_class(const string& classname);
		void set_domain(const string& domain);
		// 创建连接
		bool ylem_connect();
		bool ylem_connect(const string& host, int port,
				const string& domain,const string& classname);
		// 上传磁盘文件
		YlemState ylem_putfile(const string& key, const string& filePath,
				const string& mimetype="",
				const string& domain="",const string& classname="") const;
		// 上传内存缓冲区
		YlemState ylem_putfile(const string &key, const char *buf, uint32_t len,
				const string &mimetype="",
				const string &domain="",
				const string &classname="") const;
		// 创建文件
		YlemState ylem_createopen(YlemFile& info,const string& key,
				const string& mimetype="",
				bool multi_dest=true,int64_t fid=-1,
				const string& domain="",const string classname="") const;
		// 关闭文件
		YlemState ylem_createclose(const YlemFile& info) const;
		// 删除文件
		YlemState ylem_delete(const string& key,const string& domain="") const;

		// 获取文件的get_paths
		YlemState ylem_getpaths(mapss& out,string& mimetype,
			   	const string& key,const string& domain="", bool noverify=true) const;
		YlemState ylem_getpaths(string& out,string& mimetype,
				const string& key,const string& domain="", bool noverify=true) const;

		// 检索文件的id
		YlemState ylem_listfids(mapss& out, long long from=0, long long to=100) const;
		YlemState ylem_listfids(string& out, long long from=0, long long to=100) const;

		// 列举文件的key
		YlemState ylem_listkeys(mapss& out, const string& prefix="", const string& after="",int limit=1000, const string& domain="") const;
		YlemState ylem_listkeys(string& out, const string& prefix="", const string& after="",int limit=1000, const string& domain="") const;

	protected:
		YlemClient(const YlemClient&);
		YlemClient& operator=(const YlemClient&)const;	
		YlemClient& operator=(const YlemClient&);	
	private:
		bool   _bSocket;
		int    _port;
		string _host;
		string _domain;
		string _class;
		shared_ptr<YlemTracker> _tracker;;
	};
}

#endif   /* ----- #ifndef YLEMCLIENT_H_  ----- */

