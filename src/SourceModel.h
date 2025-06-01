//
//  RenderableModel.hpp
//  hlmv
//
//  Created by Fedor Artemenkov on 17.11.24.
//

#pragma once

#include <vector>
#include <glm/glm.hpp>

#include "SourceMDLAsset.h"

struct SourceModel
{
    ~SourceModel();
    
    void init(const SourceMDLAsset& model);
    void update(float dt);
    void draw();
    
    std::string name;
    
private:
    unsigned int m_vbo;
    unsigned int m_ibo;
    unsigned int m_vao;
    std::vector<unsigned int> m_textureIds;
    
    struct RenderableSurface
    {
        int textureIndex;
        int bufferOffset;
        int indicesCount;
    };
    
    std::vector<RenderableSurface> m_surfaces;
};
