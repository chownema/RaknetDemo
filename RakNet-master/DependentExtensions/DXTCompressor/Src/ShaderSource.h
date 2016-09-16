/*
Real-time DXT1 & YCoCg DXT5 compression (Cg 2.0)
Copyright (c) NVIDIA Corporation.
Written by: Ignacio Castano <icastano@nvidia.com>

Thanks to JMP van Waveren, Simon Green, Eric Werness, Simon Brown

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
*/

const char* pDXTCompressorShaderSource =
"	\n"
"// vertex program	\n"
"void compress_vp(float4 pos : POSITION,	\n"
"                      float2 texcoord : TEXCOORD0,	\n"
"                      out float4 hpos : POSITION,	\n"
"                      out float2 o_texcoord : TEXCOORD0	\n"
"                      )	\n"
"{	\n"
"    o_texcoord = texcoord;	\n"
"    hpos = pos;	\n"
"}	\n"
"	\n"
"typedef unsigned int uint;	\n"
"typedef unsigned int2 uint2;	\n"
"typedef unsigned int3 uint3;	\n"
"typedef unsigned int4 uint4;	\n"
"	\n"
"const float offset = 128.0 / 255.0;	\n"
"	\n"
"// Use dot product to minimize RMS instead absolute distance like in the CPU compressor.	\n"
"float colorDistance(float3 c0, float3 c1)	\n"
"{	\n"
"    return dot(c0-c1, c0-c1);	\n"
"}	\n"
"float colorDistance(float2 c0, float2 c1)	\n"
"{	\n"
"    return dot(c0-c1, c0-c1);	\n"
"}	\n"
"	\n"
"	\n"
"void ExtractColorBlockRGB(out float3 col[16], sampler2D image, float2 texcoord, float2 imageSize)	\n"
"{	\n"
"    // use TXF instruction (integer coordinates with offset)	\n"
"    // note offsets must be constant	\n"
"    //int4 base = int4(wpos*4-2, 0, 0);	\n"
"    int4 base = int4(texcoord * imageSize - 1.5, 0, 0);	\n"
"    col[0] = tex2Dfetch(image, base, int2(0, 0)).rgb;	\n"
"    col[1] = tex2Dfetch(image, base, int2(1, 0)).rgb;	\n"
"    col[2] = tex2Dfetch(image, base, int2(2, 0)).rgb;	\n"
"    col[3] = tex2Dfetch(image, base, int2(3, 0)).rgb;	\n"
"    col[4] = tex2Dfetch(image, base, int2(0, 1)).rgb;	\n"
"    col[5] = tex2Dfetch(image, base, int2(1, 1)).rgb;	\n"
"    col[6] = tex2Dfetch(image, base, int2(2, 1)).rgb;	\n"
"    col[7] = tex2Dfetch(image, base, int2(3, 1)).rgb;	\n"
"    col[8] = tex2Dfetch(image, base, int2(0, 2)).rgb;	\n"
"    col[9] = tex2Dfetch(image, base, int2(1, 2)).rgb;	\n"
"    col[10] = tex2Dfetch(image, base, int2(2, 2)).rgb;	\n"
"    col[11] = tex2Dfetch(image, base, int2(3, 2)).rgb;	\n"
"    col[12] = tex2Dfetch(image, base, int2(0, 3)).rgb;	\n"
"    col[13] = tex2Dfetch(image, base, int2(1, 3)).rgb;	\n"
"    col[14] = tex2Dfetch(image, base, int2(2, 3)).rgb;	\n"
"    col[15] = tex2Dfetch(image, base, int2(3, 3)).rgb;	\n"
"}	\n"
"	\n"
"void ExtractColorBlockBGR(out float3 col[16], sampler2D image, float2 texcoord, float2 imageSize)	\n"
"{	\n"
"    // use TXF instruction (integer coordinates with offset)	\n"
"    // note offsets must be constant	\n"
"    //int4 base = int4(wpos*4-2, 0, 0);	\n"
"    int4 base = int4(texcoord * imageSize - 1.5, 0, 0);	\n"
"    col[0] = tex2Dfetch(image, base, int2(0, 0)).bgr;	\n"
"    col[1] = tex2Dfetch(image, base, int2(1, 0)).bgr;	\n"
"    col[2] = tex2Dfetch(image, base, int2(2, 0)).bgr;	\n"
"    col[3] = tex2Dfetch(image, base, int2(3, 0)).bgr;	\n"
"    col[4] = tex2Dfetch(image, base, int2(0, 1)).bgr;	\n"
"    col[5] = tex2Dfetch(image, base, int2(1, 1)).bgr;	\n"
"    col[6] = tex2Dfetch(image, base, int2(2, 1)).bgr;	\n"
"    col[7] = tex2Dfetch(image, base, int2(3, 1)).bgr;	\n"
"    col[8] = tex2Dfetch(image, base, int2(0, 2)).bgr;	\n"
"    col[9] = tex2Dfetch(image, base, int2(1, 2)).bgr;	\n"
"    col[10] = tex2Dfetch(image, base, int2(2, 2)).bgr;	\n"
"    col[11] = tex2Dfetch(image, base, int2(3, 2)).bgr;	\n"
"    col[12] = tex2Dfetch(image, base, int2(0, 3)).bgr;	\n"
"    col[13] = tex2Dfetch(image, base, int2(1, 3)).bgr;	\n"
"    col[14] = tex2Dfetch(image, base, int2(2, 3)).bgr;	\n"
"    col[15] = tex2Dfetch(image, base, int2(3, 3)).bgr;	\n"
"}	\n"
"	\n"
"float3 toYCoCg(float3 c)	\n"
"{	\n"
"    float Y = (c.r + 2 * c.g + c.b) * 0.25;	\n"
"    float Co = ( ( 2 * c.r - 2 * c.b      ) * 0.25 + offset );	\n"
"    float Cg = ( (    -c.r + 2 * c.g - c.b) * 0.25 + offset );	\n"
"	\n"
"    return float3(Y, Co, Cg);	\n"
"}	\n"
"	\n"
"void ExtractColorBlockYCoCg_RGB(out float3 col[16], sampler2D image, float2 texcoord, float2 imageSize)	\n"
"{	\n"
"    // use TXF instruction (integer coordinates with offset)	\n"
"    // note offsets must be constant	\n"
"    //int4 base = int4(wpos*4-2, 0, 0);	\n"
"    int4 base = int4(texcoord * imageSize - 1.5, 0, 0);	\n"
"    col[0] = toYCoCg(tex2Dfetch(image, base, int2(0, 0)).rgb);	\n"
"    col[1] = toYCoCg(tex2Dfetch(image, base, int2(1, 0)).rgb);	\n"
"    col[2] = toYCoCg(tex2Dfetch(image, base, int2(2, 0)).rgb);	\n"
"    col[3] = toYCoCg(tex2Dfetch(image, base, int2(3, 0)).rgb);	\n"
"    col[4] = toYCoCg(tex2Dfetch(image, base, int2(0, 1)).rgb);	\n"
"    col[5] = toYCoCg(tex2Dfetch(image, base, int2(1, 1)).rgb);	\n"
"    col[6] = toYCoCg(tex2Dfetch(image, base, int2(2, 1)).rgb);	\n"
"    col[7] = toYCoCg(tex2Dfetch(image, base, int2(3, 1)).rgb);	\n"
"    col[8] = toYCoCg(tex2Dfetch(image, base, int2(0, 2)).rgb);	\n"
"    col[9] = toYCoCg(tex2Dfetch(image, base, int2(1, 2)).rgb);	\n"
"    col[10] = toYCoCg(tex2Dfetch(image, base, int2(2, 2)).rgb);	\n"
"    col[11] = toYCoCg(tex2Dfetch(image, base, int2(3, 2)).rgb);	\n"
"    col[12] = toYCoCg(tex2Dfetch(image, base, int2(0, 3)).rgb);	\n"
"    col[13] = toYCoCg(tex2Dfetch(image, base, int2(1, 3)).rgb);	\n"
"    col[14] = toYCoCg(tex2Dfetch(image, base, int2(2, 3)).rgb);	\n"
"    col[15] = toYCoCg(tex2Dfetch(image, base, int2(3, 3)).rgb);	\n"
"}	\n"
"	\n"
"void ExtractColorBlockYCoCg_BGR(out float3 col[16], sampler2D image, float2 texcoord, float2 imageSize)	\n"
"{	\n"
"    // use TXF instruction (integer coordinates with offset)	\n"
"    // note offsets must be constant	\n"
"    //int4 base = int4(wpos*4-2, 0, 0);	\n"
"    int4 base = int4(texcoord * imageSize - 1.5, 0, 0);	\n"
"    col[0] = toYCoCg(tex2Dfetch(image, base, int2(0, 0)).bgr);	\n"
"    col[1] = toYCoCg(tex2Dfetch(image, base, int2(1, 0)).bgr);	\n"
"    col[2] = toYCoCg(tex2Dfetch(image, base, int2(2, 0)).bgr);	\n"
"    col[3] = toYCoCg(tex2Dfetch(image, base, int2(3, 0)).bgr);	\n"
"    col[4] = toYCoCg(tex2Dfetch(image, base, int2(0, 1)).bgr);	\n"
"    col[5] = toYCoCg(tex2Dfetch(image, base, int2(1, 1)).bgr);	\n"
"    col[6] = toYCoCg(tex2Dfetch(image, base, int2(2, 1)).bgr);	\n"
"    col[7] = toYCoCg(tex2Dfetch(image, base, int2(3, 1)).bgr);	\n"
"    col[8] = toYCoCg(tex2Dfetch(image, base, int2(0, 2)).bgr);	\n"
"    col[9] = toYCoCg(tex2Dfetch(image, base, int2(1, 2)).bgr);	\n"
"    col[10] = toYCoCg(tex2Dfetch(image, base, int2(2, 2)).bgr);	\n"
"    col[11] = toYCoCg(tex2Dfetch(image, base, int2(3, 2)).bgr);	\n"
"    col[12] = toYCoCg(tex2Dfetch(image, base, int2(0, 3)).bgr);	\n"
"    col[13] = toYCoCg(tex2Dfetch(image, base, int2(1, 3)).bgr);	\n"
"    col[14] = toYCoCg(tex2Dfetch(image, base, int2(2, 3)).bgr);	\n"
"    col[15] = toYCoCg(tex2Dfetch(image, base, int2(3, 3)).bgr);	\n"
"}	\n"
"	\n"
"// find minimum and maximum colors based on bounding box in color space	\n"
"void FindMinMaxColorsBox(float3 block[16], out float3 mincol, out float3 maxcol)	\n"
"{	\n"
"    mincol = block[0];	\n"
"    maxcol = block[0];	\n"
"    	\n"
"    for (int i = 1; i < 16; i++) {	\n"
"        mincol = min(mincol, block[i]);	\n"
"        maxcol = max(maxcol, block[i]);	\n"
"    }	\n"
"}	\n"
"	\n"
"void InsetBBox(in out float3 mincol, in out float3 maxcol)	\n"
"{	\n"
"    float3 inset = (maxcol - mincol) / 16.0 - (8.0 / 255.0) / 16;	\n"
"    mincol = saturate(mincol + inset);	\n"
"    maxcol = saturate(maxcol - inset);	\n"
"}	\n"
"void InsetYBBox(in out float mincol, in out float maxcol)	\n"
"{	\n"
"    float inset = (maxcol - mincol) / 32.0 - (16.0 / 255.0) / 32.0;	\n"
"    mincol = saturate(mincol + inset);	\n"
"    maxcol = saturate(maxcol - inset);	\n"
"}	\n"
"void InsetCoCgBBox(in out float2 mincol, in out float2 maxcol)	\n"
"{	\n"
"    float2 inset = (maxcol - mincol) / 16.0 - (8.0 / 255.0) / 16;	\n"
"    mincol = saturate(mincol + inset);	\n"
"    maxcol = saturate(maxcol - inset);	\n"
"}	\n"
"	\n"
"void SelectDiagonal(float3 block[16], in out float3 mincol, in out float3 maxcol)	\n"
"{	\n"
"    float3 center = (mincol + maxcol) * 0.5;	\n"
"	\n"
"    float2 cov = 0;	\n"
"    for (int i = 0; i < 16; i++)	\n"
"    {	\n"
"        float3 t = block[i] - center;	\n"
"        cov.x += t.x * t.z;	\n"
"        cov.y += t.y * t.z;	\n"
"    }	\n"
"	\n"
"    if (cov.x < 0) {	\n"
"        float temp = maxcol.x;	\n"
"        maxcol.x = mincol.x;	\n"
"        mincol.x = temp;	\n"
"    }	\n"
"    if (cov.y < 0) {	\n"
"        float temp = maxcol.y;	\n"
"        maxcol.y = mincol.y;	\n"
"        mincol.y = temp;	\n"
"    }	\n"
"}	\n"
"	\n"
"float3 RoundAndExpand(float3 v, out uint w)	\n"
"{	\n"
"    int3 c = round(v * float3(31, 63, 31));	\n"
"    w = (c.r << 11) | (c.g << 5) | c.b;	\n"
"	\n"
"    c.rb = (c.rb << 3) | (c.rb >> 2);	\n"
"    c.g = (c.g << 2) | (c.g >> 4);	\n"
"	\n"
"    return (float3)c * (1.0 / 255.0);	\n"
"}	\n"
"	\n"
"uint EmitEndPointsDXT1(in out float3 mincol, in out float3 maxcol)	\n"
"{	\n"
"    uint2 output;	\n"
"    maxcol = RoundAndExpand(maxcol, output.x);	\n"
"    mincol = RoundAndExpand(mincol, output.y);	\n"
"	\n"
"    // We have to do this in case we select an alternate diagonal.	\n"
"    if (output.x < output.y)	\n"
"    {	\n"
"        float3 tmp = mincol;	\n"
"        mincol = maxcol;	\n"
"        maxcol = tmp;	\n"
"        return output.y | (output.x << 16);	\n"
"    }	\n"
"	\n"
"    return output.x | (output.y << 16);	\n"
"}	\n"
"	\n"
"uint EmitIndicesDXT1(float3 block[16], float3 mincol, float3 maxcol)	\n"
"{	\n"
"    const float RGB_RANGE = 3;	\n"
"	\n"
"    float3 dir = (maxcol - mincol);	\n"
"    float3 origin = maxcol + dir / (2.0 * RGB_RANGE);	\n"
"    dir /= dot(dir, dir);	\n"
"	\n"
"    // Compute indices	\n"
"    uint indices = 0;	\n"
"    for (int i = 0; i < 16; i++)	\n"
"    {	\n"
"        uint index = saturate(dot(origin - block[i], dir)) * RGB_RANGE;	\n"
"        indices |= index << (i * 2);	\n"
"    }	\n"
"	\n"
"    uint i0 = (indices & 0x55555555);	\n"
"    uint i1 = (indices & 0xAAAAAAAA) >> 1;	\n"
"    indices = ((i0 ^ i1) << 1) | i1;	\n"
"	\n"
"    // Output indices	\n"
"    return indices;	\n"
"}	\n"
"	\n"
"int ScaleYCoCg(float2 minColor, float2 maxColor)	\n"
"{	\n"
"    float2 m0 = abs(minColor - offset);	\n"
"    float2 m1 = abs(maxColor - offset);	\n"
"	\n"
"    float m = max(max(m0.x, m0.y), max(m1.x, m1.y));	\n"
"	\n"
"    const float s0 = 64.0 / 255.0;	\n"
"    const float s1 = 32.0 / 255.0;	\n"
"	\n"
"    int scale = 1;	\n"
"    if (m < s0) scale = 2;	\n"
"    if (m < s1) scale = 4;	\n"
"	\n"
"    return scale;	\n"
"}	\n"
"	\n"
"void SelectYCoCgDiagonal(const float3 block[16], in out float2 minColor, in out float2 maxColor)	\n"
"{	\n"
"    float2 mid = (maxColor + minColor) * 0.5;	\n"
"	\n"
"    float cov = 0;	\n"
"    for (int i = 0; i < 16; i++)	\n"
"    {	\n"
"        float2 t = block[i].yz - mid;	\n"
"        cov += t.x * t.y;	\n"
"    }	\n"
"    if (cov < 0) {	\n"
"        float tmp = maxColor.y;	\n"
"        maxColor.y = minColor.y;	\n"
"        minColor.y = tmp;	\n"
"    }	\n"
"}	\n"
"	\n"
"	\n"
"uint EmitEndPointsYCoCgDXT5(in out float2 mincol, in out float2 maxcol, int scale)	\n"
"{	\n"
"    maxcol = (maxcol - offset) * scale + offset;	\n"
"    mincol = (mincol - offset) * scale + offset;	\n"
"	\n"
"    InsetCoCgBBox(mincol, maxcol);	\n"
"	\n"
"    maxcol = round(maxcol * float2(31, 63));	\n"
"    mincol = round(mincol * float2(31, 63));	\n"
"	\n"
"    int2 imaxcol = maxcol;	\n"
"    int2 imincol = mincol;	\n"
"	\n"
"    uint2 output;	\n"
"    output.x = (imaxcol.r << 11) | (imaxcol.g << 5) | (scale - 1);	\n"
"    output.y = (imincol.r << 11) | (imincol.g << 5) | (scale - 1);	\n"
"	\n"
"    imaxcol.r = (imaxcol.r << 3) | (imaxcol.r >> 2);	\n"
"    imaxcol.g = (imaxcol.g << 2) | (imaxcol.g >> 4);	\n"
"    imincol.r = (imincol.r << 3) | (imincol.r >> 2);	\n"
"    imincol.g = (imincol.g << 2) | (imincol.g >> 4);	\n"
"	\n"
"    maxcol = (float2)imaxcol * (1.0 / 255.0);	\n"
"    mincol = (float2)imincol * (1.0 / 255.0);	\n"
"	\n"
"    // Undo rescale.	\n"
"    maxcol = (maxcol - offset) / scale + offset;	\n"
"    mincol = (mincol - offset) / scale + offset;	\n"
"	\n"
"    return output.x | (output.y << 16);	\n"
"}	\n"
"	\n"
"uint EmitIndicesYCoCgDXT5(float3 block[16], float2 mincol, float2 maxcol)	\n"
"{	\n"
"    const float COCG_RANGE = 3;	\n"
"	\n"
"    float2 dir = (maxcol - mincol);	\n"
"    float2 origin = maxcol + dir / (2.0 * COCG_RANGE);	\n"
"    dir /= dot(dir, dir);	\n"
"	\n"
"    // Compute indices	\n"
"    uint indices = 0;	\n"
"    for (int i = 0; i < 16; i++)	\n"
"    {	\n"
"        uint index = saturate(dot(origin - block[i].yz, dir)) * COCG_RANGE;	\n"
"        indices |= index << (i * 2);	\n"
"    }	\n"
"	\n"
"    uint i0 = (indices & 0x55555555);	\n"
"    uint i1 = (indices & 0xAAAAAAAA) >> 1;	\n"
"    indices = ((i0 ^ i1) << 1) | i1;	\n"
"	\n"
"    // Output indices	\n"
"    return indices;	\n"
"}	\n"
"	\n"
"uint EmitAlphaEndPointsYCoCgDXT5(float mincol, float maxcol)	\n"
"{	\n"
"    uint c0 = round(mincol * 255);	\n"
"    uint c1 = round(maxcol * 255);	\n"
"	\n"
"    return (c0 << 8) | c1;	\n"
"}	\n"
"	\n"
"// Optimized index selection.	\n"
"uint2 EmitAlphaIndicesYCoCgDXT5(float3 block[16], float minAlpha, float maxAlpha)	\n"
"{	\n"
"    const int ALPHA_RANGE = 7;	\n"
"	\n"
"    float bias = maxAlpha + (maxAlpha - minAlpha) / (2.0 * ALPHA_RANGE);	\n"
"    float scale = 1.0f / (maxAlpha - minAlpha);	\n"
"	\n"
"    uint2 indices = 0;	\n"
"	\n"
"    for (int i = 0; i < 6; i++)	\n"
"    {	\n"
"        uint index = saturate((bias - block[i].x) * scale) * ALPHA_RANGE;	\n"
"        indices.x |= index << (3 * i);	\n"
"    }	\n"
"	\n"
"    for (int i = 6; i < 16; i++)	\n"
"    {	\n"
"        uint index = saturate((bias - block[i].x) * scale) * ALPHA_RANGE;	\n"
"        indices.y |= index << (3 * i - 18);	\n"
"    }	\n"
"	\n"
"    uint2 i0 = (indices >> 0) & 0x09249249;	\n"
"    uint2 i1 = (indices >> 1) & 0x09249249;	\n"
"    uint2 i2 = (indices >> 2) & 0x09249249;	\n"
"	\n"
"    i2 ^= i0 & i1;	\n"
"    i1 ^= i0;	\n"
"    i0 ^= (i1 | i2);	\n"
"	\n"
"    indices.x = (i2.x << 2) | (i1.x << 1) | i0.x;	\n"
"    indices.y = (((i2.y << 2) | (i1.y << 1) | i0.y) << 2) | (indices.x >> 16);	\n"
"    indices.x <<= 16;	\n"
"	\n"
"    return indices;	\n"
"}	\n"
"	\n"
"// compress a 4x4 block to DXT1 format	\n"
"// integer version, renders to 2 x int32 buffer	\n"
"uint4 compress_DXT1_RGBA_fp(float2 texcoord : TEXCOORD0,	\n"
"                      uniform sampler2D image,	\n"
"                      uniform float2 imageSize = { 512.0, 512.0 }	\n"
"                      ) : COLOR	\n"
"{	\n"
"    // read block	\n"
"    float3 block[16];	\n"
"    ExtractColorBlockRGB(block, image, texcoord, imageSize);	\n"
"	\n"
"    // find min and max colors	\n"
"    float3 mincol, maxcol;	\n"
"    FindMinMaxColorsBox(block, mincol, maxcol);	\n"
"	\n"
"    SelectDiagonal(block, mincol, maxcol);	\n"
"	\n"
"    InsetBBox(mincol, maxcol);	\n"
"	\n"
"    uint4 output;	\n"
"    output.x = EmitEndPointsDXT1(mincol, maxcol);	\n"
"    output.w = EmitIndicesDXT1(block, mincol, maxcol);	\n"
"	\n"
"    return output;	\n"
"}	\n"
"   \n"
"uint4 compress_DXT1_BGRA_fp(float2 texcoord : TEXCOORD0,	\n"
"                      uniform sampler2D image,	\n"
"                      uniform float2 imageSize = { 512.0, 512.0 }	\n"
"                      ) : COLOR	\n"
"{	\n"
"    // read block	\n"
"    float3 block[16];	\n"
"    ExtractColorBlockBGR(block, image, texcoord, imageSize);	\n"
"	\n"
"    // find min and max colors	\n"
"    float3 mincol, maxcol;	\n"
"    FindMinMaxColorsBox(block, mincol, maxcol);	\n"
"	\n"
"    SelectDiagonal(block, mincol, maxcol);	\n"
"	\n"
"    InsetBBox(mincol, maxcol);	\n"
"	\n"
"    uint4 output;	\n"
"    output.x = EmitEndPointsDXT1(mincol, maxcol);	\n"
"    output.w = EmitIndicesDXT1(block, mincol, maxcol);	\n"
"	\n"
"   return output;	\n"
"}	\n"
"	\n"
"	\n"
"// compress a 4x4 block to YCoCg DXT5 format	\n"
"// integer version, renders to 4 x int32 buffer	\n"
"uint4 compress_YCoCgDXT5_RGBA_fp(float2 texcoord : TEXCOORD0,	\n"
"                      uniform sampler2D image,	\n"
"                      uniform float2 imageSize = { 512.0, 512.0 }	\n"
"                      ) : COLOR	\n"
"{	\n"
"    //imageSize = tex2Dsize(image, texcoord);	\n"
"	\n"
"    // read block	\n"
"    float3 block[16];	\n"
"    ExtractColorBlockYCoCg_RGB(block, image, texcoord, imageSize);	\n"
"	\n"
"    // find min and max colors	\n"
"    float3 mincol, maxcol;	\n"
"    FindMinMaxColorsBox(block, mincol, maxcol);	\n"
"	\n"
"    SelectYCoCgDiagonal(block, mincol.yz, maxcol.yz);	\n"
"	\n"
"    int scale = ScaleYCoCg(mincol.yz, maxcol.yz);	\n"
"	\n"
"    // Output CoCg in DXT1 block.	\n"
"    uint4 output;	\n"
"    output.z = EmitEndPointsYCoCgDXT5(mincol.yz, maxcol.yz, scale);	\n"
"    output.w = EmitIndicesYCoCgDXT5(block, mincol.yz, maxcol.yz);	\n"
"	\n"
"    InsetYBBox(mincol.x, maxcol.x);	\n"
"	\n"
"    // Output Y in DXT5 alpha block.	\n"
"    output.x = EmitAlphaEndPointsYCoCgDXT5(mincol.x, maxcol.x);	\n"
"	\n"
"    uint2 indices = EmitAlphaIndicesYCoCgDXT5(block, mincol.x, maxcol.x);	\n"
"    output.x |= indices.x;	\n"
"    output.y = indices.y;	\n"
"	\n"
"    return output;	\n"
"}	\n"
"	\n"
"uint4 compress_YCoCgDXT5_BGRA_fp(float2 texcoord : TEXCOORD0,	\n"
"                      uniform sampler2D image,	\n"
"                      uniform float2 imageSize = { 512.0, 512.0 }	\n"
"                      ) : COLOR	\n"
"{	\n"
"    //imageSize = tex2Dsize(image, texcoord);	\n"
"	\n"
"    // read block	\n"
"    float3 block[16];	\n"
"    ExtractColorBlockYCoCg_BGR(block, image, texcoord, imageSize);	\n"
"	\n"
"    // find min and max colors	\n"
"    float3 mincol, maxcol;	\n"
"    FindMinMaxColorsBox(block, mincol, maxcol);	\n"
"	\n"
"    SelectYCoCgDiagonal(block, mincol.yz, maxcol.yz);	\n"
"	\n"
"    int scale = ScaleYCoCg(mincol.yz, maxcol.yz);	\n"
"	\n"
"    // Output CoCg in DXT1 block.	\n"
"    uint4 output;	\n"
"    output.z = EmitEndPointsYCoCgDXT5(mincol.yz, maxcol.yz, scale);	\n"
"    output.w = EmitIndicesYCoCgDXT5(block, mincol.yz, maxcol.yz);	\n"
"	\n"
"    InsetYBBox(mincol.x, maxcol.x);	\n"
"	\n"
"    // Output Y in DXT5 alpha block.	\n"
"    output.x = EmitAlphaEndPointsYCoCgDXT5(mincol.x, maxcol.x);	\n"
"	\n"
"    uint2 indices = EmitAlphaIndicesYCoCgDXT5(block, mincol.x, maxcol.x);	\n"
"    output.x |= indices.x;	\n"
"    output.y = indices.y;	\n"
"	\n"
"    return output;	\n"
"}	\n"
"	\n"
"uniform bool reconstructColor = false;	\n"
"uniform bool displayError = false;	\n"
"uniform float errorScale = 4.0f;	\n"
"	\n"
"uniform sampler2D image : TEXUNIT0;	\n"
"uniform sampler2D originalImage : TEXUNIT1;	\n"
"	\n"
"float4 display_fp(float2 texcoord : TEXCOORD0) : COLOR	\n"
"{	\n"
"    float4 rgba = tex2D(image, texcoord);	\n"
"    	\n"
"    if (reconstructColor)	\n"
"    {	\n"
"        float Y = rgba.a;	\n"
"        float scale = 1.0 / ((255.0 / 8.0) * rgba.b + 1);	\n"
"        float Co = (rgba.r - offset) * scale;	\n"
"        float Cg = (rgba.g - offset) * scale;	\n"
"	\n"
"        float R = Y + Co - Cg;	\n"
"        float G = Y + Cg;	\n"
"        float B = Y - Co - Cg;	\n"
"	\n"
"        rgba = float4(R, G, B, 1);	\n"
"    }	\n"
"	\n"
"    if (displayError)	\n"
"    {	\n"
"        float3 originalColor = tex2D(originalImage, texcoord).rgb;	\n"
"        float3 diff = abs(rgba.rgb - originalColor) * errorScale;	\n"
"        return float4(diff, 1);	\n"
"    }	\n"
"    else	\n"
"    {	\n"
"        return rgba;	\n"
"    }	\n"
"}	\n"
"	\n";