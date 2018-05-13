//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//                                .--,       .--,
//                               ( (  \.---./  ) )
//                                '.__/o   o\__.'
//                                   {=  ^  =}
//                                    >  -  <
//     ___________________________.""`-------`"".____________________________
//    /                                                                      \
//    \    This file is part of Banana - a graphics programming framework    /
//    /                    Created: 2018 by Nghia Truong                     \
//    \                      <nghiatruong.vn@gmail.com>                      /
//    /                      https://ttnghia.github.io                       \
//    \                        All rights reserved.                          /
//    /                                                                      \
//    \______________________________________________________________________/
//                                  ___)( )(___
//                                 (((__) (__)))
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// fragment shader, bilateral gaussian pass

#version 410 core

uniform sampler2D u_DepthTex;
uniform float     u_ParticleRadius;
uniform int       u_FilterSize;
uniform int       u_MaxFilterSize;

uniform int u_ScreenWidth;
uniform int u_ScreenHeight;

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
in vec2   f_TexCoord;
out float outDepth;

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
const float thresholdRatio = 10.0;
float compute_weight2D(vec2 r, float two_sigma2)
{
    return exp(-dot(r, r) / two_sigma2);
}

float compute_weight1D(float r, float two_sigma2)
{
    return exp(-r * r / two_sigma2);
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void main()
{
    vec2 blurRadius = vec2(1.0 / float(u_ScreenWidth), 1.0 / float(u_ScreenHeight));

    float pixelDepth = texture(u_DepthTex, f_TexCoord).r;
    float finalDepth;

    if(pixelDepth >= 0.0f || pixelDepth < -1000.0f) {
        finalDepth = pixelDepth;
    } else {
        float ratio      = u_ScreenHeight / 2.0 / tan(45.0 / 2.0);
        float K          = -u_FilterSize * ratio * u_ParticleRadius * 0.1f;
        int   filterSize = min(u_MaxFilterSize, int(ceil(K / pixelDepth)));
        float sigma      = filterSize / 3.0f;
        float two_sigma2 = 2.0f * sigma * sigma;

        float threshold       = u_ParticleRadius * thresholdRatio;
        float sigmaDepth      = threshold / 3.0f;
        float two_sigmaDepth2 = 2.0f * sigmaDepth * sigmaDepth;

        vec4 f_tex = f_TexCoord.xyxy;
        vec2 r     = vec2(0, 0);
        vec4 sum4  = vec4(pixelDepth, 0, 0, 0);
        vec4 wsum4 = vec4(1, 0, 0, 0);
        vec4 sampleDepth;
        vec4 w4_r;
        vec4 w4_depth;
        vec4 rDepth;

        for(int x = 1; x <= filterSize; ++x) {
            r.x     += blurRadius.x;
            f_tex.x += blurRadius.x;
            f_tex.z -= blurRadius.x;
            vec4 f_tex1 = f_tex.xyxy;
            vec4 f_tex2 = f_tex.zwzw;

            for(int y = 1; y <= filterSize; ++y) {
                r.y += blurRadius.y;

                f_tex1.y += blurRadius.y;
                f_tex1.w -= blurRadius.y;
                f_tex2.y += blurRadius.y;
                f_tex2.w -= blurRadius.y;

                sampleDepth.x = texture(u_DepthTex, f_tex1.xy).r;
                sampleDepth.y = texture(u_DepthTex, f_tex1.zw).r;
                sampleDepth.z = texture(u_DepthTex, f_tex2.xy).r;
                sampleDepth.w = texture(u_DepthTex, f_tex2.zw).r;

                rDepth     = sampleDepth - vec4(pixelDepth);
                w4_r       = vec4(compute_weight2D(blurRadius * r, two_sigma2));
                w4_depth.x = compute_weight1D(rDepth.x, two_sigmaDepth2);
                w4_depth.y = compute_weight1D(rDepth.y, two_sigmaDepth2);
                w4_depth.z = compute_weight1D(rDepth.z, two_sigmaDepth2);
                w4_depth.w = compute_weight1D(rDepth.w, two_sigmaDepth2);

                sum4  += sampleDepth * w4_r * w4_depth;
                wsum4 += w4_r * w4_depth;
            }
        }

        vec2 filterVal;
        filterVal.x = dot(sum4, vec4(1, 1, 1, 1));
        filterVal.y = dot(wsum4, vec4(1, 1, 1, 1));

        finalDepth = filterVal.x / filterVal.y;
    }

    outDepth = finalDepth;
}
