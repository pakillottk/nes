#include <string.h>
#include <stdio.h>
#include <assert.h>
#define NES_IMPLEMENTATION
#include "nes.h"

#include "nes_mappers.cpp"
#include "nes_rom.cpp"
#include "nes_ppu.cpp"
#include "nes_apu.cpp"
#include "nes_bus.cpp"
#include "nes_cpu.cpp"

#define internal static

NES_INIT(NES_Init)
{
    // memset(context->nes.RAM, 0, sizeof(context->nes.RAM));
    
    RUN_MODE prevRunMode = context->runMode;
    context->runMode = kPause;

    // ensure to free all the resources
    if( context->nes.cartridge.mapperData )
    {
        delete context->nes.cartridge.mapperData;
    }
    if( context->nes.cartridge.usingVRAM )
    {
        delete[] context->nes.cartridge.VRAM;
    }       
    if( context->nes.cartridge.ROM )
    {
        delete[] context->nes.cartridge.ROM;
    }
    if( context->nes.cartridge.VROM )
    {
        delete[] context->nes.cartridge.VROM;
    }
    context->nes = {};
    if( LoadROM(romPath, &context->nes.cartridge) )
    {
        context->nes.dmaSyncFlag = true;

        InitializeCPU(&context->nes.cpu, &context->nes); 
        InitializePPU(&context->nes.ppu);    
        InitializeAPU(&context->nes.apu);
    }
    context->runMode = prevRunMode;
}

NES_UPDATE(NES_Update)
{
    const double TIME_PER_SAMPLE = 1.0 / 48000.0;
    const double TIME_PER_CLOCK = 1.0 / 5369318.0;    

    if( context->nes.cartridge.loaded && context->runMode != kPause )
    {    
        // generate a new frame
        context->nes.ppu.frameRendered = false;
        context->hasSample = false;
        context->totalCycles = 0;
        //do
        {
            context->audioTime += TIME_PER_CLOCK;

            UpdatePPU(&context->nes.ppu, context);     
            UpdateAPU(context, &context->nes.apu);           

            // the ppu is approx 3 times faster
            if( context->nes.clockCounter % 3 == 0 )
            {
                if( !context->nes.enableDMA )
                {
                    // Regular cpu update
                    UpdateCPU(&context->nes.cpu, context);
                }
                else
                {
                    // Perform DMA
                    if( context->nes.dmaSyncFlag )
                    {
                        if( context->nes.clockCounter % 2 == 1 )
                        {
                            context->nes.dmaSyncFlag = false;
                        }
                    }
                    else
                    {
                        if( context->nes.clockCounter % 2 == 0 )
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

            if( context->hasSample )
            {
                context->audioTime -= TIME_PER_SAMPLE;
                context->audioSample = GetSoundSample(&context->nes.apu);
                context->hasSample = false;
                context->Audio.Samples[ context->nes.apu.SamplesGenerated++ ] = context->audioSample;
            }

            if( context->nes.ppu.nmiRequested )
            {
                NMI(&context->nes.cpu, context);
                context->nes.ppu.nmiRequested = false;
            }

            ++context->nes.clockCounter;
        } //while( !context->nes.ppu.frameRendered );        
        
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

        if( context->nes.cartridge.usingVRAM )
        {
            delete[] context->nes.cartridge.VRAM;
        }       
    }
    if( context->nes.cartridge.mapperData )
    {
        delete context->nes.cartridge.mapperData;
    }
}