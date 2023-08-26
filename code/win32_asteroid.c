
#include "asteroid_platform.h"
#include "asteroid_intrinsics.h"
#include "asteroid_math.h"

#include "asteroid.h"
#include "asteroid.c"

#include <stdio.h>
#include <windows.h>
#include <dsound.h>

#include "win32_asteroid.h"

global b32 GlobalRunning;
global win32_backbuffer GlobalBackbuffer;
global game_input GlobalGameInput;

global LPDIRECTSOUNDBUFFER GlobalSoundBuffer;

internal PLATFORM_READ_ENTIRE_FILE(Win32ReadEntireFile)
{
    entire_file Result = {0};
    
    HANDLE FileHandle = CreateFileA(FileName, GENERIC_READ, 0, 0, OPEN_EXISTING, 0, 0);
    if (FileHandle != INVALID_HANDLE_VALUE)
    {
        LARGE_INTEGER FileSize;
        GetFileSizeEx(FileHandle, &FileSize);
        
        Assert(FileSize.QuadPart <= GB(4));
        
        Result.ContentsSize = FileSize.QuadPart;
        Result.Contents = VirtualAlloc(0, Result.ContentsSize,
                                       MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
        
        DWORD BytesToRead = (DWORD)Result.ContentsSize;
        DWORD BytesRead;
        
        ReadFile(FileHandle, Result.Contents, BytesToRead, &BytesRead, 0);
        Assert(BytesRead == BytesToRead);
        
        CloseHandle(FileHandle);
    }
    
    return (Result);
}

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

typedef HRESULT WINAPI direct_sound_create(LPGUID lpGuid, LPDIRECTSOUND* ppDS, LPUNKNOWN pUnkOuter);
internal void Win32InitDSound(HWND Window,
                              s32 SampleRate,
                              s32 SoundBufferSize)
{
    HMODULE DSoundLibrary = LoadLibraryA("dsound.dll");
    
    b32 Succeeded = false;
    
    if (DSoundLibrary)
    {
        direct_sound_create* DirectSoundCreate = (direct_sound_create*)
            GetProcAddress(DSoundLibrary, "DirectSoundCreate");
        
        LPDIRECTSOUND DSound;
        if (DirectSoundCreate && SUCCEEDED(DirectSoundCreate(0, &DSound, 0)))
        {
            WAVEFORMATEX WaveFormat = {0};
            WaveFormat.wFormatTag = WAVE_FORMAT_PCM;
            WaveFormat.nSamplesPerSec = SampleRate;
            WaveFormat.nChannels = 2;
            WaveFormat.wBitsPerSample = 16;
            WaveFormat.nBlockAlign = (WaveFormat.nChannels * WaveFormat.wBitsPerSample) / 8;
            WaveFormat.nAvgBytesPerSec = WaveFormat.nSamplesPerSec * WaveFormat.nBlockAlign;
            
            if (SUCCEEDED(DSound->lpVtbl->SetCooperativeLevel(DSound, Window, DSSCL_PRIORITY)))
            {
                DSBUFFERDESC BufferDescription = {0};
                BufferDescription.dwSize = sizeof(BufferDescription);
                BufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER;
                
                LPDIRECTSOUNDBUFFER PrimaryBuffer;
                if (SUCCEEDED(DSound->lpVtbl->CreateSoundBuffer(DSound,
                                                                &BufferDescription,
                                                                &PrimaryBuffer,
                                                                0)))
                {
                    if (SUCCEEDED(PrimaryBuffer->lpVtbl->SetFormat(PrimaryBuffer,
                                                                   &WaveFormat)))
                    {
                        Succeeded = true;
                    }
                }
            }
            
            if (Succeeded)
            {
                DSBUFFERDESC BufferDescription = {0};
                BufferDescription.dwSize = sizeof(BufferDescription);
                BufferDescription.dwFlags = DSBCAPS_GETCURRENTPOSITION2|DSBCAPS_GLOBALFOCUS;
                BufferDescription.dwBufferBytes = SoundBufferSize;
                BufferDescription.lpwfxFormat = &WaveFormat;
                
                if (SUCCEEDED(DSound->lpVtbl->CreateSoundBuffer(DSound,
                                                                &BufferDescription,
                                                                &GlobalSoundBuffer,
                                                                0)))
                {
                    Succeeded = true;
                }
                else
                {
                    Succeeded = false;
                }
            }
        }
    }
    
    Assert(Succeeded);
}

internal void Win32ClearSoundBuffer(s32 SoundBufferSize)
{
    void* Region;
    DWORD RegionSize;
    
    if (SUCCEEDED(GlobalSoundBuffer->lpVtbl->Lock(GlobalSoundBuffer,
                                                  0, SoundBufferSize,
                                                  &Region, &RegionSize,
                                                  0, 0,
                                                  0)))
    {
        u8* Byte = (u8*)Region;
        for (DWORD ByteIndex = 0;
             ByteIndex < RegionSize;
             ByteIndex++)
        {
            *Byte++ = 0;
        }
        
        GlobalSoundBuffer->lpVtbl->Unlock(GlobalSoundBuffer,
                                          Region, RegionSize,
                                          0, 0);
    }
}

int WINAPI wWinMain(HINSTANCE Instance,
                    HINSTANCE PrevInstance,
                    LPWSTR CommandLine,
                    int ShowFlags)
{
    HWND Window = Win32CreateWindow();
    Assert(IsWindow(Window));
    
    win32_sound_output SoundOutput = {0};
    SoundOutput.SampleRate = 48000;
    SoundOutput.LatencySampleRate = SoundOutput.SampleRate / 15;
    SoundOutput.BytesPerSample = sizeof(s16) * 2;
    SoundOutput.SoundBufferSize = SoundOutput.SampleRate * SoundOutput.BytesPerSample * 2;
    SoundOutput.RunningSampleIndex = 0;
    
    Win32InitDSound(Window,
                    SoundOutput.SampleRate,
                    SoundOutput.SoundBufferSize);
    
    Win32ClearSoundBuffer(SoundOutput.SoundBufferSize);
    
    GlobalSoundBuffer->lpVtbl->Play(GlobalSoundBuffer, 0, 0, DSBPLAY_LOOPING);
    
    game_sound_buffer GameSoundBuffer = {0};
    GameSoundBuffer.Samples = VirtualAlloc(0, SoundOutput.SoundBufferSize,
                                           MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
    GameSoundBuffer.SampleRate = SoundOutput.SampleRate;
    GameSoundBuffer.SampleCount = 0;
    
    HDC DeviceContext = GetDC(Window);
    
    game_memory GameMemory = {0};
    GameMemory.PermanentStorageSize = MB(64);
    GameMemory.TransientStorageSize = GB(4);
    
    GameMemory.PermanentStorage = VirtualAlloc(0, GameMemory.PermanentStorageSize,
                                               MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
    
    GameMemory.TransientStorage = VirtualAlloc(0, GameMemory.TransientStorageSize,
                                               MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
    
    GameMemory.PlatformReadEntireFile = Win32ReadEntireFile;
    
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
        
        DWORD PlayCursor;
        DWORD WriteCursor;
        if (SUCCEEDED(GlobalSoundBuffer->lpVtbl->GetCurrentPosition(GlobalSoundBuffer,
                                                                    &PlayCursor, &WriteCursor)))
        {
            DWORD LatencyBytes = SoundOutput.LatencySampleRate * SoundOutput.BytesPerSample;
            DWORD TargetCursor = (PlayCursor + LatencyBytes) % SoundOutput.SoundBufferSize;
            
            DWORD ByteToLock = (SoundOutput.RunningSampleIndex * SoundOutput.BytesPerSample) % SoundOutput.SoundBufferSize;
            
            DWORD BytesToWrite;
            if (ByteToLock > TargetCursor)
            {
                BytesToWrite = SoundOutput.SoundBufferSize - ByteToLock;
                BytesToWrite += TargetCursor;
            }
            else
            {
                BytesToWrite = TargetCursor - ByteToLock;
            }
            
            GameSoundBuffer.SampleCount = BytesToWrite / SoundOutput.BytesPerSample;
            GameGetSoundSamples(&GameMemory, &GameSoundBuffer);
            
            void* Region1;
            DWORD Region1Size;
            void* Region2;
            DWORD Region2Size;
            
            if (SUCCEEDED(GlobalSoundBuffer->lpVtbl->Lock(GlobalSoundBuffer,
                                                          ByteToLock, BytesToWrite,
                                                          &Region1, &Region1Size,
                                                          &Region2, &Region2Size,
                                                          0)))
            {
                s16* SourceSamples = GameSoundBuffer.Samples;
                
                s16* Samples = (s16*)Region1;
                DWORD Region1SampleCount = Region1Size / SoundOutput.BytesPerSample;
                for (DWORD SampleIndex = 0;
                     SampleIndex < Region1SampleCount;
                     SampleIndex++)
                {
                    *Samples++ = *SourceSamples++;
                    *Samples++ = *SourceSamples++;
                    
                    SoundOutput.RunningSampleIndex++;
                }
                
                Samples = (s16*)Region2;
                DWORD Region2SampleCount = Region2Size / SoundOutput.BytesPerSample;
                for (DWORD SampleIndex = 0;
                     SampleIndex < Region2SampleCount;
                     SampleIndex++)
                {
                    *Samples++ = *SourceSamples++;
                    *Samples++ = *SourceSamples++;
                    
                    SoundOutput.RunningSampleIndex++;
                }
                
                GlobalSoundBuffer->lpVtbl->Unlock(GlobalSoundBuffer,
                                                  Region1, Region1Size,
                                                  Region2, Region2Size);
            }
        }
        
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
