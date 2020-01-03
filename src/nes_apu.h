#pragma once
#ifndef NES_APU_H
#define NES_APU_H

#include "nes.h"

byte WriteAPU(NES *nes, u16 addr, byte v);
byte ReadAPU(NES *nes, u16 addr);

void InitializeAPU(APU_RP2A *apu);
i16 GetSoundSample(APU_RP2A *apu);
void UpdateAPU(NESContext *ctx, APU_RP2A *apu);

#endif // NES_APU_H