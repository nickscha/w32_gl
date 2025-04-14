#ifndef SPEG_H
#define SPEG_H

#ifdef _MSC_VER
#pragma function(memset)
#endif
void *memset(void *dest, int c, unsigned int count)
{
    char *bytes = (char *)dest;
    while (count--)
    {
        *bytes++ = (char)c;
    }
    return dest;
}

#ifdef _MSC_VER
#pragma function(memcpy)
#endif
void *memcpy(void *dest, const void *src, unsigned int count)
{
    char *dest8 = (char *)dest;
    const char *src8 = (const char *)src;
    while (count--)
    {
        *dest8++ = *src8++;
    }
    return dest;
}

#define array_size(x) (sizeof(x) / sizeof((x)[0]))

#define assert(expression)      \
    if (!(expression))          \
    {                           \
        *(volatile int *)0 = 0; \
    }

typedef int int32_t;
typedef unsigned char uint8_t;
typedef unsigned int uint32_t;
typedef char bool;
#define true 1
#define false 0

typedef struct speg_mesh
{
    bool initialized;
    bool faceCulling;

    /* Required for setup*/
    float *vertices;
    long verticesSize;

    unsigned int *indices;
    long indicesSize;

    /* Required for drawing*/
    int indicesCount;

    /*Filled by platform */
    unsigned int VAO;
    unsigned int VBO;
    unsigned int EBO;
    unsigned int IBO;
    unsigned int CBO;

    int lastInstancedNumberOfObjects;

} speg_mesh;

/*****************************/
/* Platform provided methods */
/*****************************/
typedef void (*func_speg_platform_print_console)(const char *file, const int line, char *formatString, ...);
typedef void (*func_speg_platform_sleep)(unsigned long milliseconds);
typedef void (*func_speg_platform_draw)(speg_mesh *mesh, int numberOfObjects, int changed, float models[], float colors[], float uniformProjectionView[16]);
typedef unsigned long (*func_speg_platform_perf_current_cycle_count)(void);

typedef struct speg_platform_api
{
    /* General required functions */
    func_speg_platform_print_console platform_print_console;
    func_speg_platform_sleep platform_sleep;
    func_speg_platform_draw platform_draw;

    /* Performance & Debugging Metrics */
    func_speg_platform_perf_current_cycle_count platform_perf_current_cycle_count;

} speg_platform_api;

/********************************/
/* Application provided methods */
/********************************/
typedef struct speg_controller_state
{
    int halfTransitionCount;
    bool endedDown;
} speg_controller_state;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
typedef struct speg_controller_input
{

    union
    {
        speg_controller_state keys[8];
        struct
        {
            speg_controller_state moveForward;
            speg_controller_state moveBackward;
            speg_controller_state moveLeft;
            speg_controller_state moveRight;

            speg_controller_state moveUp;
            speg_controller_state moveDown;

            speg_controller_state cameraSimulate;
            speg_controller_state cameraResetPosition;

            /* All buttons must be added above this line */
            speg_controller_state terminator;
        };
    };

    /* Mouse information*/
    bool mouseAttached;
    float mouseScrollOffset;
    float mouseXOffset;
    float mouseYOffset;

} speg_controller_input;
#pragma GCC diagnostic pop

typedef struct speg_state
{
    double dt;
    int width;
    int height;
    unsigned int renderedObjects;
    unsigned int culledObjects;
    float clearColorR;
    float clearColorG;
    float clearColorB;

} speg_state;

typedef struct speg_memory
{
    bool initialized;
    uint32_t permanentMemorySize;
    void *permanentMemory;
} speg_memory;

#ifdef SPEG_IMPORT
void speg_update_stub(speg_memory *memory, speg_controller_input *input, speg_platform_api *platformApi)
{
    (void)memory;
    (void)input;
    (void)platformApi;
}
typedef void (*func_speg_update)(speg_memory *memory, speg_controller_input *input, speg_platform_api *platformApi);
static func_speg_update speg_update = speg_update_stub;
#else
void speg_update(speg_memory *memory, speg_controller_input *input, speg_platform_api *platformApi);
#endif

#endif /* SPEG_H */

/*
   ------------------------------------------------------------------------------
   This software is available under 2 licenses -- choose whichever you prefer.
   ------------------------------------------------------------------------------
   ALTERNATIVE A - MIT License
   Copyright (c) 2025 nickscha
   Permission is hereby granted, free of charge, to any person obtaining a copy of
   this software and associated documentation files (the "Software"), to deal in
   the Software without restriction, including without limitation the rights to
   use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
   of the Software, and to permit persons to whom the Software is furnished to do
   so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in all
   copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
   ------------------------------------------------------------------------------
   ALTERNATIVE B - Public Domain (www.unlicense.org)
   This is free and unencumbered software released into the public domain.
   Anyone is free to copy, modify, publish, use, compile, sell, or distribute this
   software, either in source code form or as a compiled binary, for any purpose,
   commercial or non-commercial, and by any means.
   In jurisdictions that recognize copyright laws, the author or authors of this
   software dedicate any and all copyright interest in the software to the public
   domain. We make this dedication for the benefit of the public at large and to
   the detriment of our heirs and successors. We intend this dedication to be an
   overt act of relinquishment in perpetuity of all present and future rights to
   this software under copyright law.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
   ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
   WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
   ------------------------------------------------------------------------------
*/
