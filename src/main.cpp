#include <iostream>
#include <cstdio>
#include <string>
#include <cmath>

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "glad/glad.h"
#include <GLFW/glfw3.h>

#include "fractals.h"
#include "version.h"
#include "window_title.h"
#include "shader_loader.h"

// UI parameters
constexpr float CONTROL_COL_WIDTH = 0.2f;
constexpr float RENDER_COL_WIDTH = 1.0f - CONTROL_COL_WIDTH;
const float zoom_sensitivity = 0.6f;
ImVec2 display_col_pos;
ImVec2 display_col_size;

// State variables
Fractal selected_fractal = Fractal::MANDELBROT;
ImVec2 fractal_pos = ImVec2(0.0, 0.0);
float fractal_zoom = 2.0;
ImVec2 last_mouse_pos;
bool rendering = false;

void renderFractal(const char* name);
void adjustFractalZoom(GLFWwindow* window, double xoffset, double y_offset);
void renderControlColumn();
void renderSelectedFractal();
void moveCursorPos(float delta_x, float delta_y);
void moveCursorScreenPos(float delta_x, float delta_y);
void checkCompileErrors(GLuint shader, std::string type);

int main()
{
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

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 460 core");
    glClearColor(0.45f, 0.55f, 0.60f, 1.00f);

    glfwSetScrollCallback(window, adjustFractalZoom);

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        // Handle drag
        ImVec2 mousePos = ImGui::GetMousePos();
        if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
        {
            if (mousePos.x > display_w * CONTROL_COL_WIDTH)
            {
                fractal_pos.x -= (0.0005 * fractal_zoom) * (mousePos.x - last_mouse_pos.x);
                fractal_pos.y += (0.0005 * fractal_zoom) * (mousePos.y - last_mouse_pos.y);
            }
        }
        last_mouse_pos = mousePos;

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        glClear(GL_COLOR_BUFFER_BIT);

        ImGuiWindowFlags main_window_flags = 0;
        main_window_flags |= ImGuiWindowFlags_NoTitleBar;
        main_window_flags |= ImGuiWindowFlags_NoResize;
        main_window_flags |= ImGuiWindowFlags_NoMove;
        main_window_flags |= ImGuiWindowFlags_NoCollapse;
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2((float)display_w, (float)display_h));

        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowPadding = ImVec2(10, 10);

        // Begin full window
        ImGui::Begin("fullwindow", nullptr, main_window_flags);

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

void renderFractal(const char* name)
{
    // Shader stuff
    //
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    // Load and compile vertex shader
    char vertex_shader_name[64];
    std::sprintf(vertex_shader_name, "../shaders/%s/vertex_shader.glsl", name);
    std::string vertex_shader_source_string = loadShaderSource(vertex_shader_name);
    const char* vertex_shader_source = vertex_shader_source_string.c_str();
    glShaderSource(vertex_shader, 1, &vertex_shader_source, NULL);
    glCompileShader(vertex_shader);
    checkCompileErrors(vertex_shader, "VERTEX");

    // Load and compile fragment shader
    char fragmentShaderName[64];
    std::sprintf(fragmentShaderName, "../shaders/%s/fragment_shader.glsl", name);
    std::string fragmentShaderSourceString = loadShaderSource(fragmentShaderName);
    const char* fragmentShaderSource = fragmentShaderSourceString.c_str();
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    checkCompileErrors(fragmentShader, "FRAGMENT");

    // Link shaders to program
    GLuint shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragmentShader);
    glLinkProgram(shader_program);
    checkCompileErrors(shader_program, "PROGRAM");

    // Clean up shaders; they're linked to the program now
    glDeleteShader(vertex_shader);
    glDeleteShader(fragmentShader);

    float vertices[] = { // goofy
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
    int bottom_padding = ImGui::GetStyle().WindowPadding.y + ImGui::CalcTextSize("Display").y;
    ImVec2 render_size = ImVec2(display_col_size.x - 2 * padding,
                               display_col_size.y - bottom_padding);
    moveCursorScreenPos(padding, 0);
    ImVec2 cursor_pos = ImGui::GetCursorScreenPos();
    ImGui::InvisibleButton("##empty", render_size);

    glViewport(cursor_pos.x, cursor_pos.y, render_size.x, render_size.y);
    glEnable(GL_SCISSOR_TEST);
    glScissor(cursor_pos.x, cursor_pos.y, render_size.x, render_size.y);

    // Use the shader program and update uniforms
    glUseProgram(shader_program);
    glUniform2f(glGetUniformLocation(shader_program, "u_resolution"), render_size.x, render_size.y);
    glUniform2f(glGetUniformLocation(shader_program, "u_center"), fractal_pos.x, fractal_pos.y);
    glUniform1f(glGetUniformLocation(shader_program, "u_zoom"), fractal_zoom);

    // Draw quad
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glDisable(GL_SCISSOR_TEST);
}

void adjustFractalZoom(GLFWwindow* window, double xoffset, double y_offset)
{
    if (y_offset == 1)
        fractal_zoom *= zoom_sensitivity; // Zoom in
    else if (y_offset == -1)
        fractal_zoom *= (1 + zoom_sensitivity); // Zoom out
}

void renderControlColumn()
{
    ImGui::Text("Control");

    ImGuiWindowFlags fractal_select_flags = 0;
    fractal_select_flags |= ImGuiWindowFlags_NoTitleBar;
    fractal_select_flags |= ImGuiWindowFlags_NoMove;
    fractal_select_flags |= ImGuiWindowFlags_NoResize;
    fractal_select_flags |= ImGuiWindowFlags_NoCollapse;

    moveCursorPos(0, 20);
    if (ImGui::BeginMenu("Fractals"))
    {
        for (const auto& pair : FRACTALS)
            if (ImGui::MenuItem(pair.second)) // fractal name
                selected_fractal = pair.first; // fractal enum

        ImGui::EndMenu();
    }

    moveCursorPos(10, 10);
    ImGui::Text("Selected: %s", FRACTALS.at(selected_fractal));

    moveCursorPos(0, 20);
    float button_width = ImGui::CalcTextSize("Render").x + ImGui::GetStyle().FramePadding.x * 2;
    float offset_x = (ImGui::GetColumnWidth() - button_width) * 0.5;
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset_x);
    if (ImGui::Button("Render"))
    {
        rendering = true;
        renderSelectedFractal();
    }
}

void renderSelectedFractal()
{
    switch (selected_fractal)
    {
        case Fractal::MANDELBROT:
            renderFractal("mandelbrot");
            break;
        default:
            renderFractal("mandelbrot");
            break;
    }
}

void moveCursorPos(float delta_x, float delta_y)
{
    auto cursor_pos = ImGui::GetCursorPos();
    cursor_pos.x += delta_x;
    cursor_pos.y += delta_y;
    ImGui::SetCursorPos(cursor_pos);
}

void moveCursorScreenPos(float delta_x, float delta_y)
{
    auto cursor_pos = ImGui::GetCursorScreenPos();
    cursor_pos.x += delta_x;
    cursor_pos.y += delta_y;
    ImGui::SetCursorScreenPos(cursor_pos);
}

void checkCompileErrors(GLuint shader, std::string type)
{
    GLint success;
    GLchar info_log[1024];
    if (type != "PROGRAM")
    {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(shader, 1024, NULL, info_log);
            std::cerr << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << info_log << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
    else
    {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(shader, 1024, NULL, info_log);
            std::cerr << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << info_log << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
}
