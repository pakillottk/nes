#include "nes_ppu.h"

#define internal static

internal byte
ReadVRAM(NES *nes, u16 addr)
{
    u16 mappedAddr = addr;
    byte data = 0;
    if( addr >= 0x0000 && addr <= 0x1FFF )
    {
        // this is because of mapper0. Else we have to map the right vram bank
        data = nes->cartridge.VROM[addr];
    }
    else if( addr >= 0x2000 && addr <= 0x3EFF )
    {
        mappedAddr &= 0x0FFF;

        if( nes->cartridge.mirror == kVertical )
        {
            if( mappedAddr >= 0x0000 && mappedAddr <= 0x03FF )
            {
				data = nes->ppu.nameTable[0][mappedAddr & 0x03FF];
            }
			else if( mappedAddr >= 0x0400 && mappedAddr <= 0x07FF )
            {
				data = nes->ppu.nameTable[1][mappedAddr & 0x03FF];
            }
			else if (mappedAddr >= 0x0800 && mappedAddr <= 0x0BFF )
            {
				data = nes->ppu.nameTable[0][mappedAddr & 0x03FF];
            }
			else if( mappedAddr >= 0x0C00 && mappedAddr <= 0x0FFF )
            {
				data = nes->ppu.nameTable[1][mappedAddr & 0x03FF];
            }
        }
        else // horizontal mirroring
        {
            if( mappedAddr >= 0x0000 && mappedAddr <= 0x03FF )
            {
				data = nes->ppu.nameTable[0][mappedAddr & 0x03FF];
            }
			else if( mappedAddr >= 0x0400 && mappedAddr <= 0x07FF )
            {
				data = nes->ppu.nameTable[0][mappedAddr & 0x03FF];                
            }
			else if( mappedAddr >= 0x0800 && mappedAddr <= 0x0BFF )
            {
				data = nes->ppu.nameTable[1][mappedAddr & 0x03FF];
            }
			else if( mappedAddr >= 0x0C00 && mappedAddr <= 0x0FFF )
            {
				data = nes->ppu.nameTable[1][mappedAddr & 0x03FF];
            }
        }
    }
    else if( addr >= 0x3F00 && addr <= 0x3FFF )
    {
        mappedAddr &= 0x001F;
        if( mappedAddr == 0x0010 )
        {
            mappedAddr = 0x0000;
        } 
		else if( mappedAddr == 0x0014 ) 
        {
            mappedAddr = 0x0004;
        }
		else if( mappedAddr == 0x0018 )
        {
            mappedAddr = 0x0008;
        } 
		else if( mappedAddr == 0x001C ) 
        {
            mappedAddr = 0x000C;
        }
		data = nes->ppu.palette[addr] & (nes->ppu.ppumask.grayscale ? 0x30 : 0x3F);
    }

    return(data);
}

internal byte 
WriteVRAM(NES *nes, u16 addr, byte data)
{
    u16 mappedAddr = addr;
    if( addr >= 0x0000 && addr <= 0x1FFF )
    {
        // cant write to the cartdrige
        fprintf(stderr, "Attempt to write into the cartridge VROM...\n");
    }
    else if( addr >= 0x2000 && addr <= 0x3EFF )
    {
        mappedAddr &= 0x0FFF;

        if( nes->cartridge.mirror == kVertical )
        {
            if( mappedAddr >= 0x0000 && mappedAddr <= 0x03FF )
            {
			    nes->ppu.nameTable[0][mappedAddr & 0x03FF] = data;
            }
			else if( mappedAddr >= 0x0400 && mappedAddr <= 0x07FF )
            {
				nes->ppu.nameTable[1][mappedAddr & 0x03FF] = data;
            }
			else if (mappedAddr >= 0x0800 && mappedAddr <= 0x0BFF )
            {
				nes->ppu.nameTable[0][mappedAddr & 0x03FF] = data;
            }
			else if( mappedAddr >= 0x0C00 && mappedAddr <= 0x0FFF )
            {
				nes->ppu.nameTable[1][mappedAddr & 0x03FF] = data;
            }
        }
        else // horizontal mirroring
        {
            if( mappedAddr >= 0x0000 && mappedAddr <= 0x03FF )
            {
				nes->ppu.nameTable[0][mappedAddr & 0x03FF] = data;
            }
			else if( mappedAddr >= 0x0400 && mappedAddr <= 0x07FF )
            {
				nes->ppu.nameTable[0][mappedAddr & 0x03FF] = data;
            }
			else if( mappedAddr >= 0x0800 && mappedAddr <= 0x0BFF )
            {
				nes->ppu.nameTable[1][mappedAddr & 0x03FF] = data;
            }
			else if( mappedAddr >= 0x0C00 && mappedAddr <= 0x0FFF )
            {
				nes->ppu.nameTable[1][mappedAddr & 0x03FF] = data;
            }
        }
    }
    else if( addr >= 0x3F00 && addr <= 0x3FFF )
    {
        mappedAddr &= 0x001F;
        if( mappedAddr == 0x0010 )
        {
            mappedAddr = 0x0000;
        } 
		else if( mappedAddr == 0x0014 ) 
        {
            mappedAddr = 0x0004;
        }
		else if( mappedAddr == 0x0018 )
        {
            mappedAddr = 0x0008;
        } 
		else if( mappedAddr == 0x001C ) 
        {
            mappedAddr = 0x000C;
        }
		nes->ppu.palette[addr] = data;
    }
    return(data);
}

