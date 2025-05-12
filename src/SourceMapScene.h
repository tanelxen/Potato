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
class PlayerDebug;

struct StudioRenderer;

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
    
    std::unique_ptr<PlayerDebug> m_pPlayer;
    std::unique_ptr<StudioRenderer> studio;
    
    SourceBspMesh m_mesh;
};
