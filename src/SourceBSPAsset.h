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
    struct StaticProp_t
    {
        glm::vec3 origin;
        glm::vec3 angles;
        uint16_t modelIndex;
        uint16_t leafIndex;
        uint16_t leafCount;
    };
    
    struct LeafAmbientCube_t
    {
        ColorRGBExp32 color[6];
    };
    
    bool initFromFile(const std::string& filename);
    
    std::string m_entities;
    
    std::vector<dmaterial_t> m_materials;
    
    std::vector<glm::vec3> m_verts;
    
    std::vector<dmodel_t> m_models;
    
    std::vector<dedge_t> m_edges;
    std::vector<int32_t> m_surfedges;
    
    std::vector<dface_t> m_faces;
    
    std::vector<dispinfo_t> m_displacements;
    std::vector<dispvert_t> m_disp_verts;
    
    std::vector<dtexinfo_t> m_texinfos;
    
    std::vector<dplane_t> m_planes;
    
    std::vector<ColorRGBExp32> m_lightmap;
    
    std::vector<std::string> m_staticModelDict;
    std::vector<StaticProp_t> m_staticProps;
    
    std::vector<LeafAmbientCube_t> m_leafAmbientCubes;
};
