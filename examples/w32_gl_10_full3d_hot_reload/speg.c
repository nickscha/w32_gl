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

typedef struct camera
{
    v3 position;
    v3 front;
    v3 up;
    v3 right;
    v3 worldUp;

    float yaw;
    float pitch;
    float fov; /* = ZOOM Angle*/

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
    }

    if (input->mouseScrollOffset != 0.0f)
    {
        cam->fov = vm_clampf(cam->fov - (input->mouseScrollOffset * 2), 1.0f, 179.0f);
    }

    camera_update_vectors(cam);
}

void render_cubes(m4x4 projection, m4x4 *view, camera *cam, speg_state *state, speg_controller_input *input, speg_platform_api *platformApi, unsigned int numCubes, float range)
{
    m4x4 projection_view = vm_m4x4_mul(projection, *view);
    frustum frustum_planes = vm_frustum_extract_planes(projection_view);
    unsigned int i;
    m4x4 model;
    bool draw;
    int isInFrustum;

    vm_seed_lcg = 12345;

    for (i = 0; i < numCubes; ++i)
    {
        unsigned int color = (i == 0 ? 1 : i) * 10000000;
        unsigned int red = ((color & 0x00FF0000) >> 16) / 2; /* Avoid red */
        unsigned int green = (color & 0x0000FF00) >> 8;
        unsigned int blue = (color & 0x000000FF);

        v3 targetPosition = vm_v3(0.0f, 0.0f, 0.0f);
        v3 targetColor = vm_v3((float)red / 255.0f, (float)green / 255.0f, (float)blue / 255.0f);

        if (i == 0)
        {
            targetColor = vm_v3(1.0f, 1.0f, 1.0f);
        }
        else
        {
            float rangeMin = -range;
            float rangeMax = range;

            float xPos = vm_randf_range(rangeMin, rangeMax);
            float yPos = vm_randf_range(rangeMin, rangeMax);
            float zPos = vm_randf_range(rangeMin, rangeMax);

            targetPosition.x = xPos;
            targetPosition.y = yPos;
            targetPosition.z = zPos;
        }

        /* calculate the model matrix for each object and pass it to shader before drawing */
        model = vm_m4x4_translate(vm_m4x4_identity, targetPosition);

        if (i > 0)
        {
            model = vm_m4x4_rotate(model, vm_radf(20.0f * (float)i), vm_v3_normalize(vm_v3(1.0f, 0.3f, 0.5f)));
        }

        /* TODO: epsilon 0.15f is needed because cubes are rotating and its not considered in the frustum check */
        isInFrustum = vm_frustum_is_cube_in(frustum_planes, targetPosition, vm_v3(1.0f, 1.0f, 1.0f), 0.15f);

        draw = (bool)isInFrustum;

        if (input->cameraSimulate.endedDown)
        {
            /* We set the camera position a bit back in order to see the discarded frustum culling objects (red) in the actual view */
            v3 simulatedCamPos = cam->position;
            simulatedCamPos.z += 10.0f;
            *view = vm_m4x4_lookAt(simulatedCamPos, vm_v3_add(simulatedCamPos, cam->front), cam->up);
        }

        if (isInFrustum)
        {
            /* DRAW */
            state->renderedObjects++;
        }
        else
        {
            /* DISCARD - but in case we want to show discarded objects e.g. simulateCam = true we still render them */
            if (input->cameraSimulate.endedDown)
            {
                draw = true;
                targetColor = vm_v3(1.0f, 0.0f, 0.0f);
            }
            state->culledObjects++;
        }

        /* Finally draw to screen by using platform api */
        if (draw)
        {
            m4x4 mvp = vm_m4x4_mul(vm_m4x4_mul(projection, *view), model);
            platformApi->platform_draw(cube_vertices, cube_indices, sizeof(cube_vertices), sizeof(cube_indices), array_size(cube_indices), mvp.e, targetColor.x, targetColor.y, targetColor.z);
        }
    }
}

