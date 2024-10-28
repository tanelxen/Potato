#include <iostream>
#include <string>
#include <stdlib.h>
#include <sstream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

//#include <glm/gtx/io.hpp>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "imguiStyling.h"

#include "VertexBuffer.h"
#include "Shader.h"
#include "Camera.h"

#include "Quake3Bsp.h"
#include "BspScene.h"

#include "Grid.h"
#include "World.h"
#include "BrushTool.h"
#include "CubeGeometry.h"

#include "KeyValueCollection.h"

static void error_callback(int e, const char *d) { printf("Error %d: %s\n", e, d); }

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}


ImVec4 clear_color = ImColor(114, 144, 154);

void imgui_draw();

bool animateNextFrame(int desiredFrameRate);

int main()
{
    /* Platform */
    static GLFWwindow *window;
    const int window_width = 1440;
    const int window_height = 900;

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

    window = glfwCreateWindow(window_width, window_height, "Demo", nullptr, nullptr);

    glfwSetKeyCallback(window, key_callback);

    glfwMakeContextCurrent(window);

    gladLoadGL();

    const unsigned char* version = glGetString(GL_VERSION);
    printf("version: %s\n", version);
    
    const unsigned char* device = glGetString(GL_RENDERER);
    printf("device: %s\n", device);
    
    BspScene scene;
    Camera camera(window);
    
    {
        CQuake3BSP bsp;

        if (!bsp.initFromFile("assets/maps/level.bsp")) {
            return 1;
        }
        
        KeyValueCollection entities;
        entities.initFromString(bsp.m_pEntities);
        
        auto spawnPoints = entities.getAllWithKeyValue("classname", "info_player_deathmatch");
        
        if (spawnPoints.size() > 0)
        {
            auto first = spawnPoints[2];
            
            auto angleProperty = first.properties.find("angle");
            
            if (angleProperty != first.properties.end())
            {
                std::string valueStr = angleProperty->second;
                int value = atoi(valueStr.c_str());
                
                printf("angle = %i\n", value);
                
                camera.yaw = (-value) * 3.14f / 180.0f;
                camera.pitch = 0;
            }
            
            auto originProperty = first.properties.find("origin");
            
            if (originProperty != first.properties.end())
            {
                std::string origin = originProperty->second;
                
                std::istringstream stream(origin);

                glm::vec3 camera_pos = {0, 0, 0};
                stream >> camera_pos.x >> camera_pos.y >> camera_pos.z;
                
                camera.position.x = camera_pos.x;
                camera.position.y = camera_pos.z;
                camera.position.z = -camera_pos.y;
                
                printf("camera_pos = (%1.0f %1.0f %1.0f)\n", camera.position.x, camera.position.y, camera.position.z);
                
                camera.position.y += 60;
            }
        }
        
//        printf("num_entities = %i\n", num_entities);
        
        scene.initFromBsp(&bsp);
    }

    

    double prevTime = 0;
    double deltaTime;

    glfwSwapInterval(0);
    
    CubeGeometry::init();
    
    Grid grid;
    grid.init();
    
    World world;
    
    BrushTool tool = BrushTool(window, &camera, &world);
    tool.init();

    glEnable(GL_DEPTH_TEST);
    glFrontFace(GL_CCW);

    while (!glfwWindowShouldClose(window))
    {
        if (!animateNextFrame(60)) continue;
        
        double currTime = glfwGetTime();
        deltaTime = currTime - prevTime;
        prevTime = currTime;

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        camera.updateViewport((float)width, (float)height);
        camera.update(deltaTime);
        
//        tool.update();
        
        glViewport(0, 0, width, height);
        
        glClearColor(0.1, 0.1, 0.1, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        scene.renderFaces(&camera);
        
//        tool.draw(camera);
//        world.draw(camera);
        
//        glDisable(GL_CULL_FACE);
//        grid.draw(camera);

//        imgui_draw();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    CubeGeometry::deinit();

//    ImGui_ImplOpenGL3_Shutdown();
//    ImGui_ImplGlfw_Shutdown();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

void imgui_init(GLFWwindow* window)
{
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 150");

    setImGuiStyle();
}

void imgui_draw()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    {
        static float f = 0.0f;
        ImGui::Text("Hello, world!");
        ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
        ImGui::ColorEdit3("clear color", (float*)&clear_color);
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

bool animateNextFrame(int desiredFrameRate)
{
    static double lastTime = 0.0f;
    double elapsedTime = 0.0;

    // Get current time in seconds  (milliseconds * .001 = seconds)
    double currentTime = glfwGetTime();// * 0.001;

    // Get the elapsed time by subtracting the current time from the last time
    elapsedTime = currentTime - lastTime;

    // Check if the time since we last checked is over (1 second / framesPerSecond)
    if( elapsedTime > (1.0 / desiredFrameRate) )
    {
        // Reset the last time
        lastTime = currentTime;

        // Return TRUE, to animate the next frame of animation
        return true;
    }

    // We don't animate right now.
    return false;
}
