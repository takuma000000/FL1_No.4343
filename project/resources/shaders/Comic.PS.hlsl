#include "CopyImage.hlsli"
#include "ComicParameters.hlsli"
Texture2D<float4> gScene : register(t0);
Texture2D<float> gDepth : register(t1);
SamplerState gSampler : register(s0);
ConstantBuffer<ComicParameters> gComic : register(b0);

float ViewDepth(int2 pixel, uint2 dimensions)
{
    pixel = clamp(pixel, int2(0, 0), int2(dimensions) - 1);
    const float depth = gDepth.Load(int3(pixel, 0));
    const float2 uv = (float2(pixel) + 0.5f) / float2(dimensions);
    const float4 viewPosition = mul(float4(uv.x * 2.0f - 1.0f, 1.0f - uv.y * 2.0f, depth, 1.0f), gComic.projectionInverse);
    return viewPosition.z / max(abs(viewPosition.w), 0.000001f);
}

float4 main(VertexShaderOutput input) : SV_TARGET0
{
    uint width, height;
    gDepth.GetDimensions(width, height);
    const uint2 dimensions = uint2(width, height);
    const int2 pixel = int2(input.position.xy);
    const int stepSize = max(1, (int)round(gComic.lineWidth));
    const float center = rcp(max(ViewDepth(pixel, dimensions), 0.01f));
    float edge = 0.0f;
    // 平面では逆深度が画面上で線形になる。二階差分で面の傾きを線にせず境界を拾う。
    const int2 offsets[4] = { int2(1, 0), int2(0, 1), int2(1, 1), int2(1, -1) };
    [unroll]
    for (int i = 0; i < 4; ++i)
    {
        const int2 offset = offsets[i] * stepSize;
        const float nearDepth = rcp(max(ViewDepth(pixel - offset, dimensions), 0.01f));
        const float farDepth = rcp(max(ViewDepth(pixel + offset, dimensions), 0.01f));
        const float relativeChange = abs(nearDepth + farDepth - 2.0f * center) / max(center, 0.000001f);
        edge = max(edge, relativeChange);
    }
    const float threshold = max(gComic.depthThreshold, 0.0001f);
    const float outline = smoothstep(threshold * 0.5f, threshold, edge)
        * saturate(gComic.outlineStrength * gComic.lineColor.a);
    float4 color = gScene.Sample(gSampler, input.texcoord);
    color.rgb = lerp(color.rgb, gComic.lineColor.rgb, outline);
    return color;
}
