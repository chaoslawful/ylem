/*
 * =====================================================================================
 *
 *       Filename:  ylemUri.cc
 *    Description:  
 *        Version:  1.0
 *        Created:  2008年03月27日 10时11分14秒 CST
 *       Revision:  none *
 *         Author:  changjin (changjing.xu@alibaba-inc.com)
 *        Company:  
 *
 * =====================================================================================
 */

#ifndef YLEMURI_H_
#include "YlemUri.h"
#endif

namespace ylem
{

static log4cpp::Category& logger = log4cpp::Category::getInstance("ylem");	

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  YlemUri
 *  Description:  构造函数
 * =====================================================================================
 */
YlemUri::YlemUri():port(0)
{
	DBG(logger) << "YlemUri::YlemUri";
}


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  ~YlemUri
 *  Description:  析构函数
 * =====================================================================================
 */
YlemUri::~YlemUri()
{
	DBG(logger) << "YlemUri::~YlemUri";
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  ylem_uri_parse
 *  Description:  分析uri结构
 * =====================================================================================
 */
int YlemUri::ylem_uri_parse(const string& uri)
{
	_uri = uri;
	return ylem_uri_parse(uri.c_str());
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name: ylem_uri_parse 
 *  Description: 分析uri的结构
 * =====================================================================================
 */
int YlemUri::ylem_uri_parse(const char* uri)
{
	
	DBG(logger) << "YlemUri::ylem_uri_parse uri is " << uri;
	const char *p, *s;
 
    p = s = uri;
 
    /* => s = p = URI-reference */
 
    if (uri_lookup(*p) & URI_ALPHA) {
        while (uri_lookup(*p) & URI_SCHEME)
            p++;
 
        if (*p == ':') {
            scheme.assign(s,p);
			DBG(logger) << "find sechme,sechme is " << scheme;
            s = p + 1;
        }else{
			scheme="http";
		}
    }

    /* => s = heir-part, or s = relative-part */
 
    if (s[0] == '/' && s[1] == '/') {
        const char *pa;
 
        /* => s = "//" authority path-abempty (from expansion of
         * either heir-part of relative-part)  */
 
        /* authority = [ userinfo "@" ] host [ ":" port ] */
 
        s = pa = s + 2; /* => s = authority */
 
        while (*pa != '/' && *pa != '\0')
            pa++;
        /* => pa = path-abempty */
 
        p = s;
        while (p < pa && uri_lookup(*p) & URI_USERINFO)
            p++;
 
        if (*p == '@') {
            userinfo.assign(s,p-s);
            s = p + 1;
        }
        /* => s = host */
 
        if (s[0] == '[') {
            p = s + 1;
 
            while (*p != ']' && p < pa)
                p++;
 
            if (p == pa || (p + 1 != pa && p[1] != ':')) {
                /* Ill-formed IP-literal. */
                return -1;
            }
 
            p++; /* => p = colon */
        } else {
            /* Find the colon. */
            p = pa;
            while (*p != ':' && p > s)
                p--;
        }
 
        if (p == s) {
            p = pa;
            /* No colon; => p = path-abempty */
        } else if (p + 1 != pa) {
            /* => p = colon */
            port = atoi(p + 1);
			DBG(logger) << "find port, port is " << port;
        }
        host.assign(s,p-s);
		DBG(logger) << "find host, host is " << host;
 
        s = pa;
 
        if (*s == '\0') {
            s = "/"; /* FIXME: scheme-specific. */
        }
    }

    /* => s = path-abempty / path-absolute / path-rootless
     *      / path-empty / path-noscheme */
 
    p = s;
 
    while (uri_lookup(*p) & URI_SEGCHAR)
        p++;
 
    /* => p = [ "?" query ] [ "#" fragment ] */
 
    path.assign(s, p - s);
	DBG(logger) << "find path, path is " << path;
 
    if (*p != '\0') {
        s = p++;
 
        while (uri_lookup(*p) & URI_QUERY)
            p++;
 
        /* => p = [ "#" fragment ] */
        /* => s = [ "?" query ] [ "#" fragment ] */
 
        if (*s == '?') {
            query.assign(s + 1, p - s - 1);
 
            if (*p != '\0') {
                s = p++;
 
                while (uri_lookup(*p) & URI_FRAGMENT)
                    p++;
            }
        }
 
        /* => p now points to the next character after the
         * URI-reference; which should be the NUL byte. */
 
        if (*s == '#') {
            fragment.assign(s + 1, p - s - 1);
        }
        else if (*p || *s != '?') {
            return -1;
        }
    }
 
    return 0;
}


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  
 *  Description:  
 * =====================================================================================
 */
YlemUri::operator string() const
{
	return _uri;
}


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  
 *  Description:  
 * =====================================================================================
 */
YlemUri::operator const char*() const
{
	return _uri.c_str();
}

}











