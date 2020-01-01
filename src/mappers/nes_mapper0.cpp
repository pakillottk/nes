#include "..\nes_mappers.h"

LOAD_MAPPER(0, KB(16), KB(8), {}) 
WRITE_VMAPPER(0)
{
    // doesn't do anything
    return(v);
}
READ_VMAPPER(0)
{
    u16 mappedAddr = addr % cartridge->vromPageSize;
    return cartridge->VROM[mappedAddr];
} 
WRITE_MAPPER(0)
{
    // doesn't do anything
    return(v);
}
READ_MAPPER(0)
{
    u16 mapAddr = addr & (cartridge->pages > 1 ? 0x7FFF : 0x3FFF);
    return cartridge->ROM[mapAddr];
}