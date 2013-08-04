/*-------------------------------------------------------------------------*
 * File:  COLORIZE.C
 *-------------------------------------------------------------------------*/
/**
 * The game has many creatures and items that were rendered with gray
 * shades.  We then colorize the raw graphics using color translation
 * tables to other colors.  Doing this allows for more types of creatures.
 * These routine provide the raw optimized translation.
 *
 * @addtogroup COLORIZE
 * @brief Colorizing Routines (Grayscale to Shade table)
 * @see http://www.amuletsandarmor.com/AALicense.txt
 * @{
 *
 *<!-----------------------------------------------------------------------*/
#include "COLORIZE.H"
#include "GENERAL.H"
#include "PICS.H"
#include "RESOURCE.H"

#if (defined(WATCOM))
#pragma aux  ColorizeMemAsm parm	[ESI] [EDI] [ECX] [EBX]
#endif
T_void ColorizeMemAsm(
          T_byte8 *p_source,
          T_byte8 *p_destination,
          T_word32 count,
          T_byte8 *p_transTable) ;

static E_Boolean G_init = FALSE ;

static T_resource G_colorizeTablesRes[MAX_COLORIZE_TABLES] ;
static T_byte8 *G_colorizeTables[MAX_COLORIZE_TABLES] ;

/*-------------------------------------------------------------------------*
 * Routine:  ColorizeInitialize
 *-------------------------------------------------------------------------*/
/**
 *  Colorize Initialize loads in all the tables needed for doing
 *  colorization techniques.
 *
 *  NOTE: 
 *  Bombs if you call twice before callling ColorizeFinish.
 *
 *<!-----------------------------------------------------------------------*/
T_void ColorizeInitialize(T_void)
{
    T_word16 i ;
    T_byte8 resName[20] ;

    DebugRoutine("ColorizeInitialize") ;
    DebugCheck(G_init == FALSE) ;

    if (!G_init)  {
        /* Go through the list of colorization tables and */
        /* lock in each and every one. */
        for (i=1; i<MAX_COLORIZE_TABLES; i++)  {
            sprintf(resName, "COLORIZE/COLORT%02d", i) ;
            DebugCheck(PictureExist(resName)) ;
            G_colorizeTables[i] =
                PictureLockData(
                    resName,
                    &G_colorizeTablesRes[i]) ;
//printf("Locking colorizeTables: %p %p\n", G_colorizeTables[i], G_colorizeTablesRes[i]) ;
            DebugCheck(G_colorizeTables[i] != NULL) ;
            DebugCheck(G_colorizeTablesRes[i] != RESOURCE_BAD) ;
        }

        G_init = TRUE ;
    }

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ColorizeFinish
 *-------------------------------------------------------------------------*/
/**
 *  ColorizeFinish frees up the tables used by colorize.
 *
 *  NOTE: 
 *  Must call ColorizeInitialize first.
 *
 *<!-----------------------------------------------------------------------*/
T_void ColorizeFinish(T_void)
{
    T_word16 i ;

    DebugRoutine("ColorizeFinish") ;
    DebugCheck(G_init == TRUE) ;

    if (G_init)  {
        /* The tables are no longer needed, unlock them. */
        for (i=1; i<MAX_COLORIZE_TABLES; i++)  {
//printf("Unlocking colorizeTables: %p %p\n", G_colorizeTables[i], G_colorizeTablesRes[i]) ;
            PictureUnlockAndUnfind(G_colorizeTablesRes[i]) ;
        }

        /* We are no longer initialized. */
        G_init = FALSE ;
    }

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ColorizeMemory
 *-------------------------------------------------------------------------*/
/**
 *  ColorizeMemory does the brute work of colorizing a section of memory.
 *
 *  NOTE: 
 *  The source and destination should be either exactly the same or don't
 *  overlap.  If you do, unpredictable results will occur.
 *
 *  @param p_source -- Memory to colorize
 *  @param p_destination -- Where to put the colorized memory
 *  @param count -- How many bytes to colorize
 *  @param table -- Table to use to colorize
 *
 *<!-----------------------------------------------------------------------*/
T_void ColorizeMemory(
           T_byte8 *p_source,
           T_byte8 *p_destination,
           T_word32 count,
           E_colorizeTable table)
{
    DebugRoutine("ColorizeMemory") ;
    DebugCheck(p_source != NULL) ;
    DebugCheck(p_destination != NULL) ;
    DebugCheck(table < MAX_COLORIZE_TABLES) ;

    /* Lookup the correct table and let a highly optimized assembly */
    /* routine do the rest. */
    ColorizeMemAsm(
        p_source,
        p_destination,
        count,
        G_colorizeTables[table]) ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ColorizeGetTable
 *-------------------------------------------------------------------------*/
/**
 *  ColorizeGetTable returns the actual table of 256 translation values
 *  for the given table.
 *
 *  NOTE: 
 *  Don't call this with an unknown table or with COLORIZE_TABLE_NONE
 *
 *  @param colorTable -- Table to use to colorize
 *
 *  @return Pointer to table of 256 values
 *
 *<!-----------------------------------------------------------------------*/
T_byte8 *ColorizeGetTable(E_colorizeTable colorTable)
{
    T_byte8 *p_colorize = NULL ;

    DebugRoutine("ColorizeGetTable") ;
    DebugCheck(colorTable != COLORIZE_TABLE_NONE) ;
    DebugCheck(colorTable < MAX_COLORIZE_TABLES) ;

    p_colorize = G_colorizeTables[colorTable] ;

    DebugCheck(p_colorize != NULL) ;
    DebugEnd() ;

    return p_colorize ;
}

#ifdef NO_ASSEMBLY
T_void ColorizeMemAsm(
          T_byte8 *p_source,
          T_byte8 *p_destination,
          T_word32 count,
          T_byte8 *p_transTable)
{
    do {
        *(p_destination++) = p_transTable[*(p_source++)];
    } while (--count);
}
#endif

/** @} */
/*-------------------------------------------------------------------------*
 * End of File:  COLORIZE.C
 *-------------------------------------------------------------------------*/
