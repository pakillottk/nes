#include "../nes_mappers.h"

LOAD_MAPPER(2, KB(16), KB(8), {})
WRITE_MAPPER(2)
{
    // change the current page
    cartridge->currentPage = v;
    return(v);
}
READ_MAPPER(2)
{
    u32 mapAddr = addr;
    if( addr >= 0xC000 )
    {
        // go to last page. This range always goes to the last bank
        mapAddr = (cartridge->romPageSize * cartridge->pages - 0x4000) + (addr & 0x3fff);        
    }
    else
    {
        // go to selected page
        mapAddr = ((addr - 0x8000) & 0x3fff) | (cartridge->currentPage << 14);        
    }
    return cartridge->ROM[mapAddr];
}   
WRITE_VMAPPER(2)
{
    if( cartridge->usingVRAM )
    {
        cartridge->VRAM[addr] = v;
        return(v);
    }
    else
    {
        // this should never happen...
        return(0);
    }
}
READ_VMAPPER(2)
{
    if( cartridge->usingVRAM )
    {
        return cartridge->VRAM[addr];
    }
    else
    {
        return cartridge->VROM[addr];
    }
}