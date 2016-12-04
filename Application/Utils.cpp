#include "stdafx.h"
#include "Utils.h"
#include <thread>
#include <functional>

namespace Utils
{
	void Threading(std::vector<int> &histr, std::vector<int> &histg, std::vector<int> &histb, std::vector<int> &histj, int width, int height, void* scan0, UINT32 stride, const int n, std::function<bool()> fn)
	{
		std::vector<std::vector<int>> histRT;
		std::vector<std::vector<int>> histGT;
		std::vector<std::vector<int>> histBT;
		std::vector<std::vector<int>> histJT;
		std::vector<std::thread> thready;
		histRT.assign(n, std::vector<int>());
		histGT.assign(n, std::vector<int>());
		histBT.assign(n, std::vector<int>());
		histJT.assign(n, std::vector<int>());
		for (int i = 0; i < n; i++)
		{
			histRT[i].assign(256, 0);
			histGT[i].assign(256, 0);
			histBT[i].assign(256, 0);
			histJT[i].assign(256, 0);
			thready.push_back(std::thread(&Utils::CalcHistogram, std::ref(histRT[i]), std::ref(histGT[i]), std::ref(histBT[i]), std::ref(histJT[i]), (void *)((UINT32 *)(scan0)+(i*height*(UINT32)stride / (n * sizeof(UINT32)))), (UINT32)stride, height / n, width, fn));
		}
		for (int i = 0; i < n; i++)
		{
			thready[i].join();
		}
		for (int i = 0; i < n; i++)
		{
			for (int j = 0; j < 256; j++)
			{
				histr[j] += histRT[i][j];
				histg[j] += histGT[i][j];
				histb[j] += histBT[i][j];
				histj[j] += histJT[i][j];
			}
		}
	}

	void CalcHistogram(std::vector<int> &histr, std::vector<int> &histg, std::vector<int> &histb, std::vector<int> &histj, void* scan0, UINT32 stride, int height, int width, std::function<bool()> fn)
	{
		UINT32 *pLime = (UINT32*)scan0;
		for (int i = 0; i < height; i++)
		{
			pLime = (UINT32*)((uint8_t*)scan0 + stride*(i));
			if (fn()) return;
			for (int j = 0; j < width; j++)
			{
				histr[((*pLime) >> 16) & 0xff]++;
				histg[((*pLime) >> 8) & 0xff]++;
				histb[((*pLime)) & 0xff]++;
				histj[(unsigned __int64)(0.2126*(((*pLime) >> 16) & 0xff) + 0.7152*(((*pLime) >> 8) & 0xff) + 0.0722*((*pLime) & 0xff))]++;
				pLime++;
			}
		}
		return;
	}

	void Rotate(void* scan0, void* scan0C, UINT32 stride, UINT32 strideC, int height, int width, int right)
	{
		UINT32 *pLime = (UINT32*)scan0;
		UINT32 *pLimeC = (UINT32*)scan0C;
		float radians = (2 * 3.1416f*right) / 360;
		float cosine = (float)cos(radians);
		float sine = (float)sin(radians);

		float Point1x = (-height*sine);
		float Point1y = (height*cosine);
		float Point2x = (width*cosine - height*sine);
		float Point2y = (height*cosine + width*sine);
		float Point3x = (width*cosine);
		float Point3y = (width*sine);

		float minx = min(0, min(Point1x, min(Point2x, Point3x)));
		float miny = min(0, min(Point1y, min(Point2y, Point3y)));
		float maxx = max(Point1x, max(Point2x, Point3x));
		float maxy = max(Point1y, max(Point2y, Point3y));

		int DestBitmapWidth = (int)ceil(fabs(maxx) - minx);
		int DestBitmapHeight = (int)ceil(fabs(maxy) - miny);

		for (int x = 0; x < DestBitmapWidth; x++)
		{
			for (int y = 0; y < DestBitmapHeight; y++)
			{ 
				int SrcBitmapx = (int)((x + minx)*cosine + (y + miny)*sine);
				int SrcBitmapy = (int)((y + miny)*cosine - (x + minx)*sine);
				if (SrcBitmapx >= 0 && SrcBitmapx < width && SrcBitmapy >= 0 &&
					SrcBitmapy < height)
				{
					pLime = (UINT32*)((uint8_t*)scan0 + stride*(SrcBitmapy) + SrcBitmapx* sizeof(UINT32));
					pLimeC = (UINT32*)((uint8_t*)scan0C + strideC*(y)+ x * sizeof(UINT32));
					*pLimeC = *pLime;
					pLime++;
				}
				else
				{
					pLimeC = (UINT32*)((uint8_t*)scan0C + strideC*(y)+x * sizeof(UINT32));
					*pLimeC = ((255 << 16) & 0xff0000) | ((255 << 8) & 0xff00) | (255 & 0xff);
				}
			}
		}
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
			for (; *lpstrFile; ++lpstrFile)
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