byte 
ReadPPU(NES *nes, u16 addr)
{
    // the address entered is from 2000 to 3FFF, we map it to 0-7
    // (which is the effective address for a ppu register)
    u16 mappedAddr = addr % 8;

    byte data = 0;
    switch(mappedAddr)
    {
        case 0: // control can't be readed            
        break;

        case 1: // mask can't be readed            
        break;

        case 2: // status            
            data = (nes->ppu.ppustatus.data & 0xE0) | (nes->ppu.dataBuffer & 0x1F);

            // clear vblank
            nes->ppu.ppustatus.vblank = false;

            // reset latcher
            nes->ppu.latcher = false;
        break;

        case 3: // OAM addr            
            // TODO
        break;

        case 4: // OAM Data            
            // TODO
        break;

        case 5: // scroll can't be readed            
        break;

        case 6: // ppu addr can't be readed                        
        break;

        case 7: // data  
            data = nes->ppu.dataBuffer;
            nes->ppu.dataBuffer = ReadVRAM(nes, nes->ppu.vram_addr.data);

            if( nes->ppu.vram_addr.data >= 0x3F00 )
            {
                data = nes->ppu.dataBuffer;
            }

            nes->ppu.vram_addr.data += ( nes->ppu.ppuctrl.increment ? 32 : 1 );
        break;
    }

    return(data);
}
byte 
WritePPU(NES *nes, u16 addr, byte v)
{
     // the address entered is from 2000 to 3FFF, we map it to 0-7
    // (which is the effective address for a ppu register)
    u16 mappedAddr = addr % 8;

    switch (mappedAddr)
    {
        case 0: // control
            nes->ppu.ppuctrl.data = v;
            nes->ppu.tram_addr.nametableX = nes->ppu.ppuctrl.nametableX;
            nes->ppu.tram_addr.nametableY = nes->ppu.ppuctrl.nametableY;
        break;

        case 1: // mask
            nes->ppu.ppumask.data = v;
        break;

        case 2: // status
        break;

        case 3: // OAM Addr
        break;

        case 4: // OAM Data
        break;

        case 5: // scroll
            if( !nes->ppu.latcher )
            {
                nes->ppu.fineX = v & 0x07;
                nes->ppu.tram_addr.coarseX = v >> 3;
            }
            else
            {
                nes->ppu.tram_addr.fineY = v & 0x07;
                nes->ppu.tram_addr.coarseY = v >> 3;
            }
            // always invert the latcher
            nes->ppu.latcher = !nes->ppu.latcher;
        break;

        case 6: // PPU addr
            if( !nes->ppu.latcher )
            {
                nes->ppu.tram_addr.data = (nes->ppu.tram_addr.data & 0xFF00) | v;                
            }
            else
            {
                nes->ppu.tram_addr.data = (nes->ppu.tram_addr.data & 0x00FF) | v;
            }
            // always invert the latcher
            nes->ppu.latcher = !nes->ppu.latcher;
        break;

        case 7: // PPU Data
            WriteVRAM(nes, nes->ppu.vram_addr.data, v);
            nes->ppu.vram_addr.data += ( nes->ppu.ppuctrl.increment ? 32 : 1 );
        break;
    }

    return(v);
}

void 
InitializePPU(PPU_2C02 *ppu)
{
    // zero out everything
    *ppu = {};
}

internal void
IncrementScrollX(PPU_2C02 *ppu)
{
    if( ppu->ppumask.showBg || ppu->ppumask.showSprites )
    {
        if( ppu->vram_addr.coarseX == 31 )
        {            
            ppu->vram_addr.coarseX = 0;            
            ppu->vram_addr.nametableX = ~ppu->vram_addr.nametableX;
        }
        else
        {            
            ppu->vram_addr.coarseX++;
        }
    }
}

internal void
IncrementScrollY(PPU_2C02 *ppu)
{
    if( ppu->ppumask.showBg || ppu->ppumask.showSprites )
    {
        if (ppu->vram_addr.fineY < 7)
        {
            ++ppu->vram_addr.fineY;
        }
        else
        {
            // Reset fine y offset
            ppu->vram_addr.fineY = 0;
            
            if(ppu->vram_addr.coarseY == 29)
            {                
                ppu->vram_addr.coarseY = 0;                
                ppu->vram_addr.nametableY = ~ppu->vram_addr.nametableY;
            }
            else if (ppu->vram_addr.coarseY == 31)
            {
                ppu->vram_addr.coarseY = 0;
            }
            else
            {
                ++ppu->vram_addr.coarseY;
            }
        }        
    }
}

