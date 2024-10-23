//
//  BrushTool.cpp
//  TryOpenGL
//
//  Created by Fedor Artemenkov on 22.10.24.
//

#include <GLFW/glfw3.h>
#include "BrushTool.h"
#include "Camera.h"

void BrushTool::init()
{
    cube.init();
}

static int oldMouseButtonState = GLFW_RELEASE;
static float gridSize = 8;

void BrushTool::update()
{
    if (window == nullptr) return;
    if (camera == nullptr) return;
    
    int newMouseButtonState = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
    bool isClick = (newMouseButtonState == GLFW_RELEASE && oldMouseButtonState == GLFW_PRESS);
    bool isPress = newMouseButtonState == GLFW_PRESS;
    oldMouseButtonState = newMouseButtonState;
    
    glm::vec3 point;
    if (!intersection(point)) return;
    
    point = floor(point / gridSize) * gridSize;
    
    if (state == DrawingState::IDLE)
    {
        if (isClick)
        {
            cube.position = point;
            cube.scale = {8, 0, 8};
            
            startPoint = point;
            state = DrawingState::MAKE_PLANE;
        }
    }
    else if (state == DrawingState::MAKE_PLANE)
    {
        float x = fmin(startPoint.x, point.x);
//        float y = min(startPoint.y, point.y);
        float z = fmin(startPoint.z, point.z);
        
        float width = abs(startPoint.x - point.x) + gridSize;
        float depth = abs(startPoint.z - point.z) + gridSize;
        
        cube.position = {x, 0, z};
        cube.scale = {width, 0, depth};
        
        if (isClick)
        {
            cube.scale.y = gridSize;
            state = DrawingState::MAKE_CUBE;
        }
    }
    else if (state == DrawingState::MAKE_CUBE)
    {
        if (isClick)
        {
            state = DrawingState::IDLE;
        }
    }
}

void BrushTool::draw(const Camera &camera) const
{
    cube.draw(camera);
}

bool BrushTool::intersection(glm::vec3 &point)
{
    glm::vec3 planeNormal = {0, 1, 0};
    float planeDist = 0.0f;
    
    glm::vec3 rayOrigin;
    glm::vec3 rayDir;
    camera->getMousePosInWorld(rayOrigin, rayDir);
    
    float dotProduct = glm::dot(rayDir, planeNormal);
    
    if (abs(dotProduct) < 0.000001) return false;
    
    float t = (planeDist - glm::dot(rayOrigin, planeNormal)) / dotProduct;
    
    if (t > 0)
    {
        point = rayOrigin + rayDir * t;
        return true;
    }
    
    return false;
}
