//
//  MDLFile.cpp
//  hl2mv
//
//  Created by Fedor Artemenkov on 27.05.25.
//

#include "MDLFile.h"

#include <span>

#pragma pack(1)
struct mdlTexture_t
{
    int    name_offset;
    int    flags;
    
    int    unused2[14];
    
    const char* get_name() const {
        return (char *)this + name_offset;
    }
};

struct mdlMesh_t
{
    uint32_t    skinrefIndex;
    int         modelindex;
    
    int         numvertices;
    int         vertexoffset;
    
    int         numflexes;
    int         flexindex;
    
    int         materialtype;
    int         materialparam;
    int         meshid;
    float       center[3];
    int         unused[17];
};

struct mdlModel_t
{
    char        name[64];
    
    int         type;
    
    float       boundingradius;
    
    int         nummeshes;
    int         meshindex;
    
    int         numvertices;
    int         vertexindex;
    int         tangentsindex;
    
    int         numattachments;
    int         attachmentindex;
    
    int         numeyeballs;
    int         eyeballindex;
    
    int         unused[10];
    
    inline mdlMesh_t *pMesh(int i) const
    {
        return (mdlMesh_t *)(((uint8_t *)this) + meshindex) + i;
    }
};

struct mdlBodypart_t
{
    int         sznameindex;
    int         nummodels;
    int         base;
    int         modelindex;
    
    inline const char* get_name() const {
        return (char *)this + sznameindex;
    }
    
    inline mdlModel_t *pModel(int i) const
    {
        return (mdlModel_t *)(((uint8_t *)this) + modelindex) + i;
    }
};

struct mdlBone_t
{
    int sznameindex;
    int parent;
    int bonecontroller[6];
    
    // default values
    float pos[3];
    float quat[4];
    float rot[3];
    
    // compression scale
    float posscale[3];
    float rotscale[3];
    
    float   poseToBone[12];
    float   qAlignment[4];
    int     flags;
    int     proctype;
    int     procindex;
    int     physicsbone;
    
    int surfacepropidx;
    int contents;
    int surfacepropLookup;
    int unused[7];
    
    const char* get_name() const {
        return (char *)this + sznameindex;
    }
};

struct mdlSeqDesc_t
{
    int                    baseptr;
    
    int                    szlabelindex;
    
    int                    szactivitynameindex;
    
    int                    flags;        // looping/non-looping flags
    
    int                    activity;    // initialized at loadtime to game DLL values
    int                    actweight;
    
    int                    numevents;
    int                    eventindex;
    
    float                bbmin[3];
    float                bbmax[3];
    
    int                    numblends;
    
    // Index into array of shorts which is groupsize[0] x groupsize[1] in length
    int                    animindexindex;
    
    int                    movementindex;    // [blend] float array for blended movement
    int                    groupsize[2];
    int                    paramindex[2];    // X, Y, Z, XR, YR, ZR
    float                paramstart[2];    // local (0..1) starting value
    float                paramend[2];    // local (0..1) ending value
    int                    paramparent;
    
    float                fadeintime;        // ideal cross fate in time (0.2 default)
    float                fadeouttime;    // ideal cross fade out time (0.2 default)
    
    int                    localentrynode;        // transition node at entry
    int                    localexitnode;        // transition node at exit
    int                    nodeflags;        // transition rules
    
    float                entryphase;        // used to match entry gait
    float                exitphase;        // used to match exit gait
    
    float                lastframe;        // frame that should generation EndOfSequence
    
    int                    nextseq;        // auto advancing sequences
    int                    pose;            // index of delta animation between end and nextseq
    
    int                    numikrules;
    
    int                    numautolayers;
    int                    autolayerindex;
    
    int                    weightlistindex;
    
    int                    posekeyindex;
    
    int                    numiklocks;
    int                    iklockindex;
    
    int                    keyvalueindex;
    int                    keyvaluesize;
    
    int                    cycleposeindex;
    
    int                    unused[7];
    
    const char* get_name() const {
        return (char *)this + szlabelindex;
    }
};

struct mdlHeader_t
{
    int         id;             // IDST
    int         version;        // 44
    uint32_t    checksum;       // This has to be the same in the phy and vtx files to load!
    char        name[64];
    int         dataLength;
    
    float      eyeposition[3];    // Position of player viewpoint relative to model origin
    float      illumposition[3];  // Position (relative to model origin) used to calculate ambient light contribution and cubemap reflections for the entire model.
    float      hull_min[3];       // Corner of model hull box with the least X/Y/Z values
    float      hull_max[3];       // Opposite corner of model hull box
    float      view_bbmin[3];     // Same, but for bounding box,
    float      view_bbmax[3];     // which is used for view culling
    
