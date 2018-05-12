// fragment shader, point-sphere-view
#version 410 core

#define NUM_TOTAL_LIGHTS 8
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
        int u_NumLights;
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
    vec4 ambient;
    vec4 diffuse;
        vec4 specular;
    float shininess;
} material;

uniform int u_HasVColor;
uniform float u_PointRadius;
uniform int u_UseAnisotropyKernel;

in vec3 f_ViewDir;
in vec3 f_FragPos;
in vec3 f_Color;
in mat3 f_AnisotropyMatrix;
out vec4 outColor;

vec3 shadeLight(int lightID, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(vec3(/*viewMatrix */ lights[lightID].position) - fragPos);
    vec3 halfDir = normalize(lightDir - viewDir);
        vec4 surfaceColor = (u_HasVColor == 1)? vec4(f_Color, 1.0): material.diffuse;

    vec4 ambientColor = lights[lightID].ambient * material.ambient;
    vec4 diffuseColor = lights[lightID].diffuse * vec4(max(dot(normal, lightDir), 0.0)) * surfaceColor;
    vec4 specularColor = lights[lightID].specular * pow(max(dot(halfDir, normal), 0.0), material.shininess) * material.specular;

    return vec3(ambientColor + diffuseColor + specularColor);
}

//------------------------------------------------------------------------------------------
void main()
{
    vec3 viewDir = normalize(f_ViewDir);
    vec3 N;

    //N.xy = gl_PointCoord.xy*vec2(2.0, -2.0) + vec2(-1.0, 1.0);

        //vec3 e1 = normalize(cross(f_FragPos, vec3(1, 0, 0)));
        //vec3 e2 = normalize(cross(f_FragPos, e1));
        vec3 rayDir;// = normalize(f_FragPos + (N.x *e1 + N.y * e2) * u_PointRadius); // - camPosition, which is at origin (0,0,0)



        vec3 fc = gl_FragCoord.xyz;
        //fc.xy += vec2(0.5, 0.5);
        fc.xy /= vec2(1920, 1080);
        fc *= 2.0;
        fc -= 1.0;

        vec4 p = invProjectionMatrix * vec4(fc, 1.);

                // in perspective mode, rayorigin is always at (0, 0, 0)

                rayDir = vec3(p) / p.w;


        mat3 G = f_AnisotropyMatrix;

        mat3 transMatrix = mat3(viewMatrix) * G;

        mat3 Tn = transpose(inverse((transMatrix)));

        mat3 transInvMatrix = inverse(transMatrix);

        vec3 camT = transInvMatrix * vec3(0, 0, 0);
        vec3 fragPosT = transInvMatrix * f_FragPos;
        vec3 rayDirT = normalize(transInvMatrix * rayDir);


        // solve the ray-sphere intersection
        float tmp = dot(rayDirT, camT - fragPosT);
        float delta = tmp*tmp - dot(camT - fragPosT, camT - fragPosT) + 1; // u_PointRadius*u_PointRadius;
        if(delta < 0.0) discard;

        float d = -tmp - sqrt(delta);
        vec3 intersectionPointT = camT + rayDirT * d;
        vec3 normalT = normalize(intersectionPointT - fragPosT);

        vec3 intersectionPoint = transMatrix * intersectionPointT;
        vec3 normal = normalize(Tn * normalT);



        // vec3 fragPos = normalize(f_FragPos + N * u_PointRadius);
        //intersectionPoint.xy *= u_PointRadius;
        //normal.xy *= u_PointRadius;
        vec3 fragPos = intersectionPoint;
        //normal = normalize(intersectionPoint - f_FragPos);
    vec3 shadeColor = vec3(0, 0, 0);

    for(int i = 0; i < u_NumLights; ++i)
        shadeColor += shadeLight(i, normal, fragPos, viewDir);

        outColor = vec4(shadeColor, 1.0);

        float z = dot(vec4(fragPos, 1.), transpose(projectionMatrix)[2]);
        float w = dot(vec4(fragPos, 1.), transpose(projectionMatrix)[3]);
        gl_FragDepth = 0.5 * (z / w + 1.0);

}




