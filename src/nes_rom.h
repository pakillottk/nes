#pragma once
#ifndef NES_ROM_H
#define NES_ROM_H

#include "nes.h"

// iNES Format Header
struct NESHeader
{
    char id[4];
    byte romBanks;
    byte vromBanks;
    byte mapper1;
    byte mapper2;
    byte ramSize;
    byte system1;
    byte system2;
    char padding[5];
} header;

bool8 LoadROM(const char *romPath, NESCartridge *cartridge);


#endif // NES_ROM_H