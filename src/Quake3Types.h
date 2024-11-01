//
// Created by Fedor Artemenkov on 06.07.2024.
//

#pragma once

#include <glm/glm.hpp>

typedef unsigned char byte;

// This is our BSP header structure
struct tBSPHeader {
    char strID[4]; // This should always be 'IBSP'
    int  version;  // This should be 0x2e for Quake 3 files
};

// This is our BSP lump structure
struct tBSPLump {
    int offset; // The offset into the file for the start of this lump
    int length; // The length in bytes for this lump
};

// This is our BSP vertex structure
struct tBSPVertex {
    glm::vec3 vPosition;      // (x, y, z) position.
    glm::vec2 vTextureCoord;  // (u, v) texture coordinate
    glm::vec2 vLightmapCoord; // (u, v) lightmap coordinate
    glm::vec3 vNormal;        // (x, y, z) normal vector
    byte      color[4];       // RGBA color for the vertex
};

// This is our BSP face structure
struct tBSPFace {
    int       textureID;      // The index into the texture array
    int       effect;         // The index for the effects (or -1 = n/a)
    int       type;           // 1=polygon, 2=patch, 3=mesh, 4=billboard
    int       startVertIndex; // The starting index into this face's first vertex
    int       numOfVerts;     // The number of vertices for this face
    int       startIndex;     // The starting index into the indices array for this face
    int       numOfIndices;   // The number of indices for this face
    int       lightmapID;     // The texture index for the lightmap
    int       lMapCorner[2];  // The face's lightmap corner in the image
    int       lMapSize[2];    // The size of the lightmap section
    glm::vec3 lMapPos;        // The 3D origin of lightmap.
    glm::vec3 lMapVecs[2];    // The 3D space for s and t unit vectors.
    glm::vec3 vNormal;        // The face normal.
    int       size[2];        // The bezier patch dimensions.
};

// This is our BSP texture structure
struct tBSPTexture {
    char strName[64]; // The name of the texture w/o the extension
    int  flags;       // The surface flags (unknown)
    int  contents;    // The content flags (unknown)
};

struct tBSPLightmap {
    byte imageBits[128*128*3]; // The RGB data in a 128x128 image
};

// This stores a node in the BSP tree
struct tBSPNode
{
    int plane;                    // The index into the planes array
    int child[2]; /* front, back */
    int mins[3];
    int maxs[3];
};

// This stores a leaf (end node) in the BSP tree
struct tBSPLeaf
{
    int cluster;                // The visibility cluster
    int area;                    // The area portal
    int mins[3];
    int maxs[3];
    int leafface;                // The first index into the face array
    int numOfLeafFaces;            // The number of faces for this leaf
    int leafBrush;                // The first index for into the brushes
    int numOfLeafBrushes;        // The number of brushes for this leaf
};

// This stores a splitter plane in the BSP tree
struct tBSPPlane
{
    glm::vec3 normal;            // Plane normal.
    float distance;             // The plane distance from origin
};

// This stores the brush data
struct tBSPBrush
{
    int brushSide;                // The starting brush side for the brush
    int numOfBrushSides;        // Number of brush sides for the brush
    int textureID;                // The texture index for the brush
};

// This stores the brush side data, which stores indices for the normal and texture ID
struct tBSPBrushSide
{
    int plane;                    // The plane index
    int textureID;                // The texture index
};
