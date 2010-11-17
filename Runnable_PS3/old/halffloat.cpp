/*   SCE CONFIDENTIAL                                       */
/*   PlayStation(R)3 Programmer Tool Runtime Library 330.001 */
/*   Copyright (C) 2006 Sony Computer Entertainment Inc.    */
/*   All Rights Reserved.                                   */

#include "cellutil.h"

/*
 * Half float conversion utility functions
 *
 * 32bit floating point format
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |s|   exponent    |                  mantissa                   |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  1      8bit                         23bit
 *
 * 16bit floating point format
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |s|exponent |     mantissa      |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  1   5bit          10bit
 *
 * For FP32 to FP16 conversion, just truncate upper bits in each field.
 * For FP16 to FP32 conversion, just extend bits.
 *
 */
uint16_t cellUtilFloatToHalf(const float val)
{
	uint8_t *tmp = (uint8_t*)&val;
	uint32_t bits = ((uint32_t)tmp[0] << 24) | ((uint32_t)tmp[1] << 16) | ((uint32_t)tmp[2] << 8) |(uint32_t)tmp[3];

	if (bits == 0) {
		return 0;
	}
	int32_t e = ((bits & 0x7f800000) >> 23) - 127 + 15;
	if (e < 0) {
		return 0;
	}
	else if (e > 31) {
		e = 31;
	}
	uint32_t s = bits & 0x80000000;
	uint32_t m = bits & 0x007fffff;

	return ((s >> 16) & 0x8000) | ((e << 10) & 0x7c00) | ((m >> 13) & 0x03ff);
}

float cellUtilHalfToFloat(const uint16_t val)
{
	if (val == 0) {
		return 0.0f;
	}
	uint32_t s = val & 0x8000;
	int32_t e =((val & 0x7c00) >> 10) - 15 + 127;
	uint32_t m =  val & 0x03ff;
	uint32_t floatVal = (s << 16) | ((e << 23) & 0x7f800000) | (m << 13);
	float result;
	uint8_t *tmp = (uint8_t*)&result;
	tmp[0] = (floatVal >> 24) & 0xff;
	tmp[1] = (floatVal >> 16) & 0xff;
	tmp[2] = (floatVal >> 8) & 0xff;
	tmp[3] = floatVal & 0xff;

	return result;
}
