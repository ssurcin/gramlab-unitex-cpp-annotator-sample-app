#ifndef EXAMPLEAPPLICATION_H_
#define EXAMPLEAPPLICATION_H_

#include "uima/api.hpp"
#include <string>
#include "ConsoleLogger.h"

class ExampleApplication
{
private:
	static const std::string cs_engineDescriptor;
	
	std::string m_inputPath;
	uima::LogStream::EnEntryType m_logLevel;
	uima::AnalysisEngine* m_pAnalysisEngine;
	uima::CAS* m_pCurrentCAS;
	ConsoleLogger m_consoleLogger;
	uima::FileLogger m_fileLogger;

public:
	ExampleApplication(const char* szInputPath, int logLevel =2);
	virtual ~ExampleApplication();

	void processInput();
private:
	void processFile(std::string const& filename);
	void serializeCAS(std::string const& filename);
	void serializeUnitexAnnotations(std::string const& filename);

private:
	static bool fileExists(const char* szPath);
	static bool directoryExists(const char* szPath);

	static void checkError(uima::ErrorInfo const& errInfo);
	static void checkError(uima::TyErrorId utErrorId, const uima::AnalysisEngine&  crEngine);

	void initializeResourceManager();
	void initializeAnalysisEngine();
	void initializeCAS();
};

#endif // EXAMPLEAPPLICATION_H_
