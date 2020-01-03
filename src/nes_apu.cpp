#include "nes_apu.h"
#define internal static

byte 
WriteAPU(NES *nes, u16 addr, byte v)
{
    switch (addr)
    {
        case 0x4000:
            switch ( (v & 0xC0) >> 6  )
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
        break;

        case 0x4001:
            
        break;

        case 0x4002:
            nes->apu.pulse1Seq.reload = (nes->apu.pulse1Seq.reload & 0xFF00) | v;
        break;

        case 0x4003:
            nes->apu.pulse1Seq.reload = (u16)((v & 0x07)) << 8 | (nes->apu.pulse1Seq.reload & 0x00FF);
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
        break;

        case 0x4005:
            
        break;

        case 0x4006:
            nes->apu.pulse2Seq.reload = (nes->apu.pulse2Seq.reload & 0xFF00) | v;
        break;

        case 0x4007:
            nes->apu.pulse2Seq.reload = (u16)((v & 0x07)) << 8 | (nes->apu.pulse2Seq.reload & 0x00FF);
        break;

        case 0x4008:
            
        break;

        case 0x400C:
            
        break;

        case 0x400E:
            
        break;

        case 0x4015:
            nes->apu.pulse1Enabled = v & 0x1;
            nes->apu.pulse2Enabled = v & 0x2;
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

        }
        apu->pulse1OSC.freq = 1789773.0 / (16.0 * (double)(apu->pulse1Seq.reload + 1));   
        apu->pulse2OSC.freq = 1789773.0 / (16.0 * (double)(apu->pulse2Seq.reload + 1));        

        const double timePerSample = 1.0 / 44100.0;
        if( ctx->audioTime >= timePerSample ) 
        {
            apu->pulse1Sample = apu->pulse1Enabled ? apu->pulse1OSC.sample(ctx->globalTime) : 0; 
            apu->pulse2Sample = apu->pulse2Enabled ? apu->pulse2OSC.sample(ctx->globalTime) : 0; 
            ctx->hasSample = true;
        }     
    }
    ++apu->ClockCounter;
}