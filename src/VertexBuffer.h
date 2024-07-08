//
//  VertexBuffer.hpp
//  
//
//  Created by Fedor Artemenkov on 30.06.2024.
//

#pragma once
#include <cstddef>

#define MAX_VERTEX_ATTRIBUTES 15

struct VertexAttribute
{
    int components = 0;
    size_t offset = 0;
};

struct VertexLayout
{
    VertexAttribute attributes[MAX_VERTEX_ATTRIBUTES];
    int stride = 0;
};


class VertexBuffer
{
public:
    VertexBuffer();

    VertexBuffer(const VertexBuffer&) = delete;             // no copy allowed
    VertexBuffer& operator =(const VertexBuffer&) = delete; // no copy allowed
    
    VertexBuffer(VertexBuffer&& other) noexcept;            // move allowed
    VertexBuffer& operator=(VertexBuffer&& other) noexcept; // move allowed
    
    ~VertexBuffer();
    
public:
    void init(const void* data, unsigned int size, const void* indices, unsigned int count, VertexLayout layout);
    void init(const void* data, unsigned int size, VertexLayout layout);
    void update(const void* data, unsigned int size) const;
    
    void bind() const;
    void unbind() const;
    
    void draw() const;
    
private:
    unsigned int vbo;
    unsigned int ibo;
    unsigned int vao;
    
    unsigned int count;
};
