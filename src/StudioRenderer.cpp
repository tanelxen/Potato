//
//  StudioRenderer.cpp
//  TryOpenGL
//
//  Created by Fedor Artemenkov on 19.11.24.
//

#include <glm/gtc/matrix_transform.hpp>

#include "StudioRenderer.h"
#include "Camera.h"

#include "Q3LightGrid.h"

StudioRenderer::StudioRenderer()
{
    m_shader.init("assets/shaders/goldsrc_model.glsl");
    
    m_player = std::make_unique<GoldSrcModelInstance>();
    m_player->m_pmodel = makeModel("assets/models/v_9mmhandgun.mdl");
    m_player->animator.setSeqIndex(2);
}

void StudioRenderer::update(float dt)
{
    m_player->animator.update(m_player->m_pmodel->animation, dt);
    
    for (auto& inst : m_instances)
    {
        inst.animator.update(inst.m_pmodel->animation, dt);
    }
}

void StudioRenderer::draw(Camera* camera, Q3LightGrid* lightGrid)
{
    m_shader.bind();
    
    for (auto& inst : m_instances)
    {
        glm::mat4 model = glm::mat4(1);
        
        model = glm::translate(model, inst.position);
        model = glm::rotate(model, inst.yaw, glm::vec3(0, 0, 1));
        
        glm::vec3 ambient = glm::vec3{1};
        glm::vec3 color = glm::vec3{1};
        glm::vec3 dir = glm::vec3{0};
        
        if (lightGrid != nullptr)
        {
            glm::vec3 pos = inst.position;
            pos.z += 24;
            
            lightGrid->getValue(pos, ambient, color, dir);
        }
        
        m_shader.setUniform("u_ambient", ambient);
        m_shader.setUniform("u_color", color);
        m_shader.setUniform("u_dir", dir);
        
        // Model matrix uses for rotate normals
        m_shader.setUniform("uModel", model);
        
        glm::mat4 mvp = camera->projection * camera->view * model;
        m_shader.setUniform("uMVP", mvp);
        
        
        m_shader.setUniform("uBoneTransforms", inst.animator.getBoneTransforms());
        
        inst.m_pmodel->mesh.draw();
    }
}

void StudioRenderer::drawWeapon(Camera* camera, Q3LightGrid* lightGrid)
{
    m_shader.bind();
    
    glm::mat4 quakeToGL = {
         0,  0, -1,   0,
        -1,  0,  0,   0,
         0,  1,  0,   0,
         0,  0,  0.5, 1
    };
    
    glm::vec3 ambient = glm::vec3{1};
    glm::vec3 color = glm::vec3{1};
    glm::vec3 dir = glm::vec3{0};
    
    if (lightGrid != nullptr)
    {
        glm::vec3 pos = camera->getPosition();
        
        lightGrid->getValue(pos, ambient, color, dir);
    }
    
    dir = glm::mat3(camera->view) * dir;
    
    m_shader.setUniform("u_ambient", ambient);
    m_shader.setUniform("u_color", color);
    m_shader.setUniform("u_dir", dir);
    
    // Model matrix uses for rotate normals
    m_shader.setUniform("uModel", quakeToGL);
    
    glm::mat4 mvp = camera->weaponProjection * quakeToGL;
    m_shader.setUniform("uMVP", mvp);
    
    m_shader.setUniform("uBoneTransforms", m_player->animator.getBoneTransforms());
    
    m_player->m_pmodel->mesh.draw();
}

GoldSrcModel* StudioRenderer::makeModel(const std::string& filename)
{
    auto [it, inserted] = m_cache.try_emplace(filename);
    
    GoldSrcModel& model = it->second;
    
    if (inserted)
    {
        GoldSrc::Model asset;
        asset.loadFromFile(filename);
        
        model.mesh.init(asset);
        model.animation.sequences = asset.sequences;
        model.animation.bones = asset.bones;
    }
    else
    {
        printf("Use cached model for %s\n", filename.c_str());
    }
    
    return &model;
}

GoldSrcModelInstance* StudioRenderer::makeModelInstance(const std::string& filename)
{
    auto index = m_instances.size();
    
    GoldSrcModelInstance& modelInstance = m_instances.emplace_back();
    modelInstance.m_pmodel = makeModel(filename);
    
    auto count = modelInstance.m_pmodel->animation.sequences.size();
    int seqIndex = (int) index % count;
    
    modelInstance.animator.setSeqIndex(seqIndex);
    
    return &modelInstance;
}

