#include "speg.h"
#include "vm.h"

static float cube_vertices[] = {
    -0.5f, -0.5f, -0.5f, /* Bottom-left back */
    0.5f, -0.5f, -0.5f,  /* Bottom-right back */
    0.5f, 0.5f, -0.5f,   /* Top-right back */
    -0.5f, 0.5f, -0.5f,  /* Top-left back */

    -0.5f, -0.5f, 0.5f, /* Bottom-left front */
    0.5f, -0.5f, 0.5f,  /* Bottom-right front */
    0.5f, 0.5f, 0.5f,   /* Top-right front */
    -0.5f, 0.5f, 0.5f   /* Top-left front */
};

static unsigned int cube_indices[] = {
    /* Back face (counter-clockwise) */
    0, 3, 2,
    2, 1, 0,

    /* Front face (counter-clockwise) */
    4, 5, 6,
    6, 7, 4,

    /* Left face (counter-clockwise) */
    0, 4, 7,
    7, 3, 0,

    /* Right face (counter-clockwise) */
    1, 2, 6,
    6, 5, 1,

    /*  Bottom face (counter-clockwise)*/
    0, 1, 5,
    5, 4, 0,

    /* Top face (counter-clockwise) */
    3, 7, 6,
    6, 2, 3};

static float rectangle_vertices[] = {
    -0.5f, -0.5f, 0.0f, /* Bottom-left */
    0.5f, -0.5f, 0.0f,  /* Bottom-right */
    0.5f, 0.5f, 0.0f,   /* Top-right */
    -0.5f, 0.5f, 0.0f   /* Top-left */
};

static unsigned int rectangle_indices[] = {
    0, 1, 2, /* First triangle */
    2, 3, 0  /* Second triangle */
};

typedef struct camera
{
    v3 position;
    v3 front;
    v3 up;
    v3 right;
    v3 worldUp;

    float yaw;
    float pitch;
    float fov;

} camera;

void camera_update_vectors(camera *cam)
{
    float yawRadians = vm_radf(cam->yaw);
    float pitchRadians = vm_radf(cam->pitch);
    float pitchRadiansCos = vm_cosf(pitchRadians);

    cam->front.x = vm_cosf(yawRadians) * pitchRadiansCos;
    cam->front.y = vm_sinf(pitchRadians);
    cam->front.z = vm_sinf(yawRadians) * pitchRadiansCos;
    cam->front = vm_v3_normalize(cam->front);
    cam->right = vm_v3_normalize(vm_v3_cross(cam->front, cam->worldUp));
    cam->up = vm_v3_normalize(vm_v3_cross(cam->right, cam->front));
}

static camera camera_init(void)
{
    camera cam = {0};

    cam.position = vm_v3_zero;
    cam.front = vm_v3(0.0f, 0.0f, -1.0f);
    cam.up = vm_v3(0.0f, 1.0f, 0.0f);
    cam.worldUp = cam.up;
    cam.fov = 45.0f;
    cam.yaw = -90.0f;

    camera_update_vectors(&cam);

    return (cam);
}

void camera_update_movement(speg_controller_input *input, camera *cam, float movementSpeed)
{

    if (input->cameraResetPosition.endedDown)
    {
        *cam = camera_init();
        cam->position.z = 13.0f;
    }
    if (input->moveForward.endedDown)
    {
        cam->position = vm_v3_add(cam->position, vm_v3_mulf(cam->front, movementSpeed));
    }
    if (input->moveLeft.endedDown)
    {
        cam->position = vm_v3_sub(cam->position, vm_v3_mulf(vm_v3_normalize(vm_v3_cross(cam->front, cam->up)), movementSpeed));
    }
    if (input->moveBackward.endedDown)
    {
        cam->position = vm_v3_sub(cam->position, vm_v3_mulf(cam->front, movementSpeed));
    }
    if (input->moveRight.endedDown)
    {
        cam->position = vm_v3_add(cam->position, vm_v3_mulf(vm_v3_normalize(vm_v3_cross(cam->front, cam->up)), movementSpeed));
    }
    if (input->moveUp.endedDown)
    {
        cam->position = vm_v3_add(cam->position, vm_v3_mulf(cam->up, movementSpeed));
    }
    if (input->moveDown.endedDown)
    {
        cam->position = vm_v3_sub(cam->position, vm_v3_mulf(cam->up, movementSpeed));
    }

    if (input->mouseAttached)
    {
        const float mouseSensitivity = 0.1f;
        cam->yaw += vm_minf(input->mouseXOffset * mouseSensitivity, 89.0f);
        cam->pitch -= vm_maxf(input->mouseYOffset * mouseSensitivity, -89.0f);
        cam->pitch = vm_clampf(cam->pitch, -89.0f, 89.0f);
    }

    if (input->mouseScrollOffset != 0.0f)
    {
        cam->fov = vm_clampf(cam->fov - (input->mouseScrollOffset * 2), 1.0f, 179.0f);
    }

    camera_update_vectors(cam);
}