/* Render X, Y, Z axis lines (we use cubes but scaled in length and reduced in thichness)*/
void render_coordinate_axis(m4x4 projection, m4x4 view, speg_state *state, speg_platform_api *platformApi)
{

    const float axisLength = 160.0f;
    const float axisThickness = 0.04f;

    v3 axisPosition = vm_v3(0.0f, 0.0f, 0.0f);

    m4x4 projection_view = vm_m4x4_mul(projection, view);
    frustum frustum_planes = vm_frustum_extract_planes(projection_view);

    m4x4 axisModel = vm_m4x4_translate(vm_m4x4_identity, axisPosition);
    v3 axisColors[3];   /* X, Y , Z*/
    v3 axisSizes[3];    /* X, Y , Z*/
    m4x4 axisModels[3]; /* X, Y , Z*/

    int i;
    int isInFrustum;

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
        isInFrustum = vm_frustum_is_cube_in(frustum_planes, axisPosition, axisSizes[i], 0.15f);

        if (isInFrustum)
        {
            m4x4 mvp = vm_m4x4_mul(projection_view, axisModels[i]);

            platformApi->platform_draw(cube_vertices, cube_indices, sizeof(cube_vertices), sizeof(cube_indices), array_size(cube_indices), mvp.e, axisColors[i].x, axisColors[i].y, axisColors[i].z);
            state->renderedObjects++;
        }
        else
        {
            state->culledObjects++;
        }
    }
}

void render_transformations_test(m4x4 projection, m4x4 view, speg_state *state, speg_platform_api *platformApi)
{
    m4x4 mvp;
    m4x4 projection_view = vm_m4x4_mul(projection, view);
    transformation parent = vm_tranformation_init();
    transformation child = vm_tranformation_init();
    transformation child2 = vm_tranformation_init();
    transformation child3 = vm_tranformation_init();
    transformation child4 = vm_tranformation_init();
    transformation child41 = vm_tranformation_init();
    transformation child411 = vm_tranformation_init();

    static float rotation = 90.0f;
    rotation += (100.0f * (float)state->dt);

    parent.position.x = 4.0f;
    vm_tranformation_rotate(&parent, vm_v3(0.0f, 1.0f, 0.0f), vm_radf(rotation));

    mvp = vm_m4x4_mul(projection_view, vm_transformation_matrix(&parent));
    platformApi->platform_draw(cube_vertices, cube_indices, sizeof(cube_vertices), sizeof(cube_indices), array_size(cube_indices), mvp.e, 1.0f, 0.0f, 0.0f);

    child.position = vm_v3(3.0f, 0.0f, 0.0f);
    child.parent = &parent;

    mvp = vm_m4x4_mul(projection_view, vm_transformation_matrix(&child));
    platformApi->platform_draw(cube_vertices, cube_indices, sizeof(cube_vertices), sizeof(cube_indices), array_size(cube_indices), mvp.e, 1.0f, 0.8745f, 0.0f);

    child2.position = vm_v3(-3.0f, 0.0f, 0.0f);
    child2.parent = &parent;

    mvp = vm_m4x4_mul(projection_view, vm_transformation_matrix(&child2));
    platformApi->platform_draw(cube_vertices, cube_indices, sizeof(cube_vertices), sizeof(cube_indices), array_size(cube_indices), mvp.e, 1.0f, 0.8745f, 0.0f);

    child3.position = vm_v3(0.0f, 0.0f, 3.0f);
    child3.parent = &parent;

    mvp = vm_m4x4_mul(projection_view, vm_transformation_matrix(&child3));
    platformApi->platform_draw(cube_vertices, cube_indices, sizeof(cube_vertices), sizeof(cube_indices), array_size(cube_indices), mvp.e, 1.0f, 0.8745f, 0.0f);

    child4.position = vm_v3(0.0f, 0.0f, -3.0f);
    child4.parent = &parent;
    child4.rotation = vm_quat_rotate(vm_v3(0.0f, 1.0f, 0.0f), -vm_radf(rotation * 2.0f));

    mvp = vm_m4x4_mul(projection_view, vm_transformation_matrix(&child4));
    platformApi->platform_draw(cube_vertices, cube_indices, sizeof(cube_vertices), sizeof(cube_indices), array_size(cube_indices), mvp.e, 1.0f, 0.8745f, 0.0f);

    child41.position = vm_v3(0.0f, 0.0f, -2.0f);
    child41.parent = &child4;
    child41.rotation = vm_quat_rotate(vm_v3(0.0f, 1.0f, 0.0f), -vm_radf(rotation * 4.0f));

    mvp = vm_m4x4_mul(projection_view, vm_transformation_matrix(&child41));
    platformApi->platform_draw(cube_vertices, cube_indices, sizeof(cube_vertices), sizeof(cube_indices), array_size(cube_indices), mvp.e, 0.0f, 1.0f, 0.0f);

    child411.position = vm_v3(0.0f, 0.0f, -2.0f);
    child411.parent = &child41;

    mvp = vm_m4x4_mul(projection_view, vm_transformation_matrix(&child411));
    platformApi->platform_draw(cube_vertices, cube_indices, sizeof(cube_vertices), sizeof(cube_indices), array_size(cube_indices), mvp.e, 0.0f, 0.0f, 0.0f);
}

