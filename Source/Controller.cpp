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
    setupPlaybackControllers();
    setupFluidRenderModeControllers();
    setupRenderMaterialControllers();
    setupShadowControllers();
    setupFilterControllers();
    setupButtons();
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void Controller::connectWidgets()
{
    ////////////////////////////////////////////////////////////////////////////////
    // textures
    disconnect(m_cbFloorTexture->getComboBox(), 0, 0, 0);
    disconnect(m_sldFloorSize->getSlider(),     0, 0, 0);
    disconnect(m_sldFloorExposure->getSlider(), 0, 0, 0);
    connect(m_cbFloorTexture->getComboBox(), SIGNAL(currentIndexChanged(int)), m_RenderWidget, SLOT(setFluidFloorTexture(int)));
    connect(m_sldFloorSize->getSlider(),          &QSlider::valueChanged,      m_RenderWidget,    &RenderWidget::setFluidFloorSize);
    connect(m_sldFloorExposure->getSlider(),      &QSlider::valueChanged,      m_RenderWidget,    &RenderWidget::setFluidFloorExposure);
    ////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////
    // render modes/colors
    connect(m_smFluidRenderMode,      SIGNAL(mapped(int)),                             m_RenderWidget, SLOT(setParticleRenderMode(int)));
    connect(m_chkUseAnisotropyKernel, SIGNAL(toggled(bool)),                           m_RenderWidget, SLOT(enableAnisotropyKernel(bool)));
    connect(m_smParticleColorMode,    SIGNAL(mapped(int)),                             m_RenderWidget, SLOT(setParticleColorMode(int)));

    connect(m_msParticleMaterial,     SIGNAL(materialChanged(Material::MaterialData)), m_RenderWidget, SLOT(setParticleMaterial(Material::MaterialData)));
    connect(m_msFluidViewMaterial,    SIGNAL(materialChanged(Material::MaterialData)), m_RenderWidget, SLOT(setFluidViewMaterial(Material::MaterialData)));
    connect(m_msMeshMaterial,              &MaterialSelector::materialChanged,                      [&](const Material::MaterialData& material)
            {
                m_RenderWidget->setMeshMaterial(material, m_cbMeshMaterialID->currentIndex());
            });
    ////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////
    // render effects
    connect(m_sldFluidReflection->getSlider(),  &QSlider::valueChanged, m_RenderWidget, &RenderWidget::setSurfaceReflectionConstant);
    connect(m_sldFluidAttenuation->getSlider(), &QSlider::valueChanged, m_RenderWidget, &RenderWidget::setFluidAttennuationConstant);

    ////////////////////////////////////////////////////////////////////////////////
    // shadow
    connect(m_chkRenderShadow,                 &QCheckBox::toggled,    m_RenderWidget, &RenderWidget::enableShadow);
    connect(m_chkVisualizeShadowRegion,        &QCheckBox::toggled,    m_RenderWidget, &RenderWidget::visualizeShadowRegion);
    connect(m_sldShadowIntensity->getSlider(), &QSlider::valueChanged, m_RenderWidget, &RenderWidget::setShadowIntensity);
    ////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////
    // surface filter
    connect(m_smFilterMethod,                SIGNAL(mapped(int)),       m_RenderWidget, SLOT(setFilterMethod(int)));
    connect(m_sldNumIterations->getSlider(), SIGNAL(valueChanged(int)), m_RenderWidget, SLOT(setNumFilterIteration(int)));
    connect(m_sldFilterSize->getSlider(),    SIGNAL(valueChanged(int)), m_RenderWidget, SLOT(setFilterSize(int)));

    ////////////////////////////////////////////////////////////////////////////////
    // buttons
    connect(m_btnReloadShaders,  &QPushButton::clicked, m_RenderWidget, &RenderWidget::reloadShaders);
    connect(m_btnReloadTextures, &QPushButton::clicked, m_RenderWidget, &RenderWidget::reloadTextures);
    connect(m_btnReloadTextures, &QPushButton::clicked,           this, &Controller::reloadTextures);
    connect(m_btnClipViewPlane,  &QPushButton::clicked, m_RenderWidget, &RenderWidget::enableClipPlane);
    connect(m_btnResetCamera,    &QPushButton::clicked, m_RenderWidget, &RenderWidget::resetCameraPosition);
    //////////////////////////////////////////////////////////////////////////////
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void Controller::setupPlaybackControllers()
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
    QGroupBox* grpPlaybackControl = new QGroupBox("Playback Controls");
    grpPlaybackControl->setLayout(frameControlLayout);
    m_LayoutRenderControllers->addWidget(grpPlaybackControl);
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void Controller::setupFluidRenderModeControllers()
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

    QVBoxLayout* pRenderModesLayout = new QVBoxLayout;
    pRenderModesLayout->addLayout(pRenderModesChkLayout);
    pRenderModesLayout->addSpacing(10);
    pRenderModesLayout->addWidget(QtAppUtils::getLineSeparator());
    pRenderModesLayout->addLayout(renderAnisotropyLayout);
    QGroupBox* pRenderModesGroup = new QGroupBox;
    pRenderModesGroup->setTitle(tr("Render Mode"));
    pRenderModesGroup->setLayout(pRenderModesLayout);

    m_LayoutRenderControllers->addWidget(pRenderModesGroup);

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
void Controller::setupRenderMaterialControllers()
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
    m_LayoutRenderControllers->addWidget(colorModeGroup);

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

    m_msFluidViewMaterial = new MaterialSelector;
    m_msFluidViewMaterial->setCustomMaterial(CUSTOM_SURFACE_MATERIAL);
    //    m_msFluidVolumeMaterial->setMaterial(DEFAULT_FLUID_VOLUME_MATERIAL);
    m_msFluidViewMaterial->setMaterial(CUSTOM_SURFACE_MATERIAL);

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
    particleColorLayout->addLayout(m_msFluidViewMaterial->getLayout(), 1, 1, 1, 2);
    particleColorLayout->addWidget(new QLabel("Mesh: "), 2, 0, Qt::AlignRight);
    particleColorLayout->addLayout(meshMaterialSelectorLayout, 2, 1, 1, 2);

    QGroupBox* particleColorGroup = new QGroupBox("Material");
    particleColorGroup->setLayout(particleColorLayout);
    m_LayoutRenderControllers->addWidget(particleColorGroup);

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
    QGroupBox* grpFluidColor = new QGroupBox("Render effects");
    grpFluidColor->setLayout(fluidColorLayout);
    m_LayoutRenderControllers->addWidget(grpFluidColor);
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void Controller::setupShadowControllers()
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

    QVBoxLayout* shadowLayout = new QVBoxLayout;
    shadowLayout->addWidget(m_chkRenderShadow);
    shadowLayout->addWidget(m_chkVisualizeShadowRegion);
    shadowLayout->addSpacing(10);
    shadowLayout->addWidget(QtAppUtils::getLineSeparator());
    shadowLayout->addLayout(m_sldShadowIntensity->getLayoutWithLabel("Intensity:"));

    QGroupBox* shadowGroup = new QGroupBox;
    shadowGroup->setTitle("Render Shadow");
    shadowGroup->setLayout(shadowLayout);
    m_LayoutRenderControllers->addWidget(shadowGroup);
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void Controller::setupFilterControllers()
{
    ////////////////////////////////////////////////////////////////////////////////
    /// filter method
    QRadioButton* rdbBilateralGaussian = new QRadioButton(QString("Bilateral Gaussian"));
    QRadioButton* rdbCurvatureFlow     = new QRadioButton(QString("Curvature Flow"));
    QRadioButton* rdbPlaneFitting      = new QRadioButton(QString("Plane Fitting"));
    QRadioButton* rdbNarrowRange1D2D   = new QRadioButton(QString("NRR. Filter 1D2D"));
    QRadioButton* rdbNarrowRange1D     = new QRadioButton(QString("NRR. Filter 1D"));
    QRadioButton* rdbNarrowRange2D     = new QRadioButton(QString("NRR. Filter 2D"));

    QGridLayout* filterMethodLayout = new QGridLayout;
    filterMethodLayout->addWidget(rdbBilateralGaussian, 0, 0);
    filterMethodLayout->addWidget(rdbCurvatureFlow,     0, 1);
    filterMethodLayout->addWidget(rdbPlaneFitting,      1, 0);
    filterMethodLayout->addWidget(rdbNarrowRange1D2D,   1, 1);
    filterMethodLayout->addWidget(rdbNarrowRange1D,     2, 0);
    filterMethodLayout->addWidget(rdbNarrowRange2D,     2, 1);

    QGroupBox* filterMethodGroup = new QGroupBox;
    filterMethodGroup->setTitle(tr("Surface Filter Method"));
    filterMethodGroup->setLayout(filterMethodLayout);
    m_LayoutRenderControllers->addWidget(filterMethodGroup);

    m_smFilterMethod = new QSignalMapper(this);
    connect(rdbBilateralGaussian, SIGNAL(clicked()), m_smFilterMethod, SLOT(map()));
    connect(rdbCurvatureFlow,     SIGNAL(clicked()), m_smFilterMethod, SLOT(map()));
    connect(rdbPlaneFitting,      SIGNAL(clicked()), m_smFilterMethod, SLOT(map()));
    connect(rdbNarrowRange1D2D,   SIGNAL(clicked()), m_smFilterMethod, SLOT(map()));
    connect(rdbNarrowRange1D,     SIGNAL(clicked()), m_smFilterMethod, SLOT(map()));
    connect(rdbNarrowRange2D,     SIGNAL(clicked()), m_smFilterMethod, SLOT(map()));

    m_smFilterMethod->setMapping(rdbBilateralGaussian, static_cast<int>(FilterMethod::BilateralGaussian));
    m_smFilterMethod->setMapping(rdbCurvatureFlow,     static_cast<int>(FilterMethod::CurvatureFlow));
    m_smFilterMethod->setMapping(rdbPlaneFitting,      static_cast<int>(FilterMethod::PlaneFitting));
    m_smFilterMethod->setMapping(rdbNarrowRange1D2D,   static_cast<int>(FilterMethod::NarrowRangeFilter1D2D));
    m_smFilterMethod->setMapping(rdbNarrowRange1D,     static_cast<int>(FilterMethod::NarrowRangeFilter1D));
    m_smFilterMethod->setMapping(rdbNarrowRange2D,     static_cast<int>(FilterMethod::NarrowRangeFilter2D));
    rdbNarrowRange1D2D->setChecked(true);

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
    m_LayoutRenderControllers->addWidget(grpFilterControl);
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void Controller::setupButtons()
{
    ////////////////////////////////////////////////////////////////////////////////
    /// reload
    m_btnReloadShaders  = new QPushButton("Reload Shaders");
    m_btnReloadTextures = new QPushButton("Reload Textures");
    m_btnPause          = new QPushButton(QString("Pause"));
    m_btnPause->setCheckable(true);
    m_btnNextFrame = new QPushButton(QString("Next Frame"));

    QGridLayout* layout2Columns = new QGridLayout;
    layout2Columns->addWidget(m_btnReloadShaders,  0, 0, 1, 1);
    layout2Columns->addWidget(m_btnReloadTextures, 0, 1, 1, 1);
    layout2Columns->addWidget(m_btnPause,          1, 0, 1, 1);
    layout2Columns->addWidget(m_btnNextFrame,      1, 1, 1, 1);
    m_MainLayout->addStretch();
    m_MainLayout->addLayout(layout2Columns);

    ////////////////////////////////////////////////////////////////////////////////
    /// other buttons
    m_btnReset   = new QPushButton(QString("Reset"));
    m_btnReverse = new QPushButton(QString("Reverse"));
    m_btnReverse->setCheckable(true);
    m_btnRepeatPlay = new QPushButton(QString("Repeat"));
    m_btnRepeatPlay->setCheckable(true);

    m_btnClipViewPlane = new QPushButton(QString("Clip View"));
    m_btnClipViewPlane->setCheckable(true);
    m_btnEditClipViewPlane = new QPushButton(QString("Edit Clip Plane"));
    m_btnResetCamera       = new QPushButton(QString("Reset Camera"));

    QGridLayout* layout3Columns = new QGridLayout;
    layout3Columns->addWidget(m_btnReset,             0, 0, 1, 1);
    layout3Columns->addWidget(m_btnRepeatPlay,        0, 1, 1, 1);
    layout3Columns->addWidget(m_btnReverse,           0, 2, 1, 1);

    layout3Columns->addWidget(m_btnClipViewPlane,     1, 0, 1, 1);
    layout3Columns->addWidget(m_btnEditClipViewPlane, 1, 1, 1, 1);
    layout3Columns->addWidget(m_btnResetCamera,       1, 2, 1, 1);
    m_MainLayout->addLayout(layout3Columns);
}