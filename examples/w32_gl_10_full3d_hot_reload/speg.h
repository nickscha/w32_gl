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
    char id[20];

    bool initialized;
    bool faceCulling;

    /* Required for setup*/
    float *vertices;
    long verticesSize;

    unsigned int *indices;
    long indicesSize;

    float *uvs;
    long uvsSize;

    /* Required for drawing*/
    int indicesCount;

    /*Filled by platform */
    unsigned int VAO;
    unsigned int VBO;
    unsigned int EBO;
    unsigned int IBO;
    unsigned int CBO;
    unsigned int UBO; /* UV's */
    unsigned int TBO; /* Texture Index */

} speg_mesh;

typedef struct speg_draw_call
{
    speg_mesh *mesh;
    float *models;
    float *colors;
    int *texture_indices;
    int count_instances;
    int count_instances_max;

    int changed;
    int is_2d;

} speg_draw_call;

/*****************************/
/* Platform provided methods */
/*****************************/
typedef void (*func_speg_platform_print_console)(const char *file, const int line, char *formatString, ...);
typedef void (*func_speg_platform_format_string)(char *buffer, char *formatString, ...);
typedef void (*func_speg_platform_sleep)(unsigned long milliseconds);
typedef void (*func_speg_platform_draw)(speg_draw_call *draw_call, float uniformProjectionView[16]);
typedef unsigned long (*func_speg_platform_perf_current_cycle_count)(void);
typedef double (*func_platform_perf_current_time_nanoseconds)(void);

typedef struct speg_platform_api
{
    /* General required functions */
    func_speg_platform_print_console platform_print_console;
    func_speg_platform_format_string platform_format_string;
    func_speg_platform_sleep platform_sleep;
    func_speg_platform_draw platform_draw;

    /* Performance & Debugging Metrics */
    func_speg_platform_perf_current_cycle_count platform_perf_current_cycle_count;
    func_platform_perf_current_time_nanoseconds platform_perf_current_time_nanoseconds;

} speg_platform_api;

/********************************/
/* Application provided methods */
/********************************/
typedef struct platform_controller_state
{
    int halfTransitionCount;
    bool endedDown;
    bool active;
    bool pressed;
} platform_controller_state;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
typedef struct platform_controller_input
{

    /* ################ */
    /* # Keyboard input */
    /* ################ */
    union
    {
        platform_controller_state keys[63];
        struct
        {
            /* Mouse Keys */
            platform_controller_state key_mouse_left;
            platform_controller_state key_mouse_right;
            platform_controller_state key_mouse_middle;

            /* Function Keys */
            platform_controller_state key_f1;
            platform_controller_state key_f2;
            platform_controller_state key_f3;
            platform_controller_state key_f4;
            platform_controller_state key_f5;
            platform_controller_state key_f6;
            platform_controller_state key_f7;
            platform_controller_state key_f8;
            platform_controller_state key_f9;
            platform_controller_state key_f10;
            platform_controller_state key_f11;
            platform_controller_state key_f12;

            /* Special Keys */
            platform_controller_state key_backspace;
            platform_controller_state key_tab;
            platform_controller_state key_return;
            platform_controller_state key_shift;
            platform_controller_state key_control;
            platform_controller_state key_alt;
            platform_controller_state key_capital;
            platform_controller_state key_space;
            platform_controller_state key_arrow_left;
            platform_controller_state key_arrow_up;
            platform_controller_state key_arrow_right;
            platform_controller_state key_arrow_down;

            /* Numeric */
            platform_controller_state key_0;
            platform_controller_state key_1;
            platform_controller_state key_2;
            platform_controller_state key_3;
            platform_controller_state key_4;
            platform_controller_state key_5;
            platform_controller_state key_6;
            platform_controller_state key_7;
            platform_controller_state key_8;
            platform_controller_state key_9;

            /* Alphanumeric */
            platform_controller_state key_q;
            platform_controller_state key_w;

            platform_controller_state key_e;
            platform_controller_state key_r;
            platform_controller_state key_t;
            platform_controller_state key_z;
            platform_controller_state key_u;
            platform_controller_state key_i;
            platform_controller_state key_o;
            platform_controller_state key_p;
            platform_controller_state key_a;
            platform_controller_state key_s;
            platform_controller_state key_d;
            platform_controller_state key_f;
            platform_controller_state key_g;
            platform_controller_state key_h;
            platform_controller_state key_j;
            platform_controller_state key_k;
            platform_controller_state key_l;
            platform_controller_state key_y;
            platform_controller_state key_x;
            platform_controller_state key_c;
            platform_controller_state key_v;
            platform_controller_state key_b;
            platform_controller_state key_n;
            platform_controller_state key_m;

            /* All buttons must be added above this line */
            platform_controller_state terminator;
        };
    };

    /* ############# */
    /* # Mouse input */
    /* ############# */
    bool mouse_attached;
    float mouse_offset_scroll;
    float mouse_offset_x;
    float mouse_offset_y;
    int mouse_position_x;
    int mouse_position_y;

} platform_controller_input;

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

    uint32_t transientMemorySize;
    void *transientMemory;

} speg_memory;

#ifdef SPEG_IMPORT
void speg_update_stub(speg_memory *memory, platform_controller_input *input, speg_platform_api *platformApi)
{
    (void)memory;
    (void)input;
    (void)platformApi;
}
typedef void (*func_speg_update)(speg_memory *memory, platform_controller_input *input, speg_platform_api *platformApi);
static func_speg_update speg_update = speg_update_stub;
#else
void speg_update(speg_memory *memory, platform_controller_input *input, speg_platform_api *platformApi);
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
