//
//  Q3LightGrid.hpp
//  TryOpenGL
//
//  Created by Fedor Artemenkov on 25.11.24.
//

#pragma once

#include <vector>
#include <glm/glm.hpp>

struct Quake3BSP;

struct Q3LightGrid
{
    void init(const Quake3BSP& q3bsp);
    
    glm::vec3 getAmbient(const glm::vec3& point);
    
private:
    glm::vec3 maxs;
    glm::vec3 mins;
    
    int lightVolSizeX;
    int lightVolSizeY;
    int lightVolSizeZ;
    
    std::vector<glm::vec3> ambients;
    
    int indexForCell(int x, int y, int z);
};
