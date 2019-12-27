#include "nes_bus.h"
#include "nes_ppu.h"
#include <assert.h>

#define internal static

internal byte*
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
    else if( addr >= 0x4000 && addr <= 0x4018 ) 
    { 
        // APU I/O Space

        //TODO(pgm) for now return 0        
        return 0;
    } 
    else 
    { 
        // Cartridge space        
        // 0x6000 - 0x7FFF
        inCart = true;

        //TODO(pgm) RAM
        u16 mapAddr = addr;
        if( addr >= 0x8000 && addr <= 0xBFFF ) 
        {
            // First ROM bank    
            mapAddr = (addr%16384);
        } 
        else if(addr >= 0xC000 && addr <= 0xFFFF) 
        { 
            // Second ROM banks
            if( nes->cartridge.pages < 16 ) 
            {
                //mirror
                mapAddr = (addr%16384);
            } 
            else 
            {
                mapAddr = 16384 + (addr%16384); //Skip first bank
            }        
        } 

        return &nes->cartridge.ROM[mapAddr];
    }
    if( set ) 
    {
        assert(!inCart);
        (*chunk) = v;
    }
    
    return chunk;
}

byte*
RB(NES *nes, u16 addr)
{
    return MemAccess(nes, addr, false, 0);
}

byte*
WB(NES *nes, u16 addr, u8 v)
{
    return MemAccess(nes, addr, true, v);
}