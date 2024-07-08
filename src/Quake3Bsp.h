#pragma once

#include <cstring>
#include <map>
#include <string>
#include <vector>
#include <glad/glad.h>
#include "Quake3Types.h"

#define FACE_POLYGON 1
#define MAX_TEXTURES 1000

struct FaceBuffers {
    GLuint VAO;
    GLuint VBO;
    GLuint EBO;
};

struct FaceBuffArray {
    std::map<int, FaceBuffers> FB_Idx;
};

struct RenderBuffers
{
    std::map<int, std::vector<GLfloat>> faceVerts;
    std::map<int, std::vector<GLuint>>  faceIndices;
    std::map<int, std::string>          texvec;
    std::vector<tBSPLightmap>           lightMaps;

    std::map<GLuint, GLuint> tx_ID; // optimized texture IDs
    std::map<GLuint, GLuint> lm_ID; // optimized lightmap IDs
};

class CQuake3BSP
{
public:
    CQuake3BSP();

    // This loads a .bsp file by it's file name (Returns true if successful)
    bool LoadBSP(const char* filename);

    void BuildVBO();
    void GenerateTexture();
    void GenerateLightmap();
    void renderFaces();

private:
    int m_numOfVerts; // The number of verts in the model

    int count;
    int indcount;
    int tcoordcount;

    char   tname[MAX_TEXTURES][128];
    int    textureID; // The index into the texture array
    GLuint texture[MAX_TEXTURES];


    void RenderSingleFace(int index);

    void CreateVBO(int m_numOfFaces);
    void CreateRenderBuffers(int index);
    void CreateIndices(int index);

    int m_numOfFaces;    // The number of faces in the model
    int m_numOfIndices;  // The number of indices for the model
    int m_numOfTextures; // The number of texture maps
    int m_numOfLightmaps;
    int m_textures[MAX_TEXTURES];  // The texture and lightmap array for the level
    int m_lightmaps[MAX_TEXTURES]; // The lightmap texture array
    int numVisibleFaces;
    int skipindices;

    GLuint* m_lightmap_gen_IDs;
    GLuint* m_Textures;
    GLuint  missing_LM_id;
    GLuint  missing_id;

    int*        m_pIndices; // The object's indices for rendering
    tBSPVertex* m_pVerts;   // The object's vertices
    tBSPFace*   m_pFaces;   // The faces information of the object

    FaceBuffArray FB_array;
    RenderBuffers Rbuffers;
    tBSPTexture*  pTextures;
    tBSPLightmap* pLightmaps;

    GLuint vao;
    GLuint vbo;
};
