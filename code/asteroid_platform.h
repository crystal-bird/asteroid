/* date = August 26th 2023 7:49 am */

#ifndef ASTEROID_PLATFORM_H
#define ASTEROID_PLATFORM_H

#include <intrin.h>

#define global static
#define persist static
#define internal static

#define FourCC(A, B, C, D) \
(((u32)(A) << 0 ) | \
((u32)(B) << 8 ) | \
((u32)(C) << 16) | \
((u32)(D) << 24))

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

#define Assert(Expression) do { if(!(Expression)) { *(int*)0 = 0; } } while(0)

#define InvalidCodePath Assert(false)
#define InvalidDefaultCase default: { InvalidCodePath; } break

#define Minimum(A, B) ((A) < (B) ? (A) : (B))
#define Maximum(A, B) ((A) > (B) ? (A) : (B))

#define Abs(Value) ((Value) < 0 ? -(Value) : (Value))

#define KB(Value) ((Value) * (1ll << 10))
#define MB(Value) ((Value) * (1ll << 20))
#define GB(Value) ((Value) * (1ll << 30))
#define TB(Value) ((Value) * (1ll << 40))

typedef signed char s8;
typedef signed short s16;
typedef signed int s32;
typedef signed long long s64;

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

typedef s64 smm;
typedef u64 umm;

typedef u8 b8;
typedef u32 b32;

typedef float f32;
typedef double f64;

#define true 1
#define false 0

#define Pi32 3.14159265358979323846f
#define Tau32 6.28318530717958647693f

#define U32Max (u32)(-1)

#define F32Max (3.40282346638528859812e+38f)
#define F32Min -F32Max

#endif //ASTEROID_PLATFORM_H
