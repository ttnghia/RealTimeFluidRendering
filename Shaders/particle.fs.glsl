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
// fragment shader, particle render
#version 410 core

#define COLOR_MODE_UNIFORM_MATERIAL 0
#define COLOR_MODE_RANDOM           1
#define COLOR_MODE_RAMP             2

#define NUM_TOTAL_LIGHTS            2
struct PointLight
{
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    vec4 position;
};

layout(std140) uniform Lights
{
    PointLight lights[NUM_TOTAL_LIGHTS];
    int        u_NumLights;
};

layout(std140) uniform CameraData
{
    mat4 viewMatrix;
    mat4 projectionMatrix;
    mat4 invViewMatrix;
    mat4 invProjectionMatrix;
    mat4 shadowMatrix;
    vec4 camPosition;
};

layout(std140) uniform Material
{
    vec4  ambient;
    vec4  diffuse;
    vec4  specular;
    float shininess;
} material;

uniform int   u_ColorMode;
uniform float u_PointRadius;
uniform int   u_UseAnisotropyKernel;
uniform int   u_ScreenWidth;
uniform int   u_ScreenHeight;

in vec3      f_ViewCenter;
in vec3      f_CenterPos;
in vec3      f_Color;
flat in mat3 f_AnisotropyMatrix;

out vec4 outColor;

//------------------------------------------------------------------------------------------
vec3 shadeLight(int lightID, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    //    vec3 lightDir = normalize(vec3(/*viewMatrix */ lights[lightID].position) - fragPos);
    vec3 lightDir     = normalize(vec3(viewMatrix * lights[lightID].position) - fragPos);
    vec3 halfDir      = normalize(lightDir - viewDir);
    vec4 surfaceColor = (u_ColorMode == COLOR_MODE_UNIFORM_MATERIAL) ? material.diffuse : vec4(f_Color, 1.0);

    vec4 ambientColor  = lights[lightID].ambient * material.ambient;
    vec4 diffuseColor  = lights[lightID].diffuse * vec4(max(dot(normal, lightDir), 0.0)) * surfaceColor;
    vec4 specularColor = lights[lightID].specular * pow(max(dot(halfDir, normal), 0.0), material.shininess) * material.specular;

    return vec3(ambientColor + diffuseColor + specularColor);
}

//------------------------------------------------------------------------------------------
void main()
{
    vec3 viewDir = normalize(f_ViewCenter);
    vec3 normal;
    vec3 fragPos;

    if(u_UseAnisotropyKernel == 0) {
        normal.xy = gl_PointCoord.xy * vec2(2.0, -2.0) + vec2(-1.0, 1.0);
        float mag = dot(normal.xy, normal.xy);

        if(mag > 1.0) {
            discard;           // kill pixels outside circle
        }
        normal.z = sqrt(1.0 - mag);
        fragPos  = f_ViewCenter + normal * u_PointRadius;
    } else {
        vec3 fc = gl_FragCoord.xyz;
        fc.xy /= vec2(u_ScreenWidth, u_ScreenHeight);
        fc    *= 2.0;
        fc    -= 1.0;

        vec4 worldPos = invProjectionMatrix * vec4(fc, 1.0);
        vec3 rayDir   = vec3(worldPos) / worldPos.w;

        mat3 transMatrix    = mat3(viewMatrix) * f_AnisotropyMatrix * u_PointRadius;
        mat3 transInvMatrix = inverse(transMatrix);
        mat3 normalMatrix   = transpose(inverse((transMatrix)));

        vec3 camT    = transInvMatrix * vec3(0, 0, 0);
        vec3 centerT = transInvMatrix * f_ViewCenter;
        vec3 rayDirT = normalize(transInvMatrix * rayDir);

        // solve the ray-sphere intersection
        float tmp   = dot(rayDirT, camT - centerT);
        float delta = tmp * tmp - dot(camT - centerT, camT - centerT) + 1.0;
        if(delta < 0.0) {
            discard;             // kill pixels outside circle in parameter space
        }
        float d                  = -tmp - sqrt(delta);
        vec3  intersectionPointT = camT + rayDirT * d;
        vec3  normalT            = normalize(intersectionPointT - centerT);

        fragPos = transMatrix * intersectionPointT;;
        normal  = normalize(normalMatrix * normalT);
    }

    // correct depth
    float z = dot(vec4(fragPos, 1.0), transpose(projectionMatrix)[2]);
    float w = dot(vec4(fragPos, 1.0), transpose(projectionMatrix)[3]);
    gl_FragDepth = 0.5 * (z / w + 1.0);

    vec3 shadeColor = vec3(0, 0, 0);
    for(int i = 0; i < u_NumLights; ++i) {
        shadeColor += shadeLight(i, normal, fragPos, viewDir);
    }

    outColor = vec4(shadeColor, 1.0);
}
