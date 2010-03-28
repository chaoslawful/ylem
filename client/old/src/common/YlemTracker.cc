/*
 * =====================================================================================
 *
 *       Filename:  YlemTracker.cc
 *    Description:  
 *        Version:  1.0
 *        Created:  2008年03月24日 15时10分26秒 CST
 *       Revision:  none *
 *         Author:  changjin (changjing.xu@alibaba-inc.com)
 *        Company:  
 *
 * =====================================================================================
 */

#ifndef YLEMTRACKER_H_
#include "YlemTracker.h"
#endif

#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netdb.h>

namespace ylem{

static log4cpp::Category& logger=log4cpp::Category::getInstance("ylem");

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  YlemTracker
 *  Description:  构造函数
 * =====================================================================================
 */
YlemTracker::YlemTracker():_fd(-1)
{
	DBG(logger) << "YlemTracker::YlemTracker()";
}


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  ~YlemTracker
 *  Description:  析构函数
 * =====================================================================================
 */
YlemTracker::~YlemTracker()
{
	DBG(logger) << "YlemTracker::~YlemTracker()";
	_destroySocket();
}


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  get_connect
 *  Description:  创建到tracker的socket连接
 * =====================================================================================
 */
YlemState YlemTracker::get_connect(const string& host, int port)
{
	_destroySocket();
	return _createSocket(host,port);
}


/* 
 * ===  FUNCTION  ======================================================================
 *         Name: exeCommand 
 *  Description: 执行与tracker交互的命令
 * =====================================================================================
 */
string YlemTracker::exeCommand(const string& command) const
{
	DBG(logger) << "YlemTracker::exeCommand: " << command;

	/* 判断socket是否正确创建 */
	if(_fd == -1)
	{
		DBG(logger) << "ERR failure socket error, Do you forgot call get_connect function";
		return string("ERR failure socket error, Do you forgot call get_connect function");
	}else{
	/* 创建socket */
		if(send(_fd,command.c_str(),command.length(),0)==-1)
		{
			ERROR(logger) << "ERR failure socket error, send failure";
			return string("ERR failure socket error, send failure");
		}else{
			char buf[4096];
			int recvbytes;
			string strRe;
			bool bEnd=false; 	

			while (!bEnd && ((recvbytes=recv(_fd, buf, 4095, 0)) != -1))
			{
				if(recvbytes < 2)
					break;
				if(buf[recvbytes-2] == '\r' && buf[recvbytes-1]=='\n')
				{
					bEnd = true;
					buf[recvbytes-2] = '\0';
				}
				else{
					buf[recvbytes] = '\0';
				}
			
				strRe += buf;
			}

			return strRe;
		}	
	}
}


/* 
 * ===  FUNCTION  ======================================================================
 *         Name: exeCommandBig 
 *  Description: 执行与tracker交互的命令
 * =====================================================================================
 */
string YlemTracker::exeCommandBig(const string& command) const
{
	DBG(logger) << "YlemTracker::exeCommandBig: " << command;

	/* 判断socket是否正确创建 */
	if(_fd == -1)
	{
		DBG(logger) << "ERR failure socket error, Do you forgot call get_connect function";
		return string("ERR failure socket error, Do you forgot call get_connect function");
	}else{
	/* 创建socket */
		if(send(_fd,command.c_str(),command.length(),0)==-1)
		{
			ERROR(logger) << "ERR failure socket error, send failure";
			return string("ERR failure socket error, send failure");
		}else{
			char buf[4096];
			int recvbytes;
			string strRe;
			bool bEnd = false;

			while (!bEnd && ((recvbytes=recv(_fd, buf, 4095, 0)) != -1))
			{
				DBG(logger) << "recv: " << recvbytes;
				DBG(logger) << string(buf,recvbytes);
				if(recvbytes < 2)
					break;
				if(buf[recvbytes-2] == '\r' && buf[recvbytes-1]== '\n')
				{
					bEnd = true;
					buf[recvbytes-2] = '\0';
				}else{
					buf[recvbytes] = '\0';
				}
				strRe += buf;
			}

			return strRe;

		}	
	}
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name: _createSocket
 *  Description: 创建本地到tracker的socket连接
 * =====================================================================================
 */
YlemState YlemTracker::_createSocket(const string& hostname, int port)
{
	DBG(logger) << "YlemTracker::_createSocket";
	YlemState state = YLEM_OK;
	/* create socket */
	_fd = socket(AF_INET,SOCK_STREAM,0);
	if(_fd == -1)
	{
		state = YLEM_SOCKET_ERR;
		ERROR(logger) << "create socket error ";
	}else{
		struct hostent *host;
		if((host=gethostbyname(hostname.c_str())) == NULL)
		{
			state = YLEM_SOCKET_ERR;
			ERROR(logger) << "gethostbyname error";
		}else{
			struct sockaddr_in serv_addr;
			serv_addr.sin_family=AF_INET;
			serv_addr.sin_port=htons(port);
			serv_addr.sin_addr = *((struct in_addr *)host->h_addr);

			bzero( &(serv_addr.sin_zero),8);
			if(connect(_fd,(struct sockaddr *)&serv_addr,sizeof(struct sockaddr)) == -1)
			{
				state = YLEM_SOCKET_ERR;
				ERROR(logger) << "get connection error" ;					
			}
		}
	}

	return state;
}


/* 
 * ===  FUNCTION  ======================================================================
 *         Name: _destroySocket
 *  Description: 销毁本地到tracker的socket连接
 * =====================================================================================
 */
void YlemTracker::_destroySocket()
{
	if(_fd != -1)
	{
		/* close socket */
		close(_fd);
	}
}






}
