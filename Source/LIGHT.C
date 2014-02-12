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
#include "3D_VIEW.H"

#define LIGHT_TAG            (*((T_word32 *)"LiT"))
#define LIGHT_DEAD_TAG       (*((T_word32 *)"DlI"))

typedef struct {
    T_word32 tag ;
    T_word16 *p_table ;
    T_resource res ;

    T_word32 tableLen;
    T_word32 tableAllocLen;
} T_lightTableStruct ;

/* Variable that keeps all the light values of sectors after */
/* illuminating objects are applied. */
T_byte8 *G_preLightCalculations ;

/* variable that keeps all the intermediate light calculations. */
T_byte8 *G_lightCalculations ;

/* Internal prototypes: */
static T_void ILightIlluminate(T_byte8 *p_lightList) ;

// Append a 16-bit value to the end of the light script
static T_void ILSAppend(T_lightTableStruct *p_light, T_word16 aValue)
{
    T_word16 *p_more;
    // Are we out of room?
    if (p_light->tableLen >= p_light->tableAllocLen) {
        // Yes, need more space.  Alloc more space
        p_more = MemAlloc(p_light->tableLen+1024);
        memcpy(p_more, p_light->p_table, p_light->tableLen);
        MemFree(p_light->p_table);
        p_light->p_table = p_more;
        p_light->tableAllocLen += 1024;
    }

    // We have enough room (now), let's append to the end this value
    *((T_word16 *)(((T_byte8 *)(p_light->p_table))+p_light->tableLen)) = aValue;
    p_light->tableLen += 2;
}

static T_word16 AddToList(
             T_word16 *p_list,
             T_word16 max,
             T_word16 numItems,
             T_word16 item)
{
    T_word16 i ;

    DebugRoutine("AddToList") ;

    for (i=0; i<numItems; i++)  {
        if (p_list[i] == item)
            break ;
    }

    if (i == numItems)  {
        //printf("Placing i:%d of %d (item: %d)\n", i, numItems, item) ;  fflush(stdout) ;
        if (numItems < max)  {
            p_list[i] = item ;
            numItems++ ;
        } else {
            DebugCheck(FALSE) ;
        }
    }

    DebugEnd() ;

    return numItems ;
}

static T_word16 FindAdjacentSectors(
             T_word16 sector,
             T_word16 *p_list,
             T_word16 max)
{
    T_word16 line ;
    T_3dLine *p_line ;
    T_3dSide *p_side1 ;
    T_3dSide *p_side2 ;
    T_word16 numItems = 0 ;

    DebugRoutine("FindAdjacentSectors") ;

    for (line=0; line<G_Num3dLines; line++)  {
        //printf("line: %05d\r", line) ; fflush(stdout);
        p_line = G_3dLineArray + line ;
        if (p_line->flags & LINE_IS_TWO_SIDED)  {
            p_side1 = G_3dSideArray + p_line->side[0] ;
            p_side2 = G_3dSideArray + p_line->side[1] ;
            if (p_side1->sector == sector)
                numItems = AddToList(p_list, max, numItems, p_side2->sector) ;
            if (p_side2->sector == sector)
                numItems = AddToList(p_list, max, numItems, p_side1->sector) ;
        }
    }

    DebugEnd() ;

    return numItems ;
}

static T_word16 RemoveSectorFromList(T_word16 *p_list, T_word16 num, T_word16 item)
{
    T_word16 i ;
    T_word16 j ;

    /* Go through the list and remove all the instances of item. */
    for (i=j=0; i<num; i++)  {
        if (item != p_list[i])  {
            p_list[j] = p_list[i] ;
            j++ ;
        }
    }

    return j ;
}

static int QSort16Compare(const void *p_data1, const void *p_data2)
{
    T_word16 data1, data2 ;

    data1 = *((T_word16 *)p_data1) ;
    data2 = *((T_word16 *)p_data2) ;

    if (data1 == data2)
        return 0 ;

    if (data1 < data2)
        return -1 ;

    return 1 ;
}

