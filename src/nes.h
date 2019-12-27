#pragma once
#ifndef NES_H
#define NES_H

#include "nes_types.h"

struct NESCartridge
{
    // TODO
};

struct NESMemory
{
    // TODO
};

struct CPU_6502
{
    // TODO
};

struct PPU_2C02
{
    // TODO
};

struct APU_RP2A 
{
    // TODO
};

struct NESGamepad
{
    union
    {
        byte state;
        struct
        {
            byte left : 1;
            byte up : 1;
            byte right : 1;
            byte down : 1;
            byte a : 1;
            byte b : 1;
            byte select : 1;
            byte start : 1;
        };
    } buttons;    
};

struct NES
{
    NESCartridge cartridge;
    NESMemory memory;
    CPU_6502 cpu;
    PPU_2C02 ppu;
    APU_RP2A apu;
    NESGamepad gamepad;
};

struct NESContext
{
    u32 screenWidth;
    u32 screenHeight;
    NES nes;
    u32 deltaCycles; // cycles in current frame
    u32 totalCycles; // total cycle count
};

#define NES_INIT(name) void name(NESContext *context)
typedef NES_INIT(nes_init);

#define NES_UPDATE(name) void name(NESContext *context)
typedef NES_UPDATE(nes_update);

#ifdef NES_IMPLEMENTATION
    #ifdef _WIN32
        #define EXPORT _declspec(dllexport)
    #else
        #define EXPORT
    #endif // _WIN32

    #ifdef __cplusplus
    extern "C"
    {
    #endif // __cplusplus

        EXPORT NES_INIT(NES_Init);
        EXPORT NES_UPDATE(NES_Update);

    #ifdef __cplusplus
    }
    #endif // __cplusplus
#endif // NES_IMPLEMENTATION

#endif // NES_H