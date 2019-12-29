#include <string.h>
#include <stdio.h>
#include <assert.h>
#define NES_IMPLEMENTATION
#include "nes.h"

#include "nes_ppu.cpp"
#include "nes_bus.cpp"
#include "nes_cpu.cpp"

#define internal static

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
    if( fread(&header, 1, sizeof(NESHeader), fp) == sizeof(NESHeader) )
    {
        // check the ROM header info
        if( header.id[0] == 'N' && header.id[1] == 'E' && header.id[2] == 'S' && header.id[3] == '\32' )
        {
            // skip trainer data
            if (header.mapper1 & 0x04)
            {                
                fseek(fp, 512, SEEK_CUR);
            }

            cartridge->mirror = (header.mapper1 & 0x01) ? kVertical : kHorizontal;
            
            // buffer the ROM
            cartridge->pages = header.romBanks;
            if( cartridge->ROM )
            {
                delete[] cartridge->ROM;
            }
            cartridge->ROM = new unsigned char[ cartridge->pages * ROM_PAGESIZE];
            fread( cartridge->ROM, 1, cartridge->pages * ROM_PAGESIZE, fp );

            // buffer the VROM
            cartridge->vpages = header.vromBanks;
            if( cartridge->VROM )
            {
                delete[] cartridge->VROM;
            }
            cartridge->VROM = new unsigned char[ cartridge->vpages * VROM_PAGESIZE];
            fread( cartridge->VROM, 1, cartridge->vpages * VROM_PAGESIZE, fp );
            
            ok = true;
        }

    }

    // close
    fclose(fp); 

    cartridge->loaded = ok;  
    return(ok);
}

NES_INIT(NES_Init)
{
    // memset(context->nes.RAM, 0, sizeof(context->nes.RAM));
    
    RUN_MODE prevRunMode = context->runMode;
    context->runMode = kPause;
    context->nes = {};
    if( LoadROM(romPath, &context->nes.cartridge) )
    {
        context->nes.dmaSyncFlag = true;

        InitializeCPU(&context->nes.cpu, &context->nes); 
        InitializePPU(&context->nes.ppu);     
    }
    context->runMode = prevRunMode;
}

NES_UPDATE(NES_Update)
{
    if( context->nes.cartridge.loaded && context->runMode != kPause )
    {    
        // generate a new frame
        context->nes.ppu.frameRendered = false;
        context->totalCycles = 0;
        do
        {
            UpdatePPU(&context->nes.ppu, context);

            if( context->nes.ppu.nmiRequested )
            {
                context->nes.ppu.nmiRequested = false;
                context->nes.cpu.nmi_now = true;
            }
            // the ppu is approx 3 times faster
            for( u32 i = 0; i < 3; ++i )
            {
                if( !context->nes.enableDMA )
                {
                    // Regular cpu update
                    UpdateCPU(&context->nes.cpu, context);
                    context->totalCycles += context->deltaCycles;
                }
                else
                {
                    // Perform DMA
                    if( context->nes.dmaSyncFlag )
                    {
                        if( i % 2 == 1 )
                        {
                            context->nes.dmaSyncFlag = false;
                        }
                    }
                    else
                    {
                        if( i % 2 == 0 )
                        {
                            context->nes.dmaData = RB(&context->nes, context->nes.dmaPage << 8 | context->nes.dmaAddr);
                        }
                        else
                        {
                            ((byte*)context->nes.ppu.OAM)[context->nes.dmaAddr] = context->nes.dmaData;
                            ++context->nes.dmaAddr;
                            
                            if( context->nes.dmaAddr == 0x0 )
                            {
                                // dma transfer ended, we have wrapped around
                                context->nes.enableDMA = false;
                                context->nes.dmaSyncFlag = true;
                            }
                        }
                    }
                }
            }
        } while( !context->nes.ppu.frameRendered );        
        
        if( context->runMode == kStep ) 
        {
            context->runMode = kPause;
        }
    }
}

NES_SHUTDOWN(NES_Shutdown)
{
    context->runMode = kPause;
    if( context->nes.cartridge.loaded )
    {
        delete[] context->nes.cartridge.ROM;
        delete[] context->nes.cartridge.VROM;
    }
}