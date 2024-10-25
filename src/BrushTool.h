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
class World;

class BrushTool
{
public:
    BrushTool(GLFWwindow* window, Camera* camera, World* world)
    : window(window), camera(camera), world(world)
    { };
    
    void init();
    void update();
    
    void draw(const Camera& camera) const;
    
private:
    GLFWwindow* window;
    Camera* camera;
    World* world;
    
    WiredCube wiredCube;
    
    enum class DrawingState {
        IDLE,
        MAKE_PLANE,
        MAKE_CUBE,
    };
    
    DrawingState state = DrawingState::IDLE;
};
