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
            
            m_pCamera->yaw = radians(-angle);
            m_pCamera->pitch = 0;
        }
        
        glm::vec3 origin = {0, 0, 0};
        
        if (spawnPoint.getVec3Value("origin", origin))
        {
            printf("origin = (%1.0f %1.0f %1.0f)\n", origin.x, origin.y, origin.z);
            
            m_pCamera->position.x = origin.x;
            m_pCamera->position.y = origin.z;
            m_pCamera->position.z = -origin.y;
            
            m_pCamera->position.y += 60;
        }
    }
    
    m_mesh.initFromBsp(&bsp);
    m_collision.initFromBsp(&bsp);
}

void Q3MapScene::update(float dt)
{
    if (window == nullptr) return;
    if (m_pCamera == nullptr) return;
    
    int newMouseButtonState = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
    bool isClick = (newMouseButtonState == GLFW_RELEASE && oldMouseButtonState == GLFW_PRESS);
    oldMouseButtonState = newMouseButtonState;
    
    m_pCamera->update(dt);
    
    if (isClick)
    {
        Ray ray = m_pCamera->getMousePosInWorld();
        AABB aabb = { .mins = {0, 0, 0}, .maxs = {0, 0, 0} };
        
        m_collision.check(ray.origin, ray.origin + ray.dir * 1024.0f, aabb);
    }
}

void Q3MapScene::draw()
{
    if (m_pCamera == nullptr) return;
    
    glm::mat4x4 mvp = m_pCamera->projection * m_pCamera->view;
    m_mesh.renderFaces(mvp);
}


