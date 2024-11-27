//
//  RenderableModel.hpp
//  hlmv
//
//  Created by Fedor Artemenkov on 17.11.24.
//

#pragma once

#include <vector>
#include <glm/glm.hpp>
#include "GoldSrcMDLAsset.h"

struct GoldSrcMesh
{
    void init(const GoldSrc::Model& model);
    void draw();
    
private:
    void uploadTextures(const std::vector<GoldSrc::Texture>& textures);
    void uploadMeshes(const std::vector<GoldSrc::Mesh>& meshes);
    
private:
    unsigned int vbo;
    unsigned int ibo;
    unsigned int vao;
    std::vector<unsigned int> textures;
    
    struct RenderableSurface
    {
        unsigned int tex;
        int bufferOffset;
        int indicesCount;
    };
    
    std::vector<RenderableSurface> surfaces;
};

struct GoldSrcAnimation
{
    std::vector<GoldSrc::Sequence> sequences;
    std::vector<int> bones;
};

// Shared between instances
struct GoldSrcModel
{
    GoldSrcMesh mesh;
    GoldSrcAnimation animation;
};

struct GoldSrcAnimator
{
    void setSeqIndex(int index);
    int getSeqIndex() const;
    
private:
    float cur_frame = 0;
    float cur_frame_time = 0;
    float cur_anim_duration = 0;
    int cur_seq_index = 0;
    
    // Transforms for each bone
    std::vector<glm::mat4> transforms;
    
    void updatePose(const GoldSrcAnimation& animation);
    
    // Only StudioRenderer can update animators
    friend struct StudioRenderer;
    
    void update(const GoldSrcAnimation& animation, float dt);
    const std::vector<glm::mat4>& getBoneTransforms() const;
};

// Instance with unique transforms and animation
struct GoldSrcModelInstance
{
    GoldSrcAnimator animator;
    
    glm::vec3 position;
    float yaw;
    
private:
    // Only StudioRenderer has access to GoldSrcModel
    friend struct StudioRenderer;
    GoldSrcModel* m_pmodel;
};
