/*-------------------------------------------------------------------------*
 * File:  OBJMOVE.C
 *-------------------------------------------------------------------------*/
/**
 * The Object Movement subsystem is where all the handling of objects
 * occurs.  You can think of it as "object physics".  It handles objects
 * sliding against walls and other objects.  It does the actual movement
 * of objects in the world.  It even determines what sector(s) an object
 * is on/in.
 *
 * @addtogroup OBJMOVE
 * @brief Object Movement Subsystem
 * @see http://www.amuletsandarmor.com/AALicense.txt
 * @{
 *
 *<!-----------------------------------------------------------------------*/
#include "3D_COLLI.H"
#include "3D_IO.H"
#include "3D_TRIG.H"
#include "3D_VIEW.H"
#include "CRELOGIC.H"
#include "EFFECT.H"
#include "MAP.H"
#include "OBJECT.H"
#include "OBJMOVE.H"
#include "PLAYER.H"
#include "STATS.H"
#include "VIEWFILE.H"

#define OBJ_MOVE_STANDARD_MAX_VELOCITY 24

/* Internal prototypes: */
T_void IObjMoveClipVelocity(T_objMoveStruct *ObjMoveStruct) ;
T_3dObject *ObjMoveFindHighestObject(T_objMoveStruct *ObjMoveStruct) ;
T_3dObject *ObjMoveFindLowestObject(T_objMoveStruct *ObjMoveStruct) ;

/*-------------------------------------------------------------------------*
 * Routine:  ObjMoveUpdate
 *-------------------------------------------------------------------------*/
/**
 *  ObjMoveUpdate is called on objects to update any movement that they
 *  might have due to accelerations in any X, Y, or Z direction.   Since
 *  all moveable items in the engine are objects (monsters, players, &
 *  items), this routine should handle ALL movement in the game.  Note
 *  that if you are not using this routine, you are doing it wrong.
 *
 *  NOTE: 
 *  6/14/95 Still using Stats variables for all objects.
 *
 *  @param ObjMoveStruct -- Pointer to object to move
 *  @param delta -- Delta in time
 *
 *<!-----------------------------------------------------------------------*/
