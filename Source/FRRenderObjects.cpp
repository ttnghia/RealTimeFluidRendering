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

#include "FRRenderObjects.h"

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void FRMeshRender::render(bool bRenderShadow, bool bVisualizeShadowRegion, float shadowIntensity)
{
    assert(m_MeshObj != nullptr && m_Camera != nullptr && m_UBufferCamData != nullptr);
    if(m_MeshObj->isEmpty()) {
        return;
    }

    if(m_SelfUpdateCamera) {
        m_Camera->updateCameraMatrices();
        m_UBufferCamData->uploadData(glm::value_ptr(m_Camera->getViewMatrix()),            0,                   sizeof(Mat4x4f));
        m_UBufferCamData->uploadData(glm::value_ptr(m_Camera->getProjectionMatrix()), sizeof(Mat4x4f),          sizeof(Mat4x4f));
        m_UBufferCamData->uploadData(glm::value_ptr(m_Camera->getCameraPosition()),        5 * sizeof(Mat4x4f), sizeof(Vec3f));
    }

    m_Shader->bind();

    if(m_CurrentTexture != nullptr) {
        m_CurrentTexture->bind();
        m_Shader->setUniformValue(m_UHasTexture, 1);
        m_Shader->setUniformValue(m_UTexSampler, 0);
    } else {
        m_Shader->setUniformValue(m_UHasTexture, 0);
    }

    m_UBufferModelMatrix->bindBufferBase();
    m_Shader->bindUniformBlock(m_UBModelMatrix, m_UBufferModelMatrix->getBindingPoint());

    m_UBufferCamData->bindBufferBase();
    m_Shader->bindUniformBlock(m_UBCamData, m_UBufferCamData->getBindingPoint());

    m_Lights->bindUniformBuffer();
    m_Shader->bindUniformBlock(m_UBLight, m_Lights->getBufferBindingPoint());

    m_Material->bindUniformBuffer();
    m_Shader->bindUniformBlock(m_UBMaterial, m_Material->getBufferBindingPoint());

    if(bRenderShadow) {
        m_Lights->bindUniformBufferLightMatrix();
        m_Shader->bindUniformBlock(m_UBLightMatrices, m_Lights->getBufferLightMatrixBindingPoint());
        m_Shader->setUniformValue(m_UHasShadow,                                  1);
        m_Shader->setUniformValue(m_UShadowIntensity,              shadowIntensity);
        m_Shader->setUniformValue(m_UVisualizeShadowRegion, bVisualizeShadowRegion ? 1 : 0);

        int texID = 1;

        for(int i = 0; i < m_Lights->getNumLights(); ++i) {
            m_Shader->setUniformValue(m_USolidShadowMaps[i],      texID);
            m_SolidShadowMaps[i]->bind(texID++);

            m_Shader->setUniformValue(m_UFluidShadowMaps[i],      texID);
            m_FluidShadowMaps[i]->bind(texID++);

            m_Shader->setUniformValue(m_UFluidShadowThickness[i], texID);
            m_FluidShadowThickness[i]->bind(texID++);
        }
    } else {
        m_Shader->setUniformValue(m_UHasShadow, 0);
    }

    m_Shader->setUniformValue(m_UExposure, m_Exposure);

    glCall(glBindVertexArray(m_VAO));
    m_MeshObj->draw();
    glCall(glBindVertexArray(0));

    if(m_CurrentTexture != nullptr) {
        m_CurrentTexture->release();
    }

    if(bRenderShadow) {
        for(int i = 0; i < m_Lights->getNumLights(); ++i) {
            m_SolidShadowMaps[i]->release();
            m_FluidShadowMaps[i]->release();
            m_FluidShadowThickness[i]->release();
        }
    }

    m_Shader->release();
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void FRMeshRender::initRenderData()
{
    ////////////////////////////////////////////////////////////////////////////////
    // reconfigure shader
    m_Shader = std::make_shared<ShaderProgram>("Shaders/mesh.vs.glsl", "Shaders/mesh.fs.glsl", "MeshShader");

    m_AtrVPosition = m_Shader->getAtributeLocation("v_Position");
    m_AtrVNormal   = m_Shader->getAtributeLocation("v_Normal");
    m_AtrVTexCoord = m_Shader->getAtributeLocation("v_TexCoord");

    m_UHasTexture            = m_Shader->getUniformLocation("u_HasTexture");
    m_UHasShadow             = m_Shader->getUniformLocation("u_HasShadow");
    m_UTexSampler            = m_Shader->getUniformLocation("u_TexSampler");
    m_UShadowIntensity       = m_Shader->getUniformLocation("u_ShadowIntensity");
    m_UVisualizeShadowRegion = m_Shader->getUniformLocation("u_VisualizeShadowRegion");
    m_UExposure              = m_Shader->getUniformLocation("u_Exposure");

    for(int i = 0; i < MAX_NUM_LIGHTS; ++i) {
        char buff[128];
        sprintf(buff,      "u_SolidShadowMaps[%d]", i);
        m_USolidShadowMaps[i] = m_Shader->getUniformLocation(buff);

        sprintf(buff,      "u_FluidShadowMaps[%d]", i);
        m_UFluidShadowMaps[i] = m_Shader->getUniformLocation(buff);

        sprintf(buff, "u_FluidShadowThickness[%d]", i);
        m_UFluidShadowThickness[i] = m_Shader->getUniformLocation(buff);
    }

    m_UBModelMatrix   = m_Shader->getUniformBlockIndex("ModelMatrix");
    m_UBCamData       = m_Shader->getUniformBlockIndex("CameraData");
    m_UBLightMatrices = m_Shader->getUniformBlockIndex("LightMatrices");
    m_UBLight         = m_Shader->getUniformBlockIndex("Lights");
    m_UBMaterial      = m_Shader->getUniformBlockIndex("Material");
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// PlaneRender
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void FRPlaneRender::render(bool bRenderShadow, bool bVisualizeShadowRegion, float shadowIntensity)
{
    if(m_CurrentTexture == nullptr) {
        return;
    }
    FRMeshRender::render(bRenderShadow, bVisualizeShadowRegion, shadowIntensity);
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void FRPlaneRender::scaleTexCoord(int scaleX, int scaleY)
{
    GridObject* gridObj = static_cast<GridObject*>(m_MeshObj.get());
    assert(gridObj != nullptr);
    gridObj->scaleTexCoord(scaleX, scaleY);
    gridObj->uploadDataToGPU();
}