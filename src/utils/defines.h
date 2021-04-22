#pragma once
#ifndef DESHI_DEFINES_H
#define DESHI_DEFINES_H

#include "Tracy.hpp"

//math constants
#define M_PI         3.14159265359f
#define M_E          2.71828182846f
#define M_TWOTHIRDS  0.66666666666f
#define M_ONETWELFTH 0.08333333333f

//conversions
#define RADIANS(x) ((x) * (M_PI / 180.f))
#define DEGREES(x) ((x) * (180.f / M_PI))

//number typedefs
typedef signed char    i8;     typedef i8  int8;
typedef signed short   i16;    typedef i16 int16;
typedef signed int     i32;    typedef i32 int32;
typedef signed long    i64;    typedef i64 int64;
typedef unsigned char  u8;     typedef u8  uint8;
typedef unsigned short u16;    typedef u16 uint16;
typedef unsigned int   u32;    typedef u32 uint32;
typedef unsigned long  u64;    typedef u64 uint64;
typedef float          f32;    typedef f32 float32;
typedef double         f64;    typedef f64 float64;
typedef i32 b32; //int based boolean so c++ doesnt convert to 0 or 1

//static defines
#define static_internal static
#define local_persist   static
#define global_variable static

//delle's annoyance with c++ cast syntax; which will probably cause problems somewhere
#define i8(x)     static_cast<i8>(x)
#define i16(x)    static_cast<i16>(x)
#define i32(x)    static_cast<i32>(x)
#define i64(x)    static_cast<i64>(x)
#define u8(x)     static_cast<u8>(x)
#define u16(x)    static_cast<u16>(x)
#define u32(x)    static_cast<u32>(x)
#define u64(x)    static_cast<u64>(x)
#define f32(x)    static_cast<f32>(x)
#define f64(x)    static_cast<f64>(x)
#define int8(x)   static_cast<int8>(x)
#define int16(x)  static_cast<int16>(x)
#define int32(x)  static_cast<int32>(x)
#define int64(x)  static_cast<int64>(x)
#define uint8(x)  static_cast<uint8>(x)
#define uint16(x) static_cast<uint16>(x)
#define uint32(x) static_cast<uint32>(x)
#define uint64(x) static_cast<uint64>(x)

//sushi's annoyance with c++ cast syntax, which WILL NOT cause problems ANYWHERE
#define dyncast(name, child, base) child* name = dynamic_cast<child*>(base)

//i: variable name; x: number of iterations 
#define for_n(i,x) for(int i=0; i<x; ++i)

//dst: destination c-string; src: source c-string; bytes: number of characters to copy
#define cpystr(dst, src, bytes) strncpy_s(dst, src, bytes); dst[bytes] = '\0'

//compile-time print sizeof()
//char (*__kaboom)[sizeof( YourTypeHere )] = 1;

#endif //DESHI_DEFINES_H
