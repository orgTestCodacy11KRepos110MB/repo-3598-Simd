/*
* Simd Library Tests.
*
* Copyright (c) 2011-2013 Yermalayeu Ihar.
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

namespace Test
{
	namespace
	{
		struct Func
		{
			typedef void (*FuncPtr)(const uchar * src, size_t srcStride, size_t width, size_t height, size_t channelCount, 
				const uchar * bkg, size_t bkgStride, double shiftX, double shiftY, 
				size_t cropLeft, size_t cropTop, size_t cropRight, size_t cropBottom, uchar * dst, size_t dstStride);

			FuncPtr func;
			std::string description;

			Func(const FuncPtr & f, const std::string & d) : func(f), description(d) {}

			void Call(const View & src, const View & bkg, double shiftX, double shiftY, 
				size_t cropLeft, size_t cropTop, size_t cropRight, size_t cropBottom, View & dst) const
			{
				TEST_PERFORMANCE_TEST(description);
				func(src.data, src.stride, src.width, src.height, View::SizeOf(src.format), bkg.data, bkg.stride,
					shiftX, shiftY, cropLeft, cropTop, cropRight, cropBottom, dst.data, dst.stride);
			}
		};
	}

#define ARGS(format, width, height, function1, function2) \
	format, width, height, \
	Func(function1, std::string(#function1) + ColorDescription(format)), \
	Func(function2, std::string(#function2) + ColorDescription(format))

	bool ShiftTest(View::Format format, int width, int height, double dx, double dy, int crop, const Func & f1, const Func & f2)
	{
		bool result = true;

		std::cout << std::setprecision(1) << std::fixed 
			<< "Test " << f1.description << " & " << f2.description << " [" << width << ", " << height << "],"
			<< " (" << dx << ", " << dy << ", " << crop << ")." << std::endl;

		View s(width, height, format, NULL, TEST_ALIGN(width));
		FillRandom(s);
		View b(width, height, format, NULL, TEST_ALIGN(width));
		FillRandom(b);

		View d1(width, height, format, NULL, TEST_ALIGN(width));
		View d2(width, height, format, NULL, TEST_ALIGN(width));

		TEST_EXECUTE_AT_LEAST_MIN_TIME(f1.Call(s, b, dx, dy, crop, crop, width - crop, height - crop, d1));

		TEST_EXECUTE_AT_LEAST_MIN_TIME(f2.Call(s, b, dx, dy, crop, crop, width - crop, height - crop, d2));

		result = result && Compare(d1, d2, 0, true, 10);

		return result;
	}

	bool ShiftTest(View::Format format, int width, int height, const Func & f1, const Func & f2)
	{
		bool result = true;

		const double x0 = 7.1, dx = -5.3, y0 = -5.2, dy = 3.7;
		for(int i = 0; i < 4; ++i)
			result = result && ShiftTest(format, width, height, x0 + i*dx, y0 + i*dy, i*3, f1, f2);

		return result;
	}

	bool ShiftBilinearTest()
	{
		bool result = true;

		result = result && ShiftTest(ARGS(View::Gray8, W, H, Simd::Base::ShiftBilinear, Simd::ShiftBilinear));
		result = result && ShiftTest(ARGS(View::Gray8, W + 2, H - 1, Simd::Base::ShiftBilinear, Simd::ShiftBilinear));

		result = result && ShiftTest(ARGS(View::Uv16, W, H, Simd::Base::ShiftBilinear, Simd::ShiftBilinear));
		result = result && ShiftTest(ARGS(View::Uv16, W + 2, H - 1, Simd::Base::ShiftBilinear, Simd::ShiftBilinear));

		result = result && ShiftTest(ARGS(View::Bgr24, W, H, Simd::Base::ShiftBilinear, Simd::ShiftBilinear));
		result = result && ShiftTest(ARGS(View::Bgr24, W + 2, H - 1, Simd::Base::ShiftBilinear, Simd::ShiftBilinear));

		result = result && ShiftTest(ARGS(View::Bgra32, W, H, Simd::Base::ShiftBilinear, Simd::ShiftBilinear));
		result = result && ShiftTest(ARGS(View::Bgra32, W + 2, H - 1, Simd::Base::ShiftBilinear, Simd::ShiftBilinear));

		return result;
	}
}
