# glcube

_glcube_ is an OpenGL demo written in C11 that renders a rotating cube
using programmable shaders. it depends on _gl2_nano_, a lightweight helper
library that provides a simple vertex and index buffer abstraction, along
with utilities for loading, compiling, and linking OpenGL shader programs.

![glcube](/images/glcube.png)

## Introduction

_glcube_ is an app skeleton using _gl2_nano_, a framework for creating
apps using the modern OpenGL and GLSL shader pipeline. this work is
intended to be used as a template for tiny OpenGL demos. The shader uses
the [maj2random](https://github.com/michaeljclark/maj2random) noise function
derived from _SHA-2_, combined with UVs to create surface noise.

## Project Structure

- `src/gl2_cube.c` - OpenGL 2.1 cube using the `gl2_nano.h` shader loader.
- `src/gl3_cube.c` - OpenGL 3.2 cube using the `gl2_nano.h` shader loader.
- `src/gl4_cube.c` - OpenGL 4.5 cube using the `gl2_nano.h` shader loader.
- `src/gl2_nano.h` - header functions for OpenGL buffers and shaders.
- `src/linmath.h` - public domain linear algebra header functions.

## Build Instructions

glcube has been tested on the following operating systems:

- Ubuntu 24.04 LTS
- FreeBSD 14.3
- Windows 11
- macOS 15

glcube requires the following dependencies:

- Microsoft Windows plus Visual Studio 2022 with CMake and C/C++.
- Apple macOS with XCode Developer Tools and CMake plus Ninja.
- Linux or FreeBSD with GCC or Clang and CMake plus Ninja.

```
cmake -B build -G Ninja
cmake --build build
```

## Examples

The project includes several versions of _glcube_ ported to multiple APIs.

### gl2_cube

_gl2_cube_ is a 3D cube renderer using OpenGL 2.x and GLSL shaders.

### gl3_cube

_gl3_cube_ is the same as _gl2_cube_ with the addition of vertex array
objects from OpenGL 3.x.

### gl4_cube

_gl4_cube_ is the same as _gl3_cube_ with the addition of uniform buffer
objects from OpenGL 4.x.
