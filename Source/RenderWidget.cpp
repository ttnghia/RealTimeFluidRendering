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

#include "RenderWidget.h"
#include <SurfaceReconstruction/AniKernelGenerator.h>

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
RenderWidget::RenderWidget(QWidget* parent) : OpenGLWidget(parent)
{
    setCamera(QtAppUtils::getDefaultCamera(DEFAULT_CAMERA_POSITION, DEFAULT_CAMERA_FOCUS));
    enableRenderBox(false);
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void RenderWidget::initOpenGL()
{
    initRDataFluidFloor();
    initRDataParticle();
    initRDataMeshes();
    initRDataSolidShadow();
    initRDataFluidShadow();
    initRDataSurfaceGeneration();

    initParticleDataObj();
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void RenderWidget::resizeOpenGLWindow(int w, int h)
{
    m_RDataThicknessPass.pointScale = static_cast<GLfloat>(h) / tanf(55.0 * 0.5 * M_PI / 180.0);

    m_RDataDepthPass.offScreenRender->resize(w, h);
    m_RDataThicknessPass.offScreenRender->resize(w, h);
    m_RDataNormalPass.offScreenRender->resize(w, h);
    m_FilterFrameBuffer->resize(w, h);
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void RenderWidget::renderOpenGL()
{
    switch(m_FluidRenderMode) {
        case ParticleRenderMode::SphereParticle:
        {
            if(m_bShadowEnabled) {
                generateSolidLightDepthMaps();
                generateFluidShadowMaps();
                generateFluidShadowThicknessMaps();
                glViewport(0, 0, width(), height());
            }

            renderFluidFloor();
            renderParticles();
            renderMeshes();
            break;
        }

        case ParticleRenderMode::TransparentFluid:
        case ParticleRenderMode::OpaqueSurface:
        {
            renderSolidCameraDepthMap();

            if(m_bShadowEnabled) {
                generateSolidLightDepthMaps();
                generateFluidShadowMaps();
                generateFluidShadowThicknessMaps();
                glViewport(0, 0, width(), height());
            }

            renderDepthPass();
            runDepthFilter();
            renderNormalPass();
            renderThicknessPass();
            filterThickness();
            renderBackgroundPass();
            renderCompositionPass();
            break;
        }

        case ParticleRenderMode::ThicknessBuffer:
        {
            renderSolidCameraDepthMap();
            renderThicknessPass();
            filterThickness();
            renderScreenQuadTex(m_RDataFilterThickness.offScreenRender->getColorBuffer(), 1, 1.0);
            break;
        }

        case ParticleRenderMode::NormalBuffer:
        {
            renderDepthPass();
            runDepthFilter();
            renderNormalPass();
            renderScreenQuadTex(m_RDataNormalPass.offScreenRender->getColorBuffer(), 3);
            break;
        }

        default:
            __BNN_DIE_UNKNOWN_ERROR;
    }

    if(m_bExrportFrameToImage) {
        bool bExported = m_ParticleData->getUInt("FrameExported") > 0;

        if(!bExported) {
            int frame = static_cast<int>(m_ParticleData->getUInt("DataFrame"));

            if(exportScreenToImage(frame)) {
                m_ParticleData->setUInt("FrameExported", 1);
            }
        }
    }

    ////////////////////////////////////////////////////////////////////////////////
    resetClearColor();
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void RenderWidget::reloadShaders()
{
    bool reloadResult = true;
    makeCurrent();
    for(auto& shader : m_ExternalShaders) {
        reloadResult = reloadResult && shader->reloadShaders();
    }
    doneCurrent();

    if(reloadResult) {
        QMessageBox::information(this, "Info", "Successfully reload shaders!");
    }
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void RenderWidget::reloadTextures()
{
    makeCurrent();
    reloadSkyboxTextures();
    reloadFluidFloorTextures();
    doneCurrent();
    ////////////////////////////////////////////////////////////////////////////////
    QMessageBox::information(this, "Info", "Textures reloaded!");
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void RenderWidget::setFluidFloorSize(int size)
{
    // for fluid emitter + sphere mesh
    // for rendering transparent comparison
    //    m_PlaneRender->transform(Vec3f(0.5 + 0.035, 0, 0.5 - 0.035), Vec3f(static_cast<float>(size) + 0.5f) / 16.0f);
    //    m_PlaneRender->scaleTexCoord(static_cast<float>(size) / 16.0f, static_cast<float>(size) / 16.0f);

    //    m_PlaneRender->transform(Vec3f(0.5), Vec3f(static_cast<float>(size)));

    m_FluidFloorRender->scale(Vec3f(static_cast<float>(size)));
    m_FluidFloorRender->scaleTexCoord(static_cast<float>(size), static_cast<float>(size));
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void RenderWidget::initRDataFluidFloor()
{
    Q_ASSERT(m_UBufferCamData != nullptr && m_Lights != nullptr);
    m_FluidFloorRender = std::make_unique<FRPlaneRender>(m_Camera, m_Lights, QDir::currentPath() + "/Textures/Floor/", m_UBufferCamData);
    m_FluidFloorRender->setExposure(0.5f);
    setFluidFloorSize(10);
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void RenderWidget::updateParticleData()
{
    Q_ASSERT(m_RDataParticle.initialized);
    makeCurrent();

    ////////////////////////////////////////////////////////////////////////////////
    // position buffer
    m_RDataParticle.buffPosition->uploadDataAsync(m_ParticleData->getArray("Position")->data(), 0, m_ParticleData->getArray("Position")->size());

    if(m_RDataParticle.useAnisotropyKernel) {
        m_RDataParticle.buffAnisotropyMatrix->uploadDataAsync(m_ParticleData->getArray("AnisotropyKernelMatrix")->data(), 0, m_ParticleData->getArray("AnisotropyKernelMatrix")->size());
    }

    ////////////////////////////////////////////////////////////////////////////////
    // color from data buffer
    if(m_RDataParticle.pColorMode == ParticleColorMode::FromData) {
        Q_ASSERT(m_ParticleData->hasArray("ColorData"));
        m_RDataParticle.buffColorData->uploadDataAsync(m_ParticleData->getArray("ColorData")->data(), 0, m_ParticleData->getArray("ColorData")->size());
    }

    ////////////////////////////////////////////////////////////////////////////////
    // also upload the particle color data
    uploadParticleColorData();

    ////////////////////////////////////////////////////////////////////////////////
    doneCurrent();
    m_RDataParticle.pointRadius  = m_ParticleData->getParticleRadius<GLfloat>();
    m_RDataParticle.numParticles = m_ParticleData->getNParticles();
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void RenderWidget::setParticleColorMode(int colorMode)
{
    Q_ASSERT(colorMode < ParticleColorMode::NumColorMode);
    Q_ASSERT(m_RDataParticle.initialized);

    m_RDataParticle.hasVColor  = colorMode != ParticleColorMode::Uniform ? 1 : 0;
    m_RDataParticle.pColorMode = colorMode;

    ////////////////////////////////////////////////////////////////////////////////
    makeCurrent();
    uploadParticleColorData();
    initParticleVAOs();
    doneCurrent();
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void RenderWidget::setParticleRenderMode(int renderMode)
{
    Q_ASSERT(renderMode < ParticleRenderMode::NumRenderMode);
    m_FluidRenderMode = renderMode;
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void RenderWidget::setParticleMaterial(const Material::MaterialData& material)
{
    makeCurrent();
    m_RDataParticle.material->setMaterial(material);
    m_RDataParticle.material->uploadDataToGPU();
    doneCurrent();
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void RenderWidget::initRDataParticle()
{
    m_RDataParticle.shader = std::make_shared<ShaderProgram>("Shaders/particle.vs.glsl", "Shaders/particle.fs.glsl", "RenderPointSphere");
    m_ExternalShaders.push_back(m_RDataParticle.shader);

    m_RDataParticle.v_Position          = m_RDataParticle.shader->getAtributeLocation("v_Position");
    m_RDataParticle.v_AnisotropyMatrix0 = m_RDataParticle.shader->getAtributeLocation("v_AnisotropyMatrix0");
    m_RDataParticle.v_AnisotropyMatrix1 = m_RDataParticle.shader->getAtributeLocation("v_AnisotropyMatrix1");
    m_RDataParticle.v_AnisotropyMatrix2 = m_RDataParticle.shader->getAtributeLocation("v_AnisotropyMatrix2");
    m_RDataParticle.v_Color             = m_RDataParticle.shader->getAtributeLocation("v_Color");
    m_RDataParticle.u_ClipPlane         = m_RDataParticle.shader->getUniformLocation("u_ClipPlane");

    m_RDataParticle.ub_CamData  = m_RDataParticle.shader->getUniformBlockIndex("CameraData");
    m_RDataParticle.ub_Light    = m_RDataParticle.shader->getUniformBlockIndex("Lights");
    m_RDataParticle.ub_Material = m_RDataParticle.shader->getUniformBlockIndex("Material");

    m_RDataParticle.u_PointRadius = m_RDataParticle.shader->getUniformLocation("u_PointRadius");
    //    m_RDataParticle.u_PointScale = m_RDataParticle.shader->getUniformLocation("u_PointScale");
    m_RDataParticle.u_IsPointView         = m_RDataParticle.shader->getUniformLocation("u_IsPointView");
    m_RDataParticle.u_HasVColor           = m_RDataParticle.shader->getUniformLocation("u_HasVColor");
    m_RDataParticle.u_UseAnisotropyKernel = m_RDataParticle.shader->getUniformLocation("u_UseAnisotropyKernel");
    m_RDataParticle.u_ScreenWidth         = m_RDataParticle.shader->getUniformLocation("u_ScreenWidth");
    m_RDataParticle.u_ScreenHeight        = m_RDataParticle.shader->getUniformLocation("u_ScreenHeight");

    m_RDataParticle.buffPosition = std::make_unique<OpenGLBuffer>();
    m_RDataParticle.buffPosition->createBuffer(GL_ARRAY_BUFFER, 1, nullptr, GL_DYNAMIC_DRAW);

    m_RDataParticle.buffAnisotropyMatrix = std::make_unique<OpenGLBuffer>();
    m_RDataParticle.buffAnisotropyMatrix->createBuffer(GL_ARRAY_BUFFER, 1, nullptr, GL_DYNAMIC_DRAW);

    m_RDataParticle.buffColorRandom = std::make_unique<OpenGLBuffer>();
    m_RDataParticle.buffColorRandom->createBuffer(GL_ARRAY_BUFFER, 1);

    m_RDataParticle.buffColorRamp = std::make_unique<OpenGLBuffer>();
    m_RDataParticle.buffColorRamp->createBuffer(GL_ARRAY_BUFFER, 1);

    m_RDataParticle.buffColorData = std::make_unique<OpenGLBuffer>();
    m_RDataParticle.buffColorData->createBuffer(GL_ARRAY_BUFFER, 1, nullptr, GL_DYNAMIC_DRAW);

    m_RDataParticle.material = std::make_unique<Material>();
    m_RDataParticle.material->setMaterial(CUSTOM_PARTICLE_MATERIAL);
    m_RDataParticle.material->uploadDataToGPU();

    glCall(glGenVertexArrays(1, &m_RDataParticle.VAO));

    m_RDataParticle.initialized = true;
    initParticleVAOs();
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void RenderWidget::initParticleDataObj()
{
    Q_ASSERT(m_ParticleData != nullptr);
    m_ParticleData->setUInt("DataFrame",     0);
    m_ParticleData->setUInt("FrameExported", 0);

    const int     sizeXYZ = 20;
    const GLfloat step    = 2.0 / static_cast<GLfloat>(sizeXYZ - 1);
    m_ParticleData->setNumParticles(sizeXYZ * sizeXYZ * sizeXYZ);
    m_ParticleData->setParticleRadius(0.5 * step * 0.95);

    m_ParticleData->addArray<GLfloat, 3>("Position");
    GLfloat*      dataPtr    = reinterpret_cast<GLfloat*>(m_ParticleData->getArray("Position")->data());
    int           p          = 0;
    const GLfloat randomness = 0.5;

    for(int i = 0; i < sizeXYZ; ++i) {
        for(int j = 0; j < sizeXYZ; ++j) {
            for(int k = 0; k < sizeXYZ; ++k) {
                dataPtr[p++] = -1.0 + static_cast<GLfloat>(i) * step + ((float)rand() / RAND_MAX * 2 - 1) * m_ParticleData->getParticleRadius<float>() * randomness;
                dataPtr[p++] = 0.0 + static_cast<GLfloat>(j) * step + ((float)rand() / RAND_MAX * 2 - 1) * m_ParticleData->getParticleRadius<float>() * randomness;
                dataPtr[p++] = -1.0 + static_cast<GLfloat>(k) * step + ((float)rand() / RAND_MAX * 2 - 1) * m_ParticleData->getParticleRadius<float>() * randomness;
            }
        }
    }

    m_ParticleData->addArray<GLfloat, 3>("ColorRandom");
    m_ParticleData->addArray<GLfloat, 3>("ColorRamp");
    m_ParticleData->setUInt("ColorRandomReady", 0);
    m_ParticleData->setUInt("ColorRampReady",   0);

    computeParticleAnisotropyKernel();
    m_ParticleData->setUInt("AnisotrpyMatrixReady", 1);

    updateParticleData();
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void RenderWidget::initParticleVAOs()
{
    Q_ASSERT(m_RDataParticle.initialized);
    glCall(glBindVertexArray(m_RDataParticle.VAO));
    glCall(glEnableVertexAttribArray(m_RDataParticle.v_Position));

    m_RDataParticle.buffPosition->bind();
    glCall(glVertexAttribPointer(m_RDataParticle.v_Position, 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid*>(0)));

    m_RDataParticle.buffAnisotropyMatrix->bind();
    glCall(glEnableVertexAttribArray(m_RDataParticle.v_AnisotropyMatrix0));
    glCall(glEnableVertexAttribArray(m_RDataParticle.v_AnisotropyMatrix1));
    glCall(glEnableVertexAttribArray(m_RDataParticle.v_AnisotropyMatrix2));
    glCall(glVertexAttribPointer(m_RDataParticle.v_AnisotropyMatrix0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat), reinterpret_cast<GLvoid*>(0)));
    glCall(glVertexAttribPointer(m_RDataParticle.v_AnisotropyMatrix1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat), reinterpret_cast<GLvoid*>(sizeof(GLfloat) * 3)));
    glCall(glVertexAttribPointer(m_RDataParticle.v_AnisotropyMatrix2, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat), reinterpret_cast<GLvoid*>(sizeof(GLfloat) * 6)));

    if(m_RDataParticle.pColorMode != ParticleColorMode::Uniform) {
        glCall(glEnableVertexAttribArray(m_RDataParticle.v_Color));

        if(m_RDataParticle.pColorMode == ParticleColorMode::Random) {
            m_RDataParticle.buffColorRandom->bind();
        } else if(m_RDataParticle.pColorMode == ParticleColorMode::Ramp) {
            m_RDataParticle.buffColorRamp->bind();
        } else if(m_RDataParticle.pColorMode == ParticleColorMode::FromData) {
            m_RDataParticle.buffColorData->bind();
        }

        glCall(glVertexAttribPointer(m_RDataParticle.v_Color, 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid*>(0)));
    }

    glCall(glBindVertexArray(0));
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void RenderWidget::uploadParticleColorData()
{
    if(m_RDataParticle.pColorMode == static_cast<int>(ParticleColorMode::Random)) {
        if(m_ParticleData->getUInt("ColorRandomReady") == 0) {
            m_ParticleData->generateRandomRealData<GLfloat, 3>("ColorRandom", 0, 1);
            m_RDataParticle.buffColorRandom->uploadDataAsync(m_ParticleData->getArray("ColorRandom")->data(), 0, m_ParticleData->getArray("ColorRandom")->size());
            m_ParticleData->setUInt("ColorRandomReady", 1);
        }
    } else if(m_RDataParticle.pColorMode == static_cast<int>(ParticleColorMode::Ramp)) {
        if(m_ParticleData->getUInt("ColorRampReady") == 0) {
            m_ParticleData->generateRampData<GLfloat, Vec3f, 3>("ColorRamp", PARTICLE_COLOR_RAMP);
            m_RDataParticle.buffColorRamp->uploadDataAsync(m_ParticleData->getArray("ColorRamp")->data(), 0, m_ParticleData->getArray("ColorRamp")->size());
            m_ParticleData->setUInt("ColorRampReady", 1);
        }
    }
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void RenderWidget::renderParticles()
{
    Q_ASSERT(m_RDataParticle.initialized);

    m_RDataParticle.shader->bind();

    m_UBufferCamData->bindBufferBase();
    m_RDataParticle.shader->bindUniformBlock(m_RDataParticle.ub_CamData, m_UBufferCamData->getBindingPoint());

    m_Lights->bindUniformBuffer();
    m_RDataParticle.shader->bindUniformBlock(m_RDataParticle.ub_Light, m_Lights->getBufferBindingPoint());

    m_RDataParticle.material->bindUniformBuffer();
    m_RDataParticle.shader->bindUniformBlock(m_RDataParticle.ub_Material, m_RDataParticle.material->getBufferBindingPoint());

    m_RDataParticle.shader->setUniformValue(m_RDataParticle.u_PointRadius,  m_RDataParticle.pointRadius);
    m_RDataParticle.shader->setUniformValue(m_RDataParticle.u_IsPointView,  m_RDataParticle.isPointView);
    m_RDataParticle.shader->setUniformValue(m_RDataParticle.u_HasVColor,    m_RDataParticle.hasVColor);
    m_RDataParticle.shader->setUniformValue(m_RDataParticle.u_ClipPlane,    m_ClipPlane);
    m_RDataParticle.shader->setUniformValue(m_RDataParticle.u_ScreenWidth,  width());
    m_RDataParticle.shader->setUniformValue(m_RDataParticle.u_ScreenHeight, height());

    if(m_RDataParticle.useAnisotropyKernel && m_ParticleData->getUInt("AnisotrpyMatrixReady") == 1) {
        m_RDataParticle.shader->setUniformValue(m_RDataParticle.u_UseAnisotropyKernel, 1);
    } else {
        m_RDataParticle.shader->setUniformValue(m_RDataParticle.u_UseAnisotropyKernel, 0);
    }

    glCall(glBindVertexArray(m_RDataParticle.VAO));
    glCall(glEnable(GL_VERTEX_PROGRAM_POINT_SIZE));
    glCall(glDrawArrays(GL_POINTS, 0, m_RDataParticle.numParticles));
    glCall(glBindVertexArray(0));
    m_RDataParticle.shader->release();
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void RenderWidget::computeParticleAnisotropyKernel()
{
    m_ParticleData->addArray<GLfloat, 9>("AnisotropyKernelMatrix");

    Vec3r*   particles      = reinterpret_cast<Vec3r*>(m_ParticleData->getArray("Position")->data());
    GLfloat* kernelMatrices = reinterpret_cast<GLfloat*>(m_ParticleData->getArray("AnisotropyKernelMatrix")->data());

    AnisotropicKernelGenerator<3, float> aniKernelGenerator(m_ParticleData->getNParticles(), particles, m_ParticleData->getParticleRadius<float>());
    aniKernelGenerator.computeAniKernels();
    memcpy(particles,      aniKernelGenerator.kernelCenters().data(),  sizeof(Vec3r) * m_ParticleData->getNParticles());
    memcpy(kernelMatrices, aniKernelGenerator.kernelMatrices().data(), sizeof(Mat3x3r) * m_ParticleData->getNParticles());
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void RenderWidget::updateMeshes()
{
    makeCurrent();

    for(auto& mesh : m_MeshObjs) {
        mesh->uploadDataToGPU();
    }

    doneCurrent();
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void RenderWidget::updateNumMeshes(int numMeshes)
{
    m_NumMeshes = numMeshes;
    makeCurrent();

    for(auto& meshRender : m_MeshRenders) {
        meshRender->setupVAO();
        meshRender->initDepthBufferData(m_ClearColor);

        meshRender->setSolidShadowMaps(m_RDataSolidLightDepthMap.solidShadowMaps);
        meshRender->setFluidShadowMaps(m_RDataFluidShadowMap.fluidShadowMaps);
        meshRender->setFluidShadowThickness(m_RDataFluidShadowThickness.fluidShadowThickness);
    }

    initRDataSolidShadow();
    doneCurrent();
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void RenderWidget::setMeshMaterial(const Material::MaterialData& material, int meshID)
{
    makeCurrent();
    m_MeshRenders[meshID]->getMaterial()->setMaterial(material);
    m_MeshRenders[meshID]->getMaterial()->uploadDataToGPU();
    doneCurrent();
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void RenderWidget::initRDataMeshes()
{
    Q_ASSERT(m_UBufferCamData != nullptr && m_Lights != nullptr);

    for(int i = 0; i < MAX_NUM_MESHES; ++i) {
        std::shared_ptr<MeshObject>   mesh       = std::make_shared<MeshObject>();;
        std::unique_ptr<FRMeshRender> meshRender = std::make_unique<FRMeshRender>(mesh, m_Camera, m_Lights, nullptr, m_UBufferCamData);
        m_ExternalShaders.push_back(meshRender->getShader());

        meshRender->getMaterial()->setMaterial(DEFAULT_MESH_MATERIAL);
        meshRender->getMaterial()->uploadDataToGPU();

        m_MeshObjs.push_back(std::move(mesh));;
        m_MeshRenders.push_back(std::move(meshRender));
    }
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void RenderWidget::renderMeshes()
{
    for(int i = 0; i < m_NumMeshes; ++i) {
        auto& meshRender = m_MeshRenders[i];
        meshRender->render(m_bShadowEnabled, m_bVisualizeShadow, m_ShadowIntensity);
    }
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void RenderWidget::setFluidViewMaterial(const Material::MaterialData& material)
{
    makeCurrent();
    m_RDataCompositionPass.fluidVolumeMaterial->setMaterial(material);
    m_RDataCompositionPass.fluidVolumeMaterial->uploadDataToGPU();
    doneCurrent();
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void RenderWidget::initRDataFluidShadow()
{
    for(int i = 0; i < MAX_NUM_LIGHTS; ++i) {
        m_RDataFluidShadowMap.depthMapRenders.push_back(std::make_unique<DepthBufferRender>(SHADOWMAP_TEXTURE_SIZE, SHADOWMAP_TEXTURE_SIZE));
        m_RDataFluidShadowMap.depthMapRenders[i]->setClearDepthValue(CLEAR_DEPTH_VALUE);
    }

    m_RDataFluidShadowMap.shader = std::make_shared<ShaderProgram>("Shaders/fluid-shadow.vs.glsl", "Shaders/fluid-shadow.fs.glsl", "FluidShadow");
    m_ExternalShaders.push_back(m_RDataFluidShadowMap.shader);

    m_RDataFluidShadowMap.v_Position          = m_RDataFluidShadowMap.shader->getAtributeLocation("v_Position");
    m_RDataFluidShadowMap.v_AnisotropyMatrix0 = m_RDataFluidShadowMap.shader->getAtributeLocation("v_AnisotropyMatrix0");
    m_RDataFluidShadowMap.v_AnisotropyMatrix1 = m_RDataFluidShadowMap.shader->getAtributeLocation("v_AnisotropyMatrix1");
    m_RDataFluidShadowMap.v_AnisotropyMatrix2 = m_RDataFluidShadowMap.shader->getAtributeLocation("v_AnisotropyMatrix2");

    m_RDataFluidShadowMap.ub_LightMatrices = m_RDataFluidShadowMap.shader->getUniformBlockIndex("LightMatrices");
    m_RDataFluidShadowMap.u_LightID        = m_RDataFluidShadowMap.shader->getUniformLocation("u_LightID");
    m_RDataFluidShadowMap.u_PointRadius    = m_RDataFluidShadowMap.shader->getUniformLocation("u_PointRadius");
    //    m_RDataFluidShadowMap.u_PointScale     = m_RDataFluidShadowMap.shader->getUniformLocation("u_PointScale");
    m_RDataFluidShadowMap.u_UseAnisotropyKernel = m_RDataFluidShadowMap.shader->getUniformLocation("u_UseAnisotropyKernel");
    m_RDataFluidShadowMap.u_ScreenWidth         = m_RDataFluidShadowMap.shader->getUniformLocation("u_ScreenWidth");
    m_RDataFluidShadowMap.u_ScreenHeight        = m_RDataFluidShadowMap.shader->getUniformLocation("u_ScreenHeight");

    glCall(glGenVertexArrays(1, &m_RDataFluidShadowMap.VAO));
    glCall(glBindVertexArray(m_RDataFluidShadowMap.VAO));
    glCall(glEnableVertexAttribArray(m_RDataFluidShadowMap.v_Position));
    m_RDataParticle.buffPosition->bind();
    glCall(glVertexAttribPointer(m_RDataFluidShadowMap.v_Position, 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid*>(0)));

    // anisotropy kernel
    m_RDataParticle.buffAnisotropyMatrix->bind();
    glCall(glEnableVertexAttribArray(m_RDataFluidShadowMap.v_AnisotropyMatrix0));
    glCall(glEnableVertexAttribArray(m_RDataFluidShadowMap.v_AnisotropyMatrix1));
    glCall(glEnableVertexAttribArray(m_RDataFluidShadowMap.v_AnisotropyMatrix2));
    glCall(glVertexAttribPointer(m_RDataFluidShadowMap.v_AnisotropyMatrix0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat), reinterpret_cast<GLvoid*>(0)));
    glCall(glVertexAttribPointer(m_RDataFluidShadowMap.v_AnisotropyMatrix1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat), reinterpret_cast<GLvoid*>(sizeof(GLfloat) * 3)));
    glCall(glVertexAttribPointer(m_RDataFluidShadowMap.v_AnisotropyMatrix2, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat), reinterpret_cast<GLvoid*>(sizeof(GLfloat) * 6)));

    glCall(glBindVertexArray(0));

    m_RDataFluidShadowMap.initialized = true;

    ////////////////////////////////////////////////////////////////////////////////
    for(int i = 0; i < MAX_NUM_LIGHTS; ++i) {
        m_RDataFluidShadowThickness.offScreenRenders.push_back(std::make_unique<OffScreenRender>(SHADOWMAP_TEXTURE_SIZE * SHADOWMAP_TEXTURE_SIZE_RATIO,
                                                                                                 SHADOWMAP_TEXTURE_SIZE * SHADOWMAP_TEXTURE_SIZE_RATIO,
                                                                                                 1, GL_R16F));
    }

    m_RDataFluidShadowThickness.shader = std::make_shared<ShaderProgram>("Shaders/thickness-shadow.vs.glsl", "Shaders/thickness-shadow.fs.glsl", "ThicknessShadow");
    m_ExternalShaders.push_back(m_RDataFluidShadowThickness.shader);

    m_RDataFluidShadowThickness.v_Position       = m_RDataFluidShadowThickness.shader->getAtributeLocation("v_Position");
    m_RDataFluidShadowThickness.ub_LightMatrices = m_RDataFluidShadowThickness.shader->getUniformBlockIndex("LightMatrices");
    m_RDataFluidShadowThickness.u_LightID        = m_RDataFluidShadowThickness.shader->getUniformLocation("u_LightID");
    m_RDataFluidShadowThickness.u_PointRadius    = m_RDataFluidShadowThickness.shader->getUniformLocation("u_PointRadius");
    m_RDataFluidShadowThickness.u_PointScale     = m_RDataFluidShadowThickness.shader->getUniformLocation("u_PointScale");

    glCall(glGenVertexArrays(1, &m_RDataFluidShadowThickness.VAO));
    glCall(glBindVertexArray(m_RDataFluidShadowThickness.VAO));
    glCall(glEnableVertexAttribArray(m_RDataFluidShadowThickness.v_Position));
    m_RDataParticle.buffPosition->bind();
    glCall(glVertexAttribPointer(m_RDataFluidShadowThickness.v_Position, 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid*>(0)));
    glCall(glBindVertexArray(0));

    m_RDataFluidShadowThickness.initialized = true;

    ////////////////////////////////////////////////////////////////////////////////
    for(int i = 0; i < MAX_NUM_LIGHTS; ++i) {
        m_RDataFluidShadowMap.fluidShadowMaps.push_back(m_RDataFluidShadowMap.depthMapRenders[i]->getDepthBuffer());
        m_RDataFluidShadowThickness.fluidShadowThickness.push_back(m_RDataFluidShadowThickness.offScreenRenders[i]->getColorBuffer());
    }

    m_FluidFloorRender->setFluidShadowMaps(m_RDataFluidShadowMap.fluidShadowMaps);
    m_FluidFloorRender->setFluidShadowThickness(m_RDataFluidShadowThickness.fluidShadowThickness);
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void RenderWidget::generateFluidShadowMaps()
{
    Q_ASSERT(m_RDataFluidShadowMap.initialized);

    m_RDataFluidShadowMap.shader->bind();
    m_Lights->bindUniformBufferLightMatrix();
    m_RDataFluidShadowMap.shader->bindUniformBlock(m_RDataFluidShadowMap.ub_LightMatrices, m_Lights->getBufferLightMatrixBindingPoint());
    m_RDataFluidShadowMap.shader->setUniformValue(m_RDataFluidShadowMap.u_PointRadius, m_RDataParticle.pointRadius);

    //    m_RDataFluidShadowMap.shader->setUniformValue(m_RDataFluidShadowMap.u_PointScale, m_RDataThicknessPass.pointScale);
    if(m_RDataParticle.useAnisotropyKernel && (m_ParticleData != nullptr && m_ParticleData->getUInt("AnisotrpyMatrixReady") == 1)) {
        m_RDataFluidShadowMap.shader->setUniformValue(m_RDataFluidShadowMap.u_UseAnisotropyKernel, 1);
        m_RDataFluidShadowMap.shader->setUniformValue(m_RDataFluidShadowMap.u_ScreenWidth,         static_cast<int>(SHADOWMAP_TEXTURE_SIZE));
        m_RDataFluidShadowMap.shader->setUniformValue(m_RDataFluidShadowMap.u_ScreenHeight,        static_cast<int>(SHADOWMAP_TEXTURE_SIZE));
    } else {
        m_RDataFluidShadowMap.shader->setUniformValue(m_RDataFluidShadowMap.u_UseAnisotropyKernel, 0);
    }

    glCall(glBindVertexArray(m_RDataFluidShadowMap.VAO));

    for(int i = 0; i < m_Lights->getNumLights(); ++i) {
        m_RDataFluidShadowMap.shader->setUniformValue(m_RDataFluidShadowMap.u_LightID, i);
        m_RDataFluidShadowMap.depthMapRenders[i]->beginRender();

        glCall(glEnable(GL_VERTEX_PROGRAM_POINT_SIZE));
        glCall(glDrawArrays(GL_POINTS, 0, m_ParticleData->getNParticles()));

        m_RDataFluidShadowMap.depthMapRenders[i]->endRender(context()->defaultFramebufferObject());
    }

    glCall(glBindVertexArray(0));
    m_RDataFluidShadowMap.shader->release();
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void RenderWidget::generateFluidShadowThicknessMaps()
{
    Q_ASSERT(m_RDataFluidShadowThickness.initialized);
    glCall(glClearColor(0, 0, 0, 1.0));
    glCall(glDisable(GL_DEPTH_TEST));
    glCall(glDepthMask(GL_FALSE));
    glCall(glEnable(GL_BLEND));
    glCall(glBlendFunc(GL_ONE, GL_ONE));

    m_RDataFluidShadowThickness.shader->bind();
    glCall(glBindVertexArray(m_RDataFluidShadowThickness.VAO));

    ////////////////////////////////////////////////////////////////////////////////
    m_Lights->bindUniformBufferLightMatrix();
    m_RDataFluidShadowThickness.shader->bindUniformBlock(m_RDataFluidShadowThickness.ub_LightMatrices, m_Lights->getBufferLightMatrixBindingPoint());
    m_RDataFluidShadowThickness.shader->setUniformValue(m_RDataFluidShadowThickness.u_PointRadius, m_RDataParticle.pointRadius * 1.5f);
    m_RDataFluidShadowThickness.shader->setUniformValue(m_RDataFluidShadowThickness.u_PointScale,  m_RDataThicknessPass.pointScale);

    for(int i = 0; i < m_Lights->getNumLights(); ++i) {
        m_RDataFluidShadowThickness.shader->setUniformValue(m_RDataFluidShadowThickness.u_LightID, i);
        m_RDataFluidShadowThickness.offScreenRenders[i]->beginRender();
        glCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

        glCall(glEnable(GL_VERTEX_PROGRAM_POINT_SIZE));
        glCall(glDrawArrays(GL_POINTS, 0, m_ParticleData->getNParticles()));
        m_RDataFluidShadowThickness.offScreenRenders[i]->endRender(context()->defaultFramebufferObject());
    }

    glCall(glBindVertexArray(0));
    m_RDataFluidShadowThickness.shader->release();

    ////////////////////////////////////////////////////////////////////////////////
    glCall(glDisable(GL_BLEND));
    glCall(glDepthMask(GL_TRUE));
    glCall(glEnable(GL_DEPTH_TEST));
    resetClearColor();
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void RenderWidget::initRDataSolidShadow()
{
    ////////////////////////////////////////////////////////////////////////////////
    // light depth buffer
    if(!m_RDataSolidLightDepthMap.initialized) {
        for(int i = 0; i < MAX_NUM_LIGHTS; ++i) {
            m_RDataSolidLightDepthMap.depthMapRenders.push_back(std::make_unique<DepthBufferRender>(SHADOWMAP_TEXTURE_SIZE, SHADOWMAP_TEXTURE_SIZE));
            m_RDataSolidLightDepthMap.depthMapRenders[i]->setDefaultClearColor(m_ClearColor);
            m_RDataSolidLightDepthMap.depthMapRenders[i]->setClearDepthValue(-1000000.0);
        }

        m_RDataSolidLightDepthMap.shader           = ShaderProgram::getSimpleLightSpaceDepthShader();
        m_RDataSolidLightDepthMap.v_Position       = m_RDataSolidLightDepthMap.shader->getAtributeLocation("v_Position");
        m_RDataSolidLightDepthMap.ub_ModelMatrix   = m_RDataSolidLightDepthMap.shader->getUniformBlockIndex("ModelMatrix");
        m_RDataSolidLightDepthMap.ub_LightMatrices = m_RDataSolidLightDepthMap.shader->getUniformBlockIndex("LightMatrices");
        m_RDataSolidLightDepthMap.u_LightID        = m_RDataSolidLightDepthMap.shader->getUniformLocation("u_LightID");

        glm::mat4 modelMatrix = glm::mat4(1.0f);
        m_RDataSolidLightDepthMap.bufferModelMatrix = std::make_shared<OpenGLBuffer>();
        m_RDataSolidLightDepthMap.bufferModelMatrix->createBuffer(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), nullptr, GL_STATIC_DRAW);
        m_RDataSolidLightDepthMap.bufferModelMatrix->uploadData(glm::value_ptr(modelMatrix), 0,                 sizeof(glm::mat4));
        m_RDataSolidLightDepthMap.bufferModelMatrix->uploadData(glm::value_ptr(modelMatrix), sizeof(glm::mat4), sizeof(glm::mat4)); // normal matrix}

        m_RDataSolidLightDepthMap.initialized = true;

        ////////////////////////////////////////////////////////////////////////////////
        for(int i = 0; i < MAX_NUM_LIGHTS; ++i) {
            m_RDataSolidLightDepthMap.solidShadowMaps.push_back(m_RDataSolidLightDepthMap.depthMapRenders[i]->getDepthBuffer());
        }

        m_FluidFloorRender->setSolidShadowMaps(m_RDataSolidLightDepthMap.solidShadowMaps);
    }

    for(int i = 0; i < m_MeshObjs.size(); ++i) {
        if(m_MeshObjs[i]->isEmpty()) {
            continue;
        }

        glCall(glGenVertexArrays(1, &m_RDataSolidLightDepthMap.VAOs[i]));
        glCall(glBindVertexArray(m_RDataSolidLightDepthMap.VAOs[i]));
        m_MeshObjs[i]->getVertexBuffer()->bind();
        glCall(glEnableVertexAttribArray(m_RDataSolidLightDepthMap.v_Position));
        glCall(glVertexAttribPointer(m_RDataSolidLightDepthMap.v_Position, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0));

        if(m_MeshObjs[i]->hasIndexBuffer()) {
            m_MeshObjs[i]->getIndexBuffer()->bind();
        }

        glCall(glBindVertexArray(0));
        m_MeshObjs[i]->getVertexBuffer()->release();

        if(m_MeshObjs[i]->hasIndexBuffer()) {
            m_MeshObjs[i]->getIndexBuffer()->release();
        }
    }

    ////////////////////////////////////////////////////////////////////////////////
    // camera depth buffer
    if(!m_RDataSolidCameraDepthMap.initialized) {
        m_RDataSolidCameraDepthMap.depthMapRender = std::make_unique<DepthBufferRender>(SHADOWMAP_TEXTURE_SIZE, SHADOWMAP_TEXTURE_SIZE);
        m_RDataSolidCameraDepthMap.depthMapRender->setDefaultClearColor(m_ClearColor);
        m_RDataSolidCameraDepthMap.depthMapRender->setClearDepthValue(-1000000.0);

        m_RDataSolidCameraDepthMap.shader         = ShaderProgram::getSimpleCameraSpaceDepthShader();
        m_RDataSolidCameraDepthMap.v_Position     = m_RDataSolidCameraDepthMap.shader->getAtributeLocation("v_Position");
        m_RDataSolidCameraDepthMap.ub_ModelMatrix = m_RDataSolidCameraDepthMap.shader->getUniformBlockIndex("ModelMatrix");
        m_RDataSolidCameraDepthMap.ub_CameraData  = m_RDataSolidCameraDepthMap.shader->getUniformBlockIndex("CameraData");

        m_RDataSolidCameraDepthMap.initialized = true;
    }

    for(int i = 0; i < m_MeshObjs.size(); ++i) {
        if(m_MeshObjs[i]->isEmpty()) {
            continue;
        }

        glCall(glGenVertexArrays(1, &m_RDataSolidCameraDepthMap.VAOs[i]));
        glCall(glBindVertexArray(m_RDataSolidCameraDepthMap.VAOs[i]));
        m_MeshObjs[i]->getVertexBuffer()->bind();
        glCall(glEnableVertexAttribArray(m_RDataSolidCameraDepthMap.v_Position));
        glCall(glVertexAttribPointer(m_RDataSolidCameraDepthMap.v_Position, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0));

        if(m_MeshObjs[i]->hasIndexBuffer()) {
            m_MeshObjs[i]->getIndexBuffer()->bind();
        }

        glCall(glBindVertexArray(0));
        m_MeshObjs[i]->getVertexBuffer()->release();

        if(m_MeshObjs[i]->hasIndexBuffer()) {
            m_MeshObjs[i]->getIndexBuffer()->release();
        }
    }
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void RenderWidget::generateSolidLightDepthMaps()
{
    //    if(m_NumMeshes == 0)
    //        return;

    assert(m_RDataSolidLightDepthMap.initialized);
    assert(m_RDataSolidLightDepthMap.depthMapRenders.size() != 0);

    m_RDataSolidLightDepthMap.shader->bind();
    m_RDataSolidLightDepthMap.bufferModelMatrix->bindBufferBase();
    m_RDataSolidLightDepthMap.shader->bindUniformBlock(m_RDataSolidLightDepthMap.ub_ModelMatrix, m_RDataSolidLightDepthMap.bufferModelMatrix->getBindingPoint());

    m_Lights->bindUniformBufferLightMatrix();
    m_RDataSolidLightDepthMap.shader->bindUniformBlock(m_RDataSolidLightDepthMap.ub_LightMatrices, m_Lights->getBufferLightMatrixBindingPoint());

    for(int i = 0; i < m_Lights->getNumLights(); ++i) {
        m_RDataSolidLightDepthMap.shader->setUniformValue(m_RDataSolidLightDepthMap.u_LightID, i);
        m_RDataSolidLightDepthMap.depthMapRenders[i]->beginRender();

        for(int j = 0; j < m_NumMeshes; ++j) {
            glCall(glBindVertexArray(m_RDataSolidLightDepthMap.VAOs[j]));
            m_MeshObjs[j]->draw();
            glCall(glBindVertexArray(0));
        }

        m_RDataSolidLightDepthMap.depthMapRenders[i]->endRender(context()->defaultFramebufferObject());
    }

    m_RDataSolidLightDepthMap.shader->release();
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void RenderWidget::renderSolidCameraDepthMap()
{
    if(m_NumMeshes == 0) {
        return;
    }

    assert(m_RDataSolidCameraDepthMap.initialized);

    m_RDataSolidCameraDepthMap.shader->bind();
    m_RDataSolidLightDepthMap.bufferModelMatrix->bindBufferBase();
    m_RDataSolidCameraDepthMap.shader->bindUniformBlock(m_RDataSolidCameraDepthMap.ub_ModelMatrix, m_RDataSolidLightDepthMap.bufferModelMatrix->getBindingPoint());
    m_UBufferCamData->bindBufferBase();
    m_RDataSolidCameraDepthMap.shader->bindUniformBlock(m_RDataSolidCameraDepthMap.ub_CameraData, m_UBufferCamData->getBindingPoint());

    m_RDataSolidCameraDepthMap.depthMapRender->beginRender();

    for(int j = 0; j < m_NumMeshes; ++j) {
        glCall(glBindVertexArray(m_RDataSolidCameraDepthMap.VAOs[j]));
        m_MeshObjs[j]->draw();
        glCall(glBindVertexArray(0));
    }

    m_RDataSolidCameraDepthMap.depthMapRender->endRender(context()->defaultFramebufferObject());

    m_RDataSolidCameraDepthMap.shader->release();
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void RenderWidget::initRDataSurfaceGeneration()
{
    initRDataScreenQuadTexRender();
    initRDataDepthPass();
    initRDataThicknessPass();
    initRDataThicknessFilter();
    initRDataNormalPass();

    initRDataBackgroundPass();
    initRDataCompositionPass();

    initRDataFilterBiGaussian();
    initRDataFilterCurvatureFlow();
    initRDataFilterPlaneFitting();
    initRDataNarrowRangeFilter();

    m_FilterFrameBuffer = std::make_unique<OffScreenRender>(width(), height(), 1, GL_R32F);
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void RenderWidget::initRDataScreenQuadTexRender()
{
    m_ScreenQuadTexRender = std::make_unique<ScreenQuadTextureRender>();
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void RenderWidget::renderScreenQuadTex(const std::shared_ptr<OpenGLTexture>& texture, int texelSize /*= 1*/, float scaleValue /*= 1.0*/)
{
    Q_ASSERT(m_ScreenQuadTexRender != nullptr);

    m_ScreenQuadTexRender->setValueScale(scaleValue);
    m_ScreenQuadTexRender->setTexture(texture, texelSize);
    glCall(glViewport(0, 0, width(), height()));
    glCall(glClearColor(0, 0, 0, 1));
    glCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

    m_ScreenQuadTexRender->render();
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void RenderWidget::runDepthFilter()
{
    if(m_NumFilterIteration == 0) {
        m_RDataCompositionPass.filteredDepthTex = m_RDataDepthPass.offScreenRender->getColorBuffer();
        return;
    }

#ifdef BENCHMARK_FILTER
    static AvgTimer avgTimer;
    connect(&avgTimer, &AvgTimer::avgTimeChanged, [](double avgTime)
            {
                qDebug() << "Avg time: " << avgTime;
            });

    glCall(glFinish());
    avgTimer.tick();
#endif
    switch(m_FilterMethod) {
        case FilterMethod::BilateralGaussian:
        {
            filterBiGaussian();
            break;
        }

        case FilterMethod::CurvatureFlow:
        {
            filterCurvatureFlow();
            break;
        }

        case FilterMethod::PlaneFitting:
        {
            renderNormalPass(true);
            filterPlaneFitting();
            break;
        }

        case FilterMethod::NarrowRangeFilter1D2D:
        case FilterMethod::NarrowRangeFilter1D:
        case FilterMethod::NarrowRangeFilter2D:
        {
            filterNarrowRange();
            break;
        }

        default:
            __BNN_DIE_UNKNOWN_ERROR;
            ;
    }

    m_RDataCompositionPass.filteredDepthTex = m_FilterFrameBuffer->getColorBuffer();
    m_RDataDepthPass.offScreenRender->swapColorBuffer(m_RDataCompositionPass.depthTex);

#ifdef BENCHMARK_FILTER
    glCall(glFinish());
    avgTimer.tock();
#endif
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void RenderWidget::initRDataDepthPass()
{
    m_RDataDepthPass.offScreenRender = std::make_unique<OffScreenRender>(width(), height(), 1, GL_R32F);

    m_RDataDepthPass.shader = std::make_shared<ShaderProgram>("Shaders/depth-pass.vs.glsl", "Shaders/depth-pass.fs.glsl", "RenderDepthPass");
    m_ExternalShaders.push_back(m_RDataDepthPass.shader);

    m_RDataDepthPass.v_Position          = m_RDataDepthPass.shader->getAtributeLocation("v_Position");
    m_RDataDepthPass.v_AnisotropyMatrix0 = m_RDataDepthPass.shader->getAtributeLocation("v_AnisotropyMatrix0");
    m_RDataDepthPass.v_AnisotropyMatrix1 = m_RDataDepthPass.shader->getAtributeLocation("v_AnisotropyMatrix1");
    m_RDataDepthPass.v_AnisotropyMatrix2 = m_RDataDepthPass.shader->getAtributeLocation("v_AnisotropyMatrix2");

    m_RDataDepthPass.ub_CamData            = m_RDataDepthPass.shader->getUniformBlockIndex("CameraData");
    m_RDataDepthPass.u_PointRadius         = m_RDataDepthPass.shader->getUniformLocation("u_PointRadius");
    m_RDataDepthPass.u_UseAnisotropyKernel = m_RDataDepthPass.shader->getUniformLocation("u_UseAnisotropyKernel");
    m_RDataDepthPass.u_ScreenWidth         = m_RDataDepthPass.shader->getUniformLocation("u_ScreenWidth");
    m_RDataDepthPass.u_ScreenHeight        = m_RDataDepthPass.shader->getUniformLocation("u_ScreenHeight");

    Q_ASSERT(m_RDataParticle.initialized);
    glCall(glGenVertexArrays(1, &m_RDataDepthPass.VAO));
    glCall(glBindVertexArray(m_RDataDepthPass.VAO));
    m_RDataParticle.buffPosition->bind();
    glCall(glEnableVertexAttribArray(m_RDataDepthPass.v_Position));
    glCall(glVertexAttribPointer(m_RDataDepthPass.v_Position, 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid*>(0)));

    // anisotropy kernel
    m_RDataParticle.buffAnisotropyMatrix->bind();
    glCall(glEnableVertexAttribArray(m_RDataDepthPass.v_AnisotropyMatrix0));
    glCall(glEnableVertexAttribArray(m_RDataDepthPass.v_AnisotropyMatrix1));
    glCall(glEnableVertexAttribArray(m_RDataDepthPass.v_AnisotropyMatrix2));
    glCall(glVertexAttribPointer(m_RDataDepthPass.v_AnisotropyMatrix0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat), reinterpret_cast<GLvoid*>(0)));
    glCall(glVertexAttribPointer(m_RDataDepthPass.v_AnisotropyMatrix1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat), reinterpret_cast<GLvoid*>(sizeof(GLfloat) * 3)));
    glCall(glVertexAttribPointer(m_RDataDepthPass.v_AnisotropyMatrix2, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat), reinterpret_cast<GLvoid*>(sizeof(GLfloat) * 6)));

    glCall(glBindVertexArray(0));

    m_RDataDepthPass.initialized = true;
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void RenderWidget::renderDepthPass()
{
    Q_ASSERT(m_RDataDepthPass.initialized);

    m_RDataDepthPass.offScreenRender->beginRender();
    glCall(glClearColor(CLEAR_DEPTH_VALUE, CLEAR_DEPTH_VALUE, CLEAR_DEPTH_VALUE, 1.0));
    glCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    glCall(glEnable(GL_DEPTH_TEST));

    ////////////////////////////////////////////////////////////////////////////////
    m_RDataDepthPass.shader->bind();

    m_UBufferCamData->bindBufferBase();
    m_RDataDepthPass.shader->bindUniformBlock(m_RDataDepthPass.ub_CamData, m_UBufferCamData->getBindingPoint());
    m_RDataDepthPass.shader->setUniformValue(m_RDataDepthPass.u_PointRadius,  m_RDataParticle.pointRadius * 1.5f);
    m_RDataDepthPass.shader->setUniformValue(m_RDataDepthPass.u_ScreenWidth,  width());
    m_RDataDepthPass.shader->setUniformValue(m_RDataDepthPass.u_ScreenHeight, height());

    if(m_RDataParticle.useAnisotropyKernel && (m_ParticleData != nullptr && m_ParticleData->getUInt("AnisotrpyMatrixReady") == 1)) {
        m_RDataDepthPass.shader->setUniformValue(m_RDataDepthPass.u_UseAnisotropyKernel, 1);
    } else {
        m_RDataDepthPass.shader->setUniformValue(m_RDataDepthPass.u_UseAnisotropyKernel, 0);
    }

    glCall(glBindVertexArray(m_RDataDepthPass.VAO));
    glCall(glEnable(GL_VERTEX_PROGRAM_POINT_SIZE));
    glCall(glDrawArrays(GL_POINTS, 0, m_ParticleData->getNParticles()));
    glCall(glBindVertexArray(0));
    m_RDataDepthPass.shader->release();

    ////////////////////////////////////////////////////////////////////////////////
    m_RDataDepthPass.offScreenRender->endRender(context()->defaultFramebufferObject());
    m_RDataCompositionPass.depthTex = m_RDataDepthPass.offScreenRender->getColorBuffer();
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void RenderWidget::initRDataThicknessPass()
{
    m_RDataThicknessPass.offScreenRender = std::make_unique<OffScreenRender>(static_cast<int>(width() * THICKNESS_TEXTURE_SIZE_RATIO),
                                                                             static_cast<int>(height() * THICKNESS_TEXTURE_SIZE_RATIO),
                                                                             1, GL_R16F);

    m_RDataThicknessPass.shader = std::make_shared<ShaderProgram>("Shaders/thickness-pass.vs.glsl", "Shaders/thickness-pass.fs.glsl", "RenderThicknessPass");
    m_ExternalShaders.push_back(m_RDataThicknessPass.shader);

    m_RDataThicknessPass.v_Position      = m_RDataThicknessPass.shader->getAtributeLocation("v_Position");
    m_RDataThicknessPass.ub_CamData      = m_RDataThicknessPass.shader->getUniformBlockIndex("CameraData");
    m_RDataThicknessPass.u_PointRadius   = m_RDataThicknessPass.shader->getUniformLocation("u_PointRadius");
    m_RDataThicknessPass.u_PointScale    = m_RDataThicknessPass.shader->getUniformLocation("u_PointScale");
    m_RDataThicknessPass.u_HasSolid      = m_RDataThicknessPass.shader->getUniformLocation("u_HasSolid");
    m_RDataThicknessPass.u_SolidDepthMap = m_RDataThicknessPass.shader->getUniformLocation("u_SolidDepthMap");

    Q_ASSERT(m_RDataParticle.initialized);
    glCall(glGenVertexArrays(1, &m_RDataThicknessPass.VAO));
    glCall(glBindVertexArray(m_RDataThicknessPass.VAO));
    glCall(glEnableVertexAttribArray(m_RDataThicknessPass.v_Position));

    m_RDataParticle.buffPosition->bind();
    glCall(glVertexAttribPointer(m_RDataThicknessPass.v_Position, 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid*>(0)));
    glCall(glBindVertexArray(0));

    m_RDataThicknessPass.initialized = true;
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void RenderWidget::renderThicknessPass()
{
    Q_ASSERT(m_RDataThicknessPass.initialized);

    m_RDataThicknessPass.offScreenRender->beginRender();
    glCall(glClearColor(0, 0, 0, 1.0));
    glCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    glCall(glDepthMask(GL_FALSE));
    glCall(glDisable(GL_DEPTH_TEST));
    glCall(glEnable(GL_BLEND));
    glCall(glBlendFunc(GL_ONE, GL_ONE));

    ////////////////////////////////////////////////////////////////////////////////
    m_RDataThicknessPass.shader->bind();

    m_UBufferCamData->bindBufferBase();
    m_RDataThicknessPass.shader->bindUniformBlock(m_RDataThicknessPass.ub_CamData, m_UBufferCamData->getBindingPoint());
    m_RDataThicknessPass.shader->setUniformValue(m_RDataThicknessPass.u_PointRadius, m_RDataParticle.pointRadius * 1.2f);
    m_RDataThicknessPass.shader->setUniformValue(m_RDataThicknessPass.u_PointScale,  m_RDataThicknessPass.pointScale);

    if(m_NumMeshes == 0) {
        m_RDataThicknessPass.shader->setUniformValue(m_RDataThicknessPass.u_HasSolid, 0);
    } else {
        m_RDataThicknessPass.shader->setUniformValue(m_RDataThicknessPass.u_HasSolid,      1);
        m_RDataThicknessPass.shader->setUniformValue(m_RDataThicknessPass.u_SolidDepthMap, 0);
        m_RDataSolidCameraDepthMap.depthMapRender->getDepthBuffer()->bind();
    }

    glCall(glBindVertexArray(m_RDataThicknessPass.VAO));
    glCall(glEnable(GL_VERTEX_PROGRAM_POINT_SIZE));
    glCall(glDrawArrays(GL_POINTS, 0, m_ParticleData->getNParticles()));
    glCall(glBindVertexArray(0));
    m_RDataThicknessPass.shader->release();

    if(m_NumMeshes != 0) {
        m_RDataSolidCameraDepthMap.depthMapRender->getDepthBuffer()->release();
    }

    ////////////////////////////////////////////////////////////////////////////////
    glCall(glDepthMask(GL_TRUE));
    glCall(glEnable(GL_DEPTH_TEST));
    glCall(glDisable(GL_BLEND));
    m_RDataThicknessPass.offScreenRender->endRender(context()->defaultFramebufferObject());
    m_RDataCompositionPass.thicknessTex = m_RDataThicknessPass.offScreenRender->getColorBuffer();
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void RenderWidget::initRDataThicknessFilter()
{
    m_RDataFilterThickness.offScreenRender = std::make_unique<OffScreenRender>(static_cast<int>(width() * THICKNESS_TEXTURE_SIZE_RATIO),
                                                                               static_cast<int>(height() * THICKNESS_TEXTURE_SIZE_RATIO),
                                                                               1, GL_R16F);
    m_RDataFilterThickness.shader = ShaderProgram::getScreenQuadShaderFromFile("Shaders/thickness-filter.fs.glsl", "ThicknessFilterShader");
    m_ExternalShaders.push_back(m_RDataFilterThickness.shader);

    m_RDataFilterThickness.u_ThicknessTex = m_RDataFilterThickness.shader->getUniformLocation("u_ThicknessTex");
    m_RDataFilterThickness.u_FilterSize   = m_RDataFilterThickness.shader->getUniformLocation("u_FilterSize");
    m_RDataFilterThickness.u_ScreenWidth  = m_RDataFilterThickness.shader->getUniformLocation("u_ScreenWidth");
    m_RDataFilterThickness.u_ScreenHeight = m_RDataFilterThickness.shader->getUniformLocation("u_ScreenHeight");

    glCall(glGenVertexArrays(1, &m_RDataFilterThickness.VAO));

    m_RDataFilterThickness.initialized = true;
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void RenderWidget::filterThickness()
{
    return;
    __BNN_TODO_MSG("Should we filter thickness?");
    ////////////////////////////////////////////////////////////////////////////////
    Q_ASSERT(m_RDataFilterThickness.initialized);
    Q_ASSERT(m_RDataCompositionPass.thicknessTex != nullptr);

    ////////////////////////////////////////////////////////////////////////////////
    m_RDataFilterThickness.shader->bind();
    m_RDataFilterThickness.shader->setUniformValue(m_RDataFilterThickness.u_ThicknessTex, 0);
    m_RDataFilterThickness.shader->setUniformValue(m_RDataFilterThickness.u_FilterSize,   3);
    m_RDataFilterThickness.shader->setUniformValue(m_RDataFilterThickness.u_ScreenWidth,  static_cast<int>(width() * THICKNESS_TEXTURE_SIZE_RATIO));
    m_RDataFilterThickness.shader->setUniformValue(m_RDataFilterThickness.u_ScreenHeight, static_cast<int>(height() * THICKNESS_TEXTURE_SIZE_RATIO));

    glCall(glClearColor(0, 0, 0, 1.0));
    glCall(glBindVertexArray(m_RDataFilterThickness.VAO));
    m_RDataFilterThickness.offScreenRender->beginRender();

    m_RDataCompositionPass.thicknessTex->bind();
    glCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    glCall(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
    m_RDataCompositionPass.thicknessTex->release();

    m_RDataFilterThickness.offScreenRender->endRender(context()->defaultFramebufferObject());
    glCall(glBindVertexArray(0));
    m_RDataFilterThickness.shader->release();

    ////////////////////////////////////////////////////////////////////////////////
    m_RDataCompositionPass.thicknessTex = m_RDataFilterThickness.offScreenRender->getColorBuffer();
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void RenderWidget::initRDataNormalPass()
{
    m_RDataNormalPass.offScreenRender = std::make_unique<OffScreenRender>(width(), height(), 1, GL_RGB16F);

    m_RDataNormalPass.shader = ShaderProgram::getScreenQuadShaderFromFile("Shaders/normal-pass.fs.glsl", "RenderNormalPass");
    m_ExternalShaders.push_back(m_RDataNormalPass.shader);

    m_RDataNormalPass.ub_CamData     = m_RDataNormalPass.shader->getUniformBlockIndex("CameraData");
    m_RDataNormalPass.u_DepthTex     = m_RDataNormalPass.shader->getUniformLocation("u_DepthTex");
    m_RDataNormalPass.u_ScreenWidth  = m_RDataNormalPass.shader->getUniformLocation("u_ScreenWidth");
    m_RDataNormalPass.u_ScreenHeight = m_RDataNormalPass.shader->getUniformLocation("u_ScreenHeight");
    glCall(glGenVertexArrays(1, &m_RDataNormalPass.VAO));

    m_RDataNormalPass.initialized = true;
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void RenderWidget::renderNormalPass(bool t0 /*= false*/)
{
    Q_ASSERT(m_RDataNormalPass.initialized);

    m_RDataNormalPass.offScreenRender->beginRender();
    glCall(glClearColor(0, 0, 0, 1.0));
    glCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

    ////////////////////////////////////////////////////////////////////////////////
    m_RDataNormalPass.shader->bind();

    m_UBufferCamData->bindBufferBase();
    m_RDataNormalPass.shader->bindUniformBlock(m_RDataNormalPass.ub_CamData, m_UBufferCamData->getBindingPoint());
    m_RDataNormalPass.shader->setUniformValue(m_RDataNormalPass.u_DepthTex,     0);
    m_RDataNormalPass.shader->setUniformValue(m_RDataNormalPass.u_ScreenWidth,  width());
    m_RDataNormalPass.shader->setUniformValue(m_RDataNormalPass.u_ScreenHeight, height());

    if(t0) {
        Q_ASSERT(m_RDataCompositionPass.depthTex != nullptr);
        m_RDataCompositionPass.depthTex->bind();
    } else {
        Q_ASSERT(m_RDataCompositionPass.filteredDepthTex != nullptr);
        m_RDataCompositionPass.filteredDepthTex->bind();
    }

    glCall(glBindVertexArray(m_RDataNormalPass.VAO));
    glCall(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
    glCall(glBindVertexArray(0));

    if(t0) {
        m_RDataCompositionPass.depthTex->release();
    } else {
        m_RDataCompositionPass.filteredDepthTex->release();
    }

    m_RDataNormalPass.shader->release();

    ////////////////////////////////////////////////////////////////////////////////
    m_RDataNormalPass.offScreenRender->endRender(context()->defaultFramebufferObject());
    m_RDataCompositionPass.normalTex = m_RDataNormalPass.offScreenRender->getColorBuffer();
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void RenderWidget::initRDataBackgroundPass()
{
    m_RDataBackgroundPass.offScreenRender = std::make_unique<OffScreenRender>(width(), height(), 1, GL_RGBA);
    m_RDataBackgroundPass.initialized     = true;
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void RenderWidget::renderBackgroundPass()
{
    Q_ASSERT(m_RDataBackgroundPass.initialized);

    m_RDataBackgroundPass.offScreenRender->beginRender();
    resetClearColor();
    glCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

    ////////////////////////////////////////////////////////////////////////////////
    renderSkyBox();
    renderFluidFloor();
    //    renderBox(); // box cannot be rendered as background
    renderLight();
    renderMeshes();

    ////////////////////////////////////////////////////////////////////////////////
    m_RDataBackgroundPass.offScreenRender->endRender(context()->defaultFramebufferObject());
    m_RDataCompositionPass.backgroundTex = m_RDataBackgroundPass.offScreenRender->getColorBuffer();
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void RenderWidget::initRDataCompositionPass()
{
    m_RDataCompositionPass.shader = ShaderProgram::getScreenQuadShaderFromFile("Shaders/composition-pass.fs.glsl", "RenderCompositionPass");
    m_ExternalShaders.push_back(m_RDataCompositionPass.shader);

    m_RDataCompositionPass.fluidVolumeMaterial = std::make_unique<Material>("SurfaceMaterial");
    m_RDataCompositionPass.fluidVolumeMaterial->setMaterial(CUSTOM_SURFACE_MATERIAL);
    m_RDataCompositionPass.fluidVolumeMaterial->uploadDataToGPU();
    m_RDataCompositionPass.normalTex = m_RDataNormalPass.offScreenRender->getColorBuffer();

    m_RDataCompositionPass.ub_CamData       = m_RDataCompositionPass.shader->getUniformBlockIndex("CameraData");
    m_RDataCompositionPass.ub_Light         = m_RDataCompositionPass.shader->getUniformBlockIndex("Lights");
    m_RDataCompositionPass.ub_LightMatrices = m_RDataCompositionPass.shader->getUniformBlockIndex("LightMatrices");
    m_RDataCompositionPass.ub_Material      = m_RDataCompositionPass.shader->getUniformBlockIndex("Material");

    m_RDataCompositionPass.u_HasSolid      = m_RDataCompositionPass.shader->getUniformLocation("u_HasSolid");
    m_RDataCompositionPass.u_SolidDepthMap = m_RDataCompositionPass.shader->getUniformLocation("u_SolidDepthMap");

    m_RDataCompositionPass.u_SkyBoxTex     = m_RDataCompositionPass.shader->getUniformLocation("u_SkyBoxTex");
    m_RDataCompositionPass.u_DepthTex      = m_RDataCompositionPass.shader->getUniformLocation("u_DepthTex");
    m_RDataCompositionPass.u_ThicknessTex  = m_RDataCompositionPass.shader->getUniformLocation("u_ThicknessTex");
    m_RDataCompositionPass.u_NormalTex     = m_RDataCompositionPass.shader->getUniformLocation("u_NormalTex");
    m_RDataCompositionPass.u_BackgroundTex = m_RDataCompositionPass.shader->getUniformLocation("u_BackgroundTex");

    m_RDataCompositionPass.u_HasShadow             = m_RDataCompositionPass.shader->getUniformLocation("u_HasShadow");
    m_RDataCompositionPass.u_ShadowIntensity       = m_RDataCompositionPass.shader->getUniformLocation("u_ShadowIntensity");
    m_RDataCompositionPass.u_VisualizeShadowRegion = m_RDataCompositionPass.shader->getUniformLocation("u_VisualizeShadowRegion");

    for(int i = 0; i < MAX_NUM_LIGHTS; ++i) {
        char buff[128];
        sprintf(buff, "u_SolidShadowMaps[%d]",      i);
        m_RDataCompositionPass.u_SolidShadowMaps[i] = m_RDataCompositionPass.shader->getUniformLocation(buff);

        sprintf(buff, "u_FluidShadowMaps[%d]",      i);
        m_RDataCompositionPass.u_FluidShadowMaps[i] = m_RDataCompositionPass.shader->getUniformLocation(buff);

        sprintf(buff, "u_FluidShadowThickness[%d]", i);
        m_RDataCompositionPass.u_FluidShadowThickness[i] = m_RDataCompositionPass.shader->getUniformLocation(buff);
    }

    m_RDataCompositionPass.u_TransparentFluid     = m_RDataCompositionPass.shader->getUniformLocation("u_TransparentFluid");
    m_RDataCompositionPass.u_ReflectionConstant   = m_RDataCompositionPass.shader->getUniformLocation("u_ReflectionConstant");
    m_RDataCompositionPass.u_AttennuationConstant = m_RDataCompositionPass.shader->getUniformLocation("u_AttennuationConstant");

    glCall(glGenVertexArrays(1, &m_RDataCompositionPass.VAO));

    m_RDataCompositionPass.initialized = true;
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void RenderWidget::renderCompositionPass()
{
    Q_ASSERT(m_RDataCompositionPass.initialized);
    Q_ASSERT(m_RDataCompositionPass.filteredDepthTex != nullptr);
    Q_ASSERT(m_RDataCompositionPass.thicknessTex != nullptr);
    Q_ASSERT(m_RDataCompositionPass.normalTex != nullptr);
    Q_ASSERT(m_RDataCompositionPass.backgroundTex != nullptr);

    auto skyBoxTex = m_SkyBoxRender->getCurrentTexture();
    int  texID     = 0;

    ////////////////////////////////////////////////////////////////////////////////
    m_RDataCompositionPass.shader->bind();

    m_UBufferCamData->bindBufferBase();
    m_RDataCompositionPass.shader->bindUniformBlock(m_RDataCompositionPass.ub_CamData, m_UBufferCamData->getBindingPoint());
    m_Lights->bindUniformBuffer();
    m_RDataCompositionPass.shader->bindUniformBlock(m_RDataCompositionPass.ub_Light, m_Lights->getBufferBindingPoint());
    m_RDataCompositionPass.fluidVolumeMaterial->bindUniformBuffer();
    m_RDataCompositionPass.shader->bindUniformBlock(m_RDataCompositionPass.ub_Material, m_RDataCompositionPass.fluidVolumeMaterial->getBufferBindingPoint());

    if(m_NumMeshes == 0) {
        m_RDataCompositionPass.shader->setUniformValue(m_RDataCompositionPass.u_HasSolid, 0);
    } else {
        m_RDataCompositionPass.shader->setUniformValue(m_RDataCompositionPass.u_HasSolid,      1);
        m_RDataCompositionPass.shader->setUniformValue(m_RDataCompositionPass.u_SolidDepthMap, texID);
        m_RDataSolidCameraDepthMap.depthMapRender->getDepthBuffer()->bind(texID++);
    }

    if(skyBoxTex != nullptr) {
        m_RDataCompositionPass.shader->setUniformValue(m_RDataCompositionPass.u_SkyBoxTex, texID);
        skyBoxTex->bind(texID++);
    }

    m_RDataCompositionPass.shader->setUniformValue(m_RDataCompositionPass.u_DepthTex, texID);
    m_RDataCompositionPass.filteredDepthTex->bind(texID++);

    m_RDataCompositionPass.shader->setUniformValue(m_RDataCompositionPass.u_ThicknessTex, texID);
    m_RDataCompositionPass.thicknessTex->bind(texID++);

    m_RDataCompositionPass.shader->setUniformValue(m_RDataCompositionPass.u_NormalTex, texID);
    m_RDataCompositionPass.normalTex->bind(texID++);

    m_RDataCompositionPass.shader->setUniformValue(m_RDataCompositionPass.u_BackgroundTex, texID);
    m_RDataCompositionPass.backgroundTex->bind(texID++);

    m_RDataCompositionPass.shader->setUniformValue(m_RDataCompositionPass.u_HasShadow, m_bShadowEnabled ? 1 : 0);

    if(m_bShadowEnabled) {
        m_Lights->bindUniformBufferLightMatrix();
        m_RDataCompositionPass.shader->bindUniformBlock(m_RDataCompositionPass.ub_LightMatrices, m_Lights->getBufferLightMatrixBindingPoint());

        m_RDataCompositionPass.shader->setUniformValue(m_RDataCompositionPass.u_ShadowIntensity,       m_ShadowIntensity);
        m_RDataCompositionPass.shader->setUniformValue(m_RDataCompositionPass.u_VisualizeShadowRegion, m_bVisualizeShadow ? 1 : 0);

        for(int i = 0; i < m_Lights->getNumLights(); ++i) {
            m_RDataCompositionPass.shader->setUniformValue(m_RDataCompositionPass.u_SolidShadowMaps[i],      texID);
            m_RDataSolidLightDepthMap.solidShadowMaps[i]->bind(texID++);

            m_RDataCompositionPass.shader->setUniformValue(m_RDataCompositionPass.u_FluidShadowMaps[i],      texID);
            m_RDataFluidShadowMap.fluidShadowMaps[i]->bind(texID++);

            m_RDataCompositionPass.shader->setUniformValue(m_RDataCompositionPass.u_FluidShadowThickness[i], texID);
            m_RDataFluidShadowThickness.fluidShadowThickness[i]->bind(texID++);
        }
    }

    m_RDataCompositionPass.shader->setUniformValue(m_RDataCompositionPass.u_TransparentFluid,     m_FluidRenderMode == ParticleRenderMode::TransparentFluid ? 1 : 0);
    m_RDataCompositionPass.shader->setUniformValue(m_RDataCompositionPass.u_ReflectionConstant,   m_ReflectionConstant);
    m_RDataCompositionPass.shader->setUniformValue(m_RDataCompositionPass.u_AttennuationConstant, m_AttennuationConstant);

    ////////////////////////////////////////////////////////////////////////////////
    glCall(glBindVertexArray(m_RDataCompositionPass.VAO));
    glCall(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
    glCall(glBindVertexArray(0));

    ////////////////////////////////////////////////////////////////////////////////
    if(m_NumMeshes != 0) {
        m_RDataSolidCameraDepthMap.depthMapRender->getDepthBuffer()->release();
    }

    if(skyBoxTex != nullptr) {
        skyBoxTex->release();
    }

    m_RDataCompositionPass.filteredDepthTex->release();
    m_RDataCompositionPass.thicknessTex->release();
    m_RDataCompositionPass.normalTex->release();
    m_RDataCompositionPass.backgroundTex->release();

    if(m_bShadowEnabled) {
        for(int i = 0; i < m_Lights->getNumLights(); ++i) {
            m_RDataSolidLightDepthMap.solidShadowMaps[i]->release();
            m_RDataFluidShadowMap.fluidShadowMaps[i]->release();
            m_RDataFluidShadowThickness.fluidShadowThickness[i]->release();
        }
    }

    m_RDataCompositionPass.shader->release();
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void RenderWidget::initRDataFilterBiGaussian()
{
    m_RDataFilterBiGaussian.shader = ShaderProgram::getScreenQuadShaderFromFile("Shaders/filter-bigaussian.fs.glsl", "FilterBiGaussian");
    m_ExternalShaders.push_back(m_RDataFilterBiGaussian.shader);

    m_RDataFilterBiGaussian.u_DepthTex       = m_RDataFilterBiGaussian.shader->getUniformLocation("u_DepthTex");
    m_RDataFilterBiGaussian.u_ParticleRadius = m_RDataFilterBiGaussian.shader->getUniformLocation("u_ParticleRadius");
    m_RDataFilterBiGaussian.u_FilterSize     = m_RDataFilterBiGaussian.shader->getUniformLocation("u_FilterSize");
    m_RDataFilterBiGaussian.u_MaxFilterSize  = m_RDataFilterBiGaussian.shader->getUniformLocation("u_MaxFilterSize");
    m_RDataFilterBiGaussian.u_ScreenWidth    = m_RDataFilterBiGaussian.shader->getUniformLocation("u_ScreenWidth");
    m_RDataFilterBiGaussian.u_ScreenHeight   = m_RDataFilterBiGaussian.shader->getUniformLocation("u_ScreenHeight");

    glCall(glGenVertexArrays(1, &m_RDataFilterBiGaussian.VAO));

    m_RDataFilterBiGaussian.initialized = true;
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void RenderWidget::filterBiGaussian()
{
    Q_ASSERT(m_RDataFilterBiGaussian.initialized);
    Q_ASSERT(m_FilterFrameBuffer != nullptr);
    Q_ASSERT(m_RDataCompositionPass.depthTex != nullptr);

    ////////////////////////////////////////////////////////////////////////////////
    m_RDataFilterBiGaussian.shader->bind();
    m_RDataFilterBiGaussian.shader->setUniformValue(m_RDataFilterBiGaussian.u_DepthTex,       0);
    m_RDataFilterBiGaussian.shader->setUniformValue(m_RDataFilterBiGaussian.u_ParticleRadius, m_RDataParticle.pointRadius);
    m_RDataFilterBiGaussian.shader->setUniformValue(m_RDataFilterBiGaussian.u_FilterSize,     m_FilterSize);
    m_RDataFilterBiGaussian.shader->setUniformValue(m_RDataFilterBiGaussian.u_MaxFilterSize,  static_cast<GLint>(MAX_FILTER_SIZE));
    m_RDataFilterBiGaussian.shader->setUniformValue(m_RDataFilterBiGaussian.u_ScreenWidth,    width());
    m_RDataFilterBiGaussian.shader->setUniformValue(m_RDataFilterBiGaussian.u_ScreenHeight,   height());

    glCall(glClearColor(CLEAR_DEPTH_VALUE, CLEAR_DEPTH_VALUE, CLEAR_DEPTH_VALUE, 1.0));
    glCall(glBindVertexArray(m_RDataFilterBiGaussian.VAO));
    m_FilterFrameBuffer->beginRender();

    for(int i = 0; i < m_NumFilterIteration; ++i) {
        if(i > 0) {
            m_FilterFrameBuffer->fastSwapColorBuffer(m_RDataCompositionPass.depthTex);
        }

        m_RDataCompositionPass.depthTex->bind();
        glCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
        glCall(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
        m_RDataCompositionPass.depthTex->release();
    }

    m_FilterFrameBuffer->endRender(context()->defaultFramebufferObject());
    glCall(glBindVertexArray(0));
    m_RDataFilterBiGaussian.shader->release();
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void RenderWidget::initRDataFilterCurvatureFlow()
{
    m_RDataFilterCurvatureFlow.shader = ShaderProgram::getScreenQuadShaderFromFile("Shaders/filter-curvature-flow.fs.glsl", "FilterCurvatureFlow");
    m_ExternalShaders.push_back(m_RDataFilterCurvatureFlow.shader);

    m_RDataFilterCurvatureFlow.ub_CamData = m_RDataFilterCurvatureFlow.shader->getUniformBlockIndex("CameraData");

    m_RDataFilterCurvatureFlow.u_DepthTex       = m_RDataFilterCurvatureFlow.shader->getUniformLocation("u_DepthTex");
    m_RDataFilterCurvatureFlow.u_ParticleRadius = m_RDataFilterCurvatureFlow.shader->getUniformLocation("u_ParticleRadius");
    m_RDataFilterCurvatureFlow.u_ScreenWidth    = m_RDataFilterCurvatureFlow.shader->getUniformLocation("u_ScreenWidth");
    m_RDataFilterCurvatureFlow.u_ScreenHeight   = m_RDataFilterCurvatureFlow.shader->getUniformLocation("u_ScreenHeight");

    glCall(glGenVertexArrays(1, &m_RDataFilterCurvatureFlow.VAO));

    m_RDataFilterCurvatureFlow.initialized = true;
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void RenderWidget::filterCurvatureFlow()
{
    Q_ASSERT(m_RDataFilterCurvatureFlow.initialized);
    Q_ASSERT(m_FilterFrameBuffer != nullptr);
    Q_ASSERT(m_RDataCompositionPass.depthTex != nullptr);

    ////////////////////////////////////////////////////////////////////////////////
    m_RDataFilterCurvatureFlow.shader->bind();
    m_UBufferCamData->bindBufferBase();
    m_RDataFilterCurvatureFlow.shader->bindUniformBlock(m_RDataFilterCurvatureFlow.ub_CamData, m_UBufferCamData->getBindingPoint());

    m_RDataFilterCurvatureFlow.shader->setUniformValue(m_RDataFilterCurvatureFlow.u_DepthTex,       0);
    m_RDataFilterCurvatureFlow.shader->setUniformValue(m_RDataFilterCurvatureFlow.u_ParticleRadius, m_RDataParticle.pointRadius);
    m_RDataFilterCurvatureFlow.shader->setUniformValue(m_RDataFilterCurvatureFlow.u_ScreenWidth,    width());
    m_RDataFilterCurvatureFlow.shader->setUniformValue(m_RDataFilterCurvatureFlow.u_ScreenHeight,   height());

    glCall(glClearColor(CLEAR_DEPTH_VALUE, CLEAR_DEPTH_VALUE, CLEAR_DEPTH_VALUE, 1.0));
    glCall(glBindVertexArray(m_RDataFilterCurvatureFlow.VAO));
    m_FilterFrameBuffer->beginRender();

    for(int i = 0; i < m_NumFilterIteration; ++i) {
        if(i > 0) {
            m_FilterFrameBuffer->fastSwapColorBuffer(m_RDataCompositionPass.depthTex);
        }

        m_RDataCompositionPass.depthTex->bind();
        glCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
        glCall(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
        m_RDataCompositionPass.depthTex->release();
    }

    m_FilterFrameBuffer->endRender(context()->defaultFramebufferObject());
    glCall(glBindVertexArray(0));
    m_RDataFilterCurvatureFlow.shader->release();
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void RenderWidget::initRDataFilterPlaneFitting()
{
    m_RDataFilterPlaneFitting.shader = ShaderProgram::getScreenQuadShaderFromFile("Shaders/filter-plane-fitting.fs.glsl", "FilterPlaneFitting");
    m_ExternalShaders.push_back(m_RDataFilterPlaneFitting.shader);

    m_RDataFilterPlaneFitting.ub_CamData       = m_RDataFilterPlaneFitting.shader->getUniformBlockIndex("CameraData");
    m_RDataFilterPlaneFitting.u_DepthTex       = m_RDataFilterPlaneFitting.shader->getUniformLocation("u_DepthTex");
    m_RDataFilterPlaneFitting.u_NormalTex_t0   = m_RDataFilterPlaneFitting.shader->getUniformLocation("u_NormalTex_t0");
    m_RDataFilterPlaneFitting.u_ParticleRadius = m_RDataFilterPlaneFitting.shader->getUniformLocation("u_ParticleRadius");
    m_RDataFilterPlaneFitting.u_FilterSize     = m_RDataFilterPlaneFitting.shader->getUniformLocation("u_FilterSize");
    m_RDataFilterPlaneFitting.u_MaxFilterSize  = m_RDataFilterPlaneFitting.shader->getUniformLocation("u_MaxFilterSize");
    m_RDataFilterPlaneFitting.u_ScreenWidth    = m_RDataFilterPlaneFitting.shader->getUniformLocation("u_ScreenWidth");
    m_RDataFilterPlaneFitting.u_ScreenHeight   = m_RDataFilterPlaneFitting.shader->getUniformLocation("u_ScreenHeight");

    glCall(glGenVertexArrays(1, &m_RDataFilterPlaneFitting.VAO));

    m_RDataFilterPlaneFitting.initialized = true;
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void RenderWidget::filterPlaneFitting()
{
    Q_ASSERT(m_RDataFilterPlaneFitting.initialized);
    Q_ASSERT(m_FilterFrameBuffer != nullptr);
    Q_ASSERT(m_RDataCompositionPass.depthTex != nullptr);

    ////////////////////////////////////////////////////////////////////////////////
    m_RDataFilterPlaneFitting.shader->bind();
    m_UBufferCamData->bindBufferBase();
    m_RDataFilterPlaneFitting.shader->bindUniformBlock(m_RDataFilterPlaneFitting.ub_CamData, m_UBufferCamData->getBindingPoint());

    m_RDataFilterPlaneFitting.shader->setUniformValue(m_RDataFilterPlaneFitting.u_DepthTex,       0);
    m_RDataFilterPlaneFitting.shader->setUniformValue(m_RDataFilterPlaneFitting.u_NormalTex_t0,   1);
    m_RDataFilterPlaneFitting.shader->setUniformValue(m_RDataFilterPlaneFitting.u_ParticleRadius, m_RDataParticle.pointRadius);
    m_RDataFilterPlaneFitting.shader->setUniformValue(m_RDataFilterPlaneFitting.u_FilterSize,     m_FilterSize);
    m_RDataFilterPlaneFitting.shader->setUniformValue(m_RDataFilterPlaneFitting.u_MaxFilterSize,  static_cast<GLint>(MAX_FILTER_SIZE));
    m_RDataFilterPlaneFitting.shader->setUniformValue(m_RDataFilterPlaneFitting.u_ScreenWidth,    width());
    m_RDataFilterPlaneFitting.shader->setUniformValue(m_RDataFilterPlaneFitting.u_ScreenHeight,   height());

    glCall(glClearColor(CLEAR_DEPTH_VALUE, CLEAR_DEPTH_VALUE, CLEAR_DEPTH_VALUE, 1.0));
    glCall(glBindVertexArray(m_RDataFilterPlaneFitting.VAO));
    m_FilterFrameBuffer->beginRender();

    for(int i = 0; i < m_NumFilterIteration; ++i) {
        if(i > 0) {
            m_FilterFrameBuffer->fastSwapColorBuffer(m_RDataCompositionPass.depthTex);
        }

        m_RDataCompositionPass.depthTex->bind();
        m_RDataCompositionPass.normalTex->bind(1);

        glCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
        glCall(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));

        m_RDataCompositionPass.depthTex->release();
        m_RDataCompositionPass.normalTex->release();
    }

    m_FilterFrameBuffer->endRender(context()->defaultFramebufferObject());
    glCall(glBindVertexArray(0));
    m_RDataFilterPlaneFitting.shader->release();
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void RenderWidget::initRDataNarrowRangeFilter()
{
    m_RDataNarrowRangeFilter.shader = ShaderProgram::getScreenQuadShaderFromFile("Shaders/filter-narrow-range.fs.glsl", "FilterNarrowRange");
    m_ExternalShaders.push_back(m_RDataNarrowRangeFilter.shader);

    m_RDataNarrowRangeFilter.ub_CamData        = m_RDataNarrowRangeFilter.shader->getUniformBlockIndex("CameraData");
    m_RDataNarrowRangeFilter.u_DepthTex        = m_RDataNarrowRangeFilter.shader->getUniformLocation("u_DepthTex");
    m_RDataNarrowRangeFilter.u_ParticleRadius  = m_RDataNarrowRangeFilter.shader->getUniformLocation("u_ParticleRadius");
    m_RDataNarrowRangeFilter.u_FilterSize      = m_RDataNarrowRangeFilter.shader->getUniformLocation("u_FilterSize");
    m_RDataNarrowRangeFilter.u_MaxFilterSize   = m_RDataNarrowRangeFilter.shader->getUniformLocation("u_MaxFilterSize");
    m_RDataNarrowRangeFilter.u_ScreenWidth     = m_RDataNarrowRangeFilter.shader->getUniformLocation("u_ScreenWidth");
    m_RDataNarrowRangeFilter.u_ScreenHeight    = m_RDataNarrowRangeFilter.shader->getUniformLocation("u_ScreenHeight");
    m_RDataNarrowRangeFilter.u_FilterDirection = m_RDataNarrowRangeFilter.shader->getUniformLocation("u_FilterDirection");
    m_RDataNarrowRangeFilter.u_DoFilter1D      = m_RDataNarrowRangeFilter.shader->getUniformLocation("u_DoFilter1D");

    glCall(glGenVertexArrays(1, &m_RDataNarrowRangeFilter.VAO));

    m_RDataNarrowRangeFilter.initialized = true;
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void RenderWidget::filterNarrowRange()
{
    Q_ASSERT(m_RDataNarrowRangeFilter.initialized);

    Q_ASSERT(m_FilterFrameBuffer != nullptr);
    Q_ASSERT(m_RDataCompositionPass.depthTex != nullptr);

    ////////////////////////////////////////////////////////////////////////////////
    m_RDataNarrowRangeFilter.shader->bind();
    m_UBufferCamData->bindBufferBase();
    m_RDataNarrowRangeFilter.shader->bindUniformBlock(m_RDataNarrowRangeFilter.ub_CamData, m_UBufferCamData->getBindingPoint());

    m_RDataNarrowRangeFilter.shader->setUniformValue(m_RDataNarrowRangeFilter.u_DepthTex,       0);
    m_RDataNarrowRangeFilter.shader->setUniformValue(m_RDataNarrowRangeFilter.u_ParticleRadius, m_RDataParticle.pointRadius);
    m_RDataNarrowRangeFilter.shader->setUniformValue(m_RDataNarrowRangeFilter.u_FilterSize,     m_FilterSize);
    m_RDataNarrowRangeFilter.shader->setUniformValue(m_RDataNarrowRangeFilter.u_MaxFilterSize,  static_cast<GLint>(MAX_FILTER_SIZE));
    m_RDataNarrowRangeFilter.shader->setUniformValue(m_RDataNarrowRangeFilter.u_ScreenWidth,    width());
    m_RDataNarrowRangeFilter.shader->setUniformValue(m_RDataNarrowRangeFilter.u_ScreenHeight,   height());

    glCall(glClearColor(CLEAR_DEPTH_VALUE, CLEAR_DEPTH_VALUE, CLEAR_DEPTH_VALUE, 1.0));
    glCall(glBindVertexArray(m_RDataNarrowRangeFilter.VAO));
    m_FilterFrameBuffer->beginRender();

    ////////////////////////////////////////////////////////////////////////////////
    if(m_FilterMethod == FilterMethod::NarrowRangeFilter2D) {
        m_RDataNarrowRangeFilter.shader->setUniformValue(m_RDataNarrowRangeFilter.u_DoFilter1D, 0);

        for(int i = 0; i < m_NumFilterIteration; ++i) {
            if(i > 0) {
                m_FilterFrameBuffer->fastSwapColorBuffer(m_RDataCompositionPass.depthTex);
            }

            m_RDataCompositionPass.depthTex->bind();
            glCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
            glCall(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
            m_RDataCompositionPass.depthTex->release();
        }
    } else {
        m_RDataNarrowRangeFilter.shader->setUniformValue(m_RDataNarrowRangeFilter.u_DoFilter1D, 1);

        for(int i = 0, iend = m_FilterMethod == FilterMethod::NarrowRangeFilter1D2D ? m_NumFilterIteration - 1 : m_NumFilterIteration; i < iend; ++i) {
            if(i > 0) {
                m_FilterFrameBuffer->fastSwapColorBuffer(m_RDataCompositionPass.depthTex);
            }

            m_RDataNarrowRangeFilter.shader->setUniformValue(m_RDataNarrowRangeFilter.u_FilterDirection, 0);
            m_RDataCompositionPass.depthTex->bind();
            glCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
            glCall(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
            m_RDataCompositionPass.depthTex->release();

            m_FilterFrameBuffer->fastSwapColorBuffer(m_RDataCompositionPass.depthTex);
            m_RDataNarrowRangeFilter.shader->setUniformValue(m_RDataNarrowRangeFilter.u_FilterDirection, 1);
            m_RDataCompositionPass.depthTex->bind();
            glCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
            glCall(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
            m_RDataCompositionPass.depthTex->release();
        }

        if(m_FilterMethod == FilterMethod::NarrowRangeFilter1D2D && m_NumFilterIteration > 0) {
            m_FilterFrameBuffer->fastSwapColorBuffer(m_RDataCompositionPass.depthTex);
            m_RDataNarrowRangeFilter.shader->setUniformValue(m_RDataNarrowRangeFilter.u_DoFilter1D, -1);
            m_RDataCompositionPass.depthTex->bind();
            glCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
            glCall(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
            m_RDataCompositionPass.depthTex->release();
        }
    }

    ////////////////////////////////////////////////////////////////////////////////
    m_FilterFrameBuffer->endRender(context()->defaultFramebufferObject());
    glCall(glBindVertexArray(0));
    m_RDataNarrowRangeFilter.shader->release();
}
