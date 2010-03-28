#ifndef LOGCONFIGURE_H_
#define LOGCONFIGURE_H_

#ifndef BASE_H_
#include "Base.h"
#endif

namespace ylem {

	/**
	 *  @class      LogConfigure
	 *
	 *  @brief
	 *  Class to initialize log4cpp.  The LogInitializer class manages log4cpp
	 *  initialization.  The idea is that there should be one static instance of
	 *  this class so that log4cpp gets initialized before any of it's client
	 *  libraries are called.
	 */
	class LogConfigure
	{
	public:
		LogConfigure (const std::string& logfile);
		~LogConfigure ();

	private:
		LogConfigure ();
		LogConfigure& operator=(const LogConfigure&)const;
		LogConfigure& operator=(const LogConfigure&);

		static LogConfigure _instance;
	};

}

#endif

