#pragma once
#ifndef NES_CPU_H
#define NES_CPU_H

#include "nes.h"

void InitializeCPU(CPU_6502 *cpu);
void UpdateCPU(CPU_6502 *cpu, NESContext *context);

#endif // NES_CPU_H