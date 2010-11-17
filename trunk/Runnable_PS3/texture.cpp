/*  SCE CONFIDENTIAL
 *  PlayStation(R)3 Programmer Tool Runtime Library 330.001
 *  Copyright (C) 2010 Sony Computer Entertainment Inc.
 *  All Rights Reserved.
 */

#define __CELL_ASSERT__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <cell/sysmodule.h>

// libgcm
#include <cell/gcm.h>
#include <sysutil/sysutil_sysparam.h>

// for file
#include <cell/cell_fs.h>
#include <sys/paths.h>

// gtfloader
#include "gtfloader.h"

// gcmutil
#include "gcmutil.h"
using namespace CellGcmUtil;

using namespace cell::Gcm;

namespace{
	const uint32_t CELL_GCM_TEXTURE_COMPRESSED_B8R8_G8R8_RAW = 0x8D;
	const uint32_t CELL_GCM_TEXTURE_COMPRESSED_R8B8_R8G8_RAW = 0x8E;

	uint8_t gtfGetRawFormat(uint8_t texture_format)
	{
		return texture_format & ~(CELL_GCM_TEXTURE_LN | CELL_GCM_TEXTURE_UN);
	}

	bool gtfIsSwizzle(uint8_t format)
	{
		return !(format & CELL_GCM_TEXTURE_LN);
	}

	uint32_t gtfGetPitch(uint8_t format, uint32_t width)
	{
		uint32_t depth = 0;
		uint32_t raw_format = gtfGetRawFormat(format);
		switch(raw_format){
			case CELL_GCM_TEXTURE_B8:
				depth = 1;
				break;
			case CELL_GCM_TEXTURE_A1R5G5B5:
			case CELL_GCM_TEXTURE_A4R4G4B4:
			case CELL_GCM_TEXTURE_R5G6B5:
			case CELL_GCM_TEXTURE_G8B8:
			case CELL_GCM_TEXTURE_R6G5B5:
			case CELL_GCM_TEXTURE_DEPTH16:
			case CELL_GCM_TEXTURE_DEPTH16_FLOAT:
			case CELL_GCM_TEXTURE_X16:
			case CELL_GCM_TEXTURE_D1R5G5B5:
			case CELL_GCM_TEXTURE_R5G5B5A1:
			case CELL_GCM_TEXTURE_COMPRESSED_HILO8:
			case CELL_GCM_TEXTURE_COMPRESSED_HILO_S8:
			case CELL_GCM_TEXTURE_COMPRESSED_B8R8_G8R8_RAW:
			case CELL_GCM_TEXTURE_COMPRESSED_R8B8_R8G8_RAW:
				depth = 2;
				break;
			case CELL_GCM_TEXTURE_A8R8G8B8:
			case CELL_GCM_TEXTURE_DEPTH24_D8:
			case CELL_GCM_TEXTURE_DEPTH24_D8_FLOAT:
			case CELL_GCM_TEXTURE_Y16_X16:
			case CELL_GCM_TEXTURE_X32_FLOAT:
			case CELL_GCM_TEXTURE_D8R8G8B8:
			case CELL_GCM_TEXTURE_Y16_X16_FLOAT:
				depth = 4;
				break;
			case CELL_GCM_TEXTURE_W16_Z16_Y16_X16_FLOAT:
				depth = 8;
				break;
			case CELL_GCM_TEXTURE_W32_Z32_Y32_X32_FLOAT:
				depth = 16;
				break;
			case CELL_GCM_TEXTURE_COMPRESSED_DXT1:
			case CELL_GCM_TEXTURE_COMPRESSED_DXT23:
			case CELL_GCM_TEXTURE_COMPRESSED_DXT45:
				depth = 0;
				break;
			default:
				depth = 4;
		}

		uint32_t pitch = width * depth;
		if(raw_format == CELL_GCM_TEXTURE_COMPRESSED_DXT1){
			pitch = ((width + 3) / 4) * 8;
		}else if(raw_format == CELL_GCM_TEXTURE_COMPRESSED_DXT23 || raw_format == CELL_GCM_TEXTURE_COMPRESSED_DXT45){
			pitch = ((width + 3) / 4) * 16;
		}else if(raw_format == CELL_GCM_TEXTURE_COMPRESSED_B8R8_G8R8_RAW ||
			raw_format == CELL_GCM_TEXTURE_COMPRESSED_R8B8_R8G8_RAW)
		{
			pitch = ((width + 1) / 2) * 4;
		}

		return pitch;
	}

	uint16_t gtfGetDepth(uint8_t format)
	{
		uint16_t depth = 0;
		uint8_t raw_format = gtfGetRawFormat(format);
		switch(raw_format){
			case CELL_GCM_TEXTURE_B8:
				depth = 1;
				break;
			case CELL_GCM_TEXTURE_A1R5G5B5:
			case CELL_GCM_TEXTURE_A4R4G4B4:
			case CELL_GCM_TEXTURE_R5G6B5:
			case CELL_GCM_TEXTURE_G8B8:
			case CELL_GCM_TEXTURE_R6G5B5:
			case CELL_GCM_TEXTURE_DEPTH16:
			case CELL_GCM_TEXTURE_DEPTH16_FLOAT:
			case CELL_GCM_TEXTURE_X16:
			case CELL_GCM_TEXTURE_D1R5G5B5:
			case CELL_GCM_TEXTURE_R5G5B5A1:
			case CELL_GCM_TEXTURE_COMPRESSED_HILO8:
			case CELL_GCM_TEXTURE_COMPRESSED_HILO_S8:
			case CELL_GCM_TEXTURE_COMPRESSED_B8R8_G8R8_RAW:
			case CELL_GCM_TEXTURE_COMPRESSED_R8B8_R8G8_RAW:
				depth = 2;
				break;
			case CELL_GCM_TEXTURE_A8R8G8B8:
			case CELL_GCM_TEXTURE_DEPTH24_D8:
			case CELL_GCM_TEXTURE_DEPTH24_D8_FLOAT:
			case CELL_GCM_TEXTURE_Y16_X16:
			case CELL_GCM_TEXTURE_X32_FLOAT:
			case CELL_GCM_TEXTURE_D8R8G8B8:
			case CELL_GCM_TEXTURE_Y16_X16_FLOAT:
				depth = 4;
				break;
			case CELL_GCM_TEXTURE_W16_Z16_Y16_X16_FLOAT:
				depth = 8;
				break;
			case CELL_GCM_TEXTURE_W32_Z32_Y32_X32_FLOAT:
				depth = 16;
				break;
			case CELL_GCM_TEXTURE_COMPRESSED_DXT1:
				depth = 8;
				break;
			case CELL_GCM_TEXTURE_COMPRESSED_DXT23:
			case CELL_GCM_TEXTURE_COMPRESSED_DXT45:
				depth = 16;
				break;
			default:
				depth = 4;
		}

		return depth;
	}

	bool gtfIsDxtn(uint8_t format)
	{
		uint8_t raw_format = gtfGetRawFormat(format);
		return (raw_format == CELL_GCM_TEXTURE_COMPRESSED_DXT1 || 
				raw_format == CELL_GCM_TEXTURE_COMPRESSED_DXT23 || 
				raw_format == CELL_GCM_TEXTURE_COMPRESSED_DXT45);
	}

