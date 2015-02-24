// Pics2Png.cpp : Defines the entry point for the console application.
//
#define _CRT_SECURE_NO_WARNINGS

#include "stdafx.h"
#include <windows.h>
#include <tchar.h> 
#include <stdio.h>
#include <strsafe.h>
#include "lodepng\\lodepng.h"
//#pragma comment(lib, "User32.lib")

#ifndef MAX_PATH
#define MAX_PATH 260
#endif

char palette[256][3];

void ColorSetColor(unsigned char n, unsigned char r, unsigned char g, unsigned char b)
{
    palette[n][0] = (r<<2);
    palette[n][1] = (g<<2);
    palette[n][2] = (b<<2);
}

void ColorGlowUpdate (void)
{
	static int glows[3]={20,20,20};
	static int cycle[8]={0,0,0,0,0,0,0,0};
	static int cyclecnt=0;
	static int gv[3]={1,2,3};
	int i;
	int colorval;
	static char timeflag=0;
	static int n1=0,n2=0,n3=0,n4=0,n5=0,n6=0,n7=0,n8=0,n9=0,n10=0,n11=0,n12=0;

	glows[0]+=gv[0];
	glows[1]+=gv[1];
	glows[2]+=gv[2];

	for (i=0;i<3;i++)
	{
		if (glows[i]>63)
		{
			glows[i]=63;
			gv[i]=-gv[i];
		}
		else if (glows[i]<20)
		{
			glows[i]=20;
			gv[i]=-gv[i];
		}
	}
	n1=(rand()&31)+32;
	n2=(rand()&31)+32;
	n3=(rand()&31)+32;
	n4=(rand()&31)+32;
	n5=(rand()&31)+32;
	n6=(rand()&31)+32;
	n7=(rand()&31)+32;
	n8=(rand()&31)+32;
	n9=rand()&63;

	if (n7==32)
	{
		n10=63;
	}

	if (n8==32)
	{
		n11=63;
	}

	if (n9==32)
	{
		n12=63;
	}

	for (i=0;i<8;i++)
	{
		colorval=(i+1)*6+23;
		cycle[cyclecnt]=colorval;
		cyclecnt++;
		if (cyclecnt>7) cyclecnt=0;
	}
	cyclecnt++;
	if (cyclecnt>7) cyclecnt=0;

	if (n10>0) n10-=4;
	if (n11>0) n11-=6;
	if (n12>0) n12-=4;
	if (n10<0) n10=0;
	if (n11<0) n11=0;
	if (n12<0) n12=0;

	ColorSetColor (226,cycle[0],0,0);
	ColorSetColor (227,cycle[1],0,0);
	ColorSetColor (228,cycle[2],0,0);
	ColorSetColor (229,cycle[3],0,0);
	ColorSetColor (230,cycle[4],0,0);
	ColorSetColor (231,cycle[5],0,0);
	ColorSetColor (232,cycle[6],0,0);
	ColorSetColor (233,cycle[7],0,0);

	ColorSetColor (234,n1			,0			,0);
	ColorSetColor (235,n2			,0			,0);
	ColorSetColor (236,n3			,0			,0);
	ColorSetColor (237,n4			,n4/2		,0);
	ColorSetColor (238,n5			,n5/2		,0);
	ColorSetColor (239,n7			,n7-10		,0);
	ColorSetColor (240,n8			,n8-10		,0);
	ColorSetColor (241,n1			,n1			,n1);
	ColorSetColor (242,0			,n4			,0);
	ColorSetColor (243,0			,0			,n5);

	ColorSetColor (244,n10			,n10		,0);
	ColorSetColor (245,n11			,n11		,n11);
	ColorSetColor (246,n12			,n12		,0);

	ColorSetColor (247,glows[0]		,glows[1]	,glows[2]);
	ColorSetColor (248,glows[2]		,glows[2]	,glows[2]);
	ColorSetColor (249,glows[0]		,0			,glows[0]);
	ColorSetColor (250,glows[0]     ,0          ,0);
	ColorSetColor (251,glows[2]		,0			,glows[0]);
	ColorSetColor (252,0			,glows[1]	,0);
	ColorSetColor (253,0			,glows[1]	,glows[2]);
	ColorSetColor (254,0			,0			,glows[2]);
}


