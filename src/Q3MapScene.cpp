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

#define MINIAUDIO_IMPLEMENTATION
#include "../vendor/miniaudio.h"

#include "StudioRenderer.h"
#include "Q3LightGrid.h"

#include "Monster.h"

#include "Input.h"

#include "DebugRenderer.h"
#include "Cube.h"

#define degrees(rad) ((rad) * (180.0f / M_PI))
#define radians(deg) ((deg) * (M_PI / 180.0f))

ma_result result;
ma_engine engine;

WiredCube cube;

Q3MapScene::Q3MapScene(Camera *camera) : m_pCamera(camera)
{
    studio = std::make_unique<StudioRenderer>();
    m_pLightGrid = std::make_unique<Q3LightGrid>();
    
    loadMap("assets/q3/maps/q3dm7.bsp");
    
    result = ma_engine_init(NULL, &engine);
    
    cube.init();
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

bool intersection(const glm::vec3& start, const glm::vec3& end, const glm::vec3& mins, const glm::vec3& maxs, glm::vec3& point);

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
    
    for (int i = 0; i < m_monsters.size(); ++i)
    {
        if (Input::isKeyPressed(48 + i))
        {
            auto& monster = m_monsters[i];
    
            glm::vec3 dirToPlayer = glm::normalize(m_pPlayer->position - monster.position);
            monster.yaw = atan2(dirToPlayer.y, dirToPlayer.x);
        }
    }
    
    if (Input::isLeftMouseButtonClicked())
    {
        ma_engine_play_sound(&engine, "assets/sounds/pl_gun3.wav", NULL);
        
        glm::vec3 start = m_pPlayer->position;
        start.z += 40;
        
        glm::vec3 end = start + m_pCamera->getForward() * 1024.0f;
        
        DebugRenderer::getInstance().addLine(start, end, glm::vec3(1), 10.0f);
        
        HitResult result;
        m_collision.trace(result, start, end, glm::vec3(0), glm::vec3(0));
        
        float dist1 = glm::length(result.endpos - start);
        
        for (const auto& monster : m_monsters)
        {
            glm::vec3 mins = monster.position + glm::vec3{-15, -15, -24};
            glm::vec3 maxs = monster.position + glm::vec3{ 15,  15,  48};
            
            glm::vec3 point;
            bool check = intersection(start, end, mins, maxs, point);
            float dist2 = glm::length(point - start);
            
            if (check && dist2 < dist1)
            {
                ma_engine_play_sound(&engine, "assets/sounds/donthurtem.wav", NULL);
                break;
            }
        }
    }
    
    DebugRenderer::getInstance().update(dt);
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
    
    glm::vec3 mins = glm::vec3{-15, -15, -24};
    glm::vec3 maxs = glm::vec3{ 15,  15,  48};
    
    for (auto& monster : m_monsters)
    {
        glm::vec3 absmins = monster.position + mins;
        glm::vec3 absmaxs = monster.position + maxs;
        
        cube.position = (absmins + absmaxs) * 0.5f;
        cube.scale = absmaxs - absmins;
        
        cube.draw(*m_pCamera);
    }
    
    DebugRenderer::getInstance().draw(*m_pCamera);
    
    glClear(GL_DEPTH_BUFFER_BIT);
    
    studio->drawViewModels(m_pCamera, m_pLightGrid.get());
}

#include <algorithm>
using std::min, std::max, std::swap;

bool intersection(const glm::vec3& start, const glm::vec3& end, const glm::vec3& mins, const glm::vec3& maxs, glm::vec3& point)
{
    glm::vec3 dir = end - start;
    
    glm::vec3 t1 = (mins - start) / dir;
    glm::vec3 t2 = (maxs - start) / dir;

    // Ensure tmin and tmax are sorted
    if (t1.x > t2.x) { swap(t1.x, t2.x); }
    if (t1.y > t2.y) { swap(t1.y, t2.y); }
    if (t1.z > t2.z) { swap(t1.z, t2.z); }

    float tmin = max(max(t1.x, t1.y), t1.z);
    float tmax = min(min(t2.x, t2.y), t2.z);

    if (tmax > tmin && tmax > 0.0)
    {
        point = start + dir * tmin;
        return true;
    }
    
    return false;
}
