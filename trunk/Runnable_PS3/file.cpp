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

#include <cell/cell_fs.h>
#include <sys/paths.h>
#include <dirent.h>

// gcmutil
#include "gcmutil.h"
using namespace CellGcmUtil;

namespace{
	class CMyFsPrxLoader{
	protected:
		bool mIsLoadFsPrx;
	public:	
		CMyFsPrxLoader(void): mIsLoadFsPrx(false)
		{
			if(cellSysmoduleIsLoaded(CELL_SYSMODULE_FS) != CELL_SYSMODULE_LOADED){
				mIsLoadFsPrx = true;
				int ret = cellSysmoduleLoadModule(CELL_SYSMODULE_FS);
				if(ret != CELL_OK)
				{
					printf("cellSysmoduleLoadModule(CELL_SYSMODULE_FS) failed. (0x%x)\n", ret);
				}
			}
		}
		
		virtual ~CMyFsPrxLoader(void){
			if(mIsLoadFsPrx){
				cellSysmoduleUnloadModule(CELL_SYSMODULE_FS);
			}
		}
	};

	const uint32_t SIZE_MAX = 0xFFFFFFFFUL;
} // no name namespace

namespace CellGcmUtil{

bool cellGcmUtilReadFile(const char *fname, uint8_t **buffer, uint32_t *buf_size, uint32_t offset, uint32_t size)
{
	if(!fname) return false;
	if(!buffer) return false;
	if(!buf_size) return false;
	
	*buf_size = 0;
	*buffer = 0;
	
	CMyFsPrxLoader fs_prx_loader;
	
	CellFsStat fstat;
	if(cellFsStat(fname, &fstat) != CELL_FS_SUCCEEDED)
	{
		printf("cellGcmUtilReadFile() failed. File not found: %s\n", fname);
		return false;
	}
	if(SIZE_MAX < fstat.st_size)
	{
		printf("cellGcmUtilReadFile() failed. File too large (size=0x%llx): %s\n", fstat.st_size, fname);
		return false;
	}
	
	if(size == SIZE_MAX){
		size = static_cast<uint32_t>(fstat.st_size) - offset;
	}
	
	int fd;
	if(cellFsOpen(fname, CELL_FS_O_RDONLY, &fd, NULL, 0) != CELL_FS_SUCCEEDED)
	{
		printf("cellGcmUtilReadFile() failed. Could not open file: %s\n", fname);
		return false;
	}
	
	// seek
	uint64_t pos;
	if(cellFsLseek(fd, offset, CELL_FS_SEEK_SET, &pos) != CELL_FS_SUCCEEDED)
	{
		printf("cellGcmUtilReadFile() failed. File seek error (offset=0x%x): %s\n", offset, fname);
		cellFsClose(fd);
		return false;
	}
	
	if(size > 0){
		*buffer = new uint8_t[size];

		if(*buffer == 0)
		{
			printf("cellGcmUtilReadFile() failed. Cannot allocate memory (size=0x%x): %s\n", size, fname);
			cellFsClose(fd);
			return false;
		}
		
		uint64_t nread;
		if(cellFsRead(fd, *buffer, size, &nread) != CELL_FS_SUCCEEDED){
			nread = 0;
		}
		
		if(nread == 0){
			*buf_size = 0;
			delete [] *buffer, *buffer = 0;
		}else{
			*buf_size = size;
		}
	}
	
	cellFsClose(fd);

	return true;
}

bool cellGcmUtilWriteFile(const char *fname, uint8_t *buffer, uint32_t buf_size)
{
	if(!fname) return false;
	if(!buffer) return false;

	CMyFsPrxLoader fs_prx_loader;

	int fd;
	CellFsErrno err = cellFsOpen(fname, CELL_FS_O_CREAT | CELL_FS_O_TRUNC | CELL_FS_O_RDWR, &fd, NULL, 0);
	if(err != CELL_FS_SUCCEEDED)
	{
		printf("cellGcmUtilWriteFile() failed. Could not open file: %s\n", fname);
		return false;
	}

	uint64_t nwrite = 0;
	int ret = cellFsWrite(fd, buffer, buf_size, &nwrite);
	if(ret != CELL_FS_SUCCEEDED || buf_size != nwrite)
	{
		printf("cellGcmUtilWriteFile() failed. Failed to write (size=0x%x): %s\n", buf_size, fname);
		cellFsClose(fd);
		return false;
	}

	cellFsClose(fd);

	return true;
}

namespace{
	void fileAddFileList(FileList_t *filelist, const char *fname)
	{
		const uint32_t MY_FILE_INC = 32;

		if(filelist == 0) return;

		FileList_t &list = *filelist;
		
		if(list.count + 1 < list.size){
			uint32_t len = strlen(fname) + 1;
			list.files[list.count] = new char[len];
			memcpy(list.files[list.count], fname, len);

			//printf("added[%d]: %s\n", mFileCount, sFileName[mFileCount]);
			++list.count;
		}else{
			list.size += MY_FILE_INC;
			char** new_files = new char* [list.size];
			memset(new_files, 0, sizeof(char*) * list.size);

			for(uint32_t i = 0; i < list.count; ++i){
				new_files[i] = list.files[i];
			}

			if(list.files){
				delete [] list.files, list.files = 0;
			}
			list.files = new_files;

			fileAddFileList(filelist, fname);
		}
	}

