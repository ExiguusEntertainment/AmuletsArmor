#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <io.h>
#include <dos.h>

unsigned char *p_buffer ;
unsigned char *p_work ;

typedef struct {
    unsigned int offset ;
    unsigned char start ;
    unsigned char end ;
} entry ;

entry table[256] ;

unsigned char paletteOld[256][3] ;
unsigned char paletteNew[256][3] ;

unsigned char transTable[256] ;

void convertPal(char *filename)
{
    FILE *fp ;
    int x, y ;
    int yy, xx1, xx2 ;
    char *p_line ;
    long size ;
    char newfile[80] ;
    unsigned int offset ;
    unsigned int linesize ;
    unsigned int len ;
    unsigned int i ;
    unsigned int total ;

    printf("  %s ... ", filename) ;

    fp = fopen(filename, "rb") ;
    size = filelength(fileno(fp)) ;
    printf("(Size: %ld) ... ", size) ;
    if (size > 65000L)  {
        puts("Too big to convert!") ;
        return ;
    }
    fread(&x, sizeof(int), 1, fp) ;
    fread(&y, sizeof(int), 1, fp) ;
    fread(p_buffer, size-4, 1, fp) ;
    printf("(%d, %d) ", x, y) ;
    if (x > 320)  {
        puts("... X too big!") ;
        return ;
    }
    if (y > 200)  {
        puts("... Y too big!") ;
        return ;
    }
    fclose(fp) ;

    /* Convert using the trans table. */
    total = x*y ;
    for (i=0; i<total; i++)  {
        p_work[i] = transTable[p_buffer[i]] ;
    }

    /* Open up the data file and write it out. */
    fp = fopen(filename, "wb") ;
    fwrite(&x, sizeof(int), 1, fp) ;
    fwrite(&y, sizeof(int), 1, fp) ;
    fwrite(p_work, total, 1, fp) ;
    fclose(fp) ;

    puts("OK") ;
}

void convertMask(char *fileMask)
{
    struct find_t ffblk;
    int done;

    printf("%s:\n", fileMask) ;

    done = _dos_findfirst(fileMask, _A_NORMAL, &ffblk);
    while (!done) {
        convertPal(ffblk.name) ;
        done = _dos_findnext(&ffblk);
    }
}

void createTrans(void)
{
    int i, j ;
    unsigned char transColor ;
    unsigned long dist ;
    unsigned long bestDist ;
    unsigned long r, g, b ;
    unsigned long dr, dg, db ;

    printf("Creating conversion table ... ") ;

    transTable[0] = 0;

    for (i=1; i<256; i++)  {
        printf("%03d\b\b\b", i) ;
        r = paletteOld[i][0] ;
        g = paletteOld[i][1] ;
        b = paletteOld[i][2] ;

        bestDist = 0x7FFFFFFF ;
        transColor = 0 ;
        for (j=1; j<256; j++)  {
            dr = r - paletteNew[j][0] ;
            dg = g - paletteNew[j][1] ;
            db = b - paletteNew[j][2] ;
            dist = dr*dr + dg*dg + db*db ;

            if (dist < bestDist)  {
                bestDist = dist ;
                transColor = j ;
            }
        }

        transTable[i] = transColor ;
    }
    puts("OK ");
}

void loadPalettes(void)
{
    FILE *fp ;

    printf("Loading palettes ... ") ;
    fp = fopen("palette.old", "rb") ;
    if (fp == NULL)  {
        puts("Error!  cannot open file PALETTE.OLD!") ;
        exit(1) ;
    }
    fread(paletteOld, 768, 1, fp) ;
    fclose(fp) ;

    fp = fopen("palette.new", "rb") ;
    if (fp == NULL)  {
        puts("Error!  cannot open file PALETTE.NEW!") ;
        exit(1) ;
    }
    fread(paletteNew, 768, 1, fp) ;
    fclose(fp) ;
    puts("OK") ;

    createTrans() ;
}

void main(int argc, char *argv[])
{
    int i ;

    if (argc <= 1)  {
        puts("USAGE:  CONPAL [<file or mask> ...]") ;
        exit(1) ;
    }

    p_buffer = malloc(64400) ;
    p_work   = malloc(64400) ;

    loadPalettes() ;
    puts("Processing ...") ;
    if ((p_buffer == NULL) || (p_work == NULL))  {
        puts("NOT ENOUGH MEMORY") ;
        exit(2) ;
    }

    for (i=1; i<argc; i++)  {
        convertMask(argv[i]) ;
    }

    puts("DONE") ;
    free(p_buffer) ;
    free(p_work) ;
}
