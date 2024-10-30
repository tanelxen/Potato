//
//  BspScene.hpp
//  TryOpenGL
//
//  Created by Fedor Artemenkov on 27.10.24.
//

#pragma once

#include <vector>
#include <glm/glm.hpp>

#define MAX_TEXTURES 1000

struct Surface
{
    unsigned int texId;
    unsigned int bufferOffset; // offset in bytes
    unsigned int numVerts;
};

class Quake3BSP;

class Q3BspMesh
{
public:
    void initFromBsp(Quake3BSP* bsp);
    void renderFaces(glm::mat4x4& mvp);
    
private:
    void initBuffers();
    void GenerateTexture();
    
    unsigned int vao;
    unsigned int vbo;
    unsigned int ibo;
    
    unsigned int m_textures[MAX_TEXTURES];
    unsigned int m_lightmap;

    unsigned int missing_id;
    
    std::vector<Surface> surfaces;
    std::vector<unsigned int> indices;
};