	void fileSortFileList(FileList_t *filelist)
	{
		int data_size = (int)filelist->count;
		for(int j = data_size - 2; j >= 0; j--){
			for(int i = 0; i <= j; i++){
				if(strcmp(filelist->files[i], filelist->files[i+1]) > 0){
					char *tmp_data = filelist->files[i];
					filelist->files[i] = filelist->files[i+1];
					filelist->files[i+1] = tmp_data;
				}
			}
		}
	}

	uint32_t fileGetFileListUseDirent(const char *path, const char *ext, FileList_t *filelist)
	{
		if(filelist == 0) return 0;

		const char *dirName = path;
		size_t dirLength = strlen(dirName) + 1;

		DIR *dirp = opendir(dirName);

		if(dirp != NULL){
			dirent *dir_info = readdir(dirp);
			while(dir_info != NULL){

				if(dir_info->d_type != DT_REG){
					dir_info = readdir(dirp);
					continue;
				}

				if(ext && strcmp(ext, "") != 0){
					char *ptr = strrchr(dir_info->d_name, '.');
					if(!ptr || strcasecmp(ext, ptr+1) != 0){
						dir_info = readdir(dirp);
						continue;
					}
				}

				size_t len = strlen(dir_info->d_name) + dirLength;
				char *fname = new char[len+1];
				sprintf(fname,"%s/%s",dirName, dir_info->d_name);
				fileAddFileList(filelist, fname);
				delete [] fname;

				dir_info = readdir(dirp);
			}
		}

		closedir(dirp);
		dirp = NULL;

		fileSortFileList(filelist);

		return filelist->count;
	}

} // namespace
void cellGcmUtilAddFileList(FileList_t *filelist, const char *fname)
{
	fileAddFileList(filelist, fname);
}
void cellGcmUtilSortFileList(FileList_t *filelist)
{
	fileSortFileList(filelist);
}
uint32_t cellGcmUtilGetFileList(const char *path, const char *ext, FileList_t *filelist)
{
	if(filelist == 0) return 0;

	CMyFsPrxLoader fs_prx_loader;

    int fd = -1;
    const char *dirName = path;
    cellFsOpendir(dirName,&fd);
    size_t dirLength = strlen(dirName) + 1;
    uint32_t data_count = 0;
	
	CellFsDirectoryEntry entry;
	memset(&entry, 0, sizeof(CellFsDirectoryEntry));

    do {
		CellFsErrno er = cellFsGetDirectoryEntries(fd, &entry, sizeof(CellFsDirectoryEntry), &data_count);
		if(er < 0){
			return fileGetFileListUseDirent(path, ext, filelist);
		}

		if(entry.entry_name.d_type == CELL_FS_TYPE_DIRECTORY && 
			strcasecmp(entry.entry_name.d_name, ".") != 0 &&
			strcasecmp(entry.entry_name.d_name, "..") != 0
		){
			size_t dlen = strlen(entry.entry_name.d_name) + dirLength;
			char *dname = new char[dlen+1];
			sprintf(dname,"%s/%s",dirName, entry.entry_name.d_name);
			cellGcmUtilGetFileList(dname, ext, filelist);
			delete [] dname;
			continue;
		}else 
		if(entry.entry_name.d_type != CELL_FS_TYPE_REGULAR) continue;
		
		if(ext && strcmp(ext, "") != 0){
			char *ptr = strrchr(entry.entry_name.d_name, '.');
			if(!ptr || strcasecmp(ext, ptr+1) != 0) continue;
		}

		size_t len = strlen(entry.entry_name.d_name) + dirLength;
		char *fname = new char[len+1];
		sprintf(fname,"%s/%s",dirName, entry.entry_name.d_name);
		fileAddFileList(filelist, fname);
		delete [] fname;

    } while (data_count);

	cellFsClosedir(fd);

	fileSortFileList(filelist);

	return filelist->count;
}

void cellGcmUtilFreeFileList(FileList_t *filelist)
{
	if(filelist == 0) return;
	if(filelist->files == 0) return;

	if(filelist->files){
		for(uint32_t i = 0; i < filelist->count; ++i){
			if(filelist->files[i]){
                delete [] filelist->files[i], filelist->files[i] = 0;
			}
		}
		delete [] filelist->files, filelist->files = 0;        
	}

	memset(filelist, 0, sizeof(FileList_t));
}

bool cellGcmUtilIsFileExsits(const char *fname)
{
	CMyFsPrxLoader fs_prx_loader;

	CellFsStat fstat;
	CellFsErrno err = cellFsStat(fname, &fstat);

	return (err == CELL_FS_SUCCEEDED);
}

} // namespace CellGcmUtil