	uint32_t gtfConvFormatSurfaceToTexture(uint32_t surface_format)
	{
		uint32_t fmt = 0;
		switch(surface_format){
			case CELL_GCM_SURFACE_X1R5G5B5_Z1R5G5B5:
			case CELL_GCM_SURFACE_X1R5G5B5_O1R5G5B5:
				fmt = CELL_GCM_TEXTURE_D1R5G5B5;
				break;
			case CELL_GCM_SURFACE_R5G6B5:
				fmt = CELL_GCM_TEXTURE_R5G6B5;
				break;
			case CELL_GCM_SURFACE_B8:
				fmt  =CELL_GCM_TEXTURE_B8;
				break;
			case CELL_GCM_SURFACE_G8B8:
				fmt = CELL_GCM_TEXTURE_G8B8;
				break;
			case CELL_GCM_SURFACE_F_W16Z16Y16X16:
				fmt = CELL_GCM_TEXTURE_W16_Z16_Y16_X16_FLOAT;
				break;
			case CELL_GCM_SURFACE_F_W32Z32Y32X32:
				fmt = CELL_GCM_TEXTURE_W32_Z32_Y32_X32_FLOAT;
				break;
			case CELL_GCM_SURFACE_F_X32:
				fmt = CELL_GCM_TEXTURE_X32_FLOAT;
				break;
			case CELL_GCM_SURFACE_X8B8G8R8_Z8B8G8R8:
			case CELL_GCM_SURFACE_X8B8G8R8_O8B8G8R8:
				fmt = CELL_GCM_TEXTURE_D8R8G8B8;
				break;
			case CELL_GCM_SURFACE_A8B8G8R8:
			default:
				fmt = CELL_GCM_TEXTURE_A8R8G8B8;
		}
		return fmt;
	}

	uint32_t gtfConvFormatDepthToTexture(uint32_t depth_format, bool bAsColor)
	{
		if(bAsColor){
			if(depth_format == CELL_GCM_SURFACE_Z16) return CELL_GCM_TEXTURE_A4R4G4B4;
			if(depth_format == CELL_GCM_SURFACE_Z24S8) return CELL_GCM_TEXTURE_A8R8G8B8;
		}else{
			if(depth_format == CELL_GCM_SURFACE_Z16) return CELL_GCM_TEXTURE_DEPTH16;
			if(depth_format == CELL_GCM_SURFACE_Z24S8) return CELL_GCM_TEXTURE_DEPTH24_D8;
		}
		return 0;
	}

	uint32_t gtfConvFormatTextureToSurface(uint32_t format)
	{
		uint32_t fmt = 0;
		uint32_t raw_format = gtfGetRawFormat(format);
		switch(raw_format){
			case CELL_GCM_TEXTURE_D1R5G5B5:
				fmt = CELL_GCM_SURFACE_X1R5G5B5_O1R5G5B5;
				break;
			case CELL_GCM_TEXTURE_R5G6B5:
				fmt = CELL_GCM_SURFACE_R5G6B5;
				break;
			case CELL_GCM_TEXTURE_B8:
				fmt = CELL_GCM_SURFACE_B8;
				break;
			case CELL_GCM_TEXTURE_G8B8:
				fmt = CELL_GCM_SURFACE_G8B8;
				break;
			case CELL_GCM_TEXTURE_W16_Z16_Y16_X16_FLOAT:
				fmt = CELL_GCM_SURFACE_F_W16Z16Y16X16;
				break;
			case CELL_GCM_TEXTURE_W32_Z32_Y32_X32_FLOAT:
				fmt = CELL_GCM_SURFACE_F_W32Z32Y32X32;
				break;
			case CELL_GCM_TEXTURE_X32_FLOAT:
				fmt = CELL_GCM_SURFACE_F_X32;
				break;
			case CELL_GCM_TEXTURE_D8R8G8B8:
				fmt = CELL_GCM_SURFACE_X8B8G8R8_O8B8G8R8;
				break;
			case CELL_GCM_TEXTURE_A8R8G8B8:
			default:
				fmt = CELL_GCM_SURFACE_A8R8G8B8;
		}
		return fmt;
	}

	uint32_t gtfConvFormatTextureToDepth(uint32_t format)
	{
		uint32_t depth = gtfGetDepth(format);
		if(depth == 2) return CELL_GCM_SURFACE_Z16;
		if(depth == 4) return CELL_GCM_SURFACE_Z24S8;
		return 0;
	}

	int32_t tileGetFirstUnboundIndex(const CellGcmTileInfo *raw_tile_info, uint8_t *index)
	{
		assert(raw_tile_info);
		assert(index);

		if(raw_tile_info == 0) return CELL_GCM_ERROR_INVALID_VALUE;
		if(index == 0) return CELL_GCM_ERROR_INVALID_VALUE;

		for(uint8_t i = 0; i < CELL_GCM_MAX_TILE_INDEX; ++i)
		{
			//J CellGcmUtilUnpackedTileInfoに変換
			CellGcmUtilUnpackedTileInfo tile_info;
			cellGcmUtilUnpackTileInfo(i, raw_tile_info, &tile_info);

			//J 設定が有効な場合は繰り返し
			if(tile_info.enabled != 0) continue;

			//J 空きを発見
			*index = i;
			return CELL_OK;
		}

		//J 空きが無かった
		return CELL_GCM_ERROR_FAILURE;
	}

	int32_t tileTryGetCompressionTagBaseAddr(const CellGcmTileInfo *raw_tile_info, uint32_t size, uint16_t *comp_base)
	{
		const uint16_t COMP_MAX = 0x07FF;
		const uint32_t COMP_SHIFT = 16;

		//J エラーチェック
		assert(raw_tile_info);
		if(raw_tile_info == 0) return CELL_GCM_ERROR_INVALID_VALUE;

		assert(comp_base);
		if(comp_base == 0) return CELL_GCM_ERROR_INVALID_VALUE;

		assert((size & 0x0000FFFF) == 0);
		if(size & 0x0000FFFF) return CELL_GCM_ERROR_INVALID_VALUE;

		uint16_t tag_size = (size >> COMP_SHIFT);

		assert(tag_size <= COMP_MAX);
		if(tag_size > COMP_MAX) return CELL_GCM_ERROR_INVALID_VALUE;

		//J comp_base 情報の構造体（この関数でしか使用しない）
		struct comp_info_t{
			uint16_t base;
			uint16_t limit;
		} comp_info[CELL_GCM_MAX_TILE_INDEX];
		std::memset(comp_info, 0, sizeof(comp_info));


		//J comp_base 情報を収集
		int count = 0;

		for(uint8_t i = 0; i < CELL_GCM_MAX_TILE_INDEX; ++i)
		{
			//J CellGcmUtilUnpackedTileInfoに変換
			CellGcmUtilUnpackedTileInfo tile_info;
			cellGcmUtilUnpackTileInfo(i, raw_tile_info, &tile_info);

			//J 設定が無効な場合は繰り返し
			if(tile_info.enabled == 0) continue;

			comp_info[count].base = tile_info.base;
			comp_info[count].limit = tile_info.limit;
			++count;
		}

		//J Tile のスロットが全て埋まっている場合
		if(static_cast<uint32_t>(count) == CELL_GCM_MAX_TILE_INDEX)
		{
			return CELL_GCM_ERROR_FAILURE;
		}

		//J 未登録なので先頭を返す
		if(count == 0)
		{
			*comp_base = 0;
			return CELL_OK;
		}

		//J base_tag 順にソート
		for(int j = count - 2; j >= 0; j--){
			for(int i = 0; i <= j; i++){
				if(comp_info[i].base > comp_info[i+1].base){
					comp_info_t tmp_data = comp_info[i];
					comp_info[i] = comp_info[i+1];
					comp_info[i+1] = tmp_data;
				}
			}
		}

		//J 番兵を追加（次で行う挿入可能位置の探索用）
		comp_info[count].base = COMP_MAX;
		comp_info[count].limit = COMP_MAX;
		++count;
		
		//J 挿入可能な位置を探索
		for(uint8_t i = 0; i < count - 1; ++i)
		{
			uint16_t new_base = comp_info[i].limit + 1;
			uint16_t new_limit = new_base + tag_size - 1;

			//J 挿入不可
			if(new_limit > comp_info[i+1].base) continue;

			//J 挿入可
			*comp_base = new_base;
			return CELL_OK;
		}

		//J 挿入および追加が不可能だった
		return CELL_GCM_ERROR_FAILURE;
	}

