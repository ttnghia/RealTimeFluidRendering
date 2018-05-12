//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//
//  Copyright (c) 2017 by
//       __      _     _         _____
//    /\ \ \__ _| |__ (_) __ _  /__   \_ __ _   _  ___  _ __   __ _
//   /  \/ / _` | '_ \| |/ _` |   / /\/ '__| | | |/ _ \| '_ \ / _` |
//  / /\  / (_| | | | | | (_| |  / /  | |  | |_| | (_) | | | | (_| |
//  \_\ \/ \__, |_| |_|_|\__,_|  \/   |_|   \__,_|\___/|_| |_|\__, |
//         |___/                                              |___/
//
//  <nghiatruong.vn@gmail.com>
//  All rights reserved.
//
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

#pragma once

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//#define BENCHMARK_FILTER

#define DEFAULT_CLEAR_COLOR     Vec4f(1.0, 1.0, 1.0, 1.0)
#define DEFAULT_CAMERA_POSITION glm::vec3(-4.0, 4.0, -3.0)
#define DEFAULT_CAMERA_FOCUS    glm::vec3(0, 1, 0)

#define CUSTOM_PARTICLE_MATERIAL         \
    {                                    \
        Vec4f(0.2 * 0.2),                \
        Vec4f(0.69, 0.957, 0.259, 1.00), \
        Vec4f(1),                        \
        250.0,                           \
        std::string("ParticleMaterial")  \
    }


#define CUSTOM_SURFACE_MATERIAL         \
    {                                   \
        Vec4f(0.1,  0.1, 0.1, 1.0),     \
        Vec4f(0.42, 0.5, 1.0, 1.0),     \
        Vec4f(1.0,  1.0, 1.0, 1.0),     \
        250.0,                          \
        std::string("ParticleMaterial") \
    }


#define DEFAULT_FLUID_VOLUME_MATERIAL Material::MT_Jade
#define DEFAULT_MESH_MATERIAL_BK      Material::MT_Brass

#define DEFAULT_MESH_MATERIAL                     \
    {                                             \
        Vec4f(0.329412, 0.223529, 0.027451, 1.0), \
        Vec4f(0.780392, 0.568627, 0.113725, 1.0), \
        Vec4f(1.0,  1.0, 1.0, 1.0),               \
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
        FromData,
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
        ModifiedGaussian1D2D,
        ModifiedGaussian1D,
        ModifiedGaussian2D,
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
#include <QStringList>
#include <QString>
#include <QDir>
////////////////////////////////////////////////////////////////////////////////
inline QStringList getTextureFolders(QString texType)
{
    QDir dataDir(QDir::currentPath() + "/Textures/" + texType);
    dataDir.setFilter(QDir::NoDotAndDotDot | QDir::Dirs);

    return dataDir.entryList();
}

inline QStringList getTextureFiles(QString texType)
{
    QDir dataDir(QDir::currentPath() + "/Textures/" + texType);
    dataDir.setFilter(QDir::NoDotAndDotDot | QDir::Files);

    return dataDir.entryList();
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
#include <Banana/Setup.h>
namespace Banana {};
using namespace Banana;