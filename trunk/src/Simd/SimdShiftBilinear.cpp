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
#include <math.h>

#include "Simd/SimdEnable.h"
#include "Simd/SimdMemory.h"
#include "Simd/SimdConst.h"
#include "Simd/SimdMath.h"
#include "Simd/SimdCopy.h"
#include "Simd/SimdShiftBilinear.h"

namespace Simd
{
	namespace Base
	{
		const int LINEAR_SHIFT = 4;
		const int LINEAR_ROUND_TERM = 1 << (LINEAR_SHIFT - 1);

		const int BILINEAR_SHIFT = LINEAR_SHIFT*2;
		const int BILINEAR_ROUND_TERM = 1 << (BILINEAR_SHIFT - 1);

		const int FRACTION_RANGE = 1 << LINEAR_SHIFT;
		const double FRACTION_ROUND_TERM = 0.5/FRACTION_RANGE;

		static void CopyBackground(const uchar * src, size_t srcStride, size_t & width, size_t & height, size_t channelCount, 
			size_t cropLeft, size_t cropTop, size_t cropRight, size_t cropBottom, uchar * dst, size_t dstStride)
		{
			if(cropTop)
			{
				size_t srcOffset = 0;
				size_t dstOffset = 0;
				size_t size = width*channelCount;
				for(size_t row = 0; row < cropTop; ++row)
				{
					memcpy(dst + dstOffset, src + srcOffset, size);
					srcOffset += srcStride;
					dstOffset += dstStride;
				}
			}
			if(height - cropBottom)
			{
				size_t srcOffset = cropBottom*srcStride;
				size_t dstOffset = cropBottom*dstStride;
				size_t size = width*channelCount;
				for(size_t row = cropBottom; row < height; ++row)
				{
					memcpy(dst + dstOffset, src + srcOffset, size);
					srcOffset += srcStride;
					dstOffset += dstStride;
				}
			}
			if(cropLeft)
			{
				size_t srcOffset = cropTop*srcStride;
				size_t dstOffset = cropTop*dstStride;
				size_t size = cropLeft*channelCount;
				for(size_t row = cropTop; row < cropBottom; ++row)
				{
					memcpy(dst + dstOffset, src + srcOffset, size);
					srcOffset += srcStride;
					dstOffset += dstStride;
				}
			}
			if(width - cropRight)
			{
				size_t srcOffset = cropTop*srcStride + cropRight*channelCount;
				size_t dstOffset = cropTop*dstStride + cropRight*channelCount;
				size_t size = (width - cropRight)*channelCount;
				for(size_t row = cropTop; row < cropBottom; ++row)
				{
					memcpy(dst + dstOffset, src + srcOffset, size);
					srcOffset += srcStride;
					dstOffset += dstStride;
				}
			}
		}

		SIMD_INLINE int Interpolate(int s[2][2], int k[2][2])
		{
			return (s[0][0]*k[0][0] + s[0][1]*k[0][1] + 
				s[1][0]*k[1][0] + s[1][1]*k[1][1] + BILINEAR_ROUND_TERM) >> BILINEAR_SHIFT;
		}

		SIMD_INLINE int Interpolate(const unsigned char *src, size_t dx, size_t dy, int k[2][2])
		{
			return (src[0]*k[0][0] + src[dx]*k[0][1] + 
				src[dy]*k[1][0] + src[dx + dy]*k[1][1] + BILINEAR_ROUND_TERM) >> BILINEAR_SHIFT;
		}

		SIMD_INLINE int Interpolate(const unsigned char *src, size_t dr, int k[2])
		{
			return (src[0]*k[0] + src[dr]*k[1] + LINEAR_ROUND_TERM) >> LINEAR_SHIFT;
		}

