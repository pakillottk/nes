#pragma once
#ifndef NES_BUS_H
#define NES_BUS_H

#include "nes.h"

byte RB(NES *nes, u16 addr);
byte WB(NES *nes, u16 addr, u8 v);

#endif // NES_BUS_H