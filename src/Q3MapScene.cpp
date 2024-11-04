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

#include <GLFW/glfw3.h>

#define degrees(rad) ((rad) * (180.0f / M_PI))
#define radians(deg) ((deg) * (M_PI / 180.0f))

static int oldMouseButtonState = GLFW_RELEASE;


Q3MapScene::Q3MapScene(GLFWwindow* window, Camera *camera) : window(window), m_pCamera(camera)
{
    
}

void Q3MapScene::loadMap(const std::string &filename)
{
    Quake3BSP bsp;

    if (!bsp.initFromFile(filename.c_str())) {
        return;
    }
    
    m_collision.initFromBsp(&bsp);
    
    m_pMovement = new PlayerMovement(&m_collision);
    
    m_pPlayer = new Player(window, m_pMovement);
    
    KeyValueCollection entities;
    entities.initFromString(bsp.m_pEntities);
    
    auto spawnPoints = entities.getAllWithKeyValue("classname", "info_player_deathmatch");
    
    if (spawnPoints.size() > 0)
    {
        auto spawnPoint = spawnPoints[0];
        
        int angle = 0;
        
        if (spawnPoint.getIntValue("angle", angle))
        {
            printf("angle = %i\n", angle);
            
            m_pPlayer->yaw = radians(-angle);
            m_pPlayer->pitch = 0;
        }
        
        glm::vec3 origin = {0, 0, 0};
        
        if (spawnPoint.getVec3Value("origin", origin))
        {
            printf("origin = (%1.0f %1.0f %1.0f)\n", origin.x, origin.y, origin.z);
            
            m_pPlayer->position.x = origin.x;
            m_pPlayer->position.y = origin.z;
            m_pPlayer->position.z = -origin.y;
            
            m_pPlayer->position.y += 16;
        }
    }
    
    m_mesh.initFromBsp(&bsp);
}

void Q3MapScene::update(float dt)
{
    if (window == nullptr) return;
    if (m_pCamera == nullptr) return;
    
    m_pPlayer->update(dt);
    
    glm::vec3 camera_pos = m_pPlayer->position;
    camera_pos.y += 40;
    
    m_pCamera->setTransform(camera_pos, m_pPlayer->forward, m_pPlayer->right, m_pPlayer->up);
    
//    int newMouseButtonState = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
//    bool isClick = (newMouseButtonState == GLFW_RELEASE && oldMouseButtonState == GLFW_PRESS);
//    oldMouseButtonState = newMouseButtonState;
//    
//    
//    if (isClick)
//    {
//        Ray ray = m_pCamera->getMousePosInWorld();
//        AABB aabb = { .mins = {0, 0, 0}, .maxs = {0, 0, 0} };
//        
//        m_collision.check(ray.origin, ray.origin + ray.dir * 1024.0f, aabb);
//    }
}

void Q3MapScene::draw()
{
    if (m_pCamera == nullptr) return;
    
    glm::mat4x4 mvp = m_pCamera->projection * m_pCamera->view;
    m_mesh.renderFaces(mvp);
}


