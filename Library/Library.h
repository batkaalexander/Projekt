// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the LIBRARY_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// LIBRARY_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef LIBRARY_EXPORTS
#define LIBRARY_API __declspec(dllexport)
#else
#define LIBRARY_API __declspec(dllimport)
#endif

// This class is exported from the Library.dll
//class LIBRARY_API CLibrary {
//public:
//	CLibrary(void);
//	// TODO: add your methods here.
//};
//
//extern LIBRARY_API int nLibrary;
//
//LIBRARY_API int fnLibrary(void);
#include <functional>

LIBRARY_API std::pair< CString, std::vector<CString> > ParseFiles(LPCTSTR lpstrFile);
LIBRARY_API void CalcHistogram(std::vector<int> &histr, std::vector<int> &histg, std::vector<int> &histb, std::vector<int> &histj, void* scan0, UINT32 stride, int height, int width, std::function<bool()> fn);
LIBRARY_API void Threading(std::vector<int> &histr, std::vector<int> &histg, std::vector<int> &histb, std::vector<int> &histj, int width, int height, void* scan0, UINT32 stride, int n, std::function<bool()> fn);
