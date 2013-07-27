/****************************************************************************/
/*    FILE:  SYNCMEM.C                                                      */
/****************************************************************************/
#include "SYNCMEM.H"

#ifndef NDEBUG
typedef struct {
    char *p_name ;
    T_word32 d1, d2, d3 ;
} T_syncMem ;

static T_syncMem G_syncMem[SYNCMEM_SIZE] ;
static T_word16 G_syncEnd = 0 ;
static E_Boolean G_dumpedOnce = FALSE ;
static T_word32 G_checksum = 0 ;

T_void SyncMemAdd(char *p_name, T_word32 d1, T_word32 d2, T_word32 d3)
{
    G_syncMem[G_syncEnd].p_name = p_name ;
    G_syncMem[G_syncEnd].d1 = d1 ;
    G_syncMem[G_syncEnd].d2 = d2 ;
    G_syncMem[G_syncEnd].d3 = d3 ;
    G_syncEnd++ ;
    if (G_syncEnd == SYNCMEM_SIZE)
        G_syncEnd = 0 ;
    G_checksum += d1+d2+d3+p_name[0] ;
}

T_void SyncMemDump()
{
    FILE *fp ;
    T_word16 i ;

    fp = fopen("syncmem.dat", "w") ;
    for (i=G_syncEnd; i<SYNCMEM_SIZE; i++)  {
        if (G_syncMem[i].p_name)
            fprintf(fp,
                G_syncMem[i].p_name, 
                G_syncMem[i].d1,
                G_syncMem[i].d2,
                G_syncMem[i].d3) ;
    }
    for (i=0; i<G_syncEnd; i++)  {
        if (G_syncMem[i].p_name)
            fprintf(fp,
                G_syncMem[i].p_name, 
                G_syncMem[i].d1,
                G_syncMem[i].d2,
                G_syncMem[i].d3) ;
    }
    fclose(fp) ;
}

T_void SyncMemClear()
{
    memset(G_syncMem, 0, sizeof(G_syncMem)) ;
    G_syncEnd = 0 ;
    G_dumpedOnce = FALSE ;
}

T_void SyncMemDumpOnce(T_void)
{
    if (G_dumpedOnce == FALSE)  {
        G_dumpedOnce = TRUE ;
        SyncMemDump() ;
    }
}

T_word16 SyncMemGetChecksum(T_void)
{
    return ((G_checksum>>16)^(G_checksum & 0xFFFF)) ;
}

#endif

/****************************************************************************/
/*    END OF FILE:  ACTIVITY.C                                              */
/****************************************************************************/
