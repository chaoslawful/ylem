/*
 * =====================================================================================
 *
 *       Filename:  YlemFile.cc
 *    Description:  
 *        Version:  1.0
 *        Created:  2008年04月17日 11时20分52秒 CST
 *       Revision:  none *
 *         Author:  changjin (changjing.xu@alibaba-inc.com)
 *        Company:  
 *
 * =====================================================================================
 */

#ifndef YLEMFILE_H_
#include "YlemFile.h"
#endif
#include <boost/lexical_cast.hpp>

namespace ylem{
static log4cpp::Category& logger = log4cpp::Category::getInstance("ylem");

/* 
 * ===  FUNCTION  ======================================================================
 *         Name: get_dev_count 
 *  Description: 获取可能存储位置总数
 * =====================================================================================
 */
int YlemFile::get_dev_count() const
{
	DBG(logger) << "YlemFile::get_dev_count";
	int iRe=0;
	mapssctor it = _info.find("dev_count");
	if(it != _info.end())
	{
		iRe = boost::lexical_cast<int>(it->second);
	}else if(_info.size() > 0){
		iRe = 1;
	}

	return iRe;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name: get_fid()
 *  Description: 获取YlemFile文件信息
 * =====================================================================================
 */
string YlemFile::get_fid() const
{
	DBG(logger) << "YlemFile::get_fid(string)";
	string strRe;
	// 需要组合_idx参数
	mapssctor it = _info.find("fid");
	if(it != _info.end())
	{
		strRe = it->second;
	}

	return strRe;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name: get_devid 
 *  Description: 获取设置id操作
 * =====================================================================================
 */
int YlemFile::get_devid(int idx) const
{
	DBG(logger) << "YlemFile::get_devid";
	int iRe=0;
	ostringstream oss;
	oss << "devid";
	// 获取设备ID操作
	if(get_dev_count() > 1)
		oss<<"_"<<idx+1;

	mapssctor it = _info.find(oss.str());
	if(it != _info.end())
	{
		iRe = boost::lexical_cast<int>(it->second);
	}	

	return iRe;
}


/* 
 * ===  FUNCTION  ======================================================================
 *         Name: get_paths
 *  Description: 获取设备号对应的paths
 * =====================================================================================
 */
string YlemFile::get_path(int idx) const
{
	DBG(logger) << "YlemFile::get_paths()";
	string strRe;
	// 查找idx对应paths
	ostringstream oss;
	oss << "path";
	// 获取设备ID操作
	if(get_dev_count() > 1)
		oss<<"_"<<idx+1;
	mapssctor it = _info.find(oss.str());
	if(it != _info.end())
	{
		strRe = it->second;
	}	

	return strRe;
}


/* 
 * ===  FUNCTION  ======================================================================
 *         Name: select_dev  
 *  Description: 选择设备
 * =====================================================================================
 */
int YlemFile::select_dev(int idx)
{
	DBG(logger) << "YlemFile::select_dev";
	int iRe = _idx;
	_idx = idx+1;

	return iRe-1;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name: set_fileinfo 
 *  Description: 设置Ylem文件内部信息
 * =====================================================================================
 */
YlemFile& YlemFile::operator= (const mapss& fileinfo)
{
	DBG(logger) << "YlemFile::operator=";
	_idx = 0;
	_info = fileinfo;
		
	// 构建关闭表信息
	// domain
	mapssor it = _info.find("domain");
	if(it != _info.end()){
		DBG(logger) << "domain: " << it->second;
		_closefile.insert(make_pair("domain",it->second));
	}
	// class
	it = _info.find("class");
	if(it != _info.end()){
		DBG(logger) << "class: " << it->second;
		_closefile.insert(make_pair("class",it->second));
	}
	// key
	it = _info.find("key");
	if(it != _info.end()){
		DBG(logger) << "key: " << it->second;
		_closefile.insert(make_pair("key",it->second));
	}
	// fid
	it = _info.find("fid");
	if(it != _info.end()){
		DBG(logger) << "fid: " << it->second;
		_closefile.insert(make_pair("fid",it->second));
	}

	return *this;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  
 *  Description:  
 * =====================================================================================
 */
YlemFile::operator mapss() const
{
	DBG(logger) << "YlemFile::operator mapss";
	if(get_dev_count() > 1)
	{
		// 选择删除设备
		int iTemp = _idx;
		if(_idx == 0)
			iTemp = 1;
		ostringstream oss;
		oss << "devid_" << iTemp;
		mapssctor it = _info.find(oss.str());
		if(it != _info.end())
			_closefile["devid"] = it->second;
		// path
		oss.str("");
		oss << "path_" << iTemp;
		it = _info.find(oss.str());
		if(it != _info.end())
			_closefile["path"] = it->second;

		return _closefile;
	}

	return _info;
}

}

