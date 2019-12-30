#pragma once
#ifndef NES_H
#define NES_H

#include "nes_types.h"

enum MIRRORING
{
    kVertical,
    kHorizontal
    // TODO(pgm) More mirroring modes
};

struct NESCartridge
{
    bool8 loaded;
    MIRRORING mirror;
    u32 mapper;
    u32 romPageSize;
    u32 vromPageSize;
    u32 currentPage;
    u32 pages;
    byte *ROM;
    byte RAM[KB(8)];
    u32 currentVPage;
    u32 vpages;
    byte *VROM;
    bool8 usingVRAM;
    byte *VRAM;
    u32 vRamSize;
    void *mapperData;
};

enum CPU_REG
{
    REG_A = 0,
    REG_X = 1,
    REG_Y = 2,
};

enum P_flags 
{
    C_flag=1,
    Z_flag=2,
    I_flag=3,
    D_flag=4,
    B_flag=5,
    B2_flag=6,
    V_flag=7,
    N_flag=8
};

struct CPU_6502
{
    u32 pendingCycles;

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
            byte B : 1;
            byte B2 : 1; // unused
            byte V : 1;
            byte N : 1;        
        };
    } P;

    byte SP;

    bool8 nmi_now, nmi_processing;
};

struct PPU_2C02
{
    bool8 frameRendered;
    bool8 nmiRequested;
    bool8 latcher;
    byte dataBuffer;   

    // VRAM
    byte nameTable[2][1024];
    byte patternTable[2][4096];
    byte palette[32];

    byte fineX;
    // scanlines
    u32 cycle;
    i32 scanline;

    // PPUCTRL
    union 
    {
        byte data;
        struct
        {
            byte nametableX : 1;
            byte nametableY : 1;
            byte increment : 1;
            byte spritePattern : 1;
            byte bgPattern : 1;
            byte spriteSize : 1;
            byte masterSlave : 1; // nes doesn't use this
            byte nmi : 1;
        };
    } ppuctrl;

    // PPUMASK
    union
    {
        byte data;
        struct
        {
            byte grayscale : 1;
            byte showLeftBg : 1;
            byte showLeftSprites : 1;
            byte showBg : 1;
            byte showSprites : 1;
            byte redEmphasis : 1;
            byte greenEmphasis : 1;
            byte blueEmphasis : 1;
        };        
    } ppumask;

    // PPUSTATUS
    union 
    {
        byte data;
        struct
        {
            byte padding : 5; // unused flags
            byte overflow : 1;
            byte ZeroHit : 1;
            byte vblank : 1;
        };        
    } ppustatus;

    union loopy
    {
        u16 data;
        struct
        {
            u16 coarseX : 5;
			u16 coarseY : 5;
			u16 nametableX : 1;
			u16 nametableY : 1;
			u16 fineY : 3;
			u16 padding : 1;
        };
    };

    loopy vram_addr;
    loopy tram_addr;

    // srpites
    struct ObjectData
    {
        byte y;
        byte id;
        byte attributes;
        byte x;
    } OAM[64];
    byte oam_addr;
    // 1 to 8 sprites could be visible per scanline
    ObjectData candidateSprites[8];
    // How many sprites have been found for the scanline
    byte candidateCount;
    u16 spritePatternLo[8];
	u16 spritePatternHi[8];

    // 0 hit check
    bool8 canHaveZeroHit;
    bool8 spriteZeroRendering;

    // rendering
    byte bgTileId;
	byte bgTileAttrib;
	byte bgTileLsb;
	byte bgTileMsb;
    u16 bgPatternLo;
	u16 bgPatternHi;
	u16 bgAttribLo;
	u16 bgAttribHi;
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
            byte right : 1;
            byte left : 1;
            byte down : 1;
            byte up : 1;
            byte start : 1;
            byte select : 1;
            byte b : 1;
            byte a : 1;
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

struct Instruction 
{
    u16 offset;
    const char* label; // Ins in Assembly
    byte opcode; // hex value of the instruction
    u32 cycles; // duration in cycles
    ADDR_MODE addr_mode; // addressing mode 
    u32 operand;
    bool8 keep_pc;   // if true, pc wont be incremented
}; // Note that length in bytes is implicit by the addr_mode

struct NES
{
    NESCartridge cartridge;
    byte RAM[ KB(2) ];
    CPU_6502 cpu;
    PPU_2C02 ppu;
    APU_RP2A apu;
    NESGamepad gamepad[2];   
    byte gamepadShifter[2]; 

    // DMA for fast PPU OAM memory writting
    bool8 enableDMA;
    bool8 dmaSyncFlag;
    byte dmaPage;
    byte dmaAddr;
    byte dmaData;
};

const u32 NES_FRAMEBUFFER_WIDTH = 256;
const u32 NES_FRAMEBUFFER_HEIGHT = 240;
const u32 INSTRUCTION_BUFFER_SIZE = 50;

enum RUN_MODE
{
    kRun = 0,
    kPause,
    kStep
};

struct NESContext
{
    bool showDebugger;
    bool showPatternTables;
    u32 frameBufferTextId;
    u32 nameTableTextId[2];
    u32 patternTableTexId[2];
    u32 backbuffer[ NES_FRAMEBUFFER_WIDTH *  NES_FRAMEBUFFER_HEIGHT ];
    NES nes;
    RUN_MODE runMode;
    u32 deltaCycles; // cycles in current frame
    u32 totalCycles; // total cycle count
    
    Instruction processedInstructions[INSTRUCTION_BUFFER_SIZE];
    u32 lastInstructionCursor;
    bool8 instructionQueueOverflow;
};

#define NES_INIT(name) void name(NESContext *context, const char *romPath)
typedef NES_INIT(nes_init);

#define NES_UPDATE(name) void name(NESContext *context)
typedef NES_UPDATE(nes_update);

#define NES_SHUTDOWN(name) void name(NESContext *context)
typedef NES_SHUTDOWN(nes_shutdown);

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
        EXPORT NES_SHUTDOWN(NES_Shutdown);

    #ifdef __cplusplus
    }
    #endif // __cplusplus
#endif // NES_IMPLEMENTATION

#endif // NES_H