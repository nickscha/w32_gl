/* w32_gl.h - v0.1 - public domain data structures - nickscha 2025

A C89 standard compliant, single header, nostdlib (no C Standard Library) win32 and opengl api function prototypes without using windows.h.

LICENSE

  Placed in the public domain and also MIT licensed.
  See end of file for detailed license information.

*/
#ifndef W32_GL_H
#define W32_GL_H

#define W32_GL_STATIC_ASSERT(c, m) typedef char types_assert_##m[(c) ? 1 : -1]

#define W32_API(r) __declspec(dllimport) r __stdcall

#define W32_CALLBACK __stdcall

#define W32_ASSERT(expression)  \
    if (!(expression))          \
    {                           \
        *(volatile int *)0 = 0; \
    }

#define W32_DECLARE_HANDLE(name) \
    struct name##__              \
    {                            \
        int unused;              \
    };                           \
    typedef struct name##__ *name

W32_DECLARE_HANDLE(HWND);
W32_DECLARE_HANDLE(HINSTANCE);
W32_DECLARE_HANDLE(HICON);
W32_DECLARE_HANDLE(HBRUSH);
W32_DECLARE_HANDLE(HMENU);
W32_DECLARE_HANDLE(HRAWINPUT);

typedef char *va_list;
typedef HINSTANCE HMODULE;
typedef unsigned int UINT;
typedef void *INT_PTR;
typedef unsigned long long UINT_PTR;
typedef long long LONG_PTR;
typedef unsigned long long ULONG_PTR, *PULONG_PTR;
typedef unsigned long long ULONGLONG;
typedef long long LONGLONG;
typedef signed long GLsizeiptr;
typedef UINT_PTR WPARAM;
typedef LONG_PTR LPARAM;
typedef LONG_PTR LRESULT;
typedef char CHAR;
typedef const CHAR *LPCSTR, *PCSTR;
typedef unsigned short WCHAR;
typedef WCHAR *NWPSTR, *LPWSTR, *PWSTR;
typedef const WCHAR *LPCWSTR, *PCWSTR;
typedef CHAR *NPSTR, *LPSTR, *PSTR;
typedef void *LPVOID;
typedef void *PVOID;
typedef void *HDC;
typedef void *HGLRC;
typedef PVOID HANDLE;
typedef unsigned long DWORD;
typedef unsigned short WORD;
typedef WORD ATOM;
typedef LRESULT(W32_CALLBACK *WNDPROC)(HWND, UINT, WPARAM, LPARAM);
typedef HICON HCURSOR;
typedef int BOOL;
typedef long LONG;
typedef unsigned char BYTE;
typedef BYTE *LPBYTE;
typedef INT_PTR (*PROC)(void);
typedef ULONG_PTR DWORD_PTR, *PDWORD_PTR;
typedef ULONG_PTR SIZE_T, *PSIZE_T;
typedef DWORD *LPDWORD;
typedef DWORD ULONG;
typedef unsigned short USHORT;
typedef unsigned int *PUINT;
typedef float FLOAT;
typedef unsigned char GLubyte;
typedef unsigned int GLenum;
typedef float GLclampf;
typedef unsigned int GLuint;
typedef unsigned int GLbitfield;
typedef int GLsizei;
typedef char GLchar;
typedef int GLint;
typedef void GLvoid;
typedef unsigned char GLboolean;
typedef float GLfloat;
typedef struct __GLsync *GLsync;

typedef struct tagPOINT
{
    LONG x;
    LONG y;
} POINT, *PPOINT, *LPPOINT;

typedef struct tagMSG
{
    HWND hwnd;
    UINT message;
    WPARAM wParam;
    LPARAM lParam;
    DWORD time;
    POINT pt;
    DWORD lPrivate;
} MSG, *LPMSG;

typedef struct WNDCLASSA
{
    UINT style;
    WNDPROC lpfnWndProc;
    int cbClsExtra;
    int cbWndExtra;
    HINSTANCE hInstance;
    HICON hIcon;
    HCURSOR hCursor;
    HBRUSH hbrBackground;
    LPCSTR lpszMenuName;
    LPCSTR lpszClassName;
} WNDCLASSA;

typedef struct FILETIME
{
    DWORD dwLowDateTime;
    DWORD dwHighDateTime;
} FILETIME;

typedef struct WIN32_FILE_ATTRIBUTE_DATA
{
    DWORD dwFileAttributes;
    FILETIME ftCreationTime;
    FILETIME ftLastAccessTime;
    FILETIME ftLastWriteTime;
    DWORD nFileSizeHigh;
    DWORD nFileSizeLow;
} WIN32_FILE_ATTRIBUTE_DATA;

