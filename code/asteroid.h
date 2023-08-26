/* date = August 26th 2023 7:59 am */

#ifndef ASTEROID_H
#define ASTEROID_H

typedef struct
{
    void* Contents;
    umm ContentsSize;
} entire_file;

#define Consume(File, Type) (Type*)ConsumeSize(File, sizeof(Type))

internal void* ConsumeSize(entire_file* File, umm Size)
{
    void* Result = 0;
    
    if (File->ContentsSize >= Size)
    {
        Result = File->Contents;
        
        File->Contents = (u8*)File->Contents + Size;
        File->ContentsSize -= Size;
    }
    else
    {
        InvalidCodePath;
    }
    
    return (Result);
}

#define PLATFORM_READ_ENTIRE_FILE(Name) entire_file Name(char* FileName)
typedef PLATFORM_READ_ENTIRE_FILE(platform_read_entire_file);

typedef struct
{
    void* PermanentStorage;
    umm PermanentStorageSize;
    
    void* TransientStorage;
    umm TransientStorageSize;
    
    platform_read_entire_file* PlatformReadEntireFile;
    
    b32 IsInitialized;
} game_memory;

typedef struct
{
    s16* Samples;
    s32 SampleRate;
    u32 SampleCount;
} game_sound_buffer;

typedef struct
{
    b32 IsDown;
    b32 Changed;
} game_input_button;

internal b32 ButtonDown(game_input_button* Button)
{
    b32 Result = Button->IsDown;
    return (Result);
}

internal b32 ButtonUp(game_input_button* Button)
{
    b32 Result = !Button->IsDown;
    return (Result);
}

internal b32 ButtonPressed(game_input_button* Button)
{
    b32 Result = Button->IsDown && Button->Changed;
    return (Result);
}

internal b32 ButtonReleased(game_input_button* Button)
{
    b32 Result = (!Button->IsDown) && (Button->Changed);
    return (Result);
}

typedef struct
{
    f32 DeltaTime;
    
    union
    {
        game_input_button Buttons[4];
        struct
        {
            game_input_button Thrust;
            game_input_button TurnLeft;
            game_input_button TurnRight;
            game_input_button Shoot;
        };
    };
} game_input;

typedef struct
{
    void* Memory;
    s32 Width;
    s32 Height;
    s32 Pitch;
} game_backbuffer;

void GameUpdateAndRender(game_memory* Memory, game_input* Input, game_backbuffer* Backbuffer);

void GameGetSoundSamples(game_memory* Memory, game_sound_buffer* SoundBuffer);

typedef struct
{
    void* Base;
    umm Used;
    umm Size;
    umm TemporaryCount;
} memory_arena;

internal memory_arena InitMemoryArena(void* Base, umm Size)
{
    memory_arena Result = {0};
    Result.Base = Base;
    Result.Used = 0;
    Result.Size = Size;
    Result.TemporaryCount = 0;
    return (Result);
}

#define PushVariable(Arena, Type) (Type*)PushSize(Arena, sizeof(Type))
#define PushArray(Arena, Count, Type) (Type*)PushSize(Arena, Count * sizeof(Type))

internal void* PushSize(memory_arena* Arena, umm Size)
{
    void* Result = 0;
    
    if ((Arena->Used + Size) <= Arena->Size)
    {
        Result = (u8*)Arena->Base + Arena->Used;
        Arena->Used += Size;
    }
    else
    {
        InvalidCodePath;
    }
    
    return (Result);
}

typedef struct
{
    memory_arena* Arena;
    umm LastUsed;
} temporary_memory;

internal temporary_memory BeginTemporaryMemory(memory_arena* Arena)
{
    temporary_memory Result = {0};
    Result.Arena = Arena;
    Result.LastUsed = Arena->Used;
    
    Arena->TemporaryCount++;
    
    return (Result);
}

internal void EndTemporaryMemory(temporary_memory TempMem)
{
    memory_arena* Arena = TempMem.Arena;
    
    Assert(Arena->TemporaryCount > 0);
    Assert(Arena->Used >= TempMem.LastUsed);
    
    Arena->Used = TempMem.LastUsed;
}

typedef struct
{
    u32 PointCount;
    v2 Points[16];
} polygon;

typedef struct
{
    polygon Polygon;
    
    v2 P;
    v2 DP;
    
    f32 Angle;
    f32 ShootTimer;
} player;

typedef struct bullet
{
    v2 P;
    v2 DP;
    
    struct bullet* Next;
} bullet;

typedef struct asteroid
{
    polygon Polygon;
    
    v2 P;
    v2 DP;
    
    f32 Angle;
    f32 dAngle;
    
    u32 SplitCount;
    
    struct asteroid* Next;
} asteroid;

typedef struct
{
    u32 SampleRate;
    u32 ChannelCount;
    
    u32 SampleCount;
    s16* Samples;
} sound;

typedef struct playing_sound
{
    sound* Sound;
    f32 SamplesPlayed;
    
    f32 Volume[2];
    b32 Looped;
    
    struct playing_sound* Next;
} playing_sound;

typedef struct
{
    memory_arena PermanentArena;
    memory_arena TransientArena;
    
    sound Music;
    sound ShootSound;
    sound DeathSound;
    
    random_series Entropy;
    
    player Player;
    
    bullet* FirstBullet;
    bullet* FirstFreeBullet;
    
    asteroid* FirstAsteroid;
    asteroid* FirstFreeAsteroid;
    
    playing_sound* FirstPlayingSound;
    playing_sound* FirstFreePlayingSound;
} game_state;

#endif //ASTEROID_H
