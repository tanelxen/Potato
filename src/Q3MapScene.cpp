//
//  Q3MapScene.cpp
//  TryOpenGL
//
//  Created by Fedor Artemenkov on 30.10.24.
//

#include "Q3MapScene.h"

#include "KeyValueCollection.h"
#include "Quake3Bsp.h"
#include "Camera.h"
#include "Player.h"
#include "PlayerMovement.h"

#include <glad/glad.h>

#include "StudioRenderer.h"
#include "Q3LightGrid.h"

#include "Monster.h"

#define degrees(rad) ((rad) * (180.0f / M_PI))
#define radians(deg) ((deg) * (M_PI / 180.0f))

Q3MapScene::Q3MapScene(Camera *camera) : m_pCamera(camera)
{
    studio = std::make_unique<StudioRenderer>();
    m_pLightGrid = std::make_unique<Q3LightGrid>();
    
    loadMap("assets/q3/maps/q3dm7.bsp");
}

Q3MapScene::~Q3MapScene() = default;

void Q3MapScene::loadMap(const std::string &filename)
{
    Quake3BSP bsp;

    if (!bsp.initFromFile(filename.c_str())) {
        return;
    }
    
    m_collision.initFromBsp(&bsp);
    
    m_pMovement = std::make_unique<PlayerMovement>(&m_collision);
    
    m_pPlayer = std::make_unique<Player>(m_pMovement.get());
    
    KeyValueCollection entities;
    entities.initFromString(bsp.m_entities);
    
    auto info_player_deathmatch = entities.getAllWithKeyValue("classname", "info_player_deathmatch");
    auto info_player_start = entities.getAllWithKeyValue("classname", "info_player_start");
    auto ai_soldier = entities.getAllWithKeyValue("classname", "ai_soldier");
    
    auto spawnPoints = info_player_deathmatch;
    spawnPoints.insert(spawnPoints.end(), info_player_start.begin(), info_player_start.end());
    spawnPoints.insert(spawnPoints.end(), ai_soldier.begin(), ai_soldier.end());
    
    for (int i = 0; i < spawnPoints.size(); ++i)
    {
        auto spawnPoint = spawnPoints[i];
        
        float yaw = 0;
        glm::vec3 position = {0, 0, 0};
        
        int angle = 0;
        
        if (spawnPoint.getIntValue("angle", angle))
        {
            printf("entity_%i: angle = %i\n", i, angle);
            yaw = radians(angle);
        }
        
        glm::vec3 origin = {0, 0, 0};
        
        if (spawnPoint.getVec3Value("origin", origin))
        {
            position = origin;
        }
        
        if (i == 0)
        {
            m_pPlayer->m_pModelInstance = studio->makeModelInstance("assets/models/v_9mmhandgun.mdl");
            m_pPlayer->position = position + glm::vec3{0, 0, 0.25};
            m_pPlayer->yaw = yaw;
            m_pPlayer->pitch = 0;
            
            m_pPlayer->m_pModelInstance->animator.setSeqIndex(2);
        }
        else
        {
            auto& monster = m_monsters.emplace_back();
            monster.m_pModelInstance = studio->makeModelInstance("assets/models/barney.mdl");
            monster.position = position;
            monster.yaw = yaw;
            
            int seqIndex = (int) (m_monsters.size() - 1) % monster.m_pModelInstance->animator.getNumSeq();
            monster.m_pModelInstance->animator.setSeqIndex(seqIndex);
        }
    }
    
    m_mesh.initFromBsp(&bsp);
    
    m_pLightGrid->init(bsp);
}

void Q3MapScene::update(float dt)
{
    if (m_pCamera == nullptr) return;
    
    m_pPlayer->update(dt);
    studio->queueViewModel(m_pPlayer->m_pModelInstance.get());
    
    glm::vec3 camera_pos = m_pPlayer->position;
    camera_pos.z += 40;
    
    m_pCamera->setTransform(camera_pos, m_pPlayer->forward, m_pPlayer->right, m_pPlayer->up);
    
    for (auto& monster : m_monsters)
    {
        monster.update(dt);
        studio->queue(monster.m_pModelInstance.get());
    }
}

void Q3MapScene::draw()
{
    if (m_pCamera == nullptr) return;
    
    glEnable(GL_DEPTH_TEST);
    
    glClearColor(0.1, 0.1, 0.1, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glm::mat4x4 mvp = m_pCamera->projection * m_pCamera->view;
    m_mesh.renderFaces(mvp);
    
    studio->drawRegular(m_pCamera, m_pLightGrid.get());
    
    glClear(GL_DEPTH_BUFFER_BIT);
    
    studio->drawViewModels(m_pCamera, m_pLightGrid.get());
}


