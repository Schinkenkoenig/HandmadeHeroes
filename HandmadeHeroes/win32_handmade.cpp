#include <stdint.h>
#include <windows.h>
#include <xinput.h>

#define internal static
#define local_persist static
#define global_variable static

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

struct win32_offscreen_buffer
{
    BITMAPINFO info;
    void *memory;
    int width;
    int height;
    int pitch;
};

struct win32_window_dimension
{
    int width;
    int height;
};

// XInputGetState to default to noop
#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dw_user_index, XINPUT_STATE *p_state)
typedef X_INPUT_GET_STATE(x_input_get_state);
X_INPUT_GET_STATE(XInputGetStateStub)
{
    return 0;
}
global_variable x_input_get_state *XInputGetState_ = XInputGetStateStub;

// XInputSetState to default to noop
#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dw_user_index, XINPUT_VIBRATION *p_vibration)
typedef X_INPUT_SET_STATE(x_input_set_state);
X_INPUT_SET_STATE(XInputSetStateStub)
{
    return 0;
}
global_variable x_input_set_state *XInputSetState_ = XInputSetStateStub;

#define XInputGetState XInputGetState_
#define XInputSetState XInputSetState_

global_variable bool global_running = true;
global_variable win32_offscreen_buffer global_backbuffer;

internal void
win32_load_x_input()
{
    HMODULE x_input_library = LoadLibraryA("x_input1_4.dll");

    if (x_input_library)
    {
        XInputGetState = (x_input_get_state *)GetProcAddress(x_input_library, "XInputGetState");
        XInputSetState = (x_input_set_state *)GetProcAddress(x_input_library, "XInputSetState");
    }
}

win32_window_dimension
win32_get_window_dimension(HWND window)
{
    win32_window_dimension result;

    RECT client_rect;
    GetClientRect(window, &client_rect);
    result.width = client_rect.right - client_rect.left;
    result.height = client_rect.bottom - client_rect.top;

    return result;
}

internal void
render_weird_gradient(win32_offscreen_buffer *buffer, int blue_offset, int green_offset)
{
    uint8 *row = (uint8 *)buffer->memory;

    for(int y=0; y < buffer->height; ++y)
    {
        uint32 *pixel = (uint32 *)row;
        for(int x=0; x < buffer->width; ++x)
        {
            uint8 blue = (x + blue_offset);
            uint8 green = (y + green_offset);

            *pixel++ = ((green << 8) | blue);
        }

        row += buffer->pitch;
    }
}

internal void
win32_resize_dib_section(win32_offscreen_buffer *buffer, int width, int height)
{
    if (buffer->memory)
    {
        VirtualFree(buffer->memory, 0, MEM_RELEASE);
    }

    buffer->height = height;
    buffer->width = width;

    int bytes_per_pixel = 4;


    buffer->info.bmiHeader.biSize = sizeof(buffer->info.bmiHeader);
    buffer->info.bmiHeader.biWidth = buffer->width;
    buffer->info.bmiHeader.biHeight = -buffer->height;
    buffer->info.bmiHeader.biPlanes = 1;
    buffer->info.bmiHeader.biBitCount = 32;
    buffer->info.bmiHeader.biCompression = BI_RGB;

    int bitmap_memory_size = (buffer->width * buffer->height) * bytes_per_pixel;
    buffer->memory = VirtualAlloc(0, bitmap_memory_size, MEM_COMMIT, PAGE_READWRITE);
    buffer->pitch = width * bytes_per_pixel;
}

internal void
win32_display_buffer_in_window(win32_offscreen_buffer *buffer, HDC device_context, int window_width, int window_height)
{
    StretchDIBits(device_context,
        0, 0, window_width, window_height,
        0, 0, buffer->width, buffer->height,
        buffer->memory, &buffer->info, DIB_RGB_COLORS, SRCCOPY);
}

