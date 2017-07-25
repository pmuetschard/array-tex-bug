cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)
project(array-tex-bug)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(glfw)

include_directories(glfw/include glad/include)
add_executable(array-tex-bug main.cpp glad/src/glad.c)
target_link_libraries(array-tex-bug glfw)