typedef enum GET_FILEEX_INFO_LEVELS
{
    GetFileExInfoStandard,
    GetFileExMaxInfoLevel
} GET_FILEEX_INFO_LEVELS;

typedef struct tagRECT
{
    LONG left;
    LONG top;
    LONG right;
    LONG bottom;
} RECT, *PRECT, *LPRECT;

typedef struct LARGE_INTEGER
{
    LONGLONG QuadPart;
} LARGE_INTEGER;

typedef struct tagPIXELFORMATDESCRIPTOR
{
    WORD nSize;
    WORD nVersion;
    DWORD dwFlags;
    BYTE iPixelType;
    BYTE cColorBits;
    BYTE cRedBits;
    BYTE cRedShift;
    BYTE cGreenBits;
    BYTE cGreenShift;
    BYTE cBlueBits;
    BYTE cBlueShift;
    BYTE cAlphaBits;
    BYTE cAlphaShift;
    BYTE cAccumBits;
    BYTE cAccumRedBits;
    BYTE cAccumGreenBits;
    BYTE cAccumBlueBits;
    BYTE cAccumAlphaBits;
    BYTE cDepthBits;
    BYTE cStencilBits;
    BYTE cAuxBuffers;
    BYTE iLayerType;
    BYTE bReserved;
    DWORD dwLayerMask;
    DWORD dwVisibleMask;
    DWORD dwDamageMask;
} PIXELFORMATDESCRIPTOR, *LPPIXELFORMATDESCRIPTOR;

typedef struct RAWINPUTDEVICE
{
    USHORT usUsagePage;
    USHORT usUsage;
    DWORD dwFlags;
    HWND hwndTarget;
} RAWINPUTDEVICE;

typedef struct RAWINPUTHEADER
{
    DWORD dwType;
    DWORD dwSize;
    HANDLE hDevice;
    WPARAM wParam;
} RAWINPUTHEADER;

typedef struct RAWKEYBOARD
{
    USHORT MakeCode;
    USHORT Flags;
    USHORT Reserved;
    USHORT VKey;
    UINT Message;
    ULONG ExtraInformation;

} RAWKEYBOARD;

typedef struct RAWMOUSE
{
    USHORT usFlags;
    ULONG ulButtons;
    ULONG ulRawButtons;
    LONG lLastX;
    LONG lLastY;
    ULONG ulExtraInformation;

} RAWMOUSE;

typedef struct RAWHID
{
    DWORD dwSizeHid;
    DWORD dwCount;
    BYTE bRawData[1];
} RAWHID;

typedef struct RAWINPUT
{
    RAWINPUTHEADER header;
    union
    {
        RAWMOUSE mouse;
        RAWKEYBOARD keyboard;
        RAWHID hid;
    } data;
} RAWINPUT;

typedef const RAWINPUTDEVICE *PCRAWINPUTDEVICE;

