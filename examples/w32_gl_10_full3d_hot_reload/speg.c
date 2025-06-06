#include "speg.h"
#include "vm.h"
#include "rigid_body.h"

typedef struct speg_controller_input
{
    platform_controller_state moveForward;
    platform_controller_state moveBackward;
    platform_controller_state moveLeft;
    platform_controller_state moveRight;
    platform_controller_state moveUp;
    platform_controller_state moveDown;

    /* Debug / Dev commands */
    platform_controller_state cameraSimulate;
    platform_controller_state cameraResetPosition;
    platform_controller_state debug_mode;
    platform_controller_state debug_mode_step;
    platform_controller_state debug_mode_step_continuously;

    bool mouseAttached;
    float mouseScrollOffset;
    float mouseXOffset;
    float mouseYOffset;
    int mousePosX;
    int mousePosY;

} speg_controller_input;

speg_controller_input speg_map_controller_input(platform_controller_input *platform_input)
{
    speg_controller_input result;

    result.moveForward = platform_input->key_w;
    result.moveBackward = platform_input->key_s;
    result.moveLeft = platform_input->key_a;
    result.moveRight = platform_input->key_d;
    result.moveUp = platform_input->key_space;
    result.moveDown = platform_input->key_control;

    result.cameraSimulate = platform_input->key_f3;
    result.cameraResetPosition = platform_input->key_f5;
    result.debug_mode = platform_input->key_tab;
    result.debug_mode_step = platform_input->key_i;
    result.debug_mode_step_continuously = platform_input->key_u;

    result.mouseAttached = platform_input->mouse_attached;
    result.mouseScrollOffset = platform_input->mouse_offset_scroll;
    result.mouseXOffset = platform_input->mouse_offset_x;
    result.mouseYOffset = platform_input->mouse_offset_y;
    result.mousePosX = platform_input->mouse_position_x;
    result.mousePosY = platform_input->mouse_position_y;

    return (result);
}

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

static float cube_uvs[] = {
    0.0f, 0.0f,
    1.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f,

    0.0f, 0.0f,
    1.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f,

    0.0f, 0.0f,
    1.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f,

    0.0f, 0.0f,
    1.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f,

    0.0f, 0.0f,
    1.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f,

    0.0f, 0.0f,
    1.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f};

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

