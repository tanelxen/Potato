//
//  SourceMapScene.h
//  TryOpenGL
//
//  Created by Fedor Artemenkov on 12.05.25.
//

#pragma once

#include <string>
#include <vector>
#include <memory>

#include "SourceBspMesh.h"

class Camera;

class SourceMapScene
{
public:
    SourceMapScene(Camera* camera);
    ~SourceMapScene();
    
    void loadMap(const std::string &filename);
    void update(float dt);
    void draw();
    
private:
    Camera* m_pCamera;
    
    std::unique_ptr<class PlayerDebug> m_pPlayer;
    
    std::vector<struct StaticProp> m_staticProps;
    std::vector<struct SourceModel> m_staticPropInstances;
    
    SourceBspMesh m_mesh;
};
