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

#include "DataManager.h"
#include "Common.h"

#include <Banana/Data/ParticleSystemData.h>
#include <OpenGLHelpers/MeshObjects/MeshObject.h>
#include <QtAppHelpers/AvgTimer.h>

#include <QtWidgets>

#include <vector>
#include <memory>

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
class DataReader : public QObject
{
    Q_OBJECT
public:

    DataReader();

    void setDataPath(QString dataPath, const SharedPtr<SimulationDataInfo>& dataInfo);
    void setParticleDataObj(const SharedPtr<ParticleSystemData>& particleData);
    void setMeshObj(const Vector<SharedPtr<MeshObject>>& meshObj);

signals:
    void currentFrameChanged(int currentFrame);
    void particleDataChanged();
    void numParticlesChanged(int numParticles);
    void meshesChanged();
    void numMeshesChanged(int numMeshes);
    void readInfoChanged(double readTime, size_t bytes);

public slots:
    void setNumFrames(int numFrames);
    void setDelayTime(int frameTime) { m_DelayTime = frameTime; m_AutoTimer->setInterval(m_DelayTime); }
    void setFrameStep(int frameStep) { m_FrameStep = frameStep; }
    void enableRepeat(bool bRepeat) { m_bRepeat = bRepeat; }
    void enableReverse(bool bReverse) { m_Reverse = bReverse; }
    void pause(bool bPaused) { m_bPause = bPaused; }
    void readNextFrameByTimer() { if(!m_bPause) { readNextFrame(); } }
    void readNextFrame();
    void readFrame(int currentFrame);
    void resetToFirstFrame() { readFrame(1); }
    void enableAnisotropyKernel(bool bAniKernel) { m_bUseAnisotropyKernel = bAniKernel; }

private:
    void allocateMemory();
    bool readFluidPosition(int frameID);
    bool readFluidAnisotropyKernel(int frameID);
    bool readMeshes(int frameID);

    int m_NumFrames    = 0;
    int m_CurrentFrame = 0;
    int m_DelayTime    = DEFAULT_DELAY_TIME;
    int m_FrameStep    = 1;

    bool m_bRepeat              = false;
    bool m_Reverse              = false;
    bool m_bPause               = false;
    bool m_bUseAnisotropyKernel = true;

    QString  m_DataPath = QString("");
    size_t   m_ReadBytes;
    AvgTimer m_ReadTimer;

    UniquePtr<QTimer>             m_AutoTimer    = nullptr;
    SharedPtr<SimulationDataInfo> m_DataInfo     = nullptr;
    SharedPtr<ParticleSystemData> m_ParticleData = nullptr;

    Vector<SharedPtr<MeshObject>> m_MeshObj;
    Vector<unsigned char>         m_VReadBuffer;
    std::function<String(int)>    m_GenFluidPosFileName;
    std::function<String(int)>    m_GenFluidAniMatrixFileName;
    std::function<String(int)>    m_GenMeshFileName;
};