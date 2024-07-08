//
// Created by Fedor Artemenkov on 05.07.2024.
//

#pragma once
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

class Camera
{
public:
    Camera(GLFWwindow* window);
    ~Camera();

    void updateViewport(float width, float height);
    void update(float dt);

    glm::mat4x4 projection;
    glm::mat4x4 view;

private:
    glm::vec3 position = {0, 312, 0};
    float pitch = 0;
    float yaw = 0; // -1.57

    float moveSpeed = 64;
    float mouseSense = 0.3;

    glm::vec3 velocity = {0, 0, 0};

    double prevMouseX = 0;
    double prevMouseY = 0;
    bool isFirstFrame = true;

    glm::vec3 forward = {0, 0, 0};
    glm::vec3 right = {0, 0, 0};
    glm::vec3 up = {0, 0, 0};

    GLFWwindow* window;
};
