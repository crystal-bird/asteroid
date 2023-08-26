/* date = August 26th 2023 9:02 am */

#ifndef ASTEROID_MATH_H
#define ASTEROID_MATH_H

typedef union
{
    struct { f32 X, Y; };
    struct { f32 R, G; };
    struct { f32 E[2]; };
} v2;

typedef union
{
    struct { f32 X, Y, Z; };
    struct { f32 R, G, B; };
    struct { f32 E[3]; };
} v3;

typedef union
{
    struct { f32 X, Y, Z, W; };
    struct { f32 R, G, B, A; };
    struct { f32 E[4]; };
} v4;

#define V2(X, Y) (v2){X, Y}
#define V3(X, Y, Z) (v3){X, Y, Z}
#define V4(X, Y, Z, W) (v4){X, Y, Z, W}

#define VectorFunctions(Type, Dim) \
internal Type V##Dim##Add(Type A, Type B) \
{ \
Type Result; \
for (u32 I = 0; I < Dim; I++) \
Result.E[I] = A.E[I] + B.E[I]; \
return (Result); \
} \
internal Type V##Dim##AddScalar(Type A, f32 B) \
{ \
Type Result; \
for (u32 I = 0; I < Dim; I++) \
Result.E[I] = A.E[I] + B; \
return (Result); \
} \
internal Type V##Dim##Sub(Type A, Type B) \
{ \
Type Result; \
for (u32 I = 0; I < Dim; I++) \
Result.E[I] = A.E[I] - B.E[I]; \
return (Result); \
} \
internal Type V##Dim##SubScalar(Type A, f32 B) \
{ \
Type Result; \
for (u32 I = 0; I < Dim; I++) \
Result.E[I] = A.E[I] - B; \
return (Result); \
} \
internal Type V##Dim##Mul(Type A, Type B) \
{ \
Type Result; \
for (u32 I = 0; I < Dim; I++) \
Result.E[I] = A.E[I] * B.E[I]; \
return (Result); \
} \
internal Type V##Dim##MulScalar(Type A, f32 B) \
{ \
Type Result; \
for (u32 I = 0; I < Dim; I++) \
Result.E[I] = A.E[I] * B; \
return (Result); \
} \
internal Type V##Dim##Div(Type A, Type B) \
{ \
Type Result; \
for (u32 I = 0; I < Dim; I++) \
Result.E[I] = A.E[I] / B.E[I]; \
return (Result); \
} \
internal Type V##Dim##DivScalar(Type A, f32 B) \
{ \
Type Result; \
for (u32 I = 0; I < Dim; I++) \
Result.E[I] = A.E[I] / B; \
return (Result); \
} \
internal f32 V##Dim##Dot(Type A, Type B) \
{ \
f32 Result = 0.0f; \
for (u32 I = 0; I < Dim; I++) \
Result += A.E[I] * B.E[I]; \
return (Result); \
} \
internal f32 V##Dim##LengthSq(Type A) \
{ \
f32 Result = V##Dim##Dot(A, A); \
return (Result); \
} \
internal f32 V##Dim##Length(Type A) \
{ \
f32 Result = SquareRoot(V##Dim##Dot(A, A)); \
return (Result); \
} \
internal f32 V##Dim##InvLength(Type A) \
{ \
f32 Result = InvSquareRoot(V##Dim##Dot(A, A)); \
return (Result); \
} \
internal Type V##Dim##Normalize(Type A) \
{ \
Type Result = V##Dim##MulScalar(A, V##Dim##InvLength(A)); \
return (Result); \
} \
internal Type V##Dim##NormalizeOrZero(Type A) \
{ \
Type Result = {0}; \
if (V##Dim##LengthSq(A) < 1e-7f) \
Result = V##Dim##Normalize(A); \
return (Result); \
}

VectorFunctions(v2, 2);
VectorFunctions(v3, 3);
VectorFunctions(v4, 4);

#endif //ASTEROID_MATH_H
