#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include "agents.h"
#include "renderer.h"

int main() {
    // init GLFW
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

    // init simulation
    initRenderer();
    initAgents();

    // render loop
    while (!glfwWindowShouldClose(window)) {
        clearPixels();

        updateAgents();

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