	int32_t tileGetIndexFromOffset(const CellGcmTileInfo *raw_tile_info, uint32_t offset, uint8_t *index)
	{
		assert(raw_tile_info);
		assert(index);

		if(raw_tile_info == 0) return CELL_GCM_ERROR_INVALID_VALUE;
		if(index == 0) return CELL_GCM_ERROR_INVALID_VALUE;

		for(uint8_t i = 0; i < CELL_GCM_MAX_TILE_INDEX; ++i)
		{
			//J CellGcmUtilUnpackedTileInfoに変換
			CellGcmUtilUnpackedTileInfo tile_info;
			cellGcmUtilUnpackTileInfo(i, raw_tile_info, &tile_info);

			//J 設定が無効な場合は繰り返し
			if(tile_info.enabled == 0) continue;

			//J 該当するoffsetでない場合は繰り返し
			if(offset != tile_info.offset) continue;

			//J 該当するoffsetを発見
			*index = i;
			return CELL_OK;
		}

		//J 発見できなかった
		return CELL_GCM_ERROR_FAILURE;
	}

	int tileSetInfoHelper(
		uint32_t *arrayWidthInSample,
		uint32_t *arrayHeightInSample,
		int count,
		uint32_t bytePerSample,

		uint8_t location,

		uint32_t *outPitch,
		uint32_t *outAlign,
		uint32_t *outSize,
		uint32_t *outOffsetList
	)
	{
		//J 幅の最大値
		uint32_t max_width = 0;
		for(int i = 0; i < count; ++i)
		{
			max_width = (arrayWidthInSample[i] > max_width? arrayWidthInSample[i]: max_width);
		}

		uint32_t pitch = cellGcmGetTiledPitchSize(max_width * bytePerSample);

		//J 各バッファの先頭オフセットを pitch * 8 に揃える
		uint32_t offset = 0;
		uint32_t prev_size = 0;
		for(uint8_t i = 0; i < count; ++i)
		{
			offset += cellGcmAlign(pitch * 8, prev_size);
			prev_size = pitch * arrayHeightInSample[i];
			if(outOffsetList) outOffsetList[i] = offset;
		}
		uint32_t total_size = offset + prev_size;

		uint32_t LINE_ALIGN = CELL_GCMUTIL_TILE_ALIGN_HEIGHT(location);
		uint32_t line = cellGcmAlign(LINE_ALIGN, total_size / pitch);
		uint32_t size = cellGcmAlign(CELL_GCM_TILE_ALIGN_SIZE, pitch * line);

		if(outPitch) *outPitch = pitch;
		if(outAlign) *outAlign = CELL_GCM_TILE_ALIGN_OFFSET;
		if(outSize) *outSize = size;

		return CELL_OK;
	}

	int32_t zcullGetFirstUnboundIndex(const CellGcmZcullInfo *raw_zcull_info, uint8_t *index)
	{
		assert(raw_zcull_info);
		assert(index);

		if(raw_zcull_info == 0) return CELL_GCM_ERROR_INVALID_VALUE;
		if(index == 0) return CELL_GCM_ERROR_INVALID_VALUE;

		for(uint8_t i = 0; i < CELL_GCM_MAX_ZCULL_INDEX; ++i)
		{
			//J CellGcmUtilUnpackedTileInfoに変換
			CellGcmUtilUnpackedZcullInfo zcull_info;
			cellGcmUtilUnpackZcullInfo(i, raw_zcull_info, &zcull_info);

			//J 設定が有効な場合は繰り返し
			if(zcull_info.enabled != 0) continue;

			//J 空きを発見
			*index = i;
			return CELL_OK;
		}

		//J 空きが無かった
		return CELL_GCM_ERROR_FAILURE;
	}

	int32_t zcullTryGetRamStart(const CellGcmZcullInfo *raw_zcull_info, uint32_t size, uint32_t *cullStart)
	{
		const int ZCULL_RAM_MAX = 2048 * 1536;

		assert(raw_zcull_info);
		if(raw_zcull_info == 0) return CELL_GCM_ERROR_INVALID_VALUE;

		assert(cullStart);
		if(cullStart == 0) return CELL_GCM_ERROR_INVALID_VALUE;

		assert((size & 0x00000FFF) == 0);
		if(size & 0x00000FFF) return CELL_GCM_ERROR_INVALID_VALUE;

		struct zcull_ram_info_t{
			uint32_t start;
			uint32_t size;
		} ram_info[CELL_GCM_MAX_ZCULL_INDEX];
		std::memset(ram_info, 0, sizeof(ram_info));

		//J Zcull RAMの情報を収集
		int count = 0;
		for(uint8_t i = 0; i < CELL_GCM_MAX_ZCULL_INDEX; ++i)
		{
			//J CellGcmUtilUnpackedTileInfoに変換
			CellGcmUtilUnpackedZcullInfo zcull_info;
			cellGcmUtilUnpackZcullInfo(i, raw_zcull_info, &zcull_info);

			//J 設定が無効な場合は繰り返し
			if(zcull_info.enabled == 0) continue;
			
			ram_info[count].start = zcull_info.cullStart;
			ram_info[count].size =zcull_info.width * zcull_info.height;
			++count;
		}

		if(static_cast<uint32_t>(count) == CELL_GCM_MAX_ZCULL_INDEX)
		{
			return CELL_GCM_ERROR_FAILURE;
		}

		//J 未登録なので先頭を返す
		if(count == 0)
		{
			*cullStart = 0;
			return CELL_OK;
		}

		//J start順にソート
		for(int j = count - 2; j >= 0; j--){
			for(int i = 0; i <= j; i++){
				if(ram_info[i].start > ram_info[i+1].start){
					zcull_ram_info_t tmp_data = ram_info[i];
					ram_info[i] = ram_info[i+1];
					ram_info[i+1] = tmp_data;
				}
			}
		}

		//J 番兵を追加（次で行う挿入可能位置の探索用）
		ram_info[count].start = ZCULL_RAM_MAX;
		ram_info[count].size = ZCULL_RAM_MAX;
		++count;
		
		//J 挿入可能な位置を探索
		for(uint8_t i = 0; i < count - 1; ++i)
		{
			uint16_t new_start = ram_info[i].start + ram_info[i].size;

			//J 挿入不可
			if(new_start + size > ram_info[i+1].start) continue;

			//J 挿入可
			*cullStart = new_start;
			return CELL_OK;
		}

		//J 挿入および追加が不可能だった
		return CELL_GCM_ERROR_FAILURE;
	}

	int32_t zcullGetIndexFromOffset(const CellGcmZcullInfo *raw_zcull_info, uint32_t offset, uint8_t *index)
	{
		assert(raw_zcull_info);
		assert(index);

		if(raw_zcull_info == 0) return CELL_GCM_ERROR_INVALID_VALUE;
		if(index == 0) return CELL_GCM_ERROR_INVALID_VALUE;

		for(uint8_t i = 0; i < CELL_GCM_MAX_ZCULL_INDEX; ++i)
		{
			//J 設定が無効な場合は繰り返し
			if(raw_zcull_info[i].region == 0) continue;

			//J 該当するoffsetでない場合は繰り返し
			if(offset != raw_zcull_info[i].offset) continue;

			//J 該当するoffsetを発見
			*index = i;
			return CELL_OK;
		}

		//J 発見できなかった
		return CELL_GCM_ERROR_FAILURE;
	}

} // no name namespace