static speg_mesh cube_static = {
    "cube_static",
    false,
    true, /* Enable face culling */
    cube_vertices,
    sizeof(cube_vertices),
    cube_indices,
    sizeof(cube_indices),
    array_size(cube_indices)};

static speg_mesh cube_dynamic = {
    "cube_dynamic",
    false,
    true, /* Enable face culling */
    cube_vertices,
    sizeof(cube_vertices),
    cube_indices,
    sizeof(cube_indices),
    array_size(cube_indices)};

static speg_mesh rectangle_static = {
    "rectangle_static",
    false,
    false,
    rectangle_vertices,
    sizeof(rectangle_vertices),
    rectangle_indices,
    sizeof(rectangle_indices),
    array_size(rectangle_indices)};

void speg_draw_call_append(speg_draw_call *call, m4x4 *model, v3 *color)
{
    int m_offset = call->count_instances * VM_M4X4_ELEMENT_COUNT;
    int c_offset = call->count_instances * VM_V3_ELEMENT_COUNT;

    int i;

    assert(call->count_instances + 1 < call->count_instances_max);

    for (i = 0; i < VM_M4X4_ELEMENT_COUNT; ++i)
    {
        call->models[m_offset + i] = model->e[i];
    }

    call->colors[c_offset + 0] = color->x;
    call->colors[c_offset + 1] = color->y;
    call->colors[c_offset + 2] = color->z;

    call->count_instances += 1;
}

/* Render X, Y, Z axis lines (we use cubes but scaled in length and reduced in thichness)*/
void render_coordinate_axis(speg_draw_call *call)
{
    const float axisLength = 160.0f;
    const float axisThickness = 0.04f;

    v3 axisPosition = vm_v3_zero;
    m4x4 axisModel = vm_m4x4_translate(vm_m4x4_identity, axisPosition);
    v3 axisColors[3];   /* X, Y , Z*/
    v3 axisSizes[3];    /* X, Y , Z*/
    m4x4 axisModels[3]; /* X, Y , Z*/

    int i;

    axisColors[0] = vm_v3(1.0f, 0.0f, 0.0f);
    axisColors[1] = vm_v3(0.0f, 1.0f, 0.0f);
    axisColors[2] = vm_v3(0.0f, 0.0f, 1.0f);

    axisSizes[0] = vm_v3(axisLength, axisThickness, axisThickness);
    axisSizes[1] = vm_v3(axisThickness, axisLength, axisThickness);
    axisSizes[2] = vm_v3(axisThickness, axisThickness, axisLength);

    axisModels[0] = vm_m4x4_scale(axisModel, axisSizes[0]);
    axisModels[1] = vm_m4x4_scale(axisModel, axisSizes[1]);
    axisModels[2] = vm_m4x4_scale(axisModel, axisSizes[2]);

    for (i = 0; i < (int)array_size(axisModels); ++i)
    {
        speg_draw_call_append(call, &axisModels[i], &axisColors[i]);
    }
}