static speg_mesh cube = {
    false,
    true, /* Enable face culling */
    cube_vertices,
    sizeof(cube_vertices),
    cube_indices,
    sizeof(cube_indices),
    array_size(cube_indices)};

typedef struct speg_draw_call
{
    speg_mesh *mesh;
    float *matrices;
    int matrices_count;
    float color[3];

} speg_draw_call;

speg_draw_call render_cubes_instanced(speg_state *state, float range)
{
    /* TODO: Perform frustum culling when I have a dynamic array list implementation*/
    static bool calculatedPositions = false;

#define NUM_INSTANCED_CUBES 20000
    static int numCubes = NUM_INSTANCED_CUBES;
    static float models[VM_M4X4_ELEMENT_COUNT * NUM_INSTANCED_CUBES];
    static float color[3] = {0.8f, 0.8f, 0.8f};

    m4x4 model;

    speg_draw_call call = {0};

    if (!calculatedPositions)
    {
        int i;
        int j;

        for (i = 0; i < numCubes; ++i)
        {
            v3 targetPosition = vm_v3_zero;
            int idx_base = (i * VM_M4X4_ELEMENT_COUNT);

            if (i > 0)
            {
                float rangeMin = -range;
                float rangeMax = range;
                targetPosition.x = vm_randf_range(rangeMin, rangeMax);
                targetPosition.y = vm_randf_range(rangeMin, rangeMax);
                targetPosition.z = vm_randf_range(rangeMin, rangeMax);
            }

            model = vm_m4x4_translate(vm_m4x4_identity, targetPosition);

            for (j = 0; j < VM_M4X4_ELEMENT_COUNT; ++j)
            {
                models[idx_base + j] = model.e[j]; /* projection matrix */
            }
        }

        calculatedPositions = true;
    }

    call.mesh = &cube;
    call.matrices = models;
    call.matrices_count = numCubes;
    call.color[0] = color[0];
    call.color[1] = color[1];
    call.color[2] = color[2];

    state->renderedObjects += numCubes;

    return call;
}

void speg_update(speg_memory *memory, speg_controller_input *input, speg_platform_api *platformApi)
{
    static camera cam;
    speg_state *state = (speg_state *)memory->permanentMemory;
    unsigned long startCycleCount;
    unsigned long endCycleCount;
    m4x4 projection;
    m4x4 view;
    speg_draw_call call;

    assert(memory);
    assert(memory->permanentMemorySize > 0);
    assert(memory->permanentMemory);
    assert(input);
    assert(platformApi);

    /* Initialized only once at startup */
    if (!memory->initialized)
    {
        startCycleCount = platformApi->platform_perf_current_cycle_count();

        cam = camera_init();
        cam.position.z = 13.0f;

        memory->initialized = true;

        endCycleCount = platformApi->platform_perf_current_cycle_count();

        state->clearColorR = 0.2f;
        state->clearColorG = 0.2f;
        state->clearColorB = 0.2f;

        platformApi->platform_print_console(__FILE__, __LINE__, "[speg] initialization cycles: %4d\n", (endCycleCount - startCycleCount));
        platformApi->platform_print_console(__FILE__, __LINE__, "[speg] initialized\n");
    }

    camera_update_movement(input, &cam, 10.0f * (float)state->dt);

    projection = vm_m4x4_perspective(vm_radf(cam.fov), (float)state->width / (float)state->height, 0.1f, 1000.0f);
    view = vm_m4x4_lookAt(cam.position, vm_v3_add(cam.position, cam.front), cam.up);

    render_cubes(projection, &view, &cam, state, input, platformApi, 1000, 20.0f);
    render_transformations_test(projection, view, state, platformApi);
    render_coordinate_axis(projection, view, state, platformApi);

    call = render_cubes_instanced(state, 100.0f);

    platformApi->platform_draw_instanced(call.mesh, call.matrices_count, call.matrices, projection.e, view.e, call.color);
}

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
