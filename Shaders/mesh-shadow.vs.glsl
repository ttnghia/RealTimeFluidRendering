#version 410 core
//------------------------------------------------------------------------------------------
// vertex, mesh-depth

//------------------------------------------------------------------------------------------
// uniforms
layout(std140) uniform Matrices
{
    mat4 modelMatrix;
    mat4 normalMatrix;
    mat4 viewMatrix;
    mat4 projectionMatrix;
    mat4 invProjMatrix;
    mat4 shadowMatrix;
};


//------------------------------------------------------------------------------------------
// in variables
in vec3 v_Position;

out vec3 f_eyeCoord;
//------------------------------------------------------------------------------------------
void main()
{
    vec4 eyeCoord = viewMatrix * /*modelMatrix **/ vec4(vec3(v_Position), 1.0);
    f_eyeCoord = eyeCoord.xyz;

    gl_Position = projectionMatrix * eyeCoord;
}
