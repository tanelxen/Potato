//
//  VVDFile.h
//  hl2mv
//
//  Created by Fedor Artemenkov on 27.05.25.
//

#pragma once

#include <vector>
#include <cstdint>

struct VVDFile
{
    void loadFromBuffer(const void* buffer);
    
    struct vec3_t {
        float x, y, z;
    };
    
    struct vec2_t {
        float s, t;
    };
    
    struct Vertex
    {
        vec3_t position;
        vec3_t normal;
        vec2_t uv;
    };
    
    struct LOD
    {
        std::vector<Vertex> vertices;
    };
    
    std::vector<LOD> lods;
    
    uint32_t m_checksum;
};
