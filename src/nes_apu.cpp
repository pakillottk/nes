#include "nes_apu.h"
#define internal static

#define SET_LENGTH(LENGTH, TARGET)\
    switch ( LENGTH )\
    {\
        case 0b11111: TARGET = 30; break;\
        case 0b11101: TARGET = 28; break;\
        case 0b11011: TARGET = 26; break;\
        case 0b11001: TARGET = 24; break;\
        case 0b10111: TARGET = 22; break;\
        case 0b10101: TARGET = 20; break;\
        case 0b10011: TARGET = 18; break;\
        case 0b10001: TARGET = 16; break;\
        case 0b01111: TARGET = 14; break;\
        case 0b01101: TARGET = 12; break;\
        case 0b01011: TARGET = 10; break;\
        case 0b01001: TARGET = 8; break;\
        case 0b00111: TARGET = 6; break;\
        case 0b00101: TARGET = 4; break;\
        case 0b00011: TARGET = 2; break;\
        case 0b00001: TARGET = 254; break;\
        case 0b11110: TARGET = 32; break;\
        case 0b11100: TARGET = 1; break;\
        case 0b11010: TARGET = 72; break;\
        case 0b11000: TARGET = 192; break;\
        case 0b10110: TARGET = 96; break;\
        case 0b10100: TARGET = 48; break;\
        case 0b10010: TARGET = 24; break;\
        case 0b10000: TARGET = 12; break;\
        case 0b01110: TARGET = 26; break;\
        case 0b01100: TARGET = 14; break;\
        case 0b01010: TARGET = 60; break;\
        case 0b01000: TARGET = 160; break;\
        case 0b00110: TARGET = 80; break;\
        case 0b00100: TARGET = 40; break;\
        case 0b00010: TARGET = 20; break;\
        case 0b00000: TARGET = 10; break;\
    }


byte 
WriteAPU(NES *nes, u16 addr, byte v)
{
    switch (addr)
    {
        case 0x4000:
            switch ( (v & 0xC0) >> 6 )
            {
                case 0x0: 
                    nes->apu.pulse1Seq.seq = 0b00000001;
                    nes->apu.pulse1OSC.dutyCycle = 0.125;
                break;
                case 0x1: 
                    nes->apu.pulse1Seq.seq = 0b00000011;
                    nes->apu.pulse1OSC.dutyCycle = 0.25;
                break;
                case 0x2: 
                    nes->apu.pulse1Seq.seq = 0b00001111;
                    nes->apu.pulse1OSC.dutyCycle = 0.5;
                break;
                case 0x3: 
                    nes->apu.pulse1Seq.seq = 0b11111100;
                    nes->apu.pulse1OSC.dutyCycle = 0.75;
                break;
            }
            nes->apu.pulse1HaltLength = v & 0x20;
            nes->apu.pulse1OSC.amplitude = v & 0x0F;
        break;

        case 0x4001:
            
        break;

        case 0x4002:
            nes->apu.pulse1Seq.reload = (nes->apu.pulse1Seq.reload & 0xFF00) | v;
        break;

        case 0x4003:
        {
            nes->apu.pulse1Seq.reload = (u16)((v & 0x07)) << 8 | (nes->apu.pulse1Seq.reload & 0x00FF);
            // length counter
            byte length = v >> 3;
            SET_LENGTH(length, nes->apu.pulse1Length);            
        }
        break;

        case 0x4004:
            switch ( (v & 0xC0) >> 6  )
            {
                case 0x0: 
                    nes->apu.pulse2Seq.seq = 0b00000001;
                    nes->apu.pulse2OSC.dutyCycle = 0.125;
                break;
                case 0x1: 
                    nes->apu.pulse2Seq.seq = 0b00000011;
                    nes->apu.pulse2OSC.dutyCycle = 0.25;
                break;
                case 0x2: 
                    nes->apu.pulse2Seq.seq = 0b00001111;
                    nes->apu.pulse2OSC.dutyCycle = 0.5;
                break;
                case 0x3: 
                    nes->apu.pulse2Seq.seq = 0b11111100;
                    nes->apu.pulse2OSC.dutyCycle = 0.75;
                break;
            }
            nes->apu.pulse2HaltLength = v & 0x20;
            nes->apu.pulse2OSC.amplitude = v & 0x0F;
        break;

        case 0x4005:
            
        break;

        case 0x4006:
            nes->apu.pulse2Seq.reload = (nes->apu.pulse2Seq.reload & 0xFF00) | v;
        break;

        case 0x4007:
        {
            nes->apu.pulse2Seq.reload = (u16)((v & 0x07)) << 8 | (nes->apu.pulse2Seq.reload & 0x00FF);
            // length counter
            byte length = v >> 3;
            SET_LENGTH(length, nes->apu.pulse2Length);
        }
        break;

        case 0x4008:
            
        break;

        case 0x400C:
            
        break;

        case 0x400E:
            
        break;

        case 0x4015:
            nes->apu.pulse1Enabled = v & 0x1;
            if( !nes->apu.pulse1Enabled )
            {
                nes->apu.pulse1Length = 0;
            }
            nes->apu.pulse2Enabled = v & 0x2;
            if( !nes->apu.pulse2Enabled )
            {
                nes->apu.pulse2Length = 0;
            }
        break;

        case 0x400F:
            
        break;
    }

    return(v);
}

