#ifndef LOGGING_H_
#define LOGGING_H_

#include <log4cpp/Category.hh>
#include "LogConfigure.h"

#define DBG(logger) (logger.debugStream()<<__FILE__<<"/"<<__FUNCTION__<<"["<<__LINE__<<"]: ")
#define INFO(logger) (logger.infoStream()<<__FILE__<<"/"<<__FUNCTION__<<"["<<__LINE__<<"]: ")
#define NOTICE(logger) (logger.noticeStream()<<__FILE__<<"/"<<__FUNCTION__<<"["<<__LINE__<<"]: ")
#define WARN(logger) (logger.warnStream()<<__FILE__<<"/"<<__FUNCTION__<<"["<<__LINE__<<"]: ")
#define ERROR(logger) (logger.errorStream()<<__FILE__<<"/"<<__FUNCTION__<<"["<<__LINE__<<"]: ")
#define CRIT(logger) (logger.critStream()<<__FILE__<<"/"<<__FUNCTION__<<"["<<__LINE__<<"]: ")
#define ALERT(logger) (logger.alertStream()<<__FILE__<<"/"<<__FUNCTION__<<"["<<__LINE__<<"]: ")
#define FATAL(logger) (logger.fatalStream()<<__FILE__<<"/"<<__FUNCTION__<<"["<<__LINE__<<"]: ")
#define EMERG(logger) (logger.emergStream()<<__FILE__<<"/"<<__FUNCTION__<<"["<<__LINE__<<"]: ")

#endif

