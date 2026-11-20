#ifndef TKMAX_COMIC_PARAMETERS
#define TKMAX_COMIC_PARAMETERS
// C++ DirectXCommon::ComicParametersと同じ配置（112バイト、行ベクトル）。
struct ComicParameters
{
    float4x4 projectionInverse;
    float4 lineColor;
    float lineWidth;
    float depthThreshold;
    int shadeSteps;
    float toonBlend;
    uint toonEnabled;
    float outlineStrength;
    float2 padding;
};
#endif