void render_cubes(speg_draw_call *call, m4x4 projection, m4x4 view, speg_state *state, speg_controller_input *input, float range)
{

#define NUM_INSTANCED_FRUST_CUBES 1000
    static int numCubes = NUM_INSTANCED_FRUST_CUBES;

    m4x4 projection_view = vm_m4x4_mul(projection, view);
    frustum frustum_planes = vm_frustum_extract_planes(projection_view);
    int i;
    bool draw;

    const float rangeMin = -range;
    const float rangeMax = range;
    const v3 rotation_axis = vm_v3_normalize(vm_v3(1.0f, 0.3f, 0.5f));
    const v3 color_red = vm_v3(1.0f, 0.0f, 0.0f);
    const float color_scale = 1.0f / 255.0f;

    vm_seed_lcg = 12345;

    for (i = 0; i < numCubes; ++i)
    {
        unsigned int color = (i == 0 ? 1 : i) * 10000000;
        float r = ((float)(((color >> 16) & 0xFF) >> 1)) * color_scale;
        float g = ((float)((color >> 8) & 0xFF)) * color_scale;
        float b = ((float)(color & 0xFF)) * color_scale;

        v3 targetPosition = vm_v3_zero;
        v3 targetColor = vm_v3(r, g, b);

        if (i == 0)
        {
            targetColor = vm_v3_one;
        }
        else
        {
            targetPosition.x = vm_randf_range(rangeMin, rangeMax);
            targetPosition.y = vm_randf_range(rangeMin, rangeMax);
            targetPosition.z = vm_randf_range(rangeMin, rangeMax);
        }

        /* TODO: epsilon 0.15f is needed because cubes are rotating and its not considered in the frustum check */
        draw = (bool)vm_frustum_is_cube_in(frustum_planes, targetPosition, vm_v3_one, 0.15f);

        if (!draw)
        {
            /* DISCARD - but in case we want to show discarded objects e.g. simulateCam = true we still render them */
            if (input->cameraSimulate.endedDown)
            {
                draw = true;
                targetColor = color_red;
            }
            state->culledObjects++;
        }

        /* Finally draw to screen by using platform api */
        if (draw)
        {

            /* calculate the model matrix for each object and pass it to shader before drawing */
            m4x4 model = vm_m4x4_translate(vm_m4x4_identity, targetPosition);

            if (i > 0)
            {
                model = vm_m4x4_rotate(model, vm_radf(20.0f * (float)i), rotation_axis);
            }

            speg_draw_call_append(call, &model, &targetColor);
        }
    }
}

#define NUM_INSTANCED_CUBES 20000
static int numCubes = NUM_INSTANCED_CUBES;

void render_cubes_instanced(speg_draw_call *call, float range)
{

    int i;
    const float color_scale = 1.0f / 255.0f;
    const float rangeMin = -range;
    const float rangeMax = range;

    for (i = 0; i < numCubes; ++i)
    {
        m4x4 model;
        v3 targetPosition = vm_v3_zero;
        v3 targetColor = vm_v3_one;

        if (i > 0)
        {
            unsigned int color = (i == 0 ? 1 : i) * 10000000;
            float r = ((float)(((color >> 16) & 0xFF) >> 1)) * color_scale;
            float g = ((float)((color >> 8) & 0xFF)) * color_scale;
            float b = ((float)(color & 0xFF)) * color_scale;

            targetPosition.x = vm_randf_range(rangeMin, rangeMax);
            targetPosition.y = vm_randf_range(rangeMin, rangeMax);
            targetPosition.z = vm_randf_range(rangeMin, rangeMax);
            targetColor = vm_v3(r, g, b);
        }

        model = vm_m4x4_translate(vm_m4x4_identity, targetPosition);

        speg_draw_call_append(call, &model, &targetColor);
    }
}

void render_transformations_test(speg_draw_call *call, speg_state *state)
{
    transformation parent = vm_tranformation_init();
    transformation child = vm_tranformation_init();
    transformation child2 = vm_tranformation_init();
    transformation child3 = vm_tranformation_init();
    transformation child4 = vm_tranformation_init();
    transformation child41 = vm_tranformation_init();
    transformation child411 = vm_tranformation_init();

    m4x4 current_transform;
    v3 color;

    static float rotation = 90.0f;
    rotation += (100.0f * (float)state->dt);

    parent.position.x = 4.0f;
    vm_tranformation_rotate(&parent, vm_v3(0.0f, 1.0f, 0.0f), vm_radf(rotation));
    current_transform = vm_transformation_matrix(&parent);
    color = vm_v3(1.0f, 0.0f, 0.0f);
    speg_draw_call_append(call, &current_transform, &color);

    child.position = vm_v3(3.0f, 0.0f, 0.0f);
    child.parent = &parent;
    current_transform = vm_transformation_matrix(&child);
    color = vm_v3(1.0f, 0.8745f, 0.0f);
    speg_draw_call_append(call, &current_transform, &color);

    child2.position = vm_v3(-3.0f, 0.0f, 0.0f);
    child2.parent = &parent;
    current_transform = vm_transformation_matrix(&child2);
    color = vm_v3(1.0f, 0.8745f, 0.0f);
    speg_draw_call_append(call, &current_transform, &color);

    child3.position = vm_v3(0.0f, 0.0f, 3.0f);
    child3.parent = &parent;
    current_transform = vm_transformation_matrix(&child3);
    color = vm_v3(1.0f, 0.8745f, 0.0f);
    speg_draw_call_append(call, &current_transform, &color);

    child4.position = vm_v3(0.0f, 0.0f, -3.0f);
    child4.parent = &parent;
    child4.rotation = vm_quat_rotate(vm_v3(0.0f, 1.0f, 0.0f), -vm_radf(rotation * 2.0f));
    current_transform = vm_transformation_matrix(&child4);
    color = vm_v3(1.0f, 0.8745f, 0.0f);
    speg_draw_call_append(call, &current_transform, &color);

    child41.position = vm_v3(0.0f, 0.0f, -2.0f);
    child41.parent = &child4;
    child41.rotation = vm_quat_rotate(vm_v3(0.0f, 1.0f, 0.0f), -vm_radf(rotation * 4.0f));
    current_transform = vm_transformation_matrix(&child41);
    color = vm_v3(0.0f, 1.0f, 0.0f);
    speg_draw_call_append(call, &current_transform, &color);

    child411.position = vm_v3(0.0f, 0.0f, -2.0f);
    child411.parent = &child41;
    current_transform = vm_transformation_matrix(&child411);
    color = vm_v3_zero;
    speg_draw_call_append(call, &current_transform, &color);
}

