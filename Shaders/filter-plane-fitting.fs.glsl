// fragment shader, plane fitting pass
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
uniform sampler2D u_NormalTex_t0;

uniform float u_ParticleRadius;
uniform int   u_FilterSize;
uniform int   u_MaxFilterSize;
uniform int   u_ScreenWidth;
uniform int   u_ScreenHeight;

in vec2   f_TexCoord;
out float outDepth;

const float far            = 1000.0f;
const float near           = 0.1f;
const float thresholdRatio = 10.0;
//------------------------------------------------------------------------------------------
vec3 uvToEye(vec2 texCoord, float depth)
{
    float x = texCoord.x * 2.0 - 1.0;
    float y = texCoord.y * 2.0 - 1.0;

    float zn      = ((far + near) / (far - near) * depth + 2 * far * near / (far - near)) / depth;
    vec4  clipPos = vec4(x, y, zn, 1.0f);
    vec4  viewPos = invProjectionMatrix * clipPos;
    return viewPos.xyz / viewPos.w;
}

float compute_weight1D(float r, float two_sigma2)
{
    return exp(-r * r / two_sigma2);
}

//------------------------------------------------------------------------------------------
void main()
{
    vec2  blurRadius = vec2(1.0 / u_ScreenWidth, 1.0 / u_ScreenHeight);
    float pixelDepth = texture(u_DepthTex, f_TexCoord).r;
    vec3  viewDir    = normalize(uvToEye(f_TexCoord, pixelDepth));
    float finalDepth;

    if(pixelDepth > 0 || pixelDepth < -1000.0f) {
        finalDepth = pixelDepth;
    } else {
        float threshold       = u_ParticleRadius * thresholdRatio;
        float sigmaDepth      = threshold / 3.0f;
        float two_sigmaDepth2 = 2.0f * sigmaDepth * sigmaDepth;

        float ratio      = u_ScreenHeight / 2.0 / tan(45.0 / 2.0);
        float K          = -u_FilterSize * ratio * u_ParticleRadius * 0.1f;
        int   filterSize = min(u_MaxFilterSize, int(ceil(K / pixelDepth)));

        vec4 f_tex = f_TexCoord.xyxy;
        vec4 sampleDepth;
        vec3 sampleNormal[4];
        vec3 samplePosition[4];

        vec4 wsum4 = vec4(1, 0, 0, 0);
        vec3 nsum  = texture(u_NormalTex_t0, f_TexCoord).xyz;
        vec3 psum  = uvToEye(f_TexCoord, pixelDepth);
        vec4 rDepth;
        vec4 w4_depth;

        for(int x = 1; x <= filterSize; ++x) {
            f_tex.x += blurRadius.x;
            f_tex.z -= blurRadius.x;
            vec4 f_tex1 = f_tex.xyxy;
            vec4 f_tex2 = f_tex.zwzw;
            for(int y = 1; y <= filterSize; ++y) {
                f_tex1.y += blurRadius.y;
                f_tex1.w -= blurRadius.y;
                f_tex2.y += blurRadius.y;
                f_tex2.w -= blurRadius.y;

                sampleDepth.x = texture(u_DepthTex, f_tex1.xy).r;
                sampleDepth.y = texture(u_DepthTex, f_tex1.zw).r;
                sampleDepth.z = texture(u_DepthTex, f_tex2.xy).r;
                sampleDepth.w = texture(u_DepthTex, f_tex2.zw).r;

                sampleNormal[0] = texture(u_NormalTex_t0, f_tex1.xy).xyz;
                sampleNormal[1] = texture(u_NormalTex_t0, f_tex1.zw).xyz;
                sampleNormal[2] = texture(u_NormalTex_t0, f_tex2.xy).xyz;
                sampleNormal[3] = texture(u_NormalTex_t0, f_tex2.zw).xyz;

                samplePosition[0] = uvToEye(f_tex1.xy, sampleDepth[0]);
                samplePosition[1] = uvToEye(f_tex1.zw, sampleDepth[1]);
                samplePosition[2] = uvToEye(f_tex2.xy, sampleDepth[2]);
                samplePosition[3] = uvToEye(f_tex2.zw, sampleDepth[3]);

                rDepth     = sampleDepth - vec4(pixelDepth);
                w4_depth.x = compute_weight1D(rDepth.x, two_sigmaDepth2);
                w4_depth.y = compute_weight1D(rDepth.y, two_sigmaDepth2);
                w4_depth.z = compute_weight1D(rDepth.z, two_sigmaDepth2);
                w4_depth.w = compute_weight1D(rDepth.w, two_sigmaDepth2);

                wsum4 += w4_depth;

                nsum += sampleNormal[0] * w4_depth.x;
                nsum += sampleNormal[1] * w4_depth.y;
                nsum += sampleNormal[2] * w4_depth.z;
                nsum += sampleNormal[3] * w4_depth.w;

                psum += samplePosition[0] * w4_depth.x;
                psum += samplePosition[1] * w4_depth.y;
                psum += samplePosition[2] * w4_depth.z;
                psum += samplePosition[3] * w4_depth.w;
            }
        }

        float sum = dot(wsum4, vec4(1, 1, 1, 1));
        nsum = normalize(nsum);
        psum = psum / sum;

        float di = dot(nsum, psum);
        finalDepth = di * viewDir.z / dot(nsum, viewDir);
    }

    outDepth = finalDepth;
}
