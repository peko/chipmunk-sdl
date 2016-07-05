#include <stdlib.h>
#include <stdio.h>

#include "glad/glad.h"
#include <GLFW/glfw3.h>

#define WINDOW_W 640
#define WINDOW_H 480

int main(int argc, char** argv) {

    GLFWwindow* window;

    // glfwSetErrorCallback(on_error);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    window = glfwCreateWindow(WINDOW_W, WINDOW_H, "show mesh", NULL, NULL);
    if (!window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    // glfwSetKeyCallback(window, on_key);
    // glfwSetCursorPosCallback(window, on_mouse);
    // glfwSetMouseButtonCallback(window, on_click);
    // glfwSetScrollCallback(window, on_scroll);

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwSwapInterval(1);

    // app_init(argc, argv);
    // gui_init(window);

    while (!glfwWindowShouldClose(window)) {
        float ratio;
        int width, height;

        glfwGetFramebufferSize(window, &width, &height);
        ratio = width / (float) height;

        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);

        // app_draw(ratio);
        
        glfwPollEvents();
        // gui_logic();
        // gui_draw();


        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);

    glfwTerminate();

    // app_cleanup();
    // gui_cleanup();

    exit(EXIT_SUCCESS);
}