T_void LightScriptCreateForSector(T_lightTableStruct *p_light, T_word16 sector)
{
    E_Boolean isDoor = FALSE;
    T_word16 adjacentSectors[1000];
    T_word16 adjacentToDoor[1000];
    T_word16 adjDoors[100];
    T_word16 numAdjDoors = 0;
    T_word16 numAdjacent;
    T_word16 numDoorAdjacent;
    T_word16 percent;
    T_word16 percentRemain;
    E_Boolean isOutside = FALSE;
    T_3dSector *p_sector;
    T_word16 i, j;
    T_word16 reg = 0;
    T_word16 reg2;
#define LIGHT_SCRIPT_REGISTER(x)                 (0x8000|(x))
#define LIGHT_SCRIPT_REGISTER_SKY                LIGHT_SCRIPT_REGISTER(10)
#define LIGHT_SCRIPT_MODIFIER_100_PERCENT        256
#define LIGHT_SCRIPT_END_OF_FORMULA              0xFFFF
#define LIGHT_SCRIPT_MODIFIER_DOOR(door)         (0x8000|(door))
#define LIGHT_SCRIPT_END_OF_TABLE                0xFFFF

    DebugRoutine("LightScriptOutputSector");
    DebugCheck(sector < G_Num3dSectors);

    p_sector = G_3dSectorArray + sector;
    // Is this a sky sector?
    if ((strncmp(p_sector->ceilingTx, "F_SKY", 5) == 0)
            || (p_sector->trigger & 1)) {
        //fprintf(fp, "S%d = OUTSIDE * 100%% .\n", sector) ;

        // This is a sky sector, so just output 100% of the SKY register
        ILSAppend(p_light, sector);
        ILSAppend(p_light, LIGHT_SCRIPT_REGISTER_SKY);
        ILSAppend(p_light, LIGHT_SCRIPT_MODIFIER_100_PERCENT);
        ILSAppend(p_light, LIGHT_SCRIPT_END_OF_FORMULA);
    } else {
        // No sky.  Are we a door?
        isDoor = DoorIsAtSector(sector);

//puts("Finding adjacent") ; fflush(stdout) ;
        numAdjacent = FindAdjacentSectors(sector, adjacentSectors, 1000);

        // Do we have any adjacent sectors?
        if (numAdjacent > 0) {
            // At least one?
            if (numAdjacent > 1) {
                // Sort them by sector number (nice and orderly this way)
                qsort(adjacentSectors, numAdjacent, sizeof(T_word16),
                        QSort16Compare);
            }

            // Let's look at those adjacent sectors
            for (i = 0; i < numAdjacent; i++) {
                // Is this adjacent sector a door?
                if (DoorIsAtSector(adjacentSectors[i])) {
                    // Yes, a door.  How many are adjacent to this door?
                    numDoorAdjacent = FindAdjacentSectors(adjacentSectors[i],
                            adjacentToDoor, 1000);
                    // Any?
                    if (numDoorAdjacent > 0) {
                        // One?
                        if (numDoorAdjacent > 1) {
                            // Sort this list back
                            qsort(adjacentToDoor, numDoorAdjacent,
                                    sizeof(T_word16), QSort16Compare);
                        }

                        // Now remove the common list.  Don't want a loop of logic
                        numDoorAdjacent = RemoveSectorFromList(adjacentToDoor,
                                numDoorAdjacent, sector);

                        // How many now?  At least one?
                        if (numDoorAdjacent > 0) {
                            // Okay, let's out the registers for these intermediate
                            // values.  NOTE: There is a limit of 10 doors adjacent to
                            // a single sector!
                            adjDoors[numAdjDoors++] = adjacentSectors[i];
                            //fprintf(fp, "R%d = ", reg);
                            ILSAppend(p_light, LIGHT_SCRIPT_REGISTER(reg));

                            // Divide the sectors evenly between the doors
                            percentRemain = 100;
                            for (j = 0; j < numDoorAdjacent; j++) {
                                percent = percentRemain / (numDoorAdjacent - j);
                                percentRemain -= percent;

                                //fprintf(fp, "S%d * %d%% ", adjacentToDoor[j],
                                //        percent);
                                //fflush(fp);
                                // Output the sector multiplied by the percent
                                ILSAppend(p_light, adjacentToDoor[j]);
                                ILSAppend(p_light, (percent<<8)/100);

                                // More?
                                //if ((j + 1) == numDoorAdjacent) {
                                //} else {
                                //    fprintf(fp, "+ ");
                                //}
                            }
                            //fprintf(fp, ".\n");
                            // Got to the end, tag the end
                            ILSAppend(p_light, LIGHT_SCRIPT_END_OF_FORMULA);

                            // Now generate the formula for the register based on
                            // the door.
                            //fprintf(fp, "R%d = R%d * D%d .\n", reg, reg,
                            //        adjacentSectors[i]);
                            ILSAppend(p_light, LIGHT_SCRIPT_REGISTER(reg));
                            ILSAppend(p_light, LIGHT_SCRIPT_REGISTER(reg));
                            ILSAppend(p_light, LIGHT_SCRIPT_MODIFIER_DOOR(adjacentSectors[i]));
                            ILSAppend(p_light, LIGHT_SCRIPT_END_OF_FORMULA);

                            // Next register (for door calculations)
                            reg++;
                        }
                    }
                }
            }

            // Note where we are in the registers
            reg2 = reg;

            // Let's create the resulting formula (doors use registers, sectors use
            // final values).
            if (isDoor) {
                //fprintf(fp, "R%d = ", reg2);
                ILSAppend(p_light, LIGHT_SCRIPT_REGISTER(reg2));
            } else {
                //fprintf(fp, "S%d = ", sector);
                ILSAppend(p_light, sector);
            }

//printf("numAdj: %d\n", numAdjacent) ; fflush(stdout) ;
            // Now that we've calculated all the adjacent adjacent sectors,
            // let's do the final round for this sector
            percentRemain = 100;
            reg = 0;
            for (i = 0; i < numAdjacent; i++) {
                percent = percentRemain / (numAdjacent - i);
                percentRemain -= percent;

                // Is this an adjacent door or sector?
                if (adjDoors[reg] == adjacentSectors[i]) {
                    // Doors use previously calculated registers
                    //fprintf(fp, "R%d * %d%% ", reg, percent);
                    ILSAppend(p_light, LIGHT_SCRIPT_REGISTER(reg));
                    ILSAppend(p_light, (percent<<8)/100);
                    reg++;
                } else {
                    // Sectors use there current values
                    //fprintf(fp, "S%d * %d%% ", adjacentSectors[i], percent);
                    ILSAppend(p_light, adjacentSectors[i]);
                    ILSAppend(p_light, (percent<<8)/100);
                }
                //fflush(fp);

                //if ((i + 1) == numAdjacent) {
                //} else {
                //    fprintf(fp, "+ ");
                //}
            }

            //fprintf(fp, "\.\n");
            // End the formula for this one sector
            ILSAppend(p_light, LIGHT_SCRIPT_END_OF_FORMULA);

            if (isDoor) {
                //fprintf(fp, "S%d = R%d * D%d .\n", sector, reg2, sector);
                // The door itself gets it's own overriding formula which
                // darkens underneath it as it closes down.
                ILSAppend(p_light, sector);
                ILSAppend(p_light, LIGHT_SCRIPT_REGISTER(reg2));
                ILSAppend(p_light, LIGHT_SCRIPT_MODIFIER_DOOR(sector));
                ILSAppend(p_light, LIGHT_SCRIPT_END_OF_FORMULA);
            }
        }
    }

    DebugEnd();
}

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

    DebugRoutine("LightTableLoad") ;
    /* Allocate a structure for a light table handle. */
    p_light = MemAlloc(sizeof(T_lightTableStruct)) ;
    DebugCheck(p_light != NULL) ;

    /* Make sure we got the handle. */
    if (p_light)  {
#if 0
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
#else
        // Instead, let's generate the .LIT file at load time
        T_word16 sector ;
        p_light->p_table = MemAlloc(1024);
        p_light->tableLen = 0;
        p_light->tableAllocLen = 1024;

        for (sector=0; sector<G_Num3dSectors; sector++)  {
            LightScriptCreateForSector(p_light, sector) ;
        }
        // And close out the final script field
        ILSAppend(p_light, LIGHT_SCRIPT_END_OF_TABLE);

        p_light->tag = LIGHT_TAG;
        /* Allocate memory for the intermeditate calculations. */
        G_lightCalculations = MemAlloc(G_Num3dSectors) ;
        DebugCheck(G_lightCalculations != NULL) ;
        G_preLightCalculations = MemAlloc(G_Num3dSectors) ;
        DebugCheck(G_preLightCalculations != NULL) ;
#endif
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
#if 0
            PictureUnlockAndUnfind(p_light->res) ;
#else
            MemFree(p_light->p_table);
#endif
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
