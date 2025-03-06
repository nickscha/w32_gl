# w32_gl (in development)
A C89 standard compliant, single header, nostdlib (no C Standard Library) win32 and opengl api function prototypes without using windows.h

For more information please look at the "w32_gl.h" file or take a look at the "examples" or "tests" folder.

## Quick Start

Download or clone w32_gl.h and include it in your project.

```C
#include "w32_gl.h"

int main() {

    return 0;
}
```

## Run Example: win32, opengl, nostdlib, freestsanding

In this repo you will find the "examples/w32_gl_nostdlib.c" with the corresponding "build.bat" file which
creates an executable only linked to "kernel32","user32","gdi32","opengl32" and is not using the C standard library and executes the program afterwards.

Features:
- **1 memory allocation**: 1 for everything. Just VirtualAlloc once. Arena/Bump Memory is passed to application code.
- **hot OpenGL shader reloading**: Just edit the GLSL files and the running program will update.
- **hot application code reloading**: Let the window stay open and edit/save/compile **speg.c** and see changes immediatly without restarting the program.
- **OpenGL Instanced Rendering**
- **Frustum Culling**: From my other linear algebra library **vm.h**

Files:
- **w32_gl_nostdlib.c**: The platform specific code (here win32) is defined here. build.bat produces **w32_gl_nostdlib.exe**
- **speg.h**: Shared header between the .exe and .dll (speg.c)
- **speg.c**: The application code/logic which is pure C89 without any linkings. build.bat produces **speg.dll**
- **test.vs,test.fs,test_instanced.vs**: The OpenGL GLSL shaders
 
 
![Example of a C89 nostdlib win32/opengl program](/examples/example.png)
