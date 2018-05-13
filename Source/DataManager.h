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

    int   num_particles;
    int   max_num_particles;
    float particle_radius;
    float particle_mass;
    int   num_meshes;

    std::array<float, 3> boundary_min;
    std::array<float, 3> boundary_max;
    std::array<float, 3> movable_min;
    std::array<float, 3> movable_max;

    std::array<int, 4> num_cells;

    bool                 hasCameraInfo;
    bool                 hasLightInfo;
    std::array<float, 3> camera_position;
    std::array<float, 3> camera_focus;
    std::array<float, 3> light_position;
};

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
class DataManager : public QObject
{
    Q_OBJECT
public:
    DataManager();
    ~DataManager() = default;

    bool                                       setDataPath(QString dataPath);
    bool                                       loadDataInfo(QString dataPath);
    const std::shared_ptr<QStringList>&        getStringDataInfo() const;
    const std::shared_ptr<SimulationDataInfo>& getDataInfo() const;

private slots:
    void countFrames();

signals:
    void numFramesChanged(int numFrames);

private:
    void generateStringDataInfo();

    int                                 m_NumFrames      = 0;
    QString                             m_DataPath       = QString("");
    std::shared_ptr<QStringList>        m_StringDataInfo = std::make_shared<QStringList>();
    std::shared_ptr<SimulationDataInfo> m_DataInfo       = std::make_shared<SimulationDataInfo>();
    std::unique_ptr<QFileSystemWatcher> m_DataDirWatcher = std::make_unique<QFileSystemWatcher>();
};