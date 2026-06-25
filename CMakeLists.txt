cmake_minimum_required(VERSION 4.3)
project(OpenGLWindowApp LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.4
    GIT_SHALLOW    ON
)

set(GLFW_BUILD_DOCS OFF CACHE INTERNAL "")
set(GLFW_BUILD_TESTS OFF CACHE INTERNAL "")
set(GLFW_BUILD_EXAMPLES OFF CACHE INTERNAL "")
FetchContent_MakeAvailable(glfw)

add_library(glad STATIC lib/glad.c)
target_include_directories(glad PUBLIC include)

add_executable(blockgame
    src/main.cpp
)


target_link_libraries(blockgame PRIVATE glfw glad)