#pragma once
#ifndef NES_MAPPERS_H
#define NES_MAPPERS_H

#include "nes.h"
#include "nes_rom.h"
#include <stdio.h>

inline void
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
        cartridge->VRAM = new unsigned char[ vromSize ];
        memset(cartridge->VRAM, 0, vromSize);
    }
    else
    {
        cartridge->VROM = new unsigned char[ cartridge->vpages * cartridge->vromPageSize];
        fread( cartridge->VROM, 1, cartridge->vpages * cartridge->vromPageSize, fp );
    }
}

#define LOAD_MAPPER(NUM, ROM_SIZE, VROM_SIZE, TASKS) internal void LoadMapper##NUM(NESCartridge *cartridge, NESHeader *header, FILE *fp){ LoadMapperWithSize(cartridge, header, fp, ROM_SIZE, VROM_SIZE); {TASKS;} }
#define LOAD_MAPPER_CUSTOM(NUM) internal void LoadMapper##NUM(NESCartridge *cartridge, NESHeader *header, FILE *fp)
#define WRITE_MAPPER(NUM) internal byte WriteMapper##NUM(NESCartridge *cartridge, u16 addr, byte v)
#define READ_MAPPER(NUM) internal byte ReadMapper##NUM(NESCartridge *cartridge, u16 addr)
#define WRITE_VMAPPER(NUM) internal byte WriteVMapper##NUM(NESCartridge *cartridge, u16 addr, byte v)
#define READ_VMAPPER(NUM) internal byte ReadVMapper##NUM(NESCartridge *cartridge, u16 addr)

bool8 LoadData(NESCartridge *cartridge, NESHeader *header, FILE *fp);

byte ReadMapper(NESCartridge *cartridge, u16 addr);
byte WriteMapper(NESCartridge *cartridge, u16 addr, byte v);

// for video
byte ReadVMapper(NESCartridge *cartridge, u16 addr);
// for video
byte WriteVMapper(NESCartridge *cartridge, u16 addr, byte v);

#endif // NES_MAPPERS_H