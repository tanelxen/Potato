//
//  Player.cpp
//  TryOpenGL
//
//  Created by Fedor Artemenkov on 03.11.24.
//

#include <GLFW/glfw3.h>

#include "Player.h"
#include "PlayerMovement.h"
#include "StudioRenderer.h"

#include "Input.h"

static const float kMouseSense = 0.15;

Player::Player(PlayerMovement *movement) : m_pMovement(movement)
{
    
}

void Player::update(float dt)
{
    if (m_pMovement == nullptr) return;

    yaw -= Input::getMouseOffsetX() * kMouseSense * dt;
    pitch += Input::getMouseOffsetY() * kMouseSense * dt;
    
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

    if (Input::isKeyPressed(KEY_W))
    {
        forwardmove += 1;
    }

    if (Input::isKeyPressed(KEY_S))
    {
        forwardmove -= 1;
    }

    if (Input::isKeyPressed(KEY_D))
    {
        rightmove += 1;
    }

    if (Input::isKeyPressed(KEY_A))
    {
        rightmove -= 1;
    }
    
    bool jump = Input::isKeyPressed(32);
    
    m_pMovement->setTransform(position, forward, right, up);
    m_pMovement->setInputMovement(forwardmove, rightmove, jump);
    m_pMovement->update(dt);
    
    position = m_pMovement->getPosition();
    
    m_pModelInstance->animator.update(dt);
    
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

