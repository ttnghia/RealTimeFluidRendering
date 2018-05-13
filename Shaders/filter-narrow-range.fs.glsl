#version 410 core
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// fragment shader, modified gaussian filter
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

#define FIX_OTHER_WEIGHT
#define RANGE_EXTENSION

#define PI_OVER_8 0.392699082f

layout(std140) uniform CameraData
{
    mat4 viewMatrix;
    mat4 projectionMatrix;
    mat4 invViewMatrix;
    mat4 invProjectionMatrix;
    mat4 shadowMatrix;
    vec4 camPosition;
};

uniform sampler2D u_DepthTex;
uniform float     u_ParticleRadius;
uniform int       u_FilterSize;
uniform int       u_MaxFilterSize;
uniform int       u_ScreenWidth;
uniform int       u_ScreenHeight;

// u_DoFilter1D = 1, 0, and -1 (-1 mean filter2D with fixed radius)
uniform int u_DoFilter1D;
uniform int u_FilterDirection;

in vec2   f_TexCoord;
out float outDepth;

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
const int   fixedFilterRadius = 4;
const float thresholdRatio    = 10.5;
const float clampRatio        = 1;

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
float compute_weight1D(float r, float two_sigma2)
{
    return exp(-r * r / two_sigma2);
}

float compute_weight2D(vec2 r, float two_sigma2)
{
    return exp(-dot(r, r) / two_sigma2);
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void ModifiedGaussianFilter1D(inout float sampleDepth, inout float weight, inout float weight_other, inout float upper, inout float lower, float lower_clamp, float threshold)
{
    if(sampleDepth > upper) {
        weight = 0;
#ifdef FIX_OTHER_WEIGHT
        weight_other = 0;
#endif
    } else {
        if(sampleDepth < lower) {
            sampleDepth = lower_clamp;
        }
#ifdef RANGE_EXTENSION
        else {
            upper = max(upper, sampleDepth + threshold);
            lower = min(lower, sampleDepth - threshold);
        }
#endif
    }
}

void ModifiedGaussianFilter2D(inout float sampleDepth, inout float weight, inout float weight_other, inout float upper, inout float lower, float lower_clamp, float threshold)
{
    if(sampleDepth > upper) {
        weight = 0;
#ifdef FIX_OTHER_WEIGHT
        weight_other = 0;
#endif
    } else {
        if(sampleDepth < lower) {
            sampleDepth = lower_clamp;
        }
#ifdef RANGE_EXTENSION
        else {
            upper = max(upper, sampleDepth + threshold);
            lower = min(lower, sampleDepth - threshold);
        }
#endif
    }
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
float filter1D(float pixelDepth)
{
    if(u_FilterSize == 0) {
        return pixelDepth;
    }

    vec2  blurRadius = vec2(1.0 / u_ScreenWidth, 1.0 / u_ScreenHeight);
    float threshold  = u_ParticleRadius * thresholdRatio;
    float ratio      = u_ScreenHeight / 2.0 / tan(PI_OVER_8);
    float K          = -u_FilterSize * ratio * u_ParticleRadius * 0.1f;
    int   filterSize = min(u_MaxFilterSize, int(ceil(K / pixelDepth)));

    float upper       = pixelDepth + threshold;
    float lower       = pixelDepth - threshold;
    float lower_clamp = pixelDepth - u_ParticleRadius * clampRatio;

    float sigma      = filterSize / 3.0f;
    float two_sigma2 = 2.0f * sigma * sigma;

    vec2 sum2  = vec2(pixelDepth, 0);
    vec2 wsum2 = vec2(1, 0);
    vec4 dtc   = (u_FilterDirection == 0) ? vec4(blurRadius.x, 0, -blurRadius.x, 0) : vec4(0, blurRadius.y, 0, -blurRadius.y);

    vec4  f_tex = f_TexCoord.xyxy;
    float r     = 0;
    float dr    = dtc.x + dtc.y;

    float upper1 = upper;
    float upper2 = upper;
    float lower1 = lower;
    float lower2 = lower;
    vec2  sampleDepth;
    vec2  w2;

    for(int x = 1; x <= filterSize; ++x) {
        f_tex += dtc;
        r     += dr;

        sampleDepth.x = texture(u_DepthTex, f_tex.xy).r;
        sampleDepth.y = texture(u_DepthTex, f_tex.zw).r;

        w2 = vec2(compute_weight1D(r, two_sigma2));
        ModifiedGaussianFilter1D(sampleDepth.x, w2.x, w2.y, upper1, lower1, lower_clamp, threshold);
        ModifiedGaussianFilter1D(sampleDepth.y, w2.y, w2.x, upper2, lower2, lower_clamp, threshold);

        sum2  += sampleDepth * w2;
        wsum2 += w2;
    }

    vec2 filterVal = vec2(sum2.x, wsum2.x) + vec2(sum2.y, wsum2.y);
    return filterVal.x / filterVal.y;
}

float filter2D(float pixelDepth)
{
    if(u_FilterSize == 0) {
        return pixelDepth;
    }

    vec2  blurRadius = vec2(1.0 / u_ScreenWidth, 1.0 / u_ScreenHeight);
    float threshold  = u_ParticleRadius * thresholdRatio;
    float ratio      = u_ScreenHeight / 2.0 / tan(PI_OVER_8);
    float K          = -u_FilterSize * ratio * u_ParticleRadius * 0.1f;
    int   filterSize = (u_DoFilter1D < 0) ? fixedFilterRadius : min(u_MaxFilterSize, int(ceil(K / pixelDepth)));

    float upper       = pixelDepth + threshold;
    float lower       = pixelDepth - threshold;
    float lower_clamp = pixelDepth - u_ParticleRadius * clampRatio;

    float sigma      = filterSize / 3.0f;
    float two_sigma2 = 2.0f * sigma * sigma;

    vec4 f_tex = f_TexCoord.xyxy;

    vec2 r     = vec2(0, 0);
    vec4 sum4  = vec4(pixelDepth, 0, 0, 0);
    vec4 wsum4 = vec4(1, 0, 0, 0);
    vec4 sampleDepth;
    vec4 w4;

    for(int x = 1; x <= filterSize; ++x) {
        r.x     += blurRadius.x;
        f_tex.x += blurRadius.x;
        f_tex.z -= blurRadius.x;
        vec4 f_tex1 = f_tex.xyxy;
        vec4 f_tex2 = f_tex.zwzw;

        for(int y = 1; y <= filterSize; ++y) {
            f_tex1.y += blurRadius.y;
            f_tex1.w -= blurRadius.y;
            f_tex2.y += blurRadius.y;
            f_tex2.w -= blurRadius.y;

            sampleDepth.x = texture(u_DepthTex, f_tex1.xy).r;
            sampleDepth.y = texture(u_DepthTex, f_tex1.zw).r;
            sampleDepth.z = texture(u_DepthTex, f_tex2.xy).r;
            sampleDepth.w = texture(u_DepthTex, f_tex2.zw).r;

            r.y += blurRadius.y;
            w4   = vec4(compute_weight2D(blurRadius * r, two_sigma2));

            ModifiedGaussianFilter2D(sampleDepth.x, w4.x, w4.w, upper, lower, lower_clamp, threshold);
            ModifiedGaussianFilter2D(sampleDepth.y, w4.y, w4.z, upper, lower, lower_clamp, threshold);
            ModifiedGaussianFilter2D(sampleDepth.z, w4.z, w4.y, upper, lower, lower_clamp, threshold);
            ModifiedGaussianFilter2D(sampleDepth.w, w4.w, w4.x, upper, lower, lower_clamp, threshold);

            sum4  += sampleDepth * w4;
            wsum4 += w4;
        }
    }

    vec2 filterVal;
    filterVal.x = dot(sum4, vec4(1, 1, 1, 1));
    filterVal.y = dot(wsum4, vec4(1, 1, 1, 1));
    return filterVal.x / filterVal.y;
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void main()
{
    float pixelDepth = texture(u_DepthTex, f_TexCoord).r;

    if(pixelDepth > 0.0 || pixelDepth < -1000.0f) {
        outDepth = pixelDepth;
    } else {
        outDepth = (u_DoFilter1D == 1) ? filter1D(pixelDepth) : filter2D(pixelDepth);
    }
}
