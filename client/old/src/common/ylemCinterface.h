/*
 * =====================================================================================
 * 
 *       Filename:  ylemCinterface.h 
 *    Description:  
 *        Version:  1.0
 *        Created:  2008年04月29日 16时35分30秒 CST
 *       Revision:  none * 
 *         Author:  changjin (changjing.xu@alibaba-inc.com)
 *        Company:  
 * 
 * =====================================================================================
 */

#ifndef  YLEMCINTERFACE_H_
#define  YLEMCINTERFACE_H_

#ifndef YLEMCLIENT_H_
#include "YlemClient.h"
using namespace::ylem;
#endif

/*
 * =====================================================================================
 *  Description: 提供C接口
 * =====================================================================================
 */
#ifdef __cplusplus
extern "C" {
#endif
	/* 创建ylemClient对象 */
	void* create_ylemClient();
	/* 销毁ylemClient对象 */
	void delete_ylemClient(void*);
	/* 创建连接,0表示连接成功 */
	int connect_ylemClient(void* ylemClient,const char* host,
			int port,const char* domain, const char* classname);		
	/* 获取getpaths */
	int getpaths_ylemClient(void* ylemClient,char **out,
			const char* key);
#ifdef __cplusplus
}
#endif

#endif   /* ----- #ifndef YLEMCINTERFACE_H_  ----- */

