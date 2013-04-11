/*
* Licensed to the Apache Software Foundation (ASF) under one
* or more contributor license agreements.  See the NOTICE file
* distributed with this work for additional information
* regarding copyright ownership.  The ASF licenses this file
* to you under the Apache License, Version 2.0 (the
* "License"); you may not use this file except in compliance
* with the License.  You may obtain a copy of the License at
*
*   http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing,
* software distributed under the License is distributed on an
* "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
* KIND, either express or implied.  See the License for the
* specific language governing permissions and limitations
* under the License.
*/

#include "ExampleApplication.h"
#include <sstream>

#include "uima/api.hpp"
#include "uima/dirwalk.hpp"
#include "uima/filename.hpp"
#include "uima/xmlwriter.hpp"

#include <io.h>
#include <sys/stat.h>

#include "UnitexException.h"
#include "UnitexDocumentParameters.h"
#include "TransductionOutputAnnotation.h"
#include "LanguageArea.h"

using namespace std;
using namespace uima;
using namespace unitexcpp;
using namespace unitexcpp::annotation;

/**
* An example application that reads documents from files, sends them
* though a Text Analysis Engine, and prints all discovered annotations to
* the console.
* <p>
* The application takes two arguments:
* <ol type="1">
* <li>The path to an XML descriptor for the TAE to be executed</li>
* <li>An input directory containing files to be processed</li>
* <li>Optionally, the logging level</li>
* </ol>
*
*
*/

/////////////////////////////////////////////////////////////////////////////
//
// ExampleApplication class implementation
//
/////////////////////////////////////////////////////////////////////////////
// Static data
// **************************************************************************

const string ExampleApplication::cs_engineDescriptor = "UnitexAnnotatorCpp.xml";

// Construction & destruction
// **************************************************************************

ExampleApplication::ExampleApplication(const char* szInputPath, int logLevel)
	: m_fileLogger("gramlab.log")
{
	if (!fileExists(cs_engineDescriptor.c_str())) {
		ostringstream oss;
		oss << "Error - Cannot open file " << cs_engineDescriptor;
		throw UnitexException(oss.str());
	}

	if (!directoryExists(szInputPath) && !fileExists(szInputPath)) {
		ostringstream oss;
		oss << "The Input path " << szInputPath << " does not exist!";
		throw UnitexException(oss.str());
	}
	m_inputPath = szInputPath;

	if (logLevel < LogStream::EnMessage) {
		ostringstream oss;
		oss << "LogLevel less than minimum value (Message) = " << LogStream::EnMessage;
		throw UnitexException(oss.str());
	}
	if (logLevel > LogStream::EnError) {
		ostringstream oss;
		oss << "LogLevel greater than maximum value (Error) = " << LogStream::EnError;
		throw UnitexException(oss.str());
	}
	m_logLevel = (LogStream::EnEntryType) logLevel;

	initializeResourceManager();
	initializeAnalysisEngine();
	initializeCAS();
}

ExampleApplication::~ExampleApplication()
{
	if (m_pCurrentCAS) 
		delete m_pCurrentCAS;

	try {
	}
	catch (Exception e) {
		cout << "ExampleApplication " << e << endl;
	}
	if (m_pAnalysisEngine) {
		TyErrorId utErrorId = m_pAnalysisEngine->destroy();
		checkError(utErrorId, *m_pAnalysisEngine);
		delete m_pAnalysisEngine;
	}
}

// UIMA initialization
// **************************************************************************

/**
* Create/link up to a resource manager instance (singleton)
* and set the log level.
*/
void ExampleApplication::initializeResourceManager()
{
	ResourceManager::createInstance("GRAMLAB_EXAMPLE_APPLICATION");
	ResourceManager& resMgr = ResourceManager::getInstance();
	resMgr.registerLogger(&m_consoleLogger);
	resMgr.registerLogger(&m_fileLogger);
	resMgr.setLoggingLevel(m_logLevel);
	resMgr.getLogger().logMessage("UIMA Resource Manager initialized");
}

/**
* Initializes the analysis engine with the filename of its descriptor.
*/
void ExampleApplication::initializeAnalysisEngine()
{
	ErrorInfo errorInfo;       
	m_pAnalysisEngine = Framework::createAnalysisEngine(cs_engineDescriptor.c_str(), errorInfo);
	checkError(errorInfo);
	m_pAnalysisEngine->getAnnotatorContext().getLogger();
}

