//
//  Cube.h
//  TryOpenGL
//
//  Created by Fedor Artemenkov on 21.10.24.
//

#pragma once

#include <glm/glm.hpp>
#include "Shader.h"

class Camera;

class Cube
{
    unsigned int vbo;
    unsigned int ibo;
    unsigned int vao;
    Shader shader;
    
public:
    void init();
    void draw(const Camera& camera) const;
    
    Cube() = default;
    ~Cube();
    
    Cube(Cube&& other) noexcept = default;
    Cube& operator=(Cube&& other) noexcept = default;
    
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

class ShadedCube
{
    unsigned int pbo;
    unsigned int nbo;
    unsigned int ibo;
    unsigned int vao;
    Shader shader;
    
public:
    void init();
    void draw(const Camera& camera) const;
    
    glm::vec3 position;
    glm::vec3 scale;
};
