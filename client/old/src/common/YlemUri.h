/*
 * =====================================================================================
 * 
 *       Filename:  YlemUri.h 
 *    Description:  
 *        Version:  1.0
 *        Created:  2008年03月27日 10时11分08秒 CST
 *       Revision:  none * 
 *         Author:  changjin (changjing.xu@alibaba-inc.com)
 *        Company:  
 * 
 * =====================================================================================
 */

#ifndef  YLEMURI_H_
#define  YLEMURI_H_

/*
 * =====================================================================================
 *        Class: YlemUri;
 *  Description: 这个类主要是参考ne_uri.h,ne_uri.c中的方法，是为了解决
 *  URL分析问题
 * =====================================================================================
 */

#ifndef BASE_H_
#include "Base.h"
#endif

namespace ylem{

	class YlemUri
	{
	public:
		/* function */
		YlemUri();
		~YlemUri();

		/* Parse a URI-reference 'uri' and place parsed components in *parsed.
		 * Returns zero on success, non-zero on parse error.  On successful or
		 * error return, all the 'char *' fields of *parsed are either set to
		 * NULL, or point to malloc-allocated NUL-terminated strings.
		 * ne_uri_free can be used to free the structure after use. */
		int ylem_uri_parse(const string& uri);
		int ylem_uri_parse(const char* uri);
		/* Data */
		string scheme;
		string host,userinfo;
		unsigned int port;
		string path,query,fragment;
		string _uri;
		operator string() const;
		operator const char*() const;
	protected:
		YlemUri(const YlemUri&);
		YlemUri& operator=(const YlemUri&);
		YlemUri& operator=(const YlemUri&) const;
	};

}
#endif   /* ----- #ifndef YLEMURI_H_  ----- */

