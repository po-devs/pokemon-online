//!	@file	CrossDynamicLib.cpp
//!	@created	06.07.2008
//!	@author	Chris

#ifndef CROSSDYNAMICLIB_H_
#define CROSSDYNAMICLIB_H_

namespace cross
{
	//!	@class	DynamicLibrary
	//!	@brief	base dynamic library loader class
	class DynamicLibrary
	{
	private:
		void* libhandle;

	public:
		DynamicLibrary(const char* libname);    //! creates the library class from the given filename (with path and extension on *NIX systems!)
		~DynamicLibrary();

		void* GetFunction(const char* funcname);//!    returns a pointer to the requested function
	};
}// namespace cross

#endif // CROSSDYNAMICLIB_H_
