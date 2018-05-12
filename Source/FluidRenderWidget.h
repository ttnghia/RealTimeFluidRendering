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

#include "Common.h"
#include "FRRenderObjects.h"

#include <Banana/Data/ParticleSystemData.h>

#include <OpenGLHelpers/ShaderProgram.h>
#include <OpenGLHelpers/OpenGLBuffer.h>
#include <OpenGLHelpers/OpenGLTexture.h>

#include <QtAppHelpers/QtAppMacros.h>
#include <QtAppHelpers/OpenGLWidget.h>

#include <map>
#include <memory>

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
class FluidRenderWidget : public OpenGLWidget
{
    Q_OBJECT

public:

    FluidRenderWidget(QWidget* parent = 0);

    const std::shared_ptr<ParticleSystemData>&       getParticleDataObj() const;
    const std::vector<std::shared_ptr<MeshObject> >& getMeshObjs() const;
    const auto&                                      getLightObjs() const { return m_Lights; }

    void setCamera(const glm::vec3& cameraPosition, const glm::vec3& cameraFocus);
    void setBox(const glm::vec3& boxMin, const glm::vec3& boxMax);

protected:
    virtual void initOpenGL();
    virtual void resizeOpenGLWindow(int w, int h);
    virtual void renderOpenGL();

public slots:
    void updateParticleData();
    void updateMeshes();
    void updateNumMeshes(int numMeshes);
    void updateLights();

    void setSkyBoxTexture(int texIndex);
    void setFloorTexture(int texIndex);
    void setFloorSize(int size);
    void setParticleColorMode(int colorMode);
    void setFluidRenderMode(int renderMode);

    void reloadShaders();
    void reloadTextures();

    void setParticleMaterial(const Material::MaterialData& material);
    void setFluidVolumeMaterial(const Material::MaterialData& material);
    void setMeshMaterial(const Material::MaterialData& material, int meshID);

    void setFilterMethod(int method);
    void setNumFilterIteration(int numIteration);
    void setFilterSize(int filterSize);

    void setSurfaceReflectionConstant(int reflectionConstant);
    void setFluidAttennuationConstant(int attennuationConstant);

    void enableAnisotropyKernel(bool bAniKernel);
    void enableShadow(bool bShadowEnabled);
    void visualizeShadowRegion(bool bVisualizeShadow);
    void setShadowIntensity(int intensity);

    void enableClipPlane(bool bEnable);
    void setClipPlane(const Vec4f& clipPlane);

    void enableExportFrame(bool bEnable);

signals:
    void lightsObjChanged(const std::shared_ptr<PointLights>& lights);

private:

    ////////////////////////////////////////////////////////////////////////////////
    void initRDataLight();
    void renderLight();

    ////////////////////////////////////////////////////////////////////////////////
    void initRDataSkyBox();
    void renderSkyBox();

    ////////////////////////////////////////////////////////////////////////////////
    void initRDataFloor();
    void renderFloor();

    ////////////////////////////////////////////////////////////////////////////////
    void initRDataBox();
    void renderBox();

    ////////////////////////////////////////////////////////////////////////////////
    struct RDataParticle
    {
        std::shared_ptr<ShaderProgram> shader               = nullptr;
        std::unique_ptr<OpenGLBuffer>  buffPosition         = nullptr;
        std::unique_ptr<OpenGLBuffer>  buffAnisotropyMatrix = nullptr;
        std::unique_ptr<OpenGLBuffer>  buffColorRandom      = nullptr;
        std::unique_ptr<OpenGLBuffer>  buffColorRamp        = nullptr;
        std::unique_ptr<OpenGLBuffer>  buffColorData        = nullptr;
        std::unique_ptr<Material>      material             = nullptr;

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

    std::shared_ptr<ParticleSystemData> m_ParticleData = std::make_shared<ParticleSystemData>();


    void initRDataParticle();
    void initFluidVAOs();
    void uploadParticleColorData();
    void renderParticles();
    void initParticleDataObj();
    void computeParticleAnisotropyKernel();

    ////////////////////////////////////////////////////////////////////////////////
    void initRDataSurfaceGeneration();
    void initRDataScreenQuadTexRender();
    void renderScreenQuadTex(const std::shared_ptr<OpenGLTexture>& texture,
                             int                                   texelSize  = 1,
                             float                                 scaleValue = 1.0);
    void runDepthFilter();


    ////////////////////////////////////////////////////////////////////////////////
    struct RDataDepthPass
    {
        std::shared_ptr<ShaderProgram>   shader          = nullptr;
        std::unique_ptr<OffScreenRender> offScreenRender = nullptr;

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
        std::shared_ptr<ShaderProgram>   shader          = nullptr;
        std::unique_ptr<OpenGLBuffer>    buffPosition    = nullptr;
        std::unique_ptr<OffScreenRender> offScreenRender = nullptr;

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
        std::shared_ptr<ShaderProgram>   shader          = nullptr;
        std::unique_ptr<OffScreenRender> offScreenRender = nullptr;

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
        std::shared_ptr<ShaderProgram>   shader          = nullptr;
        std::unique_ptr<OffScreenRender> offScreenRender = nullptr;

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
        std::unique_ptr<OffScreenRender> offScreenRender = nullptr;

        //
        bool initialized = false;
    } m_RDataBackgroundPass;

    void initRDataBackgroundPass();
    void renderBackgroundPass();

