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

#include "StudioRenderer.h"

#include "Input.h"


#define degrees(rad) ((rad) * (180.0f / M_PI))
#define radians(deg) ((deg) * (M_PI / 180.0f))

SourceMapScene::SourceMapScene(Camera *camera) : m_pCamera(camera)
{
    studio = std::make_unique<StudioRenderer>();
    
    loadMap("assets/hl2/maps/d1_trainstation_02.bsp");
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
        
        int angle = 0;
        
        if (spawnPoint.getIntValue("angle", angle))
        {
            yaw = radians(angle);
        }
        
        glm::vec3 origin = {0, 0, 0};
        
        if (spawnPoint.getVec3Value("origin", origin))
        {
            printf("entity_%i: origin = (%.0f, %.0f, %.0f)\n", i, origin.x, origin.y, origin.z);
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
}
