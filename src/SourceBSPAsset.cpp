//
//  HL2BspAsset.cpp
//  TryOpenGL
//
//  Created by Fedor Artemenkov on 10.05.25.
//

#include "SourceBSPAsset.h"

#include <iostream>
#include <fstream>

enum LumpTypes
{
    LUMP_ENTITIES = 0,
    LUMP_PLANES = 1,
    LUMP_TEXDATA = 2,
    LUMP_VERTEXES = 3,
    LUMP_VISIBILITY = 4,
    LUMP_NODES = 5,
    LUMP_TEXINFO = 6,
    LUMP_FACES = 7,
    LUMP_LIGHTING = 8,
    LUMP_OCCLUSION = 9,
    LUMP_LEAFS = 10,
    LUMP_FACEIDS = 11,
    LUMP_EDGES = 12,
    LUMP_SURFEDGES = 13,
    LUMP_MODELS = 14,
    LUMP_WORLDLIGHTS = 15,
    LUMP_LEAFFACES = 16,
    LUMP_LEAFBRUSHES = 17,
    LUMP_BRUSHES = 18,
    LUMP_BRUSHSIDES = 19,
    LUMP_AREAS = 20,
    LUMP_AREAPORTALS = 21,
    LUMP_UNUSED0 = 22,
    LUMP_UNUSED1 = 23,
    LUMP_UNUSED2 = 24,
    LUMP_UNUSED3 = 25,
    LUMP_DISPINFO = 26,
    LUMP_ORIGINALFACES = 27,
    LUMP_PHYSDISP = 28,
    LUMP_PHYSCOLLIDE = 29,
    LUMP_VERTNORMALS = 30,
    LUMP_VERTNORMALINDICES = 31,
    LUMP_DISP_LIGHTMAP_ALPHAS = 32,
    LUMP_DISP_VERTS = 33,
    LUMP_DISP_LIGHTMAP_SAMPLE_POSITIONS = 34,
    LUMP_GAME_LUMP = 35,
    LUMP_LEAFWATERDATA = 36,
    LUMP_PRIMITIVES = 37,
    LUMP_PRIMVERTS = 38,
    LUMP_PRIMINDICES = 39,
    LUMP_PAKFILE = 40,
    LUMP_CLIPPORTALVERTS = 41,
    LUMP_CUBEMAPS = 42,
    LUMP_TEXDATA_STRING_DATA = 43,
    LUMP_TEXDATA_STRING_TABLE = 44,
    LUMP_OVERLAYS = 45,
    LUMP_LEAFMINDISTTOWATER = 46,
    LUMP_FACE_MACRO_TEXTURE_INFO = 47,
    LUMP_DISP_TRIS = 48,
    LUMP_PHYSCOLLIDESURFACE = 49,
    LUMP_WATEROVERLAYS = 50,
    LUMP_LEAF_AMBIENT_INDEX_HDR = 51,
    LUMP_LEAF_AMBIENT_INDEX = 52,
    LUMP_LIGHTING_HDR = 53,
    LUMP_WORLDLIGHTS_HDR = 54,
    LUMP_LEAF_AMBIENT_LIGHTING_HDR = 55,
    LUMP_LEAF_AMBIENT_LIGHTING = 56,
    LUMP_XZIPPAKFILE = 57,
    LUMP_FACES_HDR = 58,
    LUMP_MAP_FLAGS = 59,
    LUMP_OVERLAY_FADES = 60
};

#define MAKE_MAGIC(a, b, c, d) ((int)(a) | ((int)(b) << 8) | ((int)(c) << 16) | ((int)(d) << 24))
#define PRPS_MAGIC MAKE_MAGIC('p', 'r', 'p', 's')
#define PRPD_MAGIC MAKE_MAGIC('p', 'r', 'p', 'd')
#define VBSP_MAGIC MAKE_MAGIC('V', 'B', 'S', 'P')

std::string extractBaseTexture(const std::string& materialName);

