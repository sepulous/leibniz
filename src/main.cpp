#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "glad/glad.h"
#include <GLFW/glfw3.h>

#include <iostream>
#include <cstdio>
#include <string>
#include <cmath>
#include "fractals.h"
#include "version.h"
#include "window_title.h"

#include "loader.h"

// UI parameters
constexpr float CONTROL_COL_WIDTH = 0.2f;
constexpr float RENDER_COL_WIDTH = 1.0f - CONTROL_COL_WIDTH;
const float zoomSensitivity = 0.6f;
ImVec2 display_col_pos;
ImVec2 display_col_size;

// State variables
Fractal selectedFractal = Fractal::MANDELBROT;
float fractalPosX = 0;
float fractalPosY = 0;
float fractalZoom = 2;
ImVec2 lastMousePos;
bool rendering = false;

void moveCursorPos(float deltaX, float deltaY)
{
    auto cursorPos = ImGui::GetCursorPos();
    cursorPos.x += deltaX;
    cursorPos.y += deltaY;
    ImGui::SetCursorPos(cursorPos);
}

void renderFractal(const char* name)
{
    // Shader stuff
    //
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    // Load and compile vertex shader
    char vertexShaderName[64];
    std::sprintf(vertexShaderName, "../shaders/%s/vertex_shader.glsl", name);
    std::string vertexShaderSourceString = loadShaderSource(vertexShaderName);
    const char* vertexShaderSource = vertexShaderSourceString.c_str();
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    checkCompileErrors(vertexShader, "VERTEX");

    // Load and compile fragment shader
    char fragmentShaderName[64];
    std::sprintf(fragmentShaderName, "../shaders/%s/fragment_shader.glsl", name);
    std::string fragmentShaderSourceString = loadShaderSource(fragmentShaderName);
    const char* fragmentShaderSource = fragmentShaderSourceString.c_str();
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    checkCompileErrors(fragmentShader, "FRAGMENT");

    // Link shaders into a program
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    checkCompileErrors(shaderProgram, "PROGRAM");

    // Clean up shaders as they are now linked into the program
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    float vertices[] = {
        -1.0f, -1.0f,
         1.0f, -1.0f,
        -1.0f,  1.0f,
         1.0f,  1.0f
    };

    GLuint VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    int padding = 20;
    int renderWidth = (int)(display_col_size.x - 2*padding);
    int renderHeight = (int)(display_col_size.y - 2*padding);
    int renderPosX = (int)(display_col_pos.x + padding);
    int renderPosY = (int)(display_col_pos.y + padding);

    ImGui::SetCursorScreenPos(ImVec2(renderPosX, renderPosY));
    ImGui::InvisibleButton("##empty", ImVec2(renderWidth, renderHeight));

    glViewport(renderPosX, renderPosY, renderWidth, renderHeight);
    glEnable(GL_SCISSOR_TEST);
    glScissor(renderPosX, renderPosY, renderWidth, renderHeight);

    // Use the shader program and update uniforms
    glUseProgram(shaderProgram);
    glUniform2f(glGetUniformLocation(shaderProgram, "u_resolution"), (float)renderWidth, (float)renderHeight);
    glUniform2f(glGetUniformLocation(shaderProgram, "u_center"), fractalPosX, fractalPosY);
    glUniform1f(glGetUniformLocation(shaderProgram, "u_zoom"), fractalZoom);

    // Draw quad
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glDisable(GL_SCISSOR_TEST);
}

void renderSelectedFractal()
{
    switch (selectedFractal)
    {
        case Fractal::MANDELBROT:
            renderFractal("mandelbrot");
            break;
        default:
            renderFractal("mandelbrot");
            break;
    }
}

void renderControlColumn()
{
    ImGui::Text("Control");

    ImGuiWindowFlags fractalSelectFlags = 0;
    fractalSelectFlags |= ImGuiWindowFlags_NoTitleBar;
    fractalSelectFlags |= ImGuiWindowFlags_NoMove;
    fractalSelectFlags |= ImGuiWindowFlags_NoResize;
    fractalSelectFlags |= ImGuiWindowFlags_NoCollapse;

    moveCursorPos(0, 20);
    if (ImGui::BeginMenu("Fractals"))
    {
        for (const auto& pair : FRACTALS)
            if (ImGui::MenuItem(pair.second)) // fractal name
                selectedFractal = pair.first; // fractal enum

        ImGui::EndMenu();
    }

    moveCursorPos(10, 10);
    ImGui::Text("Selected: %s", FRACTALS.at(selectedFractal));

    moveCursorPos(0, 20);
    float buttonWidth = ImGui::CalcTextSize("Render").x + ImGui::GetStyle().FramePadding.x * 2;
    float offsetX = (ImGui::GetColumnWidth() - buttonWidth) * 0.5;
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offsetX);
    if (ImGui::Button("Render"))
    {
        rendering = true;
        renderSelectedFractal();
    }
}

void adjustFractalZoom(GLFWwindow* window, double xoffset, double yOffset)
{
    if (yOffset == 1)
        fractalZoom *= zoomSensitivity; // Zoom in
    else if (yOffset == -1)
        fractalZoom *= (1 + zoomSensitivity); // Zoom out
}

int main()
{
    // Initialize GLFW
    if (!glfwInit())
        glfwTerminate();

    GLFWwindow *window = glfwCreateWindow(1280, 720, getWindowTitle().c_str(), NULL, NULL);
    if (!window)
        return 1;

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        return 1;

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    ImGui::StyleColorsDark();

    // Connect ImGui
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 460 core");

    glfwSetScrollCallback(window, adjustFractalZoom);

    glClearColor(0.45f, 0.55f, 0.60f, 1.00f);

    lastMousePos = ImGui::GetMousePos();

    // Loop
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        // Handle input
        ImVec2 mousePos = ImGui::GetMousePos();
        if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
        {
            if (mousePos.x > display_w * CONTROL_COL_WIDTH)
            {
                fractalPosX -= (0.0005 * fractalZoom) * (mousePos.x - lastMousePos.x);
                fractalPosY += (0.0005 * fractalZoom) * (mousePos.y - lastMousePos.y);
            }
        }
        lastMousePos = mousePos;

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        glClear(GL_COLOR_BUFFER_BIT);

        ImGuiWindowFlags mainWindowFlags = 0;
        mainWindowFlags |= ImGuiWindowFlags_NoTitleBar;
        mainWindowFlags |= ImGuiWindowFlags_NoResize;
        mainWindowFlags |= ImGuiWindowFlags_NoMove;
        mainWindowFlags |= ImGuiWindowFlags_NoCollapse;
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2((float)display_w, (float)display_h));

        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowPadding = ImVec2(10, 10);

        // Begin full window
        ImGui::Begin("fullwindow", nullptr, mainWindowFlags);

        ImGui::Columns(2, "columns", false);
        ImGui::SetColumnWidth(0, (float)display_w * CONTROL_COL_WIDTH); // First column
        ImGui::SetColumnWidth(1, (float)display_w * RENDER_COL_WIDTH); // Second column

        renderControlColumn();

        ImGui::NextColumn();

        ImGui::Text("Display");
        display_col_pos = ImGui::GetCursorScreenPos();
        display_col_size = ImGui::GetContentRegionAvail();

        if (rendering)
            renderSelectedFractal();

        ImGui::End(); // End full window

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);

        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

