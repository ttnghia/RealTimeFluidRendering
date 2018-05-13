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

#include "FRRenderObjects.h"
#include "Common.h"

#include <Banana/Data/ParticleSystemData.h>

#include <OpenGLHelpers/ShaderProgram.h>
#include <OpenGLHelpers/OpenGLBuffer.h>
#include <OpenGLHelpers/OpenGLTexture.h>

#include <QtAppHelpers/QtAppMacros.h>
#include <QtAppHelpers/OpenGLWidget.h>

#include <map>
#include <memory>

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
class RenderWidget : public OpenGLWidget
{
    Q_OBJECT

public:
    RenderWidget(QWidget* parent = 0);
    const auto& getParticleDataObj() const { return m_ParticleData; }
    const auto& getMeshObjs() const { return m_MeshObjs; }

protected:
    virtual void initOpenGL();
    virtual void resizeOpenGLWindow(int w, int h);
    virtual void renderOpenGL();

public slots:
    void reloadShaders();
    void reloadTextures();
    void enableExportFrame(bool bEnable) { m_bExrportFrameToImage = bEnable; }
protected:
    bool                             m_bExrportFrameToImage = false;
    Vector<SharedPtr<ShaderProgram>> m_ExternalShaders;

    ////////////////////////////////////////////////////////////////////////////////
    // custom floor with fluid shadow
public slots:
    void setFluidFloorTexture(int texIndex) { Q_ASSERT(m_FluidFloorRender != nullptr); m_FluidFloorRender->setRenderTextureIndex(texIndex); }
    void setFluidFloorSize(int size);
    void setFluidFloorExposure(int percentage) { m_FluidFloorRender->setExposure(static_cast<float>(percentage) / 100.0f); }
    void reloadFluidFloorTextures() { m_FluidFloorRender->clearTextures(); m_FluidFloorRender->loadTextures(QtAppUtils::getTexturePath() + "/Floor/"); }
protected:
    void initRDataFluidFloor();
    void renderFluidFloor() { Q_ASSERT(m_FluidFloorRender != nullptr); m_FluidFloorRender->render(m_bShadowEnabled, m_bVisualizeShadow, m_ShadowIntensity); }
    UniquePtr<FRPlaneRender> m_FluidFloorRender = nullptr;
    ////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////
    // particles
public slots:
    void updateParticleData();
    void setParticleColorMode(int colorMode);
    void setParticleRenderMode(int renderMode);
    void setParticleMaterial(const Material::MaterialData& material);
    void enableAnisotropyKernel(bool bAniKernel) { m_RDataParticle.useAnisotropyKernel = bAniKernel ? 1 : 0; }
protected:
    void initRDataParticle();
    void initParticleDataObj();
    void initParticleVAOs();
    void uploadParticleColorData();
    void renderParticles();
    void computeParticleAnisotropyKernel();

    struct RDataParticle
    {
        SharedPtr<ShaderProgram> shader               = nullptr;
        UniquePtr<OpenGLBuffer>  buffPosition         = nullptr;
        UniquePtr<OpenGLBuffer>  buffAnisotropyMatrix = nullptr;
        UniquePtr<OpenGLBuffer>  buffColorRandom      = nullptr;
        UniquePtr<OpenGLBuffer>  buffColorRamp        = nullptr;
        UniquePtr<OpenGLBuffer>  buffColorData        = nullptr;
        UniquePtr<Material>      material             = nullptr;

        GLuint VAO;
        GLint  v_Position;
        GLint  v_AnisotropyMatrix0;
        GLint  v_AnisotropyMatrix1;
        GLint  v_AnisotropyMatrix2;
        GLint  v_Color;
        GLuint ub_CamData;
        GLuint ub_Light;
        GLuint ub_Material;
        GLuint u_PointRadius;
        GLuint u_ClipPlane;
        GLuint u_IsPointView;
        GLuint u_HasVColor;
        GLuint u_UseAnisotropyKernel;
        GLuint u_ScreenWidth;
        GLuint u_ScreenHeight;

        GLuint  numParticles;
        GLfloat pointRadius;

        GLint isPointView         = 0;
        GLint hasVColor           = 1;
        GLint pColorMode          = ParticleColorMode::Random;
        GLint useAnisotropyKernel = 1;
        bool  initialized         = false;
    } m_RDataParticle;