void LoadGamePalette(void)
{
    FILE *fp;
    int i;
    unsigned char rgb[3];

    fp = fopen("Game.pal", "rb");
    if (fp == NULL) {
        printf("Cannot open file Game.pal\n");
        return;
    }
    for (i=0; i<256; i++) {
        fread(rgb, 3, 1, fp);
        palette[i][0] = ((rgb[0] & 0x3F) << 2);
        palette[i][1] = ((rgb[1] & 0x3F) << 2);
        palette[i][2] = ((rgb[2] & 0x3F) << 2);
    }
    fclose(fp);
}

void CreatePICtoPNG(const char *fullpath, const char *filename, unsigned int filesize, int force=0, int flipXY=0)
{
    char name[2000];
    char nameout[2000];
    char *p;
    unsigned char *p_bitmap;
    unsigned short width;
    unsigned short height;
    unsigned int x, y;
    unsigned char pixel;
    unsigned char *rgba;
    FILE *fp;

    strcpy(name, fullpath);
    strcpy(name + strlen(name)-1, filename);
    printf("Converting %s...\n", name);

    strcpy(nameout, name);
    p = strchr(nameout, '.');
    if (!p)
        p = nameout + strlen(nameout);
    strcpy(p, ".png");
    //printf("  Into %s\n", nameout);

    fp = fopen(name, "rb");
    if (fp == NULL) {
        printf("Cannot open file %s\n", name);
        return;
    }
    fread(&width, 2, 1, fp);
    fread(&height, 2, 1, fp);

    unsigned int calcsize = width * height + 4;
    if ((calcsize != filesize) && (!force)) {
        printf("File %s is wrong size to be uncompressed bitmap\n", name);
        return;
    }

    p_bitmap = (unsigned char *)malloc(width * height * 4);
    memset(p_bitmap, 0, width*height*4);
    rgba = p_bitmap;
    if (flipXY) {
        for (y=0; y<height; y++) {
            for (x=0; x<width; x++) {
                fread(&pixel, 1, 1, fp);
                rgba = p_bitmap + ((x*height)+y)*4;
                rgba[0] = palette[pixel][0];
                rgba[1] = palette[pixel][1];
                rgba[2] = palette[pixel][2];
                rgba[3] = 0xFF;
                rgba += 4;
            }
        }
        unsigned short t;
        t = width;
        width = height;
        height = t;
    } else {
        for (y=0; y<height; y++) {
            for (x=0; x<width; x++) {
                fread(&pixel, 1, 1, fp);
                rgba[0] = palette[pixel][0];
                rgba[1] = palette[pixel][1];
                rgba[2] = palette[pixel][2];
                rgba[3] = 0xFF;
                rgba += 4;
            }
        }
    }

    lodepng_encode32_file(nameout, p_bitmap, width, height);
    fclose(fp);
    free(p_bitmap);
}

void CreateMXYtoPNG(const char *fullpath, const char *filename, unsigned int filesize)
{
    char name[2000];
    char nameout[2000];
    char *p;
    unsigned char *p_bitmap;
    unsigned short width = 320;
    unsigned short height = 200;
    unsigned int x, y;
    unsigned char pixel;
    unsigned char *rgba;
    FILE *fp;

    strcpy(name, fullpath);
    strcpy(name + strlen(name)-1, filename);
    printf("Converting MXY %s...\n", name);

    strcpy(nameout, name);
    p = strchr(nameout, '.');
    if (!p)
        p = nameout + strlen(nameout);
    strcpy(p, ".png");
    //printf("  Into %s\n", nameout);

    fp = fopen(name, "rb");
    if (fp == NULL) {
        printf("Cannot open file %s\n", name);
        return;
    }
//    fread(&width, 2, 1, fp);
//    width &= 0x7FFF;
//    fread(&height, 2, 1, fp);

    //unsigned int calcsize = width * height + 4;
    //if (calcsize != filesize) {
    //    printf("File %s is wrong size to be uncompressed bitmap\n", name);
    //    return;
    //}

    p_bitmap = (unsigned char *)malloc(width * height * 4);
    memset(p_bitmap, 0, width*height*4);
    rgba = p_bitmap;
    while (1) {
        unsigned short cmd = 0;
        fread(&cmd, 2, 1, fp);
        if (cmd == 0xFFFF)
            break;
        if (cmd & 0x8000) {
            y = cmd & 0x7FFF;
            continue;
        }
        x = cmd;
        unsigned short count;
        unsigned short i;
        fread(&count, 2, 1, fp);
        rgba = p_bitmap + ((y * 320) + x) * 4;
        for (i=0; i<count; i++) {
            fread(&pixel, 1, 1, fp);
            rgba[0] = palette[pixel][0];
            rgba[1] = palette[pixel][1];
            rgba[2] = palette[pixel][2];
            rgba[3] = 0xFF;
            rgba += 4;
        }
    }

    lodepng_encode32_file(nameout, p_bitmap, width, height);
    fclose(fp);
    free(p_bitmap);
}

