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

#include "../../w32_gl.h"
#include "../w32_open_gl_func.h"

#define false 0
#define true 1

static BOOL globalRunning = true;
static int width = 800;
static int height = 600;

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
        width = LOWORD((DWORD)lParam);
        height = HIWORD((DWORD)lParam);

        glViewport(0, 0, width, height);
    }
    break;

    case WM_SYSKEYDOWN:
    case WM_SYSKEYUP:
    case WM_KEYDOWN:
    case WM_KEYUP:
    {
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
    windowClass.lpszClassName = "w32_gl_01_simple";

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

    HWND window = CreateWindowExA(
        0,
        windowClass.lpszClassName,
        windowClass.lpszClassName,
        windowStyle,
        posX, posY,
        width, height,
        0, 0,
        instance, 0);

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

    wglSwapIntervalEXT(0);

    /***********************/
    /*OPENGL END */
    /***********************/
    ShowWindow(window, SW_SHOWDEFAULT);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glViewport(0, 0, width, height);

    while (globalRunning)
    {
        glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        MSG msg;
        while (PeekMessageA(&msg, 0, 0, 0, PM_REMOVE))
        {
            switch (msg.message)
            {
            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP:
            case WM_KEYDOWN:
            case WM_KEYUP:
            {
                WPARAM vkCode = msg.wParam;
                BOOL wasDown = ((msg.lParam & ((unsigned int)1 << 30)) != 0);
                BOOL isDown = ((msg.lParam & ((unsigned int)1 << 31)) == 0);

                (void) isDown;

                if (wasDown)
                {
                    if (vkCode == VK_RETURN)
                    {
                        globalRunning = false;
                    }
                }
            }
            break;
            case WM_QUIT:
            {
                globalRunning = false;
            }
            break;
            default:
            {
                TranslateMessage(&msg);
                DispatchMessageA(&msg);
            }
            break;
            }
        }

        SwapBuffers(dc);
    }

    ExitProcess(0);
    return 0;
}
