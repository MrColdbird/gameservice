/*   SCE CONFIDENTIAL                                       */
/*   PlayStation(R)3 Programmer Tool Runtime Library 330.001 */
/*   Copyright (C) 2006 Sony Computer Entertainment Inc.    */
/*   All Rights Reserved.                                   */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <cell/gcm.h>

#include "gcmutil.h"

#define NV_CHIP_NUM_MEMPARTS 2

static unsigned int g_tilePitch = 0;
static unsigned int g_baseAddress = 0;
static unsigned int g_bankSense   = 0;

static 
unsigned int Table4[16] =
{ 0, 1, 2, 3,
2, 3, 0, 1,
1, 2, 3, 0,
3, 0, 1, 2
};

struct MainMemAdrxlateTilingParams_t
{
	unsigned int prime;
	unsigned int factor;
	
	unsigned int base_adr;
	unsigned int tile_width;
	unsigned int bank_sense;
	unsigned int col_bits;
	unsigned int row_bits;
	unsigned int bank_bits;
	unsigned int lines_in_tile;
	unsigned int num_partitions;
	unsigned int tile_line_bits;
	unsigned int tile_width_B;
	unsigned int tile_addr_multiple;

};

static MainMemAdrxlateTilingParams_t g_tp;

/* helper function for getting a bit slice */
static unsigned int fb_bits(unsigned int d, int j, int i)
{
	if (j >= i) return (((d) >> (i)) & ((0xffffffff)>>(31-((j)-(i)))));
	else return 0;
}

/* helper function for concatenating bit slices */

static
unsigned int catbits(unsigned int f3, int e3, int s3,
                     unsigned int f2, int e2, int s2,
                     unsigned int f1, int e1, int s1,
                     unsigned int f0, int e0, int s0)
{
	unsigned int out = 0;   
	/*                  */ out  = fb_bits(f3, e3, s3);  
	out <<= (e2 - s2 + 1); out |= fb_bits(f2, e2, s2);  
	out <<= (e1 - s1 + 1); out |= fb_bits(f1, e1, s1);  
	out <<= (e0 - s0 + 1); out |= fb_bits(f0, e0, s0);  
	return out;
}

static
unsigned int catbits(
                     unsigned int f5, int e5, int s5,
                     unsigned int f4, int e4, int s4,
                     unsigned int f3, int e3, int s3,
                     unsigned int f2, int e2, int s2,
                     unsigned int f1, int e1, int s1,
                     unsigned int f0, int e0, int s0)
{
	unsigned int out = 0;   
	out <<= (e5 - s5 + 1); out |= fb_bits(f5, e5, s5);  
	out <<= (e4 - s4 + 1); out |= fb_bits(f4, e4, s4);  
	out <<= (e3 - s3 + 1); out |= fb_bits(f3, e3, s3);  
	out <<= (e2 - s2 + 1); out |= fb_bits(f2, e2, s2);  
	out <<= (e1 - s1 + 1); out |= fb_bits(f1, e1, s1);  
	out <<= (e0 - s0 + 1); out |= fb_bits(f0, e0, s0);  
	return out;
}

#if 0
static
void
splitbits(unsigned int v, 
          unsigned int *f1, int b1,
          unsigned int *f0, int b0
          )
{
	*f0 = v & ((1 << b0)-1);     v >>= b0;
	*f1 = v & ((1 << b1)-1);     v >>= b1;
}
#endif

static void cellGcmUtilSetTileConfiguration(MainMemAdrxlateTilingParams_t *my_tp)
{
	
	// copy the tiling params for adr space into the my_tp struct.
	my_tp->tile_width = 256;   // all tiles are 256B wide
	
	my_tp->col_bits = 10;
	my_tp->row_bits = 16;
	my_tp->bank_bits = 2;
	my_tp->num_partitions = 2;	
	my_tp->lines_in_tile = 64;	
	my_tp->tile_line_bits = 6;	
	my_tp->tile_width_B = 256;   // An fbtile is 256B wide on screen.
	my_tp->tile_addr_multiple = 0x10000;

	// lines_in_tile depends on partitions and column bits.
	unsigned int tile_size_B = 8 * my_tp->num_partitions * (1<< my_tp->col_bits);
	
	my_tp->lines_in_tile = tile_size_B / my_tp->tile_width;
	
	my_tp->tile_line_bits = 6;
	
	my_tp->base_adr   = g_baseAddress >> 16;
	my_tp->bank_sense = g_bankSense;
	
	unsigned int pitchTable[]  = {0x200, 0x300, 0x400, 0x500, 0x600, 0x700, 0x800, 0xA00, 0xC00, 0xD00, 0xE00, 0x1000, 0x1400, 0x1800, 0x1A00, 0x1C00, 0x2000, 0x2800, 0x3000, 0x3400, 0x3800, 0x4000, 0x5000, 0x6000, 0x6800, 0x7000, 0x8000, 0xA000, 0xC000, 0xD000, 0xE000, 0x10000 };
	
	unsigned int factorTable[] = {0x2, 0x1, 0x4, 0x1, 0x2, 0x1, 0x8, 0x2, 0x4, 0x1, 0x2, 0x10, 0x4, 0x8, 0x2, 0x4, 0x20, 0x8, 0x10, 0x4, 0x8, 0x40, 0x10, 0x20, 0x8, 0x10, 0x80, 0x20, 0x40, 0x10, 0x20, 0x100};
	unsigned int primeTable[]  = {0x1, 0x3, 0x1, 0x5, 0x3, 0x7, 0x1, 0x5, 0x3, 0xd, 0x7, 0x1, 0x5, 0x3, 0xd, 0x7, 0x1, 0x5, 0x3, 0xd, 0x7, 0x1, 0x5, 0x3, 0xd, 0x7, 0x1, 0x5, 0x3, 0xd, 0x7, 0x1};
	
	int tableNumber = -1;
	
	for (unsigned int i = 0 ; i < sizeof(pitchTable) / sizeof(unsigned int) ; ++i)
	{
		if (g_tilePitch == pitchTable[i])
		{
			tableNumber = i;
			break;
		}
	}
	
	if (tableNumber >= 0)
	{
		my_tp->prime  = primeTable[tableNumber];
		my_tp->factor = factorTable[tableNumber];
	}
	else
	{
		assert(0);
	}
}

