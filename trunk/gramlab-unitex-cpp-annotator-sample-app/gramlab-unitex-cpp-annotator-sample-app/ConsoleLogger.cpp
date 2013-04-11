#include "ConsoleLogger.h"

#ifdef _MSC_VER
#include <WinBase.h>
#endif

using namespace uima;
using namespace std;

ConsoleLogger::ConsoleLogger()
{
}

ConsoleLogger::~ConsoleLogger()
{
}

void ConsoleLogger::log(LogStream::EnEntryType enType, 
						string cpszClass,
						string cpszMethod,
						string cpszMsg, 
						long lUserCode) 
{
	//write to local log file if one is available
	string message = cpszClass + " ";
	if (cpszMethod.length() >0) {
		message += cpszMethod + " ";
	}
	message += cpszMsg;
	message = format(enType, message, lUserCode);
	cout << message;

#if defined(_MSC_VER) && defined(_DEBUG)
	// If compiled with Visual Studio, output to debugger console
	OutputDebugString(message.c_str());
#endif
}

string ConsoleLogger::format(LogStream::EnEntryType enType, 
							 const string & cpszMsg, 
							 long lUserCode)  
{

	time_t rawtime;
	time ( &rawtime );
	string currts = ctime(&rawtime);

	stringstream str;
	str << currts.substr(0,currts.length()-1);

	//map enType to string
	switch (enType) {
	case LogStream::EnWarning :
		str << " WARNING: ";
		str << " RC=" << lUserCode << " ";
		break;
	case LogStream::EnError :
		str << " SEVERE: ";
		str << " RC=" << lUserCode << " ";
		break;
	default:
		str << " INFO: ";
		if (lUserCode !=0) {
			str << " RC=" << lUserCode << " ";
		}
	}

	str << cpszMsg << endl;

	return str.str();
}
