/*
 * =====================================================================================
 * 
 *       Filename:  YlemPutfile.h 
 *    Description:  
 *        Version:  1.0
 *        Created:  2008年03月24日 13时45分06秒 CST
 *       Revision:  none * 
 *         Author:  changjin (changjing.xu@alibaba-inc.com)
 *        Company:  
 * 
 * =====================================================================================
 */

#ifndef  YLEMPUTFILE_H_
#define  YLEMPUTFILE_H_

#ifndef BASE_H_
#include "Base.h"
#endif

/*
 * =====================================================================================
 *        Class: YlemPutfile 
 *  Description: 通过webDAV上传文件
 * =====================================================================================
 */
namespace ylem
{
	class YlemPutfile
	{
		public:
			YlemPutfile();
			~YlemPutfile();
			/*
			 * 上传文件
			 * webPath: webDAV的路径
			 * key: 上传后文件名
			 * filePath: 文件在本地路径+文件名
			 * */
			YlemState putfile(const string& webPath,
					const string& key,
					const string &filePath,int& filesize)const;	
			/**
			 * 上传内存缓冲区
			 * webPath: webDAV的路径
			 * key: 上传后文件名
			 * buf: 内存缓冲区地址
			 * len: 缓冲区长度
			 * */
			YlemState putfile(const string &webPath,
					const string &key,
					const char *buf, uint32_t len,
					int &filesize) const;
		protected:
			YlemPutfile(const YlemPutfile&);
			YlemPutfile& operator=(const YlemPutfile&) const;
			YlemPutfile& operator=(const YlemPutfile&);
			int _usecurlput(const char* url, const char* data, 
					ssize_t len,const char* leninfo, const char* rangeinfo) const; 
	};
}

#endif   /* ----- #ifndef YLEMPUTFILE_H_  ----- */

