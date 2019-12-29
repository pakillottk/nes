#include "nes_mappers.h"
#define internal static

#define LOAD_MAPPER(NUM) void LoadMapper##NUM(NESCartridge *cartridge, NESHeader *header, FILE *fp)
#define WRITE_MAPPER(NUM) byte WriteMapper##NUM(NESCartridge *cartridge, u16 addr, byte v)
#define READ_MAPPER(NUM) byte ReadMapper##NUM(NESCartridge *cartridge, u16 addr)
#define WRITE_VMAPPER(NUM) byte WriteVMapper##NUM(NESCartridge *cartridge, u16 addr, byte v)
#define READ_VMAPPER(NUM) byte ReadVMapper##NUM(NESCartridge *cartridge, u16 addr)

// Mapper 0
internal
LOAD_MAPPER(0)
{
    // buffer the ROM
    cartridge->pages = header->romBanks;
    if( cartridge->ROM )
    {
        delete[] cartridge->ROM;
    }
    cartridge->ROM = new unsigned char[ cartridge->pages * ROM_PAGESIZE];
    fread( cartridge->ROM, 1, cartridge->pages * ROM_PAGESIZE, fp );

    // buffer the VROM
    cartridge->vpages = header->vromBanks;
    if( cartridge->VROM )
    {
        delete[] cartridge->VROM;
    }
    cartridge->VROM = new unsigned char[ cartridge->vpages * VROM_PAGESIZE];
    fread( cartridge->VROM, 1, cartridge->vpages * VROM_PAGESIZE, fp );
}

internal 
WRITE_VMAPPER(0)
{
    // doesn't do anything
    return(v);
}
internal
READ_VMAPPER(0)
{
    u16 mappedAddr = addr % VROM_PAGESIZE;
    return cartridge->VROM[mappedAddr];
}
internal 
WRITE_MAPPER(0)
{
    // doesn't do anything
    return(v);
}
internal
READ_MAPPER(0)
{
    u16 mapAddr = addr & (cartridge->pages > 1 ? 0x7FFF : 0x3FFF);
    return cartridge->ROM[mapAddr];
}

bool8 
LoadData(NESCartridge *cartridge, NESHeader *header, FILE *fp)
{
    #define HANDLE_LOAD_MAPPER(NUM)\
    case NUM: LoadMapper##NUM(cartridge, header, fp); break;

    switch (cartridge->mapper)
    {
        HANDLE_LOAD_MAPPER(0);

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

        default:
            // when mapper its unknown always return NOP
            return(0xea);
    }
}