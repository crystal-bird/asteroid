
internal v4 Linear1ToSRGB255(v4 Color)
{
    f32 One255 = 255.0f;
    
    v4 Result;
    Result.R = One255 * SquareRoot(Color.R);
    Result.G = One255 * SquareRoot(Color.G);
    Result.B = One255 * SquareRoot(Color.B);
    Result.A = One255 * Color.A;
    
    return (Result);
}

internal u32 Pack4x8ARGB(v4 Color)
{
    u32 Result = ((RoundF32ToU32(Color.A) << 24) |
                  (RoundF32ToU32(Color.R) << 16) |
                  (RoundF32ToU32(Color.G) << 8 ) |
                  (RoundF32ToU32(Color.B) << 0 ));
    
    return (Result);
}

internal void ClearBackbuffer(game_backbuffer* Backbuffer, v4 Color)
{
    u32 ClearColor = Pack4x8ARGB(Linear1ToSRGB255(Color));
    
    u32* Row = (u32*)Backbuffer->Memory;
    for (s32 Y = 0; Y < Backbuffer->Height; Y++)
    {
        u32* Pixel = Row;
        
        for (s32 X = 0; X < Backbuffer->Width; X++)
        {
            *Pixel++ = ClearColor;
        }
        
        Row += Backbuffer->Pitch;
    }
}

internal void PlotPixel(game_backbuffer* Backbuffer, s32 X, s32 Y, u32 Color)
{
    if ((X < 0) || (X >= Backbuffer->Width) ||
        (Y < 0) || (Y >= Backbuffer->Height))
    {
        return;
    }
    
    u32* Pixel = (u32*)Backbuffer->Memory + (Y * Backbuffer->Pitch) + X;
    *Pixel = Color;
}

internal v2 ProjectToBackbuffer(game_backbuffer* Backbuffer, v2 Position)
{
    v2 Result = Position;
    
    f32 FieldOfView = 90.0f * Pi32 / 180.0f;
    f32 AspectRatio = (f32)Backbuffer->Width / (f32)Backbuffer->Height;
    f32 FocalLength = 0.05f;
    
    f32 ScaleX = FocalLength / (tanf(FieldOfView * 0.5f) * AspectRatio);
    f32 ScaleY = FocalLength / (tanf(FieldOfView * 0.5f) * 1.0f);
    
    Result.X = (Result.X*ScaleX + 1.0f) * Backbuffer->Width*0.5f;
    Result.Y = (Result.Y*ScaleY + 1.0f) * Backbuffer->Height*0.5f;
    
    return (Result);
}

internal void DrawLine(game_backbuffer* Backbuffer,
                       v2 A, v2 B, v4 Color)
{
    A = ProjectToBackbuffer(Backbuffer, A);
    B = ProjectToBackbuffer(Backbuffer, B);
    
    s32 X0 = RoundF32ToS32(A.X);
    s32 Y0 = RoundF32ToS32(A.Y);
    s32 X1 = RoundF32ToS32(B.X);
    s32 Y1 = RoundF32ToS32(B.Y);
    
    s32 Sx = (X0 < X1) ? 1 : -1;
    s32 Sy = (Y0 < Y1) ? 1 : -1;
    
    s32 Dx = Abs(X1 - X0);
    s32 Dy = Abs(Y1 - Y0);
    
    s32 X = X0;
    s32 Y = Y0;
    
    u32 PlotColor = Pack4x8ARGB(Linear1ToSRGB255(Color));
    s32 Error = 0;
    
    for (;;)
    {
        PlotPixel(Backbuffer, X, Y, PlotColor);
        
        if ((X == X1) && (Y == Y1))
        {
            break;
        }
        
        if (Error < Dx)
        {
            X += Sx;
            Error += Dy;
        }
        
        if (Error >= Dx)
        {
            Y += Sy;
            Error -= Dx;
        }
    }
}

internal void AddPoint(polygon* Polygon, v2 Point)
{
    if (Polygon->PointCount < ArrayCount(Polygon->Points))
    {
        Polygon->Points[Polygon->PointCount++] = Point;
    }
    else
    {
        InvalidCodePath;
    }
}

