//
//  SourceModel.cpp
//  hl2mv
//
//  Created by Fedor Artemenkov on 19.11.24.
//

#include <iostream>
#include <span>

#include "SourceMDLAsset.h"
#include "MDLFile.h"
#include "VVDFile.h"
#include "VTXFile.h"

bool makeBufferFromFile(const std::string &filename, void*& buffer)
{
    if (buffer) free(buffer);
    
    FILE* fp = fopen(filename.c_str(), "rb" );
    
    if(fp == nullptr) {
        printf("unable to open %s\n", filename.c_str());
        return false;
    }
    
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    buffer = malloc(size);
    fread(buffer, size, 1, fp);
    fclose(fp);
    
    return true;
}

void SourceMDLAsset::loadFromFile(const std::string &mdl_filename)
{
    size_t ext_pos = mdl_filename.rfind(".mdl");
    
    if (ext_pos == std::string::npos)
    {
        std::cout << "Расширение не найдено в строке!" << std::endl;
        return;
    }
    
    {
        if (!makeBufferFromFile(mdl_filename, mdl_buffer)) {
            clearBuffers();
            return;
        }
    }
    
    {
        std::string vvd_filename = mdl_filename.substr(0, ext_pos) + ".vvd";
        if (!makeBufferFromFile(vvd_filename, vvd_buffer)) {
            clearBuffers();
            return;
        }
    }
    
    {
        std::string vtx_filename = mdl_filename.substr(0, ext_pos) + ".dx90.vtx";
        if (!makeBufferFromFile(vtx_filename, vtx_buffer)) {
            clearBuffers();
            return;
        }
    }
    
    MDLFile mdl;
    mdl.loadFromBuffer(mdl_buffer);
    
    VVDFile vvd;
    vvd.loadFromBuffer(vvd_buffer);
    
    VTXFile vtx;
    vtx.loadFromBuffer(vtx_buffer);
    
    m_name = mdl.m_name;
    
    if (mdl.m_checksum != vtx.m_checksum) {
        printf("Checksums don't match! (MDL <-> VTX)\n");
        clearBuffers();
        return;
    }
    
    if (mdl.m_checksum != vvd.m_checksum) {
        printf("Checksums don't match! (MDL <-> VVD)\n");
        clearBuffers();
        return;
    }
    
    for (auto& tex : mdl.m_textureNames)
    {
        auto path = mdl.m_textureDirs[0] + tex;
        m_textureNames.push_back(path);
    }
    
    const uint8_t STRIP_IS_TRILIST = 0x01;
    const uint8_t STRIP_IS_TRISTRIP = 0x02;
    
    auto& vtxMeshes = vtx.bodyparts[0].models[0].lods[0].meshes;
    auto& mdlMeshes = mdl.bodyparts[0].models[0].meshes;
    
    for (int meshIndex = 0; meshIndex < vtxMeshes.size(); ++meshIndex)
    {
        auto& meshVTX = vtxMeshes[meshIndex];
        auto& meshMDL = mdlMeshes[meshIndex];
        
        auto& newMesh = meshes.emplace_back();
        newMesh.textureIndex = meshMDL.skinrefIndex;
        
        for (auto& stripGroup : meshVTX.stripGroups)
        {
            for (auto& strip : stripGroup.strips)
            {
                if (strip.flags & STRIP_IS_TRILIST)
                {
                    for (int i = 0; i < strip.numIndices; ++i)
                    {
                        uint32_t idx = stripGroup.indices[i + strip.indexOffset];
                        uint32_t vertId = stripGroup.verts[idx].vertId;
                        newMesh.indices.push_back(vertId + meshMDL.vertexoffset);
                    }
                }
                else if (strip.flags & STRIP_IS_TRISTRIP)
                {
                    for (int i = strip.indexOffset; i < strip.numIndices + strip.indexOffset; ++i)
                    {
                        newMesh.indices.push_back(stripGroup.verts[stripGroup.indices[i + 0]].vertId + meshMDL.vertexoffset);
                        newMesh.indices.push_back(stripGroup.verts[stripGroup.indices[i + 1]].vertId + meshMDL.vertexoffset);
                        newMesh.indices.push_back(stripGroup.verts[stripGroup.indices[i + 2]].vertId + meshMDL.vertexoffset);
                    }
                }
            }
        }
    }
    
    for (auto& vert : vvd.lods[0].vertices)
    {
        m_vertices.push_back({
            .position = {vert.position.x, vert.position.y, vert.position.z},
            .normal = {vert.normal.x, vert.normal.y, vert.normal.z},
            .uv = {vert.uv.s, vert.uv.t}
        });
    }
    
    clearBuffers();
}

void SourceMDLAsset::clearBuffers()
{
    if (mdl_buffer) free(mdl_buffer);
    if (vvd_buffer) free(vvd_buffer);
    if (vtx_buffer) free(vtx_buffer);
}
