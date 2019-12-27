#define NES_IMPLEMENTATION
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "nes.h"

#include "nes_bus.cpp"
#include "nes_cpu.cpp"

#define internal static

internal bool8
LoadROM(const char *romPath, NESCartridge *cartridge)
{
    // Open rom file
    FILE* fp = fopen( romPath, "rb" );
    if( fp == NULL ) {
        return false;
    }    

    bool8 ok = false;
    // Check the NES header
    if((fgetc(fp)=='N' && fgetc(fp)=='E' && fgetc(fp)=='S' && fgetc(fp)=='\32'))
    {
        // read ROM header info
        byte pgrSize16K = fgetc(fp);
        byte chrSize8K  = fgetc(fp);
        byte flags6     = fgetc(fp);
        u32  mapper     = fgetc(fp) | (flags6 >>4);
        fseek( fp, 16, SEEK_SET );
        
        // buffer the ROM
        cartridge->pages = 8 * pgrSize16K;
        cartridge->ROM = new unsigned char[ pgrSize16K * ROM_PAGESIZE];
        fread( cartridge->ROM, pgrSize16K, 16384, fp );     
        
        // buffer the VROM
        cartridge->vpages = 8 * chrSize8K;
        cartridge->VROM = new unsigned char[ chrSize8K * VROM_PAGESIZE];
        fread( cartridge->VROM, chrSize8K, 8192, fp );
        
        ok = true;
    }

    // close
    fclose(fp); 

    cartridge->loaded = ok;  
    return(ok);
}

NES_INIT(NES_Init)
{
    memset(context->nes.RAM, 0, sizeof(context->nes.RAM));

    // TODO(pgm) This should be passed from outside
    if( LoadROM("data/Donkey Kong (World) (Rev A).nes", &context->nes.cartridge) )
    {
        InitializeCPU(&context->nes.cpu, &context->nes);    
    }
}

NES_UPDATE(NES_Update)
{
    if( context->nes.cartridge.loaded && context->runMode != kPause )
    {        
        UpdateCPU(&context->nes.cpu, context);
        if( context->runMode == kStep ) 
        {
            context->runMode = kPause;
        }
    }
}