#include "platform/platform.h"

// Windows platform layer.
#if TPLATFORM_WINDOWS

#include "core/logger.h"
#include "core/input.h"
#include "core/event.h"

#include "containers/darray.h"

#include <windows.h>
#include <windowsx.h> // param input extraction
#include <stdlib.h>

// For surface creation
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>
#include "renderer/vulkan/vulkan_types.inl"


typedef struct platform_state{
    HINSTANCE h_instance;
    HWND hwnd;
    VkSurfaceKHR surface;

}platform_state;

static platform_state* state_ptr;


// Clock
static f64 clock_frequency;
static LARGE_INTEGER start_time;

LRESULT CALLBACK win32_process_message(HWND hwnd, u32 msg, WPARAM w_param, LPARAM l_param);

void clock_setup(){
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);
    clock_frequency = 1.0 / (f64)frequency.QuadPart;
    QueryPerformanceCounter(&start_time);
}

b8 platform_system_startup(u64* memory_requirement, void* state, const char* application_name, i32 x, i32 y, i32 width, i32 height){
    
    *memory_requirement = sizeof(platform_state);
    if(state == 0){
        return TRUE;
    }
    state_ptr = state;

    state_ptr->h_instance = GetModuleHandleA(0);

    // Setup and register window class.
    HICON icon = LoadIcon(state_ptr->h_instance, IDI_APPLICATION);
    WNDCLASSA wc;
    memset(&wc, 0, sizeof(wc));
    wc.style =  CS_DBLCLKS; // Get double-clicks
    wc.lpfnWndProc = win32_process_message;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = state_ptr->h_instance;
    wc.hIcon = icon;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);   // NULL; // Manage the cursor manually
    wc.hbrBackground = NULL;                    // Transparent
    wc.lpszClassName = "taller_window_class";

    if(!RegisterClassA(&wc)){
        MessageBoxA(0, "Window registration failed", "Error", MB_ICONEXCLAMATION | MB_OK);
        return FALSE;
    }

    // Create window
    u32 client_x = x;
    u32 client_y = y;
    u32 client_width = width;
    u32 client_height = height;

    u32 window_x = client_x;
    u32 window_y = client_y;
    u32 window_width = client_width;
    u32 window_height = client_height;

    u32 window_style = WS_OVERLAPPED |WS_SYSMENU | WS_CAPTION;
    u32 window_ex_style = WS_EX_APPWINDOW;

    window_style |= WS_MAXIMIZEBOX;
    window_style |= WS_MINIMIZEBOX;
    window_style |= WS_THICKFRAME;

    // Obtain the size of the border.
    RECT border_rect = {0, 0, 0, 0};
    AdjustWindowRectEx(&border_rect, window_style, 0, window_ex_style);

    // In this case, the border rectangle is negative.
    window_x += border_rect.left;
    window_y += border_rect.top;

    // Grow by the size of the OS border.
    window_width += border_rect.right - border_rect.left;
    window_height += border_rect.bottom - border_rect.top;

    HWND handle = CreateWindowExA(window_ex_style, "taller_window_class", application_name, window_style, window_x, window_y, window_width, window_height, 0, 0, state_ptr->h_instance, 0);

    if(handle == 0){
        MessageBoxA(NULL, "Window creation failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);

        TFATAL("Window creation failed!");
        return FALSE;
    
    } else{
        state_ptr->hwnd = handle;
    }

    // Show the window
    b32 should_activate = 1; // TODO: if the window should not accept input, this should be false.
    i32 show_window_command_flag = should_activate ? SW_SHOW : SW_SHOWNOACTIVATE;
    // If initially minimized, use SW_MINIMIZE : SW_SHOWMINNOACTIVE;
    // If initially maximized, use SW_SHOWMAXIMIZED : SW_MAXIMIZE;
    ShowWindow(state_ptr->hwnd, show_window_command_flag);

    // Clock setup
    clock_setup();

    return TRUE;
}


void platform_system_shutdown(void* plat_state){
    if(state_ptr && state_ptr->hwnd){
        DestroyWindow(state_ptr->hwnd);
        state_ptr->hwnd = 0;
    }
}

b8 platform_pump_messages(){
    if(state_ptr){
        MSG message;
        while(PeekMessageA(&message, NULL, 0, 0, PM_REMOVE)){
            TranslateMessage(&message);
            DispatchMessageA(&message);
        }
    }
    return TRUE;
}

void* platform_allocate(u64 size, b8 aligned){
    return malloc(size);
}

void platform_free(void* block, b8 aligned){
    free(block);
}

void* platform_zero_memory(void* block, u64 size){
    return memset(block, 0, size);
}

void* platform_copy_memory(void* dest, const void* source, u64 size){
    return memcpy(dest, source, size);
}

void* platform_set_memory(void* dest, i32 value, u64 size){
    return memset(dest, value, size);
}

void platform_console_write(const char* message, u8 color){
    HANDLE console_handle =  GetStdHandle(STD_OUTPUT_HANDLE);
    // FATAL, ERROR, WARN, INFO, DEBUG, TRACE
    static u8 levels[6] = {64, 4, 6, 2, 1, 8};
    SetConsoleTextAttribute(console_handle, levels[color]);
    OutputDebugStringA(message);
    u64 length = strlen(message);
    LPDWORD number_written = 0;
    WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), message, (DWORD)length, number_written, 0);
}

