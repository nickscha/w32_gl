#include "../../w32_gl.h"        /* Everything needed for Windows window/screen/opengl/mouse initialization */
#include "../w32_open_gl_func.h" /* Load OpenGl functions not directly provided by windows (e.g. OPEN_GL_VERSION > 1.1) */

/* The platform independent nostdlib application code/logic */
#define SPEG_IMPORT
#include "speg.h"

typedef struct w32_type_llu
{
  unsigned long low_part;
  unsigned long high_part;
} w32_type_llu;

double w32_type_llu_to_double(w32_type_llu a)
{
  return ((double)a.high_part * 4294967296.0 + (double)a.low_part);
}

unsigned long w32_rdtsc(void)
{
  w32_type_llu rdtsc_value = {0};
  __asm __volatile("rdtsc" : "=a"(rdtsc_value.low_part), "=d"(rdtsc_value.high_part));
  return ((unsigned long)w32_type_llu_to_double(rdtsc_value));
}

static bool globalRunning = true;
static bool globalPause = false;
static bool vsync = true;
static bool wireframeMode = false;

int width = 800;
int height = 600;
double dt = 0.0;
HWND window;

static unsigned int drawCallsPerFrame = 0;
static int occludedObjectsPerFrame = 0;

typedef struct File
{
  uint32_t size;
  char *content;
} File;

File w32_read_entire_file(const char *path)
{
  File tmp = {0};

  HANDLE hFile;
  DWORD fileSize, bytesRead;
  char *buffer;

  hFile = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  if (hFile == INVALID_HANDLE_VALUE)
  {
    win32_print_console("[win32] invalid handle for file %s. Waiting shortly before retry!\n", path);
    Sleep(10);
    hFile = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
      win32_print_console("[win32] invalid handle for second retry for file %s\n", path);
      assert(hFile == INVALID_HANDLE_VALUE);
      return tmp;
    }
  }

  fileSize = GetFileSize(hFile, NULL);
  if (fileSize == INVALID_FILE_SIZE)
  {
    CloseHandle(hFile);
    return tmp;
  }

  buffer = (char *)HeapAlloc(GetProcessHeap(), 0, fileSize + 1); /* +1 for null terminator */
  if (!buffer)
  {
    CloseHandle(hFile);
    return tmp;
  }

  if (!ReadFile(hFile, buffer, fileSize, &bytesRead, NULL) || bytesRead != fileSize)
  {
    HeapFree(GetProcessHeap(), 0, buffer);
    CloseHandle(hFile);
    return tmp;
  }

  buffer[fileSize] = '\0';
  tmp.size = fileSize;
  tmp.content = buffer;

  CloseHandle(hFile);

  return tmp;
}

static const FILETIME empty = {0, 0};
FILETIME w32_file_mod_time(const char *file)
{
  WIN32_FILE_ATTRIBUTE_DATA fad;
  return GetFileAttributesExA(file, GetFileExInfoStandard, &fad) ? fad.ftLastWriteTime : empty;
}

typedef struct speg_shader
{
  unsigned int program;
  FILETIME vsTime;
  FILETIME fsTime;
  char *vsFile;
  char *fsFile;
} speg_shader;

typedef struct speg_shaders
{

  speg_shader instanced;

} speg_shaders;

static speg_shaders shaders;

unsigned int shader_compile(const char *shaderCode, GLenum shaderType)
{

  unsigned int shaderId = glCreateShader(shaderType);
  glShaderSource(shaderId, 1, &shaderCode, NULL);
  glCompileShader(shaderId);

  int success;
  glGetShaderiv(shaderId, GL_COMPILE_STATUS, &success);

  if (!success)
  {
    char infoLog[1024];
    glGetShaderInfoLog(shaderId, 1024, NULL, infoLog);
    win32_printf(infoLog);
  }

  return shaderId;
}

