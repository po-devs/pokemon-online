//!	@file	CrossDynamicLib.cpp
//!	@created	06.07.2008
//!	@author	Chris

#include "CrossDynamicLib.h"

#ifdef WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#include <exception>
#include <string>

//////////////////////////////////////////////////////////////////////////

class DynamicLibException : public std::exception
{
private:
	std::string msg;
public:
	DynamicLibException(const char* what):
		std::exception(),
                msg(what)
	{}
	virtual ~DynamicLibException() throw()
	{}
        virtual const char* what() const throw()
	{    return msg.c_str();    }
};

//////////////////////////////////////////////////////////////////////////

cross::DynamicLibrary::DynamicLibrary(const char* libname):
        libhandle(NULL)
{
#ifdef WIN32
	libhandle = LoadLibrary(libname);
#else
	libhandle = dlopen(libname, RTLD_LOCAL | RTLD_NOW);
#endif
	if(!libhandle)
		throw( DynamicLibException("Library could not be loaded.") );
}

//---------------------------------------------------------------------

cross::DynamicLibrary::~DynamicLibrary()
{
	if(libhandle)
	{
#ifdef WIN32
		FreeLibrary((HINSTANCE)libhandle);
#else
		if(dlclose(libhandle) != 0)
			throw( DynamicLibException("Library could not be closed.") );
#endif
	}
}

//---------------------------------------------------------------------

void* cross::DynamicLibrary::GetFunction(const char* funcname)
{
	void* func = NULL;
	if(!libhandle)
		throw( DynamicLibException("Access to unloaded library.") );

#ifdef WIN32
	func = (void*)GetProcAddress((HINSTANCE)libhandle, funcname);
#else
	func = dlsym(libhandle, funcname);
#endif
	return func;
}

//////////////////////////////////////////////////////////////////////////
//<eof>