void platform_console_write_error(const char* message, u8 color){
        HANDLE console_handle =  GetStdHandle(STD_ERROR_HANDLE);
    // FATAL, ERROR, WARN, INFO, DEBUG, TRACE
    static u8 levels[6] = {64, 4, 6, 2, 1, 8};
    SetConsoleTextAttribute(console_handle, levels[color]);
    OutputDebugStringA(message);
    u64 length = strlen(message);
    LPDWORD number_written = 0;
    WriteConsoleA(GetStdHandle(STD_ERROR_HANDLE), message, (DWORD)length, number_written, 0);
}

f64 platform_get_absolute_time(){
    if(!clock_frequency){
        clock_setup();
    }
    
    LARGE_INTEGER now_time;
    QueryPerformanceCounter(&now_time);
    return (f64)now_time.QuadPart * clock_frequency;
}

void platform_sleep(u64 ms){
    Sleep(ms);
}

void platform_get_required_extension_names(const char*** names_darray){
    darray_push(*names_darray, &"VK_KHR_win32_surface");
}

// Surface creation for vulkan
b8 platform_create_vulkan_surface(vulkan_context* context){
    if(!state_ptr){
        return FALSE;
    }

    VkWin32SurfaceCreateInfoKHR create_info = {VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR};
    create_info.hinstance = state_ptr->h_instance;
    create_info.hwnd = state_ptr->hwnd;

    VkResult result = vkCreateWin32SurfaceKHR(context->instance, &create_info, context->allocator, &state_ptr->surface);
    if(result != VK_SUCCESS){
        TFATAL("Vulkan surface creation failed.");
        return FALSE;
    }

    context->surface = state_ptr->surface;
    return TRUE;
}

LRESULT CALLBACK win32_process_message(HWND hwnd, u32 msg, WPARAM w_param, LPARAM l_param){
    switch(msg){
        case WM_ERASEBKGND:
            // Notify the OS that erasing will be handled by application to prevent flicker
            return 1;

        case WM_CLOSE:
            // Fire an event for the application to quit.
            event_context data = {};
            event_fire(EVENT_CODE_APPLICATION_QUIT, 0, data);
            
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        case WM_SIZE: {
            // Get the updated size.
            RECT r;
            GetClientRect(hwnd, &r);
            u32 width = r.right - r.left;
            u32 height = r.bottom - r.top;
        
            // Fire the event. The application layer should pick this up, but not handle it
            // as it shouldn be visible to other parts of the application.
            event_context context;
            context.data.u16[0] = (u16)width;
            context.data.u16[1] = (u16)height;
            event_fire(EVENT_CODE_RESIZED, 0, context);
        }break;

        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYUP: {
            // Key pressed/released
            b8 pressed = (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN);
            keys key = (u16)w_param;


            /**
             * TODO:
             * 
                @zackshrout6726
                hace 2 años
                Hey I am sure you caught this and later commits will reflect it, but the reason the shift/ctrl/alt keys are only registering as pressed 
                once is because GetKeyState() return false if the key is not pressed, so input_process_key() is never called with any of our KEY_RALT, 
                etc, enums with pressed == false... meaning that if (state.keyboard_current.keys[key] != pressed) always evaluated to false after the 
                first stroke.
               
                @TravisVroman
                hace 2 años
                Yep, it definitely wasn't right. I did wind up refactoring this somewhat down the road. 
             */
            // Check for extended scan code.
            b8 is_extended = (HIWORD(l_param) & KF_EXTENDED) == KF_EXTENDED;

            // Special keys
            if(w_param == VK_MENU){
                key = is_extended ? KEY_RALT : KEY_LALT;
            }else if(w_param == VK_SHIFT){
               // KF_EXTENDED is not set for shift keys.
               u32 left_shift = MapVirtualKey(VK_LSHIFT, MAPVK_VK_TO_VSC);
               u32 scancode = ((l_param & (0xFF << 16)) >> 16);
               key = scancode == left_shift ? KEY_LSHIFT : KEY_RSHIFT;
            } else if(w_param == VK_CONTROL){
               key = is_extended ? KEY_RCONTROL : KEY_LCONTROL;
            }

            // Pass to the input subsystemfor processing.
            input_process_key(key, pressed); 
        }break;

        case WM_MOUSEMOVE: {
            // Mouse move
            i32 x_position = GET_X_LPARAM(l_param);
            i32 y_position = GET_Y_LPARAM(l_param);
            
            // Pass over to the input subsystem
            input_process_mouse_move(x_position, y_position);
        }break;

        case WM_MOUSEWHEEL: {
            i32 z_delta = GET_WHEEL_DELTA_WPARAM(w_param);
            if(z_delta != 0) {
                // Flatten the input to an OS-independet (-1, 1)
                z_delta = (z_delta < 0) ? -1 : 1;
                input_process_mouse_wheel(z_delta);
            }
        }break;

        case WM_LBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_MBUTTONUP:
        case WM_RBUTTONUP: {
            b8 pressed = msg == WM_LBUTTONDOWN || msg == WM_MBUTTONDOWN || msg == WM_RBUTTONDOWN;
            buttons mouse_button = BUTTON_MAX_BUTTONS;
            switch(msg) {
                case WM_LBUTTONDOWN:
                case WM_LBUTTONUP:
                    mouse_button = BUTTON_LEFT;
                    break;
                case WM_MBUTTONDOWN:
                case WM_MBUTTONUP:
                    mouse_button = BUTTON_MIDDLE;
                    break;
                case WM_RBUTTONDOWN:
                case WM_RBUTTONUP:
                    mouse_button = BUTTON_RIGHT;
                    break;
            }

            // Pass over to the input subsystem.
            if(mouse_button != BUTTON_MAX_BUTTONS){
                input_process_button(mouse_button, pressed);
            }
        }break;
    }

    return DefWindowProcA(hwnd, msg, w_param, l_param);
}

#endif // TPLATFORM_WINDOWS