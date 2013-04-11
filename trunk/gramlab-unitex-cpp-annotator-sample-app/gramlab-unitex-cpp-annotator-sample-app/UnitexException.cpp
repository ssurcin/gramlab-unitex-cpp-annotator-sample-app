/*
* UnitexException.cpp
*
*  Created on: 29 déc. 2010
*      Author: sylvainsurcin
*/

#include "UnitexException.h"

#if defined(_MSC_VER) && defined(_DEBUG) && defined(DEBUG_MEMORY_LEAKS)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace std;

namespace unitexcpp
{

	UnitexException::UnitexException(const string& what_arg)
		: runtime_error(what_arg)
	{
		m_code = 1;
	}

	UnitexException::UnitexException(const string& what_arg, int code_arg)
		: runtime_error(what_arg)
	{
		m_code = code_arg;
	}

	UnitexException::~UnitexException() throw()
	{
	}

	int UnitexException::getCode() const 
	{
		return m_code;
	}
}
