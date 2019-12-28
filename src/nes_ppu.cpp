#include "nes_ppu.h"

#define internal static
#define global_variable static

#define RGB(R, G, B)  u32(0xff000000) | (R&0xff) << 16 | (G&0xFF) << 8  | (B&0xFF)

global_variable const u32 nes_palette[] = 
{    
    RGB(84, 84, 84),
	RGB(0, 30, 116),
	RGB(8, 16, 144),
	RGB(48, 0, 136),
	RGB(68, 0, 100),
	RGB(92, 0, 48),
	RGB(84, 4, 0),
	RGB(60, 24, 0),
	RGB(32, 42, 0),
	RGB(8, 58, 0),
	RGB(0, 64, 0),
	RGB(0, 60, 0),
	RGB(0, 50, 60),
	RGB(0, 0, 0),
	RGB(0, 0, 0),
	RGB(0, 0, 0),

	RGB(152, 150, 152),
	RGB(8, 76, 196),
	RGB(48, 50, 236),
	RGB(92, 30, 228),
	RGB(136, 20, 176),
	RGB(160, 20, 100),
	RGB(152, 34, 32),
	RGB(120, 60, 0),
	RGB(84, 90, 0),
	RGB(40, 114, 0),
	RGB(8, 124, 0),
	RGB(0, 118, 40),
	RGB(0, 102, 120),
	RGB(0, 0, 0),
	RGB(0, 0, 0),
	RGB(0, 0, 0),

	RGB(236, 238, 236),
	RGB(76, 154, 236),
	RGB(120, 124, 236),
	RGB(176, 98, 236),
	RGB(228, 84, 236),
	RGB(236, 88, 180),
	RGB(236, 106, 100),
	RGB(212, 136, 32),
	RGB(160, 170, 0),
	RGB(116, 196, 0),
	RGB(76, 208, 32),
	RGB(56, 204, 108),
	RGB(56, 180, 204),
	RGB(60, 60, 60),
	RGB(0, 0, 0),
	RGB(0, 0, 0),

	RGB(236, 238, 236),
	RGB(168, 204, 236),
	RGB(188, 188, 236),
	RGB(212, 178, 236),
	RGB(236, 174, 236),
	RGB(236, 174, 212),
	RGB(236, 180, 176),
	RGB(228, 196, 144),
	RGB(204, 210, 120),
	RGB(180, 222, 120),
	RGB(168, 226, 144),
	RGB(152, 226, 180),
	RGB(160, 214, 228),
	RGB(160, 162, 160),
	RGB(0, 0, 0),
	RGB(0, 0, 0)
};

internal byte
ReadVRAM(NES *nes, u16 addr)
{
    u16 mappedAddr = addr;
    byte data = 0;
    if( addr >= 0x0000 && addr <= 0x1FFF )
    {
        // this is because of mapper0. Else we have to map the right vram bank
        mappedAddr = mappedAddr % VROM_PAGESIZE;
        data = nes->cartridge.VROM[mappedAddr];
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
		data = nes->ppu.palette[mappedAddr] & (nes->ppu.ppumask.grayscale ? 0x30 : 0x3F);
    }

    return(data);
}

internal byte 
WriteVRAM(NES *nes, u16 addr, byte data)
{
    u16 mappedAddr = addr;
    if( addr >= 0x0000 && addr <= 0x1FFF )
    {        
        // doesn't make much sense...
        nes->cartridge.VROM[addr % VROM_PAGESIZE] = data;
        // nes->ppu.patternTable[(addr & 0x1000) >> 12][addr & 0x0FFF] = data;
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
		nes->ppu.palette[mappedAddr] = data;
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
            data = nes->ppu.ppuctrl.data;       
        break;

        case 1: // mask can't be readed  
            data = nes->ppu.ppumask.data;          
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
                nes->ppu.latcher = true;
            }
            else
            {
                nes->ppu.tram_addr.fineY = v & 0x07;
                nes->ppu.tram_addr.coarseY = v >> 3;
                nes->ppu.latcher = false;
            }
        break;

        case 6: // PPU addr
            if( !nes->ppu.latcher )
            {
                nes->ppu.tram_addr.data = (u16)((v & 0x3F) << 8) | (nes->ppu.tram_addr.data & 0x00FF);
                nes->ppu.latcher = true;               
            }
            else
            {
                nes->ppu.tram_addr.data = (nes->ppu.tram_addr.data & 0xFF00) | v;
			    nes->ppu.vram_addr = nes->ppu.tram_addr;
                nes->ppu.latcher = false;
            }
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
            ppu->vram_addr.nametableX = !ppu->vram_addr.nametableX;
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
                ppu->vram_addr.nametableY = !ppu->vram_addr.nametableY;
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
    if( ppu->ppumask.showBg )
    {
        ppu->bgPatternLo <<= 1;
        ppu->bgPatternHi <<= 1;
        ppu->bgAttribLo <<= 1;
        ppu->bgAttribHi <<= 1;
    }
}

internal u32
ResolvePaletteColor(NES *nes,byte palette, byte pixel)
{
    u32 idx = ReadVRAM(nes, 0x3F00 + (palette << 2) + pixel) & 0x3F;
    return nes_palette[idx];
}

void
UpdatePPU(PPU_2C02 *ppu, NESContext *context)
{
    if( ppu->scanline >= -1 && ppu->scanline <= 240 )
    {
        if(ppu->scanline == 0 && ppu->cycle == 0)
		{
			// cycle skip
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
                                                            + (ppu->vram_addr.fineY));
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

    // Render to the backbuffer
    byte bg = 0x0;  
	byte bgPaletteIdx = 0x0;

    if( ppu->ppumask.showBg )
    {
        u16 mask = 0x8000 >> ppu->fineX;

        byte p0 = (ppu->bgPatternLo & mask) > 0;
        byte p1 = (ppu->bgPatternHi & mask) > 0;

        bg = (p1 << 1) | p0;

        byte p0PalIdx = (ppu->bgAttribLo & mask) > 0;
        byte p1PalIdx = (ppu->bgAttribHi & mask) > 0;
        bgPaletteIdx = (p1PalIdx << 1) | p0PalIdx;        

        i32 x = ppu->cycle - 1;
        i32 y = ppu->scanline;
        if( x >= 0 && x < NES_FRAMEBUFFER_WIDTH && 
            y >= 0 && y < NES_FRAMEBUFFER_HEIGHT )
        {
            context->backbuffer[ y * NES_FRAMEBUFFER_WIDTH + x ] = ResolvePaletteColor(&context->nes, bgPaletteIdx, bg);
        }
    }
    

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
