//
//  BrushTool.cpp
//  TryOpenGL
//
//  Created by Fedor Artemenkov on 22.10.24.
//

#include <GLFW/glfw3.h>
#include "BrushTool.h"
#include "Camera.h"
#include "World.h"

void BrushTool::init()
{
    wiredCube.init();
}

static int oldMouseButtonState = GLFW_RELEASE;
static float gridSize = 8;

glm::vec3 startPoint;
glm::vec3 startPoint2;

Plane plane = {.normal = {0, 1, 0}, .distance = 0};

glm::vec3 cubePosition;
glm::vec3 cubeScale;

bool intersection(const Ray &ray, const Plane& plane, glm::vec3 &point);

void BrushTool::update()
{
    if (window == nullptr) return;
    if (camera == nullptr) return;
    
    int newMouseButtonState = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
    bool isClick = (newMouseButtonState == GLFW_RELEASE && oldMouseButtonState == GLFW_PRESS);
    oldMouseButtonState = newMouseButtonState;
    
    Ray ray = camera->getMousePosInWorld();
    
    glm::vec3 point;
    if (!intersection(ray, plane, point)) return;
    
    point = floor(point / gridSize) * gridSize;
    
    if (state == DrawingState::IDLE)
    {
        if (isClick)
        {
            cubePosition = point;
            cubeScale = {8, 0, 8};
            
            startPoint = point;
            state = DrawingState::MAKE_PLANE;
        }
    }
    else if (state == DrawingState::MAKE_PLANE)
    {
        cubePosition.x = fmin(startPoint.x, point.x);
        cubePosition.y = 0;
        cubePosition.z = fmin(startPoint.z, point.z);
        
        cubeScale.x = abs(startPoint.x - point.x) + gridSize;
        cubeScale.y = 0;
        cubeScale.z = abs(startPoint.z - point.z) + gridSize;
        
        if (isClick)
        {
            startPoint2 = point;
            state = DrawingState::MAKE_CUBE;
        }
    }
    else if (state == DrawingState::MAKE_CUBE)
    {
        glm::vec3 normal = camera->getForward();
        float distance = glm::dot(normal, startPoint2);
        Plane plane2 = {.normal = normal, .distance = distance};
        
        if (!intersection(ray, plane2, point)) return;
        
        point = floor(point / gridSize) * gridSize;
        
        cubePosition.y = fmin(startPoint2.y, point.y);
        cubeScale.y = abs(startPoint2.y - point.y) + gridSize;
        
        if (isClick)
        {
            state = DrawingState::IDLE;
            
            if (world != nullptr)
            {
                world->addCube(cubePosition, cubeScale);
            }
        }
    }
    
    wiredCube.position = cubePosition;
    wiredCube.scale = cubeScale;
}

void BrushTool::draw(const Camera &camera) const
{
    if (state != DrawingState::IDLE)
    {
        wiredCube.draw(camera);
    }
}

bool intersection(const Ray &ray, const Plane& plane, glm::vec3 &point)
{
    float dotProduct = glm::dot(ray.dir, plane.normal);
    
    if (abs(dotProduct) < 0.000001) return false;
    
    float t = (plane.distance - glm::dot(ray.origin, plane.normal)) / dotProduct;
    
    if (t > 0)
    {
        point = ray.origin + ray.dir * t;
        return true;
    }
    
    return false;
}
