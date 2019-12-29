#pragma once
#ifndef NES_MAPPERS_H
#define NES_MAPPERS_H

#include "nes.h"
#include "nes_rom.h"
#include <stdio.h>

bool8 LoadData(NESCartridge *cartridge, NESHeader *header, FILE *fp);

byte ReadMapper(NESCartridge *cartridge, u16 addr);
byte WriteMapper(NESCartridge *cartridge, u16 addr, byte v);

// for video
byte ReadVMapper(NESCartridge *cartridge, u16 addr);
// for video
byte WriteVMapper(NESCartridge *cartridge, u16 addr, byte v);

#endif // NES_MAPPERS_H