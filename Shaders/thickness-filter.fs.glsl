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
// fragment shader, thickness filter
#version 410 core

uniform sampler2D u_ThicknessTex;
uniform int       u_ScreenWidth;
uniform int       u_ScreenHeight;
uniform int       u_FilterSize;

in vec2   f_TexCoord;
out float outThickness;

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void main()
{
    vec2  blurRadius = vec2(1.0 / float(u_ScreenWidth), 1.0 / float(u_ScreenHeight));
    float sum        = 0;
    float wsum       = 0;
    float sigma      = float(u_FilterSize) / 3.0;
    float two_sigma2 = 2.0 * sigma * sigma;

    for(int x = -u_FilterSize; x <= u_FilterSize; x++) {
        for(int y = -u_FilterSize; y <= u_FilterSize; y++) {
            vec2  samplep         = vec2(x, y) * blurRadius + f_TexCoord;
            float sampleThickness = texture2D(u_ThicknessTex, samplep).r;
            float r               = length(vec2(x, y));
            float w               = exp(-r * r / two_sigma2);

            sum  += sampleThickness * w;
            wsum += w;
        }
    }

    outThickness = sum / wsum;
}