internal void TranslatePolygon(polygon* Polygon, v2 Translation)
{
    for (u32 PointIndex = 0;
         PointIndex < Polygon->PointCount;
         PointIndex++)
    {
        v2 P = Polygon->Points[PointIndex];
        
        P.X += Translation.X;
        P.Y += Translation.Y;
        
        Polygon->Points[PointIndex] = P;
    }
}

internal void RotatePolygon(polygon* Polygon, f32 Angle)
{
    f32 S = sinf(Angle);
    f32 C = cosf(Angle);
    
    for (u32 PointIndex = 0;
         PointIndex < Polygon->PointCount;
         PointIndex++)
    {
        v2 P = Polygon->Points[PointIndex];
        
        v2 RotatedP = P;
        RotatedP.X = C*P.X - S*P.Y;
        RotatedP.Y = S*P.X + C*P.Y;
        
        Polygon->Points[PointIndex] = RotatedP;
    }
}

internal void DrawPolygon(game_backbuffer* Backbuffer, polygon* Polygon, v4 Color)
{
    for (u32 PointIndex = 0;
         PointIndex < Polygon->PointCount;
         PointIndex++)
    {
        u32 NextPointIndex = (PointIndex + 1) % Polygon->PointCount;
        
        v2 A = Polygon->Points[PointIndex];
        v2 B = Polygon->Points[NextPointIndex];
        
        DrawLine(Backbuffer, A, B, Color);
    }
}

internal void DrawRectangle(game_backbuffer* Backbuffer, v2 Min, v2 Max, v4 Color)
{
    Min = ProjectToBackbuffer(Backbuffer, Min);
    Max = ProjectToBackbuffer(Backbuffer, Max);
    
    s32 MinX = RoundF32ToS32(Min.X);
    s32 MinY = RoundF32ToS32(Min.Y);
    s32 MaxX = RoundF32ToS32(Max.X);
    s32 MaxY = RoundF32ToS32(Max.Y);
    
    MinX = Maximum(MinX, 0);
    MinY = Maximum(MinY, 0);
    MaxX = Minimum(MaxX, Backbuffer->Width);
    MaxY = Minimum(MaxY, Backbuffer->Height);
    
    u32 FillColor = Pack4x8ARGB(Linear1ToSRGB255(Color));
    
    u32* Row = (u32*)Backbuffer->Memory + (MinY * Backbuffer->Pitch) + MinX;
    for (s32 Y = MinY; Y < MaxY; Y++)
    {
        u32* Pixel = Row;
        
        for (s32 X = MinX; X < MaxX; X++)
        {
            *Pixel++ = FillColor;
        }
        
        Row += Backbuffer->Width;
    }
}

internal b32 PointInsidePolygon(polygon* Polygon, v2 Point)
{
    b32 Inside = false;
    
    f32 MinX = F32Max;
    f32 MinY = F32Max;
    f32 MaxX = F32Min;
    f32 MaxY = F32Min;
    
    for (u32 PointIndex = 0;
         PointIndex < Polygon->PointCount;
         PointIndex++)
    {
        v2 P = Polygon->Points[PointIndex];
        
        MinX = Minimum(MinX, P.X);
        MinY = Minimum(MinY, P.Y);
        MaxX = Maximum(MaxX, P.X);
        MaxY = Maximum(MaxY, P.Y);
    }
    
    if ((Point.X >= MinX) && (Point.Y >= MinY) &&
        (Point.X <= MaxX) && (Point.Y <= MaxY))
    {
        for (u32 PointIndex = 0;
             PointIndex < Polygon->PointCount;
             PointIndex++)
        {
            u32 NextPointIndex = (PointIndex + 1) % Polygon->PointCount;
            
            v2 A = Polygon->Points[PointIndex];
            v2 B = Polygon->Points[NextPointIndex];
            
            if ((A.Y > Point.Y) != (B.Y > Point.Y))
            {
                f32 T = (Point.Y - A.Y) / (B.Y - A.Y);
                f32 IntersectX = A.X + (B.X - A.X)*T;
                
                if (Point.X < IntersectX)
                {
                    Inside = !Inside;
                }
            }
        }
    }
    
    return (Inside);
}

