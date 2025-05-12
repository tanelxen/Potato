//
//  HL2BspAsset.h
//  TryOpenGL
//
//  Created by Fedor Artemenkov on 10.05.25.
//

#pragma once

#include <vector>
#include <string>

#include "SourceBSPTypes.h"

struct dmaterial_t
{
    glm::vec3 reflecivity;
    std::string name;
    int width;
    int height;
};

struct SourceBSPAsset
{
    bool initFromFile(const std::string& filename);
    
    std::string m_entities;
    
    std::vector<dmaterial_t> m_materials;
    
    std::vector<glm::vec3> m_verts;
    
    std::vector<dmodel_t> m_models;
    
    std::vector<dedge_t> m_edges;
    std::vector<int32_t> m_surfedges;
    
    std::vector<dface_t> m_faces;
    
    std::vector<dtexinfo_t> m_texinfos;
    
    std::vector<dplane_t> m_planes;
};
