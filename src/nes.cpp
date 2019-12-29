#include <string.h>
#include <stdio.h>
#include <assert.h>
#define NES_IMPLEMENTATION
#include "nes.h"

#include "nes_mappers.cpp"
#include "nes_rom.cpp"
#include "nes_ppu.cpp"
#include "nes_bus.cpp"
#include "nes_cpu.cpp"

#define internal static

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