/**
* Gets a new CAS and initializes its type system.
*/
void ExampleApplication::initializeCAS()
{
	m_pCurrentCAS = m_pAnalysisEngine->newCAS();

	if (unitexcpp::annotation::UnitexDocumentParameters::initializeTypeSystem(m_pCurrentCAS->getTypeSystem()) != UIMA_ERR_NONE) {
		ostringstream oss;
		oss << "Could not initialize type system! Error initializing UnitexDocumentParameters type system";
		throw UnitexException(oss.str());
	}

	if (unitexcpp::annotation::TransductionOutputAnnotation::initializeTypeSystem(m_pCurrentCAS->getTypeSystem()) != UIMA_ERR_NONE) {
		ostringstream oss;
		oss << "Could not initialize type system! Error initializing TransductionOutputAnnotation type system";
		throw UnitexException(oss.str());
	}

	if (unitexcpp::annotation::LanguageArea::initializeTypeSystem(m_pCurrentCAS->getTypeSystem()) != UIMA_ERR_NONE) {
		ostringstream oss;
		oss << "Could not initialize type system! Error initializing LanguageArea type system";
		throw UnitexException(oss.str());
	}
}

// Processing
// **************************************************************************

/**
* Processes the input, either the contents of a directory, or a single file.
*/
void ExampleApplication::processInput()
{
	util::DirectoryWalk dirwalker(m_inputPath.c_str());
	if (dirwalker.isValid()) {
		util::Filename infile(m_inputPath.c_str(), "FilenamePlaceHolder");
		while (dirwalker.isValid()) {
			// Process all files or just the ones with matching suffix
			if (dirwalker.isFile() && dirwalker.matchesWildcardPattern("*.txt") && !dirwalker.matchesWildcardPattern("*_out.txt")) {
					infile.setNewName(dirwalker.getNameWithoutPath());
					std::string afile(infile.getAsCString());

					//process the file
					processFile(afile);

					//reset the cas
					m_pCurrentCAS->reset();
			}
			//get the next xcas file in the directory
			dirwalker.setToNext();
		}
	} 
	else {
		/* If has no directory entries then probably a file */
		cout << "ExampleApplication: processing file " << m_inputPath << endl;
		processFile(m_inputPath);
	}

	// Signal collection is complete
	TyErrorId utErrorId = m_pAnalysisEngine->collectionProcessComplete();
	checkError(utErrorId, *m_pAnalysisEngine);
}

/**
* Processes a single file.
*/
void ExampleApplication::processFile(std::string const& filename) 
{
	if (m_logLevel == LogStream::EnEntryType::EnMessage)
		cout << "processing file " << filename << endl;

	// Read in file contents and set CAS Document text 
	FILE * pFile = fopen(filename.c_str(),"rb");
	int filesize;
	if (pFile == NULL) {
		ostringstream oss;
		oss << "ExampleApplication: Error reading file " << filename;
		throw UnitexException(oss.str());
	}

	// Allocate buffer for file contents
	struct stat fstat;
	stat(filename.c_str(), &fstat);
	filesize = fstat.st_size;
	char * pBuffer = new char[filesize+1];
	if (pBuffer == NULL) {
		ostringstream oss;
		oss << "ExampleApplication: Error allocating buffer to hold contents of file  " << filename;
		throw UnitexException(oss.str());
	}

	// Read the file
	size_t numread = fread(pBuffer,1,filesize,pFile);
	fclose(pFile);

	// Convert to unicode and set CAS document text
	UnicodeString ustrInputText(pBuffer, filesize, "utf-8");
	size_t documentLength = ustrInputText.length();
	m_pCurrentCAS->setDocumentText(ustrInputText.getBuffer(), documentLength, true);
	delete[] pBuffer;

	// Create a UnitexDocumentParameters annotation in the CAS
	// whithout which the UnitexAnnotatorCpp cannot work.
	CAS* pView = m_pCurrentCAS->getView("_InitialView");
	UnitexDocumentParameters parameters = UnitexDocumentParameters::createUnitexDocumentParameters(*pView);
	parameters.setAnalysisStrategy("GENERAL");
	parameters.setSkip(false);
	parameters.setUri(filename.c_str());

	// Create a language area in French around the whole document
	LanguageArea languageArea = LanguageArea::createLanguageArea(*pView, 0, documentLength, UNICODE_STRING_SIMPLE("French"));
	cout << "Created a language area in " << languageArea.getLanguage() << " at (" << languageArea.getBegin() << ", " << languageArea.getEnd() << ")" << endl;
	cout << "covering: " << languageArea.getCoveredText() << endl;

	// Process the CAS by calling the UnitexAnnotatorCpp::process method
	TyErrorId utErrorId =  m_pAnalysisEngine->process(*m_pCurrentCAS);
	checkError(utErrorId, *m_pAnalysisEngine);

	// serializes the CAS
	serializeCAS(filename);
	serializeUnitexAnnotations(filename);
}