    ////////////////////////////////////////////////////////////////////////////////
    struct RDataCompositionPass
    {
        std::shared_ptr<ShaderProgram> shader              = nullptr;
        std::unique_ptr<Material>      fluidVolumeMaterial = nullptr;
        std::shared_ptr<OpenGLTexture> depthTex            = nullptr;
        std::shared_ptr<OpenGLTexture> thicknessTex        = nullptr;
        std::shared_ptr<OpenGLTexture> filteredDepthTex    = nullptr;
        std::shared_ptr<OpenGLTexture> normalTex           = nullptr;
        std::shared_ptr<OpenGLTexture> backgroundTex       = nullptr;

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
        std::shared_ptr<ShaderProgram> shader = nullptr;

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
        std::shared_ptr<ShaderProgram> shader = nullptr;

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
        std::shared_ptr<ShaderProgram> shader = nullptr;

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
    struct RDataFilterModifiedGaussian
    {
        std::shared_ptr<ShaderProgram> shader = nullptr;

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
    } m_RDataFilterModifiedGaussian;

    void initRDataFilterModifiedGaussian();
    void filterModifiedGaussian();


    ////////////////////////////////////////////////////////////////////////////////
    void initRDataMeshes();
    void renderMeshes();

    ////////////////////////////////////////////////////////////////////////////////
    struct RDataFluidShadowMap
    {
        std::vector<std::shared_ptr<OpenGLTexture> >     fluidShadowMaps;
        std::vector<std::unique_ptr<DepthBufferRender> > depthMapRenders;
        std::shared_ptr<ShaderProgram>                   shader = nullptr;
        GLuint                                           VAO;
        GLuint                                           v_Position;
        GLuint                                           v_AnisotropyMatrix0;
        GLuint                                           v_AnisotropyMatrix1;
        GLuint                                           v_AnisotropyMatrix2;
        GLuint                                           ub_LightMatrices;
        GLuint                                           u_LightID;
        GLuint                                           u_PointRadius;
        GLuint                                           u_UseAnisotropyKernel;
        GLuint                                           u_ScreenWidth;
        GLuint                                           u_ScreenHeight;
//        GLuint                                           u_PointScale;

        bool initialized = false;
    } m_RDataFluidShadowMap;

    struct RDataFluidShadowThickness
    {
        std::vector<std::shared_ptr<OpenGLTexture> >   fluidShadowThickness;
        std::vector<std::unique_ptr<OffScreenRender> > offScreenRenders;
        std::shared_ptr<ShaderProgram>                 shader = nullptr;
        GLuint                                         VAO;
        GLuint                                         v_Position;
        GLuint                                         ub_LightMatrices;
        GLuint                                         u_LightID;
        GLuint                                         u_PointRadius;
        GLuint                                         u_PointScale;

        bool initialized = false;
    } m_RDataFluidShadowThickness;

    void initRDataFluidShadow();
    void generateFluidShadowMaps();
    void generateFluidShadowThicknessMaps();

    struct RDataSolidLightDepthMap
    {
        std::vector<std::unique_ptr<DepthBufferRender> > depthMapRenders;
        std::shared_ptr<ShaderProgram>                   shader            = nullptr;
        std::shared_ptr<OpenGLBuffer>                    bufferModelMatrix = nullptr;
        std::vector<std::shared_ptr<OpenGLTexture> >     solidShadowMaps;
        GLuint                                           v_Position;
        GLuint                                           ub_ModelMatrix;
        GLuint                                           ub_LightMatrices;
        GLuint                                           u_LightID;
        GLuint                                           VAOs[MAX_NUM_MESHES];

        bool initialized = false;
    } m_RDataSolidLightDepthMap;


    struct RDataSolidCameraDepthMap
    {
        std::unique_ptr<DepthBufferRender> depthMapRender = nullptr;
        std::shared_ptr<ShaderProgram>     shader         = nullptr;
        GLuint                             v_Position;
        GLuint                             ub_ModelMatrix;
        GLuint                             ub_CameraData;
        GLuint                             VAOs[MAX_NUM_MESHES];

        bool initialized = false;
    } m_RDataSolidCameraDepthMap;

    void initRDataSolidShadow();
    void generateSolidLightDepthMaps();
    void renderSolidCameraDepthMap();

    ////////////////////////////////////////////////////////////////////////////////
    Vec4f m_ClipPlane            = DEFAULT_CLIP_PLANE;
    int   m_FluidRenderMode      = ParticleRenderMode::SphereParticle;
    int   m_FilterMethod         = FilterMethod::BilateralGaussian;
    int   m_FilterSize           = DEFAULT_FILTER_SIZE;
    int   m_NumFilterIteration   = DEFAULT_NUM_FILTER_ITERATION;
    int   m_NumMeshes            = 0;
    float m_ReflectionConstant   = 0.0;
    float m_AttennuationConstant = 1.0;
    float m_ShadowIntensity      = 1.0;
    bool  m_bShadowEnabled       = false;
    bool  m_bVisualizeShadow     = false;
    bool  m_bExrportFrameToImage = false;

    ////////////////////////////////////////////////////////////////////////////////
    std::vector<std::shared_ptr<ShaderProgram> > m_ExternalShaders;
    std::vector<std::shared_ptr<MeshObject> >    m_MeshObjs;
    std::shared_ptr<PointLights>                 m_Lights;

    std::vector<std::unique_ptr<FRMeshRender> > m_MeshRenders;
    std::unique_ptr<SkyBoxRender>               m_SkyBoxRender        = nullptr;
    std::unique_ptr<FRPlaneRender>              m_PlaneRender         = nullptr;
    std::unique_ptr<PointLightRender>           m_LightRender         = nullptr;
    std::unique_ptr<WireFrameBoxRender>         m_WireFrameBoxRender  = nullptr;
    std::unique_ptr<ScreenQuadTextureRender>    m_ScreenQuadTexRender = nullptr;
    std::unique_ptr<OffScreenRender>            m_FilterFrameBuffer   = nullptr;
};