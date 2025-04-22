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
            "[speg-profiler] cycles: %8d, ms: %12s, \"%s\"\n",                   \
            (__endCycles - __startCycles),                                       \
            floatBuffer,                                                         \
            name);                                                               \
    } while (0)

#define SPEG_INIT_MESH(name, culling, verts, indices, uvs) {name, false, culling, verts, sizeof(verts), indices, sizeof(indices), uvs, sizeof(uvs), array_size(indices)}

static speg_mesh cube_static = SPEG_INIT_MESH("cube_static", true, cube_vertices, cube_indices, cube_uvs);
static speg_mesh cube_dynamic = SPEG_INIT_MESH("cube_dynamic", true, cube_vertices, cube_indices, cube_uvs);
static speg_mesh rectangle_static = SPEG_INIT_MESH("rectangle_static", false, rectangle_vertices, rectangle_indices, rectangle_uvs);
static speg_mesh rectangle_text = SPEG_INIT_MESH("rectangle_text", false, rectangle_vertices, rectangle_indices, rectangle_uvs);

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

void spawn_random_cube(int i, float range, v3 *position, v3 *color)
{
    static const float color_scale = 1.0f / 255.0f;
    unsigned int c = (i == 0 ? 1 : i) * 10000000;
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

#define MAX_STATIC_INSTANCES 22000
static float all_static_models[MAX_STATIC_INSTANCES * VM_M4X4_ELEMENT_COUNT];
static float all_static_colors[MAX_STATIC_INSTANCES * VM_V3_ELEMENT_COUNT];
static int all_static_texture_indices[MAX_STATIC_INSTANCES];
static speg_draw_call draw_call_static = {0};

#define MAX_DYNAMIC_INSTANCES 1024
static float all_dynamic_models[MAX_DYNAMIC_INSTANCES * VM_M4X4_ELEMENT_COUNT];
static float all_dynamic_colors[MAX_DYNAMIC_INSTANCES * VM_V3_ELEMENT_COUNT];
static int all_dynamic_texture_indices[MAX_DYNAMIC_INSTANCES];
static speg_draw_call draw_call_dynamic = {0};

#define MAX_DYNAMIC_GUI_INSTANCES 128
static float all_dynamic_gui_models[MAX_DYNAMIC_GUI_INSTANCES * VM_M4X4_ELEMENT_COUNT];
static float all_dynamic_gui_colors[MAX_DYNAMIC_GUI_INSTANCES * VM_V3_ELEMENT_COUNT];
static int all_dynamic_gui_texture_indices[MAX_DYNAMIC_GUI_INSTANCES];
static speg_draw_call draw_call_dynamic_gui = {0};

#define MAX_DYNAMIC_TEXT_INSTANCES 1024
static float all_text_models[MAX_DYNAMIC_TEXT_INSTANCES * VM_M4X4_ELEMENT_COUNT];
static float all_text_colors[MAX_DYNAMIC_TEXT_INSTANCES * VM_V3_ELEMENT_COUNT];
static int all_text_indices[MAX_DYNAMIC_TEXT_INSTANCES];
static speg_draw_call draw_call_text = {0};

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
        random_char = (char)(((lcg_state / 65536) % 32768) % (printable_ascii_end - printable_ascii_start + 1)) + printable_ascii_start;

        str[i] = random_char;
    }
    str[length] = '\0';
}