typedef struct {
    unsigned short offset;
    unsigned char start;
    unsigned char end;
} T_cpcEntry;

int ConvertCompressedCPC(const char *fullpath, const char *filename)
{
    char name[2000];
    char nameout[2000];
    unsigned short width;
    unsigned short height;
    T_cpcEntry entries[256];
    char *p;
    int isAnimated = 0;

    strcpy(name, fullpath);
    strcpy(name + strlen(name)-1, filename);

    strcpy(nameout, name);
    p = strchr(nameout, '.');
    if (!p)
        p = nameout + strlen(nameout);
    strcpy(p, ".png");
//printf("Convert CPC %s to %s?\n", name, nameout);

    ColorGlowUpdate();

    FILE *fp = fopen(name, "rb");
    fread(&height, 2, 1, fp);
    fread(&width, 2, 1, fp);
    if (width > 0x100) {
//printf("  Too big\n");
        // Too big, drop it
        fclose(fp);
        return 0;
    }
    // Read all the entries
    fread(entries, sizeof(T_cpcEntry), width, fp);

    // Check the first entry
    unsigned int firstEntryShouldBe = 4 + width * sizeof(T_cpcEntry);
    if (entries[0].offset != firstEntryShouldBe) {
        // Does not start correctly
//printf("  Wrong start\n");
        fclose(fp);
        return 0;
    }

    unsigned int x, y;
    unsigned char *p_bitmap = (unsigned char *)malloc(width * height * 4);
    unsigned char pixel;
    unsigned char *rgba;
    memset(p_bitmap, 0, width*height*4);
    for (x=0; x<width; x++) {
        if (entries[x].start != 255) {
            for (y=entries[x].start; y<=entries[x].end; y++) {
                rgba = p_bitmap + ((y*width + x) * 4);
                fread(&pixel, 1, 1, fp);
                if (pixel) {
                    rgba[0] = palette[pixel][0];
                    rgba[1] = palette[pixel][1];
                    rgba[2] = palette[pixel][2];
                    rgba[3] = 0xFF;
                    if ((pixel >= 226) && (pixel <= 254)) {
                        isAnimated = 1;
                    }
                }
                rgba += 4;
            }
        }
    }
printf("Converted CPC %s to %s\n", name, nameout);
    lodepng_encode32_file(nameout, p_bitmap, width, height);
    free(p_bitmap);
    fclose(fp);

    if (isAnimated) {
        char *p_end = nameout + strlen(nameout) - 4;
        int i;
        for (i=0; i<32; i++) {
            sprintf(p_end, "_%02d.png", i);
            FILE *fp = fopen(name, "rb");
            fread(&height, 2, 1, fp);
            fread(&width, 2, 1, fp);
            // Read all the entries
            fread(entries, sizeof(T_cpcEntry), width, fp);
            unsigned int x, y;
            unsigned char *p_bitmap = (unsigned char *)malloc(width * height * 4);
            unsigned char pixel;
            unsigned char *rgba;
            memset(p_bitmap, 0, width*height*4);
            for (x=0; x<width; x++) {
                if (entries[x].start != 255) {
                    for (y=entries[x].start; y<=entries[x].end; y++) {
                        rgba = p_bitmap + ((y*width + x) * 4);
                        fread(&pixel, 1, 1, fp);
                        if (pixel) {
                            rgba[0] = palette[pixel][0];
                            rgba[1] = palette[pixel][1];
                            rgba[2] = palette[pixel][2];
                            rgba[3] = 0xFF;
                            if ((pixel >= 226) && (pixel <= 254)) {
                                isAnimated = 1;
                            }
                        }
                        rgba += 4;
                    }
                }
            }
            printf("  and animated %s\n", nameout);
            lodepng_encode32_file(nameout, p_bitmap, width, height);
            free(p_bitmap);
            fclose(fp);
            ColorGlowUpdate();
        }
    }
    return 1;
}

