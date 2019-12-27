#pragma once
#ifndef NES_H
#define NES_H

#include "nes_types.h"

static const u32 ROM_PAGESIZE = 16384;
static const u32 VROM_PAGESIZE = 8192;

struct NESCartridge
{
    bool8 loaded;
    u32 mapper;
    u32 pages;
    byte *ROM;
    u32 vpages;
    byte *VROM;
};

enum CPU_REG
{
    REG_A = 0,
    REG_X = 1,
    REG_Y = 2,
};

struct CPU_6502
{
    u16 PC;
    union 
    {
        byte data[3];
        struct
        {
            byte A;
            byte X;
            byte Y;
        };
    } regs;
    
    union
    {
        byte flags;
        struct
        {
            byte C : 1;
            byte Z : 1;
            byte I : 1;
            byte D : 1;
            byte V : 1;
            byte N : 1;        
        };
    } P;

    byte SP;

    bool8 nmi_now, nmi_processing;
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

enum ADDR_MODE {    
    ACCUMULATOR = 0,
    ABS = 1,
    ABS_X = 2,
    ABS_Y = 3,
    IMMEDIATE = 4, //#
    IMPL = 5, //implied
    IND = 6, //indirect
    X_IND = 7, //x-indexed indirect
    IND_Y = 8, //indirect y-indexed
    REL = 9, //relative
    ZPG = 10, //zero page
    ZPG_X = 11,
    ZPG_Y = 12
};

static const unsigned int addr_mode_length[] = {
//ACCUM  //ABS   //ABS_X    //ABS_Y
    1,      3,      3,      3,
//#       //impl  //ind    //X_IND
    2,      1,      3,      2,
//IND_Y   //rel  //zpg    //zpg_x
    2,      2,      2,      2,
//zpg_y
    2
};

enum P_flags 
{
    C_flag=1,
    Z_flag=2,
    I_flag=3,
    D_flag=4,
    V_flag=7,
    N_flag=8
};

struct Instruction 
{
    const char* label; // Ins in Assembly
    byte opcode; // hex value of the instruction
    u32 cycles; // duration in cycles
    ADDR_MODE addr_mode; // addressing mode 
    bool8 keep_pc;   // if true, pc wont be incremented
}; // Note that length in bytes is implicit by the addr_mode

struct NES
{
    NESCartridge cartridge;
    byte RAM[ KB(2) ];
    CPU_6502 cpu;
    PPU_2C02 ppu;
    APU_RP2A apu;
    NESGamepad gamepad;    
};

const u32 NES_FRAMEBUFFER_WIDTH = 256;
const u32 NES_FRAMEBUFFER_HEIGHT = 240;
const u32 INSTRUCTION_BUFFER_SIZE = 50;

enum RUN_MODE
{
    kPause = 0,
    kRun,
    kStep
};

struct NESContext
{
    u32 frameBufferTextId;
    u32 backbuffer[ NES_FRAMEBUFFER_WIDTH *  NES_FRAMEBUFFER_HEIGHT ];
    NES nes;
    RUN_MODE runMode;
    u32 deltaCycles; // cycles in current frame
    u32 totalCycles; // total cycle count
    
    Instruction processedInstructions[INSTRUCTION_BUFFER_SIZE];
    u32 lastInstructionCursor;
    bool8 instructionQueueOverflow;
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