#define MAX_STATIC_INSTANCES 22000
static float all_static_models[MAX_STATIC_INSTANCES * VM_M4X4_ELEMENT_COUNT];
static float all_static_colors[MAX_STATIC_INSTANCES * VM_V3_ELEMENT_COUNT];
static speg_draw_call draw_call_static = {0};

#define MAX_DYNAMIC_INSTANCES 1024
static float all_dynamic_models[MAX_DYNAMIC_INSTANCES * VM_M4X4_ELEMENT_COUNT];
static float all_dynamic_colors[MAX_DYNAMIC_INSTANCES * VM_V3_ELEMENT_COUNT];
static speg_draw_call draw_call_dynamic = {0};

#define MAX_DYNAMIC_GUI_INSTANCES 128
static float all_dynamic_gui_models[MAX_DYNAMIC_GUI_INSTANCES * VM_M4X4_ELEMENT_COUNT];
static float all_dynamic_gui_colors[MAX_DYNAMIC_GUI_INSTANCES * VM_V3_ELEMENT_COUNT];
static speg_draw_call draw_call_dynamic_gui = {0};

#define PROFILE(func_call)                                                       \
    do                                                                           \
    {                                                                            \
        unsigned long __startCycles, __endCycles;                                \
        double __startTimeNano, __endTimeNano;                                   \
        (void)__startTimeNano;                                                   \
        (void)__endTimeNano;                                                     \
        __startTimeNano = platformApi->platform_perf_current_time_nanoseconds(); \
        __startCycles = platformApi->platform_perf_current_cycle_count();        \
        func_call;                                                               \
        __endCycles = platformApi->platform_perf_current_cycle_count();          \
        __endTimeNano = platformApi->platform_perf_current_time_nanoseconds();   \
        platformApi->platform_print_console(                                     \
            __FILE__,                                                            \
            __LINE__,                                                            \
            "[speg-profiler] cycles: %8d, ms: %12s, \"%s\"\n",                   \
            (__endCycles - __startCycles),                                       \
            "0.000000",                                                          \
            #func_call);                                                         \
    } while (0)

void render_gui_rectangle(speg_draw_call *call, speg_state *state, speg_controller_input *input)
{
    float width = 200.0f;
    float height = 30.0f;

    float screen_width = (float)state->width;
    float screen_height = (float)state->height;

    v3 position = vm_v3(screen_width * 0.5f, screen_height - height, 0.0f);
    v3 color = vm_v3(1.0f, 1.0f, 1.0f);
    v3 colorSelected = vm_v3(1.0f, 0.0f, 0.0f);

    m4x4 model = vm_m4x4_scale(vm_m4x4_translate(vm_m4x4_identity, position), vm_v3(width, height, 1.0f));

    int mouseX = input->mousePosX;
    int mouseY = state->height - input->mousePosY; /* Flip Y for OpenGL coords */
    float halfWidth = width * 0.5f;
    float halfHeight = height * 0.5f;

    bool inside = (mouseX >= position.x - halfWidth &&
                   mouseX <= position.x + halfWidth &&
                   mouseY >= position.y - halfHeight &&
                   mouseY <= position.y + halfHeight);

    speg_draw_call_append(call, &model, inside ? &colorSelected : &color);
}

void speg_update(speg_memory *memory, speg_controller_input *input, speg_platform_api *platformApi)
{
    static camera cam;
    speg_state *state = (speg_state *)memory->permanentMemory;
    m4x4 projection;
    m4x4 ortho_proj;
    m4x4 view;
    m4x4 projection_view;
    m4x4 view_simulated;

    assert(memory);
    assert(memory->permanentMemorySize > 0);
    assert(memory->permanentMemory);
    assert(input);
    assert(platformApi);

    /* Initialized only once at startup */
    if (!memory->initialized)
    {
        cam = camera_init();
        cam.position.z = 13.0f;

        memory->initialized = true;

        state->clearColorR = 0.2f;
        state->clearColorG = 0.2f;
        state->clearColorB = 0.2f;

        draw_call_static.mesh = &cube_static;
        draw_call_static.count_instances_max = array_size(all_static_models);
        draw_call_static.count_instances = 0;
        draw_call_static.models = all_static_models;
        draw_call_static.colors = all_static_colors;
        draw_call_static.changed = false;

        /* Static scenes */
        PROFILE(render_coordinate_axis(&draw_call_static));
        PROFILE(render_cubes_instanced(&draw_call_static, 100.0f));

        platformApi->platform_print_console(__FILE__, __LINE__, "[speg] initialized\n");
    }

    camera_update_movement(input, &cam, 10.0f * (float)state->dt);

    projection = vm_m4x4_perspective(vm_radf(cam.fov), (float)state->width / (float)state->height, 0.1f, 1000.0f);
    view = vm_m4x4_lookAt(cam.position, vm_v3_add(cam.position, cam.front), cam.up);
    ortho_proj = vm_m4x4_orthographic(0.0f, (float)state->width, 0.0f, (float)state->height, -1.0f, 1.0f);

    if (input->cameraSimulate.endedDown)
    {
        /* We set the camera position a bit back in order to see the discarded frustum culling objects (red) in the actual view */
        v3 simulatedCamPos = cam.position;
        simulatedCamPos.z += 10.0f;

        view_simulated = view;
        view = vm_m4x4_lookAt(simulatedCamPos, vm_v3_add(simulatedCamPos, cam.front), cam.up);
    }
    else
    {
        view_simulated = view;
    }

    projection_view = vm_m4x4_mul(projection, view);

    draw_call_dynamic.mesh = &cube_dynamic;
    draw_call_dynamic.count_instances_max = array_size(all_dynamic_models);
    draw_call_dynamic.count_instances = 0;
    draw_call_dynamic.models = all_dynamic_models;
    draw_call_dynamic.colors = all_dynamic_colors;
    draw_call_dynamic.changed = true;

    draw_call_dynamic_gui.mesh = &rectangle_static;
    draw_call_dynamic_gui.count_instances_max = array_size(all_dynamic_gui_models);
    draw_call_dynamic_gui.count_instances = 0;
    draw_call_dynamic_gui.models = all_dynamic_gui_models;
    draw_call_dynamic_gui.colors = all_dynamic_gui_colors;
    draw_call_dynamic_gui.changed = true;
    draw_call_dynamic_gui.is_2d = true;

    /* Dynamic scenes */
    render_cubes(&draw_call_dynamic, projection, view_simulated, state, input, 20.0f);
    render_transformations_test(&draw_call_dynamic, state);
    render_gui_rectangle(&draw_call_dynamic_gui, state, input);

    state->renderedObjects = draw_call_static.count_instances + draw_call_dynamic.count_instances + draw_call_dynamic_gui.count_instances;

    /* Draw static and dynamic scenes */
    platformApi->platform_draw(&draw_call_static, projection_view.e);
    platformApi->platform_draw(&draw_call_dynamic, projection_view.e);
    platformApi->platform_draw(&draw_call_dynamic_gui, ortho_proj.e);
}

#ifdef _WIN32
#ifdef __clang__
#elif __GNUC__
__attribute((externally_visible))
#endif
#ifdef __i686__
__attribute((force_align_arg_pointer))
#endif
int
DllMainCRTStartup(void)
{
    return 1;
}
#endif

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
