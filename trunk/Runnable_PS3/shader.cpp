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

#include <sys/process.h>
#include <cell/sysmodule.h>

// libgcm
#include <cell/gcm.h>
#include <sysutil/sysutil_sysparam.h>

#include <Cg/cgBinary.h>

// gcmutil
#include "gcmutil.h"
using namespace CellGcmUtil;

using namespace cell::Gcm;

#define ASSERT(x) assert(x)

namespace{
	const uint32_t PROFILE_SCE_VP_RSX = 7003;
	const uint32_t PROFILE_SCE_FP_RSX = 7004;

	uint32_t shaderGetProfile(const unsigned char *buffer)
	{
		if(!buffer) return 0;
        
		const CgBinaryProgram *header = reinterpret_cast<const CgBinaryProgram *>(buffer);
		return header->profile;
	}

	bool shaderIsCgb(const unsigned char *buffer)
	{
		if(!buffer) return false;

		return strncmp(reinterpret_cast<const char*>(buffer), "CGB\0", 4) == 0;
	}

	bool cellGcmUtilLoadShaderVs(const unsigned char *vp_ptr, uint32_t vp_size, Shader_t *shader)
	{
		ASSERT(shader != 0);

		uint32_t profile = shaderGetProfile(vp_ptr);
		if(profile != PROFILE_SCE_VP_RSX) return false;

		memset(shader, 0, sizeof(Shader_t));

		if(cellGcmUtilAllocateUnmappedMain(vp_size, 16, &shader->body) == false)
		{
			printf("cellGcmUtilLoadShaderVs() failed. Cannot allocate memory (size=0x%x)\n", vp_size);
			return false;
		}

		memcpy(shader->body.addr, vp_ptr, vp_size);

		CGprogram vp = reinterpret_cast<CGprogram>(shader->body.addr);
		
		cellGcmCgInitProgram(vp);

		shader->is_cgb = false;
		shader->is_vp = true;
		shader->program = vp;

		uint32_t ucode_size;
		void *ucode;
		cellGcmCgGetUCode(shader->program, &ucode, &ucode_size);
		shader->ucode = ucode;
			
		return true;
	}

	bool cellGcmUtilLoadShaderFs(const unsigned char *fp_ptr, uint32_t fp_size, Shader_t *shader)
	{
		ASSERT(shader != 0);

		uint32_t profile = shaderGetProfile(fp_ptr);
		if(profile != PROFILE_SCE_FP_RSX) return false;

		memset(shader, 0, sizeof(Shader_t));

		if(cellGcmUtilAllocateUnmappedMain(fp_size, 16, &shader->body) == false)
		{
			printf("cellGcmUtilLoadShaderFs() failed. Cannot allocate memory (size=0x%x)\n", fp_size);
			return false;
		}

		memcpy(shader->body.addr, fp_ptr, fp_size);

		CGprogram fp = reinterpret_cast<CGprogram>(shader->body.addr);

		cellGcmCgInitProgram(fp);

		shader->is_cgb = false;
		shader->is_vp = false;
		shader->program = fp;

		shader->ucode = 0;
			
		return true;
	}

	bool cellGcmUtilLoadShaderVsCgb(const unsigned char *vp_ptr, uint32_t vp_size, Shader_t *shader)
	{
		ASSERT(shader != 0);

		if(!shaderIsCgb(vp_ptr)) return false;

		CellCgbProgram prog_cgb;
		int32_t ret = cellCgbRead(vp_ptr, vp_size, &prog_cgb);
		if(ret != CELL_CGB_OK) return false;

		CellCgbProfile profile_cgb = cellCgbGetProfile(&prog_cgb);
		if(profile_cgb != CellCgbVertexProfile) return false;

		memset(shader, 0, sizeof(Shader_t));	

		if(cellGcmUtilAllocateUnmappedMain(vp_size, 16, &shader->body) == false)
		{
			printf("cellGcmUtilLoadShaderVsCgb() failed. Cannot allocate memory (size=0x%x)\n", vp_size);
			return false;
		}

		memcpy(shader->body.addr, vp_ptr, vp_size);

		shader->is_cgb = true;
		shader->is_vp = true;

		ret = cellCgbRead(shader->body.addr, vp_size, &shader->program_cgb);
		if(ret != CELL_CGB_OK)
		{
			printf("cellGcmUtilLoadShaderVsCgb() failed. cellCgbRead() failed (ret=0x%x)\n", ret);
			cellGcmUtilDestroyShader(shader);
			return false;
		}

		//get vertex program ucode and hw configuration
		shader->ucode = const_cast<void*>(cellCgbGetUCode(&shader->program_cgb));
		cellCgbGetVertexConfiguration(&shader->program_cgb,&shader->vertex_program_conf);

		return true;
	}