void
cellGcmUtilLinearToTileAddress(unsigned long *adrOut, unsigned long adrIn)
{
	
	unsigned long myuma = adrIn;

	unsigned int bank_flip     = 0xdeadbeef;
	unsigned int column_adr    = 0xdeadbeef;
	unsigned int row_adr       = 0xdeadbeef;
	unsigned int bank_adr      = 0xdeadbeef;
	unsigned int partition_adr = 0xdeadbeef;

	// fixed constants
	unsigned int pitch_B = g_tp.prime * g_tp.factor * g_tp.tile_width_B;
	unsigned int tiles_per_row = pitch_B / g_tp.tile_width_B;
	
	unsigned int base_adr = g_tp.base_adr * g_tp.tile_addr_multiple; // base address in bytes
	
	unsigned int L = int((adrIn - base_adr) / g_tp.tile_width_B);
	
	unsigned int seg_x = L % tiles_per_row;        
	unsigned int seg_y = int(L / tiles_per_row);   // essentially the y coordinate, since the segments are 1 pixel tall
	
	unsigned int tile_x = seg_x;
	unsigned int tile_y = int(seg_y / g_tp.lines_in_tile);   
	
	unsigned int tile_num = tile_y * tiles_per_row + tile_x;      
	
	
	unsigned int tile_adr = fb_bits(tile_num,21,0) + fb_bits(base_adr,31,14);
	
	// intile offset
	unsigned int intile_offset = seg_y % 64;
	
	row_adr = (tile_adr >> g_tp.bank_bits ) & ((1 << g_tp.row_bits)-1);
	
	if (g_tp.factor==1)
	{
			bank_adr = fb_bits(tile_adr,1,0);
	}
	else if (g_tp.factor==2)
	{
			unsigned int idx = fb_bits(tile_adr+(tile_y%2?2:0),1,0)*4+fb_bits(tile_y,1,0);
			bank_adr = Table4[idx];
	}
	else if (g_tp.factor >= 4)
	{
			unsigned int idx = fb_bits(tile_adr,1,0)*4+fb_bits(tile_y,1,0);
			bank_adr = Table4[idx];
	}
	else
	{
		assert(0);
	}
	
	// remember the bank flip
	bank_adr = (bank_adr + g_tp.bank_sense) % 4;
	bank_flip = bank_adr ^ fb_bits(tile_adr,1,0);
	
	column_adr = catbits(intile_offset, g_tp.tile_line_bits-1,  3, 
		adrIn,         7,                 5,
		intile_offset, 1,                 0,
		0,             1,                 0);

	partition_adr = (fb_bits(intile_offset,2,2) + fb_bits(adrIn,6,6) ) % 2;

	partition_adr += ( ( (g_tp.num_partitions < NV_CHIP_NUM_MEMPARTS)) ? (NV_CHIP_NUM_MEMPARTS/2) : 0);
    
	myuma = catbits( 
		row_adr, 15, 0,
		bank_adr, 1, 0,
		column_adr, 9, 4,
		partition_adr, 0, 0,
		column_adr, 3, 2,
		adrIn, 4, 0);

	myuma ^= (fb_bits(myuma, 11, 11) << 10); 
	myuma ^= ( (fb_bits(myuma, 12, 12) ^ fb_bits(bank_flip,0,0) ^ fb_bits(myuma, 14, 14)) << 9);

	if (adrOut) { *adrOut = myuma;}

}



int cellGcmUtilConvertLinearToTileFormat(unsigned int baseAddress, unsigned int tilePitch, unsigned int bankSense, unsigned int texSize)
{
	
	// must be set by API
	g_baseAddress = baseAddress;
	g_tilePitch   = tilePitch;
	g_bankSense   = bankSense;
	
	unsigned long adrOut = 0;
	
	cellGcmUtilSetTileConfiguration(&g_tp);
	
	unsigned int textureWidth  = texSize;
	unsigned int textureHeight = texSize;
	unsigned int bytePerPixel  = 4;
	
	for (unsigned int y = 0 ; y < textureHeight ; ++y)
	{
		for (unsigned int x = 0 ; x < textureWidth ; ++x)
		{
			// generate linear address for the pixel
			uint32_t addr = baseAddress + (y * textureWidth + x) * bytePerPixel;
			
			// linear to tile transnaltion
			cellGcmUtilLinearToTileAddress(&adrOut, addr );
			
			// write test pixel data to the translated address
			*(unsigned long *)(adrOut) = ((y & 0xff) << 16) + ((x & 0xff) << 8);
		}
	}
	return 0;
	
}

