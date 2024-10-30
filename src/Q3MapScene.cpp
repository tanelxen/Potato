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

#define degrees(rad) ((rad) * (180.0f / M_PI))
#define radians(deg) ((deg) * (M_PI / 180.0f))


Q3MapScene::Q3MapScene(Camera *camera) : m_pCamera(camera)
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
}

void Q3MapScene::update(float dt)
{
    m_pCamera->update(dt);
}

void Q3MapScene::draw()
{
    glm::mat4x4 mvp = m_pCamera->projection * m_pCamera->view;
    m_mesh.renderFaces(mvp);
}


