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

include(CheckLibraryExists)
check_library_exists(m sqrtf "" HAVE_LIB_M)
if (HAVE_LIB_M)
    set(EXTRA_LIBS ${EXTRA_LIBS} m)
endif (HAVE_LIB_M)

find_package(PkgConfig)
pkg_check_modules(GLFW3 REQUIRED glfw3)

# Find OpenGL library
include(FindOpenGL)

# set defaults for options
set (OPENGL_EXAMPLES_DEFAULT ON)
if (NOT OpenGL_OpenGL_FOUND)
    set (OPENGL_EXAMPLES_DEFAULT OFF)
endif ()

# user configurable options
option(OPENGL_EXAMPLES "Build OpenGL examples" ${OPENGL_EXAMPLES_DEFAULT})
option(EXTERNAL_GLFW "Use external GLFW project" ON)
option(EXTERNAL_GLAD "Use external GLAD project" ON)

message(STATUS "OPENGL_EXAMPLES = ${OPENGL_EXAMPLES}")
message(STATUS "EXTERNAL_GLFW = ${EXTERNAL_GLFW}")
message(STATUS "EXTERNAL_GLAD = ${EXTERNAL_GLAD}")

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
    set(GLFW_LIBS_ALL ${GLFW_LIBRARIES} ${EXTRA_LIBS})
else ()
    add_definitions(${GLFW3_CFLAGS})
    set(GLFW_LIBS_ALL ${GLFW3_LDFLAGS} ${EXTRA_LIBS})
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
    list(APPEND GLFW_LIBS_ALL ${GLAD_LIBRARIES})
else ()
    list(APPEND GLFW_LIBS_ALL ${OPENGL_opengl_LIBRARY})
endif ()

if (OPENGL_EXAMPLES)
    foreach(prog IN ITEMS gl2_cube gl3_cube gl4_cube)
        message("-- Adding: ${prog}")
        add_executable(${prog} src/${prog}.c)
        target_link_libraries(${prog} ${GLFW_LIBS_ALL})
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