internal b32 PolygonIntersects(polygon* PolygonA, polygon* PolygonB)
{
    b32 Intersects = false;
    
    for (u32 PointIndex = 0;
         PointIndex < PolygonA->PointCount;
         PointIndex++)
    {
        v2 A = PolygonA->Points[PointIndex];
        if (PointInsidePolygon(PolygonB, A))
        {
            Intersects = true;
            break;
        }
    }
    
    return (Intersects);
}

internal v2 WrapPosition(v2 Position, v2 ArenaDim)
{
    v2 HalfArenaDim = V2MulScalar(ArenaDim, 0.5f);
    
    v2 Result = Position;
    
    if (Result.X <= -HalfArenaDim.X)
    {
        Result.X += ArenaDim.X;
    }
    else if (Result.X >= HalfArenaDim.X)
    {
        Result.X -= ArenaDim.X;
    }
    
    if (Result.Y <= -HalfArenaDim.Y)
    {
        Result.Y += ArenaDim.Y;
    }
    else if (Result.Y >= HalfArenaDim.Y)
    {
        Result.Y -= ArenaDim.Y;
    }
    
    return (Result);
}

internal asteroid* NewAsteroid(game_state* GameState, v2 P, s32 SplitCount)
{
    if (!GameState->FirstFreeAsteroid)
    {
        GameState->FirstFreeAsteroid = PushVariable(&GameState->PermanentArena, asteroid);
    }
    
    asteroid* Asteroid = GameState->FirstFreeAsteroid;
    GameState->FirstFreeAsteroid = Asteroid->Next;
    
    Asteroid->Polygon.PointCount = 0;
    Asteroid->P = P;
    Asteroid->DP = V2((SplitCount+1)*3.0f*RandomBilateral(&GameState->Entropy),
                      (SplitCount+1)*3.0f*RandomBilateral(&GameState->Entropy));
    
    Asteroid->Angle = 0.0f;
    Asteroid->dAngle = 0.5f + 0.25f*Pi32*RandomBilateral(&GameState->Entropy);
    
    Asteroid->SplitCount = SplitCount;
    
    u32 PointCount = RandomRangeU32(&GameState->Entropy, 7, 13);
    
    f32 Angle = 0.0f;
    for (u32 PointIndex = 0;
         PointIndex < PointCount;
         PointIndex++)
    {
        f32 MinRadius = 3.0f / (SplitCount + 1);
        
        f32 Radius = MinRadius + 1.0f*RandomUnilateral(&GameState->Entropy);
        
        v2 Point = V2(Radius * cosf(Angle),
                      Radius * sinf(Angle));
        
        AddPoint(&Asteroid->Polygon, Point);
        
        Angle += (2.5f + 0.25f*RandomUnilateral(&GameState->Entropy) * Tau32) / PointCount*2.0f;
    }
    
    Asteroid->Next = GameState->FirstAsteroid;
    GameState->FirstAsteroid = Asteroid;
    
    return (Asteroid);
}

internal void DestroyAsteroid(game_state* GameState, asteroid** AsteroidPtr)
{
    asteroid* Asteroid = *AsteroidPtr;
    *AsteroidPtr = Asteroid->Next;
    
    Asteroid->Next = GameState->FirstFreeAsteroid;
    GameState->FirstFreeAsteroid = Asteroid;
}

internal bullet* NewBullet(game_state* GameState, v2 P, v2 DP)
{
    if (!GameState->FirstFreeBullet)
    {
        GameState->FirstFreeBullet = PushVariable(&GameState->PermanentArena, bullet);
    }
    
    bullet* Bullet = GameState->FirstFreeBullet;
    GameState->FirstFreeBullet = Bullet->Next;
    
    Bullet->P = P;
    Bullet->DP = DP;
    
    Bullet->Next = GameState->FirstBullet;
    GameState->FirstBullet = Bullet;
    
    return (Bullet);
}

internal void DestroyBullet(game_state* GameState, bullet** BulletPtr)
{
    bullet* Bullet = *BulletPtr;
    *BulletPtr = Bullet->Next;
    
    Bullet->Next = GameState->FirstFreeBullet;
    GameState->FirstFreeBullet = Bullet;
}

