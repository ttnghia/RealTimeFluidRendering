#version 410 core
//------------------------------------------------------------------------------------------
// vertex shader, mesh shading
//------------------------------------------------------------------------------------------

layout(std140) uniform ModelMatrix
{
    mat4 modelMatrix;
    mat4 normalMatrix;
};

#define NUM_TOTAL_LIGHTS 8
struct LightMatrix
{
    mat4 viewMatrix;
    mat4 prjMatrix;
};

layout(std140) uniform LightMatrices
{
    LightMatrix lightMatrices[NUM_TOTAL_LIGHTS];
};

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

uniform int u_HasShadow;

layout(location = 0) in vec3 v_Position;
layout(location = 1) in vec3 v_Normal;
layout(location = 2) in vec2 v_TexCoord;
out VS_OUT
{
    float f_LightDistance[NUM_TOTAL_LIGHTS];
    vec4  f_FragPosLightSpace[NUM_TOTAL_LIGHTS];
    vec3  f_Normal;
    vec3  f_FragPos;
    vec3  f_ViewDir;
    vec2  f_TexCoord;
};

void main()
{
    f_Normal = vec3(normalMatrix * vec4(v_Normal, 1.0));
    vec4 worldCoord = modelMatrix * vec4(v_Position, 1.0);
    f_FragPos  = vec3(worldCoord);
    f_ViewDir  = vec3(camPosition) - vec3(worldCoord);
    f_TexCoord = v_TexCoord;
    if(u_HasShadow == 1)
    {
        for(int i = 0; i < u_NumLights; ++i)
        {
            vec4 lightCoord = lightMatrices[i].viewMatrix * worldCoord;
            f_LightDistance[i]     = lightCoord.z;
            f_FragPosLightSpace[i] = lightMatrices[i].prjMatrix * lightCoord;
        }
    }
    gl_Position = projectionMatrix * viewMatrix * worldCoord;
}