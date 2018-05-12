#version 410 core
//------------------------------------------------------------------------------------------
// fragment shader, new filter pass
//------------------------------------------------------------------------------------------

#define CEM_OPTIMIZATIONS
#define FIX_ARTIFACTS_ON_FLAT_AREAS

layout(std140) uniform Matrices
{
    mat4 modelMatrix;
    mat4 normalMatrix;
    mat4 viewMatrix;
    mat4 projectionMatrix;
    mat4 invProjMatrix;
};

uniform sampler2D u_DepthTex;

uniform int direction;

// do_filter1D = 1, 0, and -1 (-1 mean filter2D with fixed radius)
uniform int do_filter1D;
uniform float pradius;
uniform float basicFilterSize;
//------------------------------------------------------------------------------------------
in vec2 f_TexCoord;
out float outDepth;

const float screenWidth = 1920.0;
const float screenHeight = 1080.0;
const vec2 blurRadius = vec2(1.0 / screenWidth, 1.0 / screenHeight);

const int maxFilterSize = 50;
const int fixedFilterRadius = 3;

const float ratio = 1024.0 / 2.0 / tan(45.0 / 2.0);
//------------------------------------------------------------------------------------------
float compute_weight(vec2 p, float two_sigma2)
{
    vec2 r = blurRadius * p;
    float r2 = r[0]*r[0] + r[1]*r[1];
    return exp(-r2 / two_sigma2);
}
//------------------------------------------------------------------------------------------
float compute_weight1D(float r, float two_sigma2)
{
    return exp(-r*r / two_sigma2);
}
//------------------------------------------------------------------------------------------
void ModifyFilter( inout float sampleDepth, inout float weight, float upper, float lower, float lower_clamp )
{
    if(sampleDepth > upper) weight = 0;
    else if (sampleDepth < lower) sampleDepth = lower_clamp;
}
//------------------------------------------------------------------------------------------
void ModifyFilter1D( inout float sampleDepth, inout float weight, inout float upper, inout float lower, float lower_clamp, float threshold )
{
    if(sampleDepth > upper) weight = 0;
    else {
        if (sampleDepth < lower) sampleDepth = lower_clamp;
#ifdef FIX_ARTIFACTS_ON_FLAT_AREAS
        upper = sampleDepth + threshold;
        lower = sampleDepth - threshold;
#endif
    }
}
//------------------------------------------------------------------------------------------
float filter1D(float pixelDepth)
{
    float threshold = pradius * 5.0;
    float K = -basicFilterSize * ratio * pradius;

    float sigma = threshold / 2.0f;    // 3sigma law: 3*sigma will cover the filter radius
    float two_sigma2 = 2.0f * sigma*sigma;

    float upper = pixelDepth + threshold;
    float lower = pixelDepth - threshold;
    float lower_clamp = pixelDepth - pradius;

    int filterSize = int(ceil(K / pixelDepth)); //min(maxFilterSize, int(ceil(K / pixelDepth)));

#ifndef CEM_OPTIMIZATIONS

    float sum = 0;
    float wsum = 0;
    for(int x = -filterSize; x <= filterSize; ++x)
    {
        vec2 p = (direction == 0) ? vec2(x, 0) : vec2(0, x);

        vec2 samplep = p*blurRadius + f_TexCoord;
        float sampleDepth = texture(u_DepthTex, samplep).r;

        if(sampleDepth > upper) continue;
        sampleDepth = (sampleDepth < lower) ? lower_clamp : sampleDepth;

        float w = compute_weight(p, two_sigma2);
        sum += sampleDepth * w;
        wsum += w;

    }
    return (wsum > 0)? sum/wsum: pixelDepth;

#else 

    vec2 sum2 = vec2(pixelDepth,0);
    vec2 wsum2 = vec2(1,0);
    vec4 dtc = (direction == 0) ? vec4(blurRadius.x, 0, -blurRadius.x, 0) : vec4(0, blurRadius.y, 0, -blurRadius.y);

    //for(int x = -filterSize; x <= filterSize; ++x)
    vec4 f_tex = f_TexCoord.xyxy;
    float r = 0;
    float dr = dtc.x + dtc.y;

    float upper1 = upper;
    float upper2 = upper;
    float lower1 = lower;
    float lower2 = lower;

    for(int x = 1; x <= filterSize; ++x)
    {
        f_tex += dtc;
        vec2 sampleDepth;
        sampleDepth.x = texture(u_DepthTex, f_tex.xy).r;
        sampleDepth.y = texture(u_DepthTex, f_tex.zw).r;

        r += dr;
        float w = compute_weight1D(r, two_sigma2);
        vec2 w2 = vec2(w,w);

        ModifyFilter1D( sampleDepth.x, w2.x, upper1, lower1, lower_clamp, threshold );
        ModifyFilter1D( sampleDepth.y, w2.y, upper2, lower2, lower_clamp, threshold );

        sum2 += sampleDepth * w2;
        wsum2 += w2;
    }
    vec2 filterVal = vec2( sum2.x, wsum2.x ) + vec2( sum2.y, wsum2.y );
    return filterVal.x / filterVal.y;

#endif
}

