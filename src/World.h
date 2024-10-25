//
//  World.h
//  TryOpenGL
//
//  Created by Fedor Artemenkov on 25.10.24.
//

#pragma once

#include <glm/glm.hpp>
#include <vector>

class ShadedCube;
class Camera;

class World
{
private:
    std::vector<ShadedCube> brushes;
    
public:
    void addCube(const glm::vec3& position, const glm::vec3& scale);
    void draw(const Camera& camera) const;
};
