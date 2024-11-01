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

class Camera;
class GLFWwindow;

class Q3MapScene
{
public:
    Q3MapScene(GLFWwindow* window, Camera* camera);
    
    void loadMap(const std::string &filename);
    void update(float dt);
    void draw();
    
private:
    Camera* m_pCamera;
    Q3BspMesh m_mesh;
    Q3BspCollision m_collision;
    
    GLFWwindow* window;
};
