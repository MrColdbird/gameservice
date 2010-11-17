/*   SCE CONFIDENTIAL                                       */
/*   PlayStation(R)3 Programmer Tool Runtime Library 330.001 */
/*   Copyright (C) 2006 Sony Computer Entertainment Inc.    */
/*   All Rights Reserved.                                   */

#include <stdio.h>
#include <assert.h>
#include <sys/return_code.h>
#include <cell/gcm.h>

#include "cellutil.h"
#include "gcmutil.h"

void cellUtilGenerateGradationTexture(uint8_t *texture,
									  const uint32_t width,
									  const uint32_t height,
									  const uint32_t depth)
{
	for (size_t x = 0; x < width; ++x) {
		for (size_t y = 0; y < height; ++y) {
			texture[depth*(width*y+x)] = 255;
			texture[depth*(width*y+x)+1] = 255;
			texture[depth*(width*y+x)+2] = (uint8_t)(int)floorf(x*256.f/width);
			texture[depth*(width*y+x)+3] = (uint8_t)(int)floorf(y*256.f/height);
		}
    }
}

void cellUtilGenerateSwizzledGradationTexture(uint8_t *texture,
											  const uint32_t width,
											  const uint32_t height,
											  const uint32_t depth)
{
	uint8_t *p = (uint8_t*)malloc(width*height*depth);
	assert(p != NULL);

	cellUtilGenerateGradationTexture(p, width, height, depth);
	cellUtilConvertLinearToSwizzle(texture, p, width, height, depth);

	free(p);
}

void cellUtilGenerateGradationTextureFP16(uint8_t *texture,
										  const uint32_t width,
										  const uint32_t height)
{
	uint16_t *buf = (uint16_t*)texture;
	for (size_t x = 0; x < width; ++x) {
		for (size_t y = 0; y < height; ++y) {
			buf[4*(width*y+x)] = cellUtilFloatToHalf(1.0f); // R
			buf[4*(width*y+x)+1] = cellUtilFloatToHalf((float)x/width); // G
			buf[4*(width*y+x)+2] = cellUtilFloatToHalf((float)y/height); // B
			buf[4*(width*y+x)+3] = cellUtilFloatToHalf(1.0f); // A
		}
    }
}

void cellUtilGenerateSwizzledGradationTextureFP16(uint8_t *texture,
												  const uint32_t width,
												  const uint32_t height)
{
	uint8_t *p = (uint8_t*)malloc(width*height*4*2); // half float size = 2
	assert(p != NULL);

	cellUtilGenerateGradationTextureFP16(p, width, height);
	cellUtilConvertLinearToSwizzle(texture, p, width * 2, height, 4*2);

	free(p);
}

void cellUtilGenerateGradationTextureFP32(uint8_t *texture,
										  const uint32_t width,
										  const uint32_t height)
{
	float *buf = (float*)texture;
	for (size_t x = 0; x < width; ++x) {
		for (size_t y = 0; y < height; ++y) {
			buf[4*(width*y+x)] = 1.0f; // R
			buf[4*(width*y+x)+1] = (float)x/width; // G
			buf[4*(width*y+x)+2] = (float)y/height; // B
			buf[4*(width*y+x)+3] = 1.0f; // A
		}
    }
}

void cellUtilGenerateSwizzledGradationTextureFP32(uint8_t *texture,
												  const uint32_t width,
												  const uint32_t height)
{
	uint8_t *p = (uint8_t*)malloc(width*height*4*sizeof(float));
	assert(p != NULL);

	cellUtilGenerateGradationTextureFP32(p, width, height);
	cellUtilConvertLinearToSwizzle(texture, p, width * sizeof(float), height,
								   4 * sizeof(float));

	free(p);
}

