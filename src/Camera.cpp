//
// Created by Fedor Artemenkov on 05.07.2024.
//

#include "Camera.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

#define GLM_ENABLE_EXPERIMENTAL 1
#include <glm/gtx/string_cast.hpp>

Camera::Camera(GLFWwindow* window) : window(window)
{

}

Camera::~Camera()
{

}

static int oldState = GLFW_RELEASE;

void Camera::update(float dt)
{
    if (isFirstFrame)
    {
        glfwGetCursorPos(window, &prevMouseX, &prevMouseY);
        isFirstFrame = false;
        return;
    }

    double curMouseX = 0;
    double curMouseY = 0;
    glfwGetCursorPos(window, &curMouseX, &curMouseY);

    double dx = curMouseX - prevMouseX;
    double dy = curMouseY - prevMouseY;

    prevMouseX = curMouseX;
    prevMouseY = curMouseY;

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2) > GLFW_RELEASE)
    {
        yaw += dx * mouseSense * dt;
        pitch -= dy * mouseSense * dt;

        if (pitch > 1.5) pitch = 1.5;
        else if (pitch < -1.5) pitch = -1.5;

        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
    else
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }

    forward.x = cos(yaw) * cos(pitch);
    forward.y = sin(pitch);
    forward.z = sin(yaw) * cos(pitch);
    forward = glm::normalize(forward);

    glm::vec3 unit_up = {0, 1, 0};

    right = glm::cross(forward, unit_up);
    right = glm::normalize(right);

    up = glm::cross(right, forward);
    up = glm::normalize(up);

    velocity = {0, 0, 0};

    if (glfwGetKey(window, GLFW_KEY_W) > GLFW_RELEASE)
    {
        velocity += forward * (moveSpeed * dt);
    }

    if (glfwGetKey(window, GLFW_KEY_S) > GLFW_RELEASE)
    {
        velocity -= forward * (moveSpeed * dt);
    }

    if (glfwGetKey(window, GLFW_KEY_D) > GLFW_RELEASE)
    {
        velocity += right * (moveSpeed * dt);
    }

    if (glfwGetKey(window, GLFW_KEY_A) > GLFW_RELEASE)
    {
        velocity -= right * (moveSpeed * dt);
    }

    position += velocity;

    view = glm::lookAt(position, position + forward, up);
    
    int newState = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
    if (newState == GLFW_RELEASE && oldState == GLFW_PRESS)
    {
        glm::vec3 origin = {0, 0, 0};
        glm::vec3 dir = {0, 0, 0};
        getMousePosInWorld(origin, dir);
        
        std::cout << glm::to_string(dir) << std::endl;
    }
    oldState = newState;
}

void Camera::updateViewport(float width, float height)
{
    float ratio = width / height;
    float fov = glm::radians(38.0f);
    projection = glm::perspective(fov, ratio, 0.1f, 4096.0f);
}

void Camera::getMousePosInWorld(glm::vec3 &origin, glm::vec3 &dir)
{
    double mouseX = 0;
    double mouseY = 0;
    glfwGetCursorPos(window, &mouseX, &mouseY);
    
    int screenWidth = 1;
    int screenHeight = 1;
    glfwGetWindowSize(window, &screenWidth, &screenHeight);
    
    double x_ndc = (2.0f * mouseX) / screenWidth - 1.0f;
    double y_ndc = 1.0f - (2.0f * mouseY) / screenHeight;
    
    glm::vec4 nearPoint = {x_ndc, y_ndc, -1.0f, 1.0f};
    glm::vec4 farPoint = {x_ndc, y_ndc, 1.0f, 1.0f};
    
    glm::mat4 viewProjInv = glm::inverse(projection * view);
    
    glm::vec4 nearWorld = viewProjInv * nearPoint;
    glm::vec4 farWorld = viewProjInv * farPoint;
    
    nearWorld /= nearWorld.w;
    farWorld /= farWorld.w;
    
    origin = glm::vec3(nearWorld);
    
    dir = glm::vec3(farWorld - nearWorld);
    dir = glm::normalize(dir);
}
