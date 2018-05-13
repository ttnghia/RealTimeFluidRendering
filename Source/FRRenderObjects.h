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

#include <OpenGLHelpers/RenderObjects.h>
#include "Common.h"

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// FRMeshRender
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
class FRMeshRender : public MeshRender
{
public:
    FRMeshRender(const SharedPtr<MeshObject>&   meshObj,
                 const SharedPtr<Camera>&       camera,
                 const SharedPtr<PointLights>&  light,
                 QString                        textureFolder,
                 const SharedPtr<Material>&     material      = nullptr,
                 const SharedPtr<OpenGLBuffer>& bufferCamData = nullptr) :
        MeshRender(meshObj, camera, light, textureFolder, material, bufferCamData)
    {
        initRenderData();
    }

    FRMeshRender(const SharedPtr<MeshObject>&   meshObj,
                 const SharedPtr<Camera>&       camera,
                 const SharedPtr<PointLights>&  light,
                 const SharedPtr<Material>&     material      = nullptr,
                 const SharedPtr<OpenGLBuffer>& bufferCamData = nullptr) :
        MeshRender(meshObj, camera, light, material, bufferCamData)
    {
        initRenderData();
    }

    virtual void render(bool bRenderShadow, bool bVisualizeShadowRegion, float shadowIntensity = 1.0f);

    void setSolidShadowMaps(const Vector<SharedPtr<OpenGLTexture>>& shadowMaps) { m_SolidShadowMaps = shadowMaps; }
    void setFluidShadowMaps(const Vector<SharedPtr<OpenGLTexture>>& shadowMaps) { m_FluidShadowMaps = shadowMaps; }
    void setFluidShadowThickness(const Vector<SharedPtr<OpenGLTexture>>& shadowThickness) { m_FluidShadowThickness = shadowThickness; }

protected:
    virtual void initRenderData() override;

    Vector<SharedPtr<OpenGLTexture>> m_SolidShadowMaps;
    Vector<SharedPtr<OpenGLTexture>> m_FluidShadowMaps;
    Vector<SharedPtr<OpenGLTexture>> m_FluidShadowThickness;

    ////////////////////////////////////////////////////////////////////////////////
    GLuint m_USolidShadowMaps[MAX_NUM_LIGHTS];
    GLuint m_UFluidShadowMaps[MAX_NUM_LIGHTS];
    GLuint m_UFluidShadowThickness[MAX_NUM_LIGHTS];
    GLuint m_UShadowIntensity;
    GLuint m_UVisualizeShadowRegion;
};

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// FRPlaneRender
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
class FRPlaneRender : public FRMeshRender
{
public:
    FRPlaneRender(const SharedPtr<Camera>&       camera,
                  const SharedPtr<PointLights>&  light,
                  QString                        textureFolder,
                  const SharedPtr<OpenGLBuffer>& bufferCamData = nullptr) :
        FRMeshRender(std::make_shared<GridObject>(), camera, light, textureFolder, nullptr, bufferCamData)
    {}

    FRPlaneRender(const SharedPtr<Camera>&       camera,
                  const SharedPtr<PointLights>&  light,
                  const SharedPtr<OpenGLBuffer>& bufferCamData = nullptr) :
        FRMeshRender(std::make_shared<GridObject>(), camera, light, nullptr, bufferCamData)
    {}

    virtual void render(bool bRenderShadow, bool bVisualizeShadowRegion, float shadowIntensity = 1.0f);
    void         scaleTexCoord(int scaleX, int scaleY);
};