T_void ObjMoveUpdate(T_objMoveStruct *ObjMoveStruct, T_sword32 delta)
{
	T_sword32 newX, newY, newZ;
    E_Boolean status ;
    T_sword16 highFloor, lowCeiling ;
    T_word16 i ;
    T_word16 sector ;
    T_sword16 floor, ceiling ;
    T_word16 highFloorSector, lowCeilingSector ;
    T_word16 areaSector ;
    T_word16 terrainFriction ;
    T_sword32 gravity ;
    T_3dObject *p_objectOver ;
    T_sword16 objectHeight ;
    T_sword32 xDec, yDec ;
    E_Boolean isPlayer ;
    T_sword16 top ;
    T_3dSectorInfo *p_sectorInfo ;
    T_sword16 limit ;
    DebugRoutine("ObjMoveUpdate") ;

    /* Check to see if this objmove needs to be updated. */
    if (ObjMoveStruct->Flags & OBJMOVE_FLAG_PLEASE_UPDATE)  {
/*
p_obj = (T_3dObject *)ObjMoveStruct ;
if (ObjectIsCreature(p_obj))
  printf("ObjMoveUpdate creature %d from %08lX %08lX %08lX to \n",
    ObjectGetServerId(p_obj),
    ObjectGetX(p_obj),
    ObjectGetY(p_obj),
    ObjectGetZ(p_obj)) ;
*/
#ifndef SERVER_ONLY
        isPlayer = (PlayerGetObject() == ((T_3dObject *)ObjMoveStruct)) ;
#else
        isPlayer = FALSE ;
#endif

        /* If we are the player, only do normal collision. */
        if (isPlayer)
            Collide3dSetWallDefinition(LINE_IS_IMPASSIBLE) ;

        /* Note that we are moving. */
        ObjMoveStruct->Flags |= OBJMOVE_FLAG_MOVED ;

        p_sectorInfo = G_3dSectorInfoArray + ObjMoveStruct->AreaSector ;

        /* Flow the object in a direction.  */
        /* Don't flow if this object isn't allowed to  */
        if (!(ObjMoveStruct->Flags & OBJMOVE_FLAG_DOES_NOT_FLOW))  {
            /* Don't flow if this sector */
            /* is water or lava and we are flying above it. */
            if (((p_sectorInfo->type == SECTOR_TYPE_LAVA) ||
                 (p_sectorInfo->type == SECTOR_TYPE_WATER)) &&
                  ((ObjMoveStruct->Z>>16) <= G_3dSectorArray[ObjMoveStruct->AreaSector].floorHt))  {
                /* Flow if feet in lave or water. */
                ObjMoveStruct->XV += (((T_sword32)p_sectorInfo->xVelAdd)<<12)*delta ;
                ObjMoveStruct->YV += (((T_sword32)p_sectorInfo->yVelAdd)<<12)*delta ;
                ObjMoveStruct->ZV += (((T_sword32)p_sectorInfo->zVelAdd)<<8)*delta ;
            } else if ((p_sectorInfo->type != SECTOR_TYPE_LAVA) &&
                       (p_sectorInfo->type != SECTOR_TYPE_WATER))  {
                /* Flow if flying over non-water or non-lava sector. */
                ObjMoveStruct->XV += (((T_sword32)p_sectorInfo->xVelAdd)<<12)*delta ;
                ObjMoveStruct->YV += (((T_sword32)p_sectorInfo->yVelAdd)<<12)*delta ;
                ObjMoveStruct->ZV += (((T_sword32)p_sectorInfo->zVelAdd)<<8)*delta ;
            }
        }

        IObjMoveClipVelocity(ObjMoveStruct) ;

//printf("%d ObjMoveUpdate %d from (%08lX %08lX %08lX)\n", SyncTimeGet(), ObjectGetServerId((T_3dObject *)ObjMoveStruct), ObjMoveStruct->X, ObjMoveStruct->Y, ObjMoveStruct->Z) ;
        /* Find where we are going to next. */
        newX = ObjMoveStruct->X ;
        newY = ObjMoveStruct->Y ;
        newZ = ObjMoveStruct->Z ;
/*
if (ObjectIsCreature(p_obj))
  printf(" vels %08lX %08lX %08lX %d\n",
    ObjMoveStruct->XV,
    ObjMoveStruct->YV,
    ObjMoveStruct->ZV,
    delta) ;
*/
        /* Add in the regular velocity movement. */
        newX += ((ObjMoveStruct->XV*delta) >> 3) ;
        newY += ((ObjMoveStruct->YV*delta) >> 3) ;
        newZ += ((ObjMoveStruct->ZV*delta) >> 3) ;
/*
if (ObjectIsCreature(p_obj))
  printf(" xvels %08lX %08lX %08lX %d\n",
    ObjMoveStruct->XVExternal,
    ObjMoveStruct->YVExternal,
    ObjMoveStruct->ZVExternal,
    delta) ;
*/
        /* Add in the external velocity movement. */
        newX += ObjMoveStruct->XVExternal ;
        newY += ObjMoveStruct->YVExternal ;
        newZ += ObjMoveStruct->ZVExternal ;

        /* External velocity always is put back to zero. */
        ObjMoveStruct->XVExternal = 0 ;
        ObjMoveStruct->YVExternal = 0 ;
        ObjMoveStruct->ZVExternal = 0 ;

        /* Check to see if we really needed to update this */
        /* object. */
        if ((ObjMoveStruct->X == newX) &&
            (ObjMoveStruct->Y == newY) &&
            (ObjMoveStruct->Z == newZ))  {
            ObjMoveStruct->Flags &= (~OBJMOVE_FLAG_PLEASE_UPDATE) ;
        }
        /* Make movement bassed on this object (in other words, */
        /* don't collide with oneself. */
        View3dSetExceptObjectByPtr(ObjMoveStruct) ;

        /* Try moving to the new location. */
        status = Collide3dMoveToXYZ(
                     ((T_3dObject *)ObjMoveStruct),
                     newX,
                     newY,
                     newZ) ;

if (status)  {
    /* Hit something while going fast, slow down */
    /* and try again. */
    status = (View3dMoveToXY(
            &ObjMoveStruct->X,
            &ObjMoveStruct->Y,
            newX,
            newY,
            ObjMoveStruct->Radius,
            ObjMoveStruct->Z/*+(ObjMoveStruct->climbHeight<<16)*/,
            ObjMoveStruct->Z + (ObjMoveStruct->Height<<16),
            ObjMoveGetHeight(ObjMoveStruct),
            ((T_3dObject *)ObjMoveStruct)))?TRUE:FALSE ;
/*
if (ObjectIsCreature(p_obj))
  printf(" hit new loc %08lX %08lX\n",
    ObjMoveStruct->X,
    ObjMoveStruct->Y) ;
*/
}
//printf("  to (%08lX %08lX %08lX) status: %d\n", ObjMoveStruct->X, ObjMoveStruct->Y, ObjMoveStruct->Z, status) ;
        /* Update the "hit" flag. */
        if (status == FALSE)  {
//            ObjMoveStruct->Flags &= (~OBJMOVE_FLAG_BLOCKED) ;
        } else {
/*
if (ObjectIsCreature((T_3dObject *)ObjMoveStruct))
  printf("ObjMoveUpdate: %d blocked (from %s)\n",
    ObjectGetServerId((T_3dObject *)ObjMoveStruct),
    DebugGetCallerName()) ;
*/
            ObjMoveStruct->Flags |= (OBJMOVE_FLAG_BLOCKED) ;
        }

        /* We've moved.  Record what sectors and such we are */
        /* over. */
        Collide3dGetSectorsInBox(
            ObjMoveStruct->X,
            ObjMoveStruct->Y,
            ObjMoveStruct->Radius,
            MAX_OBJECT_SECTORS,
            ObjMoveStruct->OnSectors,
            &ObjMoveStruct->numOnSectors) ;

        /* Record what is the center sector. */
        ObjMoveStruct->CenterSector = ObjMoveStruct->OnSectors[0] ;

        /* Find the highest floor and ceiling in the area that */
        /* we are currently in. */
        highFloor = -32000 ;
        lowCeiling = 32000 ;
        highFloorSector = 0xFFFF ;
        lowCeilingSector = 0xFFFF ;
        for (i=0; i<ObjMoveStruct->numOnSectors; i++)  {
            sector = ObjMoveStruct->OnSectors[i] ;

            /* Find the floor height and record if higher. */
//            floor = G_3dSectorArray[sector].floorHt ;
//            if (G_3dSectorArray[sector].trigger & SECTOR_DIP_FLAG)
//                floor -= VIEW_WATER_DIP_LEVEL ;
            floor = MapGetWalkingFloorHeight(ObjMoveStruct, sector) ;
            if (floor > highFloor)  {
                highFloor = floor ;
                highFloorSector = sector ;
            }

            /* Find the ceiling height and record if lower. */
            ceiling = G_3dSectorArray[sector].ceilingHt ;
            limit = G_3dSectorInfoArray[sector].ceilingLimit ;
            if (limit < ceiling)
                ceiling = limit ;
            if (ceiling < lowCeiling)  {
                lowCeiling = ceiling ;
                lowCeilingSector = sector ;
            }
        }

        /* Record floor only if the floor is legal. */
        if (highFloorSector != 0xFFFF)  {
            /* Record the piece of floor that we are over. */
            ObjMoveStruct->AreaSector = highFloorSector ;
        }

        /* Record ceiling only if the ceiling is legal. */
        if (lowCeilingSector != 0xFFFF)  {
            /* Record the piece of ceiling that we are under. */
            ObjMoveStruct->AreaCeilingSector = lowCeilingSector ;
        }

        /* Check to see if we are on top of any objects. */
        if (!ObjectIsFullyPassable((T_3dObject *)ObjMoveStruct))  {
            p_objectOver = ObjMoveFindHighestObject(ObjMoveStruct) ;
            if (p_objectOver != NULL)  {
                objectHeight =
                    ObjectGetZ16(p_objectOver) +
                    ObjectGetHeight(p_objectOver) ;
                if ((objectHeight > highFloor) &&
                     ((objectHeight +
                         ObjMoveGetHeight(ObjMoveStruct)) < ceiling))
                    highFloor = objectHeight ;
            }
        }
        ObjMoveStruct->lowestPoint = highFloor<<16 ;

        /* Check to see if we are on top of any objects. */
        if (!ObjectIsFullyPassable((T_3dObject *)ObjMoveStruct))  {
            p_objectOver = ObjMoveFindLowestObject(ObjMoveStruct) ;
            if (p_objectOver != NULL)  {
                objectHeight = ObjectGetZ16(p_objectOver) ;
                if (objectHeight < lowCeiling)
                    lowCeiling = objectHeight ;
            }
        }
        ObjMoveStruct->highestPoint = lowCeiling<<16 ;

        /* Now that we know what sector we are standing on, */
        /* get the friction and gravity for where we just went to. */
        areaSector = ObjMoveStruct->AreaSector ;
        if (ObjMoveStruct->Flags & OBJMOVE_FLAG_FORCE_NORMAL_FRICTION)
            terrainFriction = OBJMOVE_NORMAL_FRICTION ;
        else
            terrainFriction = G_3dSectorInfoArray[areaSector].friction ;

        gravity = -G_3dSectorInfoArray[areaSector].gravity ;

        /* Is our feet on the ground, or are we falling? */
        if (ObjMoveIsAboveGround(ObjMoveStruct)==FALSE)  {
            /* Are we to ignore friction? */
            if (!(ObjMoveGetFlags(ObjMoveStruct) &
                    OBJMOVE_FLAG_IGNORE_FRICTION))  {
                /* Determine how much deceleration we have in the X dir. */
	            xDec = (ObjMoveStruct->XV>>8) * terrainFriction ;

                /* Decelerate the X velocity */
	            ObjMoveStruct->XV -= xDec * delta;

                /* If no deceleration is zero, lets force velocity to zero. */
                if (xDec == 0)
                    ObjMoveStruct->XV = 0;

                /* Determine how much deceleration we have in the Y dir. */
	            yDec = (ObjMoveStruct->YV>>8) * terrainFriction ;

                /* Decelerate the Y velocity */
	            ObjMoveStruct->YV -= yDec * delta;

                /* If no deceleration is zero, lets force velocity to zero. */
                if (yDec == 0)
                    ObjMoveStruct->YV = 0;
            }
            /* Since we are not falling, make sure the falling */
            /* velocity is zero and we are standing on the ground. */
            if (!(ObjMoveStruct->Flags & OBJMOVE_FLAG_IGNORE_GRAVITY))  {
                ObjMoveStruct->ZV = 0;

#if 0
                /* Can we climb to the next height?  If so, do it. */
                if ((highFloor - ObjMoveStruct->climbHeight) <=
                    (ObjMoveStruct->Z>>16))  {
                    ObjMoveStruct->Z = (((T_sword32)floor)<<16);
                }
#endif
            }
            /* Check to see if we are going (about to go) through the roof. */
            top = (ObjMoveStruct->Z>>16) + ObjMoveStruct->Height ;
            if (top > lowCeiling)  {
                /* Are we also going up? */
                if (ObjMoveStruct->ZV > 0)
                    /* If so, reverse the z velocity and dampen. */
                    ObjMoveStruct->ZV = -(ObjMoveStruct->ZV/2) ;

                /* Unfortunately, we don't want to force the object into */
                /* ground, so, we'll just leave him at the wrong height. */
                /* If this is a character, the calling routine will have */
                /* to check this also to lower the view. */
                top = lowCeiling-ObjMoveStruct->Height ;
                if (top < highFloor)
                    top = highFloor;
                ObjMoveStruct->Z = (((T_sword32)top)<<16) ;
            }
        } else {
            if (!(ObjMoveStruct->Flags & OBJMOVE_FLAG_IGNORE_Z_UPDATES))
                ObjMoveUpdateZVel(ObjMoveStruct, delta) ;
        }

       if (ObjMoveStruct->Flags & OBJMOVE_FLAG_STICK_TO_CEILING)  {
           /* Hang it from the ceiling.  Just pin it based on the center. */
           ObjMoveStruct->Z =
               (((T_sword32)
                   (MapGetCeilingHeight(
                       ObjMoveStruct->AreaCeilingSector) -
                       ObjMoveStruct->Height))<<16) ;
       }
    }
    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ObjMoveInit
 *-------------------------------------------------------------------------*/
/**
 *  ObjMoveInit is used to initialize an object's movement information.
 *
 *  @param ObjMoveStruct -- Pointer to object move info to init
 *
 *<!-----------------------------------------------------------------------*/
T_void ObjMoveInit (T_objMoveStruct *ObjMoveStruct)
{
	DebugRoutine ("ObjMoveInit");
	DebugCheck (ObjMoveStruct != NULL);

	if (ObjMoveStruct !=NULL)
	{
        /* Make all the variables zero, until further ado. */
        memset(ObjMoveStruct, 0, sizeof(T_objMoveStruct)) ;
		ObjMoveStruct->Z=-0x7FFFFFFE;
        ObjMoveStruct->lowestPoint = -0x7FFF ;
        ObjMoveStruct->highestPoint = 0x7FFF0000 ;
		ObjMoveStruct->Radius=32;
        ObjMoveStruct->maxVelocity = OBJ_MOVE_STANDARD_MAX_VELOCITY ;
        ObjMoveStruct->lastSound = 0 ;
	}

	DebugEnd();
}

T_void ObjMoveSetXYZ (T_objMoveStruct *ObjMoveStruct,
						T_sword32 newx,
						T_sword32 newy,
						T_sword32 newz)
{
	DebugRoutine ("ObjMoveSetXYZ");
	DebugCheck (ObjMoveStruct != NULL);

	if (ObjMoveStruct != NULL)
	{
		ObjMoveStruct->X=newx;
		ObjMoveStruct->Y=newy;
		ObjMoveStruct->Z=newz;
	}

	DebugEnd();
}


#ifndef FAST_OBJMOVE
T_void ObjMoveSetX (T_objMoveStruct *ObjMoveStruct,
					  T_sword32 newx)
{
	DebugRoutine ("ObjMoveSetX");
	DebugCheck (ObjMoveStruct !=NULL);

	if (ObjMoveStruct != NULL)
	{
		ObjMoveStruct->X=newx;
	}

	DebugEnd();
}


T_void ObjMoveSetY (T_objMoveStruct *ObjMoveStruct,
					  T_sword32 newy)
{
	DebugRoutine ("ObjMoveSetY");
	DebugCheck (ObjMoveStruct !=NULL);

	if (ObjMoveStruct != NULL)
	{
		ObjMoveStruct->Y=newy;
	}

	DebugEnd();
}


T_void ObjMoveSetZ (T_objMoveStruct *ObjMoveStruct,
					  T_sword32 newz)
{
	DebugRoutine ("ObjMoveSetZ");
	DebugCheck (ObjMoveStruct !=NULL);

	if (ObjMoveStruct != NULL)
	{
		ObjMoveStruct->Z=newz;
	}

	DebugEnd();
}


T_void ObjMoveSetFlags (T_objMoveStruct *ObjMoveStruct,
						  T_word16 newflags)
{
	DebugRoutine ("ObjMoveSetFlags");
	DebugCheck (ObjMoveStruct !=NULL);

	if (ObjMoveStruct != NULL)
	{
		ObjMoveStruct->Flags=newflags;
	}

	DebugEnd();
}


T_void ObjMoveSetAngle (T_objMoveStruct *ObjMoveStruct,
						T_word16 newAngle)
{
	DebugRoutine ("ObjMoveSetAngle");
	DebugCheck (ObjMoveStruct !=NULL);

	if (ObjMoveStruct != NULL)
	{
		ObjMoveStruct->Angle=newAngle;
	}

	DebugEnd();
}

T_void ObjMoveAddAngle (T_objMoveStruct *ObjMoveStruct,
						T_word16 addAngle)
{
	DebugRoutine ("ObjMoveAddAngle");
	DebugCheck (ObjMoveStruct !=NULL);

	if (ObjMoveStruct != NULL)
	{
		ObjMoveStruct->Angle+=addAngle;
	}

	DebugEnd();
}


T_void ObjMoveSetHeight (T_objMoveStruct *ObjMoveStruct,
						 T_sword32 newHeight)
{
	DebugRoutine ("ObjMoveSetHeight");
	DebugCheck (ObjMoveStruct !=NULL);

	if (ObjMoveStruct != NULL)
	{
		ObjMoveStruct->Height=newHeight;
        ObjMoveStruct->climbHeight = (newHeight>>2) ;
	}

	DebugEnd();
}


T_void ObjMoveSetRadius(T_objMoveStruct *ObjMoveStruct,
						 T_word16 newRadius)
{
	DebugRoutine ("ObjMoveSetRadius");
	DebugCheck (ObjMoveStruct !=NULL);

	if (ObjMoveStruct != NULL)
	{
		ObjMoveStruct->Radius=newRadius;
	}

	DebugEnd();
}



T_sword32 ObjMoveGetX (T_objMoveStruct *ObjMoveStruct)
{
	T_sword32 value;

	DebugRoutine ("ObjMoveGetX");
	DebugCheck (ObjMoveStruct !=NULL);

	if (ObjMoveStruct != NULL)
	{
		value=ObjMoveStruct->X;
	}

	DebugEnd();
	return (value);
}


T_sword32 ObjMoveGetY (T_objMoveStruct *ObjMoveStruct)
{
	T_sword32 value;

	DebugRoutine ("ObjMoveGetY");
	DebugCheck (ObjMoveStruct !=NULL);

	if (ObjMoveStruct != NULL)
	{
		value=ObjMoveStruct->Y;
	}

	DebugEnd();
	return (value);
}


T_sword32 ObjMoveGetZ (T_objMoveStruct *ObjMoveStruct)
{
	T_sword32 value;

	DebugRoutine ("ObjMoveGetZ");
	DebugCheck (ObjMoveStruct!=NULL);

	if (ObjMoveStruct != NULL)
	{
		value=ObjMoveStruct->Z;
	}

	DebugEnd();
	return (value);
}

T_sword32 ObjMoveGetXVel (T_objMoveStruct *ObjMoveStruct)
{
	T_sword32 value;

	DebugRoutine ("ObjMoveGetXVel");
	DebugCheck (ObjMoveStruct !=NULL);

	if (ObjMoveStruct != NULL)
	{
		value=ObjMoveStruct->XV;
	}

	DebugEnd();
	return (value);
}


T_sword32 ObjMoveGetYVel (T_objMoveStruct *ObjMoveStruct)
{
	T_sword32 value;

	DebugRoutine ("ObjMoveGetYVel");
	DebugCheck (ObjMoveStruct !=NULL);

	if (ObjMoveStruct != NULL)
	{
		value=ObjMoveStruct->YV;
	}

	DebugEnd();
	return (value);
}


T_sword32 ObjMoveGetZVel (T_objMoveStruct *ObjMoveStruct)
{
	T_sword32 value;

	DebugRoutine ("ObjMoveGetZVel");
	DebugCheck (ObjMoveStruct!=NULL);

	if (ObjMoveStruct != NULL)
	{
		value=ObjMoveStruct->ZV;
	}

	DebugEnd();
	return (value);
}



T_word16 ObjMoveGetAngle (T_objMoveStruct *ObjMoveStruct)
{
	T_word16 value;

	DebugRoutine ("ObjMoveGetAngle");
	DebugCheck (ObjMoveStruct!=NULL);

	if (ObjMoveStruct != NULL)
	{
		value=ObjMoveStruct->Angle;
	}

	DebugEnd();
	return (value);
}


T_word16 ObjMoveGetHeight (T_objMoveStruct *ObjMoveStruct)
{
	T_word16 value;

	DebugRoutine ("ObjMoveGetHeight");
	DebugCheck (ObjMoveStruct!=NULL);

	if (ObjMoveStruct != NULL)
	{
		value=ObjMoveStruct->Height;
	}

	DebugEnd();
	return (value);
}


T_word16 ObjMoveGetRadius (T_objMoveStruct *ObjMoveStruct)
{
	T_word16 value;

	DebugRoutine ("ObjMoveGetRadius");
	DebugCheck (ObjMoveStruct!=NULL);

	if (ObjMoveStruct != NULL)
	{
		value=ObjMoveStruct->Radius;
	}

	DebugEnd();
	return (value);
}


T_word16 ObjMoveGetFlags (T_objMoveStruct *ObjMoveStruct)
{
	T_word16 value;

	DebugRoutine ("ObjMoveGetFlags");
	DebugCheck (ObjMoveStruct!=NULL);

	if (ObjMoveStruct != NULL)
	{
		value=ObjMoveStruct->Flags;
	}

	DebugEnd();
	return (value);
}


T_word16 ObjMoveGetAreaSector (T_objMoveStruct *ObjMoveStruct)
{
    T_word16 sector ;

	DebugRoutine ("ObjMoveGetAreaSector");
	DebugCheck (ObjMoveStruct != NULL);

	if (ObjMoveStruct !=NULL)
	{
        sector = ObjMoveStruct->AreaSector ;
        DebugCheck(sector < G_Num3dSectors) ;
    } else {
        sector = 0 ;
    }

    DebugEnd() ;
    return sector ;
}

T_word16 ObjMoveGetAreaCeilingSector (T_objMoveStruct *ObjMoveStruct)
{
    T_word16 sector ;

	DebugRoutine ("ObjMoveGetAreaCeilingSector");
	DebugCheck (ObjMoveStruct != NULL);

	if (ObjMoveStruct !=NULL)
	{
        sector = ObjMoveStruct->AreaCeilingSector ;
        DebugCheck(sector < G_Num3dSectors) ;
    } else {
        sector = 0 ;
    }

    DebugEnd() ;
    return sector ;
}

T_word16 ObjMoveGetCenterSector (T_objMoveStruct *ObjMoveStruct)
{
    T_word16 sector ;

	DebugRoutine ("ObjMoveGetCenterSector");
	DebugCheck (ObjMoveStruct != NULL);

	if (ObjMoveStruct !=NULL)
	{
        sector = ObjMoveStruct->CenterSector ;
//        DebugCheck(sector < G_Num3dSectors) ;
//#ifndef NDEBUG
/*
   if (sector >= G_Num3dSectors)
   {
      printf ("ObjMoveGetCenterSector: bad sector %04X\n", sector);
      printf ("Object ID = %d\n", ObjectGetServerId (((T_3dObject *)ObjMoveStruct)));
      DebugCheck (FALSE);
   }
*/
   if (sector >= G_Num3dSectors)
       sector = 0 ;
//#endif
    } else {
        sector = 0 ;
    }

    DebugEnd() ;
    return sector ;
}

T_word16 ObjMoveGetFriction(T_objMoveStruct *ObjMoveStruct)
{
    T_word16 friction ;

    DebugRoutine("ObjMoveGetFriction") ;

    friction = G_3dSectorInfoArray[ObjMoveStruct->AreaSector].friction ;

    DebugEnd() ;

    return friction ;
}

T_word16 ObjMoveGetNumAreaSectors(T_objMoveStruct *ObjMoveStruct)
{
    T_word16 num ;

    DebugRoutine("ObjMoveGetNumAreaSectors") ;

    num = ObjMoveStruct->numOnSectors ;

    DebugEnd() ;

    return num ;
}

T_word16 ObjMoveGetNthAreaSector(
             T_objMoveStruct *ObjMoveStruct,
             T_word16 n)
{
    T_word16 sector ;

    DebugRoutine("ObjMoveSetNthAreaSector") ;

    sector = ObjMoveStruct->OnSectors[n] ;

    DebugEnd() ;

    return sector ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ObjMoveAddX
 *-------------------------------------------------------------------------*/
/**
 *  ObjMoveAddX is a quick routine to transpose the position along the
 *  X direction.
 *
 *  @param ObjMoveStruct -- object to transpose
 *  @param addx -- amount to add
 *
 *<!-----------------------------------------------------------------------*/
T_void ObjMoveAddX   (T_objMoveStruct *ObjMoveStruct,
						T_sword32 addx)
{
    DebugRoutine("ObjMoveAddX") ;
    DebugCheck(ObjMoveStruct != NULL) ;

    ObjMoveStruct->X += addx ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ObjMoveAddY
 *-------------------------------------------------------------------------*/
/**
 *  ObjMoveAddY is a quick routine to transpose the position along the
 *  Y direction.
 *
 *  @param ObjMoveStruct -- object to transpose
 *  @param addy -- amount to add
 *
 *<!-----------------------------------------------------------------------*/
T_void ObjMoveAddY   (T_objMoveStruct *ObjMoveStruct,
						T_sword32 addy)
{
    DebugRoutine("ObjMoveAddX") ;
    DebugCheck(ObjMoveStruct != NULL) ;

    ObjMoveStruct->Y += addy ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ObjMoveAddZ
 *-------------------------------------------------------------------------*/
/**
 *  ObjMoveAddZ is a quick routine to transpose the position along the
 *  Z direction.
 *
 *  @param ObjMoveStruct -- object to transpose
 *  @param addz -- amount to add
 *
 *<!-----------------------------------------------------------------------*/
T_void ObjMoveAddZ   (T_objMoveStruct *ObjMoveStruct,
						T_sword32 addz)
{
    DebugRoutine("ObjMoveAddZ") ;
    DebugCheck(ObjMoveStruct != NULL) ;

    ObjMoveStruct->Z += addz ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ObjMoveGetMaxVelocity
 *-------------------------------------------------------------------------*/
/**
 *  ObjMoveGetMaxVelocity returns the maximum velocity that the given
 *  object is allowed to move.
 *
 *  @param ObjMoveStruct -- object to get velocity of
 *
 *<!-----------------------------------------------------------------------*/
T_word16 ObjMoveGetMaxVelocity(T_objMoveStruct *ObjMoveStruct)
{
    T_word16 velocity ;

    DebugRoutine("ObjMoveGetMaxVelocity") ;
    DebugCheck(ObjMoveStruct != NULL) ;

    velocity = ObjMoveStruct->maxVelocity ;

    DebugEnd() ;

    return velocity ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ObjMoveSetMaxVelocity
 *-------------------------------------------------------------------------*/
/**
 *  ObjMoveSetMaxVelocity changes the maximum velocity that the given
 *  object is allowed to move.
 *
 *  @param ObjMoveStruct -- object to set velocity of
 *  @param maxVelocity -- new maximum velocity
 *
 *<!-----------------------------------------------------------------------*/
T_void ObjMoveSetMaxVelocity(
           T_objMoveStruct *ObjMoveStruct,
           T_word16 maxVelocity)
{
    DebugRoutine("ObjMoveSetMaxVelocity") ;
    DebugCheck(ObjMoveStruct != NULL) ;

    ObjMoveStruct->maxVelocity = maxVelocity ;

    DebugEnd() ;
}
#endif

#ifndef NDEBUG
/*-------------------------------------------------------------------------*
 * Routine:  ObjMovePrint
 *-------------------------------------------------------------------------*/
/**
 *  ObjMovePrint dumps out the given object move information to the
 *  given file/output.
 *
 *  @param fp -- File to output object move info
 *  @param p_om -- Object move struct to print
 *
 *<!-----------------------------------------------------------------------*/
T_void ObjMovePrint(FILE *fp, T_objMoveStruct *p_om)
{
    T_word16 i ;

    DebugRoutine("ObjMovePrint") ;

    fprintf(fp, "ObjMove: %p\n", p_om) ;
    fprintf(fp, "  for object ID %d\n", ObjectGetServerId (((T_3dObject *)p_om)));
    fprintf(fp, "  X : %08lX\n", p_om->X) ;
    fprintf(fp, "  Y : %08lX\n", p_om->Y) ;
    fprintf(fp, "  Z : %08lX\n", p_om->Z) ;
    fprintf(fp, "  VX: %08lX\n", p_om->XV) ;
    fprintf(fp, "  VY: %08lX\n", p_om->YV) ;
    fprintf(fp, "  VZ: %08lX\n", p_om->ZV) ;
    fprintf(fp, "  Radius: %d\n", p_om->Radius) ;
    fprintf(fp, "  Height: %d\n", p_om->Height) ;
    fprintf(fp, "  Angle : %04X\n", p_om->Angle) ;
    fprintf(fp, "  Flags : %02X\n", p_om->Flags) ;
    fprintf(fp, "  ASect : %d\n", p_om->AreaSector) ;
    fprintf(fp, "  CSect : %d\n", p_om->CenterSector) ;
    fprintf(fp, "  numSec: %d\n", p_om->numOnSectors) ;
    fprintf(fp, "  >") ;
    for (i=0; i<p_om->numOnSectors; i++)
        fprintf(fp, " %d", p_om->OnSectors[i]) ;
    fprintf(fp, "\n") ;
    fprintf(fp, "  maxVel: %d\n", p_om->maxVelocity) ;

    fflush(fp) ;

    DebugEnd() ;
}

#endif

/*-------------------------------------------------------------------------*
 * Routine:  ObjMoveAddExternalVelocity
 *-------------------------------------------------------------------------*/
/**
 *  ObjMoveAddExternalVelocity pushes an object in a direction with
 *  a constant velocity.  Unlike normal velocity, external velocity
 *  is not clipped by the maximum velocity.
 *
 *  @param ObjMoveStruct -- Current obj's move structure
 *  @param dVX -- 32 bit delta X velocity
 *  @param dVY -- 32 bit delta Y velocity
 *  @param dVZ -- 32 bit delta Z velocity
 *
 *<!-----------------------------------------------------------------------*/
T_void ObjMoveAddExternalVelocity(
           T_objMoveStruct *ObjMoveStruct,
           T_sword32 dVX,
           T_sword32 dVY,
           T_sword32 dVZ)
{
	DebugRoutine ("ObjMoveAddExternalVelocity");
	DebugCheck (ObjMoveStruct != NULL);

    ObjMoveStruct->Flags |= OBJMOVE_FLAG_PLEASE_UPDATE ;
    ObjMoveStruct->XVExternal += dVX ;
    ObjMoveStruct->YVExternal += dVY ;
    ObjMoveStruct->ZVExternal += dVZ ;

	DebugEnd();
}

/*-------------------------------------------------------------------------*
 * Routine:  ObjMoveSetAngularVelocity
 *-------------------------------------------------------------------------*/
/**
 *  ObjMoveSetAngularVelocity declares the direction and magnitude that
 *  the object is to take.  Any other velocity that the object had was lost.
 *  Object acceleration is still the same.
 *
 *  @param p_objM -- ObjMove to set angular velocity of
 *  @param angle -- Angle to move toward
 *  @param magnitude -- How fast to go.
 *
 *<!-----------------------------------------------------------------------*/
T_void ObjMoveSetAngularVelocity(
           T_objMoveStruct *p_objM,
           T_word16 angle,
           T_sword16 magnitude)
{
    DebugRoutine("ObjMoveSetAngularVelocity") ;
    DebugCheck(p_objM != NULL) ;

	ObjMoveSetXVel(p_objM, MathCosineLookup(angle)*((T_sword32)magnitude));
    ObjMoveSetYVel(p_objM, MathSineLookup(angle)*((T_sword32)magnitude));
    p_objM->Flags |= OBJMOVE_FLAG_PLEASE_UPDATE ;

    DebugEnd() ;
}

T_void ObjMoveGetXYZ (T_objMoveStruct *ObjMoveStruct,
						T_sword32 *xval,
						T_sword32 *yval,
						T_sword32 *zval)
{
	DebugRoutine ("ObjMoveGetXYZ");
	DebugCheck (ObjMoveStruct !=NULL);

	if (ObjMoveStruct != NULL)
	{
		*xval=ObjMoveStruct->X;
		*yval=ObjMoveStruct->Y;
		*zval=ObjMoveStruct->Z;
	}

	DebugEnd();
}


T_void ObjMoveAccelXYZ (T_objMoveStruct *ObjMoveStruct,
						  T_sword32 XACC,
						  T_sword32 YACC,
						  T_sword32 ZACC)
{
	DebugRoutine ("ObjMoveAccelXYZ");
	DebugCheck (ObjMoveStruct!=NULL);

	if (ObjMoveStruct != NULL)
	{
        ObjMoveStruct->Flags |= OBJMOVE_FLAG_PLEASE_UPDATE ;
		ObjMoveStruct->XV+=XACC;
		ObjMoveStruct->YV+=YACC;
		ObjMoveStruct->ZV+=ZACC;
//        IObjMoveClipVelocity(ObjMoveStruct) ;
	}

	DebugEnd();
}


T_void ObjMoveAccelFlat (T_objMoveStruct *ObjMoveStruct,
						   T_sword32 amount,
						   T_sword32 angle)
{
	DebugRoutine ("ObjMoveAccelFlat");
	DebugCheck (ObjMoveStruct!=NULL);

	if (ObjMoveStruct != NULL)
	{
        /* Ghosts don't move. */
        if ((ObjectGetServerId((T_3dObject *)ObjMoveStruct)/100)!=90)  {
            ObjMoveStruct->Flags |= OBJMOVE_FLAG_PLEASE_UPDATE ;
	    	ObjMoveStruct->XV+=(MathCosineLookup(angle)*amount);
		    ObjMoveStruct->YV+=(MathSineLookup(angle)*amount);
//            IObjMoveClipVelocity(ObjMoveStruct) ;
        }
	}

	DebugEnd();
}


E_Boolean ObjMoveIsAboveGround (T_objMoveStruct *ObjMoveStruct)
{
	E_Boolean status ;
	T_sword32 heightFloor;

	DebugRoutine("ObjMoveIsAboveGround") ;
	DebugCheck (ObjMoveStruct != NULL);

	if (ObjMoveStruct != NULL)
	{
//        if (ObjMoveStruct->AreaSector & 0x8000)  {
            heightFloor = ObjMoveStruct->lowestPoint>>16 ;
//        } else {
            /* What is the height of the floor underneath us. */
//		    heightFloor = G_3dSectorArray[ObjMoveStruct->AreaSector].floorHt ;
//        }

        /* Well, are we above that? */
		if (ObjMoveStruct->Z > (heightFloor<<16))  {
//      if ((ObjMoveStruct->Z>>16) > heightFloor)  {
			status = TRUE ;
		} else  {
			status = FALSE ;
		}
	}

	DebugEnd();
	return (status);
}


T_void ObjMoveStopMoving(T_objMoveStruct *ObjMoveStruct)
{
    DebugRoutine("ObjMoveStopMoving") ;

    /* Halt all X and Y movement. */
    ObjMoveStruct->XV = ObjMoveStruct->YV = 0 ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  IObjMoveClipVelocity
 *-------------------------------------------------------------------------*/
/**
 *  ObjMoveSetMaxVelocity changes the maximum velocity that the given
 *  object is allowed to move.
 *
 *  @param ObjMoveStruct -- object to set velocity of
 *
 *<!-----------------------------------------------------------------------*/
T_void IObjMoveClipVelocity(T_objMoveStruct *ObjMoveStruct)
{
    double velocityMag ;
    double maxVelocity ;
    double xv, yv;

    DebugRoutine("IObjMoveClipVelocity") ;
    DebugCheck(ObjMoveStruct != NULL) ;

    if (!(ObjMoveStruct->Flags & OBJMOVE_FLAG_IGNORE_MAX_VELOCITY))  {
        maxVelocity = (double)ObjMoveStruct->maxVelocity ;
        xv = ObjMoveStruct->XV/65536.0;
        yv = ObjMoveStruct->YV/65536.0;
        velocityMag = sqrt(xv*xv+yv*yv);

        /* Have we gone over the limit? */
        if (velocityMag > maxVelocity)  {
/*            if (velocityMag > 0.1) {
                printf("obj: %d (t: %d) -- velocity is %f (%f, %f), clip to %f\n",
                 ObjectGetServerId((T_3dObject *)ObjMoveStruct),
                 ObjectGetType((T_3dObject *)ObjMoveStruct),
                 velocityMag,
                 xv,
                 yv,
                 maxVelocity) ;
            } */

            ObjMoveStruct->XV = (T_sword32)((ObjMoveStruct->XV * maxVelocity) / velocityMag) ;
            ObjMoveStruct->YV = (T_sword32)((ObjMoveStruct->YV * maxVelocity) / velocityMag) ;
            //if (velocityMag > 0.1) {
            //    printf("new velocity: %f, %f\n", ObjMoveStruct->XV/65535.0, ObjMoveStruct->YV/65535.0);
            //}
        }
    }

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ObjMoveSetUpSectors
 *-------------------------------------------------------------------------*/
/**
 *  ObjMoveSetUpSectors performs all the calculations necessary to update
 *  the sector information in an obj move structure.
 *
 *  @param ObjMoveStruct -- object to fix up sectors
 *
 *<!-----------------------------------------------------------------------*/
T_void ObjMoveSetUpSectors(T_objMoveStruct *ObjMoveStruct)
{
    T_sword32 OX,OY;
    T_word16 j ;
    T_word16 *p_sectors;
    T_sword16 height ;
    T_word16 sector ;
    T_word16 numSectors ;
    E_Boolean found ;
    T_sword16 floor, floor2, ceiling, ceiling2;
    T_word16 areaSector ;
    T_word16 areaCeilingSector ;
    E_Boolean isPlayer ;
    T_word16 climbHeight ;
    T_sword32 foot, head ;
T_3dObject *p_highestObject ;
T_sword16 highestObject ;

    DebugRoutine("ObjMoveSetUpSectors") ;
    DebugCheck(ObjMoveStruct != NULL) ;

    OX = ObjMoveStruct->X ;
    OY = ObjMoveStruct->Y ;

    /* What sector are we in now? */
    sector = View3dFindSectorNum(
                 (T_sword16)(OX>>16),
                 (T_sword16)(OY>>16)) ;

#ifndef NDEBUG
#if 0
if (sector >= G_Num3dSectors)  {
 printf("Hmmm ... bad sector %04X, at %d, %d\n", sector, OX>>16, OY>>16) ;
 sector = ObjMoveStruct->CenterSector ;
// DebugCheck(FALSE) ;
}
#endif
#endif

if (sector >= G_Num3dSectors)  {
    sector = 0 ;
}

    foot = ObjMoveStruct->Z ;
    head = foot + (ObjMoveStruct->Height << 16) ;
    height = (ObjMoveStruct->Z >> 16) ;

    /* Make the sector we are on the only sector we need */
    /* to be on at this point.  The information will be */
    /* resolved later on down, but past information sometimes */
    /* need to be cleared out. */
    ObjMoveStruct->AreaSector =
    ObjMoveStruct->AreaCeilingSector =
    ObjMoveStruct->CenterSector =
    ObjMoveStruct->OnSectors[0] = sector ;
    ObjMoveStruct->numOnSectors = 1 ;
#ifndef SERVER_ONLY
    /** If we are a client+server build... **/
    /* Check if this is the player object. */
    isPlayer = (&(PlayerGetObject()->objMove) ==
                     ObjMoveStruct)?TRUE:FALSE ;

    if (isPlayer)  {
        climbHeight = StatsGetClimbHeight() ;
//printf("PclimbHeight = %d\n", climbHeight) ;
    } else {
        climbHeight = (ObjMoveStruct->Height>>2) ;
//printf("climbHeight = %d\n", climbHeight) ;
    }
#else
    /** If we are a server-only build, there is no local player. **/
    isPlayer = FALSE;
    climbHeight = (ObjMoveStruct->Height>>2);
#endif

//printf("%d %d %d\n", foot>>16, head>>16, climbHeight) ;
    /* Try taking a step. */
    View3dMoveToXY(
            &OX,
            &OY,
            OX,
            OY,
            ObjMoveStruct->Radius,
            foot + (climbHeight<<16),
            head,
            ObjMoveGetHeight(ObjMoveStruct),
            ((T_3dObject *)ObjMoveStruct)) ;

    /* What sectors are now being touched? */
    p_sectors = View3dGetSurroundingSectors(&numSectors) ;

#ifndef NDEBUG
/* Check to see if we are doing something wrong with the sectors. */
if (numSectors >= (MAX_OBJECT_SECTORS-1))  {
 numSectors = MAX_OBJECT_SECTORS-1 ;
 printf("Clipping sectors for ObjMoveUpdate !\n") ;
}
#endif

    /* Copy over the sector information. */
    ObjMoveStruct->numOnSectors = numSectors ;
    found = FALSE ;
    for (j=0; j<numSectors; j++)  {
        ObjMoveStruct->OnSectors[j] = p_sectors[j] ;
        if (p_sectors[j] == sector)
            found = TRUE ;
    }

    /* If the center sector is not in the list, it needs to be. */
    if (found == FALSE)  {
        ObjMoveStruct->OnSectors[j] = sector ;
        ObjMoveStruct->numOnSectors++ ;
    }

    Collide3dGetSectorsInBox(
        ObjMoveStruct->X,
        ObjMoveStruct->Y,
        ObjMoveStruct->Radius,
        MAX_OBJECT_SECTORS,
        ObjMoveStruct->OnSectors,
        &ObjMoveStruct->numOnSectors) ;

    /* Don't forget to store the middle sector as the center sector. */
    ObjMoveStruct->CenterSector = sector ;

    /* What was the heighest floor and the lowest ceiling */
    /* in the area we are now in. */
    View3dGetFloorAndCeilingHeight(&floor, &ceiling) ;

    View3dGetCeilingBelow(&areaCeilingSector) ;
    ObjMoveStruct->AreaCeilingSector = areaCeilingSector ;

    /* What is the sector of the floor we think we are on. */
    View3dGetFloorAbove(&areaSector) ;
    ObjMoveStruct->AreaSector = areaSector ;

    /* Is it hanging from the ceiling? */
    if (ObjMoveStruct->Flags & OBJMOVE_FLAG_STICK_TO_CEILING)  {
        /* Hang it from the ceiling.  Just pin it based on the center. */
        ObjMoveStruct->Z = (((T_sword32) (MapGetCeilingHeight(sector) -
                                            ObjMoveStruct->Height))<<16) ;

    } else {
        /* Just put it on the floor. */
        floor = MapGetWalkingFloorHeight(ObjMoveStruct, sector) ;

        /** If I don't sink, my feet are on the visible floor.  If I do, **/
        /** they are on the "real" (i.e. underwater) floor. **/
        if (ObjMoveGetFlags (ObjMoveStruct) & OBJMOVE_FLAG_DO_NOT_SINK)
            floor = MapGetFloorHeight (sector);
        else
            floor = MapGetWalkingFloorHeight(ObjMoveStruct, sector) ;
        ceiling = MapGetCeilingHeight(sector) ;

        if (numSectors != 0)  {
            /** If I don't sink, my feet are on the visible floor.  If I do, **/
            /** they are on the "real" (i.e. underwater) floor. **/
            if (ObjMoveGetFlags (ObjMoveStruct) & OBJMOVE_FLAG_DO_NOT_SINK)
                floor2 = MapGetFloorHeight (areaSector);
            else
                floor2 = MapGetWalkingFloorHeight(ObjMoveStruct, areaSector) ;

            if (floor2 > floor)
                floor = floor2 ;

DebugCheck(areaCeilingSector < G_Num3dSectors) ;
            ceiling2 = MapGetCeilingHeight(areaCeilingSector) ;
            if (ceiling2 > ceiling)
                ceiling = ceiling2 ;
        } else {
            ObjMoveStruct->AreaSector = sector ;
        }

        /* Find any objects that we might be standing on. */
        p_highestObject = ObjMoveFindHighestObject(ObjMoveStruct) ;

        /* If there is one, see if it is to replace the normal floor. */
        if (p_highestObject)  {
          highestObject = ObjectGetZ16(p_highestObject) +
                          ObjectGetHeight(p_highestObject) ;
          if (highestObject > floor)  {
              /* make sure we don't go through the ceiling. */
              if ((highestObject + ObjMoveStruct->Height) < ceiling)  {
                  floor = highestObject ;
              }
          }
        }

        ObjMoveStruct->lowestPoint =
            ObjMoveStruct->Z =
                (((T_sword32)floor)<<16) ;

        /* Find any objects that we might be under. */
        p_highestObject = ObjMoveFindLowestObject(ObjMoveStruct) ;

        /* If there is one, see if it is to replace the normal ceiling. */
        if (p_highestObject)  {
          highestObject = ObjectGetZ16(p_highestObject) +
                          ObjectGetHeight(p_highestObject) ;
          if (highestObject < ceiling)  {
              ceiling = highestObject ;
          }
        }

        ObjMoveStruct->highestPoint = ceiling<<16 ;

        /* Is this object raised? */
        if (ObjMoveStruct->Flags & OBJMOVE_FLAG_RAISED)  {
            /* Yes ... move it up 32 pixels. */
            ObjMoveStruct->Z += (32L << 16) ;
        }
    }

    DebugEnd() ;
}

T_3dObject *ObjMoveFindHighestObject(T_objMoveStruct *ObjMoveStruct)
{
    T_3dObject *p_obj ;
    T_3dObject *p_highest = NULL ;
    T_sword16 highest = -0x7FFF ;
    T_sword16 x ;
    T_sword16 y ;
    T_word16 radius ;
    T_sword16 zTop ;
    T_3dObject *p_movingObject ;
    T_sword16 hashOut ;

    T_word16 halfwidth ;
    T_sword16 objX, objY ;
    T_sword16 objBottom, objTop ;

    T_doubleLinkListElement element ;
    T_sword16 hashX, hashY ;
    T_sword16 startHashX, startHashY ;
    T_word16 group ;

    p_movingObject = ((T_3dObject *)ObjMoveStruct) ;

    /* Fully passable objects don't care. */
    if (ObjectIsFullyPassable(p_movingObject))
        return NULL ;

    x = ObjMoveGetX(ObjMoveStruct)>>16 ;
    y = ObjMoveGetY(ObjMoveStruct)>>16 ;
    radius = ObjMoveGetRadius(ObjMoveStruct) ;
    hashOut = 2+(radius>>5) ;
    zTop = (ObjMoveGetZ(ObjMoveStruct)>>16) +
                ObjMoveGetHeight(ObjMoveStruct) ;

    startHashX = ((ObjectGetX16(p_movingObject) -
                  G_3dBlockMapHeader->xOrigin) >> 6) ;
    startHashY = ((ObjectGetY16(p_movingObject) -
                  G_3dBlockMapHeader->yOrigin) >> 6) ;
    for (hashY=-hashOut; hashY<=hashOut; hashY++)  {
        /* Don't do ones that are out of bounds. */
        if ((startHashY + hashY) < 0)
            continue ;
        if ((startHashY + hashY) >= G_objCollisionNumY)
            continue ;
        for (hashX=-hashOut; hashX<=hashOut; hashX++)  {
            /* Don't do ones that are out of bounds. */
            if ((startHashX + hashX) < 0)
                continue ;
            if ((startHashX + hashX) >= G_objCollisionNumX)
                continue ;

            /* Calculate the group we need to check. */
            group =  (startHashY + hashY) * G_objCollisionNumX +
                         (startHashX + hashX) ;
            element = DoubleLinkListGetFirst(G_3dObjCollisionLists[group]) ;
            while (element != DOUBLE_LINK_LIST_ELEMENT_BAD)  {
                p_obj = (T_3dObject *)DoubleLinkListElementGetData(element) ;
                element = DoubleLinkListElementGetNext(element) ;

                if ((&(p_obj->objMove)) == ObjMoveStruct)
                    continue ;

                if (ObjectGetAttributes(p_obj) & OBJECT_ATTR_PASSABLE)
                    continue ;

                /* Determine "square" distance to be in */
                halfwidth = ObjectGetRadius(p_obj) + radius ;
                objX = ObjectGetX16(p_obj) ;
                objY = ObjectGetY16(p_obj) ;

                if ((x <= objX+halfwidth) &&
                    (x >= objX-halfwidth) &&
                    (y <= objY+halfwidth) &&
                    (y >= objY-halfwidth))  {

                    objBottom = ObjectGetZ16(p_obj) ;
                    objTop = objBottom + ObjectGetHeight(p_obj) ;

                    if ((objBottom >= zTop) && (zTop >= -0x7000))
                        continue ;

                    /* We are over this object. */
                    /* Is it taller than last? */
                    if (objTop > highest)  {
                        /* Record a new high. */
                        highest = objTop ;
                        p_highest = p_obj ;
                    }
                }
            }
        }
    }
    return p_highest ;
}

T_3dObject *ObjMoveFindLowestObject(T_objMoveStruct *ObjMoveStruct)
{
    T_3dObject *p_obj ;
    T_3dObject *p_lowest = NULL ;
    T_sword16 lowest = 0x7FFF ;
    T_sword16 x ;
    T_sword16 y ;
    T_word16 radius ;
    T_sword16 zTop ;
    T_3dObject *p_movingObject ;
    T_sword16 hashOut ;

    T_word16 halfwidth ;
    T_sword16 objX, objY ;
    T_sword16 objBottom, objTop ;

    T_doubleLinkListElement element ;
    T_sword16 hashX, hashY ;
    T_sword16 startHashX, startHashY ;
    T_word16 group ;

    p_movingObject = ((T_3dObject *)ObjMoveStruct) ;

    /* Fully passable objects don't care. */
    if (ObjectIsFullyPassable(p_movingObject))
        return NULL ;

    x = ObjMoveGetX(ObjMoveStruct)>>16 ;
    y = ObjMoveGetY(ObjMoveStruct)>>16 ;
    radius = ObjMoveGetRadius(ObjMoveStruct) ;
    zTop = (ObjMoveGetZ(ObjMoveStruct)>>16) +
                ObjMoveGetHeight(ObjMoveStruct) ;
    hashOut = 2+(radius>>5) ;


    startHashX = ((ObjectGetX16(p_movingObject) -
                  G_3dBlockMapHeader->xOrigin) >> 6) ;
    startHashY = ((ObjectGetY16(p_movingObject) -
                  G_3dBlockMapHeader->yOrigin) >> 6) ;
    for (hashY=-hashOut; hashY<=hashOut; hashY++)  {
        /* Don't do ones that are out of bounds. */
        if ((startHashY + hashY) < 0)
            continue ;
        if ((startHashY + hashY) >= G_objCollisionNumY)
            continue ;
        for (hashX=-hashOut; hashX<=hashOut; hashX++)  {
            /* Don't do ones that are out of bounds. */
            if ((startHashX + hashX) < 0)
                continue ;
            if ((startHashX + hashX) >= G_objCollisionNumX)
                continue ;

            /* Calculate the group we need to check. */
            group =  (startHashY + hashY) * G_objCollisionNumX +
                         (startHashX + hashX) ;
            element = DoubleLinkListGetFirst(G_3dObjCollisionLists[group]) ;
            while (element != DOUBLE_LINK_LIST_ELEMENT_BAD)  {
                p_obj = (T_3dObject *)DoubleLinkListElementGetData(element) ;
                element = DoubleLinkListElementGetNext(element) ;

                if ((&(p_obj->objMove)) == ObjMoveStruct)
                    continue ;

                if (ObjectGetAttributes(p_obj) & OBJECT_ATTR_PASSABLE)
                    continue ;

                /* Determine "square" distance to be in */
                halfwidth = ObjectGetRadius(p_obj) + radius ;
                objX = ObjectGetX16(p_obj) ;
                objY = ObjectGetY16(p_obj) ;

                if ((x <= objX+halfwidth) &&
                    (x >= objX-halfwidth) &&
                    (y <= objY+halfwidth) &&
                    (y >= objY-halfwidth))  {

                    objBottom = ObjectGetZ16(p_obj) ;
                    objTop = objBottom + ObjectGetHeight(p_obj) ;

                    /* Exclude objects that are already below me. */
                    if (objBottom < zTop)
                        continue ;

                    /* We are over this object. */
                    /* Is it taller than last? */
                    if (objBottom < lowest)  {
                        /* Record a new high. */
                        lowest = objBottom ;
                        p_lowest = p_obj ;
                    }
                }
            }
        }
    }
    return p_lowest ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ObjMoveSetMovedFlag
 *-------------------------------------------------------------------------*/
/**
 *  ObjMoveSetMovedFlag notes that an object has been moved and ever
 *  moved.
 *
 *  @param ObjMoveStruct -- object that moved
 *
 *<!-----------------------------------------------------------------------*/
T_void ObjMoveSetMovedFlag(T_objMoveStruct *ObjMoveStruct)
{
    DebugRoutine("ObjMoveSetMovedFlag") ;
    DebugCheck(ObjMoveStruct != NULL) ;

    ObjMoveStruct->Flags |= OBJMOVE_FLAG_MOVED ;
    ObjMoveStruct->Flags |= OBJMOVE_FLAG_HAS_EVER_MOVED ;

    DebugEnd() ;
}

/* Routine ObjMoveUpdateZVel is used to move an object along */
/* the z axis after computing in gravity. */
/* LES: 04/16/96   Created */
T_void ObjMoveUpdateZVel(
           T_objMoveStruct *ObjMoveStruct,
           T_word32 delta)
{
    T_word16 j ;
    T_sword32 gravity ;
    E_Boolean doBounce ;
    E_Boolean isPlayer ;
    T_sword16 highFloor, lowCeiling ;
    T_sword16 top ;
    T_sword32 square ;

    highFloor = ObjMoveStruct->lowestPoint>>16 ;
    lowCeiling = ObjMoveStruct->highestPoint>>16 ;

    /* We are off the ground.  Do we factor in gravity and start */
    /* fallling? */
    if (!(ObjMoveStruct->Flags & OBJMOVE_FLAG_IGNORE_GRAVITY))  {
        /* Get the gravity coefficent for this object. */
        gravity = -G_3dSectorInfoArray[ObjMoveStruct->AreaSector].gravity ;
        if (ObjMoveStruct->Flags & OBJMOVE_FLAG_LOW_GRAVITY)
            gravity >>= 1 ;

	    /* Our feet are above the ground, therefore we are */
        /* falling. */
        /* How high is the floor underneath? */
        doBounce = FALSE ;

        /* Ok, this is fairly complex, so we will have to loop */
        /* through the fall to get a nice curve. */
        for (j=0; j<delta; j++)
        {
            /* Accelerate the falling velocity accordingly. */
            ObjMoveStruct->ZV += gravity ;

            /* Change the Z position based on Z velocity. */
            ObjMoveStruct->Z += (ObjMoveStruct->ZV << 4) ;

            /* Did we go under the ground from the hard fall? */
            if ((ObjMoveStruct->Z>>16) < highFloor)  {
                /* Hmmm ... we might take damage from that. */
                /* !!! Hmmm ... is max fall velocity a object */
                /* dependend item?  I think so */
                isPlayer = (&(PlayerGetObject()->objMove) ==
                               ObjMoveStruct)?TRUE:FALSE ;
                if (isPlayer)  {
                    if (ObjMoveStruct->ZV < -StatsGetMaxFallV())  {
                        /* !!! Gee ... who do we hurt?  Do */
                        /*  creatures hurt from falls? */
                        /* ouch!! */
                        if (EffectPlayerEffectIsActive (PLAYER_EFFECT_SHOCK_ABSORB)==FALSE)
                        {
                            square = (-ObjMoveStruct->ZV - StatsGetMaxFallV())>>6 ;
                            square += ((square * square) / 100) ;

                            // Ignore small amounts of damage from falling
                            // to avoid little spills.
                            if (square > 50) {
                                if (square > 32000)
                                    square = 32000 ;
                                StatsTakeDamage (
                                     EFFECT_DAMAGE_NORMAL,
                                     square) ;
                            }
                        }
                    }
                } else if (ObjectIsCreature((T_3dObject *)ObjMoveStruct))  {
                    if (ObjMoveStruct->ZV < -31000)  {
                        square = (-ObjMoveStruct->ZV - 31000)>>6 ;
                        square += ((square * square) / 100) ;
                        if (square > 32000)
                            square = 32000 ;
                        CreatureTakeDamage(
                            (T_3dObject *)ObjMoveStruct,
                            square,
                            EFFECT_DAMAGE_NORMAL,
                            0);
                    }
                }

                /* Place us on the ground. */
                ObjMoveStruct->Z = highFloor << 16 ;

                /* Note that we hit the ground pretty good. */
                doBounce = TRUE ;
                break;
            }
        }

        if (doBounce)  {
            if (ObjMoveStruct->Flags & OBJMOVE_FLAG_BOUNCES)  {
                if (ObjMoveStruct->ZV < -0x1000)  {
                    /* Reverse the Z velocity. */
                    ObjMoveStruct->ZV = -((ObjMoveStruct->ZV*7)>>3) ;
                } else {
                    ObjMoveStruct->ZV = 0 ;
                }
            } else {
                /* If not, do a little bounce to get a better */
                /* feel of the "ground" under our feet. */
                if (ObjMoveStruct->ZV < -0x1000)  {
                    /* Reverse the Z velocity. */
                    ObjMoveStruct->ZV = -((ObjMoveStruct->ZV)>>3) ;
                } else {
                    ObjMoveStruct->ZV = 0 ;
                }
            }
        }

        /* Check to see if we are going (about to go) through the roof. */
        top = (ObjMoveStruct->Z>>16) + ObjMoveStruct->Height ;
        if (top > lowCeiling-10)  {
            /* Are we also going up? */
            if (ObjMoveStruct->ZV > 0)
                /* If so, reverse the z velocity and dampen. */
                ObjMoveStruct->ZV = -(ObjMoveStruct->ZV/2) ;

            /* Unfortunately, we don't want to force the object into */
            /* ground, so, we'll just leave him at the wrong height. */
            /* If this is a character, the calling routine will have */
            /* to check this also to lower the view. */
            top = lowCeiling-10-ObjMoveStruct->Height ;
            if (top < highFloor)
                top = highFloor;
            ObjMoveStruct->Z = (((T_sword32)top)<<16) ;
        }
    }
}

/** @} */
/*-------------------------------------------------------------------------*
 * End of File:  OBJMOVE.C
 *-------------------------------------------------------------------------*/
