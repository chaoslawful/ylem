/*
 * =====================================================================================
 * 
 *       Filename:  CSLangue.h 
 *    Description:  
 *        Version:  1.0
 *        Created:  2008年03月24日 17时04分14秒 CST
 *       Revision:  none * 
 *         Author:  changjin (changjing.xu@alibaba-inc.com)
 *        Company:  
 * 
 * =====================================================================================
 */

#ifndef  CSLANGUE_H_
#define  CSLANGUE_H_

#ifndef	BASE_H_
#include "Base.h"
#endif


/*
 * =====================================================================================
 *        Class: CSLange 
 *  Description: 客户端和服务器端交互语言
 * =====================================================================================
 */

namespace ylem{
	class CSLangue{
	public:
		string createSerLangue(const string& method,const vectorpairss& params) const;
		string createSerLangue(const char* method,const vectorpairss& params) const;

		string createSerLangue(const string& method,const mapss& params) const;
		string createSerLangue(const char* method,const mapss& params) const;

		bool parseSerLangue(const string& langue,vectorpairss& out) const;

		bool parseSerLangue(const string& langue,mapss& out) const;
	};
}

#endif   /* ----- #ifndef CSLANGUE_H_  ----- */