void cellUtilGenerateGridTexture(uint8_t *texture,
								 const uint32_t width, const uint32_t height,
								 const uint32_t depth,
								 const uint32_t linewidth,
								 const uint32_t linefreq)
{
	for (size_t y = 0; y < height; ++y) {
		for (size_t x = 0; x < width; ++x) {
			texture[depth*(width*y+x)] = 255;
			texture[depth*(width*y+x)+1] = 255;
			texture[depth*(width*y+x)+2] = 255;
			texture[depth*(width*y+x)+3] = 255;
		}
    }
	for (size_t y = 0; y < height; y++) {
		if ((y / linewidth % linefreq) == 0) {
			for (size_t x = 0; x < width; ++x) {
				texture[depth*(width*y+x)] = 255;
				texture[depth*(width*y+x)+1] = 0;
				texture[depth*(width*y+x)+2] = 0;
				texture[depth*(width*y+x)+3] = 0;
			}
		}
		else {
			for (size_t x = 0; x < width; x += linewidth*linefreq) {
				if ((x / linewidth % linefreq) == 0) {
					for (size_t i = 0; i < linewidth; ++i) {
						texture[depth*(width*y+x+i)] = 255;
						texture[depth*(width*y+x+i)+1] = 0;
						texture[depth*(width*y+x+i)+2] = 0;
						texture[depth*(width*y+x+i)+3] = 0;
					}
				}
			}
		}
    }
}

void cellUtilGenerateMipmap(uint8_t *texture,
							const uint32_t width, const uint32_t height,
							const uint32_t depth)
{
	uint32_t stride = width;
	uint32_t offset = stride*height*depth;
	uint32_t prevOffset = 0;
	uint32_t maxMipmap = ((cellUtilLog2(width)>cellUtilLog2(height)) ?
						  cellUtilLog2(width) : cellUtilLog2(height))+1;
	uint32_t textureWidth = width;
	uint32_t textureHeight = height;

	if (textureWidth != 1) {
		textureWidth /= 2;
	}
	if (textureHeight != 1) {
		textureHeight /= 2;
	}
	for (uint32_t k = 1; k < maxMipmap; ++k) {
		for (uint32_t y = 0; y < textureHeight; ++y) {
			for (uint32_t x = 0; x < textureWidth; ++x) {
				for (uint32_t i = 0; i < depth; ++i) {
					uint32_t tmp = 0;
					tmp += texture[prevOffset+depth*(stride*y*2+x*2)+i];
					tmp += texture[prevOffset+depth*(stride*y*2+x*2+1)+i];
					tmp += texture[prevOffset+depth*(stride*(y*2+1)+x*2)+i];
					tmp += texture[prevOffset+depth*(stride*(y*2+1)+x*2+1)+i];
					texture[offset+depth*(stride*y+x)+i] = tmp / 4;
				}
			}
		}

		prevOffset = offset; // save offset of last mipmap

		// for linear texture, stride is used instead of width
		offset += stride*textureHeight*depth;

		if (textureWidth != 1) {
			textureWidth /= 2;
		}
		if (textureHeight != 1) {
			textureHeight /= 2;
		}
    }
}

void cellUtilGenerateCubeMap(uint8_t *texture,
							 const uint32_t width, const uint32_t height,
							 const uint32_t depth, const uint32_t swizzled)
{
	uint32_t stride = width;
	uint32_t offset = 0;
	uint32_t colormap[6][3] = { // colorbar's color
		{ 255, 255,   0 }, {   0, 255, 255 },
		{   0, 255,   0 }, { 255,   0, 255 },
		{ 255,   0,   0 }, {   0,   0, 255 },
	};

	uint8_t *p = (uint8_t*)malloc(width*height*depth);
	assert(p != NULL);

	for (uint32_t k = 0; k < 6; ++k) {
		for (uint32_t y = 0; y < height; ++y) {
			for (uint32_t x = 0; x < width; ++x) {
				p[depth*(stride*y+x)] = 255;
				p[depth*(stride*y+x)+1] = colormap[k][0];
				p[depth*(stride*y+x)+2] = colormap[k][1];
				p[depth*(stride*y+x)+3] = colormap[k][2];
			}
		}
		if (swizzled != 0) {
			cellUtilConvertLinearToSwizzle(&texture[offset], p,
										   width, height, depth);
		}

		// for linear texture, stride is used instead of width
		offset += stride*height*depth;
    }

	free(p);
}

