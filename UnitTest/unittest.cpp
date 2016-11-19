#include "stdafx.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include "../Library/Library.h"

#include <vector>
#include "stdafx.h"
#include <stdlib.h>
#include <stdio.h>
#include <fstream>
#include <string>
#include <thread>

namespace UnitTest
{
	TEST_CLASS(ParseFilesUnitTest)
	{
	public:

		TEST_METHOD(ParseFiles_TestOneFile)
		{
			auto result = ParseFiles(L"C:\\directory\\subdirectory\\file.name.ext\0");
			Assert::AreEqual(result.first, L"C:\\directory\\subdirectory\\", L"directory");
			Assert::IsTrue(result.second.size() == 1, L"count of files");
			Assert::AreEqual(result.second[0], L"file.name.ext", "filename");
		}

		TEST_METHOD(ParseFiles_TestMultipleFiles)
		{
			auto result = ParseFiles(L"C:\\directory\\subdirectory\0file1.name.ext\0file2.name.ext\0");
			Assert::AreEqual(result.first, L"C:\\directory\\subdirectory\\", L"directory");
			Assert::IsTrue(result.second.size() == 2, L"count of files");
			Assert::AreEqual(result.second[0], L"file1.name.ext", "filename 1");
			Assert::AreEqual(result.second[1], L"file2.name.ext", "filename 2");
		}
	};

	TEST_CLASS(HistogramUnitTest)
	{
	public:
		TEST_METHOD(TestHistogram)
		{
			std::vector<int> r, g, b, j;
			int h = 256;
			int w = 256;
			r.assign(256, 0);
			g.assign(256, 0);
			b.assign(256, 0);
			j.assign(256, 0);

			//test goes here

			UINT32 pBMP[256][256];
			memset(pBMP, 0, sizeof(UINT32) * h * w);
			for (int i = 0; i < h; i++)
			{
				for (int j = 0; j < w; j++)
				{
					pBMP[i][j] = 0x99FF55;
				}
			}
			CalcHistogram(r, g, b, j, pBMP, w, h, w);
			Assert::AreEqual(r[153], h * w, L"cerveny");
			Assert::AreEqual(g[255], h * w, L"zeleny");
			Assert::AreEqual(b[85], h * w, L"modry");
		}

		TEST_METHOD(TestThreading)
		{
			int a[]{ 1,2,4,8,16 };
			std::vector<int> r, g, b, jas;
			int h = 256;
			int w = 256;
			r.assign(256, 0);
			g.assign(256, 0);
			b.assign(256, 0);
			jas.assign(256, 0);
			std::vector<std::vector<int>> histRT;
			std::vector<std::vector<int>> histGT;
			std::vector<std::vector<int>> histBT;
			std::vector<std::vector<int>> histJT;
			std::vector<std::thread> thready;
			UINT32 pBMP[256][256];
			memset(pBMP, 0, sizeof(UINT32) * h * w);
			for (int i = 0; i < h; i++)
			{
				for (int j = 0; j < w; j++)
				{
					pBMP[i][j] = 0x99FF55;
				}
			}
			for each (int var in a)
			{
				histRT.clear();
				histGT.clear();
				histBT.clear();
				histJT.clear();
				thready.clear();
				histRT.assign(var, std::vector<int>());
				histGT.assign(var, std::vector<int>());
				histBT.assign(var, std::vector<int>());
				histJT.assign(var, std::vector<int>());
				for (int i = 0; i < var; i++)
				{
					histRT[i].assign(256, 0);
					histGT[i].assign(256, 0);
					histBT[i].assign(256, 0);
					histJT[i].assign(256, 0);
					thready.push_back(std::thread(&CalcHistogram, std::ref(histRT[i]), std::ref(histGT[i]), std::ref(histBT[i]), std::ref(histJT[i]), (void *)((UINT32 *)(pBMP) + (i*h*w / (var * sizeof(UINT32)))), w, h / var, w));
				}
				for (int i = 0; i < var; i++)
				{
					thready[i].join();
				}
				for (int i = 0; i < var; i++)
				{
					for (int j = 0; j < 256; j++)
					{
						r[j] += histRT[i][j];
						g[j] += histGT[i][j];
						b[j] += histBT[i][j];
						jas[j] += histJT[i][j];
					}
				}
				Assert::AreEqual(r[153], h * w, L"cerveny");
				Assert::AreEqual(g[255], h * w, L"zeleny");
				Assert::AreEqual(b[85], h * w, L"modry");
				r.assign(256, 0);
				g.assign(256, 0);
				b.assign(256, 0);
				jas.assign(256, 0);
			}
		}
	};
}