void render_text(speg_draw_call *call, speg_state *state, speg_platform_api *platformApi)
{
    float scale = 0.8f;
    v2 size = vm_v2(17.0f * scale, 32.0f * scale);
    v3 red = vm_v3(1.0f, 0.0f, 0.0f);
    v3 green = vm_v3(0.0f, 1.0f, 0.0f);
    v3 blue = vm_v3(0.0f, 0.0f, 1.0f);
    v3 black = vm_v3(0.0f, 0.0f, 0.0f);

    int i;

    float xOffset = 0.0f;
    float yOffset = 0.0f;

    double startTimeNano;
    double endTimeNano;
    unsigned long startCycles;
    unsigned long endCycles;
    float msPassed;
    char floatBuffer[32];

    const char *str = "Hello, world!\ntest_from_pure_c89 nostdlib :)\n!\"%&/()=?{}[]*+,.:,<>@^_|~\n0123456789\nabcdefghijklmnopqrstuvwxyz\nABCDEFGHIJKLMNOPQRSTUVWXYZ";
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
            xOffset = 0.0f;
            continue;
        }

        render_character(call, state, c, i % 3 ? green : (i % 5 ? blue : red), size, xOffset, yOffset);
        xOffset += size.x;
    }

    xOffset = 0.0f;
    yOffset -= 2 * size.y;

    for (i = 0; random_string[i] != '\0'; ++i)
    {
        static const float color_scale = 1.0f / 255.0f;
        unsigned int n = (i == 0 ? 1 : i) * 3000000;
        float r = ((float)(((n >> 16) & 0xFF) >> 1)) * color_scale;
        float g = ((float)((n >> 8) & 0xFF)) * color_scale;
        float b = ((float)(n & 0xFF)) * color_scale;
        v3 random_color = vm_v3(r, g, b);

        char c = random_string[i];
        if (c == '\n')
        {
            yOffset -= size.y;
            xOffset = 0.0f;
            continue;
        }

        render_character(call, state, c, random_color, size, xOffset, yOffset);
        xOffset += size.x;
    }

    endCycles = platformApi->platform_perf_current_cycle_count();
    endTimeNano = platformApi->platform_perf_current_time_nanoseconds();
    msPassed = (float)((endTimeNano - startTimeNano) / 1000000.0);

    speg_float_to_string(msPassed, floatBuffer, 6);

    xOffset = 0.0f;
    yOffset -= 2 * size.y;

    for (i = 0; floatBuffer[i] != '\0'; ++i)
    {
        char c = floatBuffer[i];
        if (c == '\n')
        {
            yOffset -= size.y;
            xOffset = 0.0f;
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

    xOffset = 0.0f;
    yOffset -= 2 * size.y;

    speg_float_to_string((float)(endCycles - startCycles), floatBuffer, 0);

    for (i = 0; floatBuffer[i] != '\0'; ++i)
    {
        char c = floatBuffer[i];
        if (c == '\n')
        {
            yOffset -= size.y;
            xOffset = 0.0f;
            continue;
        }

        render_character(call, state, c, black, size, xOffset, yOffset);
        xOffset += size.x;
    }

    render_character(call, state, ' ', black, size, xOffset, yOffset);
    xOffset += size.x;

    render_character(call, state, 'C', black, size, xOffset, yOffset);
    xOffset += size.x;

    render_character(call, state, 'P', black, size, xOffset, yOffset);
    xOffset += size.x;

    render_character(call, state, 'U', black, size, xOffset, yOffset);
    xOffset += size.x;

    render_character(call, state, ' ', black, size, xOffset, yOffset);
    xOffset += size.x;

    render_character(call, state, 'c', black, size, xOffset, yOffset);
    xOffset += size.x;

    render_character(call, state, 'y', black, size, xOffset, yOffset);
    xOffset += size.x;

    render_character(call, state, 'c', black, size, xOffset, yOffset);
    xOffset += size.x;

    render_character(call, state, 'l', black, size, xOffset, yOffset);
    xOffset += size.x;

    render_character(call, state, 'e', black, size, xOffset, yOffset);
    xOffset += size.x;

    render_character(call, state, 's', black, size, xOffset, yOffset);
    xOffset += size.x;
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
        draw_call_static.texture_indices = all_static_texture_indices;
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
    draw_call_dynamic.texture_indices = all_dynamic_texture_indices;
    draw_call_dynamic.changed = true;

    draw_call_dynamic_gui.mesh = &rectangle_static;
    draw_call_dynamic_gui.count_instances_max = array_size(all_dynamic_gui_models);
    draw_call_dynamic_gui.count_instances = 0;
    draw_call_dynamic_gui.models = all_dynamic_gui_models;
    draw_call_dynamic_gui.colors = all_dynamic_gui_colors;
    draw_call_dynamic_gui.texture_indices = all_dynamic_gui_texture_indices;
    draw_call_dynamic_gui.changed = true;
    draw_call_dynamic_gui.is_2d = true;

    /* Text */
    draw_call_text.mesh = &rectangle_text;
    draw_call_text.count_instances_max = array_size(all_text_models);
    draw_call_text.count_instances = 0;
    draw_call_text.models = all_text_models;
    draw_call_text.colors = all_text_colors;
    draw_call_text.texture_indices = all_text_indices;
    draw_call_text.changed = true;
    draw_call_text.is_2d = true;

    /* Dynamic scenes */
    render_cubes(&draw_call_dynamic, projection, view_simulated, state, input, 20.0f, &cam);
    render_transformations_test(&draw_call_dynamic, state);
    render_gui_rectangle(&draw_call_dynamic_gui, state, input);
    render_text(&draw_call_text, state, platformApi);

    state->renderedObjects =
        draw_call_static.count_instances +
        draw_call_dynamic.count_instances +
        draw_call_dynamic_gui.count_instances +
        draw_call_text.count_instances;

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
