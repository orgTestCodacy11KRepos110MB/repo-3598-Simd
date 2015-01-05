/*
* Simd Library (http://simd.sourceforge.net).
*
* Copyright (c) 2011-2015 Yermalayeu Ihar.
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
#include <math.h>

#include "Simd/SimdMemory.h"
#include "Simd/SimdConst.h"
#include "Simd/SimdMath.h"
#include "Simd/SimdBase.h"
#include "Simd/SimdSse2.h"

namespace Simd
{
#ifdef SIMD_SSE2_ENABLE
	namespace Sse2
	{
		const __m128i K16_LINEAR_ROUND_TERM = SIMD_MM_SET1_EPI16(Base::LINEAR_ROUND_TERM);
		const __m128i K16_BILINEAR_ROUND_TERM = SIMD_MM_SET1_EPI16(Base::BILINEAR_ROUND_TERM);

		SIMD_INLINE __m128i Interpolate(__m128i s[2][2], __m128i k[2][2])
		{
			__m128i r = _mm_mullo_epi16(s[0][0], k[0][0]);
			r = _mm_add_epi16(r, _mm_mullo_epi16(s[0][1], k[0][1]));
			r = _mm_add_epi16(r, _mm_mullo_epi16(s[1][0], k[1][0]));
			r = _mm_add_epi16(r, _mm_mullo_epi16(s[1][1], k[1][1]));
			r = _mm_add_epi16(r, K16_BILINEAR_ROUND_TERM);
			return _mm_srli_epi16(r, Base::BILINEAR_SHIFT);
		}

		SIMD_INLINE __m128i Interpolate(__m128i s[2][2][2], __m128i k[2][2])
		{
			return _mm_packus_epi16(Interpolate(s[0], k), Interpolate(s[1], k));
		}

		SIMD_INLINE __m128i Interpolate(__m128i s[2], __m128i k[2])
		{
			__m128i r = _mm_mullo_epi16(s[0], k[0]);
			r = _mm_add_epi16(r, _mm_mullo_epi16(s[1], k[1]));
			r = _mm_add_epi16(r, K16_LINEAR_ROUND_TERM);
			return _mm_srli_epi16(r, Base::LINEAR_SHIFT);
		}

		SIMD_INLINE __m128i Interpolate(__m128i s[2][2], __m128i k[2])
		{
			return _mm_packus_epi16(Interpolate(s[0], k), Interpolate(s[1], k));
		}

		SIMD_INLINE void LoadBlock(const uint8_t *src, __m128i &lo, __m128i &hi)
		{
			const __m128i t = _mm_loadu_si128((__m128i*)(src));
			lo = _mm_unpacklo_epi8(t, K_ZERO);
			hi = _mm_unpackhi_epi8(t, K_ZERO);
		}

		SIMD_INLINE void LoadBlock(const uint8_t *src, size_t dx, size_t dy, __m128i s[2][2][2])
		{
			LoadBlock(src, s[0][0][0], s[1][0][0]);
			LoadBlock(src + dx, s[0][0][1], s[1][0][1]);
			LoadBlock(src + dy, s[0][1][0], s[1][1][0]);
			LoadBlock(src + dy + dx, s[0][1][1], s[1][1][1]);
		}

		SIMD_INLINE void LoadBlock(const uint8_t *src, size_t dr, __m128i s[2][2])
		{
			LoadBlock(src, s[0][0], s[1][0]);
			LoadBlock(src + dr, s[0][1], s[1][1]);
		}

		void ShiftBilinear(const uint8_t *src, size_t srcStride, size_t width, size_t height, size_t channelCount, 
			int fDx, int fDy, uint8_t *dst, size_t dstStride)
		{
			size_t size = width*channelCount; 
			size_t alignedSize = AlignLo(size, A);

			if(fDy)
			{
				if(fDx)
				{
					__m128i k[2][2], s[2][2][2];
					k[0][0] = _mm_set1_epi16((Base::FRACTION_RANGE - fDx)*(Base::FRACTION_RANGE - fDy)); 
					k[0][1] = _mm_set1_epi16(fDx*(Base::FRACTION_RANGE - fDy)); 
					k[1][0] = _mm_set1_epi16((Base::FRACTION_RANGE - fDx)*fDy); 
					k[1][1] = _mm_set1_epi16(fDx*fDy);
					for(size_t row = 0; row < height; ++row)
					{
						for(size_t col = 0; col < alignedSize; col += A)
						{
							LoadBlock(src + col, channelCount, srcStride, s);
							_mm_storeu_si128((__m128i*)(dst + col), Interpolate(s, k));
						}
						if(size != alignedSize)
						{
							LoadBlock(src + size - A, channelCount, srcStride, s);
							_mm_storeu_si128((__m128i*)(dst + size - A), Interpolate(s, k));
						}
						src += srcStride;
						dst += dstStride;
					}
				}
				else
				{
					__m128i k[2], s[2][2];
					k[0] = _mm_set1_epi16(Base::FRACTION_RANGE - fDy); 
					k[1] = _mm_set1_epi16(fDy); 
					for(size_t row = 0; row < height; ++row)
					{
						for(size_t col = 0; col < alignedSize; col += A)
						{
							LoadBlock(src + col, srcStride, s);
							_mm_storeu_si128((__m128i*)(dst + col), Interpolate(s, k));
						}
						if(size != alignedSize)
						{
							LoadBlock(src + size - A, srcStride, s);
							_mm_storeu_si128((__m128i*)(dst + size - A), Interpolate(s, k));
						}
						src += srcStride;
						dst += dstStride;
					}
				}
			}
			else
			{
				if(fDx)
				{
					__m128i k[2], s[2][2];
					k[0] = _mm_set1_epi16(Base::FRACTION_RANGE - fDx); 
					k[1] = _mm_set1_epi16(fDx); 
					for(size_t row = 0; row < height; ++row)
					{
						for(size_t col = 0; col < alignedSize; col += A)
						{
							LoadBlock(src + col, channelCount, s);
							_mm_storeu_si128((__m128i*)(dst + col), Interpolate(s, k));
						}
						if(size != alignedSize)
						{
							LoadBlock(src + size - A, channelCount, s);
							_mm_storeu_si128((__m128i*)(dst + size - A), Interpolate(s, k));
						}
						src += srcStride;
						dst += dstStride;
					}
				}
				else
				{
					for(size_t row = 0; row < height; ++row)
					{
						memcpy(dst, src, size);
						src += srcStride;
						dst += dstStride;
					}
				}
			}
		}

		void ShiftBilinear(
			const uint8_t * src, size_t srcStride, size_t width, size_t height, size_t channelCount, 
			const uint8_t * bkg, size_t bkgStride, double shiftX, double shiftY, 
			size_t cropLeft, size_t cropTop, size_t cropRight, size_t cropBottom, uint8_t * dst, size_t dstStride)
		{
			int fDx, fDy;
			Base::CommonShiftAction(src, srcStride, width, height, channelCount, bkg, bkgStride, shiftX, shiftY, 
				cropLeft, cropTop, cropRight, cropBottom, dst, dstStride, fDx, fDy);

            if(shiftX + A < cropRight - cropLeft)
                Sse2::ShiftBilinear(src, srcStride, width, height, channelCount, fDx, fDy, dst, dstStride);
            else
                Base::ShiftBilinear(src, srcStride, width, height, channelCount, fDx, fDy, dst, dstStride);
		}
	}
#endif//SIMD_SSE2_ENABLE
}