	bool cellGcmUtilLoadShaderFsCgb(const unsigned char *fp_ptr, uint32_t fp_size, Shader_t *shader)
	{
		ASSERT(shader != 0);

		if(!shaderIsCgb(fp_ptr)) return false;

		CellCgbProgram prog_cgb;
		int32_t ret = cellCgbRead(fp_ptr, fp_size, &prog_cgb);
		if(ret != CELL_CGB_OK) return false;

		CellCgbProfile profile_cgb = cellCgbGetProfile(&prog_cgb);
		if(profile_cgb != CellCgbFragmentProfile) return false;

		memset(shader, 0, sizeof(Shader_t));

		if(cellGcmUtilAllocateUnmappedMain(fp_size, 16, &shader->body) == false)
		{
			printf("cellGcmUtilLoadShaderFsCgb() failed. Cannot allocate memory (size=0x%x)\n", fp_size);
			return false;
		}

		memcpy(shader->body.addr, fp_ptr, fp_size);

		shader->is_cgb = true;
		shader->is_vp = false;
		shader->ucode = 0;

		ret = cellCgbRead(shader->body.addr, fp_size, &shader->program_cgb);
		if(ret != CELL_CGB_OK)
		{
			printf("cellGcmUtilLoadShaderFsCgb() failed. cellCgbRead() failed (ret=0x%x)\n", ret);
			cellGcmUtilDestroyShader(shader);
			return false;
		}
					
		return true;
	}

	bool cellGcmUtilGetShaderUCodeVsCgb(const Shader_t *shader, Memory_t *ucode)
	{
		if(!shader->is_vp) return false;
		if(!shader->is_cgb) return false;

		const void *p_ucode = cellCgbGetUCode(&shader->program_cgb);
		uint32_t ucode_size = cellCgbGetUCodeSize(&shader->program_cgb);

		if(cellGcmUtilAllocateUnmappedMain(ucode_size, 16, ucode) == false)
		{
			printf("cellGcmUtilGetShaderUCodeVsCgb() failed. Cannot allocate memory (size=0x%x)\n", ucode_size);
			return false;
		}
		
		memcpy(ucode->addr, p_ucode, ucode_size);
	
		return true;
	}

	bool cellGcmUtilGetShaderUCodeVs(const Shader_t *shader, Memory_t *ucode)
	{
		if(!shader->is_vp) return false;
		if(shader->is_cgb) return false;

		uint32_t ucode_size;
		void *p_ucode;
		cellGcmCgGetUCode(shader->program, &p_ucode, &ucode_size);

		if(cellGcmUtilAllocateUnmappedMain(ucode_size, 16, ucode) == false)
		{
			printf("cellGcmUtilGetShaderUCodeVs() failed. Cannot allocate memory (size=0x%x)\n", ucode_size);
			return false;
		}

		memcpy(ucode->addr, p_ucode, ucode_size);

		return true;
	}

	bool cellGcmUtilGetShaderUCodeFsCgb(const Shader_t *shader, uint8_t location, Memory_t *ucode)
	{
		if(shader->is_vp) return false;
		if(!shader->is_cgb) return false;

		//get fragment program ucode and hw configuration
		const void *p_ucode = cellCgbGetUCode(&shader->program_cgb);
		uint32_t ucode_size = cellCgbGetUCodeSize(&shader->program_cgb);

		// Microcode for a fragment shader program must be located on a boundary
		//   Local:  64B
		//   Main : 128B
		uint32_t align_size = (location == CELL_GCM_LOCATION_MAIN)? 128: 64;
		if(cellGcmUtilAllocate(ucode_size, align_size, location, ucode) == false)
		{
			printf("cellGcmUtilGetShaderUCodeFsCgb() failed. Cannot allocate memory (size=0x%x)\n", ucode_size);
			return false;
		}
		
		memcpy(ucode->addr, p_ucode, ucode_size);
		
		return true;
	}

