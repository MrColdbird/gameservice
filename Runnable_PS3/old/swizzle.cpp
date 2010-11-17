/*   SCE CONFIDENTIAL                                       */
/*   PlayStation(R)3 Programmer Tool Runtime Library 330.001 */
/*   Copyright (C) 2006 Sony Computer Entertainment Inc.    */
/*   All Rights Reserved.                                   */

#include <sys/cdefs.h>
#include <assert.h>

#include "cellutil.h"

CDECL_BEGIN

static void convertSwizzle(uint8_t *&dst, uint8_t *&src,
						   const uint32_t width, const uint32_t depth,
						   const uint32_t xpos, const uint32_t ypos,
						   const uint32_t level)
{
	if (level == 1) {
		if (depth == 16) { // FP32
			*((uint32_t*&)dst)++ = *((uint32_t*)src+(ypos * width + xpos));
		}
		else if (depth == 8) { // FP16
			*((uint32_t*&)dst)++ = *((uint32_t*)src+(ypos * width + xpos));
		}
		else if (depth == 4) { // RGBA or ARGB
			*((uint32_t*&)dst)++ = *((uint32_t*)src+(ypos * width + xpos));
		}
		else if (depth == 3) { // RGB
			*dst++ = src[(ypos * width + xpos) * depth];
			*dst++ = src[(ypos * width + xpos) * depth + 1];
			*dst++ = src[(ypos * width + xpos) * depth + 2];
		}
		else {
			assert(0); // invalid depth size
		}
		return;
	}
	else {
		convertSwizzle(dst, src, width, depth, xpos, ypos, level - 1);
		convertSwizzle(dst, src, width, depth,
					   xpos + (1U << (level - 2)), ypos, level - 1);
		convertSwizzle(dst, src, width, depth,
					   xpos, ypos + (1U << (level - 2)), level - 1);
		convertSwizzle(dst, src, width, depth, xpos + (1U << (level - 2)),
					   ypos + (1U << (level - 2)), level - 1);
	}
}

void cellUtilConvertLinearToSwizzle(uint8_t *dst,
									uint8_t *src,
									const uint32_t width,
									const uint32_t height,
									const uint32_t depth)
{
	if (width == height) {
		convertSwizzle(dst, src, width, depth, 0, 0, cellUtilLog2(width) + 1);
	}
	else if (width > height) {
		uint32_t baseLevel
			= cellUtilLog2(width)-(cellUtilLog2(width)-cellUtilLog2(height));
		for (uint32_t i = 0;
			 i < (1UL << (cellUtilLog2(width)-cellUtilLog2(height))); i++) {
			convertSwizzle(dst, src, width, depth,
						   (1U << baseLevel)*i, 0, baseLevel + 1);
		}
	}
	else if (width < height) {
		uint32_t baseLevel
			= cellUtilLog2(height)-(cellUtilLog2(height)-cellUtilLog2(width));
		for (uint32_t i = 0;
			 i < (1UL << (cellUtilLog2(height)-cellUtilLog2(width))); i++) {
			convertSwizzle(dst, src, width, depth, 0,
						   (1U << baseLevel)*i, baseLevel + 1);
		}
	}
}

CDECL_END
