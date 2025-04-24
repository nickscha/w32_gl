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

In this repo you will find the "examples/w32_gl_10_full3d_hot_reload/w32_gl_nostdlib.c" with the corresponding "build.bat" file which
creates an executable only linked to "kernel32","user32","gdi32","opengl32" and is not using the C standard library and executes the program afterwards.

Features:
- **1 memory allocation**: 1 for everything. Just VirtualAlloc once. Arena/Bump Memory is passed to application code.
- **hot OpenGL shader reloading**: Just edit the GLSL files and the running program will update.
- **hot application code reloading**: Let the window stay open and edit/save/compile **speg.c** and see changes immediatly without restarting the program.
- **OpenGL Instanced Rendering**: In the example scene we render ~20800 objects in 4 draw calls. On a NVIDIA RTX 4070 TI with around 9000 FPS (no vsync).
- **Bitmap Font Rendering**
- **Frustum Culling**: From my other linear algebra library **vm.h**

Files:
- **w32_gl_nostdlib.c**: The platform specific code (here win32) is defined here. build.bat produces **w32_gl_nostdlib.exe**
- **speg.h**: Shared header between the .exe and .dll (speg.c)
- **speg.c**: The application code/logic which is pure C89 without any linkings. build.bat produces **speg.dll**
- **test.vs,test.fs,test_instanced.vs**: The OpenGL GLSL shaders
 
![Example of a C89 nostdlib win32/opengl program](/examples/w32_gl_10_full3d_hot_reload/example.png)

## "nostdlib" Motivation & Purpose

nostdlib is a lightweight, minimalistic approach to C development that removes dependencies on the standard library. The motivation behind this project is to provide developers with greater control over their code by eliminating unnecessary overhead, reducing binary size, and enabling deployment in resource-constrained environments.

Many modern development environments rely heavily on the standard library, which, while convenient, introduces unnecessary bloat, security risks, and unpredictable dependencies. nostdlib aims to give developers fine-grained control over memory management, execution flow, and system calls by working directly with the underlying platform.

### Benefits

#### Minimal overhead
By removing the standard library, nostdlib significantly reduces runtime overhead, allowing for faster execution and smaller binary sizes.

#### Increased security
Standard libraries often include unnecessary functions that increase the attack surface of an application. nostdlib mitigates security risks by removing unused and potentially vulnerable components.

#### Reduced binary size
Without linking to the standard library, binaries are smaller, making them ideal for embedded systems, bootloaders, and operating systems where storage is limited.

#### Enhanced performance
Direct control over system calls and memory management leads to performance gains by eliminating abstraction layers imposed by standard libraries.

#### Better portability
By relying only on fundamental system interfaces, nostdlib allows for easier porting across different platforms without worrying about standard library availability.
