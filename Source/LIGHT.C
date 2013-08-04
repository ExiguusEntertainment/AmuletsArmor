/*-------------------------------------------------------------------------*
 * File:  LIGHT.C
 *-------------------------------------------------------------------------*/
/**
 * All maps have a lighting table that tells how the sectors affect
 * one another as light enters leaves based on sky, torches, and doors.
 *
 * @addtogroup LIGHT
 * @brief Light Table for Map
 * @see http://www.amuletsandarmor.com/AALicense.txt
 * @{
 *
 *<!-----------------------------------------------------------------------*/
#include "3D_IO.H"
#include "DOOR.H"
#include "LIGHT.H"
#include "MAP.H"
#include "MEMORY.H"
#include "OBJECT.H"
#include "PICS.H"
#include "RESOURCE.H"

#define LIGHT_TAG            (*((T_word32 *)"LiT"))
#define LIGHT_DEAD_TAG       (*((T_word32 *)"DlI"))

typedef struct {
    T_word32 tag ;
    T_word16 *p_table ;
    T_resource res ;
} T_lightTableStruct ;

/* Variable that keeps all the light values of sectors after */
/* illuminating objects are applied. */
T_byte8 *G_preLightCalculations ;

/* variable that keeps all the intermediate light calculations. */
T_byte8 *G_lightCalculations ;

/* Internal prototypes: */
static T_void ILightIlluminate(T_byte8 *p_lightList) ;

/*-------------------------------------------------------------------------*
 * Routine:  LightTableLoad
 *-------------------------------------------------------------------------*/
/**
 *  LightTableLoad loads in a light table and initializes a handle for
 *  future references to the light table.
 *
 *  NOTE: 
 *  The level has to be loaded before the light table is loaded.  This
 *  program needs to know the number of sectors that are on a map.
 *
 *  @param number -- Number of light table to load
 *
 *  @return Handle to light table
 *
 *<!-----------------------------------------------------------------------*/
T_lightTable LightTableLoad(T_word32 number)
{
    T_lightTableStruct *p_light ;
    T_byte8 name[40] ;

    DebugRoutine("LightTableLoad") ;

    /* Allocate a structure for a light table handle. */
    p_light = MemAlloc(sizeof(T_lightTableStruct)) ;
    DebugCheck(p_light != NULL) ;

    /* Make sure we got the handle. */
    if (p_light)  {
        /* Get the appropriate name. */
        sprintf(name, "L%ld.LIT", number) ;

        if (PictureExist(name))  {
            /* Lock the data into memory. */
            p_light->p_table = (T_word16 *)PictureLockData(name, &p_light->res) ;
            DebugCheck(p_light->p_table != NULL) ;
        } else {
            DebugCheck(FALSE) ;
            p_light->p_table = NULL ;
        }

        if (p_light->p_table == NULL)  {
            /* If we didn't get it, drop the handle */
            /* and return a bad handle. */
            MemFree(p_light) ;
            p_light = NULL ;
        } else {
            /* Everything is fine, mark the handle tag as good. */
            p_light->tag = LIGHT_TAG ;

            /* Allocate memory for the intermeditate calculations. */
            G_lightCalculations = MemAlloc(G_Num3dSectors) ;
            DebugCheck(G_lightCalculations != NULL) ;
            G_preLightCalculations = MemAlloc(G_Num3dSectors) ;
            DebugCheck(G_preLightCalculations != NULL) ;
        }
    }

    DebugEnd() ;

    /* Return what we found for a light table handle. */
    return ((T_lightTable)p_light) ;
}

/*-------------------------------------------------------------------------*
 * Routine:  LightTableUnload
 *-------------------------------------------------------------------------*/
/**
 *  LightTableUnload frees up a previously allocated light table.
 *
 *  @param light -- Handle to light table to unload
 *
 *<!-----------------------------------------------------------------------*/
