//
//  RenderableModel.cpp
//  hlmv
//
//  Created by Fedor Artemenkov on 17.11.24.
//

#include "SourceModel.h"
#include <glad/glad.h>

#define GLM_ENABLE_EXPERIMENTAL 1
#include <glm/gtx/quaternion.hpp>

#include <filesystem>

//unsigned int loadVTF(std::string filename);
unsigned int loadTexture(std::string filename);

SourceModel::~SourceModel()
{
    glDeleteTextures(m_textureIds.size(), m_textureIds.data());
    glDeleteVertexArrays(1, &m_vao);
    glDeleteBuffers(1, &m_vbo);
    glDeleteBuffers(1, &m_ibo);
}

#define VERT_POSITION_LOC 0
//#define VERT_NORMAL_LOC 1
#define VERT_DIFFUSE_TEX_COORD_LOC 1
//#define VERT_BONE_INDEX_LOC 3

void SourceModel::init(const SourceMDLAsset& model)
{
    name = model.m_name;
    
    for (auto& texture : model.m_textureNames)
    {
        std::string path = "assets/hl2/materials/" + texture + ".vtf";
        std::replace(path.begin(), path.end(), '\\', '/');
        
        std::filesystem::path fs_path(path);
        fs_path = fs_path.make_preferred();

        uint32_t id = loadTexture(fs_path.string());
        m_textureIds.push_back(id);
    }
    
    std::vector<uint32_t> indices;
    
    for (auto& mesh : model.meshes)
    {
        auto& surface = m_surfaces.emplace_back();
        surface.bufferOffset = sizeof(uint32_t) * indices.size();
        surface.indicesCount = mesh.indices.size();
        surface.textureIndex = mesh.textureIndex;
        
        indices.insert(indices.end(), mesh.indices.begin(), mesh.indices.end());
    }
    
    glGenBuffers(1, &m_ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * indices.size(), indices.data(), GL_STATIC_DRAW);
    
    glGenBuffers(1, &m_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * model.m_vertices.size(), model.m_vertices.data(), GL_STATIC_DRAW);
    
    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);
    
    glEnableVertexAttribArray(VERT_POSITION_LOC);
    glVertexAttribPointer(VERT_POSITION_LOC, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    
    glEnableVertexAttribArray(VERT_DIFFUSE_TEX_COORD_LOC);
    glVertexAttribPointer(VERT_DIFFUSE_TEX_COORD_LOC, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
}

void SourceModel::update(float dt)
{

}

void SourceModel::draw()
{
//    glEnable(GL_DEPTH_TEST);
//    glFrontFace(GL_CW);
    
//    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
    
    for (const auto& surface : m_surfaces)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, surface.textureIndex);
        
        if (surface.textureIndex < m_textureIds.size()) {
            glBindTexture(GL_TEXTURE_2D, m_textureIds[surface.textureIndex]);
        }
        
        glDrawElements(GL_TRIANGLES, surface.indicesCount, GL_UNSIGNED_INT, (void*)surface.bufferOffset);
    }
}


