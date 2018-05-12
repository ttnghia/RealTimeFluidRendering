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

#include <QMouseEvent>
#include <Banana/Utils/NumberHelpers.h>
#include "MainWindow.h"

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
MainWindow::MainWindow(QWidget* parent) : OpenGLMainWindow(parent)
{
    instantiateOpenGLWidget();
    setupRenderWidgets();
    setupPlayList();
    setupStatusBar();
    connectWidgets();
    setArthurStyle();

    setWindowTitle("Screen Space Fluid Rendering");
    setFocusPolicy(Qt::StrongFocus);
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void MainWindow::instantiateOpenGLWidget()
{
    if(m_GLWidget != nullptr) {
        delete m_GLWidget;
    }

    m_RenderWidget = new FluidRenderWidget(this);
    setupOpenglWidget(m_RenderWidget);
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
bool MainWindow::processKeyPressEvent(QKeyEvent* event)
{
    switch(event->key()) {
        case Qt::Key_B:
            m_InputPath->browse();
            return true;

        case Qt::Key_O:
            m_OutputPath->browse();
            return true;

        //    case Qt::Key_C:
        //        renderer->resetCameraPosition();

        case Qt::Key_R:
            m_Controller->m_btnReverse->click();
            return true;

        case Qt::Key_Space:
            m_Controller->m_btnPause->click();
            return true;

        case Qt::Key_N:
            m_Controller->m_btnNextFrame->click();
            return true;

        case Qt::Key_X:
            m_Controller->m_btnClipViewPlane->click();
            return true;

        case Qt::Key_F1:
            m_Controller->m_btnReset->click();
            return true;

        case Qt::Key_F5:
            loadDataInfo(m_InputPath->getCurrentPath());
            return true;

        case Qt::Key_F9:
            m_ClipPlaneEditor->show();
            return true;

        ////////////////////////////////////////////////////////////////////////////////
        //    case Qt::Key_L:
        //        loadDataInfo(QString("D:/OneDrive/Simulation/FluidSphere2"));
        //        return true;

        default:
            return OpenGLMainWindow::processKeyPressEvent(event);
    }
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void MainWindow::showEvent(QShowEvent* ev)
{
    QMainWindow::showEvent(ev);

    if(m_DataList->getListSize() > 0) {
        QTimer::singleShot(100, this, [&]()
                           {
                               m_DataList->show();
                               m_DataList->setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignBottom | Qt::AlignRight, m_DataList->size(), qApp->desktop()->availableGeometry()));
                           });
    }
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void MainWindow::updateStatusCurrentFrame(int currentFrame)
{
    m_lblStatusCurrentFrame->setText(QString("Current frame: %1").arg(currentFrame));
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void MainWindow::updateStatusNumParticles()
{
    m_lblStatusNumParticles->setText(QString("Particles: %1 | Mesh(es): %2")
                                         .arg(QString::fromStdString(NumberHelpers::formatWithCommas(m_DataManager->getDataInfo()->num_particles)))
                                         .arg(QString::fromStdString(NumberHelpers::formatWithCommas(m_DataManager->getDataInfo()->num_meshes))));
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void MainWindow::updateStatusNumMeshes()
{
    m_lblStatusNumParticles->setText(QString("Particles: %1 | Mesh(es): %2")
                                         .arg(QString::fromStdString(NumberHelpers::formatWithCommas(m_DataManager->getDataInfo()->num_particles)))
                                         .arg(QString::fromStdString(NumberHelpers::formatWithCommas(m_DataManager->getDataInfo()->num_meshes))));
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void MainWindow::updateNumFrames(int numFrames)
{
    m_DataReader->setNumFrames(numFrames);
    m_sldFrame->setRange(1, numFrames);

    ////////////////////////////////////////////////////////////////////////////////
    m_lblStatusNumFrames->setText(QString("Total frame: %1").arg(numFrames));
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void MainWindow::updateStatusReadInfo(double readTime, size_t bytes)
{
    m_lblStatusReadInfo->setText(QString("Load data time: %1 ms (%2 MB)").arg(readTime).arg(static_cast<double>(bytes) / 1048576.0));
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void MainWindow::loadDataInfo(QString dataPath)
{
    if(m_DataManager->setDataPath(dataPath)) {
        assert(m_RenderWidget != nullptr);
        m_OutputPath->setPath(dataPath);
        m_DataReader->setParticleDataObj(m_RenderWidget->getParticleDataObj());
        m_DataReader->setMeshObj(m_RenderWidget->getMeshObjs());

        m_RenderWidget->getParticleDataObj()->resize(0);
        m_RenderWidget->updateNumMeshes(0);

        const std::shared_ptr<SimulationDataInfo>& dataInfo = m_DataManager->getDataInfo();
        m_DataReader->setDataPath(dataPath, dataInfo);

        m_RenderWidget->setBox(glm::vec3(dataInfo->movable_min[0], dataInfo->movable_min[1], dataInfo->movable_min[2]),
                               glm::vec3(dataInfo->movable_max[0], dataInfo->movable_max[1], dataInfo->movable_max[2]));

        if(dataInfo->hasCameraInfo) {
            m_RenderWidget->setCamera(glm::vec3(dataInfo->camera_position[0], dataInfo->camera_position[1], dataInfo->camera_position[2]),
                                      glm::vec3(dataInfo->camera_focus[0], dataInfo->camera_focus[1], dataInfo->camera_focus[2]));
        } else {
            m_RenderWidget->setCamera(DEFAULT_CAMERA_POSITION, DEFAULT_CAMERA_FOCUS);
        }

        if(dataInfo->hasLightInfo) {
            auto& light = m_RenderWidget->getLightObjs();
            light->setNumLights(1);
            light->setLightPosition(Vec4f(dataInfo->light_position[0], dataInfo->light_position[1], dataInfo->light_position[2], 1.0), 0);
            m_RenderWidget->updateLights();
            m_Controller->m_LightEditor->lightToGUI();
        }
    }
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void MainWindow::setupRenderWidgets()
{
    ////////////////////////////////////////////////////////////////////////////////
    // slider
    m_sldFrame = new QSlider(Qt::Horizontal);
    m_sldFrame->setRange(1, 1);
    m_sldFrame->setValue(1);
    m_sldFrame->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    //    m_sldFrame->setTracking(false);

    ////////////////////////////////////////////////////////////////////////////////
    QHBoxLayout* inputOutputLayout = new QHBoxLayout;
    setupDataWidgets(inputOutputLayout);

    ////////////////////////////////////////////////////////////////////////////////
    // setup layouts
    QVBoxLayout* renderLayout = new QVBoxLayout;
    renderLayout->addWidget(m_GLWidget, 1);
    renderLayout->addWidget(m_sldFrame);
    renderLayout->addLayout(inputOutputLayout);

    m_Controller = new Controller(this);

    QHBoxLayout* mainLayout = new QHBoxLayout;
    mainLayout->addLayout(renderLayout);
    mainLayout->addWidget(m_Controller);

    QWidget* mainWidget = new QWidget(this);
    mainWidget->setLayout(mainLayout);
    setCentralWidget(mainWidget);
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void MainWindow::setupPlayList()
{
    assert(m_DataList != nullptr);
    m_DataList->setWindowTitle("Simulation List");

    ////////////////////////////////////////////////////////////////////////////////
    const QString listFile(QDir::currentPath() + "/PlayList.txt");

    if(QFile::exists(listFile)) {
        m_DataList->loadListFromFile(listFile);
    } else {
        qDebug() << "PlayList.txt does not exist. No play list loaded";
    }
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void MainWindow::setupDataWidgets(QLayout* dataLayout)
{
    ////////////////////////////////////////////////////////////////////////////////
    // input data path
    m_InputPath = new BrowsePathWidget(QIcon(":/Icons/open.png"));

    ////////////////////////////////////////////////////////////////////////////////
    // output image path
    m_chkExportFrame = new QCheckBox("Export to Images");
    m_OutputPath     = new BrowsePathWidget(QIcon(":/Icons/save.png"));

    QHBoxLayout* exportImageLayout = new QHBoxLayout;
    exportImageLayout->addWidget(m_chkExportFrame);
    exportImageLayout->addLayout(m_OutputPath->getLayout());

    QGroupBox* grExportImage = new QGroupBox("Image Output Path");
    grExportImage->setLayout(exportImageLayout);

    ////////////////////////////////////////////////////////////////////////////////
    dataLayout->addWidget(m_InputPath->getGroupBox("Input Path"));
    dataLayout->addWidget(grExportImage);
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void MainWindow::setupStatusBar()
{
    m_lblStatusCurrentFrame = new QLabel(this);
    m_lblStatusCurrentFrame->setMargin(5);
    statusBar()->addPermanentWidget(m_lblStatusCurrentFrame, 1);

    m_lblStatusNumFrames = new QLabel(this);
    m_lblStatusNumFrames->setMargin(5);
    statusBar()->addPermanentWidget(m_lblStatusNumFrames, 1);

    m_lblStatusNumParticles = new QLabel(this);
    m_lblStatusNumParticles->setMargin(5);
    statusBar()->addPermanentWidget(m_lblStatusNumParticles, 1);

    m_lblStatusReadInfo = new QLabel(this);
    m_lblStatusReadInfo->setMargin(5);
    statusBar()->addPermanentWidget(m_lblStatusReadInfo, 2);
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void MainWindow::connectWidgets()
{
    ////////////////////////////////////////////////////////////////////////////////
    // textures
    connect(m_Controller->m_cbSkyTexture->getComboBox(),   SIGNAL(currentIndexChanged(int)), m_RenderWidget, SLOT(setSkyBoxTexture(int)));
    connect(m_Controller->m_cbFloorTexture->getComboBox(), SIGNAL(currentIndexChanged(int)), m_RenderWidget, SLOT(setFloorTexture(int)));
    connect(m_Controller->m_sldFloorSize->getSlider(),     SIGNAL(valueChanged(int)),        m_RenderWidget, SLOT(setFloorSize(int)));

    ////////////////////////////////////////////////////////////////////////////////
    // frame params
    connect(m_Controller->m_sldFrameDelay->getSlider(), SIGNAL(valueChanged(int)), m_DataReader.get(), SLOT(setDelayTime(int)));
    connect(m_Controller->m_sldFrameStep->getSlider(),  SIGNAL(valueChanged(int)), m_DataReader.get(), SLOT(setFrameStep(int)));

    ////////////////////////////////////////////////////////////////////////////////
    // render modes/colors
    connect(m_Controller->m_smFluidRenderMode,     SIGNAL(mapped(int)),                             m_RenderWidget, SLOT(setFluidRenderMode(int)));
    connect(m_Controller->m_smParticleColorMode,   SIGNAL(mapped(int)),                             m_RenderWidget, SLOT(setParticleColorMode(int)));
    connect(m_Controller->m_msParticleMaterial,    SIGNAL(materialChanged(Material::MaterialData)), m_RenderWidget, SLOT(setParticleMaterial(Material::MaterialData)));
    connect(m_Controller->m_msFluidVolumeMaterial, SIGNAL(materialChanged(Material::MaterialData)), m_RenderWidget, SLOT(setFluidVolumeMaterial(Material::MaterialData)));
    connect(m_Controller->m_msMeshMaterial,        &MaterialSelector::materialChanged,              [&](const Material::MaterialData& material)
            {
                m_RenderWidget->setMeshMaterial(material, m_Controller->m_cbMeshMaterialID->currentIndex());
            });
    connect(m_Controller->m_chkUseAnisotropyKernel, SIGNAL(toggled(bool)), m_RenderWidget,     SLOT(enableAnisotropyKernel(bool)));
    connect(m_Controller->m_chkUseAnisotropyKernel, SIGNAL(toggled(bool)), m_DataReader.get(), SLOT(enableAnisotropyKernel(bool)));

    ////////////////////////////////////////////////////////////////////////////////
    // fluid render
    connect(m_Controller->m_sldFluidReflection->getSlider(),  &QSlider::valueChanged, m_RenderWidget, &FluidRenderWidget::setSurfaceReflectionConstant);
    connect(m_Controller->m_sldFluidAttenuation->getSlider(), &QSlider::valueChanged, m_RenderWidget, &FluidRenderWidget::setFluidAttennuationConstant);

    ////////////////////////////////////////////////////////////////////////////////
    // filter
    connect(m_Controller->m_smFilterMethod,                SIGNAL(mapped(int)),       m_RenderWidget, SLOT(setFilterMethod(int)));
    connect(m_Controller->m_sldNumIterations->getSlider(), SIGNAL(valueChanged(int)), m_RenderWidget, SLOT(setNumFilterIteration(int)));
    connect(m_Controller->m_sldFilterSize->getSlider(),    SIGNAL(valueChanged(int)), m_RenderWidget, SLOT(setFilterSize(int)));

    ////////////////////////////////////////////////////////////////////////////////
    // buttons
    connect(m_Controller->m_btnReloadShaders,  SIGNAL(clicked(bool)), m_RenderWidget,     SLOT(reloadShaders()));
    connect(m_Controller->m_btnReloadTextures, SIGNAL(clicked(bool)), m_RenderWidget,     SLOT(reloadTextures()));
    connect(m_Controller->m_btnReloadTextures, SIGNAL(clicked(bool)), m_Controller,       SLOT(loadTextures()));
    connect(m_Controller->m_btnPause,          SIGNAL(clicked(bool)), m_DataReader.get(), SLOT(pause(bool)));
    connect(m_Controller->m_btnNextFrame,      SIGNAL(clicked(bool)), m_DataReader.get(), SLOT(readNextFrame()));
    connect(m_Controller->m_btnReset,          SIGNAL(clicked(bool)), m_DataReader.get(), SLOT(resetToFirstFrame()));
    connect(m_Controller->m_btnRepeatPlay,     SIGNAL(clicked(bool)), m_DataReader.get(), SLOT(enableRepeat(bool)));
    connect(m_Controller->m_btnReverse,        SIGNAL(clicked(bool)), m_DataReader.get(), SLOT(enableReverse(bool)));
    connect(m_Controller->m_btnClipViewPlane,  SIGNAL(clicked(bool)), m_RenderWidget,     SLOT(enableClipPlane(bool)));

    ////////////////////////////////////////////////////////////////////////////////
    //  data handle
    connect(m_DataManager.get(),     SIGNAL(numFramesChanged(int)),           this,               SLOT(updateNumFrames(int)));
    connect(m_DataManager.get(),     SIGNAL(numFramesChanged(int)),           m_DataReader.get(), SLOT(setNumFrames(int)));
    connect(m_InputPath,             SIGNAL(pathChanged(QString)),            this,               SLOT(loadDataInfo(QString)));
    connect(m_OutputPath,            SIGNAL(pathChanged(QString)),            m_RenderWidget,     SLOT(setCapturePath(QString)));
    connect(m_chkExportFrame,        &QCheckBox::toggled,                     m_RenderWidget,     &FluidRenderWidget::enableExportFrame);

    connect(m_DataReader.get(),      SIGNAL(currentFrameChanged(int)),        this,               SLOT(updateStatusCurrentFrame(int)));
    connect(m_DataReader.get(),      SIGNAL(currentFrameChanged(int)),        m_sldFrame,         SLOT(setValue(int)));
    connect(m_sldFrame,              SIGNAL(valueChanged(int)),               m_DataReader.get(), SLOT(readFrame(int)));

    connect(m_DataReader.get(),      SIGNAL(numParticlesChanged(int)),        this,               SLOT(updateStatusNumParticles()));
    connect(m_DataReader.get(),      SIGNAL(readInfoChanged(double, size_t)), this,               SLOT(updateStatusReadInfo(double, size_t)));

    connect(m_DataReader.get(),      SIGNAL(particleDataChanged()),           m_RenderWidget,     SLOT(updateParticleData()));
    connect(m_DataReader.get(),      SIGNAL(meshesChanged()),                 m_RenderWidget,     SLOT(updateMeshes()));
    connect(m_DataReader.get(),      SIGNAL(numMeshesChanged(int)),           m_RenderWidget,     SLOT(updateNumMeshes(int)));
    connect(m_DataReader.get(),      SIGNAL(numMeshesChanged(int)),           this,               SLOT(updateStatusNumMeshes()));

    connect(m_DataList.get(),        SIGNAL(currentTextChanged(QString)),     this,               SLOT(loadDataInfo(QString)));
    connect(m_DataList.get(),        SIGNAL(currentTextChanged(QString)),     m_InputPath,        SLOT(setPath(QString)));

    connect(m_ClipPlaneEditor.get(), SIGNAL(clipPlaneChanged(Vec4f)),         m_RenderWidget,     SLOT(setClipPlane(Vec4f)));

    ////////////////////////////////////////////////////////////////////////////////
    // lights
    connect(m_Controller->m_LightEditor,                     &PointLightEditor::lightsChanged,     m_RenderWidget,              &FluidRenderWidget::updateLights);
    connect(m_RenderWidget,                                  &FluidRenderWidget::lightsObjChanged, m_Controller->m_LightEditor, &PointLightEditor::setLightObject);
    connect(m_Controller->m_chkRenderShadow,                 &QCheckBox::toggled,                  m_RenderWidget,              &FluidRenderWidget::enableShadow);
    connect(m_Controller->m_chkVisualizeShadowRegion,        &QCheckBox::toggled,                  m_RenderWidget,              &FluidRenderWidget::visualizeShadowRegion);
    connect(m_Controller->m_sldShadowIntensity->getSlider(), &QSlider::valueChanged,               m_RenderWidget,              &FluidRenderWidget::setShadowIntensity);
}