// Serialization
// **************************************************************************

void ExampleApplication::serializeCAS(string const& filename)
{
	util::Filename xmiFilename(filename.c_str());
	xmiFilename.setNewExtension(".xmi");

	if (m_logLevel == LogStream::EnEntryType::EnMessage)
		cout << "ExampleApplication: write out CAS into file " << xmiFilename << endl;

	ofstream ofs(xmiFilename.getAsCString(), ios::out);
	if (!ofs.is_open()) {
		ostringstream oss;
		oss << "Unable to create output file: " << xmiFilename;
		throw UnitexException(oss.str());
	}
	XCASWriter writer(*m_pCurrentCAS, true);
	writer.write(ofs);
	ofs.close();
}

void ExampleApplication::serializeUnitexAnnotations(string const& filename)
{
	util::Filename outFilename(filename.c_str());
	string name = outFilename.getName();
	string extension = outFilename.getExtension();
	string newName = name.substr(0, name.length() - extension.length()) + "_out" + extension;
	outFilename.setNewName(newName.c_str());

	if (m_logLevel == LogStream::EnEntryType::EnMessage)
		cout << "ExampleApplication: write out matches into " << outFilename << endl;

	ofstream ofs(outFilename.getAsCString(), ios::out);
	if (!ofs.is_open()) {
		ostringstream oss;
		oss << "Unable to create output file: " << outFilename;
		throw UnitexException(oss.str());
	}

	ANIndex index = m_pCurrentCAS->getAnnotationIndex(TransductionOutputAnnotation::getType());
	if (!index.isValid()) {
		ostringstream oss;
		oss << "Cannot get a valid annotation index!";
		throw UnitexException(oss.str());
	}

	ANIterator it = index.iterator();
	it.moveToFirst();
	while (it.isValid()) {
		TransductionOutputAnnotation outputAnnotation(it.get());
		ofs << "(" << outputAnnotation.getBegin() << ", " << outputAnnotation.getEnd() << "): "
			<< outputAnnotation.getCoveredText() << endl
			<< outputAnnotation.getOutput()
			<< endl << endl;
		it.moveToNext();
	}

	ofs.close();
}

// Helper methods
// **************************************************************************

bool ExampleApplication::directoryExists(const char* szPath) 
{
	if (_access(szPath, 0) == 0) {
		struct stat status;
		stat(szPath, &status);
		return (status.st_mode & S_IFDIR) != 0;
	}
	return false;
}

bool ExampleApplication::fileExists(const char* szPath) 
{
	bool result = true;
	ifstream ifs;
	ifs.open(szPath);
	result = ifs.is_open();
	ifs.close();
	return result;
}

void ExampleApplication::checkError(ErrorInfo const& errInfo) 
{
	if (errInfo.getErrorId() != UIMA_ERR_NONE) {
		ostringstream oss;
		oss << "   *** ExampleApplication - Error info:" << endl
			<< "Error string  : "
			<< AnalysisEngine::getErrorIdAsCString(errInfo.getErrorId())
			<< errInfo << endl;                      /* (errInfo starts with a newline) */
		throw UnitexException(oss.str(), (int)errInfo.getErrorId());
	}
}

void ExampleApplication::checkError(TyErrorId utErrorId, const AnalysisEngine&  crEngine) 
{
	if (utErrorId != UIMA_ERR_NONE) {
		ostringstream oss;
		oss << "   *** ExampleApplication - Error info:" << endl;
		oss << "Error number        : "
			<< utErrorId << endl;
		oss << "Error string        : "
			<< AnalysisEngine::getErrorIdAsCString(utErrorId) << endl;
		const TCHAR* errStr = crEngine.getAnnotatorContext().getLogger().getLastErrorAsCStr();
		if (errStr != NULL)
			oss << "  Last logged message : "  << errStr << endl;
		throw UnitexException(oss.str(), (int)utErrorId);
	}
}

/* <EOF> */