#define MEM_COMMIT 0x00001000
#define MEM_RESERVE 0x00002000
#define PAGE_READWRITE 0x04
#define INVALID_HANDLE_VALUE ((HANDLE)(LONG_PTR) - 1)
#define GENERIC_READ (0x80000000L)
#define FILE_SHARE_READ 0x00000001
#define OPEN_EXISTING 3
#define FILE_ATTRIBUTE_NORMAL 0x00000080
#define FILE_FLAG_OVERLAPPED 0x40000000
#define INVALID_FILE_SIZE ((DWORD)0xFFFFFFFF)
#define STD_OUTPUT_HANDLE ((DWORD) - 11)
#define PM_REMOVE 0x0001
#define WM_INPUT 0x00FF
#define WM_CLOSE 0x0010
#define WM_DESTROY 0x0002
#define WM_QUIT 0x0012
#define WM_SIZE 0x0005
#define WM_SYSKEYDOWN 0x0104
#define WM_SYSKEYUP 0x0105
#define WM_KEYDOWN 0x0100
#define WM_KEYUP 0x0101
#define WM_LBUTTONDOWN 0x0201
#define WM_LBUTTONUP 0x0202
#define WM_RBUTTONDOWN 0x0204
#define WM_RBUTTONUP 0x0205
#define WM_MOUSEWHEEL 0x020A
#define VK_ESCAPE 0x1B
#define VK_RETURN 0x0D
#define VK_SPACE 0x20
#define VK_CONTROL 0x11
#define VK_F1 0x70
#define VK_F2 0x71
#define VK_F3 0x72
#define VK_F4 0x73
#define VK_F5 0x74
#define CS_VREDRAW 0x0001
#define CS_HREDRAW 0x0002
#define WS_OVERLAPPED 0x00000000L
#define WS_EX_TOPMOST 0x00000008L
#define WS_CAPTION 0x00C00000L
#define WS_SYSMENU 0x00080000L
#define WS_THICKFRAME 0x00040000L
#define WS_MINIMIZEBOX 0x00020000L
#define WS_MAXIMIZEBOX 0x00010000L
#define WS_OVERLAPPEDWINDOW (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX)
#define WS_VISIBLE 0x10000000L
#define CW_USEDEFAULT ((int)0x80000000)
#define WS_CLIPSIBLINGS 0x04000000
#define WS_CLIPCHILDREN 0x02000000
#define MAKEINTRESOURCEA(i) ((LPSTR)((unsigned long)((WORD)(i))))
#define IDC_ARROW MAKEINTRESOURCEA(32512)
#define HWND_TOPMOST ((HWND) - 1)
#define SWP_NOSIZE 0x0001
#define SWP_NOMOVE 0x0002
#define SPI_GETWORKAREA 0x0030
#define LOWORD(l) ((WORD)(((DWORD_PTR)(l)) & 0xffff))
#define HIWORD(l) ((WORD)((((DWORD_PTR)(l)) >> 16) & 0xffff))
#define RID_INPUT 0x10000003
#define RIM_TYPEMOUSE 0
#define CS_OWNDC 0x0020
#define GL_TRUE 1
#define WGL_DRAW_TO_WINDOW_ARB 0x2001
#define WGL_SUPPORT_OPENGL_ARB 0x2010
#define WGL_DOUBLE_BUFFER_ARB 0x2011
#define WGL_PIXEL_TYPE_ARB 0x2013
#define WGL_TYPE_RGBA_ARB 0x202B
#define WGL_ACCELERATION_ARB 0x2003
#define WGL_FULL_ACCELERATION_ARB 0x2027
#define WGL_COLOR_BITS_ARB 0x2014
#define WGL_ALPHA_BITS_ARB 0x201B
#define WGL_DEPTH_BITS_ARB 0x2022
#define WGL_STENCIL_BITS_ARB 0x2023
#define WGL_SAMPLE_BUFFERS_ARB 0x2041
#define WGL_SAMPLES_ARB 0x2042
#define WGL_CONTEXT_MAJOR_VERSION_ARB 0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB 0x2092
#define WGL_CONTEXT_PROFILE_MASK_ARB 0x9126
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB 0x00000001
#define WGL_CONTEXT_FLAGS_ARB 0x2094
#define WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB 0x00000002
#define WGL_CONTEXT_DEBUG_BIT_ARB 0x00000001
#define GL_VERSION 0x1F02
#define GL_EXTENSIONS 0x1F03
#define SW_SHOWDEFAULT 10
#define GL_COLOR_BUFFER_BIT 0x00004000
#define GL_SAMPLES_PASSED 0x8914
#define GL_ANY_SAMPLES_PASSED 0x8C2F
#define GL_QUERY_RESULT 0x8866
#define GL_QUERY_RESULT_AVAILABLE 0x8867
#define PFD_DOUBLEBUFFER 0x00000001
#define PFD_SUPPORT_OPENGL 0x00000020
#define PFD_DRAW_TO_WINDOW 0x00000004
#define PFD_TYPE_RGBA 0
#define NULL ((void *)0)
#define GL_COMPILE_STATUS 0x8B81
#define GL_VERTEX_SHADER 0x8B31
#define GL_FRAGMENT_SHADER 0x8B30
#define GL_LINK_STATUS 0x8B82
#define GL_ARRAY_BUFFER 0x8892
#define GL_STATIC_DRAW 0x88E4
#define GL_DYNAMIC_DRAW 0x88E8
#define GL_FLOAT 0x1406
#define GL_FALSE 0
#define GL_TRIANGLES 0x0004
#define GL_DEPTH_TEST 0x0B71
#define GL_MULTISAMPLE 0x809D
#define GL_DEPTH_BUFFER_BIT 0x00000100
#define GL_FRONT_AND_BACK 0x0408
#define GL_LINE 0x1B01
#define GL_FILL 0x1B02
#define GL_CULL_FACE 0x0B44
#define GL_BACK 0x0405
#define GL_CCW 0x0901
#define GL_ELEMENT_ARRAY_BUFFER 0x8893
#define GL_UNSIGNED_INT 0x1405
#define GL_SYNC_GPU_COMMANDS_COMPLETE 0x9117
#define GL_FRAMEBUFFER 0x8D40
#define GL_DEPTH_ATTACHMENT 0x8D00
#define GL_DEPTH_COMPONENT 0x1902
#define GL_TEXTURE_2D 0x0DE1
#define GL_RGBA 0x1908
#define GL_UNSIGNED_BYTE 0x1401
#define GL_TEXTURE_MIN_FILTER 0x2801
#define GL_TEXTURE_MAG_FILTER 0x2800
#define GL_LINEAR 0x2601
#define GL_COLOR_ATTACHMENT0 0x8CE0
#define GL_RENDERBUFFER 0x8D41
#define GL_DEPTH_COMPONENT24 0x81A6
#define GL_FRAMEBUFFER_COMPLETE 0x8CD5
#define GL_NEAREST 0x2600
#define GL_NONE 0
#define GL_TEXTURE_WRAP_S 0x2802
#define GL_TEXTURE_WRAP_T 0x2803
#define GL_CLAMP_TO_BORDER 0x812D
#define GL_TEXTURE_BORDER_COLOR 0x1004

