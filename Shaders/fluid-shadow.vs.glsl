// vertex shader, fluid shadow
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

in vec3       v_Position;
in vec3       v_AnisotropyMatrix0;
in vec3       v_AnisotropyMatrix1;
in vec3       v_AnisotropyMatrix2;

out vec3      f_ViewCenter;
flat out mat3 f_AnisotropyMatrix;
flat out mat4 invPrjMatrix;

//------------------------------------------------------------------------------------------
const mat4 D = mat4(1., 0., 0., 0.,
    0., 1., 0., 0.,
    0., 0., 1., 0.,
    0., 0., 0., -1.);
void ComputePointSizeAndPosition(mat4 T)
{
    vec2  xbc;
    vec2  ybc;

    mat4  R = transpose(lightMatrices[u_LightID].prjMatrix * lightMatrices[u_LightID].viewMatrix * T);
    float A = dot(R[ 3 ], D * R[ 3 ]);
    float B = -2. * dot(R[ 0 ], D * R[ 3 ]);
    float C = dot(R[ 0 ], D * R[ 0 ]);
    xbc[ 0 ] = (-B - sqrt(B * B - 4. * A * C)) / (2.0 * A);
    xbc[ 1 ] = (-B + sqrt(B * B - 4. * A * C)) / (2.0 * A);
    float sx = abs(xbc[ 0 ] - xbc[ 1 ]) * .5 * u_ScreenWidth;

    A        = dot(R[ 3 ], D * R[ 3 ]);
    B        = -2. * dot(R[ 1 ], D * R[ 3 ]);
    C        = dot(R[ 1 ], D * R[ 1 ]);
    ybc[ 0 ] = (-B - sqrt(B * B - 4. * A * C)) / (2.0 * A);
    ybc[ 1 ] = (-B + sqrt(B * B - 4. * A * C)) / (2.0 * A);
    float sy = abs(ybc[ 0 ] - ybc[ 1 ]) * .5 * u_ScreenHeight;

    float pointSize = ceil(max(sx, sy));
    gl_PointSize = pointSize;
}

//------------------------------------------------------------------------------------------
void main()
{
    vec4  eyeCoord = lightMatrices[u_LightID].viewMatrix * vec4(v_Position, 1.0);
    vec3  posEye   = vec3(eyeCoord);
    float dist     = length(posEye);

    mat4  T = (u_UseAnisotropyKernel == 0) ?
              mat4(u_PointRadius, 0, 0, 0,
        0, u_PointRadius, 0, 0,
        0, 0, u_PointRadius, 0,
        v_Position.x, v_Position.y, v_Position.z, 1.0) :
              mat4(v_AnisotropyMatrix0[0] * u_PointRadius, v_AnisotropyMatrix0[1] * u_PointRadius, v_AnisotropyMatrix0[2] * u_PointRadius, 0,
        v_AnisotropyMatrix1[0] * u_PointRadius, v_AnisotropyMatrix1[1] * u_PointRadius, v_AnisotropyMatrix1[2] * u_PointRadius, 0,
        v_AnisotropyMatrix2[0] * u_PointRadius, v_AnisotropyMatrix2[1] * u_PointRadius, v_AnisotropyMatrix2[2] * u_PointRadius, 0,
        v_Position.x, v_Position.y, v_Position.z, 1.0);
    ComputePointSizeAndPosition(T);

    /////////////////////////////////////////////////////////////////
    // output
    f_ViewCenter       = eyeCoord.xyz;
    f_AnisotropyMatrix = (u_UseAnisotropyKernel == 0) ? mat3(0) : mat3(v_AnisotropyMatrix0, v_AnisotropyMatrix1, v_AnisotropyMatrix2);
    invPrjMatrix       = inverse(lightMatrices[u_LightID].prjMatrix);

    gl_Position = lightMatrices[u_LightID].prjMatrix * eyeCoord;
}
