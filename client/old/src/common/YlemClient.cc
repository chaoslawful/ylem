/*
 * =====================================================================================
 *
 *       Filename:  YlemClient.cc
 *    Description:  
 *        Version:  1.0
 *        Created:  2008年03月24日 13时43分59秒 CST
 *       Revision:  none *
 *         Author:  changjin (changjing.xu@alibaba-inc.com)
 *        Company:  
 *
 * =====================================================================================
 */

#ifndef YLEMCLIENT_H_
#include "YlemClient.h"
#endif
#ifndef CSLANGUE_H_
#include "CSLangue.h"
#endif
#ifndef YLEMPUTFILE_H_
#include "YlemPutfile.h"
#endif
#include <boost/lexical_cast.hpp>

namespace ylem{

static log4cpp::Category& logger=log4cpp::Category::getInstance("ylem");

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  YlemClient
 *  Description:  构造函数初始化基本变量
 * =====================================================================================
 */
YlemClient::YlemClient():_bSocket(false),_port(0)
{
	DBG(logger) << "YlemClient::YlemClient";
	_tracker.reset(new YlemTracker());
}


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  YlemClient
 *  Description:  构造函数初始化基本变量
 * =====================================================================================
 */
YlemClient::YlemClient(const string& host, int port,
				const string& domain,const string& classname):
	_port(port),_host(host),_domain(domain),_class(classname)
{
	DBG(logger) << "YlemClient::YlemClient";
	_tracker.reset(new YlemTracker());
}


/* 
 * ===  FUNCTION  ======================================================================
 *         Name: ylem_connect
 *  Description: 设置ylemClient到tracker的socket连接
 * =====================================================================================
 */
bool YlemClient::ylem_connect(const string& host, int port,
				const string& domain,const string& classname)
{
	DBG(logger) << "YlemClient::ylem_connect";
	_domain = domain;
	_class = classname;

	if(_bSocket && host == _host && port == _port)
	{
		return true;
	}
	_host = host;
	_port = port;

	return ylem_connect();
}


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  ylem_connect
 *  Description:  设置client到tracker的连接，成功返回true,失败返回false
 * =====================================================================================
 */
bool YlemClient::ylem_connect()
{
	DBG(logger) << "YlemClient::ylem_connect";
	bool bState = true;
	_bSocket = false;
	if(_port < 0 || _port > 65535 
			|| _host.length() < 1)
	{
		DBG(logger) << "error host:port " << _host <<":" << _port;
		bState = false;
		return bState;
	}

	YlemState state = _tracker->get_connect(_host,_port);
	if( state != YLEM_OK)
	{
		ERROR(logger) << "Error: _tracker->get_connect ";
		bState = false;
	}else{
		DBG(logger) << "create connect ok";
		_bSocket = true;
	}

	return bState;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name: set_class
 *  Description: reset tracker class
 * =====================================================================================
 */
void YlemClient::set_class(const string& classname)
{
	DBG(logger) << "YlemClient::set_class,new class is " << classname;
	_class = classname;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name: set_domain
 *  Description: reset tracker domain
 * =====================================================================================
 */
void YlemClient::set_domain(const string& domain)
{
	DBG(logger) << "YlemClient::set_domain,new domain is " << domain;
	_domain = domain;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name: ylem_putfile 
 *  Description: 上传客户端文件到WEBDAV
 * =====================================================================================
 */
YlemState YlemClient::ylem_putfile(const string& key, const string& filePath,
				const string& mimetype,
				const string& domain,const string& classname) const
{
	DBG(logger) << "YlemClient::ylem_putfile";
	YlemState state = YLEM_OK;
	// 临时file
	YlemFile file;
	
	state = ylem_createopen(file,key,mimetype,false,-1,domain,classname);
	
	if(state == YLEM_OK){

		mapss mdata = file;

		// 上传文件操作
		YlemPutfile put;
		int filesize=0;
		state = put.putfile(mdata["path"],key,filePath,filesize);
		// 关闭上传文件
		if(state == YLEM_OK)
			state = ylem_createclose(file);	
		else{
			CSLangue commond;
			string str = commond.createSerLangue("INFO",mdata);
			ERROR(logger) << key << ": putfile error";
			ERROR(logger) << str;
		}
	}
	return state;
}

YlemState YlemClient::ylem_putfile(const string &key, const char *buf, uint32_t len,
		const string &mimetype, const string &domain, const string &classname) const
{
	DBG(logger) << "YlemClient::ylem_putfile(buf)";
	YlemState state = YLEM_OK;
	// 临时file
	YlemFile file;
	
	state = ylem_createopen(file,key,mimetype,false,-1,domain,classname);
	
	if(state == YLEM_OK){

		mapss mdata = file;

		// 上传文件操作
		YlemPutfile put;
		int filesize=0;
		state = put.putfile(mdata["path"],key,buf,len,filesize);
		// 关闭上传文件
		if(state == YLEM_OK)
			state = ylem_createclose(file);	
		else{
			CSLangue commond;
			string str = commond.createSerLangue("INFO",mdata);
			ERROR(logger) << key << ": putfile error";
			ERROR(logger) << str;
		}
	}
	return state;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name: ylem_createopen 
 *  Description: 创建打开文件
 * =====================================================================================
 */
YlemState YlemClient::ylem_createopen(YlemFile& info,const string& key,
				const string& mimetype,
				bool multi_dest,int64_t fid,
				const string& domain,const string classname) const
{
	DBG(logger) << "YlemClient::ylem_createopen()";
	YlemState state=YLEM_OK;

	// 判断tracker是否已经连接
	if(!_bSocket){
		ERROR(logger) << "may be not create socket";
		state = YLEM_SOCKET_ERR;
		return state;
	}
	
	if(key.length() == 0){
		ERROR(logger) << "key and file Path not set null";
		state = YLEM_ERR;
		return state;
	}

	// first get webDAV paths
	CSLangue command;
	vectorpairss value;
	// domain
	if(domain.length() != 0){
		value.push_back(make_pair("domain",domain));
	}else{
		value.push_back(make_pair("domain",_domain));
	}
	// key
	value.push_back(make_pair("key",key));
	// class name
	if(classname.length() != 0){
		value.push_back(make_pair("class",classname));
	}else{
		value.push_back(make_pair("class",_class));
	}
	// muti_dest
	if(multi_dest)
		value.push_back(make_pair("multi_dest","1"));
	// mimetype
	if(mimetype.length() !=0 )
		value.push_back(make_pair("mimetype",mimetype));
	// fid
	if(fid != -1)
		value.push_back(make_pair("fid",boost::lexical_cast<string>(fid)));

	string strComm = command.createSerLangue("CREATE_OPEN",value);
	DBG(logger) << "create tracker command: " << strComm;

	// 获取的tracker服务器的返回结果
	strComm = _tracker->exeCommand(strComm);
	DBG(logger) << "get tracker command: " << strComm;	

	mapss mdata;
	bool b = command.parseSerLangue(strComm,mdata);
	if(!b){
		ERROR(logger) <<key<<" : " << strComm;
		string str = command.createSerLangue("INFO",value);
		ERROR(logger) << str;
		state = YLEM_ERR;
	}else{
		// 关闭tracker上的webDAV路径
		if(domain.length() != 0){
			mdata.insert(make_pair("domain",domain));
		}else{
			mdata.insert(make_pair("domain",_domain));
		}
		if(classname.length() != 0){
			mdata.insert(make_pair("class",classname));
		}else{
			mdata.insert(make_pair("class",_class));
		}

		mdata.insert(make_pair("key",key));
		info = mdata;
	}

	return state;
}


/* 
 * ===  FUNCTION  ======================================================================
 *         Name: ylem_createclose
 *  Description: 关闭创建文件
 * =====================================================================================
 */
YlemState YlemClient::ylem_createclose(const YlemFile& info) const
{
	DBG(logger) << "YlemClient::ylem_createclose()";
	YlemState state = YLEM_OK;
	
	CSLangue command;
	mapss mData = info;
	string strComm = command.createSerLangue("CREATE_CLOSE",mData);
	string strClose = _tracker->exeCommand(strComm);
	int testNum=1;
	bool b = command.parseSerLangue(strClose,mData);
	while(!b && testNum < LOOPNUMBER)
	{
		CSLangue command;
		string str = command.createSerLangue("INFO",mData);
		ERROR(logger) << mData["key"] <<": " << strClose;
		ERROR(logger) << str;
		++testNum;
		usleep(SLEEPTIME);
		strClose = _tracker->exeCommand(strComm);
		b = command.parseSerLangue(strClose,mData);
	}
	if(!b || state != YLEM_OK)
	{
		state = YLEM_ERR;
		CSLangue command;
		string str = command.createSerLangue("INFO",mData);
		ERROR(logger) << mData["key"] <<": " << strClose;
		ERROR(logger) << str;
	}

	return state;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name: ylem_delete 
 *  Description: 删除指定的key
 * =====================================================================================
 */
YlemState YlemClient::ylem_delete(const string& key,const string& domain) const
{
	DBG(logger) << "YlemClient::ylem_delete, key is " << key; 
	YlemState state=YLEM_ERR;

	if(key.length() < 0)
	{
		ERROR(logger) << "key do not = null";
		return state;
	}

	/* 判断是否创建traker连接 */
	if(!_bSocket)
	{
		ERROR(logger) << "may be not create socket to tracker";
		state = YLEM_SOCKET_ERR;
		return state;
	}

	/* 创建和traker交互命令 */
	CSLangue command;
	mapss data;
	if(domain.length()<1)
		data.insert(make_pair("domain",_domain));
	else
		data.insert(make_pair("domain",domain));
	data.insert(make_pair("key",key));
	string strComm = command.createSerLangue("DELETE",data);

	/* 执行tracker命令 */
	strComm = _tracker->exeCommand(strComm);

	/* 解析返回的traker命令 */
	if(command.parseSerLangue(strComm,data))
		state = YLEM_OK;
	else
		state = YLEM_ERR;

	return state;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name: ylem_getpaths 
 *  Description: 获取文件的webDAV path
 * =====================================================================================
 */
YlemState YlemClient::ylem_getpaths(string& out,string& mimetype,
		const string& key,const string& domain, bool noverify) const
{
	DBG(logger) << "YlemClient::ylem_getpaths, key is " << key;
	YlemState state=YLEM_ERR;

	if(key.length() < 1){
		ERROR(logger) << "key not = null ";
		return state;
	}

	if(!_bSocket){
		ERROR(logger) << "may be not create socket to tracker";
		state = YLEM_SOCKET_ERR;
		return state;
	}

	/* 创建交互语言 */
	CSLangue command;
	mapss mData;
	if(domain.length() < 1)
		mData.insert(make_pair("domain",_domain));
	else
		mData.insert(make_pair("domain",domain));
	mData.insert(make_pair("key",key));
	mData.insert(make_pair("noverify",noverify?"1":"0"));
	string strComm = command.createSerLangue("GET_PATHS",mData);

	/* 执行 */
	out = _tracker->exeCommand(strComm);
	state = YLEM_OK;

	return state;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name: ylem_getpaths 
 *  Description: 获取文件的webDAV path
 * =====================================================================================
 */
YlemState YlemClient::ylem_getpaths(mapss& out,string& mimetype,
		const string& key,const string& domain, bool noverify) const
{
	DBG(logger) << "YlemClient::ylem_getpaths, key is " << key;
	string strComm;

	YlemState state=ylem_getpaths(strComm,mimetype,key,domain,noverify);
	if(state == YLEM_OK)
	{
		CSLangue command;
		/* 分析返回结果 */
		bool b = command.parseSerLangue(strComm,out);
		if(!b){
			out.clear();
			state = YLEM_ERR;
		}
	}

	return state;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name: ylem_listfids
 *  Description: 列出所有的fids
 * =====================================================================================
 */
YlemState YlemClient::ylem_listfids(string& out,long long from, long long to) const
{
	DBG(logger) << "YlemClient::ylem_listfids,from:to" << from << ":" << to;
	YlemState state=YLEM_ERR;

	if(!_bSocket){
		ERROR(logger) << "may be not create socket to tracker";
		state = YLEM_SOCKET_ERR;
		return state;
	}

	string strfrom,strto;
	ostringstream oss;
	oss << from;
	strfrom=oss.str();
	oss.str("");
	oss << to;
	strto = oss.str();

	/* 创建与tracker交互命令 */
	CSLangue command;	
	mapss mData;
	mData.insert(make_pair("from",strfrom));
	mData.insert(make_pair("to",strto));
	string strComm = command.createSerLangue("LIST_FIDS",mData);

	/* 执行交互命令 */
	out = _tracker->exeCommandBig(strComm);	
	state = YLEM_OK;

	return state;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name: ylem_listfids
 *  Description: 列出所有的fids
 * =====================================================================================
 */
YlemState YlemClient::ylem_listfids(mapss& out,long long from, long long to) const
{
	DBG(logger) << "YlemClient::ylem_listfids,from:to" << from << ":" << to;
	string strComm;

	YlemState state = ylem_listfids(strComm, from, to);
	if(state == YLEM_OK)
	{
		CSLangue command;
		/* 分析返回结果 */
		bool b = command.parseSerLangue(strComm,out);
		if(!b){
			out.clear();
			state = YLEM_ERR;
		}
	}
	
	return state;
}
/* 
 * ===  FUNCTION  ======================================================================
 *         Name: ylem_listkeys 
 *  Description: 列出所有的listkey 
 * =====================================================================================
 */
YlemState YlemClient::ylem_listkeys(string& out,const string& prefix, const string& after,
			   int limit, const string& domain) const
{
	DBG(logger) << "YlemClient::ylem_listkeys";
	YlemState state=YLEM_ERR;

	if(!_bSocket){
		ERROR(logger) << "may be not create socket to tracker";
		state = YLEM_SOCKET_ERR;
		return state;
	}

	ostringstream oss;
	oss << limit;
	string strlimit(oss.str());

	/* 创建与tracker交互命令 */
	CSLangue command;	
	mapss mData;
	if(domain.length() < 1)
		mData.insert(make_pair("domain",_domain));
	else 
		mData.insert(make_pair("domain",domain));
	mData.insert(make_pair("prefix",prefix));
	mData.insert(make_pair("after",after));
	mData.insert(make_pair("limit",strlimit));
	string strComm = command.createSerLangue("LIST_KEYS",mData);

	/* 执行交互命令 */
	out = _tracker->exeCommandBig(strComm);
	state = YLEM_OK;
	DBG(logger) << out;

	return state;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name: ylem_listkeys 
 *  Description: 列出所有的listkey 
 * =====================================================================================
 */
YlemState YlemClient::ylem_listkeys(mapss& out,const string& prefix, const string& after,
			   int limit, const string& domain) const
{
	DBG(logger) << "YlemClient::ylem_listkeys";
	string strComm;

	YlemState state=ylem_listkeys(strComm, prefix, after, limit, domain);

	if(state == YLEM_OK)
	{
		CSLangue command;
		/* 分析返回结果 */
		bool b = command.parseSerLangue(strComm,out);
		if(!b){
			out.clear();
			state = YLEM_ERR;
		}
	}

	return state;
}

}





