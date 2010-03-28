/*
 * =====================================================================================
 * 
 *       Filename:  YlemTracker.h 
 *    Description:  
 *        Version:  1.0
 *        Created:  2008年03月24日 15时10分22秒 CST
 *       Revision:  none * 
 *         Author:  changjin (changjing.xu@alibaba-inc.com)
 *        Company:  
 * 
 * =====================================================================================
 */

#ifndef  YLEMTRACKER_H_
#define  YLEMTRACKER_H_

#ifndef BASE_H_
#include "Base.h"
#endif


/*
 * =====================================================================================
 *        Class:  YlemTracker
 *  Description:  与Tracker通信类
 * =====================================================================================
 */

namespace ylem{

	class YlemTracker
	{
	public:
		/* constructor and destructor function */
		YlemTracker();
		~YlemTracker();

		/* create connection to tracker 
		 * host: tracker host
		 * port: tracker port
		 * */
		YlemState get_connect(const string& host, int port); 
		string exeCommand(const string& command) const;
		string exeCommandBig(const string& command) const;
	protected:
		YlemTracker(const YlemTracker&);
		YlemTracker& operator=(const YlemTracker&);
		YlemTracker& operator=(const YlemTracker&) const;
		inline YlemState _createSocket(const string& host, int port);
		inline void _destroySocket();
	private:
                                                /* 与tracker连接的socket */
		int _fd;
	};

}

#endif   /* ----- #ifndef YLEMTRACKER_H_  ----- */