    int         flags;          // Binary flags in little-endian order.
    
    int        bone_count;    // Number of data sections (of type mstudiobone_t)
    int        bone_offset;   // Offset of first data section
    
    int        bonecontroller_count;
    int        bonecontroller_offset;
    
    int        hitbox_count;
    int        hitbox_offset;
    
    int        localanim_count;
    int        localanim_offset;
    
    int        localseq_count;
    int        localseq_offset;
    
    int        activitylistversion; // ??
    int        eventsindexed;       // ??
    
    int        texture_count;
    int        texture_offset;
    inline mdlTexture_t *pTexture(int i) const
    {
        return (mdlTexture_t *)(((uint8_t *)this) + texture_offset) + i;
    }
    
    int        texturedir_count;
    int        texturedir_offset;
    inline const char* pTextureDir(int i) const
    {
        int *pDir = (int *)(((uint8_t *)this) + texturedir_offset) + i;
        return (const char *)(((uint8_t *)this) + *pDir);
    }
    
    int        skinreference_count;
    int        skinrfamily_count;
    int        skinreference_index;
    
    int        bodypart_count;
    int        bodypart_offset;
    inline mdlBodypart_t *pBodypart(int i) const
    {
        return (mdlBodypart_t *)(((uint8_t *)this) + bodypart_offset) + i;
    }
    
    int        attachment_count;
    int        attachment_offset;
    
    int        localnode_count;
    int        localnode_index;
    int        localnode_name_index;
    
    int        flexdesc_count;
    int        flexdesc_index;
    
    int        flexcontroller_count;
    int        flexcontroller_index;
    
    int        flexrules_count;
    int        flexrules_index;
    
    int        ikchain_count;
    int        ikchain_index;
    
    int        mouths_count;
    int        mouths_index;
    
    int        localposeparam_count;
    int        localposeparam_index;
    
    int        surfaceprop_index;
    
    int        keyvalue_index;
    int        keyvalue_count;
    
    int        iklock_count;
    int        iklock_index;
    
    
    float      mass;
    
    int        contents;
    
    int        includemodel_count;
    int        includemodel_index;
    
    int        virtualModel;
    
    int        animblocks_name_index;
    int        animblocks_count;
    int        animblocks_index;
    
    int        animblockModel;
    
    int        bonetablename_index;
    
    int        vertex_base;
    int        offset_base;
    
    uint8_t        directionaldotproduct;
    
    uint8_t        rootLod;
    uint8_t        numAllowedRootLods;
    
    uint8_t     unused0;
    int         unused1;
    
    int         flexcontrollerui_count;
    int         flexcontrollerui_index;
    
    float       vertAnimFixedPointScale;
    int         unused2;
    
    int         studiohdr2index;
    int         unused3; // ??
};
#pragma pack()

void MDLFile::loadFromBuffer(const void* buffer)
{
    mdlHeader_t* pHeader = (mdlHeader_t*) buffer;
    
    m_name = pHeader->name;
    m_checksum = pHeader->checksum;
    
    m_textureNames.resize(pHeader->texture_count);
    
    for (int i = 0; i < pHeader->texture_count; ++i)
    {
        m_textureNames[i] = pHeader->pTexture(i)->get_name();
    }
    
    m_textureDirs.resize(pHeader->texturedir_count);
    
    for (int i = 0; i < pHeader->texturedir_count; ++i)
    {
        m_textureDirs[i] = pHeader->pTextureDir(i);
    }
    
    uint16_t* pskins = (uint16_t *)((uint8_t*)pHeader + pHeader->skinreference_index);
    std::span<uint16_t> skins(pskins, pHeader->skinrfamily_count * pHeader->skinreference_count);
    
    for (int bodyPartIndex = 0; bodyPartIndex < pHeader->bodypart_count; ++bodyPartIndex)
    {
        auto pBodypart = pHeader->pBodypart(bodyPartIndex);
        
        auto& bodypart = bodyparts.emplace_back();
        bodypart.m_name = pBodypart->get_name();
        
        for (int modelIndex = 0; modelIndex < pBodypart->nummodels; ++modelIndex)
        {
            auto pModel = pBodypart->pModel(modelIndex);
            
            auto& model = bodypart.models.emplace_back();
            model.m_name = pModel->name;
            
            for (int meshIndex = 0; meshIndex < pModel->nummeshes; ++meshIndex)
            {
                auto pMesh = pModel->pMesh(meshIndex);
                
                auto& mesh = model.meshes.emplace_back();
                mesh.skinrefIndex = pMesh->skinrefIndex;
                mesh.vertexoffset = pMesh->vertexoffset;
            }
        }
    }
}
