#include "nes_rom.h"
#include <stdio.h>

bool8
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
            // read all the cart data
            cartridge->mapper = ((header.mapper2 >> 4) << 4) | (header.mapper1 >> 4);
            cartridge->mirror = (header.mapper1 & 0x01) ? kVertical : kHorizontal;
            cartridge->hasBattery = (header.mapper1 & 0x02) >> 1;
            ok = LoadData(cartridge, &header, fp);
        }
    }

    // close
    fclose(fp); 

    cartridge->loaded = ok;  
    return(ok);
}