		void MixBorder(const uchar * src, size_t srcStride, size_t width, size_t height, size_t channelCount,            
			const uchar * bkg, size_t bkgStride, ptrdiff_t iDx, ptrdiff_t iDy, int fDx, int fDy, uchar * dst, size_t dstStride)
		{
			size_t bkgWidth = Abs(iDx) - (iDx < 0 && fDx ? 1 : 0);
			size_t bkgHeight = Abs(iDy) - (iDy < 0 && fDy ? 1 : 0);

			size_t mainWidth = width - bkgWidth - (fDx ? 1 : 0);
			size_t mainHeight = height - bkgHeight - (fDy ? 1 : 0); 

			int k[2][2];
			k[0][0] = (FRACTION_RANGE - fDx)*(FRACTION_RANGE - fDy); 
			k[0][1] = fDx*(FRACTION_RANGE - fDy); 
			k[1][0] = (FRACTION_RANGE - fDx)*fDy; 
			k[1][1] = fDx*fDy; 

			if(fDx)
			{
				const unsigned char* ps[2][2];
				size_t ss[2][2];
				size_t xOffset = (iDx >= 0 ? width - 1 - iDx : - iDx - 1)*channelCount;
				size_t srcOffset = (iDy > 0 ? 0 : - iDy)*srcStride + xOffset;
				size_t bkgOffset = (iDy > 0 ? 0 : - iDy)*bkgStride + xOffset;
				size_t dstOffset = (iDy > 0 ? 0 : - iDy)*dstStride + xOffset;

				if(iDx < 0)
				{
					ps[0][0] = (bkg ? bkg + bkgOffset : 0); 
					ps[0][1] = src + (iDy < 0 ? 0 : iDy)*srcStride; 
					ps[1][0] = (bkg ? bkg + bkgOffset : 0); 
					ps[1][1] = src + ((iDy < 0 ? 0 : iDy) + (fDy ? 1 : 0))*srcStride; 

					ss[0][0] = (bkg ? bkgStride : 0); 
					ss[0][1] = srcStride; 
					ss[1][0] = (bkg ? bkgStride : 0); 
					ss[1][1] = srcStride; 
				}
				else
				{
					ps[0][0] = src + (iDy < 0 ? 0 : iDy)*srcStride + (width - 1)*channelCount; 
					ps[0][1] = (bkg ? bkg + bkgOffset : 0); 
					ps[1][0] = src + ((iDy < 0 ? 0 : iDy) + (fDy ? 1 : 0))*srcStride + (width - 1)*channelCount; 
					ps[1][1] = (bkg ? bkg + bkgOffset : 0); 

					ss[0][0] = srcStride; 
					ss[0][1] = (bkg ? bkgStride : 0); 
					ss[1][0] = srcStride; 
					ss[1][1] = (bkg ? bkgStride : 0); 
				}

				for(size_t row = 0; row < mainHeight; ++row)
				{
					for(size_t channel = 0; channel < channelCount; channel++)
					{
						int s[2][2];
						s[0][0] = (ps[0][0] ? ps[0][0][channel] : 0); 
						s[0][1] = (ps[0][1] ? ps[0][1][channel] : 0); 
						s[1][0] = (ps[1][0] ? ps[1][0][channel] : 0); 
						s[1][1] = (ps[1][1] ? ps[1][1][channel] : 0); 
						dst[dstOffset + channel] = Interpolate(s, k);
					}
					ps[0][0] += (ps[0][0] ? ss[0][0] : 0); 
					ps[0][1] += (ps[0][1] ? ss[0][1] : 0); 
					ps[1][0] += (ps[1][0] ? ss[1][0] : 0); 
					ps[1][1] += (ps[1][1] ? ss[1][1] : 0); 
					dstOffset += dstStride;
				}
			}

			if(fDy)
			{
				const unsigned char* ps[2][2];
				size_t srcOffset = (iDy >= 0 ? height - 1 - iDy : - iDy - 1)*srcStride + (iDx > 0 ? 0 : -iDx)*channelCount;
				size_t bkgOffset = (iDy >= 0 ? height - 1 - iDy : - iDy - 1)*bkgStride + (iDx > 0 ? 0 : -iDx)*channelCount;
				size_t dstOffset = (iDy >= 0 ? height - 1 - iDy : - iDy - 1)*dstStride + (iDx > 0 ? 0 : -iDx)*channelCount;

				if(iDy < 0)
				{
					ps[0][0] = (bkg ? bkg + bkgOffset : 0);
					ps[0][1] = (bkg ? bkg + bkgOffset : 0); 
					ps[1][0] = src + (iDx < 0 ? 0 : iDx)*channelCount; 
					ps[1][1] = src + ((iDx < 0 ? 0 : iDx) + (fDx ? 1 : 0))*channelCount; 
				}
				else
				{
					ps[0][0] = src + (height - 1)*srcStride + (iDx < 0 ? 0 : iDx)*channelCount;
					ps[0][1] = src + (height - 1)*srcStride + ((iDx < 0 ? 0 : iDx) + (fDx ? 1 : 0))*channelCount; 
					ps[1][0] = (bkg ? bkg + bkgOffset : 0); 
					ps[1][1] = (bkg ? bkg + bkgOffset : 0);
				}

				for(size_t col = 0; col < mainWidth; ++col)
				{
					for(size_t channel = 0; channel < channelCount; channel++)
					{
						int s[2][2];
						s[0][0] = (ps[0][0] ? ps[0][0][channel] : 0); 
						s[0][1] = (ps[0][1] ? ps[0][1][channel] : 0); 
						s[1][0] = (ps[1][0] ? ps[1][0][channel] : 0); 
						s[1][1] = (ps[1][1] ? ps[1][1][channel] : 0); 
						dst[dstOffset + channel] = Interpolate(s, k);
					}
					ps[0][0] += (ps[0][0] ? channelCount : 0); 
					ps[0][1] += (ps[0][1] ? channelCount : 0); 
					ps[1][0] += (ps[1][0] ? channelCount : 0); 
					ps[1][1] += (ps[1][1] ? channelCount : 0); 
					dstOffset += channelCount;
				}
			}

			if(fDx && fDy)
			{
				const unsigned char* ps[2][2];
				size_t xOffset = (iDx >= 0 ? width - 1 - iDx : - iDx - 1)*channelCount;
				size_t srcOffset = (iDy >= 0 ? height - 1 - iDy : - iDy - 1)*srcStride + xOffset;
				size_t bkgOffset = (iDy >= 0 ? height - 1 - iDy : - iDy - 1)*bkgStride + xOffset;
				size_t dstOffset = (iDy >= 0 ? height - 1 - iDy : - iDy - 1)*dstStride + xOffset;

				ps[0][0] = (iDx >= 0 && iDy >= 0) ? (src + (height - 1)*srcStride + (width - 1)*channelCount): (bkg ? bkg + bkgOffset : 0); 
				ps[0][1] = (iDx < 0 && iDy >= 0) ? (src + (height - 1)*srcStride): (bkg ? bkg + bkgOffset : 0); 
				ps[1][0] = (iDx >= 0 && iDy < 0) ? (src + (width - 1)*channelCount): (bkg ? bkg + bkgOffset : 0); 
				ps[1][1] = (iDx < 0 && iDy < 0) ? (src): (bkg ? bkg + bkgOffset : 0); 

				for(size_t channel = 0; channel < channelCount; channel++)
				{
					int s[2][2];
					s[0][0] = (ps[0][0] ? ps[0][0][channel] : 0); 
					s[0][1] = (ps[0][1] ? ps[0][1][channel] : 0); 
					s[1][0] = (ps[1][0] ? ps[1][0][channel] : 0); 
					s[1][1] = (ps[1][1] ? ps[1][1][channel] : 0); 
					dst[dstOffset + channel] = Interpolate(s, k);
				}
			}
		}

