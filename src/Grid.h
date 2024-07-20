//
//  Grid.hpp
//  TryOpenGL
//
//  Created by Fedor Artemenkov on 20.07.24.
//

#pragma once

#include <glad/glad.h>
#include "Shader.h"
#include "Camera.h"

class Grid
{
    unsigned int vbo;
    unsigned int vao;
    Shader shader;
    
public:
//    Grid();
//    ~Grid();
    
    void init();
    void draw(const Camera& camera) const;
};
