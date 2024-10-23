//
// Created by Fedor Artemenkov on 05.07.2024.
//

#pragma once
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

struct Ray
{
    glm::vec3 origin;
    glm::vec3 dir;
};

class Camera
{
public:
    Camera(GLFWwindow* window);
    ~Camera();

    void updateViewport(float width, float height);
    void update(float dt);
    
    void getMousePosInWorld(glm::vec3 &origin, glm::vec3 &dir);

    glm::mat4x4 projection;
    glm::mat4x4 view;

private:
    glm::vec3 position = {0, 128, 256};
    float pitch = -0.5;
    float yaw = -1.57;

    float moveSpeed = 96;
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