speg_shader shader_load(char *vertexShaderFile, char *fragmentShaderFile)
{
  speg_shader result;
  char *vertexShaderCode;
  char *fragmentShaderCode;
  unsigned int vertexShader;
  unsigned int fragmentShader;
  unsigned int shaderProgram;
  int success;

  win32_print_console("[win32] load and compile shaders: %s, %s\n", vertexShaderFile, fragmentShaderFile);

  result.vsTime = w32_file_mod_time(vertexShaderFile);
  result.fsTime = w32_file_mod_time(fragmentShaderFile);
  result.vsFile = vertexShaderFile;
  result.fsFile = fragmentShaderFile;

  vertexShaderCode = w32_read_entire_file(vertexShaderFile).content;
  fragmentShaderCode = w32_read_entire_file(fragmentShaderFile).content;

  vertexShader = shader_compile(vertexShaderCode, GL_VERTEX_SHADER);
  fragmentShader = shader_compile(fragmentShaderCode, GL_FRAGMENT_SHADER);

  HeapFree(GetProcessHeap(), 0, vertexShaderCode);
  HeapFree(GetProcessHeap(), 0, fragmentShaderCode);

  shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  glLinkProgram(shaderProgram);

  glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);

  if (!success)
  {
    char infoLog[1024];
    glGetProgramInfoLog(shaderProgram, 1024, NULL, infoLog);
    win32_printf(infoLog);
  }

  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  result.program = shaderProgram;

  return (result);
}

void shader_load_all(void)
{
  shaders.instanced = shader_load("test_instanced.vs", "test_instanced.fs");
}

static const int sizeVec3 = sizeof(float) * 3;
static const int sizeM4x4 = sizeof(float) * 16;

void platform_draw(
    speg_mesh *mesh,
    int numberOfObjects,
    int changed,
    float models[],
    float colors[],
    float uniformProjectionView[16])
{
  if (numberOfObjects == 0)
  {
    return;
  }

  if (!mesh->initialized)
  {
    glGenVertexArrays(1, &mesh->VAO);
    glGenBuffers(1, &mesh->VBO);
    glGenBuffers(1, &mesh->EBO);
    glGenBuffers(1, &mesh->IBO);
    glGenBuffers(1, &mesh->CBO);

    glBindVertexArray(mesh->VAO);

    /* Vertices */
    glBindBuffer(GL_ARRAY_BUFFER, mesh->VBO);
    glBufferData(GL_ARRAY_BUFFER, mesh->verticesSize, mesh->vertices, GL_STATIC_DRAW);

    /* Incides */
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->indicesSize, mesh->indices, GL_STATIC_DRAW);

    /* position layout location = 0 */
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(mesh->vertices[0]), (void *)0);
    glEnableVertexAttribArray(0);

    /* Instanced mesh */
    glBindBuffer(GL_ARRAY_BUFFER, mesh->IBO);
    glBufferData(GL_ARRAY_BUFFER, numberOfObjects * sizeM4x4, &models[0], changed ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);

    /* set attribute pointers 2 - 5 for matrix (4 times vec4) */
    for (int i = 0; i < 4; ++i)
    {
      glEnableVertexAttribArray(2 + i);
      glVertexAttribPointer(2 + i, 4, GL_FLOAT, GL_FALSE, sizeM4x4, (void *)(i * sizeof(float) * 4));
      glVertexAttribDivisor(2 + i, 1);
    }

    /* Instance color attribute (layout = 6) */
    glBindBuffer(GL_ARRAY_BUFFER, mesh->CBO);
    glBufferData(GL_ARRAY_BUFFER, numberOfObjects * sizeVec3, &colors[0], changed ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 3, GL_FLOAT, GL_FALSE, sizeVec3, (void *)0);
    glVertexAttribDivisor(6, 1);

    glBindVertexArray(0);

    mesh->initialized = true;

    win32_print_console("[win32] mesh initialized, vao: %i, vbo: %i, ebo: %i, ibo: %i, cbo: %i, face_culling: %s, dynamic: %s\n", mesh->VAO, mesh->VBO, mesh->EBO, mesh->IBO, mesh->CBO, mesh->faceCulling ? "true" : "false", changed ? "true" : "false");
  }

  /* Mesh changed */
  if (changed)
  {
    glBindBuffer(GL_ARRAY_BUFFER, mesh->IBO);
    glBufferData(GL_ARRAY_BUFFER, numberOfObjects * sizeM4x4, &models[0], GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, mesh->CBO);
    glBufferData(GL_ARRAY_BUFFER, numberOfObjects * sizeVec3, &colors[0], GL_DYNAMIC_DRAW);
  }

  static bool initialized_gl = false;
  static GLint uniformLocationProjectionView = -1;

  if (!initialized_gl)
  {
    glUseProgram(shaders.instanced.program);
    uniformLocationProjectionView = glGetUniformLocation(shaders.instanced.program, "pv");
    initialized_gl = true;
  }

  if (!mesh->faceCulling)
  {
    glDisable(GL_CULL_FACE);
  }

  glBindVertexArray(mesh->VAO);
  glUniformMatrix4fv(uniformLocationProjectionView, 1, GL_FALSE, uniformProjectionView);
  glDrawElementsInstanced(GL_TRIANGLES, mesh->indicesCount, GL_UNSIGNED_INT, 0, numberOfObjects);
  glBindVertexArray(0);

  if (!mesh->faceCulling)
  {
    glEnable(GL_CULL_FACE);
  }

  drawCallsPerFrame++;
}