T_void LightTableUnload(T_lightTable light)
{
    T_lightTableStruct *p_light ;

    DebugRoutine("LightTableUnload") ;
    DebugCheck(light != NULL) ;

    /* Make sure we got a handle. */
    if (light)  {
        /* Dereference the handle. */
        p_light = (T_lightTableStruct *)light ;
        DebugCheck(p_light->tag == LIGHT_TAG) ;

        /* Make sure it is a legal handle. */
        if (p_light->tag == LIGHT_TAG)  {
            /* Unload the light table. */
            PictureUnlockAndUnfind(p_light->res) ;
            p_light->tag = LIGHT_DEAD_TAG ;
            MemFree(p_light) ;

            MemFree(G_lightCalculations) ;
            MemFree(G_preLightCalculations) ;
        }
    }

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  LightTableRecalculate
 *-------------------------------------------------------------------------*/
/**
 *  LightTableRecalculate does the work to recompute all the lighting
 *  values.
 *
 *  @param light -- Handle to light table to recalculate
 *  @param outsideLighting -- Lighting level of outside (main)
 *
 *<!-----------------------------------------------------------------------*/
T_void LightTableRecalculate(T_lightTable light, T_byte8 outsideLighting)
{
    T_lightTableStruct *p_light ;
    T_word16 *p_pos ;
    T_word16 storageReg ;
    T_word32 registers[11] ;
    T_word32 total ;
    T_word16 sourceReg ;
    T_word16 multReg ;
    T_word32 value ;
    T_word32 mult ;
    T_word16 i ;
    T_word16 j ;

    DebugRoutine("LightTableRecalculate") ;
    DebugCheck(light != NULL) ;

    for (j=0; j<2; j++)  {
        /* Make sure we got a handle. */
        if (light)  {
            /* Dereference the handle. */
            p_light = (T_lightTableStruct *)light ;
            DebugCheck(p_light->tag == LIGHT_TAG) ;

            /* Make sure it is a legal handle. */
            if (p_light->tag == LIGHT_TAG)  {
                /* First, get the light levels. */
                for (i=0; i<G_Num3dSectors; i++)  {
                    G_preLightCalculations[i] =
                        G_lightCalculations[i] =
                            MapGetSectorLighting(i) ;
                }

                /* Second, modify based on illuminating objects */
                ILightIlluminate(G_preLightCalculations) ;

                /* Now recalculate the light table. */
                p_pos = p_light->p_table ;
                memset(registers, 0, sizeof(registers)) ;
                registers[10] = outsideLighting ;

                /* Loop until end of table. */
                while (*p_pos != 0xFFFF)  {
                    storageReg = *(p_pos++) ;
                    total = 0 ;

                    /* Loop until end of formula. */
                    while (*p_pos != 0xFFFF)  {
                        /* Get a source variable. */
                        sourceReg = *(p_pos++) ;
                        value = 0 ;
                        /* What type of source is it? */
                        if (sourceReg & 0x8000)  {
                            /* It is a register. */
                            sourceReg &= 0x7FFF ;
                            DebugCheck(sourceReg < 11) ;
                            if (sourceReg < 11)
                                value = registers[sourceReg] ;
                        } else if (sourceReg & 0x4000)  {
                            /* It is a constant 100. */
                            value = sourceReg & 0x3FFF ;
                        } else {
                            /* It is a sector based light. */
                            DebugCheck(sourceReg < G_Num3dSectors) ;
    //                        value = G_preLightCalculations[sourceReg] ;
                            value = MapGetSectorLighting(sourceReg) ;
                        }

                        /* Get a multiplier. */
                        multReg = *(p_pos++) ;

                        /* Is it a door or just a constant? */
                        if (multReg & 0x8000)  {
                            /* It is a door. */
                            mult = DoorGetPercentOpen(multReg & 0x7FFF) ;
                        } else {
                            /* Just a constant percentage. */
                            mult = multReg ;
                        }

                        /* Add to the total. */
                        total += mult * value ;
                    }

                    /* Skip the 0xFFFF */
                    p_pos++ ;

                    /* Peel off the extra bits. */
                    total >>= 8 ;

                    /* Make sure the total is in range. */
                    if (total >= 256)
                        total = 255 ;

                    /* Store the total in the requested slot. */
                    if (storageReg & 0x8000)  {
                        /* Must be a register. */
                        storageReg &= 0x7FFF ;
                        DebugCheck(storageReg < 11) ;
                        if (storageReg < 11)
                            registers[storageReg] = total ;
                    } else {
                        /* Must be a sector. */
                        DebugCheck(storageReg < G_Num3dSectors) ;
                        G_lightCalculations[storageReg] = total ;
                    }
                }

                /* Second, modify based on illuminating objects */
                ILightIlluminate(G_lightCalculations) ;

                /* All the calculations have been made. */
                /* Store the light levels. */
                for (i=0; i<G_Num3dSectors; i++)
                    MapSetSectorLighting(i, G_lightCalculations[i]) ;
            }
        }
    }

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ILightIlluminate
 *-------------------------------------------------------------------------*/
/**
 *  ILightIlluminate changes the sector lighting values based on lit
 *  objects.
 *
 *<!-----------------------------------------------------------------------*/
static T_void ILightIlluminate(T_byte8 *p_lightList)
{
    T_3dObject *p_obj ;
    T_sword16 illum ;
    T_word16 sector ;
    T_sword16 light ;

    DebugRoutine("ILightIlluminate") ;

    p_obj = ObjectsGetFirst() ;
    while (p_obj)   {
        illum = ObjectGetIllumination(p_obj) ;
        if (illum)  {
#if 0
            num = ObjectGetNumAreaSectors(p_obj) ;
            for (i=0; i<num; i++)  {
                sector = ObjectGetNthAreaSector(p_obj, i) ;

                DebugCheck(sector < G_Num3dSectors) ;
                light = p_lightList[sector] ;
                light += illum ;
                if (light >= 256)
                    light = 255 ;
                else if (light < 0)
                    light = 0 ;
                p_lightList[sector] = light ;
            }
#else
            sector = ObjectGetCenterSector(p_obj) ;

            DebugCheck(sector < G_Num3dSectors) ;
            if (sector < G_Num3dSectors)  {
                light = p_lightList[sector] ;
                light += illum ;
                if (light >= 256)
                    light = 255 ;
                else if (light < 0)
                    light = 0 ;
                p_lightList[sector] = (T_byte8)light ;
            }
#endif
        }

        p_obj = ObjectGetNext(p_obj) ;
    }

    DebugEnd() ;
}

/** @} */
/*-------------------------------------------------------------------------*
 * End of File:  LIGHT.C
 *-------------------------------------------------------------------------*/
