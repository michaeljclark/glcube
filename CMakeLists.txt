#            ███▌                     ▐█
#            ▀▀█▌                     ▐█
#              █▌                     ▐█
#    ▗██▙▐█    █▌      ▄███▄  █▌   ▐▌ ▐█▗██▙▖    ▄███▄
#   ▗█▛▀▜██    █▌     ▟█▛▀▀█▌ █▌   ▐▌ ▐██▀▀██   ▟█▛▀▜█▌
#   ▟█   ▜█    █▌     █▛      █▌   ▐▌ ▐█▘  ▝█▌ ▗█▘   ▐█
#   █▌   ▐█    █▌    ▐█       █▌   ▐▌ ▐█    █▌ ▐█▄▄▄▄▟█
#   █▌   ▐█    █▌    ▐█       █▌   ▐▌ ▐█    █▌ ▐███████
#   █▌   ▐█    █▌    ▐█       █▌   █▌ ▐█    █▌ ▐█
#   ▜█   ▟█    █▌     █▙      █▙  ▗█▌ ▐█▖  ▗█▌ ▝█▖    ▗
#   ▝█▙▄▟██    ▐█▄▄▖  ▜█▙▄▄█▌ ▐█▄▄█▜▌ ▐██▄▄██   ▜█▙▄▄▟█
#    ▝██▛▐█     ▜██▌   ▀███▀   ▜██▘▐▌ ▐█▝██▛▘    ▀███▀▘
#        ▐█
#    ▄  ▗▟▛
#    █████▘
#     ▀▀▀

