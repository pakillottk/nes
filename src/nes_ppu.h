#pragma once
#ifndef NES_PPU_H
#define NES_PPU_H

#include "nes.h"

byte ReadPPU(NES *nes, u16 addr);
byte WritePPU(NES *nes, u16 addr, byte v);

void InitializePPU(PPU_2C02 *ppu);
void UpdatePPU(PPU_2C02 *ppu, NESContext *context);

#endif // NES_PPU_H