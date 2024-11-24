#pragma once

#include <vector>
#include <string>

#include "Quake3Types.h"

struct Quake3BSP
{
    bool initFromFile(const std::string& filename);
    
    std::string m_entities;
    std::vector<tBSPFace> m_faces;
    std::vector<int> m_indices;
    std::vector<tBSPVertex> m_verts;
    std::vector<tBSPTexture> m_textures;
    std::vector<tBSPLightmap> m_lightmaps;
    std::vector<tBSPNode> m_nodes;
    std::vector<tBSPLeaf> m_leafs;
    std::vector<tBSPPlane> m_planes;
    std::vector<tBSPBrush> m_brushes;
    std::vector<tBSPBrushSide> m_brushSides;
    std::vector<int> m_leafBrushes;
};
