@echo off
REM Compiles the program without the C standard library
REM -ftime-report    /* To see compile/link time statistic */
REM -fsanitize=undefined -fsanitize-trap

set DEF_COMPILER_FLAGS=-mconsole -march=native -mtune=native -std=c89 -pedantic -nodefaultlibs -nostdlib -mno-stack-arg-probe -Xlinker /STACK:0x100000,0x100000 ^
-fno-builtin -ffreestanding -fno-asynchronous-unwind-tables -fuse-ld=lld ^
-Wall -Wextra -Werror -Wvla -Wconversion -Wdouble-promotion -Wno-sign-conversion -Wno-missing-field-initializers -Wno-uninitialized -Winit-self -Wunused -Wunused-function -Wunused-macros -Wunused-parameter -Wunused-value -Wunused-variable -Wunused-local-typedefs

set DEF_FLAGS_LINKER=-lkernel32 -luser32 -lgdi32 -lopengl32

cc -s -Os -shared %DEF_COMPILER_FLAGS% speg.c -o speg.dll
cc -s -Os %DEF_COMPILER_FLAGS% -std=c99 w32_gl_nostdlib.c -o w32_gl_nostdlib.exe %DEF_FLAGS_LINKER%
w32_gl_nostdlib.exe