#include "stdarg.h"
void platform_print_console(const char *file, const int line, char *formatString, ...)
{
  char output[512];
  wsprintfA(output, "%s:%d ", file, line);

  va_list args;
  va_start(args, formatString);
  wvsprintfA(output + w32_strlen(output), formatString, args);
  va_end(args);

  win32_printf(output);
}

typedef struct speg_code
{
  HINSTANCE hDLL;
  FILETIME lastWriteTime;
  char *dllName;
} speg_code;

static speg_code code;

void loadCode(void)
{
  char *dllName = "speg.dll";
  const char *dllTempName = "speg_temp.dll";

  win32_print_console("[win32] load code: %s -> %s\n", dllName, dllTempName);

  if (code.hDLL != NULL)
  {
    if (!FreeLibrary(code.hDLL))
    {
      win32_print_console("[win32] cannot free library: %s\n", dllTempName);
    }
    code.hDLL = NULL;
  }

  if (!CopyFileA(dllName, dllTempName, false))
  {
    win32_print_console("[win32] cannot copy file: %s -> %s\n", dllName, dllTempName);
  }

  code.hDLL = LoadLibraryA(dllTempName);
  code.dllName = dllName;
  code.lastWriteTime = w32_file_mod_time(dllName);

  if (!code.hDLL)
  {
    return;
  }

  /* FIX for ERROR: ISO C forbids conversion of object pointer to function pointer type*/
  /* https://pubs.opengroup.org/onlinepubs/009695399/functions/dlsym.html */
  *(void **)(&speg_update) = GetProcAddress(code.hDLL, "speg_update");

  assert(speg_update);
}

#define GET_X_LPARAM(lp) ((int)(short)LOWORD(lp))
#define GET_Y_LPARAM(lp) ((int)(short)HIWORD(lp))

static LRESULT W32_CALLBACK w32_window_callback(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
  LRESULT result = 0;

  switch (message)
  {
  case WM_CLOSE:
  {
    globalRunning = false;
  }
  break;

  case WM_SIZE:
  {
    int newWidth = LOWORD((DWORD)lParam);
    int newHeight = HIWORD((DWORD)lParam);

    width = newWidth;
    height = newHeight;

    glViewport(0, 0, width, height);
  }
  break;

  case WM_SYSKEYDOWN:
  case WM_SYSKEYUP:
  case WM_KEYDOWN:
  case WM_KEYUP:
  {
    assert(1 == 0);
  }
  break;

  default:
  {
    result = DefWindowProcA(window, message, wParam, lParam);
  }
  break;
  }

  return (result);
}

void processKeyboardMessage(speg_controller_state *state, bool isDown)
{
  if (state->endedDown != isDown)
  {
    state->endedDown = isDown;
    ++state->halfTransitionCount;
  }
}

static bool simulateCam = false;