LRESULT CALLBACK
Win32MainWindowCallback(HWND window, UINT message, WPARAM w_param, LPARAM l_param)
{
    LRESULT result = 0;

    switch (message)
    {
        case WM_CLOSE:
        {
            // todo(nils): message to user?
            global_running = false;
        }
        break;

        case WM_ACTIVATEAPP:
        {
            OutputDebugStringA("WM_ACTIVATEAPP\n");
        }
        break;

        case WM_DESTROY:
        {
            global_running = false;
        }
        break;

        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYDOWN:
        case WM_KEYUP:
        {
            uint32 vk_code = w_param;
            bool wasDown = ((l_param & (1 <<30)) != 0);
            bool isDown = ((l_param & (1 << 31)) == 0);

            if (isDown == wasDown)
            {
                break;
            }

            if (vk_code == 'W')
            {
            }
            else if (vk_code == 'A')
            {
            }
            else if (vk_code == 'S')
            {
            }
            else if (vk_code == 'D')
            {
            }
            else if (vk_code == 'Q')
            {
            }
            else if (vk_code == 'E')
            {
            }
            else if (vk_code == VK_UP)
            {
            }
            else if (vk_code == VK_DOWN)
            {
            }
            else if (vk_code == VK_LEFT)
            {
            }
            else if (vk_code == VK_RIGHT)
            {
            }
            else if (vk_code == VK_ESCAPE)
            {
            }
            else if (vk_code == VK_SPACE)
            {
            }
        }


        case WM_PAINT:
        {
            PAINTSTRUCT paint;
            HDC device_context = BeginPaint(window, &paint);
            win32_window_dimension dimension = win32_get_window_dimension(window);

            win32_display_buffer_in_window(&global_backbuffer, device_context, dimension.width, dimension.height);

            EndPaint(window, &paint);
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
    win32_load_x_input();
    WNDCLASSA window_class = {};

    win32_resize_dib_section(&global_backbuffer, 1280, 720);

    window_class.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    window_class.lpfnWndProc = Win32MainWindowCallback;
    window_class.hInstance = instance;
    window_class.lpszClassName = "HandmadeHeroWindowClass";

    if (RegisterClassA(&window_class))
    {
        HWND window_handle =
            CreateWindowExA(
                0,
                window_class.lpszClassName,
                "Handmade Hero",
                WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                0,
                0,
                instance,
                0);

        if (window_handle)
        {
            HDC device_context = GetDC(window_handle);

            int x_offset = 0;
            int y_offset = 0;

            global_running = true;
            while (global_running)
            {
                MSG message;

                while (PeekMessage(&message, 0, 0, 0, PM_REMOVE))
                {
                    if (message.message == WM_QUIT)
                    {
                        global_running = false;
                    }

                    TranslateMessage(&message);
                    DispatchMessageA(&message);
                }

                for(DWORD controller_index =0;
                    controller_index < XUSER_MAX_COUNT;
                    ++controller_index)
                {
                    XINPUT_STATE controller_state;
                    if (XInputGetState(controller_index, &controller_state) == ERROR_SUCCESS)
                    {
                        // plugged in
                        XINPUT_GAMEPAD *pad = &controller_state.Gamepad;

                        // Button bitmask -> boolean flags
                        WORD buttons = pad->wButtons;
                        bool up = (buttons & XINPUT_GAMEPAD_DPAD_UP);
                        bool down = (buttons & XINPUT_GAMEPAD_DPAD_DOWN);
                        bool left = (buttons & XINPUT_GAMEPAD_DPAD_LEFT);
                        bool right = (buttons & XINPUT_GAMEPAD_DPAD_RIGHT);
                        bool startn   = (buttons & XINPUT_GAMEPAD_START);
                        bool back    = (buttons & XINPUT_GAMEPAD_BACK);
                        bool left_shoulder  = (buttons & XINPUT_GAMEPAD_LEFT_SHOULDER);
                        bool right_shoulder = (buttons & XINPUT_GAMEPAD_RIGHT_SHOULDER);
                        bool a= (buttons & XINPUT_GAMEPAD_A);
                        bool b= (buttons & XINPUT_GAMEPAD_B);
                        bool x= (buttons & XINPUT_GAMEPAD_X);
                        bool y= (buttons & XINPUT_GAMEPAD_Y);

                        int16 stick_x = pad->sThumbLX;
                        int16 stick_y = pad->sThumbLY;

                        if (a)
                        {
                            y_offset += 2;
                        }
                    }
                    else
                    {
                        // not plugged in
                    }
                }

                render_weird_gradient(&global_backbuffer, x_offset, y_offset);

                win32_window_dimension dimension = win32_get_window_dimension(window_handle);

                win32_display_buffer_in_window(&global_backbuffer, device_context, dimension.width, dimension.height);

                ++x_offset;
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
