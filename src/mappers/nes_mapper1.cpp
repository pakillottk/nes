#include "../nes_mappers.h"

#define internal static

struct MM1
{
    enum BANK_SIZE
    {
        k32kb = KB(32),
        k16kb = KB(16),
        k8kb = KB(8),
        k4kb = KB(4)
    };

    BANK_SIZE prgBankSize;
    bool8 fixLastBank;
    bool8 fixFirstBank;
    byte prgBank0;
    byte prgBank1;
    BANK_SIZE chrSwapMode; // 2 of 4kb or 1 of 8kb
    byte chrBank0;
    byte chrBank1;

    byte shiftRegister;
    byte shiftCount;
};

internal void
Reset(NESCartridge *cartridge, MM1 *mm1)
{        
    mm1->shiftRegister = 0;
    mm1->shiftCount = 0;
    mm1->prgBank1 = cartridge->pages - 1;
    mm1->fixLastBank = true;
}

LOAD_MAPPER_CUSTOM(1) 
{
    LoadMapperWithSize(cartridge, header, fp, KB(16), KB(8));

    MM1 *mm1 = new MM1();
    *mm1 = {};
    mm1->prgBankSize = MM1::k16kb;    
    mm1->prgBank0 = 0;
    Reset(cartridge, mm1);
    cartridge->mapperData = mm1;
}


WRITE_MAPPER(1)
{
    MM1 *mm1 = (MM1*)cartridge->mapperData;
    
    if( (v & 0x80) != 0 )
    {
        // reset bit
        Reset(cartridge, mm1);
        return(v);
    }

    mm1->shiftRegister |= (( v & 1) << mm1->shiftCount);
    ++mm1->shiftCount;
    if( mm1->shiftCount < 5 )
    {
        return(v);
    }

    if( addr >= 0x8000 && addr <= 0x9FFF )
    {
        // control
        switch( (mm1->shiftRegister & 0b11) )
        {
            case 0: 
                cartridge->mirror = kOneScreenTop;
            break;

            case 1: 
                cartridge->mirror = kOneScreenBottom;
            break;

            case 2:
                cartridge->mirror = kVertical;
            break;

            case 3:
                cartridge->mirror = kHorizontal;
            break;
        }

        switch ( (mm1->shiftRegister & 0b1100) >> 2 )
        {
            case 0:
            case 1:
                mm1->prgBankSize = MM1::k32kb;
                mm1->fixFirstBank = false;
                mm1->fixLastBank = false;
            break;

            case 2:
                mm1->prgBankSize = MM1::k16kb;
                mm1->fixFirstBank = true;
                mm1->prgBank0 = 0;
            break;

            case 3:
                mm1->prgBankSize = MM1::k16kb;
                mm1->fixFirstBank = false;
                mm1->fixLastBank = true;
                mm1->prgBank1 = cartridge->pages - 1;
            break;
        }

        mm1->chrSwapMode = (mm1->shiftRegister & 0b10000) ? MM1::k4kb : MM1::k8kb;
    }
    else if( addr >= 0xA000 && addr <= 0xBFFF )
    {
        // chr0
        mm1->chrBank0 = mm1->shiftRegister;
    }
    else if( addr >= 0xC000 && addr <= 0xDFFF )
    {
        if( mm1->chrSwapMode == MM1::k4kb )
        {
            // chr1
            mm1->chrBank1 = mm1->shiftRegister;
        }
    }
    else if( addr >= 0xE000 && addr <= 0xFFFF )
    {
        // prg
        if( !mm1->fixFirstBank )
        {
            mm1->prgBank0 = mm1->shiftRegister;
        }
        else if( !mm1->fixLastBank )
        {
            mm1->prgBank1 = mm1->shiftRegister;
        }
    }

    mm1->shiftRegister = 0;
    mm1->shiftCount = 0;

    return(v);
}

READ_MAPPER(1)
{
    MM1 *mm1 = (MM1*)cartridge->mapperData;
    byte data = 0;
    
    if( addr >= 0x8000 && addr <= 0xBFFF )
    {
        if( mm1->fixFirstBank )
        {
            return cartridge->ROM[addr & (mm1->prgBankSize-1)];
        }
        else
        {
            return cartridge->ROM[ (mm1->prgBankSize * mm1->prgBank0) + (addr & (mm1->prgBankSize-1)) ];
        }
    }
    else if( addr >= 0xC000 && addr <= 0xFFFF )
    {
        if( mm1->fixLastBank )
        {
            return cartridge->ROM[ (cartridge->romPageSize * cartridge->pages - 0x4000) + (addr & 0x3fff) ];
        }
        else
        {            
            return cartridge->ROM[ (cartridge->romPageSize * mm1->prgBank1) + (addr & 0x3fff) ];
        }
    }

    return(data);
}   
WRITE_VMAPPER(1)
{
    if( cartridge->usingVRAM )
    {
        cartridge->VRAM[addr] = v;
        return(v);
    }
    else
    {
        // this should never happen...
        return(0);
    }
}
READ_VMAPPER(1)
{
    MM1 *mm1 = (MM1*)cartridge->mapperData;
    if( cartridge->usingVRAM )
    {
        return cartridge->VRAM[addr];
    }
    else
    {
        if( mm1->chrSwapMode == MM1::k8kb )
        {
            // fixed
            return cartridge->VROM[addr];
        }
        else
        {
            // use the right bank
            if( addr >= 0x0000 && addr <= 0x0FFF )
            {
                // chr0
                return cartridge->VROM[ (KB(4) * mm1->chrBank0) + (addr & (KB(4)-1)) ];
            }
            else if( addr >= 0x1000 && addr <= 0x1FFF )
            {
                // chr1
                return cartridge->VROM[ (KB(4) * mm1->chrBank1) + (addr & (KB(4)-1)) ];
            }
        } 
    }

    return(0);
}