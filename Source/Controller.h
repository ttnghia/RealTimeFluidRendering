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

#include <QWidget>
#include <QtWidgets>

#include <QtAppHelpers/MaterialSelector.h>
#include <QtAppHelpers/EnhancedComboBox.h>
#include <QtAppHelpers/EnhancedSlider.h>
#include <QtAppHelpers/PointLightEditor.h>

#include "Common.h"

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
class Controller : public QWidget
{
    Q_OBJECT
    friend class MainWindow;
public:
    explicit Controller(QWidget* parent) :
        QWidget(parent)
    {
        setupGUI();
    }

public slots:
    void loadTextures();

private:
    void setupGUI();
    void setupTextureControllers(QBoxLayout* ctrLayout);
    void setupFrameControllers(QBoxLayout* ctrLayout);
    void setupFluidRenderModeControllers(QBoxLayout* ctrLayout);
    void setupColorControllers(QBoxLayout* ctrLayout);
    void setupShadowControllers(QBoxLayout* ctrLayout);
    void setupFilterControllers(QBoxLayout* ctrLayout);
    void setupButtons(QBoxLayout* ctrLayout);

    ////////////////////////////////////////////////////////////////////////////////
    QSignalMapper* m_smFluidRenderMode;
    QSignalMapper* m_smParticleColorMode;
    QSignalMapper* m_smFilterMethod;

    EnhancedSlider* m_sldFrameStep;
    EnhancedSlider* m_sldFrameDelay;
    EnhancedSlider* m_sldFluidReflection;
    EnhancedSlider* m_sldFluidAttenuation;
    EnhancedSlider* m_sldNumIterations;
    EnhancedSlider* m_sldFilterSize;

    EnhancedComboBox* m_cbSkyTexture;
    EnhancedComboBox* m_cbFloorTexture;
    EnhancedSlider*   m_sldFloorSize;
    EnhancedSlider*   m_sldFloorExposure;
    QCheckBox*        m_chkUseAnisotropyKernel;
    MaterialSelector* m_msParticleMaterial;
    MaterialSelector* m_msFluidVolumeMaterial;
    MaterialSelector* m_msMeshMaterial;
    QComboBox*        m_cbMeshMaterialID;

    QPushButton* m_btnPause;
    QPushButton* m_btnNextFrame;
    QPushButton* m_btnReset;
    QPushButton* m_btnReverse;
    QPushButton* m_btnRepeatPlay;
    QPushButton* m_btnClipViewPlane;
    QPushButton* m_btnReloadShaders;
    QPushButton* m_btnReloadTextures;

    QListWidget* m_lstSimInfo;

    QCheckBox*      m_chkRenderShadow;
    QCheckBox*      m_chkVisualizeShadowRegion;
    EnhancedSlider* m_sldShadowIntensity;

    PointLightEditor* m_LightEditor;
};