#include "stdafx.h"
#include "Utils.h"

namespace Utils
{
	void CalcHistogram(std::vector<int> &histr, std::vector<int> &histg, std::vector<int> &histb, std::vector<int> &histj, void* scan0, UINT32 stride, int height, int width)
	{
		UINT32 *pLime = (UINT32*)scan0;
		for (int i = 0; i < height; i++)
		{
			pLime = (UINT32*)((uint8_t*)scan0 + stride*(i));
			for (int j = 0; j < width; j++)
			{
				//bitmp->GetPixel(i, j, &tmp);
				histr[((*pLime) >> 16) & 0xff]++;
				histg[((*pLime) >> 8) & 0xff]++;
				histb[(*pLime) & 0xff]++;
				histj[(double)(0.2126*(((*pLime) >> 16) & 0xff) + 0.7152*(((*pLime) >> 8) & 0xff) + 0.0722*((*pLime) & 0xff))]++;
				pLime++;
			}
		}
		return;
	}
	//	parse file names from file name string in OPENFILENAME struct
	//	returns pair of directory and vector of filenames
	//
	std::pair< CString, std::vector<CString> > ParseFiles(LPCTSTR lpstrFile)
	{
		CString cs = lpstrFile;

		// skip directory name
		while (*lpstrFile) ++lpstrFile;

		if (*(++lpstrFile))
		{
			CString csDirectory;
			std::vector<CString> names;

			csDirectory = cs + _T("\\");
			// iterate filenames
			for ( ; *lpstrFile; ++lpstrFile)
			{
				names.push_back(lpstrFile);

				while (*lpstrFile) ++lpstrFile;
			}

			return std::make_pair(csDirectory, names);
		}
		else
		{	// only one filename
			CString csName, csExt;
			_tsplitpath_s(cs, nullptr, 0, nullptr, 0, csName.GetBuffer(_MAX_FNAME), _MAX_FNAME, csExt.GetBuffer(_MAX_EXT), _MAX_EXT);
			csName.ReleaseBuffer();
			csExt.ReleaseBuffer();

			return std::make_pair(cs.Left(cs.GetLength() - csName.GetLength() - csExt.GetLength()), std::vector<CString>({ csName + csExt }));
		}

	}
}
