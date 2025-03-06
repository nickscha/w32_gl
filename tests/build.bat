@echo off

set DEF_FLAGS_COMPILER=-std=c99 -pedantic -Wall -Wextra -Werror -Wvla -Wconversion -Wdouble-promotion -Wsign-conversion -Wuninitialized -Winit-self -Wunused -Wunused-macros -Wunused-local-typedefs
set DEF_FLAGS_LINKER=
set SOURCE_NAME=w32_gl_test

cc -s -O2 -ftime-report %DEF_FLAGS_COMPILER% -o %SOURCE_NAME%.exe %SOURCE_NAME%.c %DEF_FLAGS_LINKER%
%SOURCE_NAME%.exe
