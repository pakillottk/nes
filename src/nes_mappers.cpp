#include "nes_mappers.h"
#define internal static

#include "mappers/nes_mapper0.cpp"
#include "mappers/nes_mapper1.cpp"
#include "mappers/nes_mapper2.cpp"

bool8 
LoadData(NESCartridge *cartridge, NESHeader *header, FILE *fp)
{
    #define HANDLE_LOAD_MAPPER(NUM)\
    case NUM: LoadMapper##NUM(cartridge, header, fp); break;

    switch (cartridge->mapper)
    {
        HANDLE_LOAD_MAPPER(0);
        HANDLE_LOAD_MAPPER(1);
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
        HANDLE_READMAPPER(1);
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
        HANDLE_WRITREMAPPER(1);
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
        HANDLE_READVMAPPER(1);
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
        HANDLE_WRITREVMAPPER(1);
        HANDLE_WRITREVMAPPER(2);

        default:
            // when mapper its unknown always return NOP
            return(0xea);
    }
}