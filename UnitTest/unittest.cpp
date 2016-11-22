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
			CalcHistogram(r, g, b, j, pBMP, w, h, w, [w]() {return false; });
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
				Threading(r, g, b, jas, w, h, pBMP, w, var, [w]() {return false; });
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
