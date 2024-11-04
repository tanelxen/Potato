//
// Created by Fedor Artemenkov on 05.07.2024.
//

#include "Camera.h"
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

#define GLM_ENABLE_EXPERIMENTAL 1
#include <glm/gtx/string_cast.hpp>

struct Transform
{
    glm::vec3 position;
    glm::vec3 rotation;
};

Camera::Camera(GLFWwindow* window) : window(window)
{

}

Camera::~Camera()
{

}



void Camera::updateViewport(float width, float height)
{
    float ratio = width / height;
    float fov = glm::radians(38.0f);
    projection = glm::perspective(fov, ratio, 0.1f, 4096.0f);
}

Ray Camera::getMousePosInWorld() const
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
    
    glm::vec3 origin = glm::vec3(nearWorld);
    
    glm::vec3 dir = glm::vec3(farWorld - nearWorld);
    dir = glm::normalize(dir);
    
    return {.origin = origin, .dir = dir};
}

void Camera::setTransform(const glm::vec3 &position, const glm::vec3 &forward, const glm::vec3 &right, const glm::vec3 &up)
{
    view = glm::lookAt(position, position + forward, up);
}

