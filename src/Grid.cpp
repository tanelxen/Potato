//
//  Grid.cpp
//  TryOpenGL
//
//  Created by Fedor Artemenkov on 20.07.24.
//

#include "Grid.h"

void Grid::init()
{
//    float vertices[] = {
//        -0.5f, -0.5f, -2.0f,
//        -0.5f,  0.5f, -2.0f,
//         0.5f,  0.5f, -2.0f,
//        
//         0.5f,  0.5f, -2.0f,
//         0.5f, -0.5f, -2.0f,
//        -0.5f, -0.5f, -2.0f
//    };
//    
    struct Vertex
    {
        glm::vec3 pos;
        glm::vec3 col;
    };
    
    static const Vertex vertices[] = {
        { { -512.0f, 0.0f, -512.0f}, { 1.f, 0.f, 0.f } },
        { { -512.0f, 0.0f,  512.0f}, { 0.f, 1.f, 0.f } },
        { {  512.0f, 0.0f,  512.0f}, { 0.f, 0.f, 1.f } },
        { {  512.0f, 0.0f,  512.0f}, { 0.f, 0.f, 1.f } },
        { {  512.0f, 0.0f, -512.0f}, { 0.f, 0.f, 0.f } },
        { { -512.0f, 0.0f, -512.0f}, { 1.f, 0.f, 0.f } },
    };
    
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * 6, vertices, GL_STATIC_DRAW);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, pos));
    
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, col));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    shader.init("assets/shaders/grid.glsl");
}

void Grid::draw(const Camera& camera) const
{
    shader.bind();
    glm::mat4x4 mvp = camera.projection * camera.view;
    shader.setUniformMatrix((const float*) &mvp, "MVP");
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}
