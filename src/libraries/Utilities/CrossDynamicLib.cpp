//!	@file	CrossDynamicLib.cpp
//!	@created	06.07.2008
//!	@author	Chris

#include "CrossDynamicLib.h"

#ifdef _WIN32
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
    DynamicLibException(const std::string & what):
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
#ifdef _WIN32
    libhandle = LoadLibrary(libname);
    if(!libhandle)
        throw( DynamicLibException("Library could not be loaded.") );
#else
    libhandle = dlopen(libname, RTLD_LOCAL | RTLD_NOW);
    if(!libhandle)
        throw( DynamicLibException(std::string("Library could not be loaded -- ") + dlerror() + ".") );
#endif
}

//---------------------------------------------------------------------

cross::DynamicLibrary::~DynamicLibrary()
{
	if(libhandle)
	{
#ifdef _WIN32
        FreeLibrary((HINSTANCE)libhandle);
#else
		if(dlclose(libhandle) != 0)
            throw( DynamicLibException(std::string("Library could not be closed --") + dlerror() + "." ));
#endif
	}
}

//---------------------------------------------------------------------

void* cross::DynamicLibrary::GetFunction(const char* funcname)
{
	void* func = NULL;
	if(!libhandle)
		throw( DynamicLibException("Access to unloaded library.") );

#ifdef _WIN32
        func = (void*)GetProcAddress((HINSTANCE)libhandle, funcname);
#else
	func = dlsym(libhandle, funcname);
#endif
	return func;
}

//////////////////////////////////////////////////////////////////////////
//<eof>