void ProcessDirectory(const char *dir)
{
    WIN32_FIND_DATA ffd;
    char fullpath[1000];
    HANDLE hFind = INVALID_HANDLE_VALUE;
    unsigned int filesize;

    strncpy(fullpath, dir, sizeof(fullpath));
    strncat(fullpath, "\\*", sizeof(fullpath));

    hFind = FindFirstFile(fullpath, &ffd);

    // List all the files in the directory with some info about them.
    do {
        if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            if ((strcmp(ffd.cFileName, ".") != 0) && (strcmp(ffd.cFileName, "..") != 0)) {
                //printf("  %s   <DIR>\n", ffd.cFileName);
                char subpath[2000];
                sprintf(subpath, "%s\\%s", dir, ffd.cFileName);
                ProcessDirectory(subpath);
            }
        } else {
            filesize = (unsigned int)ffd.nFileSizeLow;
            char *p = strchr(ffd.cFileName, '.');
            if (p == NULL) {
                //printf("  %s:%s   %ld bytes\n", fullpath, ffd.cFileName, filesize);
                if (memcmp(fullpath, "PICS\\OBJS\\", 10) == 0) {
//printf("Checking %s:%s\n", fullpath, ffd.cFileName);
                    if (isdigit(ffd.cFileName[0])) {
                        ConvertCompressedCPC(fullpath, ffd.cFileName);
                    }
                } else if (memcmp(fullpath, "PICS\\UI\\MOUSE\\", 13) == 0) {
                    ConvertCompressedCPC(fullpath, ffd.cFileName);
                } else if (memcmp(fullpath, "PICS\\Pointers\\", 13) == 0) {
                    ConvertCompressedCPC(fullpath, ffd.cFileName);
                } else if (memcmp(fullpath, "PICS\\UI\\", 8) == 0) {
                    CreatePICtoPNG(fullpath, ffd.cFileName, filesize);
                } else if (memcmp(fullpath, "PICS\\Screens\\", 8) == 0) {
                    CreatePICtoPNG(fullpath, ffd.cFileName, filesize);
                } else if (memcmp(fullpath, "PICS\\Textures\\", 8) == 0) {
                    CreatePICtoPNG(fullpath, ffd.cFileName, filesize, 1, 1);
                }
            } else if (_stricmp(p, ".PIC")==0) {
                //printf("  %s:%s   %ld bytes\n", fullpath, ffd.cFileName, filesize);
                CreatePICtoPNG(fullpath, ffd.cFileName, filesize);
            } else if (_stricmp(p, ".MXY")==0) {
                //printf("  %s:%s   %ld bytes\n", fullpath, ffd.cFileName, filesize);
                CreateMXYtoPNG(fullpath, ffd.cFileName, filesize);
            } else if (memcmp(fullpath, "PICS\\OBJS\\", 10) == 0) {
                if (isdigit(ffd.cFileName[0])) {
                    ConvertCompressedCPC(fullpath, ffd.cFileName);
                }
            } else if (_stricmp(p, ".TXT")==0) {
                //printf("  %s:%s   %ld bytes\n", fullpath, ffd.cFileName, filesize);
            }
        }
    } while (FindNextFile(hFind, &ffd) != 0);

    FindClose(hFind);
}

int _tmain(int argc, TCHAR *argv[])
{
    size_t length_of_arg;
    DWORD dwError = 0;

    if (argc != 2) {
        printf("\nUsage: %s <directory name>\n", argv[0]);
        return (-1);
    }

    length_of_arg = strlen(argv[1]);
    if (length_of_arg > (MAX_PATH - 3)) {
        printf("\nDirectory path is too long.\n");
        return (-1);
    }

    LoadGamePalette();
    ProcessDirectory(argv[1]);

    return dwError;
}


