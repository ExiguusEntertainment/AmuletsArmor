#include <stdio.h>
#include <stdlib.h>
#include <dir.h>
#include <dos.h>
#include <string.h>

typedef unsigned char T_byte8 ;
typedef char T_sbyte8 ;
typedef unsigned int T_word16 ;
typedef int T_sword16 ;
typedef unsigned long T_word32 ;
typedef long T_sword32 ;
typedef void T_void ;

T_void BigToLittle16(T_word16 *p_16) ;
T_void BigToLittle32(T_word32 *p_32) ;

typedef struct {
    T_byte8 Id[4] ;
    T_word32 fileLength ;
    T_byte8 fileType[8] ;
    T_word32 headerLength ;
    T_word16 width ;
    T_word16 length ;
    T_word16 xOffset ;
    T_word16 yOffset ;
    T_byte8 numberOfPlanes ;
    T_byte8 mask ;
    T_byte8 encoding ;
    T_byte8 padding ;
    T_word16 transparent ;
    T_byte8 xAspectRatio ;
    T_byte8 yAspectRatio ;
    T_word16 pageWidth ;
    T_word16 pageHeight ;
} IFFHeader ;

IFFHeader *p_header ;

int mode = 0 ;
unsigned char comp_count = -1 ;
FILE *fp ;

char get_byte(void)
{
    static char waiting = 0 ;
    static char isWaiting = 0 ;
	char ret ;

    if (p_header->encoding)  {
	    if (mode==0)  {
            comp_count = fgetc(fp) ;
		    mode = (comp_count&128) ? 1 : 2 ;
	    }
	    if (mode==1)  {
            if (isWaiting)  {
                ret = waiting ;
                isWaiting = 0 ;
            }  else  {
                ret = fgetc(fp) ;
            }

		    if (comp_count==0)  {
			    mode = 0 ;
		    } else {
                isWaiting = 1 ;
                waiting = ret ;
            }
		    comp_count++ ;
	    } else {
		    if (comp_count==0)
			    mode = 0 ;
		    comp_count-- ;

            if (isWaiting)  {
                ret = waiting ;
                isWaiting = 0 ;
            }  else  {
                ret = fgetc(fp) ;
            }
	    }
    } else {
        ret = fgetc(fp) ;
    }
	return ret ;
}

/*
char get_byteold()
{
	char ret ;

    if (p_header->encoding)  {
	    if (mode==0)  {
		    comp_count = (unsigned char) *(compact_ptr++) ;
		    mode = (comp_count&128) ? 1 : 2 ;
	    }
	    if (mode==1)  {
		    ret = *(compact_ptr) ;
		    if (comp_count==0)  {
			    mode = 0 ;
			    compact_ptr++ ;
		    }
		    comp_count++ ;
	    } else {
		    if (comp_count==0)
			    mode = 0 ;
		    comp_count-- ;
		    ret = *(compact_ptr++) ;
	    }
    } else {
        ret = *(compact_ptr++) ;
    }
	return ret ;
}
*/

void start_compactor(void)
{
	mode = 0 ;
}

void get_line(char *buffer)
{
	int i,j,k,l ;
	char c ;
    int mask ;
    int bitlevel ;
    int bytewidth ;

    mask = p_header->mask ;
    if (mask==1)
        bitlevel = 512 ;
    else
        bitlevel = 256 ;
	memset(buffer, 0, 2048) ;
	start_compactor() ;
    bytewidth = 2*((p_header->width+15)/16) ;
    if (p_header->fileType[0] == 'I')  {
	    for (i=1; i!=bitlevel; i<<=1) {
		    for (j=0; j<bytewidth; j++)  {
			    c = get_byte() ;
			    for (k=128,l=0; l<8; l++,k>>=1)  {
				    if (c&k)
					    buffer[j*8+l] |= (unsigned char)i ;
			    }
		    }
	    }
    } else {
        bytewidth = p_header->width ;
        if (bytewidth & 1)
            bytewidth++ ;
        for (j=0; j<bytewidth; j++)
            buffer[j] = get_byte() ;
    }
}

T_void BigToLittle16(T_word16 *p_16)
{
    T_byte8 *p_bytes ;
    T_byte8 t ;

    p_bytes = (T_byte8 *)p_16 ;
    t = p_bytes[0] ;
    p_bytes[0] = p_bytes[1] ;
    p_bytes[1] = t ;
}

T_void BigToLittle32(T_word32 *p_32)
{
    T_byte8 *p_bytes ;
    T_byte8 t ;

    p_bytes = (T_byte8 *)p_32 ;
    t = p_bytes[0] ;
    p_bytes[0] = p_bytes[3] ;
    p_bytes[3] = t ;
    t = p_bytes[2] ;
    p_bytes[2] = p_bytes[1] ;
    p_bytes[1] = t ;
}

