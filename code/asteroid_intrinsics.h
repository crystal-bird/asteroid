/* date = August 26th 2023 8:46 am */

#ifndef ASTEROID_INTRINSICS_H
#define ASTEROID_INTRINSICS_H

internal f32 Square(f32 Value)
{
    f32 Result = (Value * Value);
    return (Result);
}

internal f32 SquareRoot(f32 Value)
{
    f32 Result = _mm_cvtss_f32(_mm_sqrt_ss(_mm_set_ss(Value)));
    return (Result);
}

internal f32 InvSquareRoot(f32 Value)
{
    f32 Result = _mm_cvtss_f32(_mm_rsqrt_ss(_mm_set_ss(Value)));
    return (Result);
}

internal u32 RotateLeft32(u32 Value, u32 Shift)
{
    u32 Result = _rotl(Value, Shift);
    return (Result);
}

internal u32 RotateRight32(u32 Value, u32 Shift)
{
    u32 Result = _rotr(Value, Shift);
    return (Result);
}

internal s32 RoundF32ToS32(f32 Value)
{
    s32 Result = _mm_cvtss_si32(_mm_round_ss(_mm_setzero_ps(), _mm_set_ss(Value),
                                             _MM_FROUND_TO_NEAREST_INT|_MM_FROUND_NO_EXC));
    
    return (Result);
}

internal u32 RoundF32ToU32(f32 Value)
{
    u32 Result = _mm_cvtss_si32(_mm_round_ss(_mm_setzero_ps(), _mm_set_ss(Value),
                                             _MM_FROUND_TO_NEAREST_INT|_MM_FROUND_NO_EXC));
    
    return (Result);
}

typedef struct
{
    u64 State;
} random_series;

internal u64 RandomSeed(void)
{
    u64 Result = 0;
    
    while (_rdseed64_step(&Result) == 0)
    {
    }
    
    return (Result);
}

internal u32 RandomU32(random_series* Entropy)
{
    u64 X = Entropy->State;
    u32 Count = (u32)(X >> 59);
    
    X = X*288230376151711745ull + 93824992236885ull;
    X ^= X << 15;
    
    Entropy->State = X;
    u32 Result = RotateRight32((u32)(X >> 7), Count);
    
    return (Result);
}

internal u32 RandomRangeU32(random_series* Entropy, u32 From, u32 To)
{
    u32 Result = From + (RandomU32(Entropy) % (To - From));
    return (Result);
}

internal f32 RandomUnilateral(random_series* Entropy)
{
    f32 Result = RandomU32(Entropy) / (f32)(U32Max);
    return (Result);
}

internal f32 RandomBilateral(random_series* Entropy)
{
    f32 Result = -1.0f + 2.0f*RandomUnilateral(Entropy);
    return (Result);
}

#endif //ASTEROID_INTRINSICS_H
