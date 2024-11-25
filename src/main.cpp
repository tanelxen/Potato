#include <iostream>
#include <string>
#include <stdlib.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Shader.h"
#include "Camera.h"

#include "Q3MapScene.h"

static void error_callback(int e, const char *d) { printf("Error %d: %s\n", e, d); }

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

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
    
//    imgui_init(window);
    
    Camera camera(window);
    
    Q3MapScene scene(window, &camera);
    scene.loadMap("assets/maps/level.bsp");

    double prevTime = 0;
    double deltaTime;

    glfwSwapInterval(0);

    glFrontFace(GL_CCW);
    
    glEnable(GL_FRAMEBUFFER_SRGB);

    while (!glfwWindowShouldClose(window))
    {
        if (!animateNextFrame(60)) continue;
        
        double currTime = glfwGetTime();
        deltaTime = currTime - prevTime;
        prevTime = currTime;

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        camera.updateViewport((float)width, (float)height);
        scene.update(deltaTime);
        
        glViewport(0, 0, width, height);
        
        scene.draw();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
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