T_void Process(char *filename)
{
    FILE *fout ;
    char outFilename[80] ;
    T_sword16 i ;
    char line[2048] ;
    T_byte8 control ;
    T_byte8 data ;

    if (p_header->width> 2048)  {
        puts("Picture too wide!") ;
        return ;
    }
    /* Generate an output filename */
    strcpy(outFilename, filename) ;
    i = strlen(outFilename) ;
    while (i>=0)  {
        if (outFilename[i] == '.')  {
            strcpy(outFilename+i, ".PIC") ;
            break ;
        }
        i-- ;
    }
    if (i < 0)
        strcat(outFilename, ".PIC") ;

    fout = fopen(outFilename, "wb") ;
    fwrite(&p_header->width, sizeof(T_word16), 1, fout) ;
    fwrite(&p_header->length, sizeof(T_word16), 1, fout) ;

	for (i=0; i<p_header->length; i++)  {
        memset(line, 1, sizeof(line)) ;
		get_line(line) ;
//        memcpy(((char far *)(0xA0000000))+i*320, line, p_header->width) ;
        fwrite(line, p_header->width, 1, fout) ;
	}
    puts("OK") ;
    fclose(fout) ;
}

void ConvertFile(char *filename)
{
    IFFHeader header ;
    T_byte8 tag[4] ;
    T_word16 numColors ;
    T_word16 i ;
    T_byte8 rgb[3] ;
    T_byte8 crange[8] ;
    T_word32 size ;
    T_word32 li ;
    T_byte8 junk[80] ;

    fp = fopen(filename, "rb") ;
    if (fp == NULL)  {
        puts("Cannot OPEN!") ;
        exit(1) ;
    }

    fread(&header, sizeof(IFFHeader), 1, fp) ;
    BigToLittle32(&header.fileLength) ;
    BigToLittle32(&header.headerLength) ;
    BigToLittle16(&header.width) ;
    BigToLittle16(&header.length) ;
    BigToLittle16(&header.xOffset) ;
    BigToLittle16(&header.yOffset) ;
    BigToLittle16(&header.transparent) ;
    BigToLittle16(&header.pageWidth) ;
    BigToLittle16(&header.pageHeight) ;

    numColors = 1<<header.numberOfPlanes ;
//    printf("Number colors: %d\n", numColors) ;

    if (strncmp(header.fileType+4, "ANNO", 4) == 0)  {
        fread(junk, 14, 1, fp) ;
        fread(header.fileType+4, sizeof(header)-12, 1, fp) ;
        BigToLittle32(&header.fileLength) ;
        BigToLittle32(&header.headerLength) ;
        BigToLittle16(&header.width) ;
        BigToLittle16(&header.length) ;
        BigToLittle16(&header.xOffset) ;
        BigToLittle16(&header.yOffset) ;
        BigToLittle16(&header.transparent) ;
        BigToLittle16(&header.pageWidth) ;
        BigToLittle16(&header.pageHeight) ;
    }

    if ((strncmp(header.Id,"FORM", 4) != 0) ||
        ((strncmp(header.fileType, "ILBM", 4) != 0) &&
         (strncmp(header.fileType, "PBM ", 4) != 0)))  {
        puts("Not Deluxe paint file!") ;
        fclose(fp) ;
        return ;
    }

    do {
        fread(tag, 4, 1, fp) ;
        if (!feof(fp))  {
            fread(&size, 4, 1, fp) ;
            BigToLittle32(&size) ;
            if (size & 1)
                size++ ;

            if (strncmp(tag, "CMAP", 4) == 0)  {
                for (li=0; li<size; li+=3)  {
                    fread(rgb, 3, 1, fp) ;
                }
            } else

            if (strncmp(tag, "BODY", 4) == 0)  {
                p_header = &header ;
                Process(filename) ;
            } else {
//                printf("Skipping %4.4s\n", tag) ;
                /* Skip that section. */
                for (li=0; li<size; li++)
                    fread(rgb, 1, 1, fp) ;
            }
        }
    } while (!feof(fp)) ;

    fclose(fp) ;
}

int main(int argc, char *argv[])
{
    struct ffblk ffblk;
    int done;

    if (argc != 2)  {
        puts("USAGE: CONVBBMO <file mask>\n") ;
        exit(1) ;
    }

    printf("Converting %s:\n", argv[1]);
    done = findfirst(argv[1],&ffblk,0);
    while (!done)
    {
       printf("  %s ... ", ffblk.ff_name);
       ConvertFile(ffblk.ff_name) ;
       done = findnext(&ffblk);
    }

    return 0;
}