static float rectangle_uvs[] = {
    0.0f, 0.0f,
    1.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f};

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
        cam->position.y = 2.0f;
    }
    if (input->moveForward.endedDown)
    {
        cam->position = vm_v3_add(cam->position, vm_v3_mulf(cam->front, movementSpeed));
    }
    if (input->moveLeft.endedDown)
    {
        cam->position = vm_v3_sub(cam->position, vm_v3_mulf(cam->right, movementSpeed));
    }
    if (input->moveBackward.endedDown)
    {
        cam->position = vm_v3_sub(cam->position, vm_v3_mulf(cam->front, movementSpeed));
    }
    if (input->moveRight.endedDown)
    {
        cam->position = vm_v3_add(cam->position, vm_v3_mulf(cam->right, movementSpeed));
    }
    if (input->moveUp.endedDown)
    {
        cam->position = vm_v3_add(cam->position, vm_v3_mulf(cam->worldUp, movementSpeed));
    }
    if (input->moveDown.endedDown)
    {
        cam->position = vm_v3_sub(cam->position, vm_v3_mulf(cam->worldUp, movementSpeed));
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

void speg_float_to_string(float value, char *buffer, int precision)
{
    int i;
    int int_part;
    char temp[64];
    int temp_index = 0;

    if (value < 0)
    {
        *buffer++ = '-';
        value = -value;
    }
    else
    {
        *buffer++ = ' ';
    }

    int_part = (int)value;
    value -= (float)int_part;

    if (int_part == 0)
    {
        temp[temp_index++] = '0';
    }
    else
    {
        while (int_part > 0)
        {
            temp[temp_index++] = (char)((int_part % 10) + '0');
            int_part /= 10;
        }
        for (i = 0; i < temp_index / 2; i++)
        {
            char t = temp[i];
            temp[i] = temp[temp_index - i - 1];
            temp[temp_index - i - 1] = t;
        }
    }

    for (i = 0; i < temp_index; ++i)
    {
        *buffer++ = temp[i];
    }

    if (precision > 0)
    {
        *buffer++ = '.';
        while (precision-- > 0)
        {
            int digit;
            value *= 10;
            digit = (int)value;
            *buffer++ = (char)(digit + '0');
            value -= (float)digit;
        }
    }

    *buffer = '\0';
}

#define PROFILE(func_call) PROFILE_WITH_NAME(func_call, #func_call)
#define PROFILE_WITH_NAME(func_call, name)                                       \
    do                                                                           \
    {                                                                            \
        char floatBuffer[32];                                                    \
        unsigned long __startCycles, __endCycles;                                \
        double __startTimeNano, __endTimeNano;                                   \
        float __timeMs;                                                          \
        __startTimeNano = platformApi->platform_perf_current_time_nanoseconds(); \
        __startCycles = platformApi->platform_perf_current_cycle_count();        \
        func_call;                                                               \
        __endCycles = platformApi->platform_perf_current_cycle_count();          \
        __endTimeNano = platformApi->platform_perf_current_time_nanoseconds();   \
        __timeMs = (float)((__endTimeNano - __startTimeNano) / 1000000.0);       \
        speg_float_to_string(__timeMs, floatBuffer, 6);                          \
        platformApi->platform_print_console(                                     \
            __FILE__,                                                            \
            __LINE__,                                                            \
            "[speg-profiler] %8d cycles, %12s ms, \"%s\"\n",                     \
            (__endCycles - __startCycles),                                       \
            floatBuffer,                                                         \
            name);                                                               \
    } while (0)

static int default_texture_index = -1;

void speg_draw_call_append(speg_draw_call *call, m4x4 *model, v3 *color, int texture_index)
{
    int m_offset = call->count_instances * VM_M4X4_ELEMENT_COUNT;
    int c_offset = call->count_instances * VM_V3_ELEMENT_COUNT;
    int t_offset = call->count_instances;

    int i;

    assert(call->count_instances + 1 < call->count_instances_max);

    for (i = 0; i < VM_M4X4_ELEMENT_COUNT; ++i)
    {
        call->models[m_offset + i] = model->e[i];
    }

    call->colors[c_offset + 0] = color->x;
    call->colors[c_offset + 1] = color->y;
    call->colors[c_offset + 2] = color->z;
    call->texture_indices[t_offset + 0] = texture_index;

    call->count_instances += 1;
}

/* Render X, Y, Z axis lines (we use cubes but scaled in length and reduced in thichness)*/
void render_coordinate_axis(speg_draw_call *call)
{
    const float axisLength = 160.0f;
    const float axisThickness = 0.04f;

    v3 axisPosition = vm_v3_zero;
    m4x4 axisModel = vm_m4x4_translate(vm_m4x4_identity, axisPosition);

    v3 axisColors[3];   /* X, Y , Z */
    v3 axisSizes[3];    /* X, Y , Z */
    m4x4 axisModels[3]; /* X, Y , Z */

    axisColors[0] = vm_v3(1.0f, 0.0f, 0.0f);
    axisColors[1] = vm_v3(0.0f, 1.0f, 0.0f);
    axisColors[2] = vm_v3(0.0f, 0.0f, 1.0f);

    axisSizes[0] = vm_v3(axisLength, axisThickness, axisThickness);
    axisSizes[1] = vm_v3(axisThickness, axisLength, axisThickness);
    axisSizes[2] = vm_v3(axisThickness, axisThickness, axisLength);

    axisModels[0] = vm_m4x4_scale(axisModel, axisSizes[0]);
    axisModels[1] = vm_m4x4_scale(axisModel, axisSizes[1]);
    axisModels[2] = vm_m4x4_scale(axisModel, axisSizes[2]);

    speg_draw_call_append(call, &axisModels[0], &axisColors[0], default_texture_index);
    speg_draw_call_append(call, &axisModels[1], &axisColors[1], default_texture_index);
    speg_draw_call_append(call, &axisModels[2], &axisColors[2], default_texture_index);
}

void render_grid(speg_draw_call *call)
{
    float grid_line_thickness = 0.01f;
    int grid_size = 101; /* Uneven for perfect alignment */
    float grid_line_length = (float)grid_size - 1.0f;
    v3 grid_color = vm_v3(0.3f, 0.3f, 0.3f);

    int i;

    for (i = 0; i < grid_size; ++i)
    {
        float grid_line_pos = (float)(i - (int)((float)grid_size * 0.5f));
        m4x4 grid_line_model_x = vm_m4x4_scale(vm_m4x4_translate(vm_m4x4_identity, vm_v3(0.0f, 0.0f, grid_line_pos)), vm_v3(grid_line_length, grid_line_thickness, grid_line_thickness));
        m4x4 grid_line_model_z = vm_m4x4_scale(vm_m4x4_translate(vm_m4x4_identity, vm_v3(grid_line_pos, 0.0f, 0.0f)), vm_v3(grid_line_thickness, grid_line_thickness, grid_line_length));
        speg_draw_call_append(call, &grid_line_model_x, &grid_color, default_texture_index);
        speg_draw_call_append(call, &grid_line_model_z, &grid_color, default_texture_index);
    }
}

void spawn_random_cube(int i, float range, v3 *position, v3 *color)
{
    static const float color_scale = 1.0f / 255.0f;
    unsigned int c = ((unsigned int)i == 0 ? 1 : (unsigned int)i) * 10000000;
    float r = ((float)(((c >> 16) & 0xFF) >> 1)) * color_scale;
    float g = ((float)((c >> 8) & 0xFF)) * color_scale;
    float b = ((float)(c & 0xFF)) * color_scale;

    if (i == 0)
    {
        *color = vm_v3_one;
        *position = vm_v3_zero;
    }
    else
    {
        *color = vm_v3(r, g, b);
        position->x = vm_randf_range(-range, range);
        position->y = vm_randf_range(-range, range);
        position->z = vm_randf_range(-range, range);
    }
}

void render_cubes(speg_draw_call *call, m4x4 projection, m4x4 view, speg_state *state, speg_controller_input *input, float range, camera *cam)
{

#define NUM_INSTANCED_FRUST_CUBES 1000
    static int numCubes = NUM_INSTANCED_FRUST_CUBES;

    m4x4 projection_view = vm_m4x4_mul(projection, view);
    frustum frustum_planes = vm_frustum_extract_planes(projection_view);
    int i;

    const v3 rotation_axis = vm_v3_normalize(vm_v3(1.0f, 0.3f, 0.5f));
    const v3 color_red = vm_v3(1.0f, 0.0f, 0.0f);

    vm_seed_lcg = 12345;

    for (i = 0; i < numCubes; ++i)
    {
        bool draw;
        v3 targetPosition;
        v3 targetColor;

        spawn_random_cube(i, range, &targetPosition, &targetColor);

        if (i == 0)
        {
            targetPosition = vm_v3(-2.0f, 0.0f, 0.0f);
        }

        /* TODO: epsilon 0.15f is needed because cubes are rotating and its not considered in the frustum check */
        draw = (bool)vm_frustum_is_cube_in(frustum_planes, targetPosition, vm_v3_one, 0.15f);

        if (!draw)
        {
            /* DISCARD - but in case we want to show discarded objects e.g. simulateCam = true we still render them */
            if (input->cameraSimulate.active)
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
            m4x4 model_base = vm_m4x4_translate(vm_m4x4_identity, targetPosition);
            m4x4 model = (i > 0)
                             ? vm_m4x4_rotate(model_base, vm_radf(20.0f * (float)i), rotation_axis)
                             : vm_m4x4_lookAt_model(targetPosition, cam->position, cam->worldUp);

            speg_draw_call_append(call, &model, &targetColor, default_texture_index);
        }
    }
}

void render_cubes_instanced(speg_draw_call *call, float range)
{
    int i;

    for (i = 0; i < 20000; ++i)
    {
        m4x4 model;
        v3 targetPosition = vm_v3_zero;
        v3 targetColor = vm_v3_one;

        spawn_random_cube(i, range, &targetPosition, &targetColor);

        model = vm_m4x4_translate(vm_m4x4_identity, targetPosition);

        speg_draw_call_append(call, &model, &targetColor, default_texture_index);
    }
}

void render_transformations_test(speg_draw_call *call, speg_state *state)
{
    transformation parent = vm_transformation_init();
    transformation child = vm_transformation_init();
    transformation child2 = vm_transformation_init();
    transformation child3 = vm_transformation_init();
    transformation child4 = vm_transformation_init();
    transformation child41 = vm_transformation_init();
    transformation child411 = vm_transformation_init();

    m4x4 current_transform;
    v3 color;

    static float rotation = 90.0f;
    rotation += (100.0f * (float)state->dt);

    parent.position.x = 4.0f;
    vm_tranformation_rotate(&parent, vm_v3(0.0f, 1.0f, 0.0f), vm_radf(rotation));
    current_transform = vm_transformation_matrix(&parent);
    color = vm_v3(1.0f, 0.0f, 0.0f);
    speg_draw_call_append(call, &current_transform, &color, default_texture_index);

    child.position = vm_v3(3.0f, 0.0f, 0.0f);
    child.parent = &parent;
    current_transform = vm_transformation_matrix(&child);
    color = vm_v3(1.0f, 0.8745f, 0.0f);
    speg_draw_call_append(call, &current_transform, &color, default_texture_index);

    child2.position = vm_v3(-3.0f, 0.0f, 0.0f);
    child2.parent = &parent;
    current_transform = vm_transformation_matrix(&child2);
    color = vm_v3(1.0f, 0.8745f, 0.0f);
    speg_draw_call_append(call, &current_transform, &color, default_texture_index);

    child3.position = vm_v3(0.0f, 0.0f, 3.0f);
    child3.parent = &parent;
    current_transform = vm_transformation_matrix(&child3);
    color = vm_v3(1.0f, 0.8745f, 0.0f);
    speg_draw_call_append(call, &current_transform, &color, default_texture_index);

    child4.position = vm_v3(0.0f, 0.0f, -3.0f);
    child4.parent = &parent;
    child4.rotation = vm_quat_rotate(vm_v3(0.0f, 1.0f, 0.0f), -vm_radf(rotation * 2.0f));
    current_transform = vm_transformation_matrix(&child4);
    color = vm_v3(1.0f, 0.8745f, 0.0f);
    speg_draw_call_append(call, &current_transform, &color, default_texture_index);

    child41.position = vm_v3(0.0f, 0.0f, -2.0f);
    child41.parent = &child4;
    child41.rotation = vm_quat_rotate(vm_v3(0.0f, 1.0f, 0.0f), -vm_radf(rotation * 4.0f));
    current_transform = vm_transformation_matrix(&child41);
    color = vm_v3(0.0f, 1.0f, 0.0f);
    speg_draw_call_append(call, &current_transform, &color, default_texture_index);

    child411.position = vm_v3(0.0f, 0.0f, -2.0f);
    child411.parent = &child41;
    current_transform = vm_transformation_matrix(&child411);
    color = vm_v3_zero;
    speg_draw_call_append(call, &current_transform, &color, default_texture_index);
}

void render_gui_rectangle(speg_draw_call *call, speg_state *state, speg_controller_input *input)
{
    float screen_width = (float)state->width;
    float screen_height = (float)state->height;

    float element_z_pos = 0.0f;
    float element_width = 200.0f;
    float element_height = 30.0f;
    float element_width_half = element_width * 0.5f;
    float element_height_half = element_height * 0.5f;

    v3 position = vm_v3(screen_width * 0.5f, screen_height - element_height, element_z_pos);
    v3 colorDefault = vm_v3(1.0f, 1.0f, 1.0f);
    v3 colorSelected = vm_v3(1.0f, 0.0f, 0.0f);

    m4x4 model = vm_m4x4_scale(vm_m4x4_translate(vm_m4x4_identity, position), vm_v3(element_width, element_height, 1.0f));

    float mouseX = (float)input->mousePosX;
    float mouseY = (float)input->mousePosY; /* Flip Y for OpenGL coords */

    bool inside = (mouseX >= position.x - element_width_half &&
                   mouseX <= position.x + element_width_half &&
                   mouseY >= position.y - element_height_half &&
                   mouseY <= position.y + element_height_half);

    speg_draw_call_append(call, &model, inside ? &colorSelected : &colorDefault, default_texture_index);
}

void render_character(speg_draw_call *call, speg_state *state, char character, v3 color, v2 dimensions, float xOffset, float yOffset)
{
    float screen_width = (float)state->width;
    float screen_height = (float)state->height;

    float element_z_pos = 0.0f;
    float element_width = dimensions.x;
    float element_height = dimensions.y;

    v3 position = vm_v3((screen_width * 0.5f) + xOffset, screen_height - (4 * element_height) + yOffset, element_z_pos);
    m4x4 model = vm_m4x4_scale(vm_m4x4_translate(vm_m4x4_identity, position), vm_v3(element_width, element_height, 1.0f));

    int c = character - 32;
    speg_draw_call_append(call, &model, &color, c);
}

void generate_random_string(char *str, int length)
{
    static unsigned long lcg_state = 123456789;
    const char printable_ascii_start = 32;
    const char printable_ascii_end = 126;
    int i;

    for (i = 0; i < length; ++i)
    {
        char random_char;

        lcg_state = lcg_state * 1103515245 + 12345;
        random_char = (char)((int)((lcg_state / 65536) % 32768) % (printable_ascii_end - printable_ascii_start + 1)) + printable_ascii_start;

        str[i] = random_char;
    }
    str[length] = '\0';
}

void render_text(speg_draw_call *call, speg_state *state, speg_platform_api *platformApi)
{
    float scale = 0.7f;
    v2 size = vm_v2(17.0f * scale, 32.0f * scale);
    v3 red = vm_v3(1.0f, 0.0f, 0.0f);
    v3 green = vm_v3(0.0f, 1.0f, 0.0f);
    v3 blue = vm_v3(0.0f, 0.0f, 1.0f);
    v3 black = vm_v3(0.0f, 0.0f, 0.0f);
    v3 grey = vm_v3(0.1f, 0.1f, 0.1f);

    int i;

    float xOffsetInitial = 40.0f;
    float xOffset = xOffsetInitial;
    float yOffset = -200.0f;

    double startTimeNano;
    double endTimeNano;
    unsigned long startCycles;
    unsigned long endCycles;
    float msPassed;
    char floatBuffer[32];
    char *textBuffer;

    static const char *str = "Hello, world!\ntest_from_pure_c89 nostdlib :)\n!\"%&/()=?{}[]*+,.:,<>@^_|~\n0123456789\nabcdefghijklmnopqrstuvwxyz\nABCDEFGHIJKLMNOPQRSTUVWXYZ";
    char random_string[17];

    startTimeNano = platformApi->platform_perf_current_time_nanoseconds();
    startCycles = platformApi->platform_perf_current_cycle_count();

    generate_random_string(random_string, 16);

    for (i = 0; str[i] != '\0'; ++i)
    {
        char c = str[i];
        if (c == '\n')
        {
            yOffset -= size.y;
            xOffset = xOffsetInitial;
            continue;
        }

        render_character(call, state, c, grey, size, xOffset + 1.0f, yOffset - 1.0f);
        render_character(call, state, c, i % 3 ? green : (i % 5 ? blue : red), size, xOffset, yOffset);
        xOffset += size.x;
    }

    xOffset = xOffsetInitial;
    yOffset -= 2 * size.y;

    for (i = 0; random_string[i] != '\0'; ++i)
    {
        static const float color_scale = 1.0f / 255.0f;
        unsigned int n = (unsigned int)((i == 0 ? 1 : i) * 3000000);
        float r = ((float)(((n >> 16) & 0xFF) >> 1)) * color_scale;
        float g = ((float)((n >> 8) & 0xFF)) * color_scale;
        float b = ((float)(n & 0xFF)) * color_scale;
        v3 random_color = vm_v3(r, g, b);

        char c = random_string[i];
        if (c == '\n')
        {
            yOffset -= size.y;
            xOffset = xOffsetInitial;
            continue;
        }

        render_character(call, state, c, random_color, size, xOffset, yOffset);
        xOffset += size.x;
    }

    endCycles = platformApi->platform_perf_current_cycle_count();
    endTimeNano = platformApi->platform_perf_current_time_nanoseconds();
    msPassed = (float)((endTimeNano - startTimeNano) / 1000000.0);

    speg_float_to_string(msPassed, floatBuffer, 6);

    xOffset = xOffsetInitial;
    yOffset -= 2 * size.y;

    for (i = 0; floatBuffer[i] != '\0'; ++i)
    {
        char c = floatBuffer[i];
        if (c == '\n')
        {
            yOffset -= size.y;
            xOffset = xOffsetInitial;
            continue;
        }

        render_character(call, state, c, black, size, xOffset, yOffset);
        xOffset += size.x;
    }

    render_character(call, state, ' ', black, size, xOffset, yOffset);
    xOffset += size.x;

    render_character(call, state, 'm', black, size, xOffset, yOffset);
    xOffset += size.x;

    render_character(call, state, 's', black, size, xOffset, yOffset);
    xOffset += size.x;

    xOffset = xOffsetInitial;
    yOffset -= size.y;

    /* CPU Cycles */
    speg_float_to_string((float)(endCycles - startCycles), floatBuffer, 0);

    for (i = 0; floatBuffer[i] != '\0'; ++i)
    {
        char c = floatBuffer[i];
        if (c == '\n')
        {
            yOffset -= size.y;
            xOffset = xOffsetInitial;
            continue;
        }

        render_character(call, state, c, black, size, xOffset, yOffset);
        xOffset += size.x;
    }

    textBuffer = " CPU cycles";

    for (i = 0; textBuffer[i] != '\0'; ++i)
    {
        char c = textBuffer[i];
        if (c == '\n')
        {
            yOffset -= size.y;
            xOffset = xOffsetInitial;
            continue;
        }

        render_character(call, state, c, black, size, xOffset, yOffset);
        xOffset += size.x;
    }
}

float simple_power_curve_evaluate(float normRPM)
{
    return (400.0f * (1.0f - normRPM));
}

void render_vector(speg_draw_call *call, v3 base_position, v3 vector, v3 color)
{
    float line_thickness = 0.04f;
    float length = vm_v3_length(vector);
    v3 dir;
    v3 up;
    v3 right;
    v3 forward;
    v3 scale;
    v3 midpoint;
    m4x4 rotation;
    m4x4 scale_matrix;
    m4x4 translation;
    m4x4 model;

    if (length < 1e-6f)
        return;

    dir = vm_v3_normalize(vector);
    up = dir;

    right = vm_v3_cross((vm_absf(up.y) < 0.999f ? vm_v3_up : vm_v3_forward), up);
    right = vm_v3_normalize(right);
    forward = vm_v3_cross(up, right);

    rotation = vm_m4x4_identity;
    rotation.e[VM_M4X4_AT(0, 0)] = right.x;
    rotation.e[VM_M4X4_AT(1, 0)] = right.y;
    rotation.e[VM_M4X4_AT(2, 0)] = right.z;

    rotation.e[VM_M4X4_AT(0, 1)] = up.x;
    rotation.e[VM_M4X4_AT(1, 1)] = up.y;
    rotation.e[VM_M4X4_AT(2, 1)] = up.z;

    rotation.e[VM_M4X4_AT(0, 2)] = forward.x;
    rotation.e[VM_M4X4_AT(1, 2)] = forward.y;
    rotation.e[VM_M4X4_AT(2, 2)] = forward.z;

    scale = vm_v3(line_thickness, length, line_thickness);
    scale_matrix = vm_m4x4_scale(vm_m4x4_identity, scale);

    midpoint = vm_v3_add(base_position, vm_v3_mulf(vector, 0.5f));
    translation = vm_m4x4_translate(vm_m4x4_identity, midpoint);

    model = vm_m4x4_mul(translation, vm_m4x4_mul(rotation, scale_matrix));

    speg_draw_call_append(call, &model, &color, default_texture_index);
}

v2 render_full_text(speg_draw_call *call, speg_state *state, char *str, v3 color, v2 dimensions, v2 offsets)
{
    v2 result;

    float xOffset = offsets.x;
    float yOffset = offsets.y;

    int i;

    for (i = 0; str[i] != '\0'; ++i)
    {
        v3 position;
        m4x4 model;
        int ascii_code;

        char c = str[i];
        if (c == '\n')
        {
            yOffset -= dimensions.y;
            xOffset = offsets.x;
            continue;
        }

        position = vm_v3(xOffset, ((float)state->height - dimensions.y) - yOffset, 0.0f);
        model = vm_m4x4_scale(vm_m4x4_translate(vm_m4x4_identity, position), vm_v3(dimensions.x, dimensions.y, 1.0f));

        ascii_code = c - 32;
        speg_draw_call_append(call, &model, &color, ascii_code);

        xOffset += dimensions.x;
    }

    result.x = xOffset;
    result.y = yOffset;

    return (result);
}

typedef struct wheel
{
    v3 local_position;

    /* Transform for setting wheel position rotation according to parent (car) position */
    transformation transform;

    /* Force 1: Suspension */
    float suspension_rest_dist;
    float suspension_ray_dist;
    float suspension_spring_strength;
    float suspension_spring_dampening;

    /* Force 2: Steering */
    bool steering_enabled;
    bool steering_inverted;
    float steering_wheel_mass;
    float steering_wheel_grip;

    /* Force 3: Acceleration */
    bool acceleration_enabled;
    float acceleration_input;

    /* Calculated */
    float distance_to_ground;

} wheel;

wheel wheel_init_with_defaults(v3 local_position, float wheel_mass, bool steering_enabled, bool steering_inverted, bool acceleration_enabled)
{
    wheel result;

    result.local_position = local_position;
    result.transform = vm_transformation_init();
    result.suspension_rest_dist = 0.5f;
    result.suspension_ray_dist = 0.3f;
    result.suspension_spring_strength = 30000.0f;
    result.suspension_spring_dampening = 2500.0f;
    result.steering_enabled = steering_enabled;
    result.steering_inverted = steering_inverted;
    result.steering_wheel_mass = wheel_mass;
    result.steering_wheel_grip = 0.9f;
    result.acceleration_enabled = acceleration_enabled;
    result.acceleration_input = 0.0f;

    return (result);
}

void wheel_update(
    rigid_body *car,
    wheel *wh,
    float dt,
    float car_top_speed,
    int index,
    speg_draw_call *call_txt,
    speg_state *state,
    speg_platform_api *platformApi)
{
    v3 wheel_position = wh->transform.position;
    v3 wheel_world_vel = rigid_body_point_velocity(car, wheel_position);

    /* Force 1: calculate suspension force*/
    {
        /* world-space direction of the spring force */
        v3 spring_dir = vm_transformation_up(&wh->transform);

        float offset = wh->suspension_rest_dist - wh->distance_to_ground;
        float velocity = vm_v3_dot(spring_dir, wheel_world_vel);
        float force = (offset * wh->suspension_spring_strength) - (velocity * wh->suspension_spring_dampening);

        v3 force_suspension = vm_v3_mulf(spring_dir, force);

        /* Text rendering force information */
        {
            char xBuffer[32], yBuffer[32], zBuffer[32], outBuffer[256];
            v2 txt_dimensions = vm_v2_mulf(vm_v2(17.0f, 32.0f), 0.6f);
            v2 txt_offset = vm_v2(10.0f, 200.0f + ((float)(index + 1) * txt_dimensions.y));
            speg_float_to_string(force_suspension.x, xBuffer, 6);
            speg_float_to_string(force_suspension.y, yBuffer, 6);
            speg_float_to_string(force_suspension.z, zBuffer, 6);
            platformApi->platform_format_string(outBuffer, "[wh-%i]   force_suspension: %14s %14s %14s", index, xBuffer, yBuffer, zBuffer);
            render_full_text(call_txt, state, outBuffer, vm_v3_one, txt_dimensions, txt_offset);
        }

        rigid_body_apply_force_at_position(car, force_suspension, wheel_position);
    }

    /* Force 2: calculate steering force */
    {
        /* world-space direction of the spring force */
        v3 steering_dir = vm_transformation_right(&wh->transform);

        float steering_vel = vm_v3_dot(steering_dir, wheel_world_vel);

        /* gripFactor is in range 0-1, 0 means no grip, 1 means full grip */
        float desired_velocity_change = -steering_vel * wh->steering_wheel_grip;
        float desired_acceleration = desired_velocity_change / dt;

        v3 force_steering = vm_v3_mulf(steering_dir, wh->steering_wheel_mass * desired_acceleration);

        /* Text rendering force information */
        {
            char xBuffer[32], yBuffer[32], zBuffer[32], outBuffer[256];
            v2 txt_dimensions = vm_v2_mulf(vm_v2(17.0f, 32.0f), 0.6f);
            v2 txt_offset = vm_v2(10.0f, 300.0f + ((float)(index + 1) * txt_dimensions.y));
            speg_float_to_string(force_steering.x, xBuffer, 6);
            speg_float_to_string(force_steering.y, yBuffer, 6);
            speg_float_to_string(force_steering.z, zBuffer, 6);
            platformApi->platform_format_string(outBuffer, "[wh-%i]     force_steering: %14s %14s %14s", index, xBuffer, yBuffer, zBuffer);
            render_full_text(call_txt, state, outBuffer, vm_v3_one, txt_dimensions, txt_offset);
        }

        rigid_body_apply_force_at_position(car, force_steering, wheel_position);
    }

    /* Force 3: acceleration / braking */
    {
        /* world-space direction of the acceleration/braking force. */
        v3 acceleration_dir = vm_transformation_forward(&wh->transform);

        float car_speed = vm_v3_dot(rigid_body_forward(car), car->velocity);
        float normalized_speed = vm_clamp01f(vm_absf(car_speed) / car_top_speed);
        float available_torque = simple_power_curve_evaluate(normalized_speed) * wh->acceleration_input;

        v3 force_acceleration = vm_v3_mulf(acceleration_dir, available_torque);

        /* Text rendering force information */
        {
            char xBuffer[32], yBuffer[32], zBuffer[32], outBuffer[256];
            v2 txt_dimensions = vm_v2_mulf(vm_v2(17.0f, 32.0f), 0.6f);
            v2 txt_offset = vm_v2(10.0f, 400.0f + ((float)(index + 1) * txt_dimensions.y));
            speg_float_to_string(force_acceleration.x, xBuffer, 6);
            speg_float_to_string(force_acceleration.y, yBuffer, 6);
            speg_float_to_string(force_acceleration.z, zBuffer, 6);
            platformApi->platform_format_string(outBuffer, "[wh-%i] force_acceleration: %14s %14s %14s", index, xBuffer, yBuffer, zBuffer);
            render_full_text(call_txt, state, outBuffer, vm_v3_one, txt_dimensions, txt_offset);
        }

        rigid_body_apply_force_at_position(car, force_acceleration, wheel_position);
    }
}

static bool car_initialized;
static rigid_body car;

void render_car(speg_draw_call *call, speg_draw_call *call_txt, speg_state *state, speg_platform_api *platformApi)
{
    float dt = (float)state->dt;
    float gravity = -9.81f;
    v3 gravity_force = vm_v3(0.0f, gravity, 0.0f);

    float car_top_speed = 20.0f;
    static float steering_angle = -0.3f;

    int i;

    transformation car_transform;
    m4x4 car_model;
    v3 car_color = vm_v3(0.4f, 0.4f, 0.4f);

#define NUM_WHEELS 4
    static wheel wheels[NUM_WHEELS];

    (void)platformApi;

    if (!car_initialized)
    {
        car = rigid_body_init(
            vm_v3(0.0f, 5.0f, 0.0f),         /* car position */
            vm_quat(0.0f, 0.0f, 0.0f, 1.0f), /* car orientation */
            1200.0f,                         /* car mass */
            2500.0f                          /* car inertia */
        );

        /* AWD drive train */
        wheels[0] = wheel_init_with_defaults(vm_v3(-1.0f, 0.0f, -1.0f), car.mass / NUM_WHEELS, true, false, true); /* Front-Left */
        wheels[1] = wheel_init_with_defaults(vm_v3(1.0f, 0.0f, -1.0f), car.mass / NUM_WHEELS, true, false, true);  /* Front-Right */
        wheels[2] = wheel_init_with_defaults(vm_v3(-1.0f, 0.0f, 1.0f), car.mass / NUM_WHEELS, false, true, true);  /* Rear-Left */
        wheels[3] = wheel_init_with_defaults(vm_v3(1.0f, 0.0f, 1.0f), car.mass / NUM_WHEELS, false, true, true);   /* Rear-Right */

        car_initialized = true;
    }

    /* Update the car transform so that the wheel transforms are also updated in the next step */
    car.force = vm_v3_add(car.force, vm_v3_mulf(gravity_force, car.mass));

    /* For each wheel */
    for (i = 0; i < NUM_WHEELS; ++i)
    {
        float current_ground_height = 0.0f;
        m4x4 wheel_model;
        v3 wheel_color = vm_v3(1.0f, 0.0f, 0.0f);

        /* TODO: check if rotate has to be applied after add !*/
        wheel wh = wheels[i];
        wh.transform.position = vm_v3_add(car.position, vm_v3_rotate(wh.local_position, car.orientation));
        wh.transform.rotation = wh.steering_enabled ? vm_quat_mul(car.orientation, vm_quat_rotate(vm_transformation_up(&wh.transform), wh.steering_inverted ? -steering_angle : steering_angle)) : car.orientation;
        wh.acceleration_input = wh.acceleration_enabled ? 1.0f : 0.0f;
        wh.distance_to_ground = wh.transform.position.y - current_ground_height;

        if (wh.distance_to_ground < wh.suspension_rest_dist + wh.suspension_ray_dist)
        {
            wheel_update(
                &car,
                &wh,
                dt,
                car_top_speed,
                i,
                call_txt,
                state,
                platformApi);
        }

        /* Visualizing the wheel up, forward, right vector */
        render_vector(call, wh.transform.position, vm_transformation_up(&wh.transform), vm_v3(0.0f, 1.0f, 0.0f));
        render_vector(call, wh.transform.position, vm_transformation_forward(&wh.transform), vm_v3(0.0f, 0.0f, 1.0f));
        render_vector(call, wh.transform.position, vm_transformation_right(&wh.transform), vm_v3(1.0f, 0.0f, 0.0f));

        /* Text Wheel information */
        {
            v2 txt_dimensions = vm_v2_mulf(vm_v2(17.0f, 32.0f), 0.6f);
            v2 txt_offset = vm_v2(10.0f, 20.0f + ((float)(i + 1) * txt_dimensions.y));
            v3 txt_color = vm_v3_one;
            char xBuffer[32], yBuffer[32], zBuffer[32], wBuffer[32], iBuffer[6], outBuffer[512];

            speg_float_to_string(wh.transform.position.x, xBuffer, 6);
            speg_float_to_string(wh.transform.position.y, yBuffer, 6);
            speg_float_to_string(wh.transform.position.z, zBuffer, 6);
            speg_float_to_string(wh.distance_to_ground, wBuffer, 6);
            speg_float_to_string((float)i, iBuffer, 0);

            platformApi->platform_format_string(outBuffer, "   [wh-%i] %10s %10s %10s dtg: %10s\n", i, xBuffer, yBuffer, zBuffer, wBuffer);
            txt_offset = render_full_text(call_txt, state, outBuffer, txt_color, txt_dimensions, txt_offset);
        }

        wh.transform.scale = vm_v3f(0.2f);
        wheel_model = vm_transformation_matrix(&wh.transform);
        speg_draw_call_append(call, &wheel_model, &wheel_color, default_texture_index);
    }

    /* Car information rendering */
    {
        char xBuffer[32], yBuffer[32], zBuffer[32], outBuffer[256];
        v2 txt_dimensions = vm_v2_mulf(vm_v2(17.0f, 32.0f), 0.6f);
        v2 txt_offset = vm_v2(10.0f, 520.0f + txt_dimensions.y);
        speg_float_to_string(car.force.x, xBuffer, 6);
        speg_float_to_string(car.force.y, yBuffer, 6);
        speg_float_to_string(car.force.z, zBuffer, 6);
        platformApi->platform_format_string(outBuffer, " [car_force] %14s %14s %14s\n", xBuffer, yBuffer, zBuffer);
        txt_offset = render_full_text(call_txt, state, outBuffer, vm_v3_one, txt_dimensions, txt_offset);

        speg_float_to_string(car.torque.x, xBuffer, 6);
        speg_float_to_string(car.torque.y, yBuffer, 6);
        speg_float_to_string(car.torque.z, zBuffer, 6);
        platformApi->platform_format_string(outBuffer, "[car_torque] %14s %14s %14s", xBuffer, yBuffer, zBuffer);
        render_full_text(call_txt, state, outBuffer, vm_v3_one, txt_dimensions, txt_offset);
    }

    rigid_body_integrate(&car, dt);

    /* Text Car information */
    {
        v2 txt_dimensions = vm_v2_mulf(vm_v2(17.0f, 32.0f), 0.6f);
        v2 txt_offset = vm_v2(10.0f, 20.0f + ((float)(4 + 1) * txt_dimensions.y));
        v3 txt_color = vm_v3_one;
        char xBuffer[32], yBuffer[32], zBuffer[32], wBuffer[32], outBuffer[512];

        speg_float_to_string(car.position.x, xBuffer, 6);
        speg_float_to_string(car.position.y, yBuffer, 6);
        speg_float_to_string(car.position.z, zBuffer, 6);

        platformApi->platform_format_string(outBuffer, "[car_pos] %10s %10s %10s\n", xBuffer, yBuffer, zBuffer);
        render_full_text(call_txt, state, outBuffer, txt_color, txt_dimensions, txt_offset);

        speg_float_to_string(car.orientation.x, xBuffer, 6);
        speg_float_to_string(car.orientation.y, yBuffer, 6);
        speg_float_to_string(car.orientation.z, zBuffer, 6);
        speg_float_to_string(car.orientation.w, wBuffer, 6);

        txt_offset = vm_v2(10.0f, 20.0f + ((float)(5 + 1) * txt_dimensions.y));

        platformApi->platform_format_string(outBuffer, "[car_rot] %10s %10s %10s %10s\n", xBuffer, yBuffer, zBuffer, wBuffer);
        render_full_text(call_txt, state, outBuffer, txt_color, txt_dimensions, txt_offset);

        speg_float_to_string(car.velocity.x, xBuffer, 6);
        speg_float_to_string(car.velocity.y, yBuffer, 6);
        speg_float_to_string(car.velocity.z, zBuffer, 6);

        txt_offset = vm_v2(10.0f, 20.0f + ((float)(6 + 1) * txt_dimensions.y));
        platformApi->platform_format_string(outBuffer, "[car_vel] %10s %10s %10s\n", xBuffer, yBuffer, zBuffer);
        render_full_text(call_txt, state, outBuffer, txt_color, txt_dimensions, txt_offset);

        speg_float_to_string(car.angularVelocity.x, xBuffer, 6);
        speg_float_to_string(car.angularVelocity.y, yBuffer, 6);
        speg_float_to_string(car.angularVelocity.z, zBuffer, 6);

        txt_offset = vm_v2(10.0f, 20.0f + ((float)(7 + 1) * txt_dimensions.y));
        platformApi->platform_format_string(outBuffer, "[car_ang] %10s %10s %10s\n", xBuffer, yBuffer, zBuffer);
        render_full_text(call_txt, state, outBuffer, txt_color, txt_dimensions, txt_offset);

        txt_offset = vm_v2(10.0f, 20.0f + ((float)(8 + 1) * txt_dimensions.y));

        speg_float_to_string(vm_v3_length(car.velocity), xBuffer, 6);

        platformApi->platform_format_string(outBuffer, "[car_spd] %10s\n", xBuffer);
        render_full_text(call_txt, state, outBuffer, txt_color, txt_dimensions, txt_offset);
    }

    /* Visualizing the car up, forward, right vector */
    render_vector(call, car.position, vm_v3_mulf(rigid_body_up(&car), 2.0f), vm_v3(0.0f, 1.0f, 0.0f));
    render_vector(call, car.position, vm_v3_mulf(rigid_body_forward(&car), 2.0f), vm_v3(0.0f, 0.0f, 1.0f));
    render_vector(call, car.position, vm_v3_mulf(rigid_body_right(&car), 2.0f), vm_v3(1.0f, 0.0f, 0.0f));
    render_vector(call, car.position, car.velocity, vm_v3(0.941f, 0.925f, 0.0f));
    render_vector(call, car.position, car.angularVelocity, vm_v3(0.941f, 0.925f, 0.0f));

    /* Visualize car not just as a cube. Doesn't affect the physics simulation !*/
    car_transform = vm_transformation_init();
    car_transform.position = car.position;
    car_transform.position.y += 0.5f;
    car_transform.rotation = car.orientation;
    car_transform.scale.y = 0.2f;
    car_transform.scale.x = 1.5f;
    car_transform.scale.z = 2.0f;

    car_model = vm_transformation_matrix(&car_transform);

    speg_draw_call_append(call, &car_model, &car_color, default_texture_index);
}

/* Draw call batching groups */
#define MAX_STATIC_INSTANCES 22000
static float all_static_models[MAX_STATIC_INSTANCES * VM_M4X4_ELEMENT_COUNT];
static float all_static_colors[MAX_STATIC_INSTANCES * VM_V3_ELEMENT_COUNT];
static int all_static_texture_indices[MAX_STATIC_INSTANCES];
static speg_draw_call draw_call_static = {0};

#define MAX_DYNAMIC_INSTANCES 2048
static float all_dynamic_models[MAX_DYNAMIC_INSTANCES * VM_M4X4_ELEMENT_COUNT];
static float all_dynamic_colors[MAX_DYNAMIC_INSTANCES * VM_V3_ELEMENT_COUNT];
static int all_dynamic_texture_indices[MAX_DYNAMIC_INSTANCES];
static speg_draw_call draw_call_dynamic = {0};

#define MAX_DYNAMIC_GUI_INSTANCES 128
static float all_dynamic_gui_models[MAX_DYNAMIC_GUI_INSTANCES * VM_M4X4_ELEMENT_COUNT];
static float all_dynamic_gui_colors[MAX_DYNAMIC_GUI_INSTANCES * VM_V3_ELEMENT_COUNT];
static int all_dynamic_gui_texture_indices[MAX_DYNAMIC_GUI_INSTANCES];
static speg_draw_call draw_call_dynamic_gui = {0};

#define MAX_DYNAMIC_TEXT_INSTANCES 2048
static float all_text_models[MAX_DYNAMIC_TEXT_INSTANCES * VM_M4X4_ELEMENT_COUNT];
static float all_text_colors[MAX_DYNAMIC_TEXT_INSTANCES * VM_V3_ELEMENT_COUNT];
static int all_text_indices[MAX_DYNAMIC_TEXT_INSTANCES];
static speg_draw_call draw_call_text = {0};

/* MESH definition for each speg_draw_call */
#define SPEG_INIT_MESH(name, culling, verts, indices, uvs) {name, false, culling, verts, sizeof(verts), indices, sizeof(indices), uvs, sizeof(uvs), array_size(indices), 0, 0, 0, 0, 0, 0, 0}

static speg_mesh cube_static = SPEG_INIT_MESH("cube_static", true, cube_vertices, cube_indices, cube_uvs);
static speg_mesh cube_dynamic = SPEG_INIT_MESH("cube_dynamic", true, cube_vertices, cube_indices, cube_uvs);
static speg_mesh rectangle_static = SPEG_INIT_MESH("rectangle_static", false, rectangle_vertices, rectangle_indices, rectangle_uvs);
static speg_mesh rectangle_text = SPEG_INIT_MESH("rectangle_text", false, rectangle_vertices, rectangle_indices, rectangle_uvs);

void speg_update(speg_memory *memory, platform_controller_input *platform_input, speg_platform_api *platformApi)
{
    static camera cam;

    speg_state *state = (speg_state *)memory->permanentMemory;
    speg_controller_input input;

    static m4x4 projection;
    static m4x4 ortho_proj;
    static m4x4 view;
    static m4x4 projection_view;
    static m4x4 view_simulated;

    assert(memory);
    assert(memory->permanentMemorySize > 0);
    assert(memory->permanentMemory);
    assert(platform_input);
    assert(platformApi);

    /* Initialized only once at startup */
    if (!memory->initialized)
    {
        cam = camera_init();
        cam.position.z = 13.0f;
        cam.position.y = 2.0f;

        memory->initialized = true;

        state->clearColorR = 0.2f;
        state->clearColorG = 0.2f;
        state->clearColorB = 0.2f;

        /* Static Cubes */
        draw_call_static.mesh = &cube_static;
        draw_call_static.count_instances_max = MAX_STATIC_INSTANCES;
        draw_call_static.count_instances = 0;
        draw_call_static.models = all_static_models;
        draw_call_static.colors = all_static_colors;
        draw_call_static.texture_indices = all_static_texture_indices;
        draw_call_static.changed = false;

        /* Dynamic Cubes */
        draw_call_dynamic.mesh = &cube_dynamic;
        draw_call_dynamic.count_instances_max = MAX_DYNAMIC_INSTANCES;
        draw_call_dynamic.count_instances = 0;
        draw_call_dynamic.models = all_dynamic_models;
        draw_call_dynamic.colors = all_dynamic_colors;
        draw_call_dynamic.texture_indices = all_dynamic_texture_indices;
        draw_call_dynamic.changed = true;

        /* 2D GUI Elements */
        draw_call_dynamic_gui.mesh = &rectangle_static;
        draw_call_dynamic_gui.count_instances_max = MAX_DYNAMIC_GUI_INSTANCES;
        draw_call_dynamic_gui.count_instances = 0;
        draw_call_dynamic_gui.models = all_dynamic_gui_models;
        draw_call_dynamic_gui.colors = all_dynamic_gui_colors;
        draw_call_dynamic_gui.texture_indices = all_dynamic_gui_texture_indices;
        draw_call_dynamic_gui.changed = true;
        draw_call_dynamic_gui.is_2d = true;

        /* 3D Text */
        draw_call_text.mesh = &rectangle_text;
        draw_call_text.count_instances_max = MAX_DYNAMIC_TEXT_INSTANCES;
        draw_call_text.count_instances = 0;
        draw_call_text.models = all_text_models;
        draw_call_text.colors = all_text_colors;
        draw_call_text.texture_indices = all_text_indices;
        draw_call_text.changed = true;
        draw_call_text.is_2d = true;

        /* Static scenes */
        PROFILE(render_coordinate_axis(&draw_call_static));
        PROFILE(render_grid(&draw_call_static));
        PROFILE(render_cubes_instanced(&draw_call_static, 100.0f));

        platformApi->platform_print_console(__FILE__, __LINE__, "[speg] initialized\n");
    }

    input = speg_map_controller_input(platform_input);

    /* Debug mode. */
    {
        static bool debug;
        static bool debug_run_step;

        if (input.debug_mode.pressed)
        {
            if (!debug)
            {
                platformApi->platform_print_console(__FILE__, __LINE__, "[speg] #######################################\n");
                platformApi->platform_print_console(__FILE__, __LINE__, "[speg] # DEBUG MODE\n");
                platformApi->platform_print_console(__FILE__, __LINE__, "[speg] # Press 'TAB' to exit.\n");
                platformApi->platform_print_console(__FILE__, __LINE__, "[speg] # Press 'I' to run 1 step.\n");
                platformApi->platform_print_console(__FILE__, __LINE__, "[speg] # Hold  'U' to continuously run 1 step.\n");
                platformApi->platform_print_console(__FILE__, __LINE__, "[speg] #######################################\n");
                debug = true;
            }
            else
            {
                platformApi->platform_print_console(__FILE__, __LINE__, "[speg] #######################################\n");
                platformApi->platform_print_console(__FILE__, __LINE__, "[speg] # Exit DEBUG MODE\n");
                platformApi->platform_print_console(__FILE__, __LINE__, "[speg] #######################################\n");
                debug = false;
            }
        }

        debug_run_step = input.debug_mode_step.pressed || input.debug_mode_step_continuously.endedDown;

        if (debug && !debug_run_step)
        {
            /* Draw existing static and dynamic scenes */
            platformApi->platform_draw(&draw_call_static, projection_view.e);
            platformApi->platform_draw(&draw_call_dynamic, projection_view.e);
            platformApi->platform_draw(&draw_call_dynamic_gui, ortho_proj.e);
            platformApi->platform_draw(&draw_call_text, ortho_proj.e);

            return;
        }
    }

    /* Reset dynamic draw call buffers */
    draw_call_dynamic.count_instances = 0;
    draw_call_dynamic_gui.count_instances = 0;
    draw_call_text.count_instances = 0;

    camera_update_movement(&input, &cam, 10.0f * (float)state->dt);

    projection = vm_m4x4_perspective(vm_radf(cam.fov), (float)state->width / (float)state->height, 0.1f, 1000.0f);
    view = vm_m4x4_lookAt(cam.position, vm_v3_add(cam.position, cam.front), cam.up);
    ortho_proj = vm_m4x4_orthographic(0.0f, (float)state->width, 0.0f, (float)state->height, -1.0f, 1.0f);

    if (input.cameraSimulate.active)
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

    /* Dynamic scenes */
    render_cubes(&draw_call_dynamic, projection, view_simulated, state, &input, 20.0f, &cam);
    render_transformations_test(&draw_call_dynamic, state);
    render_gui_rectangle(&draw_call_dynamic_gui, state, &input);
    render_text(&draw_call_text, state, platformApi);
    render_car(&draw_call_dynamic, &draw_call_text, state, platformApi);

    state->renderedObjects = (unsigned int)(draw_call_static.count_instances +
                                            draw_call_dynamic.count_instances +
                                            draw_call_dynamic_gui.count_instances +
                                            draw_call_text.count_instances);

    projection_view = vm_m4x4_mul(projection, view);

    /* Draw static and dynamic scenes */
    platformApi->platform_draw(&draw_call_static, projection_view.e);
    platformApi->platform_draw(&draw_call_dynamic, projection_view.e);
    platformApi->platform_draw(&draw_call_dynamic_gui, ortho_proj.e);
    platformApi->platform_draw(&draw_call_text, ortho_proj.e);
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
