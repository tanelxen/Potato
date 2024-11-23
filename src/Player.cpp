//
//  Player.cpp
//  TryOpenGL
//
//  Created by Fedor Artemenkov on 03.11.24.
//

#include "Player.h"
#include <GLFW/glfw3.h>
#include "PlayerMovement.h"

Player::Player(GLFWwindow *window, PlayerMovement *movement) : m_pWindow(window), m_pMovement(movement)
{
    
}

void Player::update(float dt)
{
    if (m_pWindow == nullptr) return;
    if (m_pMovement == nullptr) return;
    
    glfwSetInputMode(m_pWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    if (isFirstFrame)
    {
        glfwGetCursorPos(m_pWindow, &prevMouseX, &prevMouseY);
        isFirstFrame = false;
        return;
    }
    
    double curMouseX = 0;
    double curMouseY = 0;
    glfwGetCursorPos(m_pWindow, &curMouseX, &curMouseY);

    double dx = curMouseX - prevMouseX;
    double dy = curMouseY - prevMouseY;

    prevMouseX = curMouseX;
    prevMouseY = curMouseY;

    yaw -= dx * mouseSense * dt;
    pitch += dy * mouseSense * dt;
    
    if (pitch > 1.5) pitch = 1.5;
    else if (pitch < -1.5) pitch = -1.5;

    
    forward.x = cos(pitch) * cos(yaw);
    forward.y = cos(pitch) * sin(yaw);
    forward.z = -sin(pitch);
    forward = glm::normalize(forward);

    glm::vec3 unit_up = {0, 0, 1};

    right = glm::cross(forward, unit_up);
    right = glm::normalize(right);

    up = glm::cross(right, forward);
    up = glm::normalize(up);

    
    float forwardmove = 0;
    float rightmove = 0;

    if (glfwGetKey(m_pWindow, GLFW_KEY_W) > GLFW_RELEASE)
    {
        forwardmove += 1;
    }

    if (glfwGetKey(m_pWindow, GLFW_KEY_S) > GLFW_RELEASE)
    {
        forwardmove -= 1;
    }

    if (glfwGetKey(m_pWindow, GLFW_KEY_D) > GLFW_RELEASE)
    {
        rightmove += 1;
    }

    if (glfwGetKey(m_pWindow, GLFW_KEY_A) > GLFW_RELEASE)
    {
        rightmove -= 1;
    }
    
    bool jump = glfwGetKey(m_pWindow, GLFW_KEY_SPACE) == GLFW_PRESS;
    
    m_pMovement->setTransform(position, forward, right, up);
    m_pMovement->setInputMovement(forwardmove, rightmove, jump);
    m_pMovement->update(dt);
    
    position = m_pMovement->getPosition();
    
    
//    float moveSpeed = 96;
//    
//    velocity = {0, 0, 0};
//
//    if (glfwGetKey(m_pWindow, GLFW_KEY_W) > GLFW_RELEASE)
//    {
//        velocity += forward * (moveSpeed * dt);
//    }
//
//    if (glfwGetKey(m_pWindow, GLFW_KEY_S) > GLFW_RELEASE)
//    {
//        velocity -= forward * (moveSpeed * dt);
//    }
//
//    if (glfwGetKey(m_pWindow, GLFW_KEY_D) > GLFW_RELEASE)
//    {
//        velocity += right * (moveSpeed * dt);
//    }
//
//    if (glfwGetKey(m_pWindow, GLFW_KEY_A) > GLFW_RELEASE)
//    {
//        velocity -= right * (moveSpeed * dt);
//    }
//
//    position += velocity;
}

