//
//  World.cpp
//  TryOpenGL
//
//  Created by Fedor Artemenkov on 25.10.24.
//

#include "World.h"
#include "Camera.h"
#include "Cube.h"

void World::addCube(const glm::vec3& position, const glm::vec3& scale)
{
    ShadedCube& cube = brushes.emplace_back();
    
    cube.position = position;
    cube.scale = scale;
    cube.init();
}

void World::draw(const Camera &camera) const
{
    for (const ShadedCube& brush : brushes)
    {
        brush.draw(camera);
    }
}

