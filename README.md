# glcube

_glcube_ is OpenGL cube demo in C11 using programmable shaders. The demo
uses the _linmath.h_ and _gl2_util.h_ headers which contain a simple
vertex and index buffer abstraction, functions for loading and compiling
shaders plus basic mouse navigation with pan and zoom.

![glcube](/images/glcube.png)

## Introduction

_glcube_ is an app skeleton using _gl2_util.h_, a nano framework for
creating apps using the modern OpenGL and GLSL shader pipeline. The
`CMakeLists.txt` is intended to be used as a template for tiny demos.

The shader uses a noise function derived from _SHA-2_, combined with
UVs to create deterministic surface noise.

## Project Structure

- `src/gl2_cube.c` - OpenGL 2.x cube using the `gl2_util.h` shader loader.
- `src/gl3_cube.c` - OpenGL 3.x cube using the `gl2_util.h` shader loader.
- `src/gl4_cube.c` - OpenGL 4.x cube using the `gl2_util.h` shader loader.
- `src/gl2_util.h` - header functions for OpenGL buffers and shaders.
- `src/linmath.h` - public domain linear algebra header functions.

## Build Instructions

```
sudo apt-get install -y cmake ninja-build
cmake -G Ninja -B build .
cmake --build build
```

## Examples

The project includes several versions of _glcube_ ported to multiple APIs.

### gl2_cube

_gl2_cube_ is a 3D cube renderer using OpenGL 2.x and GLSL shaders.

### gl3_cube

_gl3_cube_ is mostly the same as _gl2_cube_ with the addition of vertex
array objects.

### gl4_cube

_gl4_cube_ is mostly the same as _gl3_cube_ with the addition of uniform
buffer objects.