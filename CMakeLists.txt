cmake_minimum_required (VERSION 3.12)

project(gl2_cube VERSION 0.0.1)

add_subdirectory(third_party/glad)
include_directories(${PROJECT_BINARY_DIR}/third_party/glad/include)

add_subdirectory(third_party/glfw)
include_directories(third_party/glfw/include)

if(UNIX)
  find_library(OPENGL_LIBRARY GL)
endif()

if(WIN32)
  find_library(OPENGL_LIBRARY OpenGL32)
endif()

add_executable(gl2_cube src/gl2_cube.c)
target_link_libraries(gl2_cube ${OPENGL_LIBRARY} glad glfw)
