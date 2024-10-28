//
//  World.cpp
//  TryOpenGL
//
//  Created by Fedor Artemenkov on 25.10.24.
//

#include "World.h"
#include "Camera.h"
#include "CubeGeometry.h"

void World::addCube(const glm::vec3& position, const glm::vec3& scale)
{
    Brush& cube = brushes.emplace_back();
    
    cube.position = position;
    cube.scale = scale;
}

void World::draw(const Camera &camera) const
{
    for (const Brush& brush : brushes)
    {
        CubeGeometry::draw(camera, brush.position, brush.scale);
    }
}
