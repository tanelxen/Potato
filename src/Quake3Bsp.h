#pragma once

//#include <cstring>
#include <unordered_map>
//#include <string>
#include <vector>
#include <glad/glad.h>
#include "Quake3Types.h"

#define FACE_POLYGON 1
#define MAX_TEXTURES 1000

struct Surface
{
    GLuint texId;
    uint32_t bufferOffset; // offset in bytes
    uint32_t numVerts;
};

class CQuake3BSP
{
public:
    CQuake3BSP();

    // This loads a .bsp file by it's file name (Returns true if successful)
    bool LoadBSP(const char* filename);

    void initBuffers();

    void GenerateTexture();
    void GenerateLightmap();
    void renderFaces();

private:
    int m_numFaces;
    tBSPFace* m_pFaces;
    
    int m_numIndices;
    int* m_pIndices;
    
    int m_numVerts;
    tBSPVertex* m_pVerts;
    
    int m_numTextures;
    int m_numLightmaps;
    
    GLuint m_textures[MAX_TEXTURES];
    GLuint m_lightmaps[MAX_TEXTURES];

    GLuint  missing_LM_id;
    GLuint  missing_id;

    tBSPTexture*  pTextures;
    tBSPLightmap* pLightmaps;

    GLuint vao;
    GLuint vbo;
    GLuint ibo;
    
    std::vector<Surface> surfaces;
    std::vector<unsigned int> indices;
};
