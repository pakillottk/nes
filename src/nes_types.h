#pragma once
#ifndef NES_TYPES_H
#define NES_TYPES_H

#include <stdint.h>

typedef uint8_t bool8;
typedef uint8_t byte;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float real32;
typedef double real64;

#define KB(v) v * 1024
#define MB(v) KB(v) * 1024

#endif // NES_TYPES_H