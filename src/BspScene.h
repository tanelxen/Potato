//
//  BspScene.hpp
//  TryOpenGL
//
//  Created by Fedor Artemenkov on 27.10.24.
//

#pragma once

#include <vector>

#define MAX_TEXTURES 1000

struct Surface
{
    unsigned int texId;
    unsigned int bufferOffset; // offset in bytes
    unsigned int numVerts;
};

class Camera;
class CQuake3BSP;

class BspScene
{
public:
    void initFromBsp(CQuake3BSP* bsp);
    void renderFaces(Camera* camera);
    
private:
    void initBuffers();
    void GenerateTexture();
    void GenerateLightmap();
    
    unsigned int vao;
    unsigned int vbo;
    unsigned int ibo;
    
    unsigned int m_textures[MAX_TEXTURES];
    unsigned int m_lightmaps[MAX_TEXTURES];

    unsigned int missing_LM_id;
    unsigned int missing_id;
    
    std::vector<Surface> surfaces;
    std::vector<unsigned int> indices;
};
