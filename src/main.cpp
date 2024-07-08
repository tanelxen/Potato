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

//struct Vertex
//{
//    glm::vec3 pos;
//    glm::vec3 col;
//};
//
//static const Vertex vertices[3] = {
//        { { -0.6f, -0.4f, -2.0f }, { 1.f, 0.f, 0.f } },
//        { {  0.6f, -0.4f, -2.0f }, { 0.f, 1.f, 0.f } },
//        { {  0.0f,  0.6f, -2.0f }, { 0.f, 0.f, 1.f } }
//};

struct Vertex
{
    glm::vec3 pos;
    glm::vec2 tx;
    glm::vec2 lm;
};

static const Vertex vertices[4] = {
        { { 352.0f,  0.0f, -256.0f }, { 1.75f, -1.0f }, { 0.17f, 0.12f } },
        { {   0.0f,  0.0f,    0.0f }, { -1.0f,  1.0f }, { 0.0039f, 0.0039f } },
        { {   0.0f,  0.0f, -256.0f }, { -1.0f, -1.f }, { 0.0039f, 0.128f } },
        { { 352.0f,  0.0f,    0.0f }, { 1.75f,  1.f }, { 0.175f, 0.0039f } },

//        { { -0.5f, -0.5f, -2.0f }, { 0.0f, 1.0f }, { 0.0f, 0.0f } },
//        { { -0.5f,  0.5f, -2.0f }, { 0.0f, 0.0f }, { 0.0f, 0.0f } },
//        { {  0.5f,  0.5f, -2.0f }, { 1.0f, 0.0f }, { 0.0f, 0.0f } },
//        { {  0.5f, -0.5f, -2.0f }, { 1.0f, 1.0f }, { 0.0f, 0.0f } },

};

static const unsigned int indices[6] = {
        0, 1, 2, 3, 1, 0
//        0, 1, 2, 2, 3, 0
};

static void error_callback(int e, const char *d) { printf("Error %d: %s\n", e, d); }

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}


ImVec4 clear_color = ImColor(114, 144, 154);

void imgui_draw();

int main()
{
    /* Platform */
    static GLFWwindow *window;
    const int window_width = 640;
    const int window_height = 480;

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
    glfwWindowHint(GLFW_SAMPLES, 8);

    window = glfwCreateWindow(window_width, window_height, "Demo", nullptr, nullptr);

    glfwSetKeyCallback(window, key_callback);

    glfwMakeContextCurrent(window);
//    glfwSwapInterval(1);

    gladLoadGL();

    const unsigned char* version = glGetString(GL_VERSION);
    printf("version: %s\n", version);

    glEnable(GL_MULTISAMPLE);
    glEnable(GL_DEPTH_TEST);

    VertexLayout layout;
    layout.attributes[0].offset = offsetof(Vertex, pos);
    layout.attributes[0].components = 3;
//    layout.attributes[1].offset = offsetof(Vertex, col);
//    layout.attributes[1].components = 3;
    layout.attributes[1].offset = offsetof(Vertex, tx);
    layout.attributes[1].components = 2;
    layout.attributes[2].offset = offsetof(Vertex, lm);
    layout.attributes[2].components = 2;
    layout.stride = sizeof(Vertex);

    VertexBuffer vertexBuffer;

    vertexBuffer.init(vertices, sizeof(vertices), indices, 6, layout);
//    vertexBuffer.init(vertices, sizeof(vertices), layout);

    Shader shader;
    shader.init("assets/shaders/basic.glsl");

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 150");

    setImGuiStyle();

    CQuake3BSP bsp;

    if (!bsp.LoadBSP("assets/maps/q3dm7.bsp")) {
        return 1;
    }

    bsp.GenerateTexture();
    bsp.GenerateLightmap();
    bsp.initBuffers();

    Camera camera(window);

    double prevTime = 0;
    double deltaTime;

    glfwSwapInterval(0);
    
    shader.bind();
    glUniform1i(glGetUniformLocation(shader.program, "s_bspTexture"), 0);
    glUniform1i(glGetUniformLocation(shader.program, "s_bspLightmap"), 1);

    while (!glfwWindowShouldClose(window))
    {
        double currTime = glfwGetTime();
        deltaTime = currTime - prevTime;
        prevTime = currTime;

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        camera.updateViewport((float)width, (float)height);
        camera.update(deltaTime);

        glClearColor(0.6, 0.8, 0.6, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4x4 mvp = camera.projection * camera.view;

        shader.bind();
        shader.setUniformMatrix((const float*) &mvp, "MVP");
        
        bsp.renderFaces();

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
