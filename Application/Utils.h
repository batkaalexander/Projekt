#pragma once

#include <utility>
#include <vector>
#include <functional>

namespace Utils
{
	std::pair< CString, std::vector<CString> > ParseFiles(LPCTSTR lpstrFile);
	void CalcHistogram(std::vector<int> &histr, std::vector<int> &histg, std::vector<int> &histb, std::vector<int> &histj, void* scan0, UINT32 stride, int height, int width, std::function<bool()> fn);
	void Threading(std::vector<int> &histr, std::vector<int> &histg, std::vector<int> &histb, std::vector<int> &histj, int width, int height, void* scan0, UINT32 stride, int n, std::function<bool()> fn);
	void Rotate(bool right, Gdiplus::Bitmap &bmp);
}