internal void ResetGame(game_state* GameState)
{
    player* Player = &GameState->Player;
    
    Player->P = V2(0, 0);
    Player->DP = V2(0, 0);
    Player->Angle = 0.0f;
    
    Player->Polygon.PointCount = 0;
    AddPoint(&Player->Polygon, V2(-0.75f, -1.00f));
    AddPoint(&Player->Polygon, V2( 0.00f, -0.75f));
    AddPoint(&Player->Polygon, V2( 0.75f, -1.00f));
    AddPoint(&Player->Polygon, V2( 0.00f,  1.00f));
    
    for (asteroid** AsteroidPtr = &GameState->FirstAsteroid;
         *AsteroidPtr;)
    {
        DestroyAsteroid(GameState, AsteroidPtr);
    }
    
    u32 AsteroidCount = 6;
    
    for (u32 AsteroidIndex = 0;
         AsteroidIndex < AsteroidCount;
         AsteroidIndex++)
    {
        f32 SpawnRadius = 20.0f + 8.0f*RandomUnilateral(&GameState->Entropy);
        f32 Angle = Tau32*RandomBilateral(&GameState->Entropy);
        
        v2 SpawnP = V2(SpawnRadius * cosf(Angle),
                       SpawnRadius * sinf(Angle));
        
        NewAsteroid(GameState, SpawnP, 0);
    }
}

