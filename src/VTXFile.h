//
//  VTXFile.h
//  hl2mv
//
//  Created by Fedor Artemenkov on 27.05.25.
//

#pragma once

#include <vector>

struct VTXFile
{
    void loadFromBuffer(const void* buffer);
    
    struct Vertex
    {
        uint16_t vertId;
    };
    
    struct Strip
    {
        uint32_t numIndices;
        uint32_t indexOffset;
        uint8_t flags;
    };
    
    struct StripGroup
    {
        std::vector<Vertex> verts;
        std::vector<uint32_t> indices;
        std::vector<Strip> strips;
    };
    
    struct Mesh
    {
        std::vector<StripGroup> stripGroups;
    };
    
    struct LOD
    {
        std::vector<Mesh> meshes;
    };
    
    struct Model
    {
        std::vector<LOD> lods;
    };
    
    struct BodyPart
    {
        std::vector<Model> models;
    };
    
    uint32_t m_checksum;
    std::vector<BodyPart> bodyparts;
};
