//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//                                .--,       .--,
//                               ( (  \.---./  ) )
//                                '.__/o   o\__.'
//                                   {=  ^  =}
//                                    >  -  <
//     ___________________________.""`-------`"".____________________________
//    /                                                                      \
//    \    This file is part of Banana - a graphics programming framework    /
//    /                    Created: 2017 by Nghia Truong                     \
//    \                      <nghiatruong.vn@gmail.com>                      /
//    /                      https://ttnghia.github.io                       \
//    \                        All rights reserved.                          /
//    /                                                                      \
//    \______________________________________________________________________/
//                                  ___)( )(___
//                                 (((__) (__)))
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

#pragma once

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

// uncomment this to measure render time by calling glFinish() each frame
//#define BENCHMARK_FILTER

#define DEFAULT_CAMERA_POSITION glm::vec3(-4.0, 4.0, -3.0)
#define DEFAULT_CAMERA_FOCUS    glm::vec3(0, 1, 0)

#define CUSTOM_PARTICLE_MATERIAL         \
    {                                    \
        Vec4f( 0.2 * 0.2),               \
        Vec4f(0.69, 0.957, 0.259, 1.00), \
        Vec4f(   1),                     \
        250.0,                           \
        std::string("ParticleMaterial")  \
    }

#define CUSTOM_SURFACE_MATERIAL         \
    {                                   \
        Vec4f( 0.1, 0.1, 0.1, 1.0),     \
        Vec4f(0.42, 0.5, 1.0, 1.0),     \
        Vec4f( 1.0, 1.0, 1.0, 1.0),     \
        250.0,                          \
        std::string("ParticleMaterial") \
    }

#define DEFAULT_FLUID_MATERIAL Material::MT_Jade

#define DEFAULT_MESH_MATERIAL                     \
    {                                             \
        Vec4f(0.329412, 0.223529, 0.027451, 1.0), \
        Vec4f(0.780392, 0.568627, 0.113725, 1.0), \
        Vec4f(     1.0,      1.0,      1.0, 1.0), \
        250.0,                                    \
        std::string("MeshMaterial")               \
    }

#define PARTICLE_COLOR_RAMP       \
    {                             \
        glm::vec3(1.0, 0.0, 0.0), \
        glm::vec3(1.0, 0.5, 0.0), \
        glm::vec3(1.0, 1.0, 0.0), \
        glm::vec3(1.0, 0.0, 1.0), \
        glm::vec3(0.0, 1.0, 0.0), \
        glm::vec3(0.0, 1.0, 1.0), \
        glm::vec3(0.0, 0.0, 1.0)  \
    }

#define DEFAULT_CLIP_PLANE           Vec4f(0.0, 0.0, -1.0, 0.0)
#define DEFAULT_DELAY_TIME           50
#define DEFAULT_NUM_FILTER_ITERATION 3
#define DEFAULT_FILTER_SIZE          5

#define MAX_FILTER_SIZE              100
#define SHADOWMAP_TEXTURE_SIZE       2048
#define CLEAR_DEPTH_VALUE            -1000000.0f

#define THICKNESS_TEXTURE_SIZE_RATIO 0.25
#define THICKNESS_FILTER_SIZE        5
//#define SHADOWMAP_TEXTURE_SIZE_RATIO 0.5
#define SHADOWMAP_TEXTURE_SIZE_RATIO 1

#define MAX_NUM_MESHES               8

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

class ParticleColorMode
{
public:
    enum
    {
        Uniform = 0,
        Random,
        Ramp,
        NumColorMode
    };
};

class ParticleRenderMode
{
public:
    enum
    {
        SphereParticle = 0,
        TransparentFluid,
        OpaqueSurface,
        ThicknessBuffer,
        NormalBuffer,
        NumRenderMode
    };
};

class FilterMethod
{
public:
    enum
    {
        BilateralGaussian = 0,
        CurvatureFlow,
        PlaneFitting,
        NarrowRangeFilter1D2D,
        NarrowRangeFilter1D,
        NarrowRangeFilter2D,
        NumFilters
    };
};

class GLSLPrograms
{
public:
    enum
    {
        RenderFloor = 0,
        RenderSkyBox,
        RenderLight,
        RenderDataDomain,
        RenderPointSphere,
        DepthPass,
        ThicknessPass,
        BilateralGaussianFilter,
        PlaneFittingFilter,
        CurvatureFlowFilter,
        ModifiedGaussianFilter,
        NormalPass,
        CompositingPass,
        RenderMesh,
        RenderMeshDepth,
        RenderMeshShadow,
        NumPrograms
    };
};

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
#include <Banana/Setup.h>
namespace Banana {};
using namespace Banana;