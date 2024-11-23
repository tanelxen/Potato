
#include <stdio.h>
#include "Quake3Bsp.h"

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

bool Quake3BSP::initFromFile(const char* filename)
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

//    // Swap from Quake to OpenGL coord system
//    for (int i = 0; i < m_numVerts; i++)
//    {
//        float temp = m_pVerts[i].vPosition.y;
//        m_pVerts[i].vPosition.y = m_pVerts[i].vPosition.z;
//        m_pVerts[i].vPosition.z = -temp;
//    }
    
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
    
    // Nodes
    m_numOfNodes = lumps[kNodes].length / sizeof(tBSPNode);
    m_pNodes     = new tBSPNode [m_numOfNodes];
    fseek(fp, lumps[kNodes].offset, SEEK_SET);
    fread(m_pNodes, m_numOfNodes, sizeof(tBSPNode), fp);

    // Leafs
    m_numOfLeafs = lumps[kLeafs].length / sizeof(tBSPLeaf);
    m_pLeafs     = new tBSPLeaf [m_numOfLeafs];
    fseek(fp, lumps[kLeafs].offset, SEEK_SET);
    fread(m_pLeafs, m_numOfLeafs, sizeof(tBSPLeaf), fp);
    
//    // Now we need to go through and convert all the leaf bounding boxes
//    // to the normal OpenGL Y up axis.
//    for(int i = 0; i < m_numOfLeafs; i++)
//    {
//        // Swap the min y and z values, then negate the new Z
//        int temp = m_pLeafs[i].mins.y;
//        m_pLeafs[i].mins[2] = m_pLeafs[i].mins[3];
//        m_pLeafs[i].mins[3] = -temp;
//        
//        // Swap the max y and z values, then negate the new Z
//        temp = m_pLeafs[i].maxs.y;
//        m_pLeafs[i].maxs[2] = m_pLeafs[i].maxs[3];
//        m_pLeafs[i].maxs[3] = -temp;
//    }
    
    // Planes
    m_numOfPlanes = lumps[kPlanes].length / sizeof(tBSPPlane);
    m_pPlanes     = new tBSPPlane [m_numOfPlanes];
    fseek(fp, lumps[kPlanes].offset, SEEK_SET);
    fread(m_pPlanes, m_numOfPlanes, sizeof(tBSPPlane), fp);
    
//    // Go through every plane and convert it's normal to the Y-axis being up
//    for(int i = 0; i < m_numOfPlanes; i++)
//    {
//        float temp = m_pPlanes[i].normal.y;
//        m_pPlanes[i].normal.y = m_pPlanes[i].normal.z;
//        m_pPlanes[i].normal.z = -temp;
//    }
    
    // Brushes
    m_numOfBrushes = lumps[kBrushes].length / sizeof(int);
    m_pBrushes     = new tBSPBrush [m_numOfBrushes];
    fseek(fp, lumps[kBrushes].offset, SEEK_SET);
    fread(m_pBrushes, m_numOfBrushes, sizeof(tBSPBrush), fp);
    
    // Brush sides
    m_numOfBrushSides = lumps[kBrushSides].length / sizeof(int);
    m_pBrushSides     = new tBSPBrushSide [m_numOfBrushSides];
    fseek(fp, lumps[kBrushSides].offset, SEEK_SET);
    fread(m_pBrushSides, m_numOfBrushSides, sizeof(tBSPBrushSide), fp);
    
    // Leaf brushes
    m_numOfLeafBrushes = lumps[kLeafBrushes].length / sizeof(int);
    m_pLeafBrushes     = new int [m_numOfLeafBrushes];
    fseek(fp, lumps[kLeafBrushes].offset, SEEK_SET);
    fread(m_pLeafBrushes, m_numOfLeafBrushes, sizeof(int), fp);

    fclose(fp);
    return (fp);
}

Quake3BSP::Quake3BSP()
{
    m_numEntities = 0;
    m_numVerts = 0;
    m_numFaces = 0;
    m_numIndices = 0;
    m_numTextures = 0;
    m_numLightmaps = 0;
    m_numOfNodes = 0;
    m_numOfLeafs = 0;
    m_numOfPlanes = 0;
    m_numOfBrushes = 0;
    m_numOfBrushSides = 0;
    m_numOfLeafBrushes = 0;

    m_pEntities = nullptr;
    m_pVerts = nullptr;
    m_pFaces = nullptr;
    m_pIndices = nullptr;
    pTextures = nullptr;
    pLightmaps = nullptr;
    m_pNodes = nullptr;
    m_pLeafs = nullptr;
    m_pPlanes = nullptr;
    m_pBrushes = nullptr;
    m_pBrushSides = nullptr;
    m_pLeafBrushes = nullptr;
}

Quake3BSP::~Quake3BSP()
{
    delete[] m_pEntities;
    delete[] m_pVerts;
    delete[] m_pFaces;
    delete[] m_pIndices;
    delete[] pTextures;
    delete[] pLightmaps;
    delete[] m_pNodes;
    delete[] m_pLeafs;
    delete[] m_pPlanes;
    delete[] m_pBrushes;
    delete[] m_pBrushSides;
    delete[] m_pLeafBrushes;
}
