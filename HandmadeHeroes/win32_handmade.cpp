#include <stdint.h>
#include <windows.h>

#define internal static
#define local_persist static
#define global_variable static

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

// todo(nils): this is global for now
global_variable bool running = true;
global_variable BITMAPINFO bitmap_info;
global_variable void *bitmap_memory;
global_variable int bitmap_height;
global_variable int bitmap_width;
global_variable int bytes_per_pixel = 4;

internal void
render_weird_gradient(int x_offset, int y_offset)
{
    int pitch = bitmap_width * bytes_per_pixel;
    uint8 *row = (uint8 *)bitmap_memory;
    for (int y = 0; y < bitmap_height; ++y)
    {
        uint32 *pixel = (uint32 *)row;
        for (int x = 0; x < bitmap_width; ++x)
        {
            uint8 blue = (x + x_offset);
            uint8 green = (y + y_offset);
            uint8 red = 3.14 * (blue + green);

            *pixel++ = ((red << 8 << 8) | (green << 8) | blue);
        }
        row += pitch;
    }
}

internal void
win32_resize_dib_section(int width, int height)
{
    if (bitmap_memory)
    {
        VirtualFree(bitmap_memory, 0, MEM_RELEASE);
    }

    bitmap_height = height;
    bitmap_width = width;

    bitmap_info.bmiHeader.biSize = sizeof(bitmap_info.bmiHeader);
    bitmap_info.bmiHeader.biWidth = bitmap_width;
    bitmap_info.bmiHeader.biHeight = -bitmap_height;
    bitmap_info.bmiHeader.biPlanes = 1;
    bitmap_info.bmiHeader.biBitCount = 32;
    bitmap_info.bmiHeader.biCompression = BI_RGB;

    int bitmap_memory_size = (bitmap_width * bitmap_height) * bytes_per_pixel;
    bitmap_memory = VirtualAlloc(NULL, bitmap_memory_size, MEM_COMMIT, PAGE_READWRITE);

    render_weird_gradient(128, 0);
}

internal void
win32_update_window(HDC device_context, RECT *window_rect, int x, int y, int width, int height)
{
    int window_width = window_rect->right - window_rect->left;
    int window_height = window_rect->bottom - window_rect->top;

    StretchDIBits(device_context, 0, 0, bitmap_width, bitmap_height, 0, 0, window_width,
                  window_height, bitmap_memory, &bitmap_info, DIB_RGB_COLORS, SRCCOPY);
}

LRESULT CALLBACK
Win32MainWindowCallback(HWND window, UINT message, WPARAM w_param, LPARAM l_param)
{
    LRESULT result = 0;

    switch (message)
    {
        case WM_SIZE:
        {
            RECT client_rect;
            GetClientRect(window, &client_rect);

            int width = client_rect.right - client_rect.left;
            int height = client_rect.bottom - client_rect.top;

            win32_resize_dib_section(width, height);
            OutputDebugStringA("WM_SIZE\n");
        }
        break;

        case WM_DESTROY:
        {
            running = false;
            OutputDebugStringA("WM_DESTROY\n");
        }
        break;

        case WM_CLOSE:
        {
            // todo(nils): message to user?
            running = false;
            OutputDebugStringA("WM_CLOSE\n");
        }
        break;

        case WM_ACTIVATEAPP:
        {
            OutputDebugStringA("WM_ACTIVATEAPP\n");
        }
        break;

        case WM_PAINT:
        {
            PAINTSTRUCT paint_struct;
            HDC device_context = BeginPaint(window, &paint_struct);

            LONG x = paint_struct.rcPaint.left;
            LONG y = paint_struct.rcPaint.top;
            LONG width = paint_struct.rcPaint.right - paint_struct.rcPaint.left;
            LONG height = paint_struct.rcPaint.bottom - paint_struct.rcPaint.top;

            RECT client_rect;
            GetClientRect(window, &client_rect);

            win32_update_window(device_context, &client_rect, x, y, width, height);

            EndPaint(window, &paint_struct);
        }
        break;

        default:
        {
            OutputDebugStringA("default\n");
            result = DefWindowProc(window, message, w_param, l_param);
        }
        break;
    }

    return (result);
}

int CALLBACK
WinMain(HINSTANCE instance, HINSTANCE prev_instance, LPSTR command_line, int show_code)
{
    WNDCLASSA window_class = {};

    window_class.style = CS_HREDRAW | CS_VREDRAW;
    window_class.lpfnWndProc = Win32MainWindowCallback;
    window_class.hInstance = instance;
    window_class.lpszClassName = "HandmadeHeroWindowClass";

    if (RegisterClassA(&window_class))
    {
        HWND window_handle =
            CreateWindowExA(0, window_class.lpszClassName, "Handmade Hero",
                            WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT,
                            CW_USEDEFAULT, CW_USEDEFAULT, nullptr, nullptr, instance, nullptr);

        if (window_handle)
        {
            running = true;
            int x_offset = 0;
            int y_offset = 0;
            while (running)
            {
                MSG message;

                while (PeekMessage(&message, 0, 0, 0, PM_REMOVE))
                {
                    if (message.message == WM_QUIT)
                    {
                        running = false;
                    }
                    TranslateMessage(&message);
                    DispatchMessageA(&message);
                }

                render_weird_gradient(x_offset, y_offset);

                HDC device_context = GetDC(window_handle);
                RECT client_rect;
                GetClientRect(window_handle, &client_rect);
                int window_width = client_rect.right - client_rect.left;
                int window_height = client_rect.bottom - client_rect.top;
                win32_update_window(device_context, &client_rect, 0, 0, window_width,
                                    window_height);
                ReleaseDC(window_handle, device_context);

                ++x_offset;
                ++y_offset;
            }
        }
        else
        {
            // todo(nils): logging
        }
    }
    else
    {
        // todo(nils): logging
    }

    return (0);
}
