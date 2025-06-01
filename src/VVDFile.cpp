//
//  VVDFile.cpp
//  hl2mv
//
//  Created by Fedor Artemenkov on 27.05.25.
//

#include "VVDFile.h"
#include <cstdint>

#define MAX_NUM_BONES_PER_VERT 3
#define MAX_NUM_LODS 8

struct mstudioboneweight_t
{
    float   weight[MAX_NUM_BONES_PER_VERT];
    int8_t  bone[MAX_NUM_BONES_PER_VERT];
    uint8_t numbones;
};

struct mstudiovertex_t
{
    mstudioboneweight_t m_BoneWeights;
    float               m_vecPosition[3];
    float               m_vecNormal[3];
    float               m_vecTexCoord[2];
};

struct vertexFileFixup_t
{
    uint32_t lod;
    uint32_t sourceVertexID;
    uint32_t numVertexes;
};

struct vvdFileHeader_t
{
    uint32_t id;
    uint32_t version;
    uint32_t checksum;
    uint32_t numLODs;
    uint32_t numLODVertexes[MAX_NUM_LODS];
    uint32_t numFixups;
    uint32_t fixupTableStart;
    uint32_t vertexDataStart;
    uint32_t tangentDataStart;
    
    inline mstudiovertex_t *pVertex( int i ) const
    {
        return (mstudiovertex_t *)(((uint8_t *)this) + vertexDataStart) + i;
    }
    
    inline vertexFileFixup_t *pFixup( int i ) const
    {
        return (vertexFileFixup_t *)(((uint8_t *)this) + fixupTableStart) + i;
    }
};

void VVDFile::loadFromBuffer(const void* buffer)
{
    vvdFileHeader_t* pHeader = (vvdFileHeader_t*) buffer;
    
    m_checksum = pHeader->checksum;
    
//    const int lod = 0;
    
    lods.resize(pHeader->numLODs);
    
    if (pHeader->numFixups == 0)
    {
        lods[0].vertices.resize(pHeader->numLODVertexes[0]);
        
        for (int i = 0; i < pHeader->numLODVertexes[0]; ++i)
        {
            auto pVert = pHeader->pVertex(i);
            
            lods[0].vertices[i].position = {
                .x = pVert->m_vecPosition[0],
                .y = pVert->m_vecPosition[1],
                .z = pVert->m_vecPosition[2]
            };
            
            lods[0].vertices[i].uv = {
                .s = pVert->m_vecTexCoord[0],
                .t = pVert->m_vecTexCoord[1]
            };
        }
    }
    else
    {
        for (int f = 0; f < pHeader->numFixups; ++f)
        {
            auto fixup = pHeader->pFixup(f);
            
            for (int i = 0; i < fixup->numVertexes; ++i)
            {
                auto pVert = pHeader->pVertex(fixup->sourceVertexID + i);
                
                for (int j = fixup->lod; j >= 0; j--)
                {
                    lods[j].vertices.push_back({
                        .position = {
                            .x = pVert->m_vecPosition[0],
                            .y = pVert->m_vecPosition[1],
                            .z = pVert->m_vecPosition[2]
                        },
                        .uv = {
                            .s = pVert->m_vecTexCoord[0],
                            .t = pVert->m_vecTexCoord[1]
                        }
                    });
                }
            }
        }
    }
}
