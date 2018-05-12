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

in VS_OUT
{
    vec3 f_ViewDir;
    vec3 f_FragPos;
    vec3 f_Position;
    vec3 f_Color;
    float f_PointSize;

    // quadric coefficients
    // | a d e g |
    // | d b f h |
    // | e f c i |
    // | g h i j |
    // ax^2 + by^2 + cz^2 + 2dxy +2exz + 2fyz + 2gx + 2hy + 2iz + j = 0
    float a;
    float b;
    float c;
    float d;
    float e;
    float f;
    float g;
    float h;
    float i;
    float j;
};

out vec4 outColor;




const vec2 viewport = vec2(1920, 1098);


vec3 raydir; // ray direction in screen space
vec3 rayorigin; // ray origin in screen space

// INTERSECTION
struct I
{
	vec3 P;
	vec3 N;
	float t;
};

// compute unit normal from gradient
vec3 ComputeNormal(vec3 P)
{
	return normalize(vec3(dot(vec4(a, d, e, 1.), vec4(P, g)), // should multiply by 2 for actual gradient
			dot(vec4(d, b, f, 1.), vec4(P, h)), // should multiply by 2 for actual gradient
			dot(vec4(e, f, c, 1.), vec4(P, i)) // should multiply by 2 for actual gradient
			));
}



// compute ray quadric intersection; if no intersection occurs I.t is < 0
// main axis length and orientation are used to clip the quadric; not
// required for closed quadrics (ellipsoids)
// | a d e g |
// | d b f h |
// | e f c i |
// | g h i j |
// ax^2 + by^2 + cz^2 + 2dxy +2exz + 2fyz + 2gx + 2hy + 2iz + j = 0
/// @todo pass vec3(a, b, c), vec3( d, e, f ) and vec3( g, h, i ) instead of single coefficients
I ComputeRayQuadricIntersection()
{
	I ip;
	ip.t = -1.0;
	vec3 P = rayorigin;
	vec3 D = raydir;
	float A = 0.0;
	float B = 0.0;
	float C = 0.0;
		
	A = dot(vec3(a, b, c), D * D) + 2. * dot(vec3(d, e, f), D.xxy * D.yzz);
	B = 2. * dot(vec3(g, h, i), D);
	C = j;
		
	float delta = B * B - 4. * A * C;
	if (delta < 0.0)
		return ip;
	float d = sqrt(delta);
	A = 1. / A;
	A *= 0.5;
	float t2 = A * (-B + d);
	float t1 = A * (-B - d);

	ip.P = rayorigin + D * min(t1, t2);
	ip.N = ComputeNormal(ip.P);
	ip.t = 0;

	return ip;
}




//------------------------------------------------------------------------------------------
vec3 shadeLight(int lightID, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(vec3(viewMatrix * lights[lightID].position) - fragPos);
    vec3 halfDir = normalize(lightDir - viewDir);
	vec4 surfaceColor = (u_HasVColor == 1)? vec4(f_Color, 1.0): material.diffuse;

    vec4 ambientColor = lights[lightID].ambient * material.ambient;
    vec4 diffuseColor = lights[lightID].diffuse * vec4(max(dot(normal, lightDir), 0.0)) * surfaceColor;
    vec4 specularColor = lights[lightID].specular * pow(max(dot(halfDir, normal), 0.0), material.shininess) * material.specular;
    
    return vec3(ambientColor + diffuseColor + specularColor);
}


		const float farZ = 1000.0f;
const float nearZ = 0.1f;
		
vec3 uvToEye(vec2 texCoord, float eyeDepth)
{
    float x = texCoord.x * 2.0 - 1.0;
    float y = texCoord.y * 2.0 - 1.0;
    float zn = ((farZ + nearZ) / (farZ - nearZ) * eyeDepth + 2 * farZ * nearZ / (farZ - nearZ)) / eyeDepth;

    vec4 clipPos = vec4(x, y, zn, 1.0f);
    vec4 viewPos = invProjectionMatrix * clipPos;
    return viewPos.xyz / viewPos.w;
}
//------------------------------------------------------------------------------------------
void main()
{
    vec3 viewDir = normalize(f_ViewDir);
 	
	if(f_PointSize < 1.0)
	{
	outColor = vec4(1, 0, 0, 1);
	gl_FragDepth = 0.1;
	return;
	}
	
	
	
	vec3 fc = gl_FragCoord.xyz;
	//fc.xy += vec2(0.5, 0.5);
	fc.xy /= viewport;
	fc *= 2.0;
	fc -= 1.0;
		
	vec4 p = invProjectionMatrix * vec4(fc, 1.);

		// in perspective mode, rayorigin is always at (0, 0, 0)
		rayorigin = vec3(0., 0., 0.);
		raydir = vec3(p) / p.w;
		
		
		  vec3 N;

    N.xy = gl_PointCoord.xy*vec2(2.0, -2.0) + vec2(-1.0, 1.0);
    
	vec3 e1 = normalize(cross(f_FragPos, vec3(1, 0, 0)));
	vec3 e2 = normalize(cross(f_FragPos, e1));
	//raydir = normalize(f_FragPos + (N.x *e1 + N.y * e2) * u_PointRadius); // - camPosition, which is at origin (0,0,0)
	//raydir = uvToEye(gl_FragCoord.xy / viewport, gl_FragCoord.z);
		
	// compute intersection
	I ip = ComputeRayQuadricIntersection();
	if (ip.t < 0)
	{
	//outColor = vec4(0.9, 0.9, 0.9, 0.5);
	//gl_FragDepth = 0.99;
		discard;
	}
		
	{
	
	
	// update depth by projecting point and updating depth coordinate
	// the transposed version of the projection matrix is used to
	// perform vector, matrix row product in one line:
	// M[2][*] x V = Vt x Mt[*][2] where:
	//   % V  is a column vector
	//   % Vt is a row vector
	//   % M is a square matrix
	//   % Mt is the transpose of M
	float z = dot(vec4(ip.P, 1.), transpose(projectionMatrix)[2]);
	float w = dot(vec4(ip.P, 1.), transpose(projectionMatrix)[3]);
	gl_FragDepth = 0.5 * (z / w + 1.0);
	
	
	
	
	
		//vec3 fragPos = intersectionPoint;
    vec3 shadeColor = vec3(0, 0, 0);

    for(int i = 0; i < u_NumLights; ++i)
        shadeColor += shadeLight(i, ip.N, ip.P, viewDir);
    
	outColor = vec4(shadeColor, 1.0);
	
	}
	
}




