/*
* Simd Library.
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
#include "Simd/SimdEnable.h"
#include "Simd/SimdMemory.h"
#include "Simd/SimdInit.h"
#include "Simd/SimdYuvToBgr.h"
#include "Simd/SimdYuvToBgra.h"

namespace Simd
{
#ifdef SIMD_AVX2_ENABLE    
    namespace Avx2
    {
		template <bool align> SIMD_INLINE void AdjustedYuv16ToBgra(__m256i y16, __m256i u16, __m256i v16, 
			const __m256i & a_0, __m256i * bgra)
		{
			const __m256i b16 = AdjustedYuvToBlue16(y16, u16);
			const __m256i g16 = AdjustedYuvToGreen16(y16, u16, v16);
			const __m256i r16 = AdjustedYuvToRed16(y16, v16);
			const __m256i bg8 = _mm256_or_si256(b16, _mm256_slli_si256(g16, 1));
			const __m256i ra8 = _mm256_or_si256(r16, a_0);
			Store<align>(bgra + 0, _mm256_unpacklo_epi16(bg8, ra8));
			Store<align>(bgra + 1, _mm256_unpackhi_epi16(bg8, ra8));
		}

		template <bool align> SIMD_INLINE void Yuv16ToBgra(__m256i y16, __m256i u16, __m256i v16, 
			const __m256i & a_0, __m256i * bgra)
		{
			AdjustedYuv16ToBgra<align>(AdjustY16(y16), AdjustUV16(u16), AdjustUV16(v16), a_0, bgra);
		}

		template <bool align> SIMD_INLINE void Yuv8ToBgra(__m256i y8, __m256i u8, __m256i v8, const __m256i & a_0, __m256i * bgra)
		{
			Yuv16ToBgra<align>(_mm256_unpacklo_epi8(y8, K_ZERO), _mm256_unpacklo_epi8(u8, K_ZERO), 
				_mm256_unpacklo_epi8(v8, K_ZERO), a_0, bgra + 0);
			Yuv16ToBgra<align>(_mm256_unpackhi_epi8(y8, K_ZERO), _mm256_unpackhi_epi8(u8, K_ZERO), 
				_mm256_unpackhi_epi8(v8, K_ZERO), a_0, bgra + 2);
		}

		template <bool align> SIMD_INLINE void Yuv444ToBgra(const uchar * y, const uchar * u, 
			const uchar * v, const __m256i & a_0, uchar * bgra)
		{
			Yuv8ToBgra<align>(LoadPermuted<align>((__m256i*)y), LoadPermuted<align>((__m256i*)u), LoadPermuted<align>((__m256i*)v), a_0, (__m256i*)bgra);
		}

		template <bool align> void Yuv444ToBgra(const uchar * y, size_t yStride, const uchar * u, size_t uStride, const uchar * v, size_t vStride, 
			size_t width, size_t height, uchar * bgra, size_t bgraStride, uchar alpha)
		{
			assert(width >= A);
			if(align)
			{
				assert(Aligned(y) && Aligned(yStride) && Aligned(u) &&  Aligned(uStride));
				assert(Aligned(v) && Aligned(vStride) && Aligned(bgra) && Aligned(bgraStride));
			}

			__m256i a_0 = _mm256_slli_si256(_mm256_set1_epi16(alpha), 1);
			size_t bodyWidth = AlignLo(width, A);
			size_t tail = width - bodyWidth;
			for(size_t row = 0; row < height; ++row)
			{
				for(size_t colYuv = 0, colBgra = 0; colYuv < bodyWidth; colYuv += A, colBgra += QA)
				{
					Yuv444ToBgra<align>(y + colYuv, u + colYuv, v + colYuv, a_0, bgra + colBgra);
				}
				if(tail)
				{
					size_t col = width - A;
					Yuv444ToBgra<false>(y + col, u + col, v + col, a_0, bgra + 4*col);
				}
				y += yStride;
				u += uStride;
				v += vStride;
				bgra += bgraStride;
			}
		}

		template <bool align> SIMD_INLINE void Yuv420ToBgra(const uchar * y, const __m256i & u, const __m256i & v, 
			const __m256i & a_0, uchar * bgra)
		{
			Yuv8ToBgra<align>(LoadPermuted<align>((__m256i*)y + 0), 
                _mm256_permute4x64_epi64(_mm256_unpacklo_epi8(u, u), 0xD8), 
                _mm256_permute4x64_epi64(_mm256_unpacklo_epi8(v, v), 0xD8), a_0, (__m256i*)bgra + 0);
			Yuv8ToBgra<align>(LoadPermuted<align>((__m256i*)y + 1), 
                _mm256_permute4x64_epi64(_mm256_unpackhi_epi8(u, u), 0xD8), 
                _mm256_permute4x64_epi64(_mm256_unpackhi_epi8(v, v), 0xD8), a_0, (__m256i*)bgra + 4);
		}

		template <bool align> void Yuv420ToBgra(const uchar * y, size_t yStride, const uchar * u, size_t uStride, const uchar * v, size_t vStride, 
			size_t width, size_t height, uchar * bgra, size_t bgraStride, uchar alpha)
		{
			assert((width%2 == 0) && (height%2 == 0) && (width >= DA) && (height >= 2));
			if(align)
			{
				assert(Aligned(y) && Aligned(yStride) && Aligned(u) &&  Aligned(uStride));
				assert(Aligned(v) && Aligned(vStride) && Aligned(bgra) && Aligned(bgraStride));
			}

			__m256i a_0 = _mm256_slli_si256(_mm256_set1_epi16(alpha), 1);
			size_t bodyWidth = AlignLo(width, DA);
			size_t tail = width - bodyWidth;
			for(size_t row = 0; row < height; row += 2)
			{
				for(size_t colUV = 0, colY = 0, colBgra = 0; colY < bodyWidth; colY += DA, colUV += A, colBgra += OA)
				{
					__m256i u_ = LoadPermuted<align>((__m256i*)(u + colUV));
					__m256i v_ = LoadPermuted<align>((__m256i*)(v + colUV));
					Yuv420ToBgra<align>(y + colY, u_, v_, a_0, bgra + colBgra);
					Yuv420ToBgra<align>(y + colY + yStride, u_, v_, a_0, bgra + colBgra + bgraStride);
				}
				if(tail)
				{
					size_t offset = width - DA;
					__m256i u_ = LoadPermuted<false>((__m256i*)(u + offset/2));
					__m256i v_ = LoadPermuted<false>((__m256i*)(v + offset/2));
					Yuv420ToBgra<false>(y + offset, u_, v_, a_0, bgra + 4*offset);
					Yuv420ToBgra<false>(y + offset + yStride, u_, v_, a_0, bgra + 4*offset + bgraStride);
				}
				y += 2*yStride;
				u += uStride;
				v += vStride;
				bgra += 2*bgraStride;
			}
		}

		void Yuv420ToBgra(const uchar * y, size_t yStride, const uchar * u, size_t uStride, const uchar * v, size_t vStride, 
			size_t width, size_t height, uchar * bgra, ptrdiff_t bgraStride, uchar alpha)
		{
			if(Aligned(y) && Aligned(yStride) && Aligned(u) && Aligned(uStride) 
				&& Aligned(v) && Aligned(vStride) && Aligned(bgra) && Aligned(bgraStride))
				Yuv420ToBgra<true>(y, yStride, u, uStride, v, vStride, width, height, bgra, bgraStride, alpha);
			else
				Yuv420ToBgra<false>(y, yStride, u, uStride, v, vStride, width, height, bgra, bgraStride, alpha);
		}

		void Yuv444ToBgra(const uchar * y, size_t yStride, const uchar * u, size_t uStride, const uchar * v, size_t vStride, 
			size_t width, size_t height, uchar * bgra, ptrdiff_t bgraStride, uchar alpha)
		{
			if(Aligned(y) && Aligned(yStride) && Aligned(u) && Aligned(uStride) 
				&& Aligned(v) && Aligned(vStride) && Aligned(bgra) && Aligned(bgraStride))
				Yuv444ToBgra<true>(y, yStride, u, uStride, v, vStride, width, height, bgra, bgraStride, alpha);
			else
				Yuv444ToBgra<false>(y, yStride, u, uStride, v, vStride, width, height, bgra, bgraStride, alpha);
		}
    }
#endif// SIMD_AVX2_ENABLE
}