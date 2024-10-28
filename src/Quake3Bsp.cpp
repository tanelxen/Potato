
#include "Quake3Bsp.h"

#include <algorithm> // std::sort
#include <cstring>   // GCC7 fix
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

// This is our lumps enumeration
enum eLumps {
    kEntities = 0, // Stores player/object positions, etc...
    kTextures,     // Stores texture information
    kPlanes,       // Stores the splitting planes
    kNodes,        // Stores the BSP nodes
    kLeafs,        // Stores the leafs of the nodes
    kLeafFaces,    // Stores the leaf's indices into the faces
    kLeafBrushes,  // Stores the leaf's indices into the brushes
    kModels,       // Stores the info of world models
    kBrushes,      // Stores the brushes info (for collision)
    kBrushSides,   // Stores the brush surfaces info
    kVertices,     // Stores the level vertices
    kIndices,      // Stores the level indices
    kShaders,      // Stores the shader files (blending, anims..)
    kFaces,        // Stores the faces for the level
    kLightmaps,    // Stores the lightmaps for the level
    kLightVolumes, // Stores extra world lighting information
    kVisData,      // Stores PVS and cluster info (visibility)
    kMaxLumps      // A constant to store the number of lumps
};

bool CQuake3BSP::initFromFile(const char* filename)
{
    if (!filename) {
        printf("ERROR:: You must specify BSP file as parameter");
        return 0;
    }

    FILE* fp = NULL;
    if ((fp = fopen(filename, "rb")) == NULL) {
        printf("ERROR:: cannot open BSP file: %s\n", filename);
        return 0;
    }

    // Initialize the header and lump structures
    tBSPHeader header = {0};
    tBSPLump   lumps[kMaxLumps] = {0};

    // Read in the header and lump data
    fread(&header, 1, sizeof(tBSPHeader), fp);
    fread(&lumps, kMaxLumps, sizeof(tBSPLump), fp);
    
    m_numEntities = lumps[kEntities].length;
    m_pEntities = new char[m_numEntities];
    fseek(fp, lumps[kEntities].offset, SEEK_SET);
    fread(m_pEntities, m_numEntities, sizeof(char), fp);
    
    // Faces
    m_numFaces = lumps[kFaces].length / sizeof(tBSPFace);
    m_pFaces = new tBSPFace[m_numFaces];
    fseek(fp, lumps[kFaces].offset, SEEK_SET);
    fread(m_pFaces, m_numFaces, sizeof(tBSPFace), fp);

    // Indices
    m_numIndices = lumps[kIndices].length / sizeof(int);
    m_pIndices = new int[m_numIndices];
    fseek(fp, lumps[kIndices].offset, SEEK_SET);
    fread(m_pIndices, m_numIndices, sizeof(int), fp);

    // Vertices
    m_numVerts = lumps[kVertices].length / sizeof(tBSPVertex);
    m_pVerts = new tBSPVertex[m_numVerts];
    fseek(fp, lumps[kVertices].offset, SEEK_SET);
    fread(m_pVerts, m_numVerts, sizeof(tBSPVertex), fp);

    // Swap from Quake to OpenGL coord system
    for (int i = 0; i < m_numVerts; i++)
    {
        float temp = m_pVerts[i].vPosition.y;
        m_pVerts[i].vPosition.y = m_pVerts[i].vPosition.z;
        m_pVerts[i].vPosition.z = -temp;
    }
    
    // Textures (.shader filenames)
    m_numTextures = lumps[kTextures].length / sizeof(tBSPTexture);
    pTextures = new tBSPTexture[m_numTextures];
    fseek(fp, lumps[kTextures].offset, SEEK_SET);
    fread(pTextures, m_numTextures, sizeof(tBSPTexture), fp);

    // Lightmap
    m_numLightmaps = lumps[kLightmaps].length / sizeof(tBSPLightmap);
    pLightmaps = new tBSPLightmap[m_numLightmaps];
    fseek(fp, lumps[kLightmaps].offset, SEEK_SET);
    fread(pLightmaps, m_numLightmaps, sizeof(tBSPLightmap), fp);

    fclose(fp);
    return (fp);
}

CQuake3BSP::CQuake3BSP()
{
    m_numEntities = 0;
    m_numVerts = 0;
    m_numFaces = 0;
    m_numIndices = 0;
    m_numTextures = 0;
    m_numLightmaps = 0;

    m_pEntities = nullptr;
    m_pVerts = nullptr;
    m_pFaces = nullptr;
    m_pIndices = nullptr;
    pTextures = nullptr;
    pLightmaps = nullptr;
}

CQuake3BSP::~CQuake3BSP()
{
    delete[] m_pEntities;
    delete[] m_pVerts;
    delete[] m_pFaces;
    delete[] m_pIndices;
    delete[] pTextures;
    delete[] pLightmaps;
}
