// fragment shader, fluid shadow
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
uniform int   u_UseAnisotropyKernel;
uniform int   u_ScreenWidth;
uniform int   u_ScreenHeight;

in vec3       f_ViewCenter;
flat in mat3  f_AnisotropyMatrix;
flat in mat4  invPrjMatrix;
out float     outDepth;

//------------------------------------------------------------------------------------------
void main()
{
    vec3 viewDir = normalize(f_ViewCenter);
    vec3 normal;
    vec3 fragPos;

    if(u_UseAnisotropyKernel == 0)
    {
        normal.xy = gl_PointCoord.xy * vec2(2.0, -2.0) + vec2(-1.0, 1.0);
        float mag = dot(normal.xy, normal.xy);

        if(mag > 1.0)
            discard;           // kill pixels outside circle
        normal.z = sqrt(1.0 - mag);
        fragPos  = f_ViewCenter + normal * u_PointRadius;
    }
    else
    {
        vec3 fc = gl_FragCoord.xyz;
        fc.xy /= vec2(u_ScreenWidth, u_ScreenHeight);
        fc    *= 2.0;
        fc    -= 1.0;

        vec4 worldPos = invPrjMatrix * vec4(fc, 1.0);
        vec3 rayDir   = vec3(worldPos) / worldPos.w;

        mat3 transMatrix    = mat3(lightMatrices[u_LightID].viewMatrix) * f_AnisotropyMatrix * u_PointRadius;
        mat3 transInvMatrix = inverse(transMatrix);
        mat3 normalMatrix   = transpose(inverse((transMatrix)));

        vec3 camT    = transInvMatrix * vec3(0, 0, 0);
        vec3 centerT = transInvMatrix * f_ViewCenter;
        vec3 rayDirT = normalize(transInvMatrix * rayDir);

        // solve the ray-sphere intersection
        float tmp   = dot(rayDirT, camT - centerT);
        float delta = tmp * tmp - dot(camT - centerT, camT - centerT) + 1.0;
        if(delta < 0.0)
            discard;             // kill pixels outside circle in parameter space

        float d                  = -tmp - sqrt(delta);
        vec3  intersectionPointT = camT + rayDirT * d;
        fragPos = transMatrix * intersectionPointT;;
    }

    //calculate depth
    vec4 clipSpacePos = lightMatrices[u_LightID].prjMatrix * vec4(fragPos, 1.0);
    outDepth = fragPos.z;

    gl_FragDepth = clipSpacePos.z / clipSpacePos.w;
}
