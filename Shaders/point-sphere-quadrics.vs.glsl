// vertex shader, point-sphere-view
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

uniform float u_PointScale;
uniform float u_PointRadius;
uniform int u_IsPointView;

//------------------------------------------------------------------------------------------
in vec3 v_Position;
in vec3 v_Color;
in vec3 v_AnisotropyMatrix0;
in vec3 v_AnisotropyMatrix1;
in vec3 v_AnisotropyMatrix2;

out VS_OUT
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

//------------------------------------------------------------------------------------------
const vec4 yzPlane = vec4(1, 0, 0, -1);

// quadric matrix in canonical form for ellipsoids
const mat4 D  = mat4( 1., 0., 0., 0.,
                      0., 1., 0., 0.,
                      0., 0., 1., 0.,
                      0., 0., 0., -1. );

const vec2 viewport = vec2(1920, 1098);

//------------------------------------------------------------------------------
/// Compute point size and center using the technique described in:
/// "GPU-Based Ray-Casting of Quadratic Surfaces"
/// by Christian Sigg, Tim Weyrich, Mario Botsch, Markus Gross.
void ComputePointSizeAndPosition(mat4 T)
{
    vec2 xbc;
    vec2 ybc;

    mat4 R = transpose( projectionMatrix * viewMatrix * T );
    float A = dot( R[ 3 ], D * R[ 3 ] );
    float B = -2. * dot( R[ 0 ], D * R[ 3 ] );
    float C = dot( R[ 0 ], D * R[ 0 ] );
    xbc[ 0 ] = ( -B - sqrt( B * B - 4. * A * C ) ) / ( 2.0 * A );
    xbc[ 1 ] = ( -B + sqrt( B * B - 4. * A * C ) ) / ( 2.0 * A );
    float sx = abs( xbc[ 0 ] - xbc[ 1 ] ) * .5 * viewport.x;

    A = dot( R[ 3 ], D * R[ 3 ] );
    B = -2. * dot( R[ 1 ], D * R[ 3 ] );
    C = dot( R[ 1 ], D * R[ 1 ] );
    ybc[ 0 ] = ( -B - sqrt( B * B - 4. * A * C ) ) / ( 2.0 * A );
    ybc[ 1 ] = ( -B + sqrt( B * B - 4. * A * C ) ) / ( 2.0 * A );
    float sy = abs( ybc[ 0 ] - ybc[ 1 ]  ) * .5 * viewport.y;

    float pointSize = ceil( max( sx, sy ) );
    gl_PointSize = pointSize;
    f_PointSize = pointSize;
}

//------------------------------------------------------------------------------------------
void main()
{
    vec4 eyeCoord = viewMatrix * vec4(v_Position, 1.0);
    vec3 posEye = vec3(eyeCoord);
    float dist = length(posEye);

    /////////////////////////////////////////////////////////////////
    // output
    f_ViewDir = posEye;
        f_FragPos = vec3(viewMatrix * vec4(v_Position, 1.0));
    f_Color = v_Color;
        f_Position = v_Position;


    if(u_IsPointView == 1)
        gl_PointSize = 2.0;
    else
        gl_PointSize = u_PointRadius * (u_PointScale / dist);
    gl_Position = projectionMatrix * eyeCoord;
    gl_ClipDistance[0] = dot(vec4(v_Position, 1.0), yzPlane);

if(u_PointScale < 0)
f_Color *= 2;

        if(u_IsPointView < 0)
f_Color *= 2;


    float iradius = 1.0 / u_PointRadius;




//    mat4 T =  mat4(u_PointRadius, 0., 0., 0.,
//              0., u_PointRadius, 0., 0.,
//              0., 0., u_PointRadius * 0.5, 0.,
//              v_Position.x, v_Position.y, v_Position.z, 1.0 );





mat4 T = mat4(v_AnisotropyMatrix0[0], v_AnisotropyMatrix0[1], v_AnisotropyMatrix0[2], 0,
              v_AnisotropyMatrix1[0], v_AnisotropyMatrix1[1], v_AnisotropyMatrix1[2], 0,
              v_AnisotropyMatrix2[0], v_AnisotropyMatrix2[1], v_AnisotropyMatrix2[2], 0,
                                                  v_Position.x, v_Position.y, v_Position.z, 1.0 );// * u_PointRadius;
if(u_PointRadius  < 0)
T*=10;




                          /*
    mat4 Ti =  mat4( iradius, 0., 0., 0.,
              0., iradius, 0., 0.,
              0., 0., iradius * 2, 0.,
              -v_Position.x*iradius, -v_Position.y*iradius, -v_Position.z*iradius * 2, 1.0 );
*/
mat4 Ti = inverse(T);
    ComputePointSizeAndPosition(T);
        //ComputePointSizeAndPositionWithProjection(T);



    mat4 Tit = transpose(Ti);
    mat4 Q = transpose(invViewMatrix)* Tit * D * Ti * invViewMatrix;
    //////////////////
    // | a d e g |
    // | d b f h |
    // | e f c i |
    // | g h i j |
    // ax^2 + by^2 + cz^2 + 2dxy +2exz + 2fyz + 2gx + 2hy + 2iz + j = 0
    a = Q[ 0 ][ 0 ];
    b = Q[ 1 ][ 1 ];
    c = Q[ 2 ][ 2 ];
    d = Q[ 1 ][ 0 ];
    e = Q[ 2 ][ 0 ];
    f = Q[ 2 ][ 1 ];
    g = Q[ 3 ][ 0 ];
    h = Q[ 3 ][ 1 ];
    i = Q[ 3 ][ 2 ];
    j = Q[ 3 ][ 3 ];

}
