#include "ExampleApplication.h"
#include "UnitexException.h"
#include <iostream>
#include "uima/api.hpp"

using namespace std;
using namespace unitexcpp;

void tell() 
{
	cerr << "Usage: LinguisticResourcesDirectory InputDirectory <-l LogLevel>" << endl;
}

int main(int argc, char* argv[]) 
{
	int exitCode = 0;
	try 
	{
		int loglevel = -1;

		// Check  the number of command line args
		if ((argc != 2) && (argc != 4)) {
			tell();
			return 1;
		}

		// Get the log level (if any)
		if (argc == 4) {
			if (!strcmp(argv[2], "-l")) {
				loglevel = atoi(argv[3]);
			} else {
				cerr << "Unexpected option: " << argv[2] << endl;
				tell();
				return 1;
			}
		}

		ExampleApplication application(argv[1], loglevel);
		application.processInput();
	} 
	catch (uima::Exception const& e) {
		cerr << "ExampleApplication " << e << endl;
		exitCode = -1;
	}
	catch (UnitexException& e) {
		cerr << "Exception: " << e.what() << endl;
		exitCode = e.getCode();
	}

	/* If we got this far everything went OK */
	if (!exitCode)
		cout << "ExampleApplication: processing finished sucessfully! " << endl;

	cout << "Press a key to exit." << endl;
	cin.ignore();

	return exitCode;
}


