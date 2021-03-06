#pragma once

#include <utility>
#include <vector>
#include <functional>

namespace Utils
{
	std::pair< CString, std::vector<CString> > ParseFiles(LPCTSTR lpstrFile);
	void CalcHistogram(std::vector<int> &histr, std::vector<int> &histg, std::vector<int> &histb, std::vector<int> &histj, void* scan0, UINT32 stride, int height, int width, std::function<bool()> fn);
	void Threading(std::vector<int> &histr, std::vector<int> &histg, std::vector<int> &histb, std::vector<int> &histj, int width, int height, void* scan0, UINT32 stride, int n, std::function<bool()> fn);
	void Rotate(void* scan0, void* scan0C, UINT32 stride, UINT32 strideC, int height, int width, int right, const int n, std::function<bool()> fn);
	void RotateThreadingAlg(void* scan0, void* scan0C, UINT32 stride, UINT32 strideC, int height, int width, int startx, int starty, int endx, int endy, float minx, float miny, float maxx, float maxy, float sine, float cosine, std::function<bool()> fn);
}