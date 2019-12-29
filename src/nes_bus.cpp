#include "nes_bus.h"
#include "nes_ppu.h"
#include <assert.h>

#define internal static

internal byte
MemAccess(NES *nes, u16 addr, bool8 set = false, byte v = 0)
{
    byte* chunk;
    bool8 inCart = false;
    if( addr >= 0 && addr <= 0x1FFF ) 
    {    
        // All RAM space, with mirroring
        chunk = &nes->RAM[ (addr%0x800) ];
    } 
    else if( addr >= 0x2000 && addr <= 0x3FFF ) 
    { 
        // PPU SPACE 
        if( set )
        {
            return WritePPU(nes, addr, v);
        }
        else
        {            
            return ReadPPU(nes, addr);
        }
    } 
    else if( addr >= 0x4000 && addr <= 0x4015 ) 
    { 
        // APU I/O Space

        //TODO(pgm) for now return 0        
        return 0;
    } 
    else if( addr >= 0x4016 && addr <= 0x4017 )
    {
        // gamepads
        if( set )
        {
            return nes->gamepadShifter[ addr & 0x0001 ] = nes->gamepad[ addr & 0x0001 ].buttons.state;        
        }
        else
        {
            byte data = (nes->gamepadShifter[ addr & 0x0001 ] & 0x80) > 0;
            nes->gamepadShifter[ addr & 0x0001 ] <<= 1;
            return data;
        }
    }
    else if(addr >= 0x8000 && addr <= 0xFFFF) 
    { 
        // Cartridge space   
        inCart = true;

        //TODO(pgm) RAM
        u16 mapAddr = addr & (nes->cartridge.pages > 1 ? 0x7FFF : 0x3FFF);            
        // u16 mapAddr = addr % ROM_PAGESIZE;
        return nes->cartridge.ROM[mapAddr];
    }
    else
    {
        return(0);
    }
    if( set ) 
    {
        assert(!inCart);
        (*chunk) = v;
    }
    
    return *chunk;
}

byte
RB(NES *nes, u16 addr)
{
    return MemAccess(nes, addr, false, 0);
}

byte
WB(NES *nes, u16 addr, u8 v)
{
    return MemAccess(nes, addr, true, v);
}