cmake_minimum_required (VERSION 3.16)

project(gl2_cube VERSION 0.0.1)

add_subdirectory(third_party/glad)
include_directories(${PROJECT_BINARY_DIR}/third_party/glad/include)

add_subdirectory(third_party/glfw)
include_directories(third_party/glfw/include)

add_executable(gl2_cube src/gl2_cube.c)
target_link_libraries(gl2_cube GL m glad glfw)
