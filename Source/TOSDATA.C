#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_STANCES 64
#define MAX_ANGLES 8
#define MAX_LAYERS 7
#define MAX_PARTS 16
#define MAX_SUB_PARTS 16

unsigned char stances[MAX_STANCES][MAX_ANGLES][MAX_LAYERS+1][2] ;
unsigned char table[MAX_PARTS][MAX_SUB_PARTS][5] ;

void main(void)
{
    FILE *fp ;
    int mode=0 ;
    char buffer[100] ;
    int stanceNum = -1 ;
    char dummy[100] ;
    int angle = 0 ;
    int layer = 0 ;
    char *ptr ;
    int part = 0 ;
    int subpart = 0 ;

    fp = fopen("layers.dat", "r") ;
    if (fp == NULL)  {
        puts("Cannot open file LAYERS.DAT!") ;
        exit(1) ;
    }

    while ((mode != -1) && (!feof(fp)))  {
        switch(mode)  {
            case 0: /* Looking for START or TABLE */
                fgets(buffer, 100, fp) ;
                if (strncmp(buffer, "START", 5) == 0)
                    mode = 1 ;
                if (strncmp(buffer, "TABLE", 5) == 0)
                    mode = 2 ;
                break ;
            case 1: /* Looking for a STANCE. */
                fgets(buffer, 100, fp) ;
                if (strncmp(buffer, "END", 3) == 0)
                    mode = 0 ;
                else {
                    sscanf(buffer, "%s%d", dummy, &stanceNum) ;
                    if (strcmp(dummy, "STANCE")!=0)
                         stanceNum = -1 ;
                    else  {
                         if (stanceNum >= MAX_STANCES)  {
                             printf("Stance number %d out of range!\n", stanceNum) ;
                             exit(1) ;
                         }
                         printf("Stance #%d\n", stanceNum) ;
                         mode = 3 ;
                         angle = 0 ;
                         layer = 0 ;
                    }
                }
                break ;
            case 2:
                buffer[0] = '\0' ;
                fgets(buffer, 100, fp) ;
                if (strncmp(buffer, "END", 3) == 0)
                    mode = 0 ;
                else if ((buffer[0] >= 'a') && (buffer[0] <= 'z'))  {
                    part = buffer[0] - 'a' ;
                    subpart = buffer[1] - 'a' ;
                    if (buffer[2] != ' ')  {
                        printf("Error with line:\n>%s\n", buffer) ;
                        exit(1) ;
                    }
                    if (part >= MAX_PARTS)  {
                        printf("Bad part:\n>%s\n", buffer) ;
                        exit(1) ;
                    }
                    if (subpart >= MAX_SUB_PARTS)  {
                        printf("Bad sub part:\n>%s\n", buffer) ;
                        exit(1) ;
                    }
                    sscanf(buffer+3, "%s", dummy) ;
                    if (strlen(dummy) > 4) {
                        printf("prefix %s is too long!\n", dummy) ;
                        exit(1) ;
                    }
                    strncpy(table[part][subpart], dummy, 5) ;
                }
                break ;
            case 3:
                buffer[0] = '\0' ;
                fgets(buffer, 100, fp) ;
                if (angle == MAX_ANGLES)
                    mode = 1 ;
                else if ((buffer[0] >= 'a') && (buffer[0] <= 'z'))  {
                    ptr = buffer ;
                    if (angle == 8)  {
                        printf("Too many angles at line\n>%s\n", buffer) ;
                        exit(1) ;
                    }
                    while (*ptr != '\0')  {
                        if (layer > MAX_LAYERS)  {
                            printf("Too many layers on line\n>%s\n", buffer) ;
                            exit(1) ;
                        }
                        if (*ptr == '*')  {
                            stances[stanceNum][angle][layer][0] = 0xFF ;
                            angle++ ;
                            layer = 0 ;
                            break ;
                        } else {
                            stances[stanceNum][angle][layer][0] =
                                 *(ptr++) - 'a' ;
                            stances[stanceNum][angle][layer][1] =
                                 *(ptr++) - 'a' ;
                            layer++ ;
                        }
                    }
                }
                break ;
        }
    }

    if (mode != -1)  {
        if (mode != 0)  {
            puts("Missing END!") ;
            exit(1) ;
        }
    }

    fclose(fp) ;

    fp = fopen("STANCES.DAT", "wb") ;
    if (fp == NULL)  {
        puts("Error! Cannot open output file STANCES.DAT") ;
        exit(1) ;
    }
    fwrite(stances, sizeof(stances), 1, fp) ;
    fwrite(table, sizeof(table), 1, fp) ;
    fclose(fp) ;
    puts("Done.") ;
}
