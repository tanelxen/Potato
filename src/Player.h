//
//  Player.h
//  TryOpenGL
//
//  Created by Fedor Artemenkov on 03.11.24.
//

#pragma once

#include <glm/glm.hpp>

struct GLFWwindow;
class PlayerMovement;

class Player
{
public:
    Player(GLFWwindow* window, PlayerMovement* movement);
    
    void update(float dt);
    
    glm::vec3 position = {0, 128, 256};
    
    glm::vec3 forward = {0, 0, 0};
    glm::vec3 right = {0, 0, 0};
    glm::vec3 up = {0, 0, 0};
    
    float pitch = -0.5;
    float yaw = -1.57;
    
private:
    GLFWwindow* m_pWindow;
    PlayerMovement* m_pMovement;
    
    float mouseSense = 0.15;

    double prevMouseX = 0;
    double prevMouseY = 0;
    bool isFirstFrame = true;
    
    glm::vec3 velocity = {0, 0, 0};
};
