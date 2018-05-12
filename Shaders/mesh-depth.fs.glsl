#version 410 core
//------------------------------------------------------------------------------------------
// fragment, mesh-depth

layout(std140) uniform Matrices
{
    mat4 modelMatrix;
    mat4 normalMatrix;
    mat4 viewMatrix;
    mat4 projectionMatrix;
    mat4 invProjMatrix;
    mat4 shadowMatrix;
};

in vec3 f_eyeCoord;
out float outDepth;

//------------------------------------------------------------------------------------------
void main()
{
    outDepth = 1000.0f; // positive: in front of the camera

    vec4 pixelPos = vec4(f_eyeCoord, 1.0f);
    vec4 clipSpacePos = projectionMatrix * pixelPos;
    gl_FragDepth = clipSpacePos.z / clipSpacePos.w;
}
