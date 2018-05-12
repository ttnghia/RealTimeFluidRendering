// fragment shader, curvature flow pass
#version 410 core

layout(std140) uniform CameraData
{
    mat4 viewMatrix;
    mat4 projectionMatrix;
    mat4 invViewMatrix;
    mat4 invProjectionMatrix;
    mat4 shadowMatrix;
    vec4 camPosition;
};

uniform sampler2D u_DepthTex;
uniform float     u_ParticleRadius;
uniform int       u_ScreenWidth;
uniform int       u_ScreenHeight;

in vec2           f_TexCoord;
out float         outDepth;

const float thresholdRatio    = 5.0;
//------------------------------------------------------------------------------------------
vec3 meanCurvature(vec2 pos)
{
    // Width of one pixel
    vec2 dx = vec2(1.0f / u_ScreenWidth, 0.0f);
    vec2 dy = vec2(0.0f, 1.0f / u_ScreenHeight);

    // Central z value
    float zc = texture(u_DepthTex, pos).r - 1.0f;

    float zdxp = texture(u_DepthTex, pos + dx).r - 1.0f;
    float zdxn = texture(u_DepthTex, pos - dx).r - 1.0f;

    float zdx = 0.5f * (zdxp - zdxn);

    float zdyp = texture(u_DepthTex, pos + dy).r - 1.0f;
    float zdyn = texture(u_DepthTex, pos - dy).r - 1.0f;

    float zdy = 0.5f * (zdyp - zdyn);

    // Take second order finite differences
    float zdx2 = zdxp + zdxn - 2.0f * zc;
    float zdy2 = zdyp + zdyn - 2.0f * zc;


    // Second order finite differences, alternating variables
    float zdxpyp = texture(u_DepthTex, pos + dx + dy).r - 1.0f;
    float zdxnyn = texture(u_DepthTex, pos - dx - dy).r - 1.0f;
    float zdxpyn = texture(u_DepthTex, pos + dx - dy).r - 1.0f;
    float zdxnyp = texture(u_DepthTex, pos - dx + dy).r - 1.0f;

    float zdxy = (zdxpyp + zdxnyn - zdxpyn - zdxnyp) / 4.0f;

    if(abs(zdx) > u_ParticleRadius * thresholdRatio)
    {
        zdx  = 0.0f;
        zdx2 = 0.0f;
    }

    if(abs(zdy) > u_ParticleRadius * thresholdRatio)
    {
        zdy  = 0.0f;
        zdy2 = 0.0f;
    }

    if(abs(zdxy) > u_ParticleRadius * thresholdRatio)
    {
        zdxy = 0.0f;
    }

    // Projection transform inversion terms
    float cx = 2.0f / (u_ScreenWidth * -projectionMatrix[0][0]);
    float cy = 2.0f / (u_ScreenHeight * -projectionMatrix[1][1]);

    // Normalization term
    float d = cy * cy * zdx * zdx + cx * cx * zdy * zdy + cx * cx * cy * cy * zc * zc;

    // Derivatives of said term
    float ddx = cy * cy * 2.0f * zdx * zdx2 + cx * cx * 2.0f * zdy * zdxy + cx * cx * cy * cy * 2.0f * zc * zdx;
    float ddy = cy * cy * 2.0f * zdx * zdxy + cx * cx * 2.0f * zdy * zdy2 + cx * cx * cy * cy * 2.0f * zc * zdy;

    // Temporary variables to calculate mean curvature
    float ex = 0.5f * zdx * ddx - zdx2 * d;
    float ey = 0.5f * zdy * ddy - zdy2 * d;

    // Finally, mean curvature
    float h = 0.5f * ((cy * ex + cx * ey) / pow(d, 1.5f));

    return (vec3(zdx, zdy, h));
}

void main()
{
    float particleDepth = texture(u_DepthTex, f_TexCoord).r;
    if(particleDepth < -1000.0f || particleDepth > 0)
    {
        outDepth = particleDepth;
    }
    else
    {
        const float dt   = 0.0003f;
        const float dzt  = 1000.0f;
        vec3        dxyz = meanCurvature(f_TexCoord);

        outDepth = particleDepth + dxyz.z * dt * (1.0f + (abs(dxyz.x) + abs(dxyz.y)) * dzt);
    }
}
