
#include "asteroid_platform.h"
#include "asteroid_intrinsics.h"
#include "asteroid_math.h"

#include "asteroid.h"
#include "asteroid.c"

#include <stdio.h>
#include <windows.h>

#include "win32_asteroid.h"

global b32 GlobalRunning;
global win32_backbuffer GlobalBackbuffer;
global game_input GlobalGameInput;

internal void Win32HandleKeyboardButton(game_input_button* Button, b32 IsDown)
{
    Button->IsDown = IsDown;
    Button->Changed = true;
}

internal void Win32ToggleFullscreen(HWND Window)
{
    persist WINDOWPLACEMENT LastWindowPlacement;
    
    DWORD Style = GetWindowLongA(Window, GWL_STYLE);
    if (Style & WS_OVERLAPPEDWINDOW)
    {
        HMONITOR Monitor = MonitorFromWindow(Window, MONITOR_DEFAULTTOPRIMARY);
        
        MONITORINFO MonitorInfo = {0};
        MonitorInfo.cbSize = sizeof(MonitorInfo);
        
        if (GetWindowPlacement(Window, &LastWindowPlacement) &&
            GetMonitorInfoA(Monitor, &MonitorInfo))
        {
            SetWindowLongA(Window, GWL_STYLE, Style & ~WS_OVERLAPPEDWINDOW);
            
            RECT MonitorRect = MonitorInfo.rcMonitor;
            
            s32 X = MonitorRect.left;
            s32 Y = MonitorRect.top;
            s32 Width = MonitorRect.right - MonitorRect.left;
            s32 Height = MonitorRect.bottom - MonitorRect.top;
            
            SetWindowPos(Window, HWND_TOP,
                         X, Y, Width, Height,
                         SWP_FRAMECHANGED|SWP_NOOWNERZORDER);
        }
    }
    else
    {
        SetWindowLongA(Window, GWL_STYLE, Style | WS_OVERLAPPEDWINDOW);
        SetWindowPlacement(Window, &LastWindowPlacement);
        SetWindowPos(Window, 0,
                     0, 0, 0, 0,
                     SWP_NOSIZE|SWP_NOMOVE|SWP_NOZORDER|
                     SWP_FRAMECHANGED|SWP_NOOWNERZORDER);
    }
}

