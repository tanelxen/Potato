//
//  Q3MapScene.hpp
//  TryOpenGL
//
//  Created by Fedor Artemenkov on 30.10.24.
//

#pragma once

#include <string>
#include "Q3BspMesh.h"
#include "Q3BspCollision.h"

struct GLFWwindow;
class Camera;
class PlayerMovement;
class Player;

struct StudioRenderer;

class Q3MapScene
{
public:
    Q3MapScene(GLFWwindow* window, Camera* camera);
    
    void loadMap(const std::string &filename);
    void update(float dt);
    void draw();
    
private:
    Camera* m_pCamera;
    
    Player* m_pPlayer;
    PlayerMovement* m_pMovement;
    
    Q3BspMesh m_mesh;
    Q3BspCollision m_collision;
    
    GLFWwindow* window;
    
    StudioRenderer* studio;
};
