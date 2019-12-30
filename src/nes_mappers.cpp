#include "nes_mappers.h"
#define internal static

internal void
LoadMapperWithSize(NESCartridge *cartridge, NESHeader *header, FILE *fp, u32 romSize, u32 vromSize)
{
    cartridge->romPageSize = romSize;
    cartridge->vromPageSize = vromSize;

    // buffer the ROM
    cartridge->pages = header->romBanks;   
    cartridge->ROM = new unsigned char[ cartridge->pages * cartridge->romPageSize];
    fread( cartridge->ROM, 1, cartridge->pages * cartridge->romPageSize, fp );

    // buffer the VROM
    cartridge->vpages = header->vromBanks;    
    if( cartridge->vpages == 0 )
    {
        cartridge->usingVRAM = true;
        cartridge->VRAM = new unsigned char[ KB(8) ];
        memset(cartridge->VRAM, 0, KB(8));
    }
    else
    {
        cartridge->VROM = new unsigned char[ cartridge->vpages * cartridge->vromPageSize];
        fread( cartridge->VROM, 1, cartridge->vpages * cartridge->vromPageSize, fp );
    }
}

#define LOAD_MAPPER(NUM, ROM_SIZE, VROM_SIZE, TASKS) internal void LoadMapper##NUM(NESCartridge *cartridge, NESHeader *header, FILE *fp){ LoadMapperWithSize(cartridge, header, fp, ROM_SIZE, VROM_SIZE); TASKS; }
#define LOAD_MAPPER_CUSTOM(NUM) internal void LoadMapper##NUM(NESCartridge *cartridge, NESHeader *header, FILE *fp)
#define WRITE_MAPPER(NUM) internal byte WriteMapper##NUM(NESCartridge *cartridge, u16 addr, byte v)
#define READ_MAPPER(NUM) internal byte ReadMapper##NUM(NESCartridge *cartridge, u16 addr)
#define WRITE_VMAPPER(NUM) internal byte WriteVMapper##NUM(NESCartridge *cartridge, u16 addr, byte v)
#define READ_VMAPPER(NUM) internal byte ReadVMapper##NUM(NESCartridge *cartridge, u16 addr)

// Mapper 0
#pragma region MAPPER_0
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
#pragma endregion

// Mapper 2
#pragma region MAPPER_2
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

#pragma endregion

bool8 
LoadData(NESCartridge *cartridge, NESHeader *header, FILE *fp)
{
    #define HANDLE_LOAD_MAPPER(NUM)\
    case NUM: LoadMapper##NUM(cartridge, header, fp); break;

    switch (cartridge->mapper)
    {
        HANDLE_LOAD_MAPPER(0);
        HANDLE_LOAD_MAPPER(2);

        default:
            return(false);
    }

    return(true);
}

byte 
ReadMapper(NESCartridge *cartridge, u16 addr)
{
    #define HANDLE_READMAPPER(NUM)\
    case NUM: return ReadMapper##NUM(cartridge, addr)

    switch( cartridge->mapper )
    {
        HANDLE_READMAPPER(0);
        HANDLE_READMAPPER(2);

        default:
            // when mapper its unknown always return NOP
            return(0xea);
    }
}

byte
WriteMapper(NESCartridge *cartridge, u16 addr, byte v)
{
    #define HANDLE_WRITREMAPPER(NUM)\
    case NUM: return WriteMapper##NUM(cartridge, addr, v)

    switch( cartridge->mapper )
    {
        HANDLE_WRITREMAPPER(0);
        HANDLE_WRITREMAPPER(2);

        default:
            // when mapper its unknown always return NOP
            return(0xea);
    }
}

byte 
ReadVMapper(NESCartridge *cartridge, u16 addr)
{
    #define HANDLE_READVMAPPER(NUM)\
    case NUM: return ReadVMapper##NUM(cartridge, addr)

    switch( cartridge->mapper )
    {
        HANDLE_READVMAPPER(0);
        HANDLE_READVMAPPER(2);

        default:
            // when mapper its unknown always return NOP
            return(0xea);
    }
}

byte 
WriteVMapper(NESCartridge *cartridge, u16 addr, byte v)
{
    #define HANDLE_WRITREVMAPPER(NUM)\
    case NUM: return WriteVMapper##NUM(cartridge, addr, v)

    switch( cartridge->mapper )
    {
        HANDLE_WRITREVMAPPER(0);
        HANDLE_WRITREVMAPPER(2);

        default:
            // when mapper its unknown always return NOP
            return(0xea);
    }
}