/*
* Simd Library (http://ermig1979.github.io/Simd).
*
* Copyright (c) 2011-2019 Yermalayeu Ihar.
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
#include "Simd/SimdSynetDeconvolution32f.h"
#include "Simd/SimdSynetConvolution32f.h"
#include "Simd/SimdSynetConvolution32fCommon.h"
#include "Simd/SimdExtract.h"
#include "Simd/SimdSynet.h"
#include "Simd/SimdSse1.h"
#include "Simd/SimdSse2.h"
#include "Simd/SimdGemm.h"
#include "Simd/SimdExp.h"

namespace Simd
{
#ifdef SIMD_SSE2_ENABLE    
    namespace Sse2
    {
        SynetDeconvolution32fGemmNN::SynetDeconvolution32fGemmNN(const DeconvParam32f & p)
            : Base::SynetDeconvolution32fGemmNN(p)
        {
            _gemm.Init(InitGemmFuncs(Sse::Gemm32fNN, "Sse", p.gemm, "Ext"));
            if (_param.trans && _param.group == 1)
            {
                if (NHWC_GEMM_RUNTIME)
                {
                    _gemmCb.Init(InitGemmCbFuncs(Sse::Gemm32fNNcbBufferSize, Sse::Gemm32fNNcbReorderB, Sse::Gemm32fNNcbRun, "Sse", GemmKernelF2, GemmKernelF3));
                    _nhwcWeight.Resize(_gemmCb.At(0).BufferSize(_M*_merge, _N, _K));
                }
                else
                    _nhwcWeight.Resize(Sse::Gemm32fNNcbBufferSize(_M*_merge, _N, _K, GemmKernelAny, NHWC_GEMM_COMPATIBLE));
                _nhwcRun = Sse::Gemm32fNNcbRun;
                _nhwcReorderB = Sse::Gemm32fNNcbReorderB;
            }
            _biasAndActivation = Sse2::ConvolutionBiasAndActivation;
        }

        //---------------------------------------------------------------------

        void * SynetDeconvolution32fInit(size_t batch, const SimdConvolutionParameters * conv, SimdGemm32fNNPtr gemm)
        {
            DeconvParam32f param(batch, conv, gemm);
            if (!param.Valid())
                return NULL;
            //if (SynetConvolution32fDirectNhwc::Preferable(param))
            //    return new SynetConvolution32fDirectNhwc(param);
            //else
                return new SynetDeconvolution32fGemmNN(param);
        }
    }
#endif//SIMD_SSE2_ENABLE
}