void processKeyboardMessages(speg_controller_input *oldInput, speg_controller_input *newInput)
{
  /* Reset mouse offsets*/
  newInput->mouseScrollOffset = 0.0f;
  newInput->mouseXOffset = 0.0f;
  newInput->mouseYOffset = 0.0f;

  for (int i = 0; i < (int)array_size(newInput->keys); ++i)
  {
    newInput->keys[i].endedDown = oldInput->keys[i].endedDown;
    newInput->mouseAttached = oldInput->mouseAttached;
  }

  MSG message;
  while (PeekMessageA(&message, 0, 0, 0, PM_REMOVE))
  {
    switch (message.message)
    {
    case WM_INPUT:
    {
      /* Handle raw input */
      UINT dwSize = 0;
      GetRawInputData((HRAWINPUT)message.lParam, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));

      /* Ensure the size is within our fixed buffer size */
      if (dwSize > 64)
      {
        /* If the raw input data size is larger than our fixed buffer size, handle the error */
        assert(1 == 0);
      }

      LPBYTE lpb[64];

      /* Get the raw input data into the fixed buffer */
      if (GetRawInputData((HRAWINPUT)message.lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER)) != dwSize)
      {
        assert(1 == 0); /* Handle data retrieval failure */
      }

      RAWINPUT *raw = (RAWINPUT *)lpb;

      if (raw->header.dwType == RIM_TYPEMOUSE)
      {
        newInput->mouseXOffset += (float)raw->data.mouse.lLastX;
        newInput->mouseYOffset += (float)raw->data.mouse.lLastY;
      }
    }
    break;
    case WM_MOUSEWHEEL:
    {
      int wheelDelta = ((short)HIWORD(message.wParam));

      if (wheelDelta > 0)
      {
        newInput->mouseScrollOffset += 1.0f;
      }
      else if (wheelDelta < 0)
      {
        newInput->mouseScrollOffset -= 1.0f;
      }
    }
    break;
    case WM_SYSKEYDOWN:
    case WM_SYSKEYUP:
    case WM_KEYDOWN:
    case WM_KEYUP:
    {
      WPARAM vkCode = message.wParam;
      bool wasDown = ((message.lParam & ((uint32_t)1 << 30)) != 0);
      bool isDown = ((message.lParam & ((uint32_t)1 << 31)) == 0);

      if (wasDown != isDown)
      {
        if (vkCode == 'W')
        {
          processKeyboardMessage(&newInput->moveForward, isDown);
        }
        else if (vkCode == 'A')
        {
          processKeyboardMessage(&newInput->moveLeft, isDown);
        }
        else if (vkCode == 'S')
        {
          processKeyboardMessage(&newInput->moveBackward, isDown);
        }
        else if (vkCode == 'D')
        {
          processKeyboardMessage(&newInput->moveRight, isDown);
        }
        else if (vkCode == 'E' || vkCode == VK_SPACE)
        {
          processKeyboardMessage(&newInput->moveUp, isDown);
        }
        else if (vkCode == 'Q' || vkCode == VK_CONTROL)
        {
          processKeyboardMessage(&newInput->moveDown, isDown);
        }
        else if (vkCode == VK_F5)
        {
          processKeyboardMessage(&newInput->cameraResetPosition, isDown);
        }
      }

      if (wasDown)
      {
        if (vkCode == 'P')
        {
          globalPause = !globalPause;
        }

        else if (vkCode == VK_RETURN || vkCode == VK_ESCAPE)
        {
          globalRunning = false;
        }

        else if (vkCode == VK_F1)
        {
          wireframeMode = !wireframeMode;
          glPolygonMode(GL_FRONT_AND_BACK, wireframeMode ? GL_LINE : GL_FILL);
        }

        else if (vkCode == VK_F2)
        {
          POINT mousePoint;
          mousePoint.x = width / 2;
          mousePoint.y = height / 2;
          ClientToScreen(window, &mousePoint);
          SetCursorPos(mousePoint.x, mousePoint.y);
        }

        else if (vkCode == VK_F3)
        {
          simulateCam = !simulateCam;
          processKeyboardMessage(&newInput->cameraSimulate, simulateCam);
        }

        else if (vkCode == VK_F4)
        {
          vsync = !vsync;
          wglSwapIntervalEXT(vsync);
        }
      }

      bool altKeyWasDown = ((message.lParam & ((uint32_t)1 << 29)) != 0);

      if ((vkCode == VK_F4) && altKeyWasDown)
      {
        globalRunning = false;
      }
    }
    break;

    case WM_QUIT:
    {
      globalRunning = false;
    }
    break;

    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    {
      int x = GET_X_LPARAM(message.lParam);
      int y = GET_Y_LPARAM(message.lParam);

      assert(x >= 0 && y >= 0);

      RECT rect;
      GetClientRect(window, &rect);

      POINT ul;
      ul.x = rect.left;
      ul.y = rect.top;

      POINT lr;
      lr.x = rect.right;
      lr.y = rect.bottom;

      MapWindowPoints(window, 0, &ul, 1);
      MapWindowPoints(window, 0, &lr, 1);

      rect.left = ul.x;
      rect.top = ul.y;

      rect.right = lr.x;
      rect.bottom = lr.y;

      ClipCursor(&rect);
      ShowCursor(false);
      newInput->mouseAttached = true;
    }
    break;

    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    {
      ClipCursor(0);

      POINT mousePoint;
      mousePoint.x = width / 2;
      mousePoint.y = height / 2;
      ClientToScreen(window, &mousePoint);
      SetCursorPos(mousePoint.x, mousePoint.y);

      ShowCursor(true);
      newInput->mouseAttached = false;
    }
    break;

    default:
    {
      TranslateMessage(&message);
      DispatchMessageA(&message);
    }
    break;
    }
  }
}

