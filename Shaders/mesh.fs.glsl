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
// fragment shader, mesh shading
#version 410 core

#define NUM_TOTAL_LIGHTS 8
//#define SHADOW_BIAS      -0.15 // for fluid emitter + sphere mesh
#define SHADOW_BIAS      -0.05

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

layout(std140) uniform Material
{
    vec4  ambient;
    vec4  diffuse;
    vec4  specular;
    float shininess;
} material;

uniform int       u_HasTexture;
uniform int       u_HasShadow;
uniform int       u_VisualizeShadowRegion;
uniform float     u_ShadowIntensity;
uniform float     u_Exposure = 1.0f;
uniform sampler2D u_TexSampler;
uniform sampler2D u_SolidShadowMaps[NUM_TOTAL_LIGHTS];
uniform sampler2D u_FluidShadowMaps[NUM_TOTAL_LIGHTS];
uniform sampler2D u_FluidShadowThickness[NUM_TOTAL_LIGHTS];

in VS_OUT
{
    float f_LightDistance[NUM_TOTAL_LIGHTS];
    vec4  f_FragPosLightSpace[NUM_TOTAL_LIGHTS];
    vec3  f_Normal;
    vec3  f_FragPos;
    vec3  f_ViewDir;
    vec2  f_TexCoord;
};
out vec4 outColor;

#define RENDER_BLUE

#ifdef RENDER_BLUE
const float k_r = 0.5f;
const float k_g = 0.2f;
const float k_b = 0.05f;
#else
const float k_r = 0.8f;
const float k_g = 0.2f;
const float k_b = 0.9f;
#endif

vec3 computeAttennuation(float thickness)
{
    return vec3(exp(-k_r * thickness), exp(-k_g * thickness), exp(-k_b * thickness));
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
vec3 shadeLight(int lightID, vec3 normal, vec3 fragPos, vec3 viewDir, vec2 f_TexCoord)
{
    vec3 lightDir = normalize(vec3(lights[lightID].position) - fragPos);
    vec3 halfDir  = normalize(lightDir + viewDir);

    vec4 ambientColor  = lights[lightID].ambient;
    vec4 diffuseColor  = lights[lightID].diffuse * vec4(max(dot(normal, lightDir), 0.0));;
    vec4 specularColor = lights[lightID].specular * pow(max(dot(halfDir, normal), 0.0), material.shininess);
    if(u_HasTexture == 1) {
        vec4 objColor = texture(u_TexSampler, f_TexCoord);
        ambientColor  *= (objColor * 0.2);
        diffuseColor  *= objColor;
        specularColor *= objColor;
    } else {
        ambientColor  *= material.ambient;
        diffuseColor  *= material.diffuse;
        specularColor *= material.specular;
    }

    if(u_HasShadow == 1) {
        float inSolidShadow = 0.0;
        vec3  projCoords    = f_FragPosLightSpace[lightID].xyz / f_FragPosLightSpace[lightID].w;
        projCoords = projCoords * 0.5 + 0.5;         // Transform to [0,1] range

        vec2 shadowTexCoord = clamp(projCoords.xy, 0.0, 1.0);
        vec2 texelSize      = 1.0 / textureSize(u_SolidShadowMaps[lightID], 0);
        for(int x = -1; x <= 1; ++x) {
            for(int y = -1; y <= 1; ++y) {
                float sampleSolidDepth = texture(u_SolidShadowMaps[lightID], shadowTexCoord + vec2(x, y) * texelSize).r;
                inSolidShadow += (f_LightDistance[lightID] < sampleSolidDepth + SHADOW_BIAS) ? 1.0 : 0.0;
            }
        }
        inSolidShadow /= 9.0;

        float sampleFluidDepth     = texture(u_FluidShadowMaps[lightID], shadowTexCoord).r;
        float inFluidShadow        = (f_LightDistance[lightID] < sampleFluidDepth + SHADOW_BIAS) ? 1.0 : 0.0;
        float fluidShadowThickness = texture(u_FluidShadowThickness[lightID], shadowTexCoord).r;
        vec3  fluidShadowColor     = inFluidShadow > 0 ? computeAttennuation(u_ShadowIntensity * fluidShadowThickness) : vec3(1.0);

        vec3 shadowColor = (1.0 - u_ShadowIntensity * inSolidShadow) * fluidShadowColor;
        vec3 shadeColor  = vec3(ambientColor) + shadowColor * vec3(diffuseColor + specularColor);

        if((u_VisualizeShadowRegion == 1) && (projCoords.x < 0.0 || projCoords.x > 1.0 || projCoords.y < 0.0 || projCoords.y > 1.0)) {
            shadeColor *= vec3(0.1, 0.1, 1.0);
        }

        return shadeColor;
    } else {
        return vec3(ambientColor + diffuseColor + specularColor);
    }
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void main()
{
    vec3 normal     = normalize(f_Normal);
    vec3 viewDir    = normalize(f_ViewDir);
    vec3 shadeColor = vec3(0, 0, 0);

    for(int i = 0; i < u_NumLights; ++i) {
        shadeColor += shadeLight(i, normal, f_FragPos, viewDir, f_TexCoord);
    }

    outColor = vec4(u_Exposure * shadeColor, 1.0);
}
