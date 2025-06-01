//
//  SourceModel.h
//  hl2mv
//
//  Created by Fedor Artemenkov on 19.11.24.
//

#pragma once

#include <vector>
#include <string>
#include <span>



struct Vertex
{
    struct vec2_t {
        float s, t;
    };
    
    struct vec3_t {
        float x, y, z;
    };
    
    vec3_t position;
    vec2_t uv;
};

struct Mesh
{
    std::vector<uint32_t> indices;
    int textureIndex;
};

struct SourceMDLAsset
{
    std::string m_name;
    
//    std::vector<Sequence> sequences;
//    std::vector<int> bones;
    
    std::vector<std::string> m_textureNames;
    std::vector<Vertex> m_vertices;
    std::vector<Mesh> meshes;
    
    void loadFromFile(const std::string& filename);
    
private:
    void clearBuffers();
    
    void *mdl_buffer = nullptr;
    void *vvd_buffer = nullptr;
    void *vtx_buffer = nullptr;
};
