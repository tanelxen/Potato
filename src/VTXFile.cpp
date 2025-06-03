//
//  VTXFile.cpp
//  hl2mv
//
//  Created by Fedor Artemenkov on 27.05.25.
//

#include "VTXFile.h"

#define MAX_NUM_BONES_PER_VERT 3

#pragma pack(1)
struct vtxVertex_t
{
    uint8_t boneWeightIndex[MAX_NUM_BONES_PER_VERT];
    uint8_t numBones;
    uint16_t origMeshVertID;
    int8_t boneID[MAX_NUM_BONES_PER_VERT];
};

struct vtxStripHeader_t
{
    uint32_t numIndices;
    uint32_t indexOffset;
    uint32_t numVerts;
    uint32_t vertOffset;
    int16_t numBones;
    uint8_t flags;
    uint32_t numBoneStateChanges;
    uint32_t boneStateChangeOffset;
    
    inline vtxVertex_t *pVertex( int i ) const
    {
        return (vtxVertex_t *)(((uint8_t *)this) + vertOffset) + i;
    }
    
    inline uint16_t *pIndex( int i ) const
    {
        return (uint16_t *)(((uint8_t *)this) + indexOffset) + i;
    }
};

struct vtxStripGroupHeader_t
{
    uint32_t numVerts;
    uint32_t vertOffset;
    uint32_t numIndices;
    uint32_t indexOffset;
    uint32_t numStrips;
    uint32_t stripOffset;
    uint8_t flags;
    
    inline vtxVertex_t *pVertex( int i ) const
    {
        return (vtxVertex_t *)(((uint8_t *)this) + vertOffset) + i;
    }
    
    inline uint16_t *pIndex( int i ) const
    {
        return (uint16_t *)(((uint8_t *)this) + indexOffset) + i;
    }
    
    inline vtxStripHeader_t *pStrip( int i ) const
    {
        return (vtxStripHeader_t *)(((uint8_t *)this) + stripOffset) + i;
    }
};

struct vtxMeshHeader_t
{
    uint32_t numStripGroups;
    uint32_t stripGroupHeaderOffset;
    uint8_t flags;
    
    inline vtxStripGroupHeader_t *pStrigGroup( int i ) const
    {
        return ( vtxStripGroupHeader_t *)(((uint8_t *)this) + stripGroupHeaderOffset) + i;
    }
};

struct vtxModelLODHeader_t
{
    uint32_t numMeshes;
    uint32_t meshOffset;
    float switchPoint;
    
    inline vtxMeshHeader_t *pMesh( int i ) const
    {
        return ( vtxMeshHeader_t *)(((uint8_t *)this) + meshOffset) + i;
    }
};

struct vtxModelHeader_t
{
    uint32_t numLODs;
    uint32_t lodOffset;
    
    inline vtxModelLODHeader_t *pLOD( int i ) const
    {
        return ( vtxModelLODHeader_t *)(((uint8_t *)this) + lodOffset) + i;
    }
};

struct vtxBodyPartHeader_t
{
    uint32_t numModels;
    uint32_t modelOffset;
    
    inline vtxModelHeader_t *pModel( int i ) const
    {
        return (vtxModelHeader_t *)(((uint8_t *)this) + modelOffset) + i;
    }
};

struct vtxFileHeader_t
{
    uint32_t version;
    
    uint32_t vertCacheSize;
    uint16_t maxBonesPerStrip;
    uint16_t maxBonesPerTri;
    uint32_t maxBonesPerVert;
    
    uint32_t checkSum;
    
    uint32_t numLODs;
    
    // one of these for each LOD
    uint32_t materialReplacementListOffset;
    
    uint32_t numBodyParts;
    uint32_t bodyPartOffset;
    
    inline vtxBodyPartHeader_t *pBodyPart( int i ) const
    {
        return (vtxBodyPartHeader_t *)(((uint8_t *)this) + bodyPartOffset) + i;
    }
};
#pragma pack()

void VTXFile::loadFromBuffer(const void* buffer)
{
    vtxFileHeader_t* pHeader = (vtxFileHeader_t*) buffer;
    
    m_checksum = pHeader->checkSum;
    
    for (int bp = 0; bp < pHeader->numBodyParts; ++bp)
    {
        auto& bodyPart = bodyparts.emplace_back();
        
        auto pBodypart = pHeader->pBodyPart(bp);
        
        for (int sm = 0; sm < pBodypart->numModels; ++sm)
        {
            auto& subModel = bodyPart.models.emplace_back();
            
            auto pModel = pBodypart->pModel(sm);
            
            for (int l = 0; l < pModel->numLODs; ++l)
            {
                auto& subModelLOD = subModel.lods.emplace_back();
                
                auto pLOD = pModel->pLOD(l);
                
                for (int m = 0; m < pLOD->numMeshes; ++m)
                {
                    auto& mesh = subModelLOD.meshes.emplace_back();
                    
                    auto pMesh = pLOD->pMesh(m);
                    
                    for (int sg = 0; sg < pMesh->numStripGroups; ++sg)
                    {
                        auto& stripGroup = mesh.stripGroups.emplace_back();
                        
                        auto pStripGroup = pMesh->pStrigGroup(sg);
                        
                        
                        for (int v = 0; v < pStripGroup->numVerts; ++v)
                        {
                            auto& vert = stripGroup.verts.emplace_back();
                            vert.vertId = pStripGroup->pVertex(v)->origMeshVertID;
                        }
                        
                        for (int i = 0; i < pStripGroup->numIndices; ++i)
                        {
                            uint16_t index = *pStripGroup->pIndex(i);
                            stripGroup.indices.push_back(index);
                        }
                        
                        for (int s = 0; s < pStripGroup->numStrips; ++s)
                        {
                            auto pStrip = pStripGroup->pStrip(s);
                            
                            auto& strip = stripGroup.strips.emplace_back();
                            strip.numIndices = pStrip->numIndices;
                            strip.indexOffset = pStrip->indexOffset;
                            strip.flags = pStrip->flags;
                        }
                    }
                }
            }
        }
    }
}