internal LRESULT CALLBACK Win32WindowCallback(HWND Window,
                                              UINT Message,
                                              WPARAM WParam,
                                              LPARAM LParam)
{
    LRESULT Result = 0;
    
    switch (Message)
    {
        case WM_CLOSE:
        {
            GlobalRunning = false;
        } break;
        
        case WM_SIZE:
        {
            GlobalBackbuffer.Width = LOWORD(LParam);
            GlobalBackbuffer.Height = HIWORD(LParam);
            GlobalBackbuffer.Pitch = GlobalBackbuffer.Width;
            
            GlobalBackbuffer.BitmapInfo.bmiHeader.biSize = sizeof(GlobalBackbuffer.BitmapInfo.bmiHeader);
            GlobalBackbuffer.BitmapInfo.bmiHeader.biWidth = GlobalBackbuffer.Pitch;
            GlobalBackbuffer.BitmapInfo.bmiHeader.biHeight = GlobalBackbuffer.Height;
            GlobalBackbuffer.BitmapInfo.bmiHeader.biPlanes = 1;
            GlobalBackbuffer.BitmapInfo.bmiHeader.biBitCount = 32;
            GlobalBackbuffer.BitmapInfo.bmiHeader.biCompression = BI_RGB;
            
            if (GlobalBackbuffer.Memory)
            {
                VirtualFree(GlobalBackbuffer.Memory, 0, MEM_RELEASE);
            }
            
            u32 BytesPerPixel = 4;
            u32 BackbufferSize = GlobalBackbuffer.Pitch * GlobalBackbuffer.Height * BytesPerPixel;
            
            GlobalBackbuffer.Memory = VirtualAlloc(0, BackbufferSize,
                                                   MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
        } break;
        
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYDOWN:
        case WM_KEYUP:
        {
            u32 VKCode = (u32)WParam;
            
            b32 IsDown = ((LParam & (1 << 31)) == 0);
            b32 AltIsDown = ((LParam & (1 << 29)) != 0);
            
            if (IsDown)
            {
                if (AltIsDown && VKCode == VK_F4)
                {
                    GlobalRunning = false;
                }
                else if (VKCode == VK_F11)
                {
                    Win32ToggleFullscreen(Window);
                }
            }
            
            if (VKCode == VK_UP)
            {
                Win32HandleKeyboardButton(&GlobalGameInput.Thrust, IsDown);
            }
            else if (VKCode == VK_LEFT)
            {
                Win32HandleKeyboardButton(&GlobalGameInput.TurnLeft, IsDown);
            }
            else if (VKCode == VK_RIGHT)
            {
                Win32HandleKeyboardButton(&GlobalGameInput.TurnRight, IsDown);
            }
            else if (VKCode == 'Z')
            {
                Win32HandleKeyboardButton(&GlobalGameInput.Shoot, IsDown);
            }
        } break;
        
        default:
        {
            Result = DefWindowProcW(Window, Message, WParam, LParam);
        } break;
    }
    
    return (Result);
}

internal HWND Win32CreateWindow(void)
{
    WNDCLASSW WindowClass = {0};
    WindowClass.hInstance = GetModuleHandleA(0);
    WindowClass.hIcon = LoadIcon(0, IDI_APPLICATION);
    WindowClass.hCursor = LoadCursor(0, IDC_ARROW);
    WindowClass.lpfnWndProc = &Win32WindowCallback;
    WindowClass.lpszClassName = L"Asteroid";
    
    HWND Window = 0;
    if (RegisterClassW(&WindowClass))
    {
        Window = CreateWindowExW(WS_EX_APPWINDOW,
                                 WindowClass.lpszClassName,
                                 L"Asteroid",
                                 WS_OVERLAPPEDWINDOW,
                                 CW_USEDEFAULT, CW_USEDEFAULT,
                                 CW_USEDEFAULT, CW_USEDEFAULT,
                                 0, 0, WindowClass.hInstance, 0);
    }
    
    return (Window);
}

int WINAPI wWinMain(HINSTANCE Instance,
                    HINSTANCE PrevInstance,
                    LPWSTR CommandLine,
                    int ShowFlags)
{
    HWND Window = Win32CreateWindow();
    Assert(IsWindow(Window));
    
    HDC DeviceContext = GetDC(Window);
    
    game_memory GameMemory = {0};
    GameMemory.PermanentStorageSize = MB(64);
    GameMemory.TransientStorageSize = GB(4);
    
    GameMemory.PermanentStorage = VirtualAlloc(0, GameMemory.PermanentStorageSize,
                                               MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
    
    GameMemory.TransientStorage = VirtualAlloc(0, GameMemory.TransientStorageSize,
                                               MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
    
    GameMemory.IsInitialized = false;
    
    LARGE_INTEGER Frequency;
    QueryPerformanceFrequency(&Frequency);
    
    umm TargetFrameRate = 60;
    
    {
        HDC DisplayDC = GetDC(0);
        s32 RefreshRate = GetDeviceCaps(DisplayDC, VREFRESH);
        if (RefreshRate > 1)
        {
            TargetFrameRate = RefreshRate;
        }
        ReleaseDC(0, DisplayDC);
    }
    
    umm TargetClockElapsed = Frequency.QuadPart / TargetFrameRate;
    
    GlobalRunning = true;
    ShowWindow(Window, SW_SHOWDEFAULT);
    
    while (GlobalRunning)
    {
        LARGE_INTEGER FrameStart;
        QueryPerformanceCounter(&FrameStart);
        
        for (u32 ButtonIndex = 0;
             ButtonIndex < ArrayCount(GlobalGameInput.Buttons);
             ButtonIndex++)
        {
            game_input_button* Button = GlobalGameInput.Buttons + ButtonIndex;
            Button->Changed = false;
        }
        
        MSG Message;
        while (PeekMessageW(&Message, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&Message);
            DispatchMessageW(&Message);
        }
        
        game_backbuffer GameBackbuffer = {0};
        GameBackbuffer.Memory = GlobalBackbuffer.Memory;
        GameBackbuffer.Width = GlobalBackbuffer.Width;
        GameBackbuffer.Height = GlobalBackbuffer.Height;
        GameBackbuffer.Pitch = GlobalBackbuffer.Pitch;
        
        GameUpdateAndRender(&GameMemory, &GlobalGameInput, &GameBackbuffer);
        
        StretchDIBits(DeviceContext,
                      0, 0, GlobalBackbuffer.Width, GlobalBackbuffer.Height,
                      0, 0, GlobalBackbuffer.Width, GlobalBackbuffer.Height,
                      GlobalBackbuffer.Memory,
                      &GlobalBackbuffer.BitmapInfo,
                      DIB_RGB_COLORS, SRCCOPY);
        
        LARGE_INTEGER FrameEnd;
        QueryPerformanceCounter(&FrameEnd);
        
        umm ClockElapsed = FrameEnd.QuadPart - FrameStart.QuadPart;
        if (ClockElapsed < TargetClockElapsed)
        {
            umm ClockRemaining = TargetClockElapsed - ClockElapsed;
            DWORD MSRemaining = (DWORD)(1000 * ClockRemaining / Frequency.QuadPart);
            if (MSRemaining > 0)
            {
                timeBeginPeriod(1);
                Sleep(MSRemaining);
                timeEndPeriod(1);
            }
            
            while (ClockElapsed < TargetClockElapsed)
            {
                QueryPerformanceCounter(&FrameEnd);
                ClockElapsed = FrameEnd.QuadPart - FrameStart.QuadPart;
            }
        }
        
        GlobalGameInput.DeltaTime = (f32)ClockElapsed / (f32)Frequency.QuadPart;
        
        persist char Buffer[256];
        snprintf(Buffer, sizeof(Buffer),
                 "Asteroid | RenderFPS=%.2f",
                 1.0 / GlobalGameInput.DeltaTime);
        SetWindowTextA(Window, Buffer);
    }
    
    return (0);
}
