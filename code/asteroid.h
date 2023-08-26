/* date = August 26th 2023 7:59 am */

#ifndef ASTEROID_H
#define ASTEROID_H

typedef struct
{
    void* PermanentStorage;
    umm PermanentStorageSize;
    
    void* TransientStorage;
    umm TransientStorageSize;
    
    b32 IsInitialized;
} game_memory;

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

typedef struct
{
    void* Base;
    umm Used;
    umm Size;
} memory_arena;

internal memory_arena InitMemoryArena(void* Base, umm Size)
{
    memory_arena Result = {0};
    Result.Base = Base;
    Result.Used = 0;
    Result.Size = Size;
    return (Result);
}

#define PushVariable(Arena, Type) (Type*)PushSize(Arena, sizeof(Type))

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
    memory_arena PermanentArena;
    memory_arena TransientArena;
    
    random_series Entropy;
    
    player Player;
    
    bullet* FirstBullet;
    bullet* FirstFreeBullet;
    
    asteroid* FirstAsteroid;
    asteroid* FirstFreeAsteroid;
} game_state;

#endif //ASTEROID_H