int cellGcmUtilGetTextureAttribute(const uint32_t glFormat,
								   uint32_t *gcmFormat, uint32_t *remap,
								   const uint32_t swizzle, const uint32_t normalize)
{
	switch (glFormat) {
	case CELL_GCM_UTIL_ARGB8: // GL_ARGB8
		*gcmFormat = CELL_GCM_TEXTURE_A8R8G8B8;
		if (swizzle == 0) {
			*gcmFormat |= CELL_GCM_TEXTURE_LN;
		}
		if (normalize == 0) {
			*gcmFormat |= CELL_GCM_TEXTURE_UN;
		}
		*remap = CELL_GCM_TEXTURE_REMAP_REMAP << 14 |
			CELL_GCM_TEXTURE_REMAP_REMAP << 12 |
			CELL_GCM_TEXTURE_REMAP_REMAP << 10 |
			CELL_GCM_TEXTURE_REMAP_REMAP << 8 |
			CELL_GCM_TEXTURE_REMAP_FROM_B << 6 |
			CELL_GCM_TEXTURE_REMAP_FROM_G << 4 |
			CELL_GCM_TEXTURE_REMAP_FROM_R << 2 |
			CELL_GCM_TEXTURE_REMAP_FROM_A;
		break;
	case CELL_GCM_UTIL_RGBA8: // GL_RGBA8
		*gcmFormat = CELL_GCM_TEXTURE_A8R8G8B8;
		if (swizzle == 0) {
			*gcmFormat |= CELL_GCM_TEXTURE_LN;
		}
		if (normalize == 0) {
			*gcmFormat |= CELL_GCM_TEXTURE_UN;
		}
		*remap = CELL_GCM_TEXTURE_REMAP_REMAP << 14 |
			CELL_GCM_TEXTURE_REMAP_REMAP << 12 |
			CELL_GCM_TEXTURE_REMAP_REMAP << 10 |
			CELL_GCM_TEXTURE_REMAP_REMAP << 8 |
			CELL_GCM_TEXTURE_REMAP_FROM_G << 6 |
			CELL_GCM_TEXTURE_REMAP_FROM_R << 4 |
			CELL_GCM_TEXTURE_REMAP_FROM_A << 2 |
			CELL_GCM_TEXTURE_REMAP_FROM_B;
		break;
	case CELL_GCM_UTIL_BGRA8: // GL_BGRA8
		*gcmFormat = CELL_GCM_TEXTURE_A8R8G8B8;
		if (swizzle == 0) {
			*gcmFormat |= CELL_GCM_TEXTURE_LN;
		}
		if (normalize == 0) {
			*gcmFormat |= CELL_GCM_TEXTURE_UN;
		}
		*remap = CELL_GCM_TEXTURE_REMAP_REMAP << 14 |
			CELL_GCM_TEXTURE_REMAP_REMAP << 12 |
			CELL_GCM_TEXTURE_REMAP_REMAP << 10 |
			CELL_GCM_TEXTURE_REMAP_REMAP << 8 |
			CELL_GCM_TEXTURE_REMAP_FROM_A << 6 |
			CELL_GCM_TEXTURE_REMAP_FROM_R << 4 |
			CELL_GCM_TEXTURE_REMAP_FROM_G << 2 |
			CELL_GCM_TEXTURE_REMAP_FROM_B;
		break;
	case CELL_GCM_UTIL_DXT1: // GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
		*gcmFormat = CELL_GCM_TEXTURE_COMPRESSED_DXT1 |
			CELL_GCM_TEXTURE_SZ | CELL_GCM_TEXTURE_NR;
		*remap = CELL_GCM_TEXTURE_REMAP_REMAP << 14 |
			CELL_GCM_TEXTURE_REMAP_REMAP << 12 |
			CELL_GCM_TEXTURE_REMAP_REMAP << 10 |
			CELL_GCM_TEXTURE_REMAP_REMAP << 8 |
			CELL_GCM_TEXTURE_REMAP_FROM_B << 6 |
			CELL_GCM_TEXTURE_REMAP_FROM_G << 4 |
			CELL_GCM_TEXTURE_REMAP_FROM_R << 2 |
			CELL_GCM_TEXTURE_REMAP_FROM_A;
		break;
	case CELL_GCM_UTIL_DXT3: // GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
		*gcmFormat = CELL_GCM_TEXTURE_COMPRESSED_DXT23 |
			CELL_GCM_TEXTURE_SZ | CELL_GCM_TEXTURE_NR;
		*remap = CELL_GCM_TEXTURE_REMAP_REMAP << 14 |
			CELL_GCM_TEXTURE_REMAP_REMAP << 12 |
			CELL_GCM_TEXTURE_REMAP_REMAP << 10 |
			CELL_GCM_TEXTURE_REMAP_REMAP << 8 |
			CELL_GCM_TEXTURE_REMAP_FROM_B << 6 |
			CELL_GCM_TEXTURE_REMAP_FROM_G << 4 |
			CELL_GCM_TEXTURE_REMAP_FROM_R << 2 |
			CELL_GCM_TEXTURE_REMAP_FROM_A;
		break;
	case CELL_GCM_UTIL_DXT5: // GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
		*gcmFormat = CELL_GCM_TEXTURE_COMPRESSED_DXT45 |
			CELL_GCM_TEXTURE_SZ | CELL_GCM_TEXTURE_NR;
		*remap = CELL_GCM_TEXTURE_REMAP_REMAP << 14 |
			CELL_GCM_TEXTURE_REMAP_REMAP << 12 |
			CELL_GCM_TEXTURE_REMAP_REMAP << 10 |
			CELL_GCM_TEXTURE_REMAP_REMAP << 8 |
			CELL_GCM_TEXTURE_REMAP_FROM_B << 6 |
			CELL_GCM_TEXTURE_REMAP_FROM_G << 4 |
			CELL_GCM_TEXTURE_REMAP_FROM_R << 2 |
			CELL_GCM_TEXTURE_REMAP_FROM_A;
		break;
	case CELL_GCM_UTIL_FP16: // GL_RGBA16F_ARB
	case CELL_GCM_UTIL_FP32: // GL_RGBA32F_ARB
		if (glFormat == CELL_GCM_UTIL_FP16) {
			*gcmFormat =  CELL_GCM_TEXTURE_W16_Z16_Y16_X16_FLOAT;
		}
		else {
			*gcmFormat =  CELL_GCM_TEXTURE_W32_Z32_Y32_X32_FLOAT;
		}
		if (swizzle == 0) {
			*gcmFormat |= CELL_GCM_TEXTURE_LN;
		}
		if (normalize == 0) {
			*gcmFormat |= CELL_GCM_TEXTURE_UN;
		}
		*remap = CELL_GCM_TEXTURE_REMAP_REMAP << 14 |
			CELL_GCM_TEXTURE_REMAP_REMAP << 12 |
			CELL_GCM_TEXTURE_REMAP_REMAP << 10 |
			CELL_GCM_TEXTURE_REMAP_REMAP << 8 |
			CELL_GCM_TEXTURE_REMAP_FROM_B << 6 |
			CELL_GCM_TEXTURE_REMAP_FROM_G << 4 |
			CELL_GCM_TEXTURE_REMAP_FROM_R << 2 |
			CELL_GCM_TEXTURE_REMAP_FROM_A;
		break;
	default:
		printf("format:%x\n", glFormat);
		assert(false); // invalid texture format
		break;
	}

	return 0;
}
