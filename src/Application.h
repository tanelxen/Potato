//
//  Application.hpp
//  TryOpenGL
//
//  Created by Fedor Artemenkov on 30.11.24.
//

#pragma once

struct Application
{
    Application();
    ~Application();
    
    void run();
    
    static void setCursorEnabled(bool state);
    
private:
    struct GLFWwindow* m_pWindow;
    
    class Camera* m_pCamera;
    class SourceMapScene* m_pScene;
    
    void updateInputState();
    
    static Application* currentInstance;
};