/* WIN32 Function prototyes */
W32_API(BOOL) PeekMessageA(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg);
W32_API(BOOL) TranslateMessage(const MSG *lpMsg);
W32_API(LRESULT) DispatchMessageA(const MSG *lpMsg);
W32_API(BOOL) ShowWindow(HWND hWnd, int nCmdShow);
W32_API(BOOL) DestroyWindow(HWND hWnd);
W32_API(LRESULT) DefWindowProcA(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
W32_API(HCURSOR) LoadCursorA(HINSTANCE hInstance, LPCSTR lpCursorName);
W32_API(int) ShowCursor(BOOL bShow);
W32_API(ATOM) RegisterClassA(const WNDCLASSA *lpWndClass);
W32_API(HWND) CreateWindowExA(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam);
W32_API(BOOL) SetWindowTextA(HWND hWnd, LPCSTR lpString);
W32_API(BOOL) SetWindowPos(HWND hWnd, HWND hWndInsertAfter, int X, int Y, int cx, int cy, UINT uFlags);
W32_API(BOOL) AdjustWindowRect(LPRECT lpRect, DWORD dwStyle, BOOL bMenu);
W32_API(void) Sleep(DWORD dwMilliseconds);
W32_API(void) ExitProcess(UINT uExitCode);
W32_API(void) PostQuitMessage(int nExitCode);
W32_API(int) wsprintfA(LPSTR unnamedParam1, LPCSTR unnamedParam2, ...);
W32_API(int) wvsprintfA(LPSTR unnamedParam1, LPCSTR unnamedParam2, va_list arglist);
W32_API(long) CompareFileTime(const FILETIME *lpFileTime1, const FILETIME *lpFileTime2);
W32_API(BOOL) GetFileAttributesExA(LPCSTR lpFileName, GET_FILEEX_INFO_LEVELS fInfoLevelId, LPVOID lpFileInformation);
W32_API(BOOL) CopyFileA(LPCSTR lpExistingFileName, LPCSTR lpNewFileName, BOOL bFailIfExists);
W32_API(HMODULE) LoadLibraryA(LPCSTR lpLibFileName);
W32_API(BOOL) FreeLibrary(HMODULE hLibModule);
W32_API(INT_PTR) GetProcAddress(HMODULE hModule, LPCSTR lpProcName);
W32_API(BOOL) CloseHandle(HANDLE hObject);
W32_API(BOOL) GetClientRect(HWND hWnd, LPRECT lpRect);
W32_API(int) MapWindowPoints(HWND hWndFrom, HWND hWndTo, LPPOINT lpPoints, UINT cPoints);
W32_API(BOOL) ClipCursor(const RECT *lpRect);
W32_API(BOOL) GetCursorPos(LPPOINT lpPoint);
W32_API(BOOL) SetCursorPos(int X, int Y);
W32_API(BOOL) ScreenToClient(HWND hWnd, LPPOINT lpPoint);
W32_API(BOOL) ClientToScreen(HWND hWnd, LPPOINT lpPoint);
W32_API(BOOL) QueryPerformanceCounter(LARGE_INTEGER *lpPerformanceCount);
W32_API(BOOL) QueryPerformanceFrequency(LARGE_INTEGER *lpFrequency);
W32_API(BOOL) SystemParametersInfoA(UINT uiAction, UINT uiParam, PVOID pvParam, UINT fWinIni);
W32_API(BOOL) SwapBuffers(HDC unnamedParam1);
W32_API(HDC) GetDC(HWND hWnd);
W32_API(int) ReleaseDC(HWND hWnd, HDC hDC);
W32_API(BOOL) TextOutA(HDC hdc, int x, int y, LPCSTR lpString, int c);
W32_API(int) ChoosePixelFormat(HDC hdc, const PIXELFORMATDESCRIPTOR *ppfd);
W32_API(BOOL) SetPixelFormat(HDC hdc, int format, const PIXELFORMATDESCRIPTOR *ppfd);
W32_API(int) DescribePixelFormat(HDC hdc, int iPixelFormat, UINT nBytes, LPPIXELFORMATDESCRIPTOR ppfd);
W32_API(HANDLE) CreateFileA(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, void *, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);
W32_API(BOOL) ReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, void *lpOverlapped);
W32_API(DWORD) GetFileSize(HANDLE hFile, LPDWORD lpFileSizeHigh);
W32_API(LPVOID) HeapAlloc(HANDLE hHeap, DWORD dwFlags, SIZE_T dwBytes);
W32_API(HANDLE) GetProcessHeap(void);
W32_API(BOOL) HeapFree(HANDLE hHeap, DWORD dwFlags, LPVOID lpMem);
W32_API(LPVOID) VirtualAlloc(LPVOID lpAddress, SIZE_T dwSize, DWORD flAllocationType, DWORD flProtect);
W32_API(HANDLE) GetStdHandle(DWORD nStdHandle);
W32_API(BOOL) WriteConsoleA(HANDLE hConsoleOutput, const void *lpBuffer, DWORD nNumberOfCharsToWrite, LPDWORD lpNumberOfCharsWritten, LPVOID lpReserved);
W32_API(HGLRC) wglCreateContext(HDC unnamedParam1);
W32_API(HGLRC) wglGetCurrentContext(void);
W32_API(HDC) wglGetCurrentDC(void);
W32_API(BOOL) wglDeleteContext(HGLRC unnamedParam1);
W32_API(BOOL) wglMakeCurrent(HDC unnamedParam1, HGLRC unnamedParam2);
W32_API(PROC) wglGetProcAddress(LPCSTR unnamedParam1);
/* Windows provided OPEN GL 1.X functions */
W32_API(const GLubyte) * glGetString(GLenum name);
W32_API(void) glClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
W32_API(void) glClear(GLbitfield mask);
W32_API(GLenum) glGetError(void);
W32_API(void) glEnable(GLenum cap);
W32_API(void) glDisable(GLenum cap);
W32_API(void) glPolygonMode(GLenum face, GLenum mode);
W32_API(void) glCullFace(GLenum mode);
W32_API(void) glFrontFace(GLenum mode);
W32_API(void) glViewport(GLint x, GLint y, GLsizei width, GLsizei height);
W32_API(void) glDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices);
W32_API(void) glColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
W32_API(void) glDepthMask(GLboolean flag);
W32_API(void) glReadBuffer(GLenum mode);
W32_API(void) glDrawBuffer(GLenum mode);
W32_API(void) glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels);
W32_API(void) glGenTextures(GLsizei n, GLuint *textures);
W32_API(void) glBindTexture(GLenum target, GLuint texture);
W32_API(void) glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLint format, GLenum type, const GLvoid *pixels);
W32_API(void) glTexParameteri(GLenum target, GLenum pname, GLint param);
W32_API(void) glTexParameterfv(GLenum target, GLenum pname, const GLfloat *params);
W32_API(BOOL) RegisterRawInputDevices(PCRAWINPUTDEVICE pRawInputDevices, UINT uiNumDevices, UINT cbSize);
W32_API(UINT) GetRawInputData(HRAWINPUT hRawInput, UINT uiCommand, LPVOID pData, PUINT pcbSize, UINT cbSizeHeader);

DWORD w32_strlen(const char *str)
{
    DWORD length = 0;
    while (str[length] != '\0')
    {
        length++;
    }
    return length;
}

void win32_printf(const char *str)
{
    DWORD written;
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    WriteConsoleA(hConsole, str, w32_strlen(str), &written, NULL);
}

#define win32_print_console(formatString, ...)                         \
    char output[512];                                                  \
    wsprintfA(output, "%s:%d ", __FILE__, __LINE__);                   \
    wsprintfA(output + w32_strlen(output), formatString, __VA_ARGS__); \
    win32_printf(output);

#endif /* W32_GL_H */

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
