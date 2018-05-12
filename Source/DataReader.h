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
    ~DataReader();

    void setDataPath(QString dataPath, const std::shared_ptr<SimulationDataInfo>& dataInfo);
    void setParticleDataObj(const std::shared_ptr<ParticleSystemData>& particleData);
    void setMeshObj(const std::vector<std::shared_ptr<MeshObject> >& meshObj);

signals:
    void currentFrameChanged(int currentFrame);
    void particleDataChanged();
    void numParticlesChanged(int numParticles);
    void meshesChanged();
    void numMeshesChanged(int numMeshes);
    void readInfoChanged(double readTime, size_t bytes);

public slots:
    void setDelayTime(int frameTime);
    void setFrameStep(int frameStep);
    void enableRepeat(bool bRepeat);
    void enableReverse(bool bReverse);
    void pause(bool bPaused);
    void readNextFrameByTimer();
    void readNextFrame();
    void readFrame(int currentFrame);
    void resetToFirstFrame();
    void setNumFrames(int numFrames);
    void enableAnisotropyKernel(bool bAniKernel);

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


    std::unique_ptr<QTimer>             m_AutoTimer    = nullptr;
    std::shared_ptr<SimulationDataInfo> m_DataInfo     = nullptr;
    std::shared_ptr<ParticleSystemData> m_ParticleData = nullptr;

    std::vector<std::shared_ptr<MeshObject> > m_MeshObj;
    std::vector<unsigned char>                m_VReadBuffer;
    std::function<std::string(int)>           m_GenFluidPosFileName;
    std::function<std::string(int)>           m_GenFluidAniMatrixFileName;
    std::function<std::string(int)>           m_GenMeshFileName;
};