cmake_minimum_required (VERSION 3.12)
project(glcube VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include(CheckLibraryExists)
check_library_exists(m sqrtf "" HAVE_LIB_M)
if (HAVE_LIB_M)
    set(EXTRA_LIBS ${EXTRA_LIBS} m)
endif (HAVE_LIB_M)

find_package(PkgConfig)
pkg_check_modules(GLFW3 glfw3)

# Find OpenGL library
include(FindOpenGL)

# set defaults for options
set (OPENGL_EXAMPLES_DEFAULT ${OpenGL_OpenGL_FOUND})

# user configurable options
option(OPENGL_EXAMPLES "Build OpenGL examples" ${OPENGL_EXAMPLES_DEFAULT})
option(EXTERNAL_GLFW "Use external GLFW project" ON)
option(EXTERNAL_GLAD "Use external GLAD project" ON)

message(STATUS "OPENGL_EXAMPLES = ${OPENGL_EXAMPLES}")
message(STATUS "EXTERNAL_GLFW = ${EXTERNAL_GLFW}")
message(STATUS "EXTERNAL_GLAD = ${EXTERNAL_GLAD}")

if(APPLE)
  find_library(COREFOUNDATION_LIBRARY CoreFoundation)
  find_library(IOKIT_LIBRARY IOKit)
  find_library(COCOA_LIBRARY Cocoa)
  list(APPEND GLFW_LIBS_ALL ${COREFOUNDATION_LIBRARY} ${IOKIT_LIBRARY} ${COCOA_LIBRARY})
endif()

# host cpu identification
TRY_RUN(
  cpuident_result
  cpuident_compile_result
  ${CMAKE_CURRENT_BINARY_DIR}/
  ${CMAKE_CURRENT_SOURCE_DIR}/src/cpuident.c
  COMPILE_OUTPUT_VARIABLE test_compile_output
  RUN_OUTPUT_VARIABLE cpuident_output
)
foreach(opt_name_value IN ITEMS ${cpuident_output})
  string(REGEX MATCH "^[^=]+" opt_name ${opt_name_value})
  string(REPLACE "${opt_name}=" "" opt_value ${opt_name_value})
  cmake_language(EVAL CODE "SET(${opt_name} ${opt_value})")
endforeach()

message("cpuident: x86_sse4_2:${x86_sse4_2} x86_avx:${x86_avx} x86_avx2:${x86_avx2} x86_avx512f:${x86_avx512f}")

include(CheckCXXCompilerFlag)
check_cxx_compiler_flag("-march=skylake-avx512" has_skylake_avx512 "int main() { return 0; }")

# Support for external glfw
if (EXTERNAL_GLFW)
    include(ExternalProject)
    ExternalProject_Add(
        GLFW
        GIT_REPOSITORY "https://github.com/glfw/glfw.git"
        SOURCE_DIR "${CMAKE_BINARY_DIR}/third_party/glfw3"
        STEP_TARGETS build
        EXCLUDE_FROM_ALL TRUE
        CMAKE_ARGS -DGLFW_BUILD_EXAMPLES=OFF -DGLFW_BUILD_TESTS=OFF
                   -DGLFW_BUILD_DOCS=OFF -DGLFW_INSTALL=OFF
    )
    set(GLFW_INCLUDE_DIRS "${CMAKE_BINARY_DIR}/third_party/glfw3/include")
    set(GLFW_LIBRARY_DIR "${CMAKE_BINARY_DIR}/GLFW-prefix/src/GLFW-build/src/")
    set(GLFW_LIBRARIES glfw3 pthread dl)
    include_directories(${GLFW_INCLUDE_DIRS})
    link_directories(${GLFW_LIBRARY_DIR})
    list(APPEND GLFW_LIBS_ALL ${GLFW_LIBRARIES} ${EXTRA_LIBS})
else ()
    add_definitions(${GLFW3_CFLAGS})
    list(APPEND GLFW_LIBS_ALL ${GLFW3_LDFLAGS} ${EXTRA_LIBS})
endif ()

# Support for external glad
if (EXTERNAL_GLAD)
    include(ExternalProject)
    ExternalProject_Add(
        GLAD
        GIT_REPOSITORY "https://github.com/Dav1dde/glad.git"
        SOURCE_DIR "${CMAKE_BINARY_DIR}/third_party/glad"
        STEP_TARGETS build
        EXCLUDE_FROM_ALL TRUE
        CMAKE_ARGS -DGLAD_EXPORT=OFF
    )
    set(GLAD_INCLUDE_DIRS "${CMAKE_BINARY_DIR}/GLAD-prefix/src/GLAD-build/include/")
    set(GLAD_LIBRARY_DIR "${CMAKE_BINARY_DIR}/GLAD-prefix/src/GLAD-build/")
    set(GLAD_LIBRARIES glad)
    include_directories(${GLAD_INCLUDE_DIRS})
    link_directories(${GLAD_LIBRARY_DIR})
    list(APPEND OPENGL_LOADER_LIBS ${GLAD_LIBRARIES})
else ()
    list(APPEND OPENGL_LOADER_LIBS ${OPENGL_opengl_LIBRARY})
endif ()

if (OPENGL_EXAMPLES)
    foreach(prog IN ITEMS gl2_cube gl3_cube gl4_cube)
        message("-- Adding: ${prog}")
        add_executable(${prog} src/${prog}.c)
        target_link_libraries(${prog} ${GLFW_LIBS_ALL} ${OPENGL_LOADER_LIBS})
        if (EXTERNAL_GLAD)
            target_compile_definitions(${prog} PRIVATE -DHAVE_GLAD)
        endif ()
        if (EXTERNAL_GLFW)
            add_dependencies(${prog} GLFW-build)
        endif ()
        if (EXTERNAL_GLAD)
            add_dependencies(${prog} GLAD-build)
        endif ()
    endforeach(prog)
endif (OPENGL_EXAMPLES)

add_executable(test_maj2 src/test_maj2.c)
target_link_libraries(test_maj2 ${EXTRA_LIBS})

add_executable(bench_maj2 src/bench_maj2.cc)

if(x86_avx512f AND has_skylake_avx512)
  set_source_files_properties(src/bench_maj2.cc PROPERTIES COMPILE_FLAGS "-O3 -march=skylake-avx512")
  set_source_files_properties(src/test_maj2.c PROPERTIES COMPILE_FLAGS "-O3 -march=skylake-avx512")
endif()