byte
ReadAPU(NES *nes, u16 addr)
{    
    // In theory this should never happen...
    return(0);
}

void 
InitializeAPU(APU_RP2A *apu)
{
    *apu = {};
}

internal void
SeqSquareWave(u32 *s)
{
    *s =((*s & 0x1) << 7) | ((*s & 0xFE) >> 1);
}

i16 
GetSoundSample(APU_RP2A *apu)
{
    double p1 = apu->pulse1Enabled ? apu->pulse1Sample : 0;
    double p2 = apu->pulse1Enabled ? apu->pulse2Sample : 0;
    double pulse_out = 95.88 / ((8128 / (p1 + p2)) + 100.0);
    return i16(10000.0 * pulse_out);
}   

void 
UpdateAPU(NESContext *ctx, APU_RP2A *apu)
{
    ctx->globalTime += (0.3333333333333 / 1789773.0);
    bool8 quarterFrameClock = false;
    bool8 halfFrameClock = false;

    if( apu->ClockCounter % 6 == 0)
    {
        ++apu->FrameClockCounter;
        quarterFrameClock = apu->FrameClockCounter == 3729 || apu->FrameClockCounter == 7457 
                            || apu->FrameClockCounter == 11186 || apu->FrameClockCounter == 14916;
        halfFrameClock = apu->FrameClockCounter == 7457 || apu->FrameClockCounter == 14916;
        if( apu->FrameClockCounter == 14916 )
        {
            apu->FrameClockCounter = 0;
        }

        if( quarterFrameClock )
        {
            // volume envelope
        }

        if( halfFrameClock )
        {
            // note length and frequency sweepers
            if( !apu->pulse1HaltLength && apu->pulse1Length > 0 )
            {
                --apu->pulse1Length;
            }   
            if( !apu->pulse2HaltLength && apu->pulse2Length > 0 )
            {
                --apu->pulse2Length;
            }   
        }
        
        const double timePerSample = 1.0 / 44100.0;        
        apu->pulse1OSC.freq = 1789773.0 / (16.0 * (double)(apu->pulse1Seq.reload + 1));   
        apu->pulse2OSC.freq = 1789773.0 / (16.0 * (double)(apu->pulse2Seq.reload + 1)); 
             
        if( ctx->audioTime >= timePerSample ) 
        {            
            ctx->hasSample = true;
            apu->pulse1Sample = apu->pulse1Enabled && apu->pulse1Length > 0 ? apu->pulse1OSC.sample(ctx->globalTime) : 0; 
            apu->pulse2Sample = apu->pulse2Enabled && apu->pulse2Length > 0 ? apu->pulse2OSC.sample(ctx->globalTime) : 0;
        }     
    }
    ++apu->ClockCounter;
}