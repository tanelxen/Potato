//
//  MDLFile.hpp
//  hl2mv
//
//  Created by Fedor Artemenkov on 27.05.25.
//

#pragma once

#include <vector>
#include <string>

struct MDLFile
{
    void loadFromBuffer(const void* buffer);
    
    struct Mesh
    {
        uint32_t skinrefIndex;
        uint32_t vertexoffset;
    };
    
    struct Model
    {
        std::string m_name;
        std::vector<Mesh> meshes;
    };
    
    struct BodyPart
    {
        std::string m_name;
        std::vector<Model> models;
    };
    
    uint32_t m_checksum;
    std::string m_name;
    std::vector<std::string> m_textureNames;
    std::vector<std::string> m_textureDirs;
    std::vector<BodyPart> bodyparts;
};
