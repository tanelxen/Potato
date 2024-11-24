
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

bool Quake3BSP::initFromFile(const std::string& filename)
{
    FILE* fp = fopen(filename.c_str(), "rb" );
    
    if(fp == nullptr) {
        printf("unable to open %s\n", filename.c_str());
    }

    // Initialize the header and lump structures
    tBSPHeader header;
    tBSPLump lumps[kMaxLumps];

    // Read in the header and lump data
    fread(&header, 1, sizeof(tBSPHeader), fp);
    fread(&lumps, kMaxLumps, sizeof(tBSPLump), fp);
    
    int numEntities = lumps[kEntities].length;
    m_entities.resize(numEntities);
    fseek(fp, lumps[kEntities].offset, SEEK_SET);
    fread(m_entities.data(), numEntities, sizeof(char), fp);
    
    // Faces
    int numFaces = lumps[kFaces].length / sizeof(tBSPFace);
    m_faces.resize(numFaces);
    fseek(fp, lumps[kFaces].offset, SEEK_SET);
    fread(m_faces.data(), numFaces, sizeof(tBSPFace), fp);

    // Indices
    int numIndices = lumps[kIndices].length / sizeof(int);
    m_indices.resize(numIndices);
    fseek(fp, lumps[kIndices].offset, SEEK_SET);
    fread(m_indices.data(), numIndices, sizeof(int), fp);

    // Vertices
    int numVerts = lumps[kVertices].length / sizeof(tBSPVertex);
    m_verts.resize(numVerts);
    fseek(fp, lumps[kVertices].offset, SEEK_SET);
    fread(m_verts.data(), numVerts, sizeof(tBSPVertex), fp);
    
    // Textures (.shader filenames)
    int numTextures = lumps[kTextures].length / sizeof(tBSPTexture);
    m_textures.resize(numTextures);
    fseek(fp, lumps[kTextures].offset, SEEK_SET);
    fread(m_textures.data(), numTextures, sizeof(tBSPTexture), fp);

    // Lightmap
    int numLightmaps = lumps[kLightmaps].length / sizeof(tBSPLightmap);
    m_lightmaps.resize(numLightmaps);
    fseek(fp, lumps[kLightmaps].offset, SEEK_SET);
    fread(m_lightmaps.data(), numLightmaps, sizeof(tBSPLightmap), fp);

    // Nodes
    int numNodes = lumps[kNodes].length / sizeof(tBSPNode);
    m_nodes.resize(numNodes);
    fseek(fp, lumps[kNodes].offset, SEEK_SET);
    fread(m_nodes.data(), numNodes, sizeof(tBSPNode), fp);

    // Leafs
    int numLeafs = lumps[kLeafs].length / sizeof(tBSPLeaf);
    m_leafs.resize(numLeafs);
    fseek(fp, lumps[kLeafs].offset, SEEK_SET);
    fread(m_leafs.data(), numLeafs, sizeof(tBSPLeaf), fp);

    // Planes
    int numPlanes = lumps[kPlanes].length / sizeof(tBSPPlane);
    m_planes.resize(numPlanes);
    fseek(fp, lumps[kPlanes].offset, SEEK_SET);
    fread(m_planes.data(), numPlanes, sizeof(tBSPPlane), fp);
    
    // Brushes
    int numBrushes = lumps[kBrushes].length / sizeof(int);
    m_brushes.resize(numBrushes);
    fseek(fp, lumps[kBrushes].offset, SEEK_SET);
    fread(m_brushes.data(), numBrushes, sizeof(tBSPBrush), fp);
    
    // Brush sides
    int numBrushSides = lumps[kBrushSides].length / sizeof(int);
    m_brushSides.resize(numBrushSides);
    fseek(fp, lumps[kBrushSides].offset, SEEK_SET);
    fread(m_brushSides.data(), numBrushSides, sizeof(tBSPBrushSide), fp);
    
    // Leaf brushes
    int numLeafBrushes = lumps[kLeafBrushes].length / sizeof(int);
    m_leafBrushes.resize(numLeafBrushes);
    fseek(fp, lumps[kLeafBrushes].offset, SEEK_SET);
    fread(m_leafBrushes.data(), numLeafBrushes, sizeof(int), fp);

    fclose(fp);
    return (fp);
}
