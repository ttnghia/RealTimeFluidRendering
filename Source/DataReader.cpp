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

#include "Common.h"
#include "DataReader.h"
#include <Banana/Utils/FileHelpers.h>
#include <fstream>

#define NEW_DATA

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
DataReader::DataReader()
{
    m_GenFluidPosFileName = [&](int fileID)
                            {
                                static char buff[512];
                                __BNN_SPRINT(buff, "%s/%s/%s.%04d.%s", m_DataPath.toStdString().c_str(), "FluidFrame", "frame", fileID, "pos");
                                return std::string(buff);
                            };
    m_GenFluidAniMatrixFileName = [&](int fileID)
                                  {
                                      static char buff[512];
                                      __BNN_SPRINT(buff, "%s/%s/%s.%04d.%s", m_DataPath.toStdString().c_str(), "FluidFrame", "frame", fileID, "ani");
                                      return std::string(buff);
                                  };
    m_GenMeshFileName = [&](int fileID)
                        {
                            static char buff[512];
                            __BNN_SPRINT(buff, "%s/%s/%s.%04d.%s", m_DataPath.toStdString().c_str(), "SolidFrame", "frame", fileID, "pos");
                            return std::string(buff);
                        };

    m_AutoTimer = std::make_unique<QTimer>(this);
    connect(m_AutoTimer.get(), SIGNAL(timeout()), this, SLOT(readNextFrameByTimer()));
    m_AutoTimer->start(m_DelayTime);
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void DataReader::setDataPath(QString dataPath, const SharedPtr<SimulationDataInfo>& dataInfo)
{
    m_DataPath     = dataPath;
    m_CurrentFrame = 0;
    if(m_DataInfo == nullptr) {
        m_DataInfo = dataInfo;
    }
    allocateMemory();
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void DataReader::setParticleDataObj(const SharedPtr<ParticleSystemData>& particleData)
{
    if(m_ParticleData == nullptr) {
        m_ParticleData = particleData;
    }
}

void DataReader::setMeshObj(const std::vector<SharedPtr<MeshObject>>& meshObj)
{
    Q_ASSERT(meshObj.size() > 0);

    if(m_MeshObj.size() == 0) {
        m_MeshObj = meshObj;
    }
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void DataReader::setNumFrames(int numFrames)
{
    m_NumFrames = numFrames;
    if(m_CurrentFrame > numFrames) {
        readFrame(numFrames);
    }
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void DataReader::readNextFrame()
{
    int nextFrame = m_Reverse ? m_CurrentFrame - m_FrameStep : m_CurrentFrame + m_FrameStep;

    if(m_bRepeat) {
        if(nextFrame <= 0) {
            nextFrame = m_NumFrames;
        } else if(nextFrame > m_NumFrames) {
            nextFrame = 1;
        }
    } else if((nextFrame <= 0) || (nextFrame > m_NumFrames)) {
        return;
    }

    readFrame(nextFrame);
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void DataReader::readFrame(int frame)
{
    if(m_ParticleData == nullptr || m_DataInfo == nullptr || frame == m_CurrentFrame) {
        return;
    }

    m_ReadBytes = 0;

    bool successReadFluid = readFluidPosition(frame);

    if(successReadFluid && m_bUseAnisotropyKernel) {
        successReadFluid = readFluidAnisotropyKernel(frame);
    }

    readMeshes(frame);

    if(successReadFluid) {
        m_CurrentFrame = frame;
        m_ParticleData->setUInt("DataFrame",     frame);
        m_ParticleData->setUInt("FrameExported", 0);

        emit particleDataChanged();
        emit currentFrameChanged(m_CurrentFrame);
        emit readInfoChanged(m_ReadTimer.getAvgTime(), m_ReadBytes);

        if(fabs(m_DataInfo->particle_radius - m_ParticleData->getParticleRadius<float>()) > 1e-6) {
            m_ParticleData->setParticleRadius(m_DataInfo->particle_radius);
        }

        if(m_DataInfo->num_particles != m_ParticleData->getNParticles()) {
            m_ParticleData->setNumParticles(m_DataInfo->num_particles);
            m_ParticleData->setUInt("ColorRandomReady", 0);
            m_ParticleData->setUInt("ColorRampReady",   0);
            //            m_ParticleData->setUInt("AnisotrpyMatrixReady", 0);

            emit numParticlesChanged(m_DataInfo->num_particles);
        }
    }
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void DataReader::allocateMemory()
{
    assert(m_ParticleData != nullptr);

    m_ParticleData->addArray<float, 3>("Position");
    m_ParticleData->addArray<float, 3>("ColorRandom");
    m_ParticleData->addArray<float, 3>("ColorRamp");
    m_ParticleData->addArray<float, 9>("AnisotropyKernelMatrix");
    m_ParticleData->reserve(1000000);
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
bool DataReader::readFluidPosition(int frameID)
{
    std::string   fileName = m_GenFluidPosFileName(frameID);
    std::ifstream file(fileName.c_str(), std::ios::binary | std::ios::ate);

    if(!file.is_open()) {
        return false;
    }

    m_ReadTimer.tick();
    size_t fileSize = (size_t)file.tellg();
    m_VReadBuffer.resize(fileSize);

    file.seekg(0, std::ios::beg);
    file.read((char*)m_VReadBuffer.data(), fileSize);
    file.close();
    m_ReadBytes += fileSize;
    m_ReadTimer.tock();

    ////////////////////////////////////////////////////////////////////////////////
    unsigned int numParticles;
    float        particleRadius;

#ifdef NEW_DATA
    memcpy(&numParticles,   m_VReadBuffer.data(),                             sizeof(unsigned int));
    memcpy(&particleRadius, &m_VReadBuffer.data()[sizeof(unsigned int)],      sizeof(float));
#else
    memcpy(&numParticles,   &m_VReadBuffer.data()[5 * sizeof(unsigned int)],  sizeof(unsigned int));
    memcpy(&particleRadius, &m_VReadBuffer.data()[11 * sizeof(unsigned int)], sizeof(float));
#endif

    if(fabs(particleRadius - m_DataInfo->particle_radius) > 1e-6) {
        m_DataInfo->particle_radius = particleRadius;
    }

    if(numParticles != m_DataInfo->num_particles) {
        m_DataInfo->num_particles = numParticles;
        m_ParticleData->allocate(numParticles);
    }

#ifdef NEW_DATA
    memcpy(m_ParticleData->getArray("Position")->data(), &m_VReadBuffer.data()[sizeof(unsigned int) + sizeof(float)], 3 * numParticles * sizeof(float));
#else
    memcpy(m_ParticleData->getArray("Position")->data(), &m_VReadBuffer.data()[18 * sizeof(unsigned int)],            3 * numParticles * sizeof(float));

    //    static Vector<String> strs(numParticles);
    //    Vec3f*                ppos = (Vec3f*)m_ParticleData->getArray("Position")->data();

    //    for(UInt i = 0; i < numParticles; ++i) {
    //        strs[i] = String("v ") + NumberHelpers::formatToScientific(ppos[i][0], 10) + String(" ") + NumberHelpers::formatToScientific(ppos[i][1], 10) + String(" ") + NumberHelpers::formatToScientific(ppos[i][2], 10);
    //    }

    //    static char buff[512];
    //    __BNN_SPRINT(buff, "%s/%s/%s.%d.%s", m_DataPath.toStdString().c_str(), "Fluid", "frame", frameID, "obj");
    //    FileHelpers::writeFile(strs, buff);
#endif
    ////////////////////////////////////////////////////////////////////////////////
    return true;
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
bool DataReader::readFluidAnisotropyKernel(int frameID)
{
    std::string   fileName = m_GenFluidAniMatrixFileName(frameID);
    std::ifstream file(fileName.c_str(), std::ios::binary | std::ios::ate);

    if(!file.is_open()) {
        return false;
    }

    m_ReadTimer.tick();
    size_t fileSize = (size_t)file.tellg();
    m_VReadBuffer.resize(fileSize);

    file.seekg(0, std::ios::beg);
    file.read((char*)m_VReadBuffer.data(), fileSize);
    file.close();
    m_ReadBytes += fileSize;
    m_ReadTimer.tock();

    unsigned int numParticles;
    memcpy(&numParticles, m_VReadBuffer.data(), sizeof(unsigned int));

    Q_ASSERT(numParticles == m_DataInfo->num_particles);

    size_t dataSize = 9 * numParticles * sizeof(float);
    Q_ASSERT(dataSize + sizeof(unsigned int) == fileSize);
    memcpy(m_ParticleData->getArray("AnisotropyKernelMatrix")->data(), &m_VReadBuffer.data()[sizeof(unsigned int)], dataSize);
    m_ParticleData->setUInt("AnisotrpyMatrixReady", 1);

    ////////////////////////////////////////////////////////////////////////////////
    return true;
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
bool DataReader::readMeshes(int frameID)
{
    std::string   fileName = m_GenMeshFileName(frameID);
    std::ifstream file(fileName.c_str(), std::ios::binary | std::ios::ate);

    if(!file.is_open()) {
        return false;
    }

    m_ReadTimer.tick();
    size_t fileSize = (size_t)file.tellg();
    m_VReadBuffer.resize(fileSize);

    file.seekg(0, std::ios::beg);
    file.read((char*)m_VReadBuffer.data(), fileSize);
    file.close();
    m_ReadBytes += fileSize;
    m_ReadTimer.tock();

    ////////////////////////////////////////////////////////////////////////////////
    unsigned int numMeshes = 0;
#ifdef NEW_DATA
    memcpy(&numMeshes, m_VReadBuffer.data(), sizeof(unsigned int));

    for(int i = numMeshes; i < m_DataInfo->num_meshes; ++i) {
        m_MeshObj[i]->clearData();
    }

    size_t offset   = sizeof(unsigned int);
    size_t dataSize = sizeof(unsigned int);

    for(unsigned int i = 0; i < numMeshes; ++i) {
        unsigned int numVertices;
        dataSize = sizeof(UInt);

        memcpy(&numVertices, &m_VReadBuffer.data()[offset], dataSize);
        offset += dataSize;

        dataSize = 3 * numVertices * sizeof(float);
        assert(dataSize * 2 + offset <= m_VReadBuffer.size());

        m_MeshObj[i]->setVertices((void*)&m_VReadBuffer.data()[offset], dataSize);
        offset += dataSize;
        m_MeshObj[i]->setVertexNormal((void*)&m_VReadBuffer.data()[offset], dataSize);
        offset += dataSize;
    }
#else
    numMeshes = 1;
    unsigned int numVertices;

    memcpy(&numVertices, &m_VReadBuffer.data()[5 * sizeof(unsigned int)], sizeof(unsigned int));

    size_t dataSize = 3 * numVertices * sizeof(float);
    m_MeshObj[0]->setVertices((void*)&m_VReadBuffer.data()[18 * sizeof(unsigned int)], dataSize);
    m_MeshObj[0]->setVertexNormal((void*)&m_VReadBuffer.data()[24 * sizeof(unsigned int) + dataSize], dataSize);
#endif

    // emit directly here
    emit meshesChanged();

    if(numMeshes != m_DataInfo->num_meshes) {
        m_DataInfo->num_meshes = numMeshes;
        emit numMeshesChanged(numMeshes);
    }

    ////////////////////////////////////////////////////////////////////////////////
    return true;
}