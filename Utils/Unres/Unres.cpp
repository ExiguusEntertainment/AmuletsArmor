// Unres.cpp : Defines the entry point for the console application.
//

#define _CRT_SECURE_NO_WARNINGS

#include "stdafx.h"
#include <stdlib.h>
#include <stdio.h>
#include <direct.h>

#define PACK

typedef unsigned char T_byte8;
typedef unsigned short T_word16;
typedef unsigned int T_word32;
typedef void T_void;
typedef signed short T_sword16;
typedef T_word32 T_file;
typedef T_void *T_resource;
typedef T_word16 T_resourceFile;

#define MAX_RESOURCE_FILES 10

#define RESOURCE_ENTRY_TYPE_MEMORY     0x00
#define RESOURCE_ENTRY_TYPE_DISK       0x01
#define RESOURCE_ENTRY_TYPE_DISCARDED  0x02

#ifndef NDEBUG
#define RESOURCE_ENTRY_TYPE_FILE_LOADED 0x80
#endif

#define RESOURCE_ENTRY_TYPE_FILE       0x00
#define RESOURCE_ENTRY_TYPE_DIRECTORY  0x10
#define RESOURCE_ENTRY_TYPE_LINK       0x20
#define RESOURCE_ENTRY_TYPE_APPEND     0x30

#define RESOURCE_ENTRY_TYPE_UNKNOWN    0xC3

#define RESOURCE_ENTRY_TYPE_MASK_WHERE    0x0F
#define RESOURCE_ENTRY_TYPE_MASK_TYPE     0xF0


#define RESOURCE_FILE_UNIQUE_ID (*((T_word32 *)("Res!")))

typedef struct {
    T_byte8 resID[4]            PACK; /* Should contain "ReS"+'\0' id */
    T_byte8 p_resourceName[14]  PACK; /* Case sensitive, 13 characters + '\0' */
    T_word32 fileOffset         PACK;
    T_word32 size               PACK; /* Size in bytes. */
    T_word16 lockCount          PACK; /* 0 = unlocked. */
    T_byte8 resourceType        PACK;
    T_byte8 *p_data             PACK;
    T_resourceFile resourceFile PACK; /* Resource file this is from. */
    T_void *ownerDir            PACK; /* Locked in owner directory (or NULL) */
} T_resourceEntry ;

/* General information stored for each resource directory in the system. */
typedef struct {
    T_file fileHandle           PACK;
    T_word16 numberEntries      PACK;
    T_resourceEntry *p_entries  PACK;
    T_resource ownerRes         PACK;
    T_sword16 nextResource      PACK;
} T_resourceDirInfo ;

/* Header that is placed at the beginning of the file to locate */
/* the resource index. */
typedef struct {
    T_word32 uniqueID           PACK;
    T_word32 indexOffset        PACK;
    T_word32 indexSize          PACK; /* Size of index in bytes. */
    T_word16 numEntries         PACK; /* Number of entries in the index. */
} T_resourceFileHeader ;

void SubDirectory(FILE *fp, unsigned int base, const char *aSubdir, unsigned int aOffset, unsigned int aSize)
{
    unsigned int i;
    printf("Creating sub dir: %s\n", aSubdir);
    _mkdir(aSubdir);
    _chdir(aSubdir);
    
    unsigned char *p = (unsigned char *)malloc(aSize);
    fseek(fp, base+aOffset, SEEK_SET);
    fread(p, aSize, 1, fp);
    T_resourceEntry *p_entry = (T_resourceEntry *)p;
    for (i=0; i<aSize; i+=sizeof(T_resourceEntry), p_entry++) {
        if ((p_entry->resourceType & RESOURCE_ENTRY_TYPE_MASK_TYPE) ==
                RESOURCE_ENTRY_TYPE_DIRECTORY) {
            T_resourceFileHeader header;
            fseek(fp, p_entry->fileOffset, SEEK_SET);
            fread(&header, sizeof(header), 1, fp);

            SubDirectory(fp, 0, (const char *)p_entry->p_resourceName, header.indexOffset, header.indexSize);
        } else {
            // Create a file from the resource
            unsigned char *p_mem = (unsigned char *)malloc(p_entry->size);
            fseek(fp, base+p_entry->fileOffset, SEEK_SET);
            fread(p_mem, p_entry->size, 1, fp);
            FILE *fout = fopen((const char *)p_entry->p_resourceName, "wb");
            fwrite(p_mem, p_entry->size, 1, fout);
            fclose(fout);
            free(p_mem);
        }
    }
    free(p);

    _chdir("..");
}

int _tmain(int argc, _TCHAR* argv[])
{
    char directory[1000] = "files\\";

    if (argc != 2) {
        puts("USAGE: Unres <resource.res>");
        exit(1);
    }
    FILE *fp = fopen(argv[1], "rb");
    if (fp == NULL) {
        printf("Cannot open file %s\n", argv[1]);
        exit(2);
    }
    T_resourceFileHeader header;
    fread(&header, sizeof(header), 1, fp);
    SubDirectory(fp, 0, "files", header.indexOffset, header.indexSize);
    fseek(fp, header.indexOffset, SEEK_SET);
    fclose(fp);
	return 0;
}

