# gl2_cube

_gl2_cube_ is OpenGL cube demo in C11 using programmable shaders. The
demo uses the _linmath.h_ and _gl2_util.h_ headers which contain a simple
vertex and index buffer abstraction, functions for loading and compiling
shaders plus basic mouse navigation with pan and zoom.

![glcube](/images/glcube.png)

## Build Instructions

```
sudo apt-get install -y cmake ninja-build
cmake -G Ninja -B build .
cmake --build build
```