float filter2D(float pixelDepth)
{
    float threshold = pradius * 5.0;
    float K = -basicFilterSize * ratio * pradius;

    float sigma = threshold / 3.0f;
    float two_sigma2 = 2.0f * sigma*sigma;


    float upper = pixelDepth + threshold;
    float lower = pixelDepth - threshold;
    float lower_clamp = pixelDepth - pradius;

    int filterSize = (do_filter1D < 0) ? fixedFilterRadius : min(maxFilterSize, int(K / pixelDepth));

#ifndef CEM_OPTIMIZATIONS

    float sum = 0;
    float wsum = 0;
    for(int x = -filterSize; x <= filterSize; ++x)
    {
        for(int y = -filterSize; y <= filterSize; ++y)
        {
           vec2 p = vec2(x, y);

           vec2 samplep = p*blurRadius + f_TexCoord;
           float sampleDepth = texture(u_DepthTex, samplep).r;

           if(sampleDepth > upper) continue;
           sampleDepth = (sampleDepth < lower) ? lower_clamp : sampleDepth;

           float w = compute_weight(p, two_sigma2);
           sum += sampleDepth * w;
           wsum += w;
       }

    }
    return (wsum > 0)? sum/wsum: pixelDepth;

#else

    vec4 f_tex = f_TexCoord.xyxy;

    vec2 r = vec2(0,0);
    vec4 sum4 = vec4(pixelDepth, 0, 0, 0);
    vec4 wsum4 = vec4(1, 0, 0, 0);
    for(int x = 1; x <= filterSize; ++x)
    {
        r.x += blurRadius.x;
        f_tex.x += blurRadius.x;
        f_tex.z -= blurRadius.x;
        vec4 f_tex1 = f_tex.xyxy;
        vec4 f_tex2 = f_tex.zwzw;
        for(int y = 1; y <= filterSize; ++y)
        {
            f_tex1.y += blurRadius.y;
            f_tex1.w -= blurRadius.y;
            f_tex2.y += blurRadius.y;
            f_tex2.w -= blurRadius.y;
  
            vec4 sampleDepth;
            sampleDepth.x = texture(u_DepthTex, f_tex1.xy).r;
            sampleDepth.y = texture(u_DepthTex, f_tex1.zw).r;
            sampleDepth.z = texture(u_DepthTex, f_tex2.xy).r;
            sampleDepth.w = texture(u_DepthTex, f_tex2.zw).r;

            r.y += blurRadius.y;
            float w = compute_weight(r, two_sigma2);
            vec4 w4 = vec4(w,w,w,w);

            ModifyFilter( sampleDepth.x, w4.x, upper, lower, lower_clamp );
            ModifyFilter( sampleDepth.y, w4.y, upper, lower, lower_clamp );
            ModifyFilter( sampleDepth.z, w4.z, upper, lower, lower_clamp );
            ModifyFilter( sampleDepth.w, w4.w, upper, lower, lower_clamp );

            sum4 += sampleDepth * w4;
            wsum4 += w4;
       }

    }
    vec2 filterVal;
    filterVal.x = dot(sum4,vec4(1,1,1,1));
    filterVal.y = dot(wsum4,vec4(1,1,1,1));
    return filterVal.x / filterVal.y;

#endif
}
//------------------------------------------------------------------------------------------
void main()
{
    float pixelDepth = texture(u_DepthTex, f_TexCoord).r;

    if(pixelDepth > 0.0 || pixelDepth < -1000.0f)
    {
        outDepth = pixelDepth;
    }
    else
    {
        outDepth = (do_filter1D == 1)? filter1D(pixelDepth):
                                        filter2D(pixelDepth);
    }
}