void GameUpdateAndRender(game_memory* Memory, game_input* Input, game_backbuffer* Backbuffer)
{
    game_state* GameState = (game_state*)Memory->PermanentStorage;
    
    if (!Memory->IsInitialized)
    {
        GameState->PermanentArena = InitMemoryArena((u8*)Memory->PermanentStorage + sizeof(game_state),
                                                    Memory->PermanentStorageSize - sizeof(game_state));
        
        GameState->TransientArena = InitMemoryArena(Memory->TransientStorage,
                                                    Memory->TransientStorageSize);
        
        GameState->Entropy.State = RandomSeed();
        
        ResetGame(GameState);
        
        Memory->IsInitialized = true;
    }
    
    v2 ArenaDim = V2(87.5f, 47.5f);
    v2 HalfArenaDim = V2MulScalar(ArenaDim, 0.5f);
    
    ClearBackbuffer(Backbuffer, V4(0.005f, 0.005f, 0.0065f, 1.0f));
    
    {
        player* Player = &GameState->Player;
        
        f32 dAngle = 1.5f*Pi32;
        if (ButtonDown(&Input->TurnLeft))
        {
            Player->Angle += dAngle * Input->DeltaTime;
        }
        if (ButtonDown(&Input->TurnRight))
        {
            Player->Angle -= dAngle * Input->DeltaTime;
        }
        
        if (Player->Angle <= 0.0f)
        {
            Player->Angle += Tau32;
        }
        else if (Player->Angle >= Tau32)
        {
            Player->Angle -= Tau32;
        }
        
        if (ButtonDown(&Input->Thrust))
        {
            f32 Force = 10.0f;
            v2 PlayerDDP = V2(Force * -sinf(Player->Angle),
                              Force *  cosf(Player->Angle));
            
            Player->DP = V2Add(Player->DP, V2MulScalar(PlayerDDP, Input->DeltaTime));
        }
        
        Player->ShootTimer -= Input->DeltaTime;
        
        if ((ButtonReleased(&Input->Shoot)) &&
            (Player->ShootTimer <= 0.0f))
        {
            f32 Velocity = 40.0f;
            
            v2 DP = V2(Velocity * -sinf(Player->Angle),
                       Velocity *  cosf(Player->Angle));
            
            v2 P = V2(Player->P.X + 1.0f * -sinf(Player->Angle),
                      Player->P.Y + 1.0f *  cosf(Player->Angle));
            
            NewBullet(GameState, P, DP);
            
            Player->ShootTimer = 0.25f;
        }
        
        Player->P = V2Add(Player->P, V2MulScalar(Player->DP, Input->DeltaTime));
        Player->P = WrapPosition(Player->P, ArenaDim);
        
        polygon PlayerPolygon = Player->Polygon;
        
        RotatePolygon(&PlayerPolygon, Player->Angle);
        TranslatePolygon(&PlayerPolygon, Player->P);
        
        DrawPolygon(Backbuffer, &PlayerPolygon, V4(1.0f, 0.3f, 0.2f, 1.0f));
        
        for (asteroid* Asteroid = GameState->FirstAsteroid;
             Asteroid;
             Asteroid = Asteroid->Next)
        {
            polygon AsteroidPolygon = Asteroid->Polygon;
            RotatePolygon(&AsteroidPolygon, Asteroid->Angle);
            TranslatePolygon(&AsteroidPolygon, Asteroid->P);
            
            if (PolygonIntersects(&PlayerPolygon, &AsteroidPolygon))
            {
                ResetGame(GameState);
                break;
            }
        }
    }
    
    if (!GameState->FirstAsteroid)
    {
        ResetGame(GameState);
    }
    
    for (bullet** BulletPtr = &GameState->FirstBullet;
         *BulletPtr;)
    {
        bullet* Bullet = *BulletPtr;
        
        Bullet->P = V2Add(Bullet->P, V2MulScalar(Bullet->DP, Input->DeltaTime));
        
        f32 Thickness = 0.2f;
        f32 HalfThickness = Thickness*0.5f;
        
        v2 BulletMin = V2SubScalar(Bullet->P, HalfThickness);
        v2 BulletMax = V2AddScalar(Bullet->P, HalfThickness);
        
        DrawRectangle(Backbuffer, BulletMin, BulletMax, V4(0.9f, 0.9f, 0.9f, 1.0f));
        
        b32 DestroytedAsteroid = false;
        
        for (asteroid** AsteroidPtr = &GameState->FirstAsteroid;
             *AsteroidPtr;)
        {
            asteroid* Asteroid = *AsteroidPtr;
            
            polygon TestPolygon = Asteroid->Polygon;
            RotatePolygon(&TestPolygon, Asteroid->Angle);
            TranslatePolygon(&TestPolygon, Asteroid->P);
            
            if (PointInsidePolygon(&TestPolygon, Bullet->P))
            {
                u32 MaxAsteroidSplitCount = 3;
                
                if (Asteroid->SplitCount != MaxAsteroidSplitCount)
                {
                    NewAsteroid(GameState, Asteroid->P, Asteroid->SplitCount + 1);
                    NewAsteroid(GameState, Asteroid->P, Asteroid->SplitCount + 1);
                }
                
                DestroyAsteroid(GameState, AsteroidPtr);
                
                DestroytedAsteroid = true;
                break;
            }
            else
            {
                AsteroidPtr = &Asteroid->Next;
            }
        }
        
        if (DestroytedAsteroid)
        {
            DestroyBullet(GameState, BulletPtr);
        }
        else if ((Bullet->P.X < -HalfArenaDim.X) ||
                 (Bullet->P.Y < -HalfArenaDim.Y))
        {
            DestroyBullet(GameState, BulletPtr);
        }
        else if ((Bullet->P.X > HalfArenaDim.X) ||
                 (Bullet->P.Y > HalfArenaDim.Y))
        {
            DestroyBullet(GameState, BulletPtr);
        }
        else
        {
            BulletPtr = &Bullet->Next;
        }
    }
    
    for (asteroid* Asteroid = GameState->FirstAsteroid;
         Asteroid;
         Asteroid = Asteroid->Next)
    {
        Asteroid->P = V2Add(Asteroid->P, V2MulScalar(Asteroid->DP, Input->DeltaTime));
        Asteroid->Angle += Asteroid->dAngle*Input->DeltaTime;
        
        if (Asteroid->Angle <= 0.0f)
        {
            Asteroid->Angle += Tau32;
        }
        else if (Asteroid->Angle >= Tau32)
        {
            Asteroid->Angle -= Tau32;
        }
        
        Asteroid->P = WrapPosition(Asteroid->P, ArenaDim);
        
        polygon Polygon = Asteroid->Polygon;
        
        RotatePolygon(&Polygon, Asteroid->Angle);
        TranslatePolygon(&Polygon, Asteroid->P);
        
        DrawPolygon(Backbuffer, &Polygon, V4(0.3f, 0.4f, 1.0f, 1.0f));
    }
}
