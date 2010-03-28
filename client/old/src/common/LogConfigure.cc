#include <log4cpp/PropertyConfigurator.hh>
#include <log4cpp/Configurator.hh>

#ifndef LOGCONFIGURE_H_
#include "LogConfigure.h"
#endif
#include <iostream>
using namespace::std;

//#define LOGFILE_PATH "/home/y/conf/ylemClient/log4cpp.conf"
#define LOGFILE_PATH "/home/y/conf/ylemClient/log4cpp.conf"

namespace ylem {

	LogConfigure LogConfigure::_instance(LOGFILE_PATH);

	LogConfigure::LogConfigure (const string& logfile)
	{

		std::string filename=logfile;

		if (!filename.length ()) {
			cerr<<"Unable to access log4cpp configuration files, logging turned off. Production config file '"<<logfile<<"'\n";
			return;
		}

		/*
		 *  Attempt to configure the logging object. If make sure, all exceptions
		 *  are caught in this constructor.
		 */
		try {
			log4cpp::PropertyConfigurator::configure (filename);
		} catch (const log4cpp::ConfigureFailure &e) {
			cerr<<"Failed to load log4cpp config file '"<<filename<<"': "<<e.what()<<endl;
		} catch(const std::exception &e) {
			cerr<<"Exception caught while configuring log4cpp via '"<<filename<<"': "<<e.what()<<endl;
		} catch (...) {
			cerr<<"Unknown exception while configuring log4cpp via '"<<filename<<"'"<<endl;
		}
	}

	LogConfigure::~LogConfigure()
	{
	}

}

