//
//  Cube.h
//  TryOpenGL
//
//  Created by Fedor Artemenkov on 21.10.24.
//

#pragma once

#include <glm/glm.hpp>
#include "Shader.h"
#include "Camera.h"

class Cube
{
    unsigned int vbo;
    unsigned int ibo;
    unsigned int vao;
    Shader shader;
    
public:
    void init();
    void draw(const Camera& camera) const;
    
    glm::vec3 position;
    glm::vec3 scale;
};

class WiredCube
{
    unsigned int vbo;
    unsigned int ibo;
    unsigned int vao;
    Shader shader;
    
public:
    void init();
    void draw(const Camera& camera) const;
    
    glm::vec3 position;
    glm::vec3 scale;
};
