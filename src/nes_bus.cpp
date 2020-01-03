#include "nes_bus.h"
#include "nes_ppu.h"
#include "nes_apu.h"
#include "nes_mappers.h"
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
    else if( (addr >= 0x4000 && addr <= 0x4013) || addr == 0x4015 || addr == 0x4017 ) 
    { 
        // APU I/O Space
        if( set )        
        {
            return WriteAPU(nes, addr, v);
        }
        else
        {
            return ReadAPU(nes, addr);
        }    
    } 
    else if( addr == 0x4014 )
    {
        if( set )
        {
            nes->dmaPage = v;
            nes->dmaAddr = 0x0;
            nes->enableDMA = true;
        }

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
    else if( addr >= 0x6000 && addr <= 0x7FFF )
    {
        // Cartridge RAM
        chunk = &nes->cartridge.RAM[ addr & (KB(8)-1) ];
    }
    else if(addr >= 0x8000 && addr <= 0xFFFF) 
    { 
        // Cartridge space   
        inCart = true;
        if(set)
        {
            return WriteMapper(&nes->cartridge, addr, v);
        }
        else
        {
            return ReadMapper(&nes->cartridge, addr);
        }
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