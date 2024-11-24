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
#include <GLFW/glfw3.h>

#include "StudioRenderer.h"

#define degrees(rad) ((rad) * (180.0f / M_PI))
#define radians(deg) ((deg) * (M_PI / 180.0f))

//static int oldMouseButtonState = GLFW_RELEASE;


Q3MapScene::Q3MapScene(GLFWwindow* window, Camera *camera) : window(window), m_pCamera(camera)
{
    studio = new StudioRenderer();
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
    entities.initFromString(bsp.m_entities);
    
    auto spawnPoints = entities.getAllWithKeyValue("classname", "info_player_deathmatch");
    
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
//            position.x = origin.x;
//            position.y = origin.z;
//            position.z = -origin.y;
            
            position = origin;
        }
        
        if (i == 0)
        {
            m_pPlayer->position = position + glm::vec3{0, 0, 8};
            m_pPlayer->yaw = yaw;
            m_pPlayer->pitch = 0;
        }
        else
        {
            auto modelInstance = studio->makeModelInstance("assets/models/barney.mdl");
            modelInstance->position = position + glm::vec3{0, 0, -24};
            modelInstance->yaw = yaw;
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
    camera_pos.z += 38;
    
    m_pCamera->setTransform(camera_pos, m_pPlayer->forward, m_pPlayer->right, m_pPlayer->up);
    
    studio->update(dt);
    
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
    
    glEnable(GL_DEPTH_TEST);
    
    glClearColor(0.1, 0.1, 0.1, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glm::mat4x4 mvp = m_pCamera->projection * m_pCamera->view;
    m_mesh.renderFaces(mvp);
    
    studio->draw(m_pCamera);
    
    glClear(GL_DEPTH_BUFFER_BIT);
    
    studio->drawWeapon(m_pCamera);
}


