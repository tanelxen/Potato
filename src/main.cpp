#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "imguiStyling.h"

#include "VertexBuffer.h"
#include "Shader.h"
#include "Camera.h"

#include "Quake3Bsp.h"
#include "Grid.h"
#include "BrushTool.h"

#include <string>

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

    glEnable(GL_DEPTH_TEST);

//    Shader shader;
//    shader.init("assets/shaders/basic.glsl");

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 150");

    setImGuiStyle();

//    CQuake3BSP bsp;
//
//    if (!bsp.LoadBSP("assets/maps/level.bsp")) {
//        return 1;
//    }
//
//    bsp.GenerateTexture();
//    bsp.GenerateLightmap();
//    bsp.initBuffers();

    Camera camera(window);

    double prevTime = 0;
    double deltaTime;

    glfwSwapInterval(0);
    
//    shader.bind();
//    glUniform1i(glGetUniformLocation(shader.program, "s_bspTexture"), 0);
//    glUniform1i(glGetUniformLocation(shader.program, "s_bspLightmap"), 1);
    
    Grid grid;
    grid.init();
    
    BrushTool tool = BrushTool(window, &camera);
    tool.init();

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
        
        tool.update();
        
        glViewport(0, 0, width, height);
        
        glClearColor(0.1, 0.1, 0.1, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

//        shader.bind();
//        glm::mat4x4 mvp = camera.projection * camera.view;
//        shader.setUniformMatrix((const float*) &mvp, "MVP");
//        
//        bsp.renderFaces();
        
        tool.draw(camera);
        
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        grid.draw(camera);

        imgui_draw();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
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
