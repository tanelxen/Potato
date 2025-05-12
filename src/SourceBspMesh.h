//
//  SourceBspMesh.h
//  TryOpenGL
//
//  Created by Fedor Artemenkov on 11.05.25.
//

#pragma once

#include <vector>
#include <glm/glm.hpp>

struct SourceBSPAsset;

struct mvert_t
{
    glm::vec3 pos;
    glm::vec3 nrm;
    glm::vec3 clr;
    glm::vec2 uv1;
};

struct mface_t
{
    unsigned int firstVert;
    unsigned int numVerts;
};

struct SourceBspMesh
{
    void initFromBsp(SourceBSPAsset* bsp);
    void renderFaces(glm::mat4x4& mvp);
    
private:
    std::vector<mface_t> faces;
    std::vector<mvert_t> verts;
    std::vector<unsigned int> indices;
    
    unsigned int vao;
    unsigned int vbo;
    unsigned int ibo;

    unsigned int missing_id;
    
    void initBuffers();
    void generateTextures();
};
