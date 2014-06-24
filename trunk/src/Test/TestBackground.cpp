/*
* Simd Library Tests.
*
* Copyright (c) 2011-2014 Yermalayeu Ihar.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/
#include "Test/TestUtils.h"
#include "Test/TestPerformance.h"
#include "Test/TestData.h"
#include "Test/Test.h"

namespace Test
{
	namespace
	{
		struct Func1
		{
			typedef void (*FuncPtr)(const uint8_t * value, size_t valueStride, size_t width, size_t height,
				 uint8_t * lo, size_t loStride, uint8_t * hi, size_t hiStride);

			FuncPtr func;
			std::string description;

			Func1(const FuncPtr & f, const std::string & d) : func(f), description(d) {}

			void Call(const View & value, const View & loSrc, const View & hiSrc, View & loDst, View & hiDst) const
			{
				Simd::Copy(loSrc, loDst);
				Simd::Copy(hiSrc, hiDst);
				TEST_PERFORMANCE_TEST(description);
				func(value.data, value.stride, value.width, value.height, loDst.data, loDst.stride, hiDst.data, hiDst.stride);
			}
		};
	}

#define FUNC1(function) Func1(function, std::string(#function))

	bool BackgroundChangeRangeAutoTest(int width, int height, const Func1 & f1, const Func1 & f2)
	{
		bool result = true;

		std::cout << "Test " << f1.description << " & " << f2.description << " [" << width << ", " << height << "]." << std::endl;

		View value(width, height, View::Gray8, NULL, TEST_ALIGN(width));
		FillRandom(value);
		View loSrc(width, height, View::Gray8, NULL, TEST_ALIGN(width));
		FillRandom(loSrc);
		View hiSrc(width, height, View::Gray8, NULL, TEST_ALIGN(width));
		FillRandom(hiSrc);

		View loDst1(width, height, View::Gray8, NULL, TEST_ALIGN(width));
		View hiDst1(width, height, View::Gray8, NULL, TEST_ALIGN(width));
		View loDst2(width, height, View::Gray8, NULL, TEST_ALIGN(width));
		View hiDst2(width, height, View::Gray8, NULL, TEST_ALIGN(width));

		TEST_EXECUTE_AT_LEAST_MIN_TIME(f1.Call(value, loSrc, hiSrc, loDst1, hiDst1));

		TEST_EXECUTE_AT_LEAST_MIN_TIME(f2.Call(value, loSrc, hiSrc, loDst2, hiDst2));

		result = result && Compare(loDst1, loDst2, 0, true, 32, 0, "lo");
		result = result && Compare(hiDst1, hiDst2, 0, true, 32, 0, "hi");

		return result;
	}

    bool BackgroundChangeRangeAutoTest(const Func1 & f1, const Func1 & f2)
    {
        bool result = true;

        result = result && BackgroundChangeRangeAutoTest(W, H, f1, f2);
        result = result && BackgroundChangeRangeAutoTest(W + 3, H - 3, f1, f2);
        result = result && BackgroundChangeRangeAutoTest(W - 3, H + 3, f1, f2);

        return result;
    }

	namespace
	{
		struct Func2
		{
			typedef void (*FuncPtr)(const uint8_t * value, size_t valueStride, size_t width, size_t height,
				const uint8_t * loValue, size_t loValueStride, const uint8_t * hiValue, size_t hiValueStride,
				 uint8_t * loCount, size_t loCountStride, uint8_t * hiCount, size_t hiCountStride);

			FuncPtr func;
			std::string description;

			Func2(const FuncPtr & f, const std::string & d) : func(f), description(d) {}

			void Call(const View & value, const View & loValue, const View & hiValue,
				const View & loCountSrc, const View & hiCountSrc, View & loCountDst, View & hiCountDst) const
			{
				Simd::Copy(loCountSrc, loCountDst);
				Simd::Copy(hiCountSrc, hiCountDst);
				TEST_PERFORMANCE_TEST(description);
				func(value.data, value.stride, value.width, value.height, 
					loValue.data, loValue.stride, hiValue.data, hiValue.stride,
					loCountDst.data, loCountDst.stride, hiCountDst.data, hiCountDst.stride);
			}
		};
	}

#define FUNC2(function) Func2(function, std::string(#function))

	bool BackgroundIncrementCountAutoTest(int width, int height, const Func2 & f1, const Func2 & f2)
	{
		bool result = true;

		std::cout << "Test " << f1.description << " & " << f2.description << " [" << width << ", " << height << "]." << std::endl;

		View value(width, height, View::Gray8, NULL, TEST_ALIGN(width));
		FillRandom(value);
		View loValue(width, height, View::Gray8, NULL, TEST_ALIGN(width));
		FillRandom(loValue);
		View hiValue(width, height, View::Gray8, NULL, TEST_ALIGN(width));
		FillRandom(hiValue);
		View loCountSrc(width, height, View::Gray8, NULL, TEST_ALIGN(width));
		FillRandom(loCountSrc);
		View hiCountSrc(width, height, View::Gray8, NULL, TEST_ALIGN(width));
		FillRandom(hiCountSrc);

		View loCountDst1(width, height, View::Gray8, NULL, TEST_ALIGN(width));
		View hiCountDst1(width, height, View::Gray8, NULL, TEST_ALIGN(width));
		View loCountDst2(width, height, View::Gray8, NULL, TEST_ALIGN(width));
		View hiCountDst2(width, height, View::Gray8, NULL, TEST_ALIGN(width));

		TEST_EXECUTE_AT_LEAST_MIN_TIME(f1.Call(value, loValue, hiValue, loCountSrc, hiCountSrc, loCountDst1, hiCountDst1));

		TEST_EXECUTE_AT_LEAST_MIN_TIME(f2.Call(value, loValue, hiValue, loCountSrc, hiCountSrc, loCountDst2, hiCountDst2));

		result = result && Compare(loCountDst1, loCountDst2, 0, true, 10, 0, "lo");
		result = result && Compare(hiCountDst1, hiCountDst2, 0, true, 10, 0, "hi");

		return result;
	}

    bool BackgroundIncrementCountAutoTest(const Func2 & f1, const Func2 & f2)
    {
        bool result = true;

        result = result && BackgroundIncrementCountAutoTest(W, H, f1, f2);
        result = result && BackgroundIncrementCountAutoTest(W + 3, H - 3, f1, f2);
        result = result && BackgroundIncrementCountAutoTest(W - 3, H + 3, f1, f2);

        return result;
    }

	namespace
	{
		struct Func3
		{
			typedef void (*FuncPtr)(uint8_t * loCount, size_t loCountStride, size_t width, size_t height, 
				 uint8_t * loValue, size_t loValueStride, uint8_t * hiCount, size_t hiCountStride, 
				 uint8_t * hiValue, size_t hiValueStride, uint8_t threshold);

			FuncPtr func;
			std::string description;

			Func3(const FuncPtr & f, const std::string & d) : func(f), description(d) {}

			void Call(const View & loCountSrc, const View & loValueSrc, const View & hiCountSrc, const View & hiValueSrc, 
				View & loCountDst, View & loValueDst, View & hiCountDst, View & hiValueDst, uint8_t threshold) const
			{
				Simd::Copy(loCountSrc, loCountDst);
				Simd::Copy(loValueSrc, loValueDst);
				Simd::Copy(hiCountSrc, hiCountDst);
				Simd::Copy(hiValueSrc, hiValueDst);
				TEST_PERFORMANCE_TEST(description);
				func(loCountDst.data, loCountDst.stride, loValueDst.width, loValueDst.height, loValueDst.data, loValueDst.stride, 
					hiCountDst.data, hiCountDst.stride, hiValueDst.data, hiValueDst.stride, threshold);
			}
		};
	}

#define FUNC3(function) Func3(function, std::string(#function))

	bool BackgroundAdjustRangeAutoTest(int width, int height, const Func3 & f1, const Func3 & f2)
	{
		bool result = true;

		std::cout << "Test " << f1.description << " & " << f2.description << " [" << width << ", " << height << "]." << std::endl;

		View loCountSrc(width, height, View::Gray8, NULL, TEST_ALIGN(width));
		FillRandom(loCountSrc);
		View loValueSrc(width, height, View::Gray8, NULL, TEST_ALIGN(width));
		FillRandom(loValueSrc);
		View hiCountSrc(width, height, View::Gray8, NULL, TEST_ALIGN(width));
		FillRandom(hiCountSrc);
		View hiValueSrc(width, height, View::Gray8, NULL, TEST_ALIGN(width));
		FillRandom(hiValueSrc);

		View loCountDst1(width, height, View::Gray8, NULL, TEST_ALIGN(width));
		View loValueDst1(width, height, View::Gray8, NULL, TEST_ALIGN(width));
		View hiCountDst1(width, height, View::Gray8, NULL, TEST_ALIGN(width));
		View hiValueDst1(width, height, View::Gray8, NULL, TEST_ALIGN(width));
		View loCountDst2(width, height, View::Gray8, NULL, TEST_ALIGN(width));
		View loValueDst2(width, height, View::Gray8, NULL, TEST_ALIGN(width));
		View hiCountDst2(width, height, View::Gray8, NULL, TEST_ALIGN(width));
		View hiValueDst2(width, height, View::Gray8, NULL, TEST_ALIGN(width));

		TEST_EXECUTE_AT_LEAST_MIN_TIME(f1.Call(loCountSrc, loValueSrc,hiCountSrc, hiValueSrc,  
			loCountDst1, loValueDst1, hiCountDst1, hiValueDst1, 0x80));

		TEST_EXECUTE_AT_LEAST_MIN_TIME(f2.Call(loCountSrc, loValueSrc,hiCountSrc, hiValueSrc,  
			loCountDst2, loValueDst2, hiCountDst2, hiValueDst2, 0x80));

		result = result && Compare(loCountDst1, loCountDst2, 0, true, 10, 0, "loCount");
		result = result && Compare(loValueDst1, loValueDst2, 0, true, 10, 0, "loValue");
		result = result && Compare(hiCountDst1, hiCountDst2, 0, true, 10, 0, "hiCount");
		result = result && Compare(hiValueDst1, hiValueDst2, 0, true, 10, 0, "hiValue");

		return result;
	}

    bool BackgroundAdjustRangeAutoTest(const Func3 & f1, const Func3 & f2)
    {
        bool result = true;

        result = result && BackgroundAdjustRangeAutoTest(W, H, f1, f2);
        result = result && BackgroundAdjustRangeAutoTest(W + 3, H - 3, f1, f2);
        result = result && BackgroundAdjustRangeAutoTest(W - 3, H + 3, f1, f2);

        return result;
    }

	namespace
	{
		struct Func4
		{
			typedef void (*FuncPtr)(uint8_t * loCount, size_t loCountStride, size_t width, size_t height, 
				 uint8_t * loValue, size_t loValueStride, uint8_t * hiCount, size_t hiCountStride, 
				 uint8_t * hiValue, size_t hiValueStride, uint8_t threshold, const uint8_t * mask, size_t maskStride);

			FuncPtr func;
			std::string description;

			Func4(const FuncPtr & f, const std::string & d) : func(f), description(d) {}

			void Call(const View & loCountSrc, const View & loValueSrc, const View & hiCountSrc, const View & hiValueSrc, 
				View & loCountDst, View & loValueDst, View & hiCountDst, View & hiValueDst, uint8_t threshold, const View & mask) const
			{
				Simd::Copy(loCountSrc, loCountDst);
				Simd::Copy(loValueSrc, loValueDst);
				Simd::Copy(hiCountSrc, hiCountDst);
				Simd::Copy(hiValueSrc, hiValueDst);
				TEST_PERFORMANCE_TEST(description);
				func(loCountDst.data, loCountDst.stride, loValueDst.width, loValueDst.height, loValueDst.data, loValueDst.stride, 
					hiCountDst.data, hiCountDst.stride, hiValueDst.data, hiValueDst.stride, threshold, mask.data, mask.stride);
			}
		};
	}

#define FUNC4(function) Func4(function, std::string(#function))

	bool BackgroundAdjustRangeMaskedAutoTest(int width, int height, const Func4 & f1, const Func4 & f2)
	{
		bool result = true;

		std::cout << "Test " << f1.description << " & " << f2.description << " [" << width << ", " << height << "]." << std::endl;

		View loCountSrc(width, height, View::Gray8, NULL, TEST_ALIGN(width));
		FillRandom(loCountSrc);
		View loValueSrc(width, height, View::Gray8, NULL, TEST_ALIGN(width));
		FillRandom(loValueSrc);
		View hiCountSrc(width, height, View::Gray8, NULL, TEST_ALIGN(width));
		FillRandom(hiCountSrc);
		View hiValueSrc(width, height, View::Gray8, NULL, TEST_ALIGN(width));
		FillRandom(hiValueSrc);
		View mask(width, height, View::Gray8, NULL, TEST_ALIGN(width));
		FillRandomMask(mask, 0xFF);

		View loCountDst1(width, height, View::Gray8, NULL, TEST_ALIGN(width));
		View loValueDst1(width, height, View::Gray8, NULL, TEST_ALIGN(width));
		View hiCountDst1(width, height, View::Gray8, NULL, TEST_ALIGN(width));
		View hiValueDst1(width, height, View::Gray8, NULL, TEST_ALIGN(width));
		View loCountDst2(width, height, View::Gray8, NULL, TEST_ALIGN(width));
		View loValueDst2(width, height, View::Gray8, NULL, TEST_ALIGN(width));
		View hiCountDst2(width, height, View::Gray8, NULL, TEST_ALIGN(width));
		View hiValueDst2(width, height, View::Gray8, NULL, TEST_ALIGN(width));

		TEST_EXECUTE_AT_LEAST_MIN_TIME(f1.Call(loCountSrc, loValueSrc,hiCountSrc, hiValueSrc,  
			loCountDst1, loValueDst1, hiCountDst1, hiValueDst1, 0x80, mask));

		TEST_EXECUTE_AT_LEAST_MIN_TIME(f2.Call(loCountSrc, loValueSrc,hiCountSrc, hiValueSrc,  
			loCountDst2, loValueDst2, hiCountDst2, hiValueDst2, 0x80, mask));

		result = result && Compare(loCountDst1, loCountDst2, 0, true, 10, 0, "loCount");
		result = result && Compare(loValueDst1, loValueDst2, 0, true, 10, 0, "loValue");
		result = result && Compare(hiCountDst1, hiCountDst2, 0, true, 10, 0, "hiCount");
		result = result && Compare(hiValueDst1, hiValueDst2, 0, true, 10, 0, "hiValue");

		return result;
	}

    bool BackgroundAdjustRangeMaskedAutoTest(const Func4 & f1, const Func4 & f2)
    {
        bool result = true;

        result = result && BackgroundAdjustRangeMaskedAutoTest(W, H, f1, f2);
        result = result && BackgroundAdjustRangeMaskedAutoTest(W + 3, H - 3, f1, f2);
        result = result && BackgroundAdjustRangeMaskedAutoTest(W - 3, H + 3, f1, f2);

        return result;
    }

	namespace
	{
		struct Func5
		{
			typedef void (*FuncPtr)(const uint8_t * value, size_t valueStride, size_t width, size_t height,
				 uint8_t * lo, size_t loStride, uint8_t * hi, size_t hiStride, const uint8_t * mask, size_t maskStride);

			FuncPtr func;
			std::string description;

			Func5(const FuncPtr & f, const std::string & d) : func(f), description(d) {}

			void Call(const View & value, const View & loSrc, const View & hiSrc, View & loDst, View & hiDst, const View & mask) const
			{
				Simd::Copy(loSrc, loDst);
				Simd::Copy(hiSrc, hiDst);
				TEST_PERFORMANCE_TEST(description);
				func(value.data, value.stride, value.width, value.height, loDst.data, loDst.stride, hiDst.data, hiDst.stride,
					mask.data, mask.stride);
			}
		};
	}

#define FUNC5(function) Func5(function, std::string(#function))

	bool BackgroundShiftRangeMaskedAutoTest(int width, int height, const Func5 & f1, const Func5 & f2)
	{
		bool result = true;

		std::cout << "Test " << f1.description << " & " << f2.description << " [" << width << ", " << height << "]." << std::endl;

		View value(width, height, View::Gray8, NULL, TEST_ALIGN(width));
		FillRandom(value);
		View loSrc(width, height, View::Gray8, NULL, TEST_ALIGN(width));
		FillRandom(loSrc);
		View hiSrc(width, height, View::Gray8, NULL, TEST_ALIGN(width));
		FillRandom(hiSrc);
		View mask(width, height, View::Gray8, NULL, TEST_ALIGN(width));
		FillRandomMask(mask, 0xFF);

		View loDst1(width, height, View::Gray8, NULL, TEST_ALIGN(width));
		View hiDst1(width, height, View::Gray8, NULL, TEST_ALIGN(width));
		View loDst2(width, height, View::Gray8, NULL, TEST_ALIGN(width));
		View hiDst2(width, height, View::Gray8, NULL, TEST_ALIGN(width));

		TEST_EXECUTE_AT_LEAST_MIN_TIME(f1.Call(value, loSrc, hiSrc, loDst1, hiDst1, mask));

		TEST_EXECUTE_AT_LEAST_MIN_TIME(f2.Call(value, loSrc, hiSrc, loDst2, hiDst2, mask));

		result = result && Compare(loDst1, loDst2, 0, true, 10, 0, "lo");
		result = result && Compare(hiDst1, hiDst2, 0, true, 10, 0, "hi");

		return result;
	}

	namespace
	{
		struct Func6
		{
			typedef void (*FuncPtr)(const uint8_t * src, size_t srcStride, size_t width, size_t height,
				 uint8_t index, uint8_t value, uint8_t * dst, size_t dstStride);

			FuncPtr func;
			std::string description;

			Func6(const FuncPtr & f, const std::string & d) : func(f), description(d) {}

			void Call(const View & src, uint8_t index, uint8_t value, View & dst) const
			{
				TEST_PERFORMANCE_TEST(description);
				func(src.data, src.stride, src.width, src.height, index, value, dst.data, dst.stride);
			}
		};
	}

#define FUNC6(function) Func6(function, std::string(#function))

	bool BackgroundInitMaskAutoTest(int width, int height, const Func6 & f1, const Func6 & f2)
	{
		bool result = true;

		std::cout << "Test " << f1.description << " & " << f2.description << " [" << width << ", " << height << "]." << std::endl;

		 uint8_t index = 1 + Random(255);
		View src(width, height, View::Gray8, NULL, TEST_ALIGN(width));
		FillRandomMask(src, index);

		View dst1(width, height, View::Gray8, NULL, TEST_ALIGN(width));
		memset(dst1.data, 0, dst1.stride*height);
		View dst2(width, height, View::Gray8, NULL, TEST_ALIGN(width));
		memset(dst2.data, 0, dst2.stride*height);

		TEST_EXECUTE_AT_LEAST_MIN_TIME(f1.Call(src, index, 0xFF, dst1));

		TEST_EXECUTE_AT_LEAST_MIN_TIME(f2.Call(src, index, 0xFF, dst2));

		result = result && Compare(dst1, dst2, 0, true, 10);

		return result;
	}

	bool BackgroundGrowRangeSlowAutoTest()
	{
		bool result = true;

		result = result && BackgroundChangeRangeAutoTest(FUNC1(Simd::Base::BackgroundGrowRangeSlow), FUNC1(SimdBackgroundGrowRangeSlow));

#ifdef SIMD_SSE2_ENABLE
        if(Simd::Sse2::Enable)
            result = result && BackgroundChangeRangeAutoTest(FUNC1(Simd::Sse2::BackgroundGrowRangeSlow), FUNC1(SimdBackgroundGrowRangeSlow));
#endif 

#ifdef SIMD_AVX2_ENABLE
        if(Simd::Avx2::Enable)
            result = result && BackgroundChangeRangeAutoTest(FUNC1(Simd::Avx2::BackgroundGrowRangeSlow), FUNC1(SimdBackgroundGrowRangeSlow));
#endif 

#ifdef SIMD_VSX_ENABLE
        if(Simd::Vsx::Enable)
            result = result && BackgroundChangeRangeAutoTest(FUNC1(Simd::Vsx::BackgroundGrowRangeSlow), FUNC1(SimdBackgroundGrowRangeSlow));
#endif 

		return result;
	}

	bool BackgroundGrowRangeFastAutoTest()
	{
        bool result = true;

        result = result && BackgroundChangeRangeAutoTest(FUNC1(Simd::Base::BackgroundGrowRangeFast), FUNC1(SimdBackgroundGrowRangeFast));

#ifdef SIMD_SSE2_ENABLE
        if(Simd::Sse2::Enable)
            result = result && BackgroundChangeRangeAutoTest(FUNC1(Simd::Sse2::BackgroundGrowRangeFast), FUNC1(SimdBackgroundGrowRangeFast));
#endif 

#ifdef SIMD_AVX2_ENABLE
        if(Simd::Avx2::Enable)
            result = result && BackgroundChangeRangeAutoTest(FUNC1(Simd::Avx2::BackgroundGrowRangeFast), FUNC1(SimdBackgroundGrowRangeFast));
#endif 

#ifdef SIMD_VSX_ENABLE
        if(Simd::Vsx::Enable)
            result = result && BackgroundChangeRangeAutoTest(FUNC1(Simd::Vsx::BackgroundGrowRangeFast), FUNC1(SimdBackgroundGrowRangeFast));
#endif 

        return result;
	}

	bool BackgroundIncrementCountAutoTest()
	{
		bool result = true;

		result = result && BackgroundIncrementCountAutoTest(FUNC2(Simd::Base::BackgroundIncrementCount), FUNC2(SimdBackgroundIncrementCount));

#ifdef SIMD_SSE2_ENABLE
        if(Simd::Sse2::Enable)
            result = result && BackgroundIncrementCountAutoTest(FUNC2(Simd::Sse2::BackgroundIncrementCount), FUNC2(SimdBackgroundIncrementCount));
#endif 

#ifdef SIMD_AVX2_ENABLE
        if(Simd::Avx2::Enable)
            result = result && BackgroundIncrementCountAutoTest(FUNC2(Simd::Avx2::BackgroundIncrementCount), FUNC2(SimdBackgroundIncrementCount));
#endif 

#ifdef SIMD_VSX_ENABLE
        if(Simd::Vsx::Enable)
            result = result && BackgroundIncrementCountAutoTest(FUNC2(Simd::Vsx::BackgroundIncrementCount), FUNC2(SimdBackgroundIncrementCount));
#endif 

		return result;
	}

	bool BackgroundAdjustRangeAutoTest()
	{
		bool result = true;

		result = result && BackgroundAdjustRangeAutoTest(FUNC3(Simd::Base::BackgroundAdjustRange), FUNC3(SimdBackgroundAdjustRange));

#ifdef SIMD_SSE2_ENABLE
        if(Simd::Sse2::Enable)
            result = result && BackgroundAdjustRangeAutoTest(FUNC3(Simd::Sse2::BackgroundAdjustRange), FUNC3(SimdBackgroundAdjustRange));
#endif 

#ifdef SIMD_AVX2_ENABLE
        if(Simd::Avx2::Enable)
            result = result && BackgroundAdjustRangeAutoTest(FUNC3(Simd::Avx2::BackgroundAdjustRange), FUNC3(SimdBackgroundAdjustRange));
#endif 

#ifdef SIMD_VSX_ENABLE
        if(Simd::Vsx::Enable)
            result = result && BackgroundAdjustRangeAutoTest(FUNC3(Simd::Vsx::BackgroundAdjustRange), FUNC3(SimdBackgroundAdjustRange));
#endif 

		return result;
	}

    bool BackgroundAdjustRangeMaskedAutoTest()
    {
        bool result = true;

        result = result && BackgroundAdjustRangeMaskedAutoTest(FUNC4(Simd::Base::BackgroundAdjustRangeMasked), FUNC4(SimdBackgroundAdjustRangeMasked));

#ifdef SIMD_SSE2_ENABLE
        if(Simd::Sse2::Enable)
            result = result && BackgroundAdjustRangeMaskedAutoTest(FUNC4(Simd::Sse2::BackgroundAdjustRangeMasked), FUNC4(SimdBackgroundAdjustRangeMasked));
#endif 

#ifdef SIMD_AVX2_ENABLE
        if(Simd::Avx2::Enable)
            result = result && BackgroundAdjustRangeMaskedAutoTest(FUNC4(Simd::Avx2::BackgroundAdjustRangeMasked), FUNC4(SimdBackgroundAdjustRangeMasked));
#endif 

#ifdef SIMD_VSX_ENABLE
        if(Simd::Vsx::Enable)
            result = result && BackgroundAdjustRangeMaskedAutoTest(FUNC4(Simd::Vsx::BackgroundAdjustRangeMasked), FUNC4(SimdBackgroundAdjustRangeMasked));
#endif 

        return result;
    }

	bool BackgroundShiftRangeAutoTest()
	{
		bool result = true;

		result = result && BackgroundChangeRangeAutoTest(FUNC1(Simd::Base::BackgroundShiftRange), FUNC1(SimdBackgroundShiftRange));

#ifdef SIMD_SSE2_ENABLE
        if(Simd::Sse2::Enable)
            result = result && BackgroundChangeRangeAutoTest(FUNC1(Simd::Sse2::BackgroundShiftRange), FUNC1(SimdBackgroundShiftRange));
#endif 

#ifdef SIMD_AVX2_ENABLE
        if(Simd::Avx2::Enable)
            result = result && BackgroundChangeRangeAutoTest(FUNC1(Simd::Avx2::BackgroundShiftRange), FUNC1(SimdBackgroundShiftRange));
#endif 

#ifdef SIMD_VSX_ENABLE
        if(Simd::Vsx::Enable)
            result = result && BackgroundChangeRangeAutoTest(FUNC1(Simd::Vsx::BackgroundShiftRange), FUNC1(SimdBackgroundShiftRange));
#endif 

		return result;
	}

	bool BackgroundShiftRangeMaskedAutoTest()
	{
		bool result = true;

		result = result && BackgroundShiftRangeMaskedAutoTest(W, H, FUNC5(Simd::Base::BackgroundShiftRangeMasked), FUNC5(SimdBackgroundShiftRangeMasked));
		result = result && BackgroundShiftRangeMaskedAutoTest(W + 1, H - 1, FUNC5(Simd::Base::BackgroundShiftRangeMasked), FUNC5(SimdBackgroundShiftRangeMasked));
        result = result && BackgroundShiftRangeMaskedAutoTest(W - 1, H + 1, FUNC5(Simd::Base::BackgroundShiftRangeMasked), FUNC5(SimdBackgroundShiftRangeMasked));

#if defined(SIMD_SSE2_ENABLE) && defined(SIMD_AVX2_ENABLE)
        if(Simd::Sse2::Enable && Simd::Avx2::Enable)
        {
            result = result && BackgroundShiftRangeMaskedAutoTest(W, H, FUNC5(Simd::Sse2::BackgroundShiftRangeMasked), FUNC5(Simd::Avx2::BackgroundShiftRangeMasked));
            result = result && BackgroundShiftRangeMaskedAutoTest(W + 1, H - 1, FUNC5(Simd::Sse2::BackgroundShiftRangeMasked), FUNC5(Simd::Avx2::BackgroundShiftRangeMasked));
            result = result && BackgroundShiftRangeMaskedAutoTest(W - 1, H + 1, FUNC5(Simd::Sse2::BackgroundShiftRangeMasked), FUNC5(Simd::Avx2::BackgroundShiftRangeMasked));
        }
#endif 

		return result;
	}

	bool BackgroundInitMaskAutoTest()
	{
		bool result = true;

		result = result && BackgroundInitMaskAutoTest(W, H, FUNC6(Simd::Base::BackgroundInitMask), FUNC6(SimdBackgroundInitMask));
		result = result && BackgroundInitMaskAutoTest(W + 1, H - 1, FUNC6(Simd::Base::BackgroundInitMask), FUNC6(SimdBackgroundInitMask));
        result = result && BackgroundInitMaskAutoTest(W - 1, H + 1, FUNC6(Simd::Base::BackgroundInitMask), FUNC6(SimdBackgroundInitMask));

#if defined(SIMD_SSE2_ENABLE) && defined(SIMD_AVX2_ENABLE)
        if(Simd::Sse2::Enable && Simd::Avx2::Enable)
        {
            result = result && BackgroundInitMaskAutoTest(W, H, FUNC6(Simd::Sse2::BackgroundInitMask), FUNC6(Simd::Avx2::BackgroundInitMask));
            result = result && BackgroundInitMaskAutoTest(W + 1, H - 1, FUNC6(Simd::Sse2::BackgroundInitMask), FUNC6(Simd::Avx2::BackgroundInitMask));
            result = result && BackgroundInitMaskAutoTest(W - 1, H + 1, FUNC6(Simd::Sse2::BackgroundInitMask), FUNC6(Simd::Avx2::BackgroundInitMask));
        }
#endif 

		return result;
	}

    //-----------------------------------------------------------------------

    bool BackgroundChangeRangeDataTest(bool create, int width, int height, const Func1 & f)
    {
        bool result = true;

        Data data(f.description);

        std::cout << (create ? "Create" : "Verify") << " test " << f.description << " [" << width << ", " << height << "]." << std::endl;

        View value(width, height, View::Gray8, NULL, TEST_ALIGN(width));
        View loSrc(width, height, View::Gray8, NULL, TEST_ALIGN(width));
        View hiSrc(width, height, View::Gray8, NULL, TEST_ALIGN(width));

        View loDst1(width, height, View::Gray8, NULL, TEST_ALIGN(width));
        View hiDst1(width, height, View::Gray8, NULL, TEST_ALIGN(width));
        View loDst2(width, height, View::Gray8, NULL, TEST_ALIGN(width));
        View hiDst2(width, height, View::Gray8, NULL, TEST_ALIGN(width));

        if(create)
        {
            FillRandom(value);
            FillRandom(loSrc);
            FillRandom(hiSrc);

            TEST_SAVE(value);
            TEST_SAVE(loSrc);
            TEST_SAVE(hiSrc);

            f.Call(value, loSrc, hiSrc, loDst1, hiDst1);

            TEST_SAVE(loDst1);
            TEST_SAVE(hiDst1);
        }
        else
        {
            TEST_LOAD(value);
            TEST_LOAD(loSrc);
            TEST_LOAD(hiSrc);

            TEST_LOAD(loDst1);
            TEST_LOAD(hiDst1);

            f.Call(value, loSrc, hiSrc, loDst2, hiDst2);

            TEST_SAVE(loDst2);
            TEST_SAVE(hiDst2);

            result = result && Compare(loDst1, loDst2, 0, true, 10, 0, "lo");
            result = result && Compare(hiDst1, hiDst2, 0, true, 10, 0, "hi");
        }

        return result;
    }

    bool BackgroundGrowRangeSlowDataTest(bool create)
    {
        bool result = true;

        result = result && BackgroundChangeRangeDataTest(create, DW, DH, FUNC1(SimdBackgroundGrowRangeSlow));

        return result;
    }

    bool BackgroundGrowRangeFastDataTest(bool create)
    {
        bool result = true;

        result = result && BackgroundChangeRangeDataTest(create, DW, DH, FUNC1(SimdBackgroundGrowRangeFast));

        return result;
    }

    bool BackgroundIncrementCountDataTest(bool create, int width, int height, const Func2 & f)
    {
        bool result = true;

        Data data(f.description);

        std::cout << (create ? "Create" : "Verify") << " test " << f.description << " [" << width << ", " << height << "]." << std::endl;

        View value(width, height, View::Gray8, NULL, TEST_ALIGN(width));
        View loValue(width, height, View::Gray8, NULL, TEST_ALIGN(width));
        View hiValue(width, height, View::Gray8, NULL, TEST_ALIGN(width));
        View loCountSrc(width, height, View::Gray8, NULL, TEST_ALIGN(width));
        View hiCountSrc(width, height, View::Gray8, NULL, TEST_ALIGN(width));

        View loCountDst1(width, height, View::Gray8, NULL, TEST_ALIGN(width));
        View hiCountDst1(width, height, View::Gray8, NULL, TEST_ALIGN(width));
        View loCountDst2(width, height, View::Gray8, NULL, TEST_ALIGN(width));
        View hiCountDst2(width, height, View::Gray8, NULL, TEST_ALIGN(width));

        if(create)
        {
            FillRandom(value);
            FillRandom(loValue);
            FillRandom(hiValue);
            FillRandom(loCountSrc);
            FillRandom(hiCountSrc);

            TEST_SAVE(value);
            TEST_SAVE(loValue);
            TEST_SAVE(hiValue);
            TEST_SAVE(loCountSrc);
            TEST_SAVE(hiCountSrc);

            f.Call(value, loValue, hiValue, loCountSrc, hiCountSrc, loCountDst1, hiCountDst1);

            TEST_SAVE(loCountDst1);
            TEST_SAVE(hiCountDst1);
        }
        else
        {
            TEST_LOAD(value);
            TEST_LOAD(loValue);
            TEST_LOAD(hiValue);
            TEST_LOAD(loCountSrc);
            TEST_LOAD(hiCountSrc);

            TEST_LOAD(loCountDst1);
            TEST_LOAD(hiCountDst1);

            f.Call(value, loValue, hiValue, loCountSrc, hiCountSrc, loCountDst2, hiCountDst2);

            TEST_SAVE(loCountDst2);
            TEST_SAVE(hiCountDst2);

            result = result && Compare(loCountDst1, loCountDst2, 0, true, 10, 0, "lo");
            result = result && Compare(hiCountDst1, hiCountDst2, 0, true, 10, 0, "hi");
        }

        return result;
    }

    bool BackgroundIncrementCountDataTest(bool create)
    {
        bool result = true;

        result = result && BackgroundIncrementCountDataTest(create, DW, DH, FUNC2(SimdBackgroundIncrementCount));

        return result;
    }

    bool BackgroundAdjustRangeDataTest(bool create, int width, int height, const Func3 & f)
    {
        bool result = true;

        Data data(f.description);

        std::cout << (create ? "Create" : "Verify") << " test " << f.description << " [" << width << ", " << height << "]." << std::endl;

        View loCountSrc(width, height, View::Gray8, NULL, TEST_ALIGN(width));
        View loValueSrc(width, height, View::Gray8, NULL, TEST_ALIGN(width));
        View hiCountSrc(width, height, View::Gray8, NULL, TEST_ALIGN(width));
        View hiValueSrc(width, height, View::Gray8, NULL, TEST_ALIGN(width));

        View loCountDst1(width, height, View::Gray8, NULL, TEST_ALIGN(width));
        View loValueDst1(width, height, View::Gray8, NULL, TEST_ALIGN(width));
        View hiCountDst1(width, height, View::Gray8, NULL, TEST_ALIGN(width));
        View hiValueDst1(width, height, View::Gray8, NULL, TEST_ALIGN(width));
        View loCountDst2(width, height, View::Gray8, NULL, TEST_ALIGN(width));
        View loValueDst2(width, height, View::Gray8, NULL, TEST_ALIGN(width));
        View hiCountDst2(width, height, View::Gray8, NULL, TEST_ALIGN(width));
        View hiValueDst2(width, height, View::Gray8, NULL, TEST_ALIGN(width));

        if(create)
        {
            FillRandom(loCountSrc);
            FillRandom(loValueSrc);
            FillRandom(hiCountSrc);
            FillRandom(hiValueSrc);

            TEST_SAVE(loCountSrc);
            TEST_SAVE(loValueSrc);
            TEST_SAVE(hiCountSrc);
            TEST_SAVE(hiValueSrc);

            f.Call(loCountSrc, loValueSrc,hiCountSrc, hiValueSrc, loCountDst1, loValueDst1, hiCountDst1, hiValueDst1, 0x80);

            TEST_SAVE(loCountDst1);
            TEST_SAVE(loValueDst1);
            TEST_SAVE(hiCountDst1);
            TEST_SAVE(hiValueDst1);
        }
        else
        {
            TEST_LOAD(loCountSrc);
            TEST_LOAD(loValueSrc);
            TEST_LOAD(hiCountSrc);
            TEST_LOAD(hiValueSrc);

            TEST_LOAD(loCountDst1);
            TEST_LOAD(loValueDst1);
            TEST_LOAD(hiCountDst1);
            TEST_LOAD(hiValueDst1);

            f.Call(loCountSrc, loValueSrc,hiCountSrc, hiValueSrc, loCountDst2, loValueDst2, hiCountDst2, hiValueDst2, 0x80);

            TEST_SAVE(loCountDst2);
            TEST_SAVE(loValueDst2);
            TEST_SAVE(hiCountDst2);
            TEST_SAVE(hiValueDst2);

            result = result && Compare(loCountDst1, loCountDst2, 0, true, 10, 0, "loCount");
            result = result && Compare(loValueDst1, loValueDst2, 0, true, 10, 0, "loValue");
            result = result && Compare(hiCountDst1, hiCountDst2, 0, true, 10, 0, "hiCount");
            result = result && Compare(hiValueDst1, hiValueDst2, 0, true, 10, 0, "hiValue");
        }

        return result;
    }

    bool BackgroundAdjustRangeDataTest(bool create)
    {
        bool result = true;

        result = result && BackgroundAdjustRangeDataTest(create, DW, DH, FUNC3(SimdBackgroundAdjustRange));

        return result;
    }

    bool BackgroundAdjustRangeMaskedDataTest(bool create, int width, int height, const Func4 & f)
    {
        bool result = true;

        Data data(f.description);

        std::cout << (create ? "Create" : "Verify") << " test " << f.description << " [" << width << ", " << height << "]." << std::endl;

        View loCountSrc(width, height, View::Gray8, NULL, TEST_ALIGN(width));
        View loValueSrc(width, height, View::Gray8, NULL, TEST_ALIGN(width));
        View hiCountSrc(width, height, View::Gray8, NULL, TEST_ALIGN(width));
        View hiValueSrc(width, height, View::Gray8, NULL, TEST_ALIGN(width));
        View mask(width, height, View::Gray8, NULL, TEST_ALIGN(width));


        View loCountDst1(width, height, View::Gray8, NULL, TEST_ALIGN(width));
        View loValueDst1(width, height, View::Gray8, NULL, TEST_ALIGN(width));
        View hiCountDst1(width, height, View::Gray8, NULL, TEST_ALIGN(width));
        View hiValueDst1(width, height, View::Gray8, NULL, TEST_ALIGN(width));
        View loCountDst2(width, height, View::Gray8, NULL, TEST_ALIGN(width));
        View loValueDst2(width, height, View::Gray8, NULL, TEST_ALIGN(width));
        View hiCountDst2(width, height, View::Gray8, NULL, TEST_ALIGN(width));
        View hiValueDst2(width, height, View::Gray8, NULL, TEST_ALIGN(width));

        if(create)
        {
            FillRandom(loCountSrc);
            FillRandom(loValueSrc);
            FillRandom(hiCountSrc);
            FillRandom(hiValueSrc);
            FillRandomMask(mask, 0xFF);

            TEST_SAVE(loCountSrc);
            TEST_SAVE(loValueSrc);
            TEST_SAVE(hiCountSrc);
            TEST_SAVE(hiValueSrc);
            TEST_SAVE(mask);

            f.Call(loCountSrc, loValueSrc,hiCountSrc, hiValueSrc, loCountDst1, loValueDst1, hiCountDst1, hiValueDst1, 0x80, mask);

            TEST_SAVE(loCountDst1);
            TEST_SAVE(loValueDst1);
            TEST_SAVE(hiCountDst1);
            TEST_SAVE(hiValueDst1);
        }
        else
        {
            TEST_LOAD(loCountSrc);
            TEST_LOAD(loValueSrc);
            TEST_LOAD(hiCountSrc);
            TEST_LOAD(hiValueSrc);
            TEST_LOAD(mask);

            TEST_LOAD(loCountDst1);
            TEST_LOAD(loValueDst1);
            TEST_LOAD(hiCountDst1);
            TEST_LOAD(hiValueDst1);

            f.Call(loCountSrc, loValueSrc,hiCountSrc, hiValueSrc, loCountDst2, loValueDst2, hiCountDst2, hiValueDst2, 0x80, mask);

            TEST_SAVE(loCountDst2);
            TEST_SAVE(loValueDst2);
            TEST_SAVE(hiCountDst2);
            TEST_SAVE(hiValueDst2);

            result = result && Compare(loCountDst1, loCountDst2, 0, true, 10, 0, "loCount");
            result = result && Compare(loValueDst1, loValueDst2, 0, true, 10, 0, "loValue");
            result = result && Compare(hiCountDst1, hiCountDst2, 0, true, 10, 0, "hiCount");
            result = result && Compare(hiValueDst1, hiValueDst2, 0, true, 10, 0, "hiValue");
        }

        return result;
    }

    bool BackgroundAdjustRangeMaskedDataTest(bool create)
    {
        bool result = true;

        result = result && BackgroundAdjustRangeMaskedDataTest(create, DW, DH, FUNC4(SimdBackgroundAdjustRangeMasked));

        return result;
    }

    bool BackgroundShiftRangeDataTest(bool create)
    {
        bool result = true;

        result = result && BackgroundChangeRangeDataTest(create, DW, DH, FUNC1(SimdBackgroundShiftRange));

        return result;
    }
}
