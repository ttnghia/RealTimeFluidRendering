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

#include "Controller.h"
#include "Common.h"
#include <QtAppHelpers/QtAppUtils.h>

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void Controller::setupGUI()
{
    QVBoxLayout* controlLayout = new QVBoxLayout;
    QVBoxLayout* btnLayout     = new QVBoxLayout;

    setupTextureControllers(controlLayout);
    setupFrameControllers(controlLayout);
    setupFluidRenderModeControllers(controlLayout);
    setupColorControllers(controlLayout);
    setupShadowControllers(controlLayout);
    setupFilterControllers(controlLayout);

    setupButtons(btnLayout);

    ////////////////////////////////////////////////////////////////////////////////
    m_LightEditor = new PointLightEditor(nullptr, this);
    QWidget* mainControls = new QWidget;
    mainControls->setLayout(controlLayout);

    QTabWidget* tabWidget = new QTabWidget;
    tabWidget->setTabPosition(QTabWidget::South);
    tabWidget->setTabShape(QTabWidget::Triangular);
    tabWidget->addTab(mainControls,  "Main Controls");
    tabWidget->addTab(m_LightEditor, "Lights");

    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addWidget(tabWidget);
    mainLayout->addStretch();
    mainLayout->addLayout(btnLayout);
    setLayout(mainLayout);

    ////////////////////////////////////////////////////////////////////////////////
    //    setLayout(controlLayout);
    setFixedWidth(350);
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void Controller::loadTextures()
{
    ////////////////////////////////////////////////////////////////////////////////
    // sky textures
    int currentSkyTexID = m_cbSkyTexture->getComboBox()->currentIndex();
    m_cbSkyTexture->getComboBox()->clear();
    m_cbSkyTexture->getComboBox()->addItem("None");
    auto skyTexFolders = QtAppUtils::getTextureFolders("Sky");
    foreach(QString tex, skyTexFolders) {
        m_cbSkyTexture->getComboBox()->addItem(tex);
    }
    m_cbSkyTexture->getComboBox()->setCurrentIndex(currentSkyTexID > 0 ? currentSkyTexID : 0);

    ////////////////////////////////////////////////////////////////////////////////
    // floor textures
    int currentFloorTexID = m_cbFloorTexture->getComboBox()->currentIndex();
    m_cbFloorTexture->getComboBox()->clear();
    m_cbFloorTexture->getComboBox()->addItem("None");
    auto floorTexFolders = QtAppUtils::getTextureFiles("Floor");
    foreach(QString tex, floorTexFolders) {
        m_cbFloorTexture->getComboBox()->addItem(tex);
    }

    m_cbFloorTexture->getComboBox()->setCurrentIndex(currentFloorTexID > 0 ? currentFloorTexID : 0);
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void Controller::setupTextureControllers(QBoxLayout* ctrLayout)
{
    ////////////////////////////////////////////////////////////////////////////////
    // sky textures
    m_cbSkyTexture = new EnhancedComboBox;
    ctrLayout->addWidget(m_cbSkyTexture->getGroupBox("Sky Texture"));

    ////////////////////////////////////////////////////////////////////////////////
    // floor textures
    m_cbFloorTexture = new EnhancedComboBox;

    m_sldFloorSize = new EnhancedSlider;
    m_sldFloorSize->setRange(1, 200);
    //    m_sldFloorSize->getSlider()->setValue(48); // for fluid emitter + sphere mesh
    m_sldFloorSize->getSlider()->setValue(3);

    m_sldFloorExposure = new EnhancedSlider;
    m_sldFloorExposure->setRange(1, 100);
    m_sldFloorExposure->getSlider()->setValue(50);

    QFrame* line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);

    QVBoxLayout* shadowLayout = new QVBoxLayout;
    shadowLayout->addLayout(m_cbFloorTexture->getLayout());
    shadowLayout->addSpacing(10);
    shadowLayout->addWidget(line);
    shadowLayout->addLayout(m_sldFloorSize->getLayoutWithLabel("Size:"));
    shadowLayout->addLayout(m_sldFloorExposure->getLayoutWithLabel("Exposure:"));

    QGroupBox* shadowGroup = new QGroupBox;
    shadowGroup->setTitle("Floor Texture");
    shadowGroup->setLayout(shadowLayout);
    ctrLayout->addWidget(shadowGroup);

    ////////////////////////////////////////////////////////////////////////////////
    loadTextures();
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void Controller::setupFrameControllers(QBoxLayout* ctrLayout)
{
    ////////////////////////////////////////////////////////////////////////////////
    // delay time between frames
    m_sldFrameDelay = new EnhancedSlider;
    m_sldFrameDelay->setRange(0, 1000);
    m_sldFrameDelay->setTracking(false);
    m_sldFrameDelay->setValue(DEFAULT_DELAY_TIME);

    ///////////////////////////////////////////////////////////////////////////////
    // frame step
    m_sldFrameStep = new EnhancedSlider;
    m_sldFrameStep->setRange(1, 100);
    m_sldFrameStep->setTracking(false);

    ////////////////////////////////////////////////////////////////////////////////
    QVBoxLayout* frameControlLayout = new QVBoxLayout;
    frameControlLayout->addLayout(m_sldFrameDelay->getLayoutWithLabel("Delay:"));
    frameControlLayout->addLayout(m_sldFrameStep->getLayoutWithLabel("Step:"));
    QGroupBox* grpFrameControl = new QGroupBox("Playback Controls");
    grpFrameControl->setLayout(frameControlLayout);
    ctrLayout->addWidget(grpFrameControl);
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void Controller::setupFluidRenderModeControllers(QBoxLayout* ctrLayout)
{
    ////////////////////////////////////////////////////////////////////////////////
    /// particle viewing modes
    QRadioButton* rdbPVSphere    = new QRadioButton("Particle");
    QRadioButton* rdbPVSurface   = new QRadioButton("Surface");
    QRadioButton* rdbPVFluid     = new QRadioButton("Fluid");
    QRadioButton* rdbPVThickness = new QRadioButton("Thickness");
    QRadioButton* rdbPVNormal    = new QRadioButton("Normal");

    rdbPVSphere->setChecked(true);

    QGridLayout* pRenderModesChkLayout = new QGridLayout;
    pRenderModesChkLayout->addWidget(rdbPVSphere,  0, 0);
    pRenderModesChkLayout->addWidget(rdbPVSurface, 0, 1);
    pRenderModesChkLayout->addWidget(rdbPVFluid,   0, 2);
    //    pRenderModesChkLayout->addWidget(rdbPVThickness, 0, 1);
    //    pRenderModesChkLayout->addWidget(rdbPVNormal,    1, 1);

    m_chkUseAnisotropyKernel = new QCheckBox("Use Anisotropy Kernel");
    m_chkUseAnisotropyKernel->setChecked(true);
    QHBoxLayout* renderAnisotropyLayout = new QHBoxLayout;
    renderAnisotropyLayout->addWidget(m_chkUseAnisotropyKernel);

    QFrame* line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);

    QVBoxLayout* pRenderModesLayout = new QVBoxLayout;
    pRenderModesLayout->addLayout(pRenderModesChkLayout);
    pRenderModesLayout->addSpacing(10);
    pRenderModesLayout->addWidget(line);
    pRenderModesLayout->addLayout(renderAnisotropyLayout);
    QGroupBox* pRenderModesGroup = new QGroupBox;
    pRenderModesGroup->setTitle(tr("Render Mode"));
    pRenderModesGroup->setLayout(pRenderModesLayout);

    ctrLayout->addWidget(pRenderModesGroup);

    m_smFluidRenderMode = new QSignalMapper(this);
    connect(rdbPVSphere,    SIGNAL(clicked(bool)), m_smFluidRenderMode, SLOT(map()));
    connect(rdbPVFluid,     SIGNAL(clicked(bool)), m_smFluidRenderMode, SLOT(map()));
    connect(rdbPVSurface,   SIGNAL(clicked(bool)), m_smFluidRenderMode, SLOT(map()));
    connect(rdbPVThickness, SIGNAL(clicked(bool)), m_smFluidRenderMode, SLOT(map()));
    connect(rdbPVNormal,    SIGNAL(clicked(bool)), m_smFluidRenderMode, SLOT(map()));

    m_smFluidRenderMode->setMapping(rdbPVSphere,    static_cast<int>(ParticleRenderMode::SphereParticle));
    m_smFluidRenderMode->setMapping(rdbPVFluid,     static_cast<int>(ParticleRenderMode::TransparentFluid));
    m_smFluidRenderMode->setMapping(rdbPVSurface,   static_cast<int>(ParticleRenderMode::OpaqueSurface));
    m_smFluidRenderMode->setMapping(rdbPVThickness, static_cast<int>(ParticleRenderMode::ThicknessBuffer));
    m_smFluidRenderMode->setMapping(rdbPVNormal,    static_cast<int>(ParticleRenderMode::NormalBuffer));
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void Controller::setupColorControllers(QBoxLayout* ctrLayout)
{
    ////////////////////////////////////////////////////////////////////////////////
    /// color modes
    QRadioButton* rdbColorRandom  = new QRadioButton("Random");
    QRadioButton* rdbColorRamp    = new QRadioButton("Ramp");
    QRadioButton* rdbColorUniform = new QRadioButton("Uniform");

    rdbColorRandom->setChecked(true);

    QGridLayout* colorModeLayout = new QGridLayout;
    colorModeLayout->addWidget(rdbColorRandom,  0, 0);
    colorModeLayout->addWidget(rdbColorRamp,    0, 1);
    colorModeLayout->addWidget(rdbColorUniform, 0, 2);

    QGroupBox* colorModeGroup = new QGroupBox;
    colorModeGroup->setTitle(tr("Particle Color"));
    colorModeGroup->setLayout(colorModeLayout);
    ctrLayout->addWidget(colorModeGroup);

    m_smParticleColorMode = new QSignalMapper(this);
    connect(rdbColorRandom,  SIGNAL(clicked()), m_smParticleColorMode, SLOT(map()));
    connect(rdbColorRamp,    SIGNAL(clicked()), m_smParticleColorMode, SLOT(map()));
    connect(rdbColorUniform, SIGNAL(clicked()), m_smParticleColorMode, SLOT(map()));

    m_smParticleColorMode->setMapping(rdbColorRandom,  static_cast<int>(ParticleColorMode::Random));
    m_smParticleColorMode->setMapping(rdbColorRamp,    static_cast<int>(ParticleColorMode::Ramp));
    m_smParticleColorMode->setMapping(rdbColorUniform, static_cast<int>(ParticleColorMode::Uniform));

    m_msParticleMaterial = new MaterialSelector;
    m_msParticleMaterial->setCustomMaterial(CUSTOM_PARTICLE_MATERIAL);
    m_msParticleMaterial->setDefaultCustomMaterial(true);

    m_msFluidVolumeMaterial = new MaterialSelector;
    m_msFluidVolumeMaterial->setCustomMaterial(CUSTOM_SURFACE_MATERIAL);
    //    m_msFluidVolumeMaterial->setMaterial(DEFAULT_FLUID_VOLUME_MATERIAL);
    m_msFluidVolumeMaterial->setMaterial(CUSTOM_SURFACE_MATERIAL);

    m_msMeshMaterial = new MaterialSelector;
    m_msMeshMaterial->setCustomMaterial(DEFAULT_MESH_MATERIAL);
    m_msMeshMaterial->setMaterial(DEFAULT_MESH_MATERIAL);

    m_cbMeshMaterialID = new QComboBox;

    for(int i = 0; i < MAX_NUM_MESHES; ++i) {
        m_cbMeshMaterialID->addItem(QString("%1").arg(i));
    }

    QHBoxLayout* meshMaterialSelectorLayout = new QHBoxLayout;
    meshMaterialSelectorLayout->addWidget(m_cbMeshMaterialID);
    meshMaterialSelectorLayout->addLayout(m_msMeshMaterial->getLayout());

    QGridLayout* particleColorLayout = new QGridLayout;
    particleColorLayout->addWidget(new QLabel("Particle View: "), 0, 0, Qt::AlignRight);
    particleColorLayout->addLayout(m_msParticleMaterial->getLayout(), 0, 1, 1, 2);
    particleColorLayout->addWidget(new QLabel("Fluid View: "), 1, 0, Qt::AlignRight);
    particleColorLayout->addLayout(m_msFluidVolumeMaterial->getLayout(), 1, 1, 1, 2);
    particleColorLayout->addWidget(new QLabel("Mesh: "), 2, 0, Qt::AlignRight);
    particleColorLayout->addLayout(meshMaterialSelectorLayout, 2, 1, 1, 2);

    QGroupBox* particleColorGroup = new QGroupBox("Material");
    particleColorGroup->setLayout(particleColorLayout);
    ctrLayout->addWidget(particleColorGroup);

    ///////////////////////////////////////////////////////////////////////////////
    // Reflection and attennuation
    m_sldFluidReflection = new EnhancedSlider;
    //    m_sldFluidReflection->setTracking(false);
    m_sldFluidAttenuation = new EnhancedSlider;
    m_sldFluidAttenuation->getSlider()->setValue(100);
    //    m_sldFluidAttenuation->setTracking(false);

    ////////////////////////////////////////////////////////////////////////////////
    QVBoxLayout* fluidColorLayout = new QVBoxLayout;
    fluidColorLayout->addLayout(m_sldFluidReflection->getLayoutWithLabel("Reflection: "));
    fluidColorLayout->addLayout(m_sldFluidAttenuation->getLayoutWithLabel("Attennuation: "));
    QGroupBox* grpFluidColor = new QGroupBox("Fluid Color");
    grpFluidColor->setLayout(fluidColorLayout);
    ctrLayout->addWidget(grpFluidColor);
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void Controller::setupShadowControllers(QBoxLayout* ctrLayout)
{
    ////////////////////////////////////////////////////////////////////////////////
    /// show shadow
    m_chkRenderShadow    = new QCheckBox("Shadow");
    m_sldShadowIntensity = new EnhancedSlider;
    m_sldShadowIntensity->getSlider()->setValue(100);
    m_sldShadowIntensity->setEnabled(false);
    connect(m_chkRenderShadow, &QCheckBox::toggled, m_sldShadowIntensity, &EnhancedSlider::setEnabled);

    m_chkVisualizeShadowRegion = new QCheckBox("Hightlight Shadow FOV");
    m_chkVisualizeShadowRegion->setEnabled(false);
    connect(m_chkRenderShadow, &QCheckBox::toggled, m_chkVisualizeShadowRegion, &QCheckBox::setEnabled);

    QFrame* line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);

    QVBoxLayout* shadowLayout = new QVBoxLayout;
    shadowLayout->addWidget(m_chkRenderShadow);
    shadowLayout->addWidget(m_chkVisualizeShadowRegion);
    shadowLayout->addSpacing(10);
    shadowLayout->addWidget(line);
    shadowLayout->addLayout(m_sldShadowIntensity->getLayoutWithLabel("Intensity:"));

    QGroupBox* shadowGroup = new QGroupBox;
    shadowGroup->setTitle("Render Shadow");
    shadowGroup->setLayout(shadowLayout);
    ctrLayout->addWidget(shadowGroup);
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void Controller::setupFilterControllers(QBoxLayout* ctrLayout)
{
    ////////////////////////////////////////////////////////////////////////////////
    /// filter method
    QRadioButton* rdbBilateralGaussian    = new QRadioButton(QString("Bilateral Gaussian"));
    QRadioButton* rdbCurvatureFlow        = new QRadioButton(QString("Curvature Flow"));
    QRadioButton* rdbPlaneFitting         = new QRadioButton(QString("Plane Fitting"));
    QRadioButton* rdbModifiedGaussian1D2D = new QRadioButton(QString("NRR. Filter 1D2D"));
    QRadioButton* rdbModifiedGaussian1D   = new QRadioButton(QString("NRR. Filter 1D"));
    QRadioButton* rdbModifiedGaussian2D   = new QRadioButton(QString("NRR. Filter 2D"));

    QGridLayout* filterMethodLayout = new QGridLayout;
    filterMethodLayout->addWidget(rdbBilateralGaussian,    0, 0);
    filterMethodLayout->addWidget(rdbCurvatureFlow,        0, 1);
    filterMethodLayout->addWidget(rdbPlaneFitting,         1, 0);
    filterMethodLayout->addWidget(rdbModifiedGaussian1D2D, 1, 1);
    filterMethodLayout->addWidget(rdbModifiedGaussian1D,   2, 0);
    filterMethodLayout->addWidget(rdbModifiedGaussian2D,   2, 1);

    QGroupBox* filterMethodGroup = new QGroupBox;
    filterMethodGroup->setTitle(tr("Surface Filter Method"));
    filterMethodGroup->setLayout(filterMethodLayout);
    ctrLayout->addWidget(filterMethodGroup);

    m_smFilterMethod = new QSignalMapper(this);
    connect(rdbBilateralGaussian,    SIGNAL(clicked()), m_smFilterMethod, SLOT(map()));
    connect(rdbCurvatureFlow,        SIGNAL(clicked()), m_smFilterMethod, SLOT(map()));
    connect(rdbPlaneFitting,         SIGNAL(clicked()), m_smFilterMethod, SLOT(map()));
    connect(rdbModifiedGaussian1D2D, SIGNAL(clicked()), m_smFilterMethod, SLOT(map()));
    connect(rdbModifiedGaussian1D,   SIGNAL(clicked()), m_smFilterMethod, SLOT(map()));
    connect(rdbModifiedGaussian2D,   SIGNAL(clicked()), m_smFilterMethod, SLOT(map()));

    m_smFilterMethod->setMapping(rdbBilateralGaussian,    static_cast<int>(FilterMethod::BilateralGaussian));
    m_smFilterMethod->setMapping(rdbCurvatureFlow,        static_cast<int>(FilterMethod::CurvatureFlow));
    m_smFilterMethod->setMapping(rdbPlaneFitting,         static_cast<int>(FilterMethod::PlaneFitting));
    m_smFilterMethod->setMapping(rdbModifiedGaussian1D2D, static_cast<int>(FilterMethod::NarrowRangeFilter1D2D));
    m_smFilterMethod->setMapping(rdbModifiedGaussian1D,   static_cast<int>(FilterMethod::NarrowRangeFilter1D));
    m_smFilterMethod->setMapping(rdbModifiedGaussian2D,   static_cast<int>(FilterMethod::NarrowRangeFilter2D));
    rdbBilateralGaussian->setChecked(true);

    ///////////////////////////////////////////////////////////////////////////////
    // num iteration
    m_sldNumIterations = new EnhancedSlider;
    m_sldNumIterations->setRange(0, 200);
    m_sldNumIterations->setValue(DEFAULT_NUM_FILTER_ITERATION);
    m_sldNumIterations->setTracking(false);

    ///////////////////////////////////////////////////////////////////////////////
    // BasicFilterSize
    m_sldFilterSize = new EnhancedSlider;
    m_sldFilterSize->setTracking(false);
    m_sldFilterSize->setValue(DEFAULT_FILTER_SIZE);

    ////////////////////////////////////////////////////////////////////////////////
    QVBoxLayout* filterControlLayout = new QVBoxLayout;
    filterControlLayout->addLayout(m_sldNumIterations->getLayoutWithLabel("Num. Iter:"));
    filterControlLayout->addLayout(m_sldFilterSize->getLayoutWithLabel("Filter Size:"));
    QGroupBox* grpFilterControl = new QGroupBox("Filter Controls");
    grpFilterControl->setLayout(filterControlLayout);
    ctrLayout->addWidget(grpFilterControl);
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void Controller::setupButtons(QBoxLayout* ctrLayout)
{
    ////////////////////////////////////////////////////////////////////////////////
    /// reload
    m_btnReloadShaders  = new QPushButton("Reload Shaders");
    m_btnReloadTextures = new QPushButton("Reload Textures");

    QGridLayout* btnReloadLayout = new QGridLayout;
    btnReloadLayout->addWidget(m_btnReloadShaders,  0, 0, 1, 1);
    btnReloadLayout->addWidget(m_btnReloadTextures, 0, 1, 1, 1);
    ctrLayout->addLayout(btnReloadLayout);

    ////////////////////////////////////////////////////////////////////////////////
    /// other buttons
    m_btnPause = new QPushButton(QString("Pause"));
    m_btnPause->setCheckable(true);
    m_btnNextFrame = new QPushButton(QString("Next Frame"));
    m_btnReset     = new QPushButton(QString("Reset"));
    m_btnReverse   = new QPushButton(QString("Reverse"));
    m_btnReverse->setCheckable(true);
    m_btnRepeatPlay = new QPushButton(QString("Repeat"));
    m_btnRepeatPlay->setCheckable(true);
    m_btnClipViewPlane = new QPushButton(QString("Clip View"));
    m_btnClipViewPlane->setCheckable(true);

    QGridLayout* btnLayout = new QGridLayout;
    btnLayout->addWidget(m_btnPause,         0, 0, 1, 1);
    btnLayout->addWidget(m_btnNextFrame,     0, 1, 1, 1);
    btnLayout->addWidget(m_btnReset,         1, 0, 1, 1);
    btnLayout->addWidget(m_btnRepeatPlay,    1, 1, 1, 1);
    btnLayout->addWidget(m_btnReverse,       2, 0, 1, 1);
    btnLayout->addWidget(m_btnClipViewPlane, 2, 1, 1, 1);
    ctrLayout->addLayout(btnLayout);
}