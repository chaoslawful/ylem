/*
 * =====================================================================================
 *
 *       Filename:  ylemCinterface.cc
 *    Description:  
 *        Version:  1.0
 *        Created:  2008年04月29日 16时59分16秒 CST
 *       Revision:  none *
 *         Author:  changjin (changjing.xu@alibaba-inc.com)
 *        Company:  
 *
 * =====================================================================================
 */
#ifndef BASE_H_
#include "Base.h"
#endif

#ifndef YLEMCLIENT_H_
#include "YlemClient.h"
using namespace::ylem;
#endif
#ifndef YLEMCINTERFACE_H_
#include "ylemCinterface.h"
#endif

/* 
 * ===  FUNCTION  ======================================================================
 *         Name: create_ylemClient
 *  Description: 创建ylemClient对象
 * =====================================================================================
 */
void* create_ylemClient()
{
	// 创建对象
	YlemClient *client = new YlemClient();
	return reinterpret_cast<void*>(client);			
}


/* 
 * ===  FUNCTION  ======================================================================
 *         Name: delete_ylemClient
 *  Description: 销毁ylemClient对象
 * =====================================================================================
 */
void delete_ylemClient(void* client)
{
	// 对象转换
	YlemClient *pclient = reinterpret_cast<YlemClient*>(client);		
	// 转换失败为NULL时，也不会影响delete操作
	delete (pclient);
}

int connect_ylemClient(void* ylemClient,const char* host,
		int port,const char* domain, const char* classname)
{
	// 对象转换
	YlemClient *client = reinterpret_cast<YlemClient*>(ylemClient);		
	if(client == NULL)
		return -1;

	// 创建对象连接
	bool bState = client->ylem_connect(host,port,domain,classname);  

	// 0表示创建成功,其他表示失败
	return !bState;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name: getpaths_ylemClient
 *  Description: 获取文件详细地址 
 * =====================================================================================
 */
int getpaths_ylemClient(void* ylemClient,char **out,
		const char* key)
{
	// 对象转换
	YlemClient *client = reinterpret_cast<YlemClient*>(ylemClient);		
	if(client == NULL)
		return -1;

	// 获取信息
	string info;
	string mimetype;
	
	YlemState state = client->ylem_getpaths(info,mimetype,key);	
	if(state == YLEM_OK)
	{
		*out = (char*)calloc(1,info.length()+1);
		strcpy(*out,info.c_str());
	}

	return state;
}