		void ShiftBilinear(const uchar * src, size_t srcStride, size_t width, size_t height, size_t channelCount, 
			int fDx, int fDy, uchar * dst, size_t dstStride)
		{
			size_t size = width*channelCount;
			if(fDy)
			{
				if(fDx)
				{
					int k[2][2];
					k[0][0] = (FRACTION_RANGE - fDx)*(FRACTION_RANGE - fDy); 
					k[0][1] = fDx*(FRACTION_RANGE - fDy); 
					k[1][0] = (FRACTION_RANGE - fDx)*fDy; 
					k[1][1] = fDx*fDy; 
					for(size_t row = 0; row < height; ++row)
					{
						for(size_t col = 0; col < size; col++)
						{
							dst[col] = Interpolate(src + col, channelCount, srcStride, k);
						}
						src += srcStride;
						dst += dstStride;
					}
				}
				else
				{
					int k[2];
					k[0] = FRACTION_RANGE - fDy; 
					k[1] = fDy; 
					for(size_t row = 0; row < height; ++row)
					{
						for(size_t col = 0; col < size; col++)
						{
							dst[col] = Interpolate(src + col, srcStride, k);
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
					int k[2];
					k[0] = FRACTION_RANGE - fDx; 
					k[1] = fDx; 
					for(size_t row = 0; row < height; ++row)
					{
						for(size_t col = 0; col < size; col++)
						{
							dst[col] = Interpolate(src + col, channelCount, k);
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
			const uchar * src, size_t srcStride, size_t width, size_t height, size_t channelCount, 
			const uchar * bkg, size_t bkgStride, double shiftX, double shiftY, 
			size_t cropLeft, size_t cropTop, size_t cropRight, size_t cropBottom, uchar * dst, size_t dstStride)
		{
			assert(cropLeft <= cropRight && cropTop <= cropBottom && cropRight <= width && cropBottom <= height);
			assert(shiftX < cropRight - cropLeft && shiftY < cropBottom - cropTop);

			CopyBackground(src, srcStride, width, height, channelCount, cropLeft, cropTop, cropRight, cropBottom, dst, dstStride);

			dst += dstStride*cropTop + cropLeft*channelCount;
			src += srcStride*cropTop + cropLeft*channelCount;
			bkg += bkgStride*cropTop + cropLeft*channelCount;
			width = cropRight - cropLeft;
			height = cropBottom - cropTop;

			ptrdiff_t iDx = (ptrdiff_t)floor(shiftX + FRACTION_ROUND_TERM);
			ptrdiff_t iDy = (ptrdiff_t)floor(shiftY + FRACTION_ROUND_TERM);
			int fDx = (int)floor((shiftX + FRACTION_ROUND_TERM - iDx)*FRACTION_RANGE);
			int fDy = (int)floor((shiftY + FRACTION_ROUND_TERM - iDy)*FRACTION_RANGE);

			int left = (iDx < 0 ? (-iDx - (fDx ? 1 : 0)) : 0);
			int top = (iDy < 0 ? (-iDy - (fDy ? 1 : 0)) : 0);
			int right = (iDx < 0 ? width : width - iDx);
			int bottom = (iDy < 0 ? height : height - iDy);

			CopyBackground(bkg, bkgStride, width, height, channelCount, left, top, right, bottom, dst, dstStride);

			MixBorder(src, srcStride, width, height, channelCount, bkg, bkgStride, iDx, iDy, fDx, fDy, dst, dstStride);

			src += Max(0, iDy)*srcStride + Max(0, iDx)*channelCount;
			dst += Max(0,-iDy)*dstStride + Max(0,-iDx)*channelCount;

			width = width - Abs(iDx) + (iDx < 0 && fDx ? 1 : 0) - (fDx ? 1 : 0);
			height = height - Abs(iDy) + (iDy < 0 && fDy ? 1 : 0) - (fDy ? 1 : 0); 

			ShiftBilinear(src, srcStride, width, height, channelCount, fDx, fDy, dst, dstStride);
		}
	}

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

		SIMD_INLINE void LoadBlock(const uchar *src, __m128i &lo, __m128i &hi)
		{
			const __m128i t = _mm_loadu_si128((__m128i*)(src));
			lo = _mm_unpacklo_epi8(t, K_ZERO);
			hi = _mm_unpackhi_epi8(t, K_ZERO);
		}

		SIMD_INLINE void LoadBlock(const uchar *src, size_t dx, size_t dy, __m128i s[2][2][2])
		{
			LoadBlock(src, s[0][0][0], s[1][0][0]);
			LoadBlock(src + dx, s[0][0][1], s[1][0][1]);
			LoadBlock(src + dy, s[0][1][0], s[1][1][0]);
			LoadBlock(src + dy + dx, s[0][1][1], s[1][1][1]);
		}

		SIMD_INLINE void LoadBlock(const uchar *src, size_t dr, __m128i s[2][2])
		{
			LoadBlock(src, s[0][0], s[1][0]);
			LoadBlock(src + dr, s[0][1], s[1][1]);
		}

		void ShiftBilinear(const uchar *src, size_t srcStride, size_t width, size_t height, size_t channelCount, 
			int fDx, int fDy, uchar *dst, size_t dstStride)
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
			const uchar * src, size_t srcStride, size_t width, size_t height, size_t channelCount, 
			const uchar * bkg, size_t bkgStride, double shiftX, double shiftY, 
			size_t cropLeft, size_t cropTop, size_t cropRight, size_t cropBottom, uchar * dst, size_t dstStride)
		{
			assert(cropLeft <= cropRight && cropTop <= cropBottom && cropRight <= width && cropBottom <= height);
			assert(shiftX + A < cropRight - cropLeft && shiftY < cropBottom - cropTop);

			Base::CopyBackground(src, srcStride, width, height, channelCount, cropLeft, cropTop, cropRight, cropBottom, dst, dstStride);

			dst += dstStride*cropTop + cropLeft*channelCount;
			src += srcStride*cropTop + cropLeft*channelCount;
			bkg += bkgStride*cropTop + cropLeft*channelCount;
			width = cropRight - cropLeft;
			height = cropBottom - cropTop;

			ptrdiff_t iDx = (ptrdiff_t)floor(shiftX + Base::FRACTION_ROUND_TERM);
			ptrdiff_t iDy = (ptrdiff_t)floor(shiftY + Base::FRACTION_ROUND_TERM);
			int fDx = (int)floor((shiftX + Base::FRACTION_ROUND_TERM - iDx)*Base::FRACTION_RANGE);
			int fDy = (int)floor((shiftY + Base::FRACTION_ROUND_TERM - iDy)*Base::FRACTION_RANGE);

			int left = (iDx < 0 ? (-iDx - (fDx ? 1 : 0)) : 0);
			int top = (iDy < 0 ? (-iDy - (fDy ? 1 : 0)) : 0);
			int right = (iDx < 0 ? width : width - iDx);
			int bottom = (iDy < 0 ? height : height - iDy);

			Base::CopyBackground(bkg, bkgStride, width, height, channelCount, left, top, right, bottom, dst, dstStride);

			Base::MixBorder(src, srcStride, width, height, channelCount, bkg, bkgStride, iDx, iDy, fDx, fDy, dst, dstStride);

			src += Max(0, iDy)*srcStride + Max(0, iDx)*channelCount;
			dst += Max(0,-iDy)*dstStride + Max(0,-iDx)*channelCount;

			width = width - Abs(iDx) + (iDx < 0 && fDx ? 1 : 0) - (fDx ? 1 : 0);
			height = height - Abs(iDy) + (iDy < 0 && fDy ? 1 : 0) - (fDy ? 1 : 0); 

			ShiftBilinear(src, srcStride, width, height, channelCount, fDx, fDy, dst, dstStride);
		}
	}
#endif//SIMD_SSE2_ENABLE

	void ShiftBilinear(
		const uchar * src, size_t srcStride, size_t width, size_t height, size_t channelCount, 
		const uchar * bkg, size_t bkgStride, double shiftX, double shiftY, 
		size_t cropLeft, size_t cropTop, size_t cropRight, size_t cropBottom, uchar * dst, size_t dstStride)
	{

#ifdef SIMD_SSE2_ENABLE
		if(Sse2::Enable && shiftX + Sse2::A < cropRight - cropLeft)
			Sse2::ShiftBilinear(src, srcStride, width, height, channelCount, bkg, bkgStride,
			shiftX, shiftY, cropLeft, cropTop, cropRight, cropBottom, dst, dstStride);
		else
#endif//SIMD_SSE2_ENABLE
			Base::ShiftBilinear(src, srcStride, width, height, channelCount, bkg, bkgStride,
			shiftX, shiftY, cropLeft, cropTop, cropRight, cropBottom, dst, dstStride);
	}

	void ShiftBilinear(const View & src, const View & bkg, double shiftX, double shiftY, 
		size_t cropLeft, size_t cropTop, size_t cropRight, size_t cropBottom, View & dst)
	{
		assert(src.format == dst.format && src.width == dst.width && src.height == dst.height);
		assert(src.format == bkg.format && src.width == bkg.width && src.height == bkg.height);
		assert(src.format == View::Gray8 || src.format == View::Uv16 || src.format == View::Bgr24 || src.format == View::Bgra32);

		ShiftBilinear(src.data, src.stride, src.width, src.height, View::SizeOf(src.format), bkg.data, bkg.stride,
			shiftX, shiftY, cropLeft, cropTop, cropRight, cropBottom, dst.data, dst.stride);
	}
}

