@echo off
REM Compiles the program without the C standard library
REM -ftime-report /* To see compile/link time statistic */

set NAME_PLATFORM_LAYER=w32_gl_nostdlib
set NAME_APPLICATION=speg

set DEF_COMPILER_FLAGS=-mconsole -march=native -mtune=native -std=c89 -pedantic -nodefaultlibs -nostdlib -mno-stack-arg-probe -Xlinker /STACK:0x100000,0x100000 ^
-fno-builtin -ffreestanding -fno-asynchronous-unwind-tables -fuse-ld=lld ^
-Wall -Wextra -Werror -Wvla -Wconversion -Wdouble-promotion -Wsign-conversion -Wuninitialized -Winit-self -Wunused -Wunused-function -Wunused-macros -Wunused-parameter -Wunused-value -Wunused-variable -Wunused-local-typedefs

set DEF_FLAGS_LINKER=-lkernel32 -luser32 -lgdi32 -lopengl32

cc -s -Os -shared %DEF_COMPILER_FLAGS% %NAME_APPLICATION%.c -o %NAME_APPLICATION%.dll
cc -s -Os %DEF_COMPILER_FLAGS% -std=c99 %NAME_PLATFORM_LAYER%.c -o %NAME_PLATFORM_LAYER%.exe %DEF_FLAGS_LINKER%
%NAME_PLATFORM_LAYER%.exe
