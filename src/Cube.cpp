//
//  Cube.cpp
//  TryOpenGL
//
//  Created by Fedor Artemenkov on 21.10.24.
//

#include <glad/glad.h>
#include "glm/gtc/matrix_transform.hpp"

#include "Cube.h"

void Cube::init()
{
    static const GLfloat vertices[] = {
        // front
        0.0,  0.0,  1.0,
        1.0,  0.0,  1.0,
        1.0,  1.0,  1.0,
        0.0,  1.0,  1.0,
        // back
        0.0,  0.0,  0.0,
        1.0,  0.0,  0.0,
        1.0,  1.0,  0.0,
        0.0,  1.0,  0.0
    };
    
    static const GLushort indices[] = {
        // front
        0, 1, 2,
        2, 3, 0,
        // right
        1, 5, 6,
        6, 2, 1,
        // back
        7, 6, 5,
        5, 4, 7,
        // left
        4, 0, 3,
        3, 7, 4,
        // bottom
        4, 5, 1,
        1, 0, 4,
        // top
        3, 2, 6,
        6, 7, 3
    };
    
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * 8, vertices, GL_STATIC_DRAW);
    
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * 3 * 12, indices, GL_STATIC_DRAW);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    shader.init("assets/shaders/cube.glsl");
    
    position = glm::vec3(0);
    scale = glm::vec3(8);
}

void Cube::draw(const Camera& camera) const
{
    glm::mat4 model(1.0f);
    model = glm::translate(model, position);
    model = glm::scale(model, scale);
    
    glm::mat4x4 mvp = camera.projection * camera.view * model;
    
    shader.bind();
    shader.setUniformMatrix((const float*) &mvp, "MVP");
    
    glm::vec4 color{0.5, 0.5, 0.5, 0.5};
    shader.setUniformVector4((const float*) &color, "color");
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, 0);
}

void WiredCube::init()
{
    static const GLfloat vertices[] = {
        // front
        0.0,  0.0,  1.0,
        1.0,  0.0,  1.0,
        1.0,  1.0,  1.0,
        0.0,  1.0,  1.0,
        // back
        0.0,  0.0,  0.0,
        1.0,  0.0,  0.0,
        1.0,  1.0,  0.0,
        0.0,  1.0,  0.0
    };
    
    static const GLushort indices[] = {
        0, 1,
        1, 2,
        2, 3,
        3, 0,
        
        4, 5,
        5, 6,
        6, 7,
        7, 4,
        
        0, 4,
        1, 5,
        2, 6,
        3, 7
    };
    
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * 8, vertices, GL_STATIC_DRAW);
    
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * 2 * 12, indices, GL_STATIC_DRAW);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    shader.init("assets/shaders/cube.glsl");
    
    position = glm::vec3(0);
    scale = glm::vec3(8);
}

void WiredCube::draw(const Camera& camera) const
{
    glm::mat4 model(1.0f);
    model = glm::translate(model, position);
    model = glm::scale(model, scale);
    
    glm::mat4x4 mvp = camera.projection * camera.view * model;
    
    shader.bind();
    shader.setUniformMatrix((const float*) &mvp, "MVP");
    
    glm::vec4 color{1, 1, 1, 1};
    shader.setUniformVector4((const float*) &color, "color");
    
    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glDrawElements(GL_LINES, 24, GL_UNSIGNED_SHORT, 0);
}
