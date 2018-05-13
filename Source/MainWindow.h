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

#include "FluidRenderWidget.h"
#include "Controller.h"
#include "DataManager.h"
#include "DataReader.h"

#include <QtAppHelpers/OpenGLMainWindow.h>
#include <QtAppHelpers/BrowsePathWidget.h>
#include <QtAppHelpers/OpenGLWidgetTestRender.h>
#include <QtAppHelpers/DataList.h>
#include <QtAppHelpers/ClipPlaneEditor.h>

#include <QEvent>
#include <memory>

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
class MainWindow : public OpenGLMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = 0);

protected:
    virtual void instantiateOpenGLWidget();
    virtual bool processKeyPressEvent(QKeyEvent* event) override;
    virtual void showEvent(QShowEvent* ev);

public slots:
    void updateStatusCurrentFrame(int currentFrame);
    void updateStatusNumParticles();
    void updateStatusNumMeshes();
    void updateNumFrames(int numFrames);
    void updateStatusReadInfo(double readTime, size_t bytes);

    void loadDataInfo(QString dataPath);

private:
    void setupRenderWidgets();
    void setupPlayList();
    void setupDataWidgets(QLayout* dataLayout);
    void setupStatusBar();
    void connectWidgets();

    ////////////////////////////////////////////////////////////////////////////////
    FluidRenderWidget* m_RenderWidget   = nullptr;
    Controller*        m_Controller     = nullptr;
    QSlider*           m_sldFrame       = nullptr;
    QCheckBox*         m_chkExportFrame = nullptr;
    BrowsePathWidget*  m_InputPath      = nullptr;
    BrowsePathWidget*  m_OutputPath     = nullptr;

    QLabel* m_lblStatusNumParticles = nullptr;
    QLabel* m_lblStatusCurrentFrame = nullptr;
    QLabel* m_lblStatusNumFrames    = nullptr;
    QLabel* m_lblStatusReadInfo     = nullptr;

    std::unique_ptr<ClipPlaneEditor> m_ClipPlaneEditor = std::make_unique<ClipPlaneEditor>();
    std::unique_ptr<DataManager>     m_DataManager     = std::make_unique<DataManager>();
    std::unique_ptr<DataReader>      m_DataReader      = std::make_unique<DataReader>();
    std::unique_ptr<DataList>        m_DataList        = std::make_unique<DataList>(nullptr, true, true);
};