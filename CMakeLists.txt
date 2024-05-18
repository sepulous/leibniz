CMAKE_MINIMUM_REQUIRED(VERSION 3.7)
PROJECT(leibniz)

SET(CMAKE_CXX_STANDARD 14)
SET(CMAKE_BUILD_TYPE DEBUG)

SET(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
SET(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
SET(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
SET(OpenGL_GL_PREFERENCE GLVND)

ADD_SUBDIRECTORY(/home/blake/glfw /home/blake/glfw/src)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/imgui)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/imgui/backends)

FIND_PACKAGE(OpenGL REQUIRED)

SET(SOURCE_FILES
    src/main.cpp
    imgui/imgui.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/backends/imgui_impl_glfw.cpp
    imgui/backends/imgui_impl_opengl3.cpp
)

ADD_EXECUTABLE(leibniz ${SOURCE_FILES})
TARGET_LINK_LIBRARIES(leibniz glfw)
TARGET_LINK_LIBRARIES(leibniz OpenGL::GL)

