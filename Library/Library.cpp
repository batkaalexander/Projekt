// Library.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "Library.h"
#include "../Application/Utils.h"
#include <functional>


//// This is an example of an exported variable
//LIBRARY_API int nLibrary=0;
//
//// This is an example of an exported function.
//LIBRARY_API int fnLibrary(void)
//{
//    return 42;
//}
//
//// This is the constructor of a class that has been exported.
//// see Library.h for the class definition
//CLibrary::CLibrary()
//{
//    return;
//}

//	parse file names from file name string in OPENFILENAME struct
//	returns pair of directory and vector of filenames
//
LIBRARY_API std::pair< CString, std::vector<CString> > ParseFiles(LPCTSTR lpstrFile)
{
	return Utils::ParseFiles(lpstrFile);
}

LIBRARY_API void CalcHistogram(std::vector<int> &histr, std::vector<int> &histg, std::vector<int> &histb, std::vector<int> &histj, void* scan0, UINT32 stride, int height, int width, std::function<bool()> fn)
{
	return Utils::CalcHistogram(histr, histg, histb, histj, scan0, stride, height, width, fn);
}

LIBRARY_API void Threading(std::vector<int> &histr, std::vector<int> &histg, std::vector<int> &histb, std::vector<int> &histj, int width, int height, void* scan0, UINT32 stride, int n, std::function<bool()> fn)
{
	return Utils::Threading(histr, histg, histb, histj, width, height, scan0, stride, n, fn);
}
