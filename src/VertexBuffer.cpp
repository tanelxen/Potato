//
//  VertexBuffer.cpp
//  
//
//  Created by Fedor Artemenkov on 30.06.2024.
//

#include "VertexBuffer.h"
#include <glad/glad.h>

VertexBuffer::VertexBuffer() : vbo(0), vao(0), ibo(0), count(0)
{
}

VertexBuffer::VertexBuffer(VertexBuffer&& other) noexcept
{
    vao = other.vao;
    other.vao = 0;
    
    vbo = other.vbo;
    other.vbo = 0;
    
    count = other.count;
    other.count = 0;
}

VertexBuffer& VertexBuffer::operator=(VertexBuffer&& other) noexcept
{
    vao = other.vao;
    other.vao = 0;
    
    vbo = other.vbo;
    other.vbo = 0;
    
    count = other.count;
    other.count = 0;
    
    return *this;
}

void VertexBuffer::init(const void* data, unsigned int size, const void* indices, unsigned int count, VertexLayout layout)
{
    this->count = count;

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);

    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * count, indices, GL_STATIC_DRAW);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    for(int i = 0; i < MAX_VERTEX_ATTRIBUTES; ++i)
    {
        int components = layout.attributes[i].components;
        size_t offset = layout.attributes[i].offset;

        if (components == 0) continue;

        glEnableVertexAttribArray(i);
        glVertexAttribPointer(i, components, GL_FLOAT, GL_FALSE, layout.stride, (void*) offset);
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void VertexBuffer::init(const void* data, unsigned int size, VertexLayout layout)
{
    this->count = size / layout.stride;

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    for(int i = 0; i < MAX_VERTEX_ATTRIBUTES; ++i)
    {
        int components = layout.attributes[i].components;
        size_t offset = layout.attributes[i].offset;

        if (components == 0) continue;

        glEnableVertexAttribArray(i);
        glVertexAttribPointer(i, components, GL_FLOAT, GL_FALSE, layout.stride, (void*) offset);
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void VertexBuffer::update(const void* data, unsigned int size) const
{
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void VertexBuffer::bind() const
{
    glBindVertexArray(vao);
}

void VertexBuffer::unbind() const
{
    glBindVertexArray(0);
}

void VertexBuffer::draw() const
{
    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

//    glDrawArrays(GL_TRIANGLES, 0, count);
    glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, 0);
}

VertexBuffer::~VertexBuffer()
{
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
}