    SharedPtr<ParticleSystemData> m_ParticleData    = std::make_shared<ParticleSystemData>();
    int                           m_FluidRenderMode = ParticleRenderMode::SphereParticle;
    ////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////
    // mesh
public slots:
    void updateMeshes();
    void updateNumMeshes(int numMeshes);
    void setMeshMaterial(const Material::MaterialData& material, int meshID);
protected:
    void initRDataMeshes();
    void renderMeshes();

    int                             m_NumMeshes = 0;
    Vector<SharedPtr<MeshObject>>   m_MeshObjs;
    Vector<UniquePtr<FRMeshRender>> m_MeshRenders;
    ////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////
    // fluid render
public slots:
    void setFluidViewMaterial(const Material::MaterialData& material);
    void setSurfaceReflectionConstant(int reflectionConstant) { m_ReflectionConstant = static_cast<float>(reflectionConstant) / 100.0f; }
    void setFluidAttennuationConstant(int attennuationConstant) { m_AttennuationConstant = static_cast<float>(attennuationConstant) / 100.0f; }
protected:
    float m_ReflectionConstant   = 0.0;
    float m_AttennuationConstant = 1.0;
    ////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////
    // shadow
public slots:
    void enableShadow(bool bShadowEnabled) { m_bShadowEnabled = bShadowEnabled; }
    void visualizeShadowRegion(bool bVisualizeShadow) { m_bVisualizeShadow = bVisualizeShadow; }
    void setShadowIntensity(int intensity) { m_ShadowIntensity = static_cast<float>(intensity) / 100.0f; }
protected:
    struct RDataFluidShadowMap
    {
        Vector<SharedPtr<OpenGLTexture>>     fluidShadowMaps;
        Vector<UniquePtr<DepthBufferRender>> depthMapRenders;
        SharedPtr<ShaderProgram>             shader = nullptr;
        GLuint                               VAO;
        GLuint                               v_Position;
        GLuint                               v_AnisotropyMatrix0;
        GLuint                               v_AnisotropyMatrix1;
        GLuint                               v_AnisotropyMatrix2;
        GLuint                               ub_LightMatrices;
        GLuint                               u_LightID;
        GLuint                               u_PointRadius;
        GLuint                               u_UseAnisotropyKernel;
        GLuint                               u_ScreenWidth;
        GLuint                               u_ScreenHeight;
        //        GLuint                                           u_PointScale;

        bool initialized = false;
    } m_RDataFluidShadowMap;

    struct RDataFluidShadowThickness
    {
        Vector<SharedPtr<OpenGLTexture>>   fluidShadowThickness;
        Vector<UniquePtr<OffScreenRender>> offScreenRenders;
        SharedPtr<ShaderProgram>           shader = nullptr;
        GLuint                             VAO;
        GLuint                             v_Position;
        GLuint                             ub_LightMatrices;
        GLuint                             u_LightID;
        GLuint                             u_PointRadius;
        GLuint                             u_PointScale;

        bool initialized = false;
    } m_RDataFluidShadowThickness;

    void initRDataFluidShadow();
    void generateFluidShadowMaps();
    void generateFluidShadowThicknessMaps();

    struct RDataSolidLightDepthMap
    {
        Vector<UniquePtr<DepthBufferRender>> depthMapRenders;
        SharedPtr<ShaderProgram>             shader            = nullptr;
        SharedPtr<OpenGLBuffer>              bufferModelMatrix = nullptr;
        Vector<SharedPtr<OpenGLTexture>>     solidShadowMaps;
        GLuint                               v_Position;
        GLuint                               ub_ModelMatrix;
        GLuint                               ub_LightMatrices;
        GLuint                               u_LightID;
        GLuint                               VAOs[MAX_NUM_MESHES];

        bool initialized = false;
    } m_RDataSolidLightDepthMap;

    struct RDataSolidCameraDepthMap
    {
        UniquePtr<DepthBufferRender> depthMapRender = nullptr;
        SharedPtr<ShaderProgram>     shader         = nullptr;
        GLuint                       v_Position;
        GLuint                       ub_ModelMatrix;
        GLuint                       ub_CameraData;
        GLuint                       VAOs[MAX_NUM_MESHES];

        bool initialized = false;
    } m_RDataSolidCameraDepthMap;

