//
//  World.h
//  TryOpenGL
//
//  Created by Fedor Artemenkov on 25.10.24.
//

#pragma once

#include <glm/glm.hpp>
#include <vector>

class Camera;

class World
{
public:
    void addCube(const glm::vec3& position, const glm::vec3& scale);
    void draw(const Camera& camera) const;
    
private:
    struct Brush
    {
        glm::vec3 position;
        glm::vec3 scale;
    };
    
    std::vector<Brush> brushes;
};