bool SourceBSPAsset::initFromFile(const std::string &filename)
{
    FILE* fp = fopen(filename.c_str(), "rb" );
    
    if(fp == nullptr) {
        printf("unable to open %s\n", filename.c_str());
        return false;
    }
    
    dheader_t header;
    fread(&header, 1, sizeof(dheader_t), fp);
    
    if (header.ident != VBSP_MAGIC) {
        printf("%s isn't VBSP\n", filename.c_str());
        return false;
    }
    
    if (!(header.version == 19 || header.version == 20)) {
        printf("%s isn't 19 or 20 version\n", filename.c_str());
        return false;
    }
    
    lump_t lumps[64];
    fread(&lumps, 64, sizeof(lump_t), fp);
    
    int numEntities = lumps[LUMP_ENTITIES].filelen;
    m_entities.resize(numEntities);
    fseek(fp, lumps[LUMP_ENTITIES].fileofs, SEEK_SET);
    fread(m_entities.data(), numEntities, sizeof(char), fp);
    
    
    // ========================== READ MATERIALS ===========================
    int numTexdatas = lumps[LUMP_TEXDATA].filelen / sizeof(dtexdata_t);
    int stringTableSize = lumps[LUMP_TEXDATA_STRING_TABLE].filelen / sizeof(int);
    int stringDataSize = lumps[LUMP_TEXDATA_STRING_DATA].filelen;
    
    std::vector<dtexdata_t> texdatas;
    texdatas.resize(numTexdatas);
    fseek(fp, lumps[LUMP_TEXDATA].fileofs, SEEK_SET);
    fread(texdatas.data(), numTexdatas, sizeof(dtexdata_t), fp);
    
    std::vector<unsigned int> stringTable;
    stringTable.resize(stringTableSize);
    fseek(fp, lumps[LUMP_TEXDATA_STRING_TABLE].fileofs, SEEK_SET);
    fread(stringTable.data(), stringTableSize, sizeof(unsigned int), fp);
    
    const char* stringData = new char[stringDataSize];
    fseek(fp, lumps[LUMP_TEXDATA_STRING_DATA].fileofs, SEEK_SET);
    fread((void*)stringData, stringDataSize, sizeof(char), fp);
    
    for(auto& tex : texdatas)
    {
        auto& material = m_materials.emplace_back();
        
        material.width = tex.width;
        material.height = tex.height;
        material.name = stringData + stringTable[tex.nameStringTableID];
        material.reflecivity = tex.reflectivity;
        
        for(auto& c : material.name)
        {
            c = tolower(c);
        }
        
        material.name = extractBaseTexture(material.name);
    }
    
    delete [] stringData;
    // ======================================================================
    
    int numPlanes = lumps[LUMP_PLANES].filelen / sizeof(dplane_t);
    m_planes.resize(numPlanes);
    fseek(fp, lumps[LUMP_PLANES].fileofs, SEEK_SET);
    fread(m_planes.data(), numPlanes, sizeof(dplane_t), fp);
    
    {
        int count = lumps[LUMP_NODES].filelen / sizeof(dnode_t);
        m_nodes.resize(count);
        fseek(fp, lumps[LUMP_NODES].fileofs, SEEK_SET);
        fread(m_nodes.data(), count, sizeof(dnode_t), fp);
    }
    
    int numModels = lumps[LUMP_MODELS].filelen / sizeof(dmodel_t);
    m_models.resize(numModels);
    fseek(fp, lumps[LUMP_MODELS].fileofs, SEEK_SET);
    fread(m_models.data(), numModels, sizeof(dmodel_t), fp);
    
    int numVerts = lumps[LUMP_VERTEXES].filelen / sizeof(glm::vec3);
    m_verts.resize(numVerts);
    fseek(fp, lumps[LUMP_VERTEXES].fileofs, SEEK_SET);
    fread(m_verts.data(), numVerts, sizeof(glm::vec3), fp);
    
    int numEdges = lumps[LUMP_EDGES].filelen / sizeof(dedge_t);
    m_edges.resize(numEdges);
    fseek(fp, lumps[LUMP_EDGES].fileofs, SEEK_SET);
    fread(m_edges.data(), numEdges, sizeof(dedge_t), fp);
    
    int numSurfedges = lumps[LUMP_SURFEDGES].filelen / sizeof(int32_t);
    m_surfedges.resize(numSurfedges);
    fseek(fp, lumps[LUMP_SURFEDGES].fileofs, SEEK_SET);
    fread(m_surfedges.data(), numSurfedges, sizeof(int32_t), fp);
    
    int numTexinfos = lumps[LUMP_TEXINFO].filelen / sizeof(dtexinfo_t);
    m_texinfos.resize(numTexinfos);
    fseek(fp, lumps[LUMP_TEXINFO].fileofs, SEEK_SET);
    fread(m_texinfos.data(), numTexinfos, sizeof(dtexinfo_t), fp);
    
    int numFaces = lumps[LUMP_FACES].filelen / sizeof(dface_t);
    m_faces.resize(numFaces);
    fseek(fp, lumps[LUMP_FACES].fileofs, SEEK_SET);
    fread(m_faces.data(), numFaces, sizeof(dface_t), fp);
    
    int numLightmapSamples = lumps[LUMP_LIGHTING].filelen / sizeof(ColorRGBExp32);
    m_lightmap.resize(numLightmapSamples);
    fseek(fp, lumps[LUMP_LIGHTING].fileofs, SEEK_SET);
    fread(m_lightmap.data(), numLightmapSamples, sizeof(ColorRGBExp32), fp);
    
    if (lumps[LUMP_LEAFS].version == 0)
    {
        int count = lumps[LUMP_LEAFS].filelen / sizeof(dleaf_t_19);
        
        std::vector<dleaf_t_19> old_leafs(count);
        
        fseek(fp, lumps[LUMP_LEAFS].fileofs, SEEK_SET);
        fread(old_leafs.data(), count, sizeof(dleaf_t_19), fp);
        
        m_leafAmbientCubes.resize(count);
        
        for (int i = 0; i < count; ++i)
        {
            m_leafAmbientCubes[i].color[0] = old_leafs[i].ambientLighting[0];
            m_leafAmbientCubes[i].color[1] = old_leafs[i].ambientLighting[1];
            m_leafAmbientCubes[i].color[2] = old_leafs[i].ambientLighting[2];
            m_leafAmbientCubes[i].color[3] = old_leafs[i].ambientLighting[3];
            m_leafAmbientCubes[i].color[4] = old_leafs[i].ambientLighting[4];
            m_leafAmbientCubes[i].color[5] = old_leafs[i].ambientLighting[5];
            
            m_leafAmbientCubes[i].pos.x = (old_leafs[i].mins[0] + old_leafs[i].maxs[0]) * 0.5;
            m_leafAmbientCubes[i].pos.y = (old_leafs[i].mins[1] + old_leafs[i].maxs[1]) * 0.5;
            m_leafAmbientCubes[i].pos.z = (old_leafs[i].mins[2] + old_leafs[i].maxs[2]) * 0.5;
        }
    }
    
//    {
//        int pakLength = lumps[LUMP_PAKFILE].filelen;
//        std::vector<char> pakData;
//        pakData.resize(pakLength);
//        fseek(fp, lumps[LUMP_PAKFILE].fileofs, SEEK_SET);
//        fread(pakData.data(), pakLength, sizeof(char), fp);
//        
//        std::ofstream file("myfile.zip", std::ios::binary);
//        file.write(pakData.data(), pakLength);
//    }
    
    {
        int count = lumps[LUMP_DISPINFO].filelen / sizeof(dispinfo_t);
        m_displacements.resize(count);
        fseek(fp, lumps[LUMP_DISPINFO].fileofs, SEEK_SET);
        fread(m_displacements.data(), count, sizeof(dispinfo_t), fp);
    }
    
    {
        int count = lumps[LUMP_DISP_VERTS].filelen / sizeof(dispvert_t);
        m_disp_verts.resize(count);
        fseek(fp, lumps[LUMP_DISP_VERTS].fileofs, SEEK_SET);
        fread(m_disp_verts.data(), count, sizeof(dispvert_t), fp);
    }
    
    {
        uint32_t lumpCount;
        fseek(fp, lumps[LUMP_GAME_LUMP].fileofs, SEEK_SET);
        fread(&lumpCount, 1, sizeof(uint32_t), fp);
        
        std::vector<dgamelump_t> gamelumps(lumpCount);
        fread(gamelumps.data(), lumpCount, sizeof(dgamelump_t), fp);
        
        for (auto& lump : gamelumps)
        {
            if (lump.id == PRPS_MAGIC)
            {
                uint32_t staticModelDictCount = 0;
                
                fseek(fp, lump.fileofs, SEEK_SET);
                fread(&staticModelDictCount, 1, sizeof(uint32_t), fp);
                
                m_staticModelDict.resize(staticModelDictCount);
                
                for (int i = 0; i < staticModelDictCount; ++i)
                {
                    char str[128];
                    fread(str, 1, sizeof(str), fp);
                    m_staticModelDict[i] = str;
                }
                
                uint32_t leafListCount = 0;
                fread(&leafListCount, 1, sizeof(uint32_t), fp);
                
                long offset = ftell(fp);
                fseek(fp, offset + leafListCount * sizeof(uint16_t), SEEK_SET);
                
                uint32_t staticObjectCount = 0;
                fread(&staticObjectCount, 1, sizeof(uint32_t), fp);
                
                m_staticProps.resize(staticObjectCount);
                
                offset = ftell(fp);
                
                for (int i = 0; i < staticObjectCount; ++i)
                {
                    auto& prop = m_staticProps[i];
                    fseek(fp, offset, SEEK_SET);
                    
                    fread(&prop.origin, 1, sizeof(glm::vec3), fp);
                    fread(&prop.angles, 1, sizeof(glm::vec3), fp);
                    fread(&prop.modelIndex, 1, sizeof(uint16_t), fp);
                    fread(&prop.leafIndex, 1, sizeof(uint16_t), fp);
                    fread(&prop.leafCount, 1, sizeof(uint16_t), fp);
                    
                    uint8_t solid;
                    fread(&solid, 1, sizeof(uint8_t), fp);
                    
                    uint8_t flags;
                    fread(&flags, 1, sizeof(uint8_t), fp);
                    
                    uint32_t skin;
                    fread(&skin, 1, sizeof(uint32_t), fp);
                    
                    float FadeMinDist;
                    fread(&FadeMinDist, 1, sizeof(float), fp);
                    
                    float FadeMaxDist;
                    fread(&FadeMaxDist, 1, sizeof(float), fp);
                    
                    fread(&prop.lightOrigin, 1, sizeof(glm::vec3), fp);
                    
                    offset += 56;
                    
                    if (lump.version > 4) offset += 4;
                    if (lump.version > 5) offset += 4;
                }
            }
        }
    }
    
    {
        int count = lumps[LUMP_WORLDLIGHTS].filelen / sizeof(dworldlight_t);
        m_worldLights.resize(count);
        fseek(fp, lumps[LUMP_WORLDLIGHTS].fileofs, SEEK_SET);
        fread(m_worldLights.data(), count, sizeof(dworldlight_t), fp);
        int o;
    }
    
    fclose(fp);
    return (fp);
}

int SourceBSPAsset::findLeaf(glm::vec3 pos)
{
    int i = m_models[0].headnode;
    float distance = 0.0f;
    
    while(i >= 0)
    {
        const dnode_t& node = m_nodes[i];
        const dplane_t& plane = m_planes[node.planenum];
        
        distance = plane.normal.x * pos.x + plane.normal.y * pos.y + plane.normal.z * pos.z - plane.dist;
        
        if(distance >= 0)
        {
            i = node.children[0];
        }
        else
        {
            i = node.children[1];
        }
    }
    
    return -(i + 1);
}


std::string extractBaseTexture(const std::string& materialName)
{
    if (materialName.rfind("maps/", 0) == 0)
    {
        // Отбросим "maps/имя_карты/"
        size_t secondSlash = materialName.find('/', 5);
        
        if (secondSlash != std::string::npos)
        {
            std::string sub = materialName.substr(secondSlash + 1);
            
            // Отбросим координаты в конце
            size_t underscore = sub.find_last_of('_');
            
            if (underscore != std::string::npos) {
                sub = sub.substr(0, underscore);
            }
            
            underscore = sub.find_last_of('_');
            
            if (underscore != std::string::npos) {
                sub = sub.substr(0, underscore);
            }
            
            underscore = sub.find_last_of('_');
            
            if (underscore != std::string::npos) {
                sub = sub.substr(0, underscore);
            }
            
            return sub;
        }
    }
    
    return materialName;
}