    void initRDataSolidShadow();
    void generateSolidLightDepthMaps();
    void renderSolidCameraDepthMap();

    float m_ShadowIntensity  = 1.0;
    bool  m_bShadowEnabled   = false;
    bool  m_bVisualizeShadow = false;
    ////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////
    // filter
public slots:
    void setFilterMethod(int method) { Q_ASSERT(method >= 0 && method < static_cast<int>(FilterMethod::NumFilters)); m_FilterMethod = method; }
    void setNumFilterIteration(int numIteration) { m_NumFilterIteration = numIteration; }
    void setFilterSize(int filterSize) { m_FilterSize = filterSize; }
protected:
    void initRDataSurfaceGeneration();
    void initRDataScreenQuadTexRender();
    void renderScreenQuadTex(const SharedPtr<OpenGLTexture>& texture, int texelSize = 1, float scaleValue = 1.0);
    void runDepthFilter();
    ////////////////////////////////////////////////////////////////////////////////

    int m_FilterMethod       = FilterMethod::BilateralGaussian;
    int m_FilterSize         = DEFAULT_FILTER_SIZE;
    int m_NumFilterIteration = DEFAULT_NUM_FILTER_ITERATION;
    ////////////////////////////////////////////////////////////////////////////////
    struct RDataDepthPass
    {
        SharedPtr<ShaderProgram>   shader          = nullptr;
        UniquePtr<OffScreenRender> offScreenRender = nullptr;

        GLuint VAO;
        GLint  v_Position;
        GLint  v_AnisotropyMatrix0;
        GLint  v_AnisotropyMatrix1;
        GLint  v_AnisotropyMatrix2;
        GLuint ub_CamData;
        GLuint u_PointRadius;

        //        GLuint u_PointScale;
        GLuint u_UseAnisotropyKernel;
        GLuint u_ScreenWidth;
        GLuint u_ScreenHeight;

        bool initialized = false;
    } m_RDataDepthPass;

    void initRDataDepthPass();
    void renderDepthPass();

    ////////////////////////////////////////////////////////////////////////////////
    struct RDataThicknessPass
    {
        SharedPtr<ShaderProgram>   shader          = nullptr;
        UniquePtr<OpenGLBuffer>    buffPosition    = nullptr;
        UniquePtr<OffScreenRender> offScreenRender = nullptr;

        GLuint VAO;
        GLint  v_Position;
        GLuint ub_CamData;
        GLuint u_PointRadius;
        GLuint u_PointScale;
        GLuint u_HasSolid;
        GLuint u_SolidDepthMap;

        GLfloat pointScale;
        bool    initialized = false;
    } m_RDataThicknessPass;

    void initRDataThicknessPass();
    void renderThicknessPass();

    ////////////////////////////////////////////////////////////////////////////////
    struct RDataFilterThickness
    {
        SharedPtr<ShaderProgram>   shader          = nullptr;
        UniquePtr<OffScreenRender> offScreenRender = nullptr;

        GLuint VAO;
        GLuint u_ThicknessTex;
        GLuint u_FilterSize;
        GLuint u_ScreenWidth;
        GLuint u_ScreenHeight;

        bool initialized = false;
    } m_RDataFilterThickness;

    void initRDataThicknessFilter();
    void filterThickness();

    ////////////////////////////////////////////////////////////////////////////////
    struct RDataNormalPass
    {
        SharedPtr<ShaderProgram>   shader          = nullptr;
        UniquePtr<OffScreenRender> offScreenRender = nullptr;

        GLuint VAO;
        GLuint ub_CamData;
        GLuint u_DepthTex;
        GLuint u_ScreenWidth;
        GLuint u_ScreenHeight;

        bool initialized = false;
    } m_RDataNormalPass;

    void initRDataNormalPass();
    void renderNormalPass(bool t0 = false);

    ////////////////////////////////////////////////////////////////////////////////
    struct RDataBackgroundPass
    {
        UniquePtr<OffScreenRender> offScreenRender = nullptr;

        //
        bool initialized = false;
    } m_RDataBackgroundPass;

    void initRDataBackgroundPass();
    void renderBackgroundPass();

