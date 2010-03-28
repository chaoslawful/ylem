/*
 * =====================================================================================
 *
 *       Filename:  CSLangue.cc
 *    Description:  
 *        Version:  1.0
 *        Created:  2008年03月24日 17时04分20秒 CST
 *       Revision:  none *
 *         Author:  changjin (changjing.xu@alibaba-inc.com)
 *        Company:  
 *
 * =====================================================================================
 */

#ifndef CSLANGUE_H_
#include "CSLangue.h"
#endif
#include <yaxx/StringUtils.h>   // for toLower()/valueOf()/joinFromVector()

namespace ylem{

static log4cpp::Category& logger = log4cpp::Category::getInstance("ylem");
	
/* 
 * ===  FUNCTION  ======================================================================
 *         Name: createSerLange 
 *  Description: 创建向tracker发送的命令
 * =====================================================================================
 */
string CSLangue::createSerLangue(const string& method,
		const vectorpairss& params ) const
{
	return createSerLangue(method.c_str(),params);
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name: createSerLangue 
 *  Description: 创建向tracker发送的命令
 * =====================================================================================
 */
string CSLangue::createSerLangue(const char* method,
		const mapss& params) const
{
	DBG(logger) << "CSLangue::createSerLangue";
	if(strlen(method) < 1 || params.size() == 0)
		return string();
	
	ostringstream oss;
	oss << method << " ";
	/* 单独处理第一项 */
	/* EQUALSYMBOL value = */

	bool start=true;
	for(mapssctor it=params.begin();it != params.end(); ++it)
	{
		/* JOINSYMBOL value & */
		if((*it).second.size()>0) {
			if(!start) {
				oss << JOINSYMBOL;
			} else {
				start=false;
			}
			oss << (*it).first << EQUALSYMBOL << (*it).second;
		}
	}

	/* end symbol = "\r\n" */
	oss << ENDSYMBOL;

	return string(oss.str());
}


/* 
 * ===  FUNCTION  ======================================================================
 *         Name: createSerLangue
 *  Description: 创建与tracker服务器交互命令
 * =====================================================================================
 */
string CSLangue::createSerLangue(const string& method,
		const mapss& params ) const
{
	return createSerLangue(method.c_str(),params);
}


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  
 *  Description:  
 * =====================================================================================
 */
string CSLangue::createSerLangue(const char* method, 
		const vectorpairss& params)const
{
	mapss mapstrs;
	for(vectorpairssctor it=params.begin(); it != params.end();++it)
	{
		mapstrs.insert((*it));
	}
	return createSerLangue(method,mapstrs);
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name: parseSerLangue
 *  Description:  
 * =====================================================================================
 */
bool CSLangue::parseSerLangue(const string& langue,vectorpairss& out) const
{
	DBG(logger) << "CSLangue::parseSerLangue: " << langue;
	bool bState = false;
	if(langue.length() < 1)
	{
		out.push_back(make_pair("ERR","first param do not equal null"));
		ERROR(logger) << "langue not == NULL";
		return bState;
	}

	/* 返回正确的处理结果 */
	if(langue.find("OK") != string::npos)
	{
		bState = true;
		/* 转化为vector<string>形式，string value a=b */
		vector<string> vectstrs;
		YaxxStringUtils::splitIntoVector(langue.substr(3,langue.length())
				 ,JOINSYMBOL,vectstrs);
		
		vector<string>::iterator it;
		for(it = vectstrs.begin(); it != vectstrs.end(); ++it)
		{
			string::size_type pos;
			pos = (*it).find(EQUALSYMBOL);
			out.push_back(
					make_pair((*it).substr(0,pos),(*it).substr(pos+1,(*it).length())));
			DBG(logger) << "key: " << it->substr(0,pos) << " value: " 
				<< it->substr(pos+1,(*it).length()); 
		}
	}else{
		/* 错误处理结果  */
		bState=false;
		out.push_back(make_pair("ERR",langue.substr(3,langue.length())));
		DBG(logger) << langue;
	}

	return bState;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name: parseSerLangue
 *  Description:  
 * =====================================================================================
 */
bool CSLangue::parseSerLangue(const string& langue,mapss& out) const
{
	vectorpairss vout;
	bool b = parseSerLangue(langue,vout);
	for(vectorpairssor it = vout.begin(); it != vout.end(); ++it)
	{
		out.insert((*it));
	}
	return b;
}

}
