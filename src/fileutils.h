/************************************************************
 * Check license.txt in project root for license information *
 *********************************************************** */

#ifndef FILEUTILS_H
#define FILEUTILS_H

#include "utils.h"


static void* _load_file(char* const path, char* const filetype, size_t* fileSize)
{
	*fileSize = 0;
	FILE* fp = fopen(path,filetype);
	if(fp == NULL ) return NULL;
	fseek(fp, 0, SEEK_END);
	size_t len = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	void* ptrToMem = 0;
	ptrToMem = malloc(len);
	fread(ptrToMem,len,1,fp);
	if(fileSize) *fileSize = len;
	return ptrToMem;
}

static inline void* load_binary_file(char* const path, size_t* fileSize)
{
	return _load_file(path,"rb", fileSize);
}

static inline char* load_file(char* const path, size_t* fileSize)
{
	return _load_file(path,"r", fileSize);
}

#endif /* FILEUTILS_H */
