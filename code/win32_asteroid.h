/* date = August 26th 2023 7:54 am */

#ifndef WIN32_ASTEROID_H
#define WIN32_ASTEROID_H

typedef struct
{
    void* Memory;
    s32 Width;
    s32 Height;
    s32 Pitch;
    
    BITMAPINFO BitmapInfo;
} win32_backbuffer;

typedef struct
{
    s32 SampleRate;
    s32 LatencySampleRate;
    
    s32 BytesPerSample;
    s32 SoundBufferSize;
    
    u64 RunningSampleIndex;
} win32_sound_output;

#endif //WIN32_ASTEROID_H
