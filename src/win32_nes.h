#pragma once
#ifndef WIN32_NES_H
#define WIN32_NES_H

#include <Windows.h>
#include "nes_types.h"
#include "nes.h"

struct NesCode
{
    bool8 isValid;
    HMODULE dllHandle;
    FILETIME lastWriteTime;
    nes_init *initialize;
    nes_update *update;    
};

#endif // WIN32_NES_H