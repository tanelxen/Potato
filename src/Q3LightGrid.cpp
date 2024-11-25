//
//  Q3LightGrid.cpp
//  TryOpenGL
//
//  Created by Fedor Artemenkov on 25.11.24.
//

#include <algorithm>

#include "Q3LightGrid.h"
#include "Quake3Bsp.h"

using std::min, std::max;

void Q3LightGrid::init(const Quake3BSP& q3bsp)
{
    maxs = q3bsp.m_models[0].max;
    mins = q3bsp.m_models[0].min;
    
    lightVolSizeX = floor(maxs.x / 64) - ceil(mins.x / 64) + 1;
    lightVolSizeY = floor(maxs.y / 64) - ceil(mins.y / 64) + 1;
    lightVolSizeZ = floor(maxs.z / 128) - ceil(mins.z / 128) + 1;
    
    ambients.resize(q3bsp.m_lightVolumes.size());
    
    for (int i = 0; i < q3bsp.m_lightVolumes.size(); ++i)
    {
        ambients[i].r = float(q3bsp.m_lightVolumes[i].ambient[0]) / 255.0;
        ambients[i].g = float(q3bsp.m_lightVolumes[i].ambient[1]) / 255.0;
        ambients[i].b = float(q3bsp.m_lightVolumes[i].ambient[2]) / 255.0;
    }
}

glm::vec3 Q3LightGrid::getAmbient(const glm::vec3& pos)
{
    if (ambients.empty()) return glm::vec3{0};
    
    int cellX = floor(pos.x / 64) - ceil(mins.x / 64) + 1;
    int cellY = floor(pos.y / 64) - ceil(mins.y / 64) + 1;
    int cellZ = floor(pos.z / 128) - ceil(mins.z / 128) + 1;
    
    int index = indexForCell(cellX, cellY, cellZ);

    if (index >= ambients.size()) return glm::vec3{0};
    
    auto result = ambients[index];

    return result;
}

int Q3LightGrid::indexForCell(int x, int y, int z)
{
    int cellX = min(max(x, 0), lightVolSizeX);
    int cellY = min(max(y, 0), lightVolSizeY);
    int cellZ = min(max(z, 0), lightVolSizeZ);
    
    return cellX + cellY * lightVolSizeX + cellZ * lightVolSizeX * lightVolSizeY;
}