#ifdef __clang__
#elif __GNUC__
__attribute((externally_visible))
#endif
#ifdef __i686__
__attribute((force_align_arg_pointer))
#endif
int
mainCRTStartup(void)
{

  HINSTANCE instance = {0};
  WNDCLASSA windowClass = {0};

  windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
  windowClass.lpfnWndProc = w32_window_callback;
  windowClass.hInstance = instance;
  windowClass.hCursor = LoadCursorA(0, IDC_ARROW);
  windowClass.lpszClassName = "win32";

  if (!RegisterClassA(&windowClass))
  {
    return 1;
  }

  /***********************/
  /*OPENGL INITIALIZATION*/
  /***********************/
  HWND fakeWND = CreateWindowExA(
      0,
      windowClass.lpszClassName,
      windowClass.lpszClassName,
      WS_CAPTION | WS_SYSMENU | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
      0, 0,
      1, 1,
      0, 0,
      instance, 0);

  if (!fakeWND)
  {
    return 1;
  }

  HDC fakeDC = GetDC(fakeWND);

  PIXELFORMATDESCRIPTOR fakePFD = {0};
  fakePFD.nSize = sizeof(fakePFD);
  fakePFD.nVersion = 1;
  fakePFD.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
  fakePFD.iPixelType = PFD_TYPE_RGBA;
  fakePFD.cColorBits = 32;
  fakePFD.cAlphaBits = 8;
  fakePFD.cDepthBits = 24;

  const int fakePFDID = ChoosePixelFormat(fakeDC, &fakePFD);
  if (!fakePFDID)
  {
    return 1;
  }

  if (!SetPixelFormat(fakeDC, fakePFDID, &fakePFD))
  {
    return 1;
  }

  HGLRC fakeRC = wglCreateContext(fakeDC);

  if (!fakeRC)
  {
    return 1;
  }

  if (!wglMakeCurrent(fakeDC, fakeRC))
  {
    return 1;
  }

  if (!w32_gl_init_gl_functions())
  {
    return 1;
  }

  /* Find out center location of the window*/
  DWORD windowStyle = WS_CAPTION | WS_SYSMENU | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_THICKFRAME;

  RECT rect = {0, 0, width, height};
  AdjustWindowRect(&rect, windowStyle, false);
  width = rect.right - rect.left;
  height = rect.bottom - rect.top;

  RECT primaryDisplaySize;
  SystemParametersInfoA(SPI_GETWORKAREA, 0, &primaryDisplaySize, 0); /* system taskbar and application desktop toolbars not included */
  int posX = (primaryDisplaySize.right - width) / 2;
  int posY = (primaryDisplaySize.bottom - height) / 2;

  window = CreateWindowExA(
      0,
      windowClass.lpszClassName,
      windowClass.lpszClassName,
      windowStyle,
      posX, posY,
      width, height,
      0, 0,
      instance, 0);

  /* Modal window */
  SetWindowPos(window, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

  HDC dc = GetDC(window);

  const int pixelAttribs[] = {
      WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
      WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
      WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
      WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
      WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
      WGL_COLOR_BITS_ARB, 32,
      WGL_ALPHA_BITS_ARB, 8,
      WGL_DEPTH_BITS_ARB, 24,
      WGL_STENCIL_BITS_ARB, 8,
      WGL_SAMPLE_BUFFERS_ARB, GL_TRUE,
      WGL_SAMPLES_ARB, 4,
      0};

  int pixelFormatID;
  UINT numFormats;
  const BOOL status = wglChoosePixelFormatARB(dc, pixelAttribs, 0, 1, &pixelFormatID, &numFormats);

  if (!status || !numFormats)
  {
    return 1;
  }

  PIXELFORMATDESCRIPTOR PFD;
  DescribePixelFormat(dc, pixelFormatID, sizeof(PFD), &PFD);
  SetPixelFormat(dc, pixelFormatID, &PFD);

  /* Open GL 3.3 specification */
  int contextAttribs[] = {
      WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
      WGL_CONTEXT_MINOR_VERSION_ARB, 3,
      WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB | WGL_CONTEXT_DEBUG_BIT_ARB,
      WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
      0};

  HGLRC rc = wglCreateContextAttribsARB(dc, 0, contextAttribs);
  if (!rc)
  {
    return 1;
  }

  wglMakeCurrent(0, 0);
  wglDeleteContext(fakeRC);
  ReleaseDC(fakeWND, fakeDC);
  DestroyWindow(fakeWND);

  if (!wglMakeCurrent(dc, rc))
  {
    return 1;
  }

  LPCSTR glRenderer = (LPCSTR)glGetString(GL_RENDERER);
  LPCSTR glVersion = (LPCSTR)glGetString(GL_VERSION);

  wglSwapIntervalEXT(vsync);

  /***********************/
  /*OPENGL END */
  /***********************/
  ShowWindow(window, SW_SHOWDEFAULT);

  /* Register for raw input from the mouse */
  RAWINPUTDEVICE rid;
  rid.usUsagePage = 0x01; /* Generic desktop controls */
  rid.usUsage = 0x02;     /* Mouse */
  rid.dwFlags = 0;
  rid.hwndTarget = window;

  if (!RegisterRawInputDevices(&rid, 1, sizeof(rid)))
  {
    win32_print_console("Failed to register raw input device\n", "");
    return -1;
  }

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glFrontFace(GL_CCW);
  glViewport(0, 0, width, height);

  loadCode();
  shader_load_all();

  /**************/
  /* Perf. Data */
  /**************/
  unsigned long lastCycleCount = w32_rdtsc();

  LARGE_INTEGER perfCountFrequency;
  QueryPerformanceFrequency(&perfCountFrequency);

  LARGE_INTEGER lastCounter;
  QueryPerformanceCounter(&lastCounter);

  POINT mousePoint;
  mousePoint.x = width / 2;
  mousePoint.y = height / 2;
  ClientToScreen(window, &mousePoint);
  SetCursorPos(mousePoint.x, mousePoint.y);

  speg_platform_api platformApi = {0};
  platformApi.platform_draw = platform_draw;
  platformApi.platform_print_console = platform_print_console;
  platformApi.platform_sleep = Sleep;
  platformApi.platform_perf_current_cycle_count = w32_rdtsc;

  speg_memory memory = {0};
  memory.permanentMemorySize = 1024 * 1024 * 1; /* 1 MB Allocation */
  memory.permanentMemory = VirtualAlloc(0, memory.permanentMemorySize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

  assert(memory.permanentMemorySize > 0);
  assert(memory.permanentMemory);

  speg_controller_input inputs[2] = {0};
  speg_controller_input *newInput = &inputs[0];
  speg_controller_input *oldInput = &inputs[1];

  while (globalRunning)
  {
    /*********************************/
    /* (1) HOT-Reload Code & Shaders */
    /*********************************/
    FILETIME ddlFtCurrent = w32_file_mod_time(code.dllName);

    if (CompareFileTime(&ddlFtCurrent, &code.lastWriteTime) != 0)
    {
      win32_print_console("%s", "[win32] hot reload code dll\n");
      loadCode();
      memory.initialized = false;
    }

    FILETIME vs = w32_file_mod_time(shaders.instanced.vsFile);

    if (CompareFileTime(&vs, &shaders.instanced.vsTime) != 0)
    {
      win32_print_console("%s", "[win32] hot reload shader files\n");
      glDeleteProgram(shaders.instanced.program);
      shader_load_all();
    }

    /************************/
    /* (2) Input Processing */
    /************************/
    processKeyboardMessages(oldInput, newInput);

    /*****************/
    /* (3) Rendering */
    /*****************/
    speg_state *state = (speg_state *)memory.permanentMemory;

    if (!globalPause)
    {
      glClearColor(state->clearColorR, state->clearColorG, state->clearColorB, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      state->renderedObjects = 0;
      state->culledObjects = 0;
      state->dt = dt;
      state->width = width;
      state->height = height;

      drawCallsPerFrame = 0;
      occludedObjectsPerFrame = 0;

      speg_update(&memory, newInput, &platformApi);

      SwapBuffers(dc);
    }
    else
    {
      TextOutA(GetDC(window), 10, 40, "!!! PAUSE !!! (Press P to unpause)", 34);
      Sleep(1);
    }

    speg_controller_input *tmp = newInput;
    newInput = oldInput;
    oldInput = tmp;

    /******************/
    /* (4) Perf. Data */
    /******************/
    unsigned long endCycleCount = w32_rdtsc();
    LARGE_INTEGER endCounter;
    QueryPerformanceCounter(&endCounter);

    unsigned long cyclesElapsed = endCycleCount - lastCycleCount;
    LONGLONG counterElapsed = endCounter.QuadPart - lastCounter.QuadPart;
    int32_t msPerFrame = (int32_t)(((1000 * counterElapsed) / perfCountFrequency.QuadPart));
    double msPassedPerFrame = ((1000.0 * (double)counterElapsed) / (double)perfCountFrequency.QuadPart);
    LONGLONG fps = perfCountFrequency.QuadPart / (counterElapsed == 0 ? 1 : counterElapsed);

    dt = ((double)(counterElapsed) / (double)perfCountFrequency.QuadPart);

    lastCounter = endCounter;
    lastCycleCount = endCycleCount;

    /*****************/
    /* (5) Debugging */
    /*****************/
    static double msPassed = 0;
    msPassed += msPassedPerFrame;

    /* Every second */
    if (msPassed >= 1000.0)
    {
      char buffer[256];
      wsprintfA(buffer, "%4d ms/f, %4d fps, %10d cycles, size: %4d / %4d, %s, %s\n", msPerFrame, fps, cyclesElapsed, width, height, glRenderer, glVersion);
      SetWindowTextA(window, buffer);
      win32_print_console("[win32] objs: %4d, culled: %4d, occluded: %4d, dc/f: %4d, %4d ms/f, %5d fps, %10d cycles, wireframe(F1) %1d, simCam(F3) %1d, vsync(F4) %1d\n", state->renderedObjects, state->culledObjects, occludedObjectsPerFrame, drawCallsPerFrame, msPerFrame, fps, cyclesElapsed, wireframeMode, simulateCam, vsync);
      msPassed = 0;
    }
  }

  ExitProcess(0);
  return 0;
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
