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

#include <Banana/Setup.h>

#include <QtWidgets>
#include <QFileSystemWatcher>
#include <QList>
#include <QString>

#include <array>
#include <memory>

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
struct SimulationDataInfo
{
    void resetData()
    {
        num_particles     = 0;
        max_num_particles = 0;
        particle_radius   = 0;;
        particle_mass     = 0;

        num_meshes = 0;

        boundary_min = { 0, 0, 0 };
        boundary_max = { 0, 0, 0 };
        movable_min  = { 0, 0, 0 };
        movable_max  = { 0, 0, 0 };

        num_cells = { 0, 0, 0, 0 };

        camera_position = { 0, 0, 0 };
        camera_focus    = { 0, 0, 0 };

        hasCameraInfo = false;
        hasLightInfo  = false;
    }

    Int   num_particles;
    Int   max_num_particles;
    float particle_radius;
    float particle_mass;
    Int   num_meshes;

    Vec3f boundary_min;
    Vec3f boundary_max;
    Vec3f movable_min;
    Vec3f movable_max;

    Vec4i num_cells;

    bool  hasCameraInfo;
    bool  hasLightInfo;
    Vec3f camera_position;
    Vec3f camera_focus;
    Vec3f light_position;
};

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
class DataManager : public QObject
{
    Q_OBJECT
public:
    DataManager();

    bool setDataPath(QString dataPath);
    bool loadDataInfo(QString dataPath);

    const auto& getStringDataInfo() const { return m_StringDataInfo; }
    const auto& getDataInfo() const { return m_DataInfo; }

private slots:
    void countFrames();

signals:
    void numFramesChanged(int numFrames);

private:
    void generateStringDataInfo();

    Int                           m_NumFrames      = 0;
    QString                       m_DataPath       = QString("");
    SharedPtr<QStringList>        m_StringDataInfo = std::make_shared<QStringList>();
    SharedPtr<SimulationDataInfo> m_DataInfo       = std::make_shared<SimulationDataInfo>();
    UniquePtr<QFileSystemWatcher> m_DataDirWatcher = std::make_unique<QFileSystemWatcher>();
};