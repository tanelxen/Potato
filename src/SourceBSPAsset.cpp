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
    LUMP_ENTITIES = 0,     // *
    LUMP_PLANES = 1,       // *
    LUMP_TEXDATA = 2,      // *
    LUMP_VERTEXES = 3,     // *
    LUMP_VISIBILITY = 4,   // *
    LUMP_NODES = 5,        // *
    LUMP_TEXINFO = 6,      // *
    LUMP_FACES = 7,        // *
    LUMP_LIGHTING = 8,     // *
    LUMP_OCCLUSION = 9,
    LUMP_LEAFS = 10,       // *
    LUMP_FACEIDS = 11,
    LUMP_EDGES = 12,       // *
    LUMP_SURFEDGES = 13,   // *
    LUMP_MODELS = 14,      // *
    LUMP_WORLDLIGHTS = 15, //
    LUMP_LEAFFACES = 16,   // *
    LUMP_LEAFBRUSHES = 17, // *
    LUMP_BRUSHES = 18,     // *
    LUMP_BRUSHSIDES = 19,  // *
    LUMP_AREAS = 20,       // *
    LUMP_AREAPORTALS = 21, // *
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
    LUMP_DISP_VERTS = 33,                     // CDispVerts
    LUMP_DISP_LIGHTMAP_SAMPLE_POSITIONS = 34, // For each displacement
    //     For each lightmap sample
    //         byte for index
    //         if 255, then index = next byte + 255
    //         3 bytes for barycentric coordinates
    // The game lump is a method of adding game-specific lumps
    // FIXME: Eventually, all lumps could use the game lump system
    LUMP_GAME_LUMP = 35,
    LUMP_LEAFWATERDATA = 36,
    LUMP_PRIMITIVES = 37,
    LUMP_PRIMVERTS = 38,
    LUMP_PRIMINDICES = 39,
    // A pak file can be embedded in a .bsp now, and the file system will search the pak
    //  file first for any referenced names, before deferring to the game directory
    //  file system/pak files and finally the base directory file system/pak files.
    LUMP_PAKFILE = 40,
    LUMP_CLIPPORTALVERTS = 41,
    // A map can have a number of cubemap entities in it which cause cubemap renders
    // to be taken after running vrad.
    LUMP_CUBEMAPS = 42,
    LUMP_TEXDATA_STRING_DATA = 43,
    LUMP_TEXDATA_STRING_TABLE = 44,
    LUMP_OVERLAYS = 45,
    LUMP_LEAFMINDISTTOWATER = 46,
    LUMP_FACE_MACRO_TEXTURE_INFO = 47,
    LUMP_DISP_TRIS = 48,
    LUMP_PHYSCOLLIDESURFACE = 49,     // deprecated.  We no longer use win32-specific Havok compression on terrain
    LUMP_WATEROVERLAYS = 50,
    LUMP_LEAF_AMBIENT_INDEX_HDR = 51, // index of LUMP_LEAF_AMBIENT_LIGHTING_HDR
    LUMP_LEAF_AMBIENT_INDEX = 52,     // index of LUMP_LEAF_AMBIENT_LIGHTING
    
    // optional lumps for HDR
    LUMP_LIGHTING_HDR = 53,
    LUMP_WORLDLIGHTS_HDR = 54,
    LUMP_LEAF_AMBIENT_LIGHTING_HDR = 55, // NOTE: this data overrides part of the data stored in LUMP_LEAFS.
    LUMP_LEAF_AMBIENT_LIGHTING = 56,     // NOTE: this data overrides part of the data stored in LUMP_LEAFS.
    
    LUMP_XZIPPAKFILE = 57,               // deprecated. xbox 1: xzip version of pak file
    LUMP_FACES_HDR = 58,                 // HDR maps may have different face data.
    LUMP_MAP_FLAGS = 59,                 // extended level-wide flags. not present in all levels
    LUMP_OVERLAY_FADES = 60,             // Fade distances for overlays
};

std::string extractBaseTexture(const std::string& materialName);

bool SourceBSPAsset::initFromFile(const std::string &filename)
{
    FILE* fp = fopen(filename.c_str(), "rb" );
    
    if(fp == nullptr) {
        printf("unable to open %s\n", filename.c_str());
        return false;
    }
    
    // Initialize the header and lump structures
    dheader_t header;
    lump_t lumps[64];
    
    // Read in the header and lump data
    fread(&header, 1, sizeof(dheader_t), fp);
    fread(&lumps, 64, sizeof(lump_t), fp);
    
    int numEntities = lumps[LUMP_ENTITIES].filelen;
    m_entities.resize(numEntities);
    fseek(fp, lumps[LUMP_ENTITIES].fileofs, SEEK_SET);
    fread(m_entities.data(), numEntities, sizeof(char), fp);
    
//    std::cout << "Entities:\n" << m_entities << std::endl;
    
    
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
    
    fclose(fp);
    return (fp);
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