	bool cellGcmUtilGetShaderUCodeFs(const Shader_t *shader, uint8_t location, Memory_t *ucode)
	{
		if(shader->is_vp) return false;
		if(shader->is_cgb) return false;

		uint32_t ucode_size;
		void *p_ucode;
		cellGcmCgGetUCode(shader->program, &p_ucode, &ucode_size);

		// Microcode for a fragment shader program must be located on a boundary
		//   Local:  64B
		//   Main : 128B
		uint32_t align_size = (location == CELL_GCM_LOCATION_MAIN)? 128: 64;
		if(cellGcmUtilAllocate(ucode_size, align_size, location, ucode) == false)
		{
			printf("cellGcmUtilGetShaderUCodeFs() failed. Cannot allocate memory (size=0x%x)\n", ucode_size);
			return false;
		}
		
		memcpy(ucode->addr, p_ucode, ucode_size);

		return true;
	}



} // no name namespace

namespace CellGcmUtil{

bool cellGcmUtilLoadShader(const char *fname, Shader_t *shader)
{
	ASSERT(shader != 0);

	memset(shader, 0, sizeof(Shader_t));

	uint8_t *binary = 0;
	uint32_t size = 0;

	if(cellGcmUtilReadFile(fname, &binary, &size) == false)
	{
		printf("cellGcmUtilLoadShader() failed.\n");
		return false;
	}

	bool bLoaded = cellGcmUtilLoadShaderFromMemory(binary, size, shader);

	delete [] binary, binary = 0;
	size = 0;

	return bLoaded;
}

void cellGcmUtilDestroyShader(Shader_t *shader)
{
	if(shader == 0) return;

	cellGcmUtilFree(&shader->body);

	memset(shader, 0, sizeof(Shader_t));
}

bool cellGcmUtilLoadShaderFromMemory(const unsigned char *buffer, uint32_t size, Shader_t *shader)
{
	if(shaderIsCgb(buffer)){
		CellCgbProgram prog_cgb;
		int32_t ret = cellCgbRead(buffer, size, &prog_cgb);
		if(ret == CELL_CGB_OK){
			CellCgbProfile profile_cgb = cellCgbGetProfile(&prog_cgb);
			if(profile_cgb == CellCgbVertexProfile){
				return cellGcmUtilLoadShaderVsCgb(buffer, size, shader);
			}else if(profile_cgb == CellCgbFragmentProfile){
				return cellGcmUtilLoadShaderFsCgb(buffer, size, shader);
			}
		}
	}

	uint32_t profile = shaderGetProfile(buffer);
	if(profile == PROFILE_SCE_VP_RSX){
		return cellGcmUtilLoadShaderVs(buffer, size, shader);
	}else if(profile == PROFILE_SCE_FP_RSX){
		return cellGcmUtilLoadShaderFs(buffer, size, shader);
	}

	printf("cellGcmUtilLoadShaderFromMemory() failed.\n");

	return false;
}

bool cellGcmUtilGetVertexUCode(const Shader_t *shader, Memory_t *ucode)
{
	if(shader == 0) return false;
	if(ucode == 0) return false;
	if(!shader->is_vp) return false;

	if(shader->is_cgb){
		return cellGcmUtilGetShaderUCodeVsCgb(shader, ucode);
	}
	return cellGcmUtilGetShaderUCodeVs(shader, ucode);
}

bool cellGcmUtilGetFragmentUCode(const Shader_t *shader, uint8_t location, Memory_t *ucode)
{
	if(shader == 0) return false;
	if(ucode == 0) return false;
	if(shader->is_vp) return false;

	if(shader->is_cgb){
		return cellGcmUtilGetShaderUCodeFsCgb(shader, location, ucode);
	}
	return cellGcmUtilGetShaderUCodeFs(shader, location, ucode);
}

} // namespace CellGcmUtil
