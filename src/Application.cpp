//
//  Application.cpp
//  TryOpenGL
//
//  Created by Fedor Artemenkov on 30.11.24.
//

#include <stdio.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Q3MapScene.h"
#include "Camera.h"

#include "Input.h"
#include "HUD.h"

#include "Application.h"


static void error_callback(int e, const char *d) { printf("Error %d: %s\n", e, d); }

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

Application::Application()
{
    /* GLFW */
    glfwSetErrorCallback(error_callback);

    if (!glfwInit()) {
        fprintf(stdout, "[GFLW] failed to init!\n");
        exit(1);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GL_TRUE);
#endif

    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
//    glfwWindowHint(GLFW_SAMPLES, 8);

    const int window_width = 1440;
    const int window_height = 900;
    m_pWindow = glfwCreateWindow(window_width, window_height, "Demo", nullptr, nullptr);

    glfwSetKeyCallback(m_pWindow, key_callback);

    glfwMakeContextCurrent(m_pWindow);

    gladLoadGL();

    const unsigned char* version = glGetString(GL_VERSION);
    printf("version: %s\n", version);
    
    const unsigned char* device = glGetString(GL_RENDERER);
    printf("device: %s\n", device);
    
    m_pCamera = new Camera();
    m_pScene = new Q3MapScene(m_pCamera);
    
    glfwSwapInterval(0);
    glFrontFace(GL_CCW);
}

Application::~Application()
{
    delete m_pScene;
    delete m_pCamera;
    
    glfwDestroyWindow(m_pWindow);
    glfwTerminate();
}

void Application::run()
{
    HUD hud;
    hud.init();
    
    double lastTime = 0;
    const double desiredFrameTime = 1.0 / 60;
    
    glfwSetInputMode(m_pWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    while (!glfwWindowShouldClose(m_pWindow))
    {
        double currTime = glfwGetTime();
        double elapsedTime = currTime - lastTime;
        
        if (elapsedTime < desiredFrameTime) continue;
        
        lastTime = currTime;

        int width, height;
        glfwGetFramebufferSize(m_pWindow, &width, &height);

        m_pCamera->setAspectRatio((float)width / (float)height);
        m_pScene->update(elapsedTime);
        
        glViewport(0, 0, width, height);
        
        m_pScene->draw();
        
        hud.resize(width, height);
        hud.draw();

        glfwSwapBuffers(m_pWindow);
        
        glfwPollEvents();
        updateInputState();
    }
}


void Application::updateInputState()
{
    static bool isFirstFrame = true;
    
    for (int i = 32; i < 256; ++i)
    {
        Input::getInstance().prev_keys[i] = Input::getInstance().keys[i];
        Input::getInstance().keys[i] = glfwGetKey(m_pWindow, i) > GLFW_RELEASE;
    }
    
    Input::getInstance().m_prevLeftMouseButtonState = Input::getInstance().m_currLeftMouseButtonState;
    Input::getInstance().m_prevRightMouseButtonState = Input::getInstance().m_currRightMouseButtonState;
    
    Input::getInstance().m_currLeftMouseButtonState = glfwGetMouseButton(m_pWindow, GLFW_MOUSE_BUTTON_LEFT) > GLFW_RELEASE;
    Input::getInstance().m_currRightMouseButtonState = glfwGetMouseButton(m_pWindow, GLFW_MOUSE_BUTTON_RIGHT) > GLFW_RELEASE;
    
    
    
    double mouseX, mouseY;
    glfwGetCursorPos(m_pWindow, &mouseX, &mouseY);
    
    if (isFirstFrame)
    {
        Input::getInstance().m_mouseOffsetX = 0;
        Input::getInstance().m_mouseOffsetY = 0;
    }
    else
    {
        Input::getInstance().m_mouseOffsetX = mouseX - Input::getInstance().m_mouseX;
        Input::getInstance().m_mouseOffsetY = mouseY - Input::getInstance().m_mouseY;
    }
    
    Input::getInstance().m_mouseX = mouseX;
    Input::getInstance().m_mouseY = mouseY;
    
    isFirstFrame = false;
}
