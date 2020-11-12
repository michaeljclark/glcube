cmake_minimum_required (VERSION 3.16)

project(glcube VERSION 0.0.1)

add_executable(gl2_cube src/gl2_cube.c)
target_link_libraries(gl2_cube GL m glfw)
