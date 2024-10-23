//
//  BrushTool.h
//  TryOpenGL
//
//  Created by Fedor Artemenkov on 22.10.24.
//

#pragma once

#include <glm/glm.hpp>
#include "Cube.h"

class GLFWwindow;
class Camera;

class BrushTool
{
public:
    BrushTool(GLFWwindow* window, Camera* camera)
    : window(window), camera(camera)
    { };
    
    void init();
    void update();
    
    void draw(const Camera& camera) const;
    
private:
    GLFWwindow* window;
    Camera* camera;
    
    Cube cube;
    
    glm::vec3 startPoint;
    
    enum class DrawingState {
        IDLE,
        MAKE_PLANE,
        MAKE_CUBE,
    };
    
    DrawingState state = DrawingState::IDLE;
    
private:
    bool intersection(glm::vec3 &point);
};