internal void
TransferX(PPU_2C02 *ppu)
{
    if( ppu->ppumask.showBg || ppu->ppumask.showSprites )
    {
        ppu->vram_addr.nametableX = ppu->tram_addr.nametableX;
        ppu->vram_addr.coarseX = ppu->tram_addr.coarseX;
    }
}

internal void
TransferY(PPU_2C02 *ppu)
{
    if( ppu->ppumask.showBg || ppu->ppumask.showSprites )
    {
        ppu->vram_addr.nametableY = ppu->tram_addr.nametableY;
        ppu->vram_addr.coarseY = ppu->tram_addr.coarseY;
        ppu->vram_addr.fineY = ppu->tram_addr.fineY;
    }
}

internal void
LoadBackgroundBits(PPU_2C02 *ppu)
{
    ppu->bgPatternLo = (ppu->bgPatternLo & 0xFF00) | ppu->bgTileLsb;
    ppu->bgPatternHi = (ppu->bgPatternHi & 0xFF00) | ppu->bgTileMsb;

    ppu->bgAttribLo = (ppu-> bgAttribLo & 0xFF00) | ((ppu->bgTileAttrib & 0b01) ? 0xFF : 0x00);
    ppu->bgAttribHi = (ppu-> bgAttribHi & 0xFF00) | ((ppu->bgTileAttrib & 0b10) ? 0xFF : 0x00);
}

internal void
UpdateBits(PPU_2C02 *ppu)
{
    ppu->bgPatternLo <<= 1;
    ppu->bgPatternHi <<= 1;
    ppu->bgAttribLo <<= 1;
    ppu->bgAttribHi <<= 1;
}

void
UpdatePPU(PPU_2C02 *ppu, NESContext *context)
{
    if( ppu->scanline >= -1 && ppu->scanline <= 240 )
    {
        if(ppu->scanline == 0 && ppu->cycle == 0)
		{
			// "Odd Frame" cycle skip
			ppu->cycle = 1;
		}

        if( ppu->scanline == -1 && ppu->cycle == 1 )
		{
			// Start of new frame
			ppu->ppustatus.vblank = 0;
		}

        if( (ppu->cycle >= 2 && ppu->cycle < 258) || (ppu->cycle >= 321 && ppu->cycle < 338) )
        {
            UpdateBits(ppu);

            switch( (ppu->cycle-1) % 8 )
            {
                case 0:
                    LoadBackgroundBits(ppu);
                    ppu->bgTileId = ReadVRAM(&context->nes, 0x2000 | (ppu->vram_addr.data & 0x0FFF));                    
                break;

                case 2:
                    ppu->bgTileAttrib = ReadVRAM(&context->nes, 0x23C0  | (ppu->vram_addr.nametableY << 11) 
                                                                        | (ppu->vram_addr.nametableX << 10) 
                                                                        | ((ppu->vram_addr.coarseY >> 2) << 3) 
                                                                        | (ppu->vram_addr.coarseX >> 2));
                    if( ppu->vram_addr.coarseY & 0x02 )
                    {
                        ppu->bgTileAttrib >>= 4;
                    }
                    if( ppu->vram_addr.coarseX & 0x02 )
                    {
                        ppu->bgTileAttrib >>= 2;
                    }
                    ppu->bgTileAttrib &= 0x03;
                break;

                case 4:
                    ppu->bgTileLsb = ReadVRAM(&context->nes, (ppu->ppuctrl.bgPattern << 12) 
                                                            + ((u16)ppu->bgTileId << 4) 
                                                            + ppu->vram_addr.fineY);                                                            
                break;

                case 6:
                    ppu->bgTileMsb = ReadVRAM(&context->nes, (ppu->ppuctrl.bgPattern << 12)
					                       + ((u16)ppu->bgTileId << 4)
					                       + (ppu->vram_addr.fineY) + 8);
                break;

                case 7:
                    IncrementScrollX(ppu);
                break;
            }
        }

        if( ppu->cycle == 256 )
        {
            IncrementScrollY(ppu);
        }

        if( ppu->cycle == 257 )
        {
            LoadBackgroundBits(ppu);
            TransferX(ppu);
        }

        if( ppu->cycle == 338 || ppu->cycle == 340 )
		{
			ppu->bgTileId = ReadVRAM(&context->nes, 0x2000 | (ppu->vram_addr.data & 0x0FFF));
		}

        if( ppu->scanline == -1 && ppu->cycle >= 280 && ppu->cycle < 305 )
        {
            TransferY(ppu);
        }
    }

    if( ppu->scanline == 241 && ppu->cycle == 1 )
    {
        ppu->ppustatus.vblank = true;
        if( ppu->ppuctrl.nmi )
        {
            ppu->nmiRequested = true;
        }
    }

    // TODO(pgm) Render to the backbuffer

    ++ppu->cycle;
    if( ppu->cycle >= 341 )
    {
        ppu->cycle = 0;
        ++ppu->scanline;
        if( ppu->scanline >= 261 )
        {
            ppu->scanline = -1;            
            // frame rendered  
            ppu->frameRendered = true;          
        }
    }
}
