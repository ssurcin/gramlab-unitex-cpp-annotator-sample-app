#ifndef CONSOLELOGGER_H_
#define CONSOLELOGGER_H_

#include "uima/api.hpp"
#include "uima/log.hpp"
#include <string>

class ConsoleLogger : public uima::Logger 
{
public:
	ConsoleLogger();
	virtual ~ConsoleLogger();

	void log(uima::LogStream::EnEntryType enType, std::string cpszClass, std::string cpszMethod, std::string cpszMsg, long lUserCode);

private:
	std::string format(uima::LogStream::EnEntryType enType,	const std::string& cpszMsg, long lUserCode) ;
};

#endif // CONSOLELOGGER_H_
