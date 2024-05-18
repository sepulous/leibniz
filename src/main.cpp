#include <GLFW/glfw3.h>
#include <iostream>
#include "AL/al.h"
#include "AL/alc.h"

int main()
{
    if (!glfwInit())
    {
        std::cout << "FAIL\n";
        glfwTerminate();
    }

    GLFWwindow* window = glfwCreateWindow(640, 480, "Title", NULL, NULL);
    if (!window)
    {
        std::cout << "FAIL\n";
        glfwTerminate();
    }

    glfwMakeContextCurrent(window);

    ALCdevice *alcDevice = alcOpenDevice(NULL);
    ALCcontext *alcContext;

    if (alcDevice)
    {
        alcContext = alcCreateContext(alcDevice, NULL);
        alcMakeContextCurrent(alcContext);
    }

    // TODO: Get sound to play

    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);

        glfwSwapBuffers(window);

        glfwPollEvents();
    }

    glfwDestroyWindow(window);

    glfwTerminate();

    return 0;
}
