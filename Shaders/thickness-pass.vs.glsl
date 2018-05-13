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
// vertex shader, thickness pass
#version 410 core
#define DEPTH_BIAS 0.0001

layout(std140) uniform CameraData
{
    mat4 viewMatrix;
    mat4 projectionMatrix;
    mat4 invViewMatrix;
    mat4 invProjectionMatrix;
    mat4 shadowMatrix;
    vec4 camPosition;
};

uniform float     u_PointRadius;
uniform float     u_PointScale;
uniform int       u_HasSolid;
uniform sampler2D u_SolidDepthMap;
in vec3           v_Position;
flat out int      f_InShadow;

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void main()
{
    vec4  eyeCoord = viewMatrix * vec4(vec3(v_Position), 1.0);
    vec3  posEye   = vec3(eyeCoord);
    float dist     = length(posEye);

    /////////////////////////////////////////////////////////////////
    // output
    gl_PointSize = u_PointRadius * (u_PointScale / dist) * 4.0f;
    vec4 NDCoord = projectionMatrix * eyeCoord;
    gl_Position = NDCoord;

    f_InShadow = 0;
    if(u_HasSolid == 1) {
        vec2  prjCoords = (NDCoord.xy / NDCoord.w) * 0.5f + 0.5f;
        float pcfDepth  = texture(u_SolidDepthMap, prjCoords).r;
        f_InShadow = (eyeCoord.z < pcfDepth + DEPTH_BIAS) ? 1 : 0;
    }
}
