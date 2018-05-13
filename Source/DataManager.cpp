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

#include <Banana/Utils/NumberHelpers.h>
#include <QtGui>
#include <QtWidgets>
#include <fstream>
#include <sstream>

#include "DataManager.h"
#include "Common.h"

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
DataManager::DataManager()
{
    connect(m_DataDirWatcher.get(), &QFileSystemWatcher::directoryChanged, this, &DataManager::countFrames);
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
bool DataManager::setDataPath(QString dataPath)
{
    if(!loadDataInfo(dataPath)) {
        return false;
    }

    if(m_DataPath != "") {
        m_DataDirWatcher->removePath(m_DataPath);
    }

    m_DataPath = dataPath;
    m_DataDirWatcher->addPath(m_DataPath + "/FluidFrame");

    countFrames();
    //    qDebug() << numFrames;

    return true;
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
bool DataManager::loadDataInfo(QString dataPath)
{
    QString       fileName = dataPath + "/viz_info.txt";
    std::ifstream inFile(fileName.toStdString());

    if(!inFile.is_open()) {
        QMessageBox::critical(nullptr, "Error", "Cannot read viz info from the data path.");
        return false;
    }

    m_DataInfo->resetData();

    std::string line;
    std::string paramName, paramValue;

    while(std::getline(inFile, line)) {
        line.erase(line.find_last_not_of(" \n\r\t") + 1);

        if(line == "") {
            continue;
        }

        if(line.find("//") != std::string::npos) {
            continue;
        }

        std::istringstream iss(line);
        iss >> paramName >> paramValue;

        if(paramName == "num_fluid_particles" || paramName == "num_fluid_particle") {
            m_DataInfo->num_particles = atoi(paramValue.c_str());
            continue;
        }

        if(paramName == "max_fluid_particles" || paramName == "max_fluid_particle") {
            m_DataInfo->max_num_particles = atoi(paramValue.c_str());
            continue;
        }

        if(paramName == "fluid_particle_radius") {
            m_DataInfo->particle_radius = atof(paramValue.c_str());
            continue;
        }

        if(paramName == "fluid_particle_mass") {
            m_DataInfo->particle_mass = atof(paramValue.c_str());
            continue;
        }

        if(paramName == "boundary_min_x") {
            m_DataInfo->boundary_min[0] = atof(paramValue.c_str());
            continue;
        }

        if(paramName == "boundary_min_y") {
            m_DataInfo->boundary_min[1] = atof(paramValue.c_str());
            continue;
        }

        if(paramName == "boundary_min_z") {
            m_DataInfo->boundary_min[2] = atof(paramValue.c_str());
            continue;
        }

        if(paramName == "boundary_max_x") {
            m_DataInfo->boundary_max[0] = atof(paramValue.c_str());
            continue;
        }

        if(paramName == "boundary_max_y") {
            m_DataInfo->boundary_max[1] = atof(paramValue.c_str());
            continue;
        }

        if(paramName == "boundary_max_z") {
            m_DataInfo->boundary_max[2] = atof(paramValue.c_str());
            continue;
        }

        if(paramName == "movable_min_x") {
            m_DataInfo->movable_min[0] = atof(paramValue.c_str());
            continue;
        }

        if(paramName == "movable_min_y") {
            m_DataInfo->movable_min[1] = atof(paramValue.c_str());
            continue;
        }

        if(paramName == "movable_min_z") {
            m_DataInfo->movable_min[2] = atof(paramValue.c_str());
            continue;
        }

        if(paramName == "movable_max_x") {
            m_DataInfo->movable_max[0] = atof(paramValue.c_str());
            continue;
        }

        if(paramName == "movable_max_y") {
            m_DataInfo->movable_max[1] = atof(paramValue.c_str());
            continue;
        }

        if(paramName == "movable_max_z") {
            m_DataInfo->movable_max[2] = atof(paramValue.c_str());
            continue;
        }

        if(paramName == "num_cells[0]") {
            m_DataInfo->num_cells[0] = atoi(paramValue.c_str());
            continue;
        }

        if(paramName == "num_cells[1]") {
            m_DataInfo->num_cells[1] = atoi(paramValue.c_str());
            continue;
        }

        if(paramName == "num_cells[2]") {
            m_DataInfo->num_cells[2] = atoi(paramValue.c_str());
            continue;
        }

        if(paramName == "camera_position[0]") {
            m_DataInfo->camera_position[0] = atof(paramValue.c_str());
            m_DataInfo->hasCameraInfo      = true;
            continue;
        }

        if(paramName == "camera_position[1]") {
            m_DataInfo->camera_position[1] = atof(paramValue.c_str());
            m_DataInfo->hasCameraInfo      = true;
            continue;
        }

        if(paramName == "camera_position[2]") {
            m_DataInfo->camera_position[2] = atof(paramValue.c_str());
            m_DataInfo->hasCameraInfo      = true;
            continue;
        }

        if(paramName == "camera_focus[0]") {
            m_DataInfo->camera_focus[0] = atof(paramValue.c_str());
            m_DataInfo->hasCameraInfo   = true;
            continue;
        }

        if(paramName == "camera_focus[1]") {
            m_DataInfo->camera_focus[1] = atof(paramValue.c_str());
            m_DataInfo->hasCameraInfo   = true;
            continue;
        }

        if(paramName == "camera_focus[2]") {
            m_DataInfo->camera_focus[2] = atof(paramValue.c_str());
            m_DataInfo->hasCameraInfo   = true;
            continue;
        }

        if(paramName == "light_position[0]") {
            m_DataInfo->light_position[0] = atof(paramValue.c_str());
            m_DataInfo->hasLightInfo      = true;
            continue;
        }

        if(paramName == "light_position[1]") {
            m_DataInfo->light_position[1] = atof(paramValue.c_str());
            m_DataInfo->hasLightInfo      = true;
            continue;
        }

        if(paramName == "light_position[2]") {
            m_DataInfo->light_position[2] = atof(paramValue.c_str());
            m_DataInfo->hasLightInfo      = true;
            continue;
        }
    } // end while

    inFile.close();

    // make sure the max fluid particles is alway a big number
    if(m_DataInfo->max_num_particles == 0 && m_DataInfo->num_particles != 0) {
        m_DataInfo->max_num_particles = m_DataInfo->num_particles;
    }

    m_DataInfo->num_cells[3] = m_DataInfo->num_cells[0] * m_DataInfo->num_cells[1] * m_DataInfo->num_cells[2];

    generateStringDataInfo();

    ////////////////////////////////////////////////////////////////////////////////
    return true;
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void DataManager::countFrames()
{
    QDir dataDir(m_DataPath);

    dataDir.cd("./FluidFrame");

    dataDir.setFilter(QDir::NoDotAndDotDot | QDir::AllEntries);
    dataDir.setNameFilters(QStringList() << "*.pos");

    Int numFiles = dataDir.entryList().count();

    if(m_NumFrames != numFiles) {
        m_NumFrames = numFiles;

        emit numFramesChanged(m_NumFrames);
    }
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
void DataManager::generateStringDataInfo()
{
    m_StringDataInfo->clear();

    m_StringDataInfo->append(QString("Num. particles: ") + QString::fromStdString(NumberHelpers::formatWithCommas(m_DataInfo->num_particles)));
    m_StringDataInfo->append(QString("Max. particles: " + QString::fromStdString(NumberHelpers::formatWithCommas(m_DataInfo->max_num_particles))));
    m_StringDataInfo->append(QString("Particle radius: %1").arg(m_DataInfo->particle_radius));
    m_StringDataInfo->append(QString("Particle mass: %1").arg(m_DataInfo->particle_mass));

    m_StringDataInfo->append(QString("Box min: %1, %2, %3").arg(m_DataInfo->boundary_min[0]).arg(m_DataInfo->boundary_min[1]).arg(m_DataInfo->boundary_min[2]));
    m_StringDataInfo->append(QString("Box max: %1, %2, %3").arg(m_DataInfo->boundary_max[0]).arg(m_DataInfo->boundary_max[1]).arg(m_DataInfo->boundary_max[2]));
    m_StringDataInfo->append(QString("Grid: %1, %2, %3").arg(m_DataInfo->num_cells[0]).arg(m_DataInfo->num_cells[1]).arg(m_DataInfo->num_cells[2]));
    m_StringDataInfo->append(QString("Total cells: " + QString::fromStdString(NumberHelpers::formatWithCommas(m_DataInfo->num_cells[3]))));
}
