
#ifndef __LOG_DEFINES__
#define __LOG_DEFINES__

#ifdef RECORD_LOG
#include <sstream>
#include <windows.h>
#include <log4cplus/logger.h>
#include <log4cplus/configurator.h>
#include <log4cplus/loggingmacros.h>
#endif

#ifndef RECORD_LOG

#define LOG_INIT()
#define LOG_INIT_EX(cfgfile)

// for log in class
#define LOG_CLS_DEC()
#define LOG_CLS_DEC_EX(loggername)
#define LOG_METHOD()
#define LOG_TRACE(msg)
#define LOG_DEBUG(msg)
#define LOG_INFO(msg)
#define LOG_WARN(msg)
#define LOG_ERROR(msg)
#define LOG_FATAL(msg)

// for log in global
#define LOGGER_DEC(logger)
#define LOGGER_IMP(logger, loggername)
#define LOGGER_IMP_EX(logger, loggername)
#define LOGGER_METHOD(logger)
#define LOGGER_TRACE(logger,msg)
#define LOGGER_DEBUG(logger,msg)
#define LOGGER_INFO(logger,msg)
#define LOGGER_WARN(logger,msg)
#define LOGGER_ERROR(logger,msg)
#define LOGGER_FATAL(logger,msg)

#else // !defined(RECORD_LOG)

extern "C" IMAGE_DOS_HEADER __ImageBase;

template <bool t_Managered>
class log4cplus_toolT
{
public:
	static log4cplus::tstring GetModuleLoggerName()
	{
		TCHAR szModuleName[MAX_PATH];
		GetModuleFileName((HMODULE)&__ImageBase, szModuleName, MAX_PATH);
		log4cplus::tstring sOut = szModuleName;
		size_t nStart = sOut.find_last_of(TCHAR('\\'));
		size_t nEnd = sOut.rfind(TCHAR('.'));
		sOut = sOut.substr(nStart+1, nEnd-nStart-1);
		return sOut;
	}
};

typedef log4cplus_toolT<true> log4cplus_tool;


#define LOG_INIT()                              log4cplus::PropertyConfigurator::doConfigure(LOG4CPLUS_TEXT("log4cplus.cfg"))
#define LOG_INIT_EX(cfgfile)                    log4cplus::PropertyConfigurator::doConfigure(cfgfile)

// for log in global
#define LOGGER_DEC(logger)                      extern log4cplus::Logger logger
#define LOGGER_IMP(logger, loggername)          LOGGER_IMP_EX( logger, log4cplus_tool::GetModuleLoggerName() + LOG4CPLUS_TEXT('.') + loggername )
#define LOGGER_IMP_EX(logger, loggername)       log4cplus::Logger logger = log4cplus::Logger::getInstance( loggername )
#define LOGGER_METHOD(logger)                   LOG4CPLUS_TRACE_METHOD(logger, LOG4CPLUS_C_STR_TO_TSTRING(__FUNCTION__))
#define LOGGER_TRACE(logger,msg)                LOG4CPLUS_TRACE(logger, __if_exists(this){'[' << this << ']' <<} LOG4CPLUS_TEXT('[') << __FUNCTIONW__ << "] " << msg )
#define LOGGER_DEBUG(logger,msg)                LOG4CPLUS_DEBUG(logger, __if_exists(this){'[' << this << ']' <<} LOG4CPLUS_TEXT('[') << __FUNCTIONW__ << "] " << msg )
#define LOGGER_INFO(logger,msg)                 LOG4CPLUS_INFO(logger,  __if_exists(this){'[' << this << ']' <<} LOG4CPLUS_TEXT('[') << __FUNCTIONW__ << "] " << msg )
#define LOGGER_WARN(logger,msg)                 LOG4CPLUS_WARN(logger,  __if_exists(this){'[' << this << ']' <<} LOG4CPLUS_TEXT('[') << __FUNCTIONW__ << "] " << msg )
#define LOGGER_ERROR(logger,msg)                LOG4CPLUS_ERROR(logger, __if_exists(this){'[' << this << ']' <<} LOG4CPLUS_TEXT('[') << __FUNCTIONW__ << "] " << msg )
#define LOGGER_FATAL(logger,msg)                LOG4CPLUS_FATAL(logger, __if_exists(this){'[' << this << ']' <<} LOG4CPLUS_TEXT('[') << __FUNCTIONW__ << "] " << msg )

// for log in class
#define LOG_CLS_DEC()                           LOG_CLS_DEC_EX( log4cplus_tool::GetModuleLoggerName() + LOG4CPLUS_TEXT('.') + LOG4CPLUS_C_STR_TO_TSTRING(__FUNCTION__) )
#define LOG_CLS_DEC_EX(loggername)              static log4cplus::Logger & _Logger() { log4cplus::tstring s = loggername; size_t nPos = s.find(TCHAR(':')); s=s.substr(0, nPos); static log4cplus::Logger& s_logger = log4cplus::Logger::getInstance( s ); return s_logger; }
#define LOG_METHOD()                            LOGGER_METHOD(_Logger())
#define LOG_TRACE(msg)                          LOGGER_TRACE(_Logger(), msg)
#define LOG_DEBUG(msg)                          LOGGER_DEBUG(_Logger(), msg)
#define LOG_INFO(msg)                           LOGGER_INFO(_Logger(), msg)
#define LOG_WARN(msg)                           LOGGER_WARN(_Logger(), msg)
#define LOG_ERROR(msg)                          LOGGER_ERROR(_Logger(), msg)
#define LOG_FATAL(msg)                          LOGGER_FATAL(_Logger(), msg)

#endif // RECORD_LOG

#endif // end of __LOG_DEFINES__
