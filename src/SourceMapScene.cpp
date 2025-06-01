//
//  SourceMapScene.cpp
//  TryOpenGL
//
//  Created by Fedor Artemenkov on 12.05.25.
//

#include "SourceMapScene.h"

#include "KeyValueCollection.h"
#include "SourceBSPAsset.h"
#include "Camera.h"
#include "Player.h"
#include "PlayerMovement.h"

#include <glad/glad.h>

#include "Shader.h"
#include "SourceModel.h"

#include "Input.h"


#define degrees(rad) ((rad) * (180.0f / M_PI))
#define radians(deg) ((deg) * (M_PI / 180.0f))

static Shader mdl_shader;

void transformMatrix(glm::mat4x4 &dst, const glm::vec3 &angles, const glm::vec3 &pos);

struct StaticProp
{
    glm::mat4x4 transform;
    uint16_t instance;
};

SourceMapScene::SourceMapScene(Camera *camera) : m_pCamera(camera)
{
    mdl_shader.init("assets/shaders/source_mdl.glsl");
    glUniform1i(glGetUniformLocation(mdl_shader.program, "s_texture"), 0);
    
    loadMap("assets/hl2/maps/d1_canals_01.bsp");
}

SourceMapScene::~SourceMapScene() = default;

void SourceMapScene::loadMap(const std::string &filename)
{
    SourceBSPAsset bsp;

    if (!bsp.initFromFile(filename.c_str())) {
        return;
    }
    
    m_pPlayer = std::make_unique<PlayerDebug>();
    
    KeyValueCollection entities;
    entities.initFromString(bsp.m_entities);
    
    auto info_player_start = entities.getAllWithKeyValue("classname", "info_player_start");
    
    for (int i = 0; i < info_player_start.size(); ++i)
    {
        auto spawnPoint = info_player_start[i];
        
        float yaw = 0;
        glm::vec3 position = {0, 0, 0};
        
        glm::vec3 angles = {0, 0, 0};
        
        if (spawnPoint.getVec3Value("angles", angles))
        {
            yaw = radians(angles.y);
        }
        
        glm::vec3 origin = {0, 0, 0};
        
        if (spawnPoint.getVec3Value("origin", origin))
        {
            position = origin;
        }
        
        if (i == 0)
        {
            m_pPlayer->position = position + glm::vec3{0, 0, 0.25};
            m_pPlayer->yaw = yaw;
            m_pPlayer->pitch = 0;
        }
        
        break;
    }
    
    m_staticPropInstances.resize(bsp.m_staticModelDict.size());
    
    for (int i = 0; i < bsp.m_staticModelDict.size(); ++i)
    {
        std::string path = "assets/hl2/" + bsp.m_staticModelDict[i];
        
        SourceMDLAsset model;
        model.loadFromFile(path);
        
        m_staticPropInstances[i].init(model);
    }
    
    m_staticProps.resize(bsp.m_staticProps.size());
    
    for (int i = 0; i < bsp.m_staticProps.size(); ++i)
    {
        transformMatrix(m_staticProps[i].transform, bsp.m_staticProps[i].angles, bsp.m_staticProps[i].origin);
        m_staticProps[i].instance = bsp.m_staticProps[i].modelIndex;
    }
    
    m_mesh.initFromBsp(&bsp);
}

void SourceMapScene::update(float dt)
{
    if (m_pCamera == nullptr) return;
    
    m_pPlayer->update(dt);
    
    glm::vec3 camera_pos = m_pPlayer->position;
    camera_pos.z += 40;
    
    m_pCamera->setTransform(camera_pos, m_pPlayer->forward, m_pPlayer->right, m_pPlayer->up);
}

void SourceMapScene::draw()
{
    if (m_pCamera == nullptr) return;
    
    glEnable(GL_DEPTH_TEST);
    
    glClearColor(0.1, 0.1, 0.1, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glm::mat4x4 mvp = m_pCamera->projection * m_pCamera->view;
    m_mesh.renderFaces(mvp);
    
    mdl_shader.bind();
    
    for (auto& prop : m_staticProps)
    {
        mvp = m_pCamera->projection * m_pCamera->view * prop.transform;
        mdl_shader.setUniform("uMVP", mvp);
        
        m_staticPropInstances[prop.instance].draw();
    }
}

void transformMatrix(glm::mat4x4 &dst, const glm::vec3 &angles, const glm::vec3 &pos)
{
    const float pitch = radians(angles[0]);
    const float yaw =   radians(angles[1]);
    const float roll =  radians(angles[2]);
    
    const float sinP = sin(pitch), cosP = cos(pitch);
    const float sinY = sin(yaw),   cosY = cos(yaw);
    const float sinR = sin(roll),  cosR = cos(roll);
    
    dst[0][0] =  (cosP * cosY);
    dst[0][1] =  (cosP * sinY);
    dst[0][2] =  (-sinP);
    dst[0][3] =  0.0;
    
    dst[1][0] =  (sinP * sinR * cosY - cosR * sinY);
    dst[1][1] =  (sinP * sinR * sinY + cosR * cosY);
    dst[1][2] =  (sinR * cosP);
    dst[1][3] =  0.0;
    
    dst[2][0] =  (sinP * cosR * cosY + sinR * sinY);
    dst[2][1] =  (sinP * cosR * sinY - sinR * cosY);
    dst[2][2] = (cosR * cosP);
    dst[2][3] = 0.0;
    
    dst[3][0] = pos[0];
    dst[3][1] = pos[1];
    dst[3][2] = pos[2];
    dst[3][3] = 1.0;
}
