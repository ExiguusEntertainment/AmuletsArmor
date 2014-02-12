#include <stdio.h>
#include <stdlib.h>
#include <mem.h>
#include <conio.h>
#include <malloc.h>
#include <io.h>
#include <ctype.h>
#include <bios.h>
#include <string.h>

#define QUOTE_START    0xFF
#define QUOTE_END      0xFE

typedef void T_void ;
typedef char T_sbyte8 ;
typedef unsigned char T_byte8 ;
typedef short int T_sword16 ;
typedef unsigned short int T_word16 ;
typedef long T_sword32 ;
typedef unsigned long T_word32 ;
typedef enum T_byte8 {
    FALSE,
    TRUE,
    BOOLEAN_UNKNOWN
} E_Boolean ;

typedef enum {
    DATA_MODE_SEEK_START_LINE,
    DATA_MODE_SKIP_EQUAL,
    DATA_MODE_SEEK_START_WEIGHT,
    DATA_MODE_SEEK_MULTIPLY,
    DATA_MODE_SEEK_MODIFIER,
    DATA_MODE_SEEK_ADD_OR_END
} E_dataMode ;

T_void main(int argc, char *argv[])
{
    T_byte8 outFilename[80] ;
    T_byte8 *p_ext ;
    FILE *fout ;
    FILE *fp ;
    T_word16 value ;
    E_dataMode mode = DATA_MODE_SEEK_START_LINE ;
    T_byte8 token[300] ;
    T_word16 endOfFileMarker = 0xFFFF ;
    T_word16 outsideMarker = 0x800A ;

    textcolor(LIGHTRED) ;
    cprintf("\n<<< ") ;
    textcolor(WHITE) ;
    cprintf("Light Scripting Compiler ver0.1 -- 1996 (C) Lysle E. Shields III") ;
    textcolor(LIGHTRED) ;
    cprintf(" >>>\n\r") ;
    textcolor(LIGHTGREEN) ;
    if (argc != 2)  {
        textcolor(LIGHTRED) ;
        cprintf("USAGE: LIGHTC <input file>\n\r") ;
        textcolor(LIGHTGRAY) ;
        cprintf("Returning to DOS...\r\n") ;
        exit(1) ;
    }

    fp = fopen(argv[1], "r") ;
    if (fp == NULL)  {
        strupr(argv[1]) ;
        textcolor(LIGHTRED) ;
        cprintf("Cannot open file '%s'\r\n", argv[1]) ;
        textcolor(LIGHTGRAY) ;
        cprintf("Returning to DOS...\r\n") ;
        exit(2) ;
    }

    strcpy(outFilename, argv[1]) ;
    strupr(outFilename) ;
    p_ext = strstr(outFilename, ".") ;
    if (p_ext == NULL)
        p_ext = outFilename + strlen(outFilename) ;
    strcpy(p_ext, ".LIT") ;

    fout = fopen(outFilename, "wb") ;
    if (fout == NULL)  {
        textcolor(LIGHTRED) ;
        cprintf("Cannot open file '%s'\r\n", outFilename) ;
        textcolor(LIGHTGRAY) ;
        cprintf("Returning to DOS...\r\n") ;
        exit(2) ;
    }

    fscanf(fp, "%s", token) ;
    while (!feof(fp))  {
        strupr(token) ;
        if ((token[0] == ';') || (strncmp(token, "//", 2)==0))  {
            /* Pass up any comments. */
            fgets(token, 200, fp) ;
        } else {
            cprintf("%s", token) ;

            switch(mode)  {
                case DATA_MODE_SEEK_START_LINE:
                    if (token[0] == 'R')  {
                        sscanf(token+1, "%u", &value) ;
                        if (value >= 10)  {
                            textcolor(LIGHTRED) ;
                            cprintf("\r\nRegister %u is out of range!\r\n", value) ;
                            exit(1) ;
                        }

                        value |= 0x8000 ;
                        fwrite(&value, 2, 1, fout) ;
                        mode = DATA_MODE_SKIP_EQUAL ;
                    } else if (token[0] == 'S')  {
                        sscanf(token+1, "%u", &value) ;
                        fwrite(&value, 2, 1, fout) ;
                        mode = DATA_MODE_SKIP_EQUAL ;
                    } else {
                        textcolor(LIGHTRED) ;
                        cprintf("\r\nUnknown start of command '%s'\r\n", token) ;
                        exit(1) ;
                    }
                    break ;
                case DATA_MODE_SKIP_EQUAL:
                    if (strcmp(token, "=") == 0)  {
                        mode = DATA_MODE_SEEK_START_WEIGHT ;
                    } else {
                        textcolor(LIGHTRED) ;
                        cprintf("\r\nMissing '='!\r\n", token) ;
                    }
                    break ;
                case DATA_MODE_SEEK_START_WEIGHT:
                    if (token[0] == 'R')  {
                        sscanf(token+1, "%u", &value) ;
                        if (value >= 10)  {
                            textcolor(LIGHTRED) ;
                            cprintf("\r\nRegister %u is out of range!\r\n", value) ;
                            exit(1) ;
                        }
                        value |= 0x8000 ;
                        fwrite(&value, 2, 1, fout) ;
                        mode = DATA_MODE_SEEK_MULTIPLY ;
                    } else if (token[0] == 'S')  {
                        sscanf(token+1, "%u", &value) ;
                        fwrite(&value, 2, 1, fout) ;
                        mode = DATA_MODE_SEEK_MULTIPLY ;
                    } else if (strcmp(token, "100")==0)  {
                        value = 256 | 0x4000;
                        fwrite(&value, 2, 1, fout) ;
                        mode = DATA_MODE_SEEK_MULTIPLY ;
                    } else if (strcmp(token, "OUTSIDE") == 0)  {
                        fwrite(&outsideMarker, 2, 1, fout) ;
                        mode = DATA_MODE_SEEK_MULTIPLY ;
                    } else {
                        textcolor(LIGHTRED) ;
                        cprintf("\r\nUnknown start of weight! '%s'\r\n", token) ;
                        exit(1) ;
                    }
                    break ;
                case DATA_MODE_SEEK_MULTIPLY :
                    if (strcmp(token, "*") == 0)  {
                        mode = DATA_MODE_SEEK_MODIFIER ;
                    } else {
                        textcolor(LIGHTRED) ;
                        cprintf("\r\nMissing '*'!\r\n", token) ;
                        exit(1) ;
                    }
                    break ;
                case DATA_MODE_SEEK_MODIFIER:
                    if (token[0] == 'D')  {
                        sscanf(token+1, "%u", &value) ;
                        value |= 0x8000 ;
                        fwrite(&value, 2, 1, fout) ;
                        mode = DATA_MODE_SEEK_ADD_OR_END ;
                    } else if (token[strlen(token)-1] == '%')  {
                        sscanf(token, "%u", &value) ;
                        if (value > 100)  {
                            textcolor(LIGHTRED) ;
                            cprintf(
                                "\r\nMultiplier of value %u is too high!",
                                value) ;
                            exit(1) ;
                        }
                        /* Turn value of 0-100 into 0-256 range. */
                        value = (value<<8)/100 ;
                        fwrite(&value, 2, 1, fout) ;
                        mode = DATA_MODE_SEEK_ADD_OR_END ;
                    } else {
                        textcolor(LIGHTRED) ;
                        cprintf("\r\nUnknown multiplier '%s'!\r\n") ;
                        exit(1) ;
                    }
                    break ;
                case DATA_MODE_SEEK_ADD_OR_END:
                    if (strcmp(token, "+") == 0)  {
                        mode = DATA_MODE_SEEK_START_WEIGHT ;
                    } else if (strcmp(token, ".") == 0)  {
                        value = 0xFFFF ;
                        fwrite(&value, 2, 1, fout) ;
                        mode = DATA_MODE_SEEK_START_LINE ;
                        cprintf("\r\n") ;
                    } else {
                        textcolor(LIGHTRED) ;
                        cprintf("\r\nMissing '+' or '.'!\n\r") ;
                        exit(1) ;
                    }
                    break ;
                default:
                    textcolor(LIGHTRED) ;
                    cprintf("\r\nInternal error #100!\n\r") ;
                    exit(1) ;
                    break ;
            }
        }

        /* Get the next token. */
        fscanf(fp, "%s", token) ;
    }

    fwrite(&endOfFileMarker, 2, 1, fout) ;
    fclose(fp) ;
    fclose(fout) ;

    textcolor(LIGHTGRAY) ;
    cprintf("\r\nDone.\r\n") ;

    exit(0) ;
}