namespace CellGcmUtil{

bool cellGcmUtilLoadTexture(const char* fname, uint32_t location, CellGcmTexture *texture, Memory_t *image)
{
	if(fname == 0) return false;

	uint32_t size = 0;
	uint8_t *buffer = 0;

	if(cellGcmUtilReadFile(fname, &buffer, &size) == false){
		return false;
	}

	uint32_t rq_size = cellGtfGetTextureSizeFromMemory(buffer, size);
	if(rq_size == 0 || cellGcmUtilAllocate(rq_size, 128, location, image) == false){
		delete [] buffer, buffer = 0;
		size = 0;
		printf("cellGcmUtilLoadTexture() failed. Invalid texture format / size: %s\n", fname);
		return false;
	}

	CELL_GTF_RESULT ret = cellGtfLoadTextureFromMemory(buffer, size, texture, location, image->addr);
	if(ret != CELL_GTF_OK){
		printf("cellGcmUtilLoadTexture() failed. Invalid texture format: %s\n", fname);
		cellGcmUtilFree(image);
	}

	delete [] buffer, buffer = 0;
	size = 0;

	return (ret == CELL_GTF_OK);
}

bool cellGcmUtilLoadTextureFromMemory(const unsigned char *buffer, uint32_t size, uint32_t location, CellGcmTexture *texture, Memory_t *image)
{
	uint32_t rq_size = cellGtfGetTextureSizeFromMemory(buffer, size);
	if(rq_size == 0 || cellGcmUtilAllocate(rq_size, 128, location, image) == false){
		delete [] buffer, buffer = 0;
		size = 0;
		printf("cellGcmUtilLoadTexture() failed. Invalid texture format / size.\n");
		return false;
	}

	CELL_GTF_RESULT ret = cellGtfLoadTextureFromMemory(buffer, size, texture, location, image->addr);
	if(ret != CELL_GTF_OK){
		printf("cellGcmUtilLoadTextureFromMemory() failed. Invalid texture format.\n");
		cellGcmUtilFree(image);
	}

	return (ret == CELL_GTF_OK);
}

uint32_t cellGcmUtilLoadPackedTexture(const char* fname, uint32_t index, uint32_t location, CellGcmTexture *texture, Memory_t *image)
{
	if(fname == 0) return false;

	uint32_t size = 0;
	uint8_t *buffer = 0;

	if(cellGcmUtilReadFile(fname, &buffer, &size) == false){
		return 0;
	}

	uint32_t pack_num = cellGtfGetPackedTextureNumberFromMemory(buffer, size);
	if(pack_num <= index){
		delete [] buffer, buffer = 0;
		size = 0;
		printf("cellGcmUtilLoadPackedTexture() failed. Invalid texture index / format: %s\n", fname);
		return pack_num;
	}

	uint32_t rq_size = cellGtfGetPackedTextureSizeFromMemory(buffer, size, index);
	if(rq_size == 0 || cellGcmUtilAllocate(rq_size, 128, location, image) == false){
		delete [] buffer, buffer = 0;
		size = 0;
		printf("cellGcmUtilLoadPackedTexture() failed. Invalid texture index / format / size: %s\n", fname);
		return pack_num;
	}

	CELL_GTF_RESULT ret = cellGtfLoadPackedTextureFromMemory(buffer, size, index, texture, location, image->addr);
	if(ret != CELL_GTF_OK){
		printf("cellGcmUtilLoadPackedTexture() failed. Invalid texture format: %s\n", fname);
		cellGcmUtilFree(image);
	}

	delete [] buffer, buffer = 0;
	size = 0;

	return pack_num;
}

uint32_t cellGcmUtilGetTextureRequireSize(const CellGcmTexture tex)
{
	uint8_t raw_format = gtfGetRawFormat(tex.format);
	bool is_dxt = gtfIsDxtn(raw_format);
	bool is_comp = (raw_format == CELL_GCM_TEXTURE_COMPRESSED_B8R8_G8R8_RAW || 
				raw_format == CELL_GCM_TEXTURE_COMPRESSED_R8B8_R8G8_RAW);

	uint16_t width = tex.width;
	uint16_t height = tex.height;
	uint16_t depth = tex.depth;
	uint32_t gtf_pitch = tex.pitch;
	uint8_t cube = 1;
	if(tex.cubemap == CELL_GCM_TRUE){
		cube = 6;
	}

	bool is_cube = (cube == 6);
	bool is_volume = (depth > 1);
	bool is_mip = (tex.mipmap > 1);

	if(gtf_pitch == 0){
        gtf_pitch = gtfGetPitch(raw_format, width);
	}
	uint16_t color_depth = gtfGetDepth(raw_format);

	uint32_t latest_gtf_linear_offset = 0;
	uint32_t latest_gtf_linear_size = 0;
	uint32_t latest_gtf_swizzle_offset = 0;
	uint32_t latest_gtf_swizzle_size = 0;

	// cubemap
	for(uint32_t n = 0; n < cube; ++n){
		bool new_face = true;
		uint16_t w = width;
		uint16_t h = height;
		uint16_t v = depth;
		uint16_t m = 0;
		
		// mipmap
		while( m < tex.mipmap ){
	
			uint32_t gtf_swizzle_size;
			uint32_t gtf_linear_size;

			if(is_dxt){
				// dxtn
				gtf_swizzle_size = ((w + 3) / 4) * ((h + 3) / 4) * color_depth;
			}else if(is_comp){
				// B8R8_G8R8, R8B8_R8G8
				gtf_swizzle_size = ((w + 1) / 2) * h * 4;
			}else{
				gtf_swizzle_size = w * h * color_depth;
			}

			// linear gtf size
			if(is_dxt){
				// not power of 2 dxtn
				gtf_linear_size = ((h + 3) / 4) * gtf_pitch;
			}else{
				gtf_linear_size = h * gtf_pitch;
			}

			// volume
			if(is_volume){
				gtf_swizzle_size *= v;
				gtf_linear_size *= v;
			}

			// offset
			uint32_t gtf_swizzle_offset = latest_gtf_swizzle_offset + latest_gtf_swizzle_size;
			if(is_cube && new_face){
				// when swizzle cubemap, each face must be aligned on a 128-byte boundary
				gtf_swizzle_offset = cellGcmAlign(CELL_GCM_TEXTURE_SWIZZLED_CUBEMAP_FACE_ALIGN_OFFSET, gtf_swizzle_offset);
				new_face = false;
			}
			uint32_t gtf_linear_offset = latest_gtf_linear_offset + latest_gtf_linear_size;

			// update
			latest_gtf_linear_offset = gtf_linear_offset;
			latest_gtf_linear_size = gtf_linear_size;
			latest_gtf_swizzle_offset = gtf_swizzle_offset;
			latest_gtf_swizzle_size = gtf_swizzle_size;
			//printf("[%d] %dx%d ln=%d sw=%d\n", m, w, h, latest_gtf_linear_size, latest_gtf_swizzle_size);


			// next miplevel
			if(!is_mip)	break;

			w >>= 1;
			h >>= 1;
			v >>= 1;
			if(w == 0 && h == 0 && v == 0) break;
			if(w == 0) w = 1;
			if(h == 0) h = 1;
			if(v == 0) v = 1;
			++m;
		}
	}

	if(gtfIsSwizzle(tex.format)){
		return latest_gtf_swizzle_offset + latest_gtf_swizzle_size;
	}else{
		return latest_gtf_linear_offset + latest_gtf_linear_size;
	}
}

bool cellGcmUtilCreateTexture(CellGcmTexture *texture, Memory_t *image)
{
	if(texture == 0) return false;
	if(image == 0) return false;

	uint32_t size = cellGcmUtilGetTextureRequireSize(*texture);

	if(cellGcmUtilAllocate(size, 128, texture->location, image) == false)
	{
		printf("cellGcmUtilCreateTexture() failed. Cannot allocate memory (size=0x%x)\n", size);
		return false;
	}

	memset(image->addr, 0, size);

	texture->location = image->location;
	texture->offset = image->offset;

	return true;
}

bool cellGcmUtilCreateSimpleTexture(uint32_t width, uint32_t height, uint8_t location, CellGcmTexture *texture, Memory_t *image)
{
	if(texture == 0) return false;
	if(image == 0) return false;

	CellGcmTexture tex;
	tex.format = CELL_GCM_TEXTURE_A8R8G8B8 | CELL_GCM_TEXTURE_LN;
	tex.mipmap = 1;
	tex.dimension = CELL_GCM_TEXTURE_DIMENSION_2;
	tex.cubemap = CELL_GCM_FALSE;
	tex.remap = cellGcmUtilStr2Remap("ARGB");
	tex.width = width;
	tex.height = height;
	tex.depth = 1;
	tex.pitch = width * 4;
	tex.location = location;
	tex.offset = 0;

	Memory_t img;

	if(cellGcmUtilCreateTexture(&tex, &img) == false){
		return false;
	}

	*texture = tex;
	*image = img;

	return true;
}

bool cellGcmUtilCreateTiledTexture(uint32_t width, uint32_t height, uint8_t format, uint8_t location, uint32_t comp_mode, uint32_t number, CellGcmTexture *texture_array, Memory_t *buffer)
{
	const uint32_t SHARE_NUM_MAX = 16;
	uint32_t arrayWidth[SHARE_NUM_MAX];
	uint32_t arrayHeight[SHARE_NUM_MAX];

	uint8_t index = 0;
	if(tileGetFirstUnboundIndex(cellGcmGetTileInfo(), &index) != CELL_OK) return false;
	if(number > SHARE_NUM_MAX) return false;

	for(uint32_t i = 0; i < number; ++i){
		arrayWidth[i] = width;
		arrayHeight[i] = height;
	}

	uint32_t offset_list[SHARE_NUM_MAX];

	uint32_t pitch = 0;
	uint32_t align = 0;
	uint32_t size = 0;
	int ret = tileSetInfoHelper(arrayWidth, arrayHeight, number, gtfGetDepth(format), location, &pitch, &align, &size, offset_list);
	if(ret != CELL_OK)
	{
		return false;
	}

	CellGcmTexture texture;
	texture.format = cellGcmUtilGetRawFormat(format) | CELL_GCM_TEXTURE_LN;
	texture.mipmap = 1;
	texture.dimension = CELL_GCM_TEXTURE_DIMENSION_2;
	texture.cubemap = CELL_GCM_FALSE;
	texture.remap = cellGcmUtilStr2Remap("ARGB");
	texture.width = width;
	texture.height = height;
	texture.depth = 1;
	texture.pitch = pitch;
	texture.location = location;
	texture.offset = 0;
	texture._padding = 0;

	if(cellGcmUtilAllocate(size, align, location, buffer) == false){
		return false;
	}

	uint16_t comp_base = 0;
	if(comp_mode != CELL_GCM_COMPMODE_DISABLED){
		if(tileTryGetCompressionTagBaseAddr(cellGcmGetTileInfo(), size, &comp_base) != CELL_OK)
		{
			//J 失敗
			comp_mode = CELL_GCM_COMPMODE_DISABLED;
		}
	}

	uint32_t tile_bank = (comp_mode >= CELL_GCM_COMPMODE_Z32_SEPSTENCIL? 2: 0);

	int retSetTile = cellGcmSetTileInfo(index, location, buffer->offset, size, pitch, comp_mode, comp_base, tile_bank);
	int retBindTile = cellGcmBindTile(index);
	if(retSetTile != CELL_OK || retBindTile != CELL_OK)
	{
		cellGcmUtilFree(buffer);
		cellGcmUnbindTile(index);
		return false;
	}

	for(uint32_t i = 0; i < number; ++i)
	{
		texture_array[i] = texture;
		texture_array[i].offset = buffer->offset + offset_list[i];
	}

	return true;
}

bool cellGcmUtilCreateRenderTexture(uint32_t width, uint32_t height, uint8_t format, uint8_t location, uint32_t comp_mode, CellGcmTexture *texture, Memory_t *image)
{
	return cellGcmUtilCreateTiledTexture(width, height, format, location, comp_mode, 1, texture, image);
}

bool cellGcmUtilCreateDepthTexture(uint32_t width, uint32_t height, uint8_t format, uint8_t location, uint32_t msaa_mode, bool bZCull, bool bTile, CellGcmTexture *texture, Memory_t *image)
{
	if(texture == 0) return false;
	if(image == 0) return false;

	CellGcmTexture &tex = *texture;
	tex.format = cellGcmUtilGetRawFormat(format) | CELL_GCM_TEXTURE_LN;
	tex.mipmap = 1;
	tex.dimension = CELL_GCM_TEXTURE_DIMENSION_2;
	tex.cubemap = CELL_GCM_FALSE;
	tex.remap = cellGcmUtilStr2Remap("ARGB");
	tex.width = width;
	tex.height = height;
	tex.depth = 1;
	tex.pitch = width * gtfGetDepth(format);
	tex.location = location;
	tex.offset = 0;

	uint32_t comp_mode = CELL_GCM_COMPMODE_DISABLED;
	switch(msaa_mode){
		case CELL_GCM_SURFACE_CENTER_1:
			comp_mode = CELL_GCM_COMPMODE_Z32_SEPSTENCIL;
			break;
		case CELL_GCM_SURFACE_DIAGONAL_CENTERED_2:
			comp_mode = CELL_GCM_COMPMODE_Z32_SEPSTENCIL_DIAGONAL;
			break;
		case CELL_GCM_SURFACE_SQUARE_CENTERED_4:
			comp_mode = CELL_GCM_COMPMODE_Z32_SEPSTENCIL_REGULAR;
			break;
		case CELL_GCM_SURFACE_SQUARE_ROTATED_4:
			comp_mode = CELL_GCM_COMPMODE_Z32_SEPSTENCIL_ROTATED;
			break;
		default:
			comp_mode = CELL_GCM_COMPMODE_Z32_SEPSTENCIL_REGULAR;
	}

	uint8_t zcull_index = 0;
	if(bZCull){
		bZCull = (location == CELL_GCM_LOCATION_LOCAL);
		int ret = zcullGetFirstUnboundIndex(cellGcmGetZcullInfo(), &zcull_index);

		bZCull = (ret == CELL_OK);
		if(bZCull){
			tex.width = cellGcmAlign(CELL_GCM_ZCULL_ALIGN_WIDTH, tex.width);
			tex.height = cellGcmAlign(CELL_GCM_ZCULL_ALIGN_HEIGHT, tex.height);
			tex.pitch = tex.width * gtfGetDepth(format);
		}
	}

	bool bRet = false;
	if(bTile){
		bRet = cellGcmUtilCreateRenderTexture(tex.width, tex.height, format, location, comp_mode, texture, image);
	}

	if(!bRet){
		uint32_t size = cellGcmUtilGetTextureRequireSize(*texture);

		if(cellGcmUtilAllocate(size, CELL_GCM_ZCULL_ALIGN_OFFSET, texture->location, image)){
			memset(image->addr, 0, size);

			texture->location = image->location;
			texture->offset = image->offset;

			bRet = true;
		}
	}

	uint32_t cullStart = 0;
	if(bZCull){
		int ret = zcullTryGetRamStart(cellGcmGetZcullInfo(), tex.pitch * tex.height, &cullStart);
		bZCull = (ret == CELL_OK);
	}

	if(bRet && bZCull){
		uint8_t zcull_format = (gtfGetDepth(format) != 2? CELL_GCM_ZCULL_Z24S8: CELL_GCM_ZCULL_Z16);
		
		bRet = (cellGcmBindZcull(zcull_index, image->offset, tex.width, tex.height, cullStart,
				zcull_format, msaa_mode,
				CELL_GCM_ZCULL_LESS, CELL_GCM_ZCULL_LONES,
				CELL_GCM_SCULL_SFUNC_LESS, 0x80, 0xff) == CELL_OK);
	}

	tex.width = width;
	tex.height = height;

	return bRet;
}

uint8_t cellGcmUtilBindSharedTile(CellGcmTexture *texture, uint32_t comp_mode, uint32_t size)
{
	if(texture == 0) return CELL_GCM_MAX_TILE_INDEX;
	if(gtfIsSwizzle(texture->format)) return CELL_GCM_MAX_TILE_INDEX;
	if(texture->mipmap != 1) return CELL_GCM_MAX_TILE_INDEX;
	if(texture->dimension != CELL_GCM_TEXTURE_DIMENSION_2) return CELL_GCM_MAX_TILE_INDEX;
	if(texture->cubemap != CELL_GCM_FALSE) return CELL_GCM_MAX_TILE_INDEX;
	if(texture->depth != 1) return CELL_GCM_MAX_TILE_INDEX;
	if(texture->pitch != cellGcmGetTiledPitchSize(texture->pitch)) return CELL_GCM_MAX_TILE_INDEX;
	if(texture->offset & 0x0000FFFF) return CELL_GCM_MAX_TILE_INDEX;

	if(size == 0){
		uint32_t LINE_ALIGN = CELL_GCMUTIL_TILE_ALIGN_HEIGHT(texture->location);
		uint32_t line = cellGcmAlign(LINE_ALIGN, texture->height);
		size = line * texture->pitch;

		size = cellGcmAlign(CELL_GCM_TILE_ALIGN_SIZE, size);
	}

	uint8_t index = 0;
	if(tileGetFirstUnboundIndex(cellGcmGetTileInfo(), &index) != CELL_OK) return CELL_GCM_MAX_TILE_INDEX;

	uint16_t comp_base = 0;
	if(comp_mode != CELL_GCM_COMPMODE_DISABLED)
	{
		if(tileTryGetCompressionTagBaseAddr(cellGcmGetTileInfo(), size, &comp_base) != CELL_OK) return CELL_GCM_MAX_TILE_INDEX;
	} 

	uint32_t tile_bank = (comp_mode >= CELL_GCM_COMPMODE_Z32_SEPSTENCIL? 2: 0);
	
	uint32_t ret = 0;
	ret = cellGcmSetTileInfo(index, texture->location, texture->offset, size, texture->pitch, comp_mode, comp_base, tile_bank);
	if(ret != CELL_OK) return CELL_GCM_MAX_TILE_INDEX;
	ret = cellGcmBindTile(index);
	if(ret != CELL_OK) return CELL_GCM_MAX_TILE_INDEX;

	return index;
}

uint8_t cellGcmUtilBindTile(CellGcmTexture *texture, uint32_t comp_mode)
{
	return cellGcmUtilBindSharedTile(texture, comp_mode, 0);
}

uint32_t cellGcmUtilBindZCull(CellGcmTexture *texture)
{
	if(texture == 0) return CELL_GCM_MAX_ZCULL_INDEX;
	if(gtfIsSwizzle(texture->format)) return CELL_GCM_MAX_ZCULL_INDEX;
	if(texture->mipmap != 1) return CELL_GCM_MAX_ZCULL_INDEX;
	if(texture->dimension != CELL_GCM_TEXTURE_DIMENSION_2) return CELL_GCM_MAX_ZCULL_INDEX;
	if(texture->cubemap != CELL_GCM_FALSE) return CELL_GCM_MAX_ZCULL_INDEX;
	if(texture->depth != 1) return CELL_GCM_MAX_ZCULL_INDEX;
	if(texture->location != CELL_GCM_LOCATION_LOCAL) return CELL_GCM_MAX_ZCULL_INDEX;
	if(texture->offset & 0x0000FFFF) return CELL_GCM_MAX_ZCULL_INDEX;
	
	uint32_t width = cellGcmAlign(CELL_GCM_ZCULL_ALIGN_WIDTH, texture->width);
	uint32_t height = cellGcmAlign(CELL_GCM_ZCULL_ALIGN_HEIGHT, texture->height);
	uint32_t pitch = width * gtfGetDepth(texture->format);
	if(texture->pitch != pitch && texture->pitch != cellGcmGetTiledPitchSize(pitch)) return CELL_GCM_MAX_ZCULL_INDEX;

	const uint32_t ZCULL_MEMORY_LIMIT = CELL_GCM_ZCULL_RAM_SIZE_MAX * 4;
	if(texture->pitch * height >= ZCULL_MEMORY_LIMIT) return CELL_GCM_MAX_ZCULL_INDEX;

	uint8_t zcull_index = 0;
	if(zcullGetFirstUnboundIndex(cellGcmGetZcullInfo(), &zcull_index) != CELL_OK) return CELL_GCM_MAX_ZCULL_INDEX;

	uint8_t zcull_format = (gtfGetDepth(texture->format) != 2? CELL_GCM_ZCULL_Z24S8: CELL_GCM_ZCULL_Z16);

	uint32_t cullStart = 0;
	if(zcullTryGetRamStart(cellGcmGetZcullInfo(), texture->pitch * texture->height, &cullStart) != CELL_OK)
	{
		return CELL_GCM_MAX_ZCULL_INDEX;
	}

	if(cellGcmBindZcull(zcull_index, texture->offset, width, height, cullStart,
			zcull_format, CELL_GCM_SURFACE_CENTER_1,
			CELL_GCM_ZCULL_LESS, CELL_GCM_ZCULL_LONES,
			CELL_GCM_SCULL_SFUNC_LESS, 0x80, 0xff) != CELL_OK)
	{
		return CELL_GCM_MAX_ZCULL_INDEX;
	}

	return zcull_index;
}

void cellGcmUtilUnbindTile(CellGcmTexture *texture)
{
	if(texture == 0) return;
	
	uint8_t index = 0;
	int ret = tileGetIndexFromOffset(cellGcmGetTileInfo(), texture->offset, &index);
	if(ret == CELL_OK){
		cellGcmUnbindTile(index);
	}
}

void cellGcmUtilUnbindZCull(CellGcmTexture *texture)
{
	if(texture == 0) return;

	uint8_t index = 0;
	int ret = zcullGetIndexFromOffset(cellGcmGetZcullInfo(), texture->offset, &index);
	if(ret == CELL_OK){
		cellGcmUnbindZcull(index);
	}
}

void cellGcmUtilSetTextureUnit(uint32_t tex_unit, const CellGcmTexture *texture)
{
	if(texture == 0){
		cellGcmUtilInvalidateTextureUnit(tex_unit);
		return;
	}

	cellGcmSetInvalidateTextureCache(CELL_GCM_INVALIDATE_TEXTURE);
	
	// bind texture
	cellGcmSetTexture(tex_unit, texture);
	
	// bind texture and set filter
	cellGcmSetTextureControl(tex_unit, CELL_GCM_TRUE, 0<<8, 12<<8, CELL_GCM_TEXTURE_MAX_ANISO_1); // MIN:0,MAX:12
	
	cellGcmSetTextureAddress(tex_unit,
							 CELL_GCM_TEXTURE_MIRROR,
							 CELL_GCM_TEXTURE_MIRROR,
							 CELL_GCM_TEXTURE_MIRROR,
							 CELL_GCM_TEXTURE_UNSIGNED_REMAP_NORMAL,
							 CELL_GCM_TEXTURE_ZFUNC_LESS, 0);

	uint8_t min_filter = CELL_GCM_TEXTURE_NEAREST;
	uint8_t mag_filter = CELL_GCM_TEXTURE_NEAREST;

	if(texture->mipmap == 1){
		min_filter = CELL_GCM_TEXTURE_LINEAR;
		mag_filter = CELL_GCM_TEXTURE_LINEAR;
	}else{
		min_filter = CELL_GCM_TEXTURE_LINEAR_LINEAR;
		mag_filter = CELL_GCM_TEXTURE_LINEAR;
	}

	uint32_t raw_format = gtfGetRawFormat(texture->format);
	if(raw_format == CELL_GCM_TEXTURE_W32_Z32_Y32_X32_FLOAT || raw_format == CELL_GCM_TEXTURE_X32_FLOAT){
		min_filter = CELL_GCM_TEXTURE_NEAREST;
		mag_filter = CELL_GCM_TEXTURE_NEAREST;
	}

	cellGcmSetTextureFilter(tex_unit, 0, min_filter, mag_filter, CELL_GCM_TEXTURE_CONVOLUTION_QUINCUNX);
}

void cellGcmUtilInvalidateTextureUnit(uint32_t tex_unit)
{
	cellGcmSetTextureControl(tex_unit, CELL_GCM_FALSE, 0, 0, 0);
}

uint8_t cellGcmUtilGetRawFormat(uint8_t texture_format)
{
	return gtfGetRawFormat(texture_format);
}
const char* cellGcmUtilGetFormatName(uint8_t texture_format)
{
	const struct {
		uint8_t format;
		const char* string;
	} sFormatString[] = {
		{ 0xFF, "Unknown Format" },
		{ 0x81, "CELL_GCM_TEXTURE_B8" }, 
		{ 0x82, "CELL_GCM_TEXTURE_A1R5G5B5" }, 
		{ 0x83, "CELL_GCM_TEXTURE_A4R4G4B4" }, 
		{ 0x84, "CELL_GCM_TEXTURE_R5G6B5" }, 
		{ 0x85, "CELL_GCM_TEXTURE_A8R8G8B8" }, 
		{ 0x86, "CELL_GCM_TEXTURE_COMPRESSED_DXT1" }, 
		{ 0x87, "CELL_GCM_TEXTURE_COMPRESSED_DXT23" }, 
		{ 0x88, "CELL_GCM_TEXTURE_COMPRESSED_DXT45" }, 
		{ 0x8B, "CELL_GCM_TEXTURE_G8B8" }, 
		{ 0x8D, "CELL_GCM_TEXTURE_COMPRESSED_B8R8_G8R8" },
		{ 0x8E, "CELL_GCM_TEXTURE_COMPRESSED_R8B8_R8G8" },
		{ 0x8F, "CELL_GCM_TEXTURE_R6G5B5" }, 
		{ 0x90, "CELL_GCM_TEXTURE_DEPTH24_D8" }, 
		{ 0x91, "CELL_GCM_TEXTURE_DEPTH24_D8_FLOAT" }, 
		{ 0x92, "CELL_GCM_TEXTURE_DEPTH16" }, 
		{ 0x93, "CELL_GCM_TEXTURE_DEPTH16_FLOAT" }, 
		{ 0x94, "CELL_GCM_TEXTURE_X16" }, 
		{ 0x95, "CELL_GCM_TEXTURE_Y16_X16" }, 
		{ 0x97, "CELL_GCM_TEXTURE_R5G5B5A1" }, 
		{ 0x98, "CELL_GCM_TEXTURE_COMPRESSED_HILO8" }, 
		{ 0x99, "CELL_GCM_TEXTURE_COMPRESSED_HILO_S8" }, 
		{ 0x9A, "CELL_GCM_TEXTURE_W16_Z16_Y16_X16_FLOAT" }, 
		{ 0x9B, "CELL_GCM_TEXTURE_W32_Z32_Y32_X32_FLOAT" }, 
		{ 0x9C, "CELL_GCM_TEXTURE_X32_FLOAT" }, 
		{ 0x9D, "CELL_GCM_TEXTURE_D1R5G5B5" }, 
		{ 0x9E, "CELL_GCM_TEXTURE_D8R8G8B8" }, 
		{ 0x9F, "CELL_GCM_TEXTURE_Y16_X16_FLOAT" },
	};

	const int FORMAT_NUM = sizeof(sFormatString)/sizeof(sFormatString[0]);

	for(int i = 0; i < FORMAT_NUM; ++i){
		if(gtfGetRawFormat(texture_format) != sFormatString[i].format) continue;

		return &sFormatString[i].string[17];
	}

	return sFormatString[0].string;
}

uint16_t cellGcmUtilStr2Remap(const char*str)
{
	uint16_t remap_pass_throw  = 
				CELL_GCM_TEXTURE_REMAP_REMAP << 14 |
				CELL_GCM_TEXTURE_REMAP_REMAP << 12 |
				CELL_GCM_TEXTURE_REMAP_REMAP << 10 |
				CELL_GCM_TEXTURE_REMAP_REMAP << 8 |
				CELL_GCM_TEXTURE_REMAP_FROM_B << 6 |
				CELL_GCM_TEXTURE_REMAP_FROM_G << 4 |
				CELL_GCM_TEXTURE_REMAP_FROM_R << 2 |
				CELL_GCM_TEXTURE_REMAP_FROM_A;

	if(strlen(str) != 4) return remap_pass_throw;

	uint16_t remap = 0;
	for(int i = 0; i < 4; ++i){
		uint16_t pass_throw_color = CELL_GCM_TEXTURE_REMAP_FROM_A + i;
		uint16_t flag = (CELL_GCM_TEXTURE_REMAP_ONE << 8) | pass_throw_color;

		char c = str[i];
		if(c == '0'){
			flag = (CELL_GCM_TEXTURE_REMAP_ZERO << 8) | pass_throw_color;
		}else if(c == 'A'){
			flag = (CELL_GCM_TEXTURE_REMAP_REMAP << 8) | CELL_GCM_TEXTURE_REMAP_FROM_A;
		}else if(c == 'R'){
			flag = (CELL_GCM_TEXTURE_REMAP_REMAP << 8) | CELL_GCM_TEXTURE_REMAP_FROM_R;
		}else if(c == 'G'){
			flag = (CELL_GCM_TEXTURE_REMAP_REMAP << 8) | CELL_GCM_TEXTURE_REMAP_FROM_G;
		}else if(c == 'B'){
			flag = (CELL_GCM_TEXTURE_REMAP_REMAP << 8) | CELL_GCM_TEXTURE_REMAP_FROM_B;
		}

		remap |= flag << (i * 2);
	}
	
	return remap;
}

const char* cellGcmUtilRemap2Str(uint16_t remap)
{
	static char strRemap[5] = "ARGB";

	for(int i = 0; i < 4; ++i){
		uint16_t src = (remap >> (i * 2)) & 0x3;

		char c = 'N';

		if(src == CELL_GCM_TEXTURE_REMAP_FROM_A){
			c = 'A';
		}else if(src == CELL_GCM_TEXTURE_REMAP_FROM_R){
			c = 'R';
		}else if(src == CELL_GCM_TEXTURE_REMAP_FROM_G){
			c = 'G';
		}else if(src == CELL_GCM_TEXTURE_REMAP_FROM_B){
			c = 'B';
		}

		uint16_t opp = (remap >> (i * 2 + 8)) & 0x3; // default: CELL_GCM_TEXTURE_REMAP_REMAP
		if(opp == CELL_GCM_TEXTURE_REMAP_ZERO){
			c = '0';
		}else if(opp == CELL_GCM_TEXTURE_REMAP_ONE){
			c = '1';
		}

		strRemap[i] = c;
	}

	return strRemap;
}

CellGcmTexture cellGcmUtilSurfaceToTexture(const CellGcmSurface *surface, uint32_t index)
{
	CellGcmTexture tex;

	{
		uint32_t format = gtfConvFormatSurfaceToTexture(surface->colorFormat);
		format |= (surface->type == CELL_GCM_SURFACE_PITCH)? CELL_GCM_TEXTURE_LN: CELL_GCM_TEXTURE_SZ;

		uint32_t remap = 	CELL_GCM_TEXTURE_REMAP_REMAP << 14 |
							CELL_GCM_TEXTURE_REMAP_REMAP << 12 |
							CELL_GCM_TEXTURE_REMAP_REMAP << 10 |
							CELL_GCM_TEXTURE_REMAP_REMAP <<  8 |
							CELL_GCM_TEXTURE_REMAP_FROM_B << 6 |
							CELL_GCM_TEXTURE_REMAP_FROM_G << 4 |
							CELL_GCM_TEXTURE_REMAP_FROM_R << 2 |
							CELL_GCM_TEXTURE_REMAP_FROM_A;

		CellGcmTexture &texture = tex;
		texture.format = format;
		texture.mipmap = 1;
		texture.dimension = CELL_GCM_TEXTURE_DIMENSION_2;
		texture.cubemap = CELL_GCM_FALSE;
		texture.remap = remap;
		texture.width = surface->width;
		texture.height = surface->height;
		texture.depth = 1;
		texture.pitch = surface->colorPitch[index % 4];
		texture.location = surface->colorLocation[index % 4];
		texture.offset = surface->colorOffset[index % 4];
	}

	return tex;
}

CellGcmTexture cellGcmUtilDepthToTexture(const CellGcmSurface *surface, bool bAsColor)
{
	CellGcmTexture tex;

	{
		uint32_t format = gtfConvFormatDepthToTexture(surface->depthFormat, bAsColor);
		format |= (surface->type == CELL_GCM_SURFACE_PITCH)? CELL_GCM_TEXTURE_LN: CELL_GCM_TEXTURE_SZ;

		uint32_t remap = 	CELL_GCM_TEXTURE_REMAP_REMAP << 14 |
							CELL_GCM_TEXTURE_REMAP_REMAP << 12 |
							CELL_GCM_TEXTURE_REMAP_REMAP << 10 |
							CELL_GCM_TEXTURE_REMAP_REMAP <<  8 |
							CELL_GCM_TEXTURE_REMAP_FROM_B << 6 |
							CELL_GCM_TEXTURE_REMAP_FROM_G << 4 |
							CELL_GCM_TEXTURE_REMAP_FROM_R << 2 |
							CELL_GCM_TEXTURE_REMAP_FROM_A;

		CellGcmTexture &texture = tex;
		texture.format = format;
		texture.mipmap = 1;
		texture.dimension = CELL_GCM_TEXTURE_DIMENSION_2;
		texture.cubemap = CELL_GCM_FALSE;
		texture.remap = remap;
		texture.width = surface->width;
		texture.height = surface->height;
		texture.depth = 1;
		texture.pitch = surface->depthPitch;
		texture.location = surface->depthLocation;
		texture.offset = surface->depthOffset;
	}

	return tex;
}

CellGcmSurface cellGcmUtilTextureToSurface(const CellGcmTexture *color, const CellGcmTexture *depth)
{
	CellGcmSurface surface;
	memset(&surface, 0, sizeof(CellGcmSurface));

	if(color == 0 && depth == 0) return surface;

	if(color){
		surface.colorFormat 	= gtfConvFormatTextureToSurface(color->format);
		surface.colorTarget		= CELL_GCM_SURFACE_TARGET_0;
		surface.colorLocation[0]	= color->location;
		surface.colorOffset[0]	 	= color->offset;
		surface.colorPitch[0]		= color->pitch;
	}else{
		surface.colorFormat		= CELL_GCM_SURFACE_A8R8G8B8;
		surface.colorTarget		= CELL_GCM_SURFACE_TARGET_NONE;
		surface.colorLocation[0]	= CELL_GCM_LOCATION_LOCAL;
		surface.colorOffset[0]	 	= 0;
		surface.colorPitch[0]		= 64;
	}

	// not be used
	surface.colorLocation[1]	= CELL_GCM_LOCATION_LOCAL;
	surface.colorLocation[2]	= CELL_GCM_LOCATION_LOCAL;
	surface.colorLocation[3]	= CELL_GCM_LOCATION_LOCAL;
	surface.colorOffset[1] 	= 0;
	surface.colorOffset[2] 	= 0;
	surface.colorOffset[3] 	= 0;
	surface.colorPitch[1]	= 64;
	surface.colorPitch[2]	= 64;
	surface.colorPitch[3]	= 64;

	if(depth){
		surface.depthFormat 	= gtfConvFormatTextureToDepth(depth->format);
		surface.depthLocation	= depth->location;
		surface.depthOffset	= depth->offset;
		surface.depthPitch 	= depth->pitch;
	}else{
		surface.depthFormat 	= CELL_GCM_SURFACE_Z24S8;
		surface.depthLocation	= CELL_GCM_LOCATION_LOCAL;
		surface.depthOffset	= 0;
		surface.depthPitch 	= 64;
	}

	bool is_swizzle = false;
	if(color){
		is_swizzle = (color->format & CELL_GCM_TEXTURE_LN) != CELL_GCM_TEXTURE_LN;
		surface.width 	= color->width;
		surface.height 	= color->height;
	}else{
		is_swizzle = (depth->format & CELL_GCM_TEXTURE_LN) != CELL_GCM_TEXTURE_LN;
		surface.width 	= depth->width;
		surface.height 	= depth->height;
	}
	surface.type		= (is_swizzle? CELL_GCM_SURFACE_SWIZZLE: CELL_GCM_SURFACE_PITCH);

	surface.antialias	= CELL_GCM_SURFACE_CENTER_1;

	surface.x 		= 0;
	surface.y 		= 0;

	return surface;
}

namespace{
	inline void* fileWriteBuffer(void* dest, const void *src, size_t size)
	{
		memmove(dest, src, size);
		return reinterpret_cast<char*>(dest) + size;
	}
	/*
	inline const void* fileReadBuffer(void* dest, const void *src, size_t size)
	{
		memmove(dest, src, size);
		return reinterpret_cast<const char*>(src) + size;
	}
	*/
	inline void* fileSetBuffer(void* dest, uint8_t byte, size_t size)
	{
		memset(dest, byte, size);
		return reinterpret_cast<char*>(dest) + size;
	}
}

bool cellGcmUtilSaveTexture(const char* fname, const CellGcmTexture *texture, Memory_t *image)
{
	if(fname == 0) return false;
	if(texture == 0) return false;

	uint32_t size = 0;
	size += sizeof(CellGtfFileHeader);
	size += sizeof(CellGtfTextureAttribute);
	const int GTF_FILE_ALIGNMENT = 128;
	uint32_t image_offset = cellGcmAlign(GTF_FILE_ALIGNMENT, size);
	size = image_offset;

	uint32_t image_size = 0;
	void *image_ptr = 0;
	if(image != 0){
		image_size = image->size;
		image_ptr = image->addr;
	}else{
		image_size = cellGcmUtilGetTextureRequireSize(*texture);
		if(cellGcmUtilOffsetToAddress(texture->location, texture->offset, &image_ptr) == false)
		{
			const char *loc = (texture->location == 0? "LOCAL": "MAIN");
			printf("cellGcmUtilSaveTexture() failed. Invalid offset: location=%s offset=0x%08x file=%s\n", loc, texture->offset, fname);
			return false;
		}
	}
	size += image_size;

	Memory_t out;
	if(cellGcmUtilAllocateUnmappedMain(size, 128, &out) == false)
	{
		printf("cellGcmUtilSaveTexture() failed. Cannot allocate temporary memory (size=0x%x)\n", size);
		return false;
	}

	CellGtfFileHeader gfh;
	CellGtfTextureAttribute gta;

	gfh.numTexture = 1;
	gfh.size = size;
	const uint32_t GTF_VERSION = 0x000000FF;
	gfh.version = GTF_VERSION;

	gta.id = 0;
	gta.offsetToTex = image_offset;
	gta.tex = *texture;
	gta.tex._padding = 0;
	gta.tex.location = 0;
	gta.tex.offset = 0;
	gta.textureSize = image_size;

	// write to buffer
	{
		void *ptr = out.addr;
		ptr = fileWriteBuffer(ptr, &gfh, sizeof(CellGtfFileHeader));
		ptr = fileWriteBuffer(ptr, &gta, sizeof(CellGtfTextureAttribute));
		ptr = fileSetBuffer(ptr, 0, image_offset - sizeof(CellGtfFileHeader) - sizeof(CellGtfTextureAttribute));
		ptr = fileWriteBuffer(ptr, image_ptr, image_size);
	}

	bool result = cellGcmUtilWriteFile(fname, (uint8_t*)out.addr, out.size);
	cellGcmUtilFree(&out);

	if(result == false)
	{
		printf("cellGcmUtilSaveTexture() failed. Cannot write file: %s\n", fname);
		return false;
	}

	return result;
}

} // namespace CellGcmUtil
