// vertex shader, fluid thickness shadow
#version 410 core
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

uniform int   u_LightID;
uniform float u_PointRadius;
uniform float u_PointScale;

in vec3 v_Position;
//------------------------------------------------------------------------------------------
void main()
{
    vec4 lightCoord = lightMatrices[u_LightID].viewMatrix * vec4(v_Position, 1.0);
    gl_Position = lightMatrices[u_LightID].prjMatrix * lightCoord;

    float dist = length(vec3(lightCoord));
    gl_PointSize = u_PointRadius * (u_PointScale / dist) * 4.0f;
}
