#pragma once

#include "Quake3Types.h"

struct Quake3BSP
{
    Quake3BSP();
    ~Quake3BSP();
    
    bool initFromFile(const char* filename);
    
    char* m_pEntities;
    int m_numEntities;

    tBSPFace* m_pFaces;
    int m_numFaces;
    
    int* m_pIndices;
    int m_numIndices;
    
    tBSPVertex* m_pVerts;
    int m_numVerts;
    
    tBSPTexture* pTextures;
    int m_numTextures;
    
    tBSPLightmap* pLightmaps;
    int m_numLightmaps;
    
    tBSPNode *m_pNodes;
    int m_numOfNodes;
    
    tBSPLeaf *m_pLeafs;
    int m_numOfLeafs;
    
    tBSPPlane *m_pPlanes;
    int m_numOfPlanes;
    
    tBSPBrush *m_pBrushes;
    int m_numOfBrushes;
    
    tBSPBrushSide *m_pBrushSides;
    int m_numOfBrushSides;
    
    int *m_pLeafBrushes;
    int m_numOfLeafBrushes;
};
