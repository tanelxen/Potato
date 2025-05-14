//
//  ValveBSPTypes.h
//  TryOpenGL
//
//  Created by Fedor Artemenkov on 10.05.25.
//

#pragma once

#include <cstdint>
#include <glm/glm.hpp>

typedef unsigned char byte;

#define HEADER_LUMPS 64

struct lump_t
{
    int32_t fileofs;    // offset into file (bytes)
    int32_t filelen;    // length of lump (bytes)
    int32_t version;    // lump format version
    uint8_t fourCC[4];  // lump ident code
};

struct dheader_t
{
    int32_t ident;      // BSP file identifier
    int32_t version;    // BSP file version
};

struct dplane_t
{
    glm::vec3   normal; // normal vector
    float       dist;   // distance from origin
    int32_t     type;   // plane axis identifier
};

struct dnode_t
{
    int             planenum;       // index into plane array
    int             children[2];    // negative numbers are -(leafs + 1), not nodes
    short           mins[3];        // for frustum culling
    short           maxs[3];
    unsigned short  firstface;      // index into face array
    unsigned short  numfaces;       // counting both sides
    short           area;           // If all leaves below this node are in the same area, then
    // this is the area index. If not, this is -1.
    short           padding;        // pad to 32 bytes length
};

struct dleaf_t
{
    int                    contents;            // OR of all brushes (not needed?)
    short                cluster;            // cluster this leaf is in
    short                area : 9;            // area this leaf is in
    short                flags : 7;            // flags
    short                mins[ 3 ];            // for frustum culling
    short                maxs[ 3 ];
    unsigned short        firstleafface;        // index into leaffaces
    unsigned short        numleaffaces;
    unsigned short        firstleafbrush;        // index into leafbrushes
    unsigned short        numleafbrushes;
    short                leafWaterDataID;    // -1 for not in water
};

struct dface_t
{
    unsigned short    planenum;        // the plane number
    char            side;            // faces opposite to the node's plane direction
    char            onNode;            // 1 of on node, 0 if in leaf
    int                firstedge;        // index into surfedges
    short            numedges;        // number of surfedges
    short            texinfo;        // texture info
    short            dispinfo;        // displacement info
    char            surfaceFogVolumeID;    // ???
    char            styles[4];        // switchable lighting info
    int                lightofs;        // offset into lightmap lump
    float            area;            // face area in units^2
    int                LightmapTextureMinsInLuxels[2];    // texture lighting info
    int                LightmapTextureSizeInLuxels[2];    // texture lighting info
    int                origFace;        // original face this was split from
    unsigned short    numPrims;        // primitives
    unsigned short    firstPrimID;    // ???
    unsigned int    smoothingGroups;// lightmap smoothing group
};

struct dtexinfo_t
{
    float    textureVecs[2][4];    // [s/t][xyz offset]
    float    lightmapVecs[2][4];    // [s/t][xyz offset] - length is in units of texels/area
    int    flags;            // miptex flags    overrides
    int    texdata;        // Pointer to texture name, size, etc.
};

struct dtexdata_t
{
    glm::vec3   reflectivity;
    int         nameStringTableID;
    int         width;
    int         height;
    int         view_width;
    int         view_height;
};

struct dedge_t
{
    uint16_t v[2];
};

struct dmodel_t
{
    glm::vec3    mins, maxs;        // bounding box
    glm::vec3    origin;            // for sounds or lights
    int32_t    headnode;        // index into node array
    int32_t    firstface, numfaces;    // index into face array
};

struct ColorRGBExp32
{
    byte r, g, b;
    signed char exponent;
};