    ////////////////////////////////////////////////////////////////////////////////
    struct RDataCompositionPass
    {
        SharedPtr<ShaderProgram> shader              = nullptr;
        UniquePtr<Material>      fluidVolumeMaterial = nullptr;
        SharedPtr<OpenGLTexture> depthTex            = nullptr;
        SharedPtr<OpenGLTexture> thicknessTex        = nullptr;
        SharedPtr<OpenGLTexture> filteredDepthTex    = nullptr;
        SharedPtr<OpenGLTexture> normalTex           = nullptr;
        SharedPtr<OpenGLTexture> backgroundTex       = nullptr;

        GLuint VAO;
        GLuint ub_CamData;
        GLuint ub_Light;
        GLuint ub_LightMatrices;
        GLuint ub_Material;

        GLuint u_HasSolid;
        GLuint u_SolidDepthMap;
        GLuint u_SkyBoxTex;
        GLuint u_DepthTex;
        GLuint u_ThicknessTex;
        GLuint u_NormalTex;
        GLuint u_BackgroundTex;
        GLuint u_FoamTex;
        GLuint u_SolidShadowMaps[MAX_NUM_LIGHTS];
        GLuint u_FluidShadowMaps[MAX_NUM_LIGHTS];
        GLuint u_FluidShadowThickness[MAX_NUM_LIGHTS];

        GLuint u_HasFoamTex;
        GLuint u_TransparentFluid;
        GLuint u_ReflectionConstant;
        GLuint u_AttennuationConstant;

        GLuint u_HasShadow;
        GLuint u_ShadowIntensity;
        GLuint u_VisualizeShadowRegion;

        bool initialized = false;
    } m_RDataCompositionPass;

    void initRDataCompositionPass();
    void renderCompositionPass();

    ////////////////////////////////////////////////////////////////////////////////
    struct RDataFilterBiGaussian
    {
        SharedPtr<ShaderProgram> shader = nullptr;

        GLuint VAO;
        GLuint u_DepthTex;
        GLuint u_ParticleRadius;
        GLuint u_FilterSize;
        GLuint u_MaxFilterSize;
        GLuint u_ScreenWidth;
        GLuint u_ScreenHeight;

        bool initialized = false;
    } m_RDataFilterBiGaussian;

    void initRDataFilterBiGaussian();
    void filterBiGaussian();

    ////////////////////////////////////////////////////////////////////////////////
    struct RDataFilterCurvatureFlow
    {
        SharedPtr<ShaderProgram> shader = nullptr;

        GLuint VAO;
        GLuint ub_CamData;
        GLuint u_DepthTex;
        GLuint u_ParticleRadius;
        GLuint u_ScreenWidth;
        GLuint u_ScreenHeight;

        bool initialized = false;
    } m_RDataFilterCurvatureFlow;

    void initRDataFilterCurvatureFlow();
    void filterCurvatureFlow();

    ////////////////////////////////////////////////////////////////////////////////
    struct RDataFilterPlaneFitting
    {
        SharedPtr<ShaderProgram> shader = nullptr;

        GLuint VAO;
        GLuint ub_CamData;
        GLuint u_DepthTex;
        GLuint u_NormalTex_t0;
        GLuint u_ParticleRadius;
        GLuint u_FilterSize;
        GLuint u_MaxFilterSize;
        GLuint u_ScreenWidth;
        GLuint u_ScreenHeight;

        bool initialized = false;
    } m_RDataFilterPlaneFitting;

    void initRDataFilterPlaneFitting();
    void filterPlaneFitting();

    ////////////////////////////////////////////////////////////////////////////////
    struct RDataFilterNarrowRangeFilter
    {
        SharedPtr<ShaderProgram> shader = nullptr;

        GLuint VAO;
        GLuint ub_CamData;
        GLuint u_DepthTex;
        GLuint u_ParticleRadius;
        GLuint u_FilterSize;
        GLuint u_MaxFilterSize;
        GLuint u_ScreenWidth;
        GLuint u_ScreenHeight;
        GLuint u_FilterDirection;
        GLuint u_DoFilter1D;                         // =1: only 1D, 0: only 2D, and -1: 1D+2D

        bool initialized = false;
    } m_RDataNarrowRangeFilter;

    void initRDataNarrowRangeFilter();
    void filterNarrowRange();

    ////////////////////////////////////////////////////////////////////////////////
    UniquePtr<ScreenQuadTextureRender> m_ScreenQuadTexRender = nullptr;
    UniquePtr<OffScreenRender>         m_FilterFrameBuffer   = nullptr;
};