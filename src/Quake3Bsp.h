#pragma once

#include "Quake3Types.h"

struct CQuake3BSP
{
    CQuake3BSP();
    ~CQuake3BSP();
    
    bool initFromFile(const char* filename);
    
    char* m_pEntities;
    int m_numEntities;

    tBSPFace* m_pFaces;
    int m_numFaces;
    
    int* m_pIndices;
    int m_numIndices;
    
    tBSPVertex* m_pVerts;
    int m_numVerts;
    
    tBSPTexture*  pTextures;
    int m_numTextures;
    
    tBSPLightmap* pLightmaps;
    int m_numLightmaps;
};
