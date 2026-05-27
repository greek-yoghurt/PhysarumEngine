#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include "renderer.h"
#include "agents.h"

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Physarum Engine", NULL, NULL);
    if (!window) {
        std::cout << "Failed to create window" << std::endl;
        return -1;
    }

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    initRenderer();
    initAgents();

    while (!glfwWindowShouldClose(window)) {
        // decay and diffuse trails each frame
        decayAndDiffuseTrails(0.02f);

        // agents sense, turn, move, deposit
        updateAgents();
        for (int i = 0; i < NUM_AGENTS; i++) {
            depositTrail((int)agents[i].x, (int)agents[i].y, 1.0f);
        }

        // render trails with agent pixels on top
        trailToPixels();
        for (int i = 0; i < NUM_AGENTS; i++) {
            setPixel((int)agents[i].x, (int)agents[i].y);
        }

        renderPixels();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    cleanupRenderer();
    glfwTerminate();
    return 0;
}