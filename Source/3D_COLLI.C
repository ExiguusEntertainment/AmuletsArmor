/*-------------------------------------------------------------------------*
 * File:  3D_COLLI.C
 *-------------------------------------------------------------------------*/
/**
 * @addtogroup _3D_COLLI
 * @brief 3D Collision Routines
 * @see http://www.amuletsandarmor.com/AALicense.txt
 * @{
 *
 *<!-----------------------------------------------------------------------*/
#include "3D_COLLI.H"
#include "3D_IO.H"
#include "3D_TRIG.H"
#include "3D_VIEW.H"
#include "GENERAL.H"
#include "MAP.H"
#include "OBJECT.H"
#include "OBJMOVE.H"
#include "SYNCMEM.H"
#include "VIEW.H"

/*****************************/
/*   CONSTANTS and DEFINES   */
/*****************************/

#define MAX_LINE_HITS 20
#define easyabs(x) (((x)>=0)?(x):-(x))


/*****************************/
/*      GLOBAL VARIABLES     */
/*****************************/

/* Global variables used by View3dFindLineHits and ICheckLineHitsLine. */
/* Represents the slope of the line in question. */
static T_sword32 G_cosLineAngle ;
static T_sword32 G_sinLineAngle ;


/* Global data telling how many hits occured, */
T_word16 G_numHits = 0 ;

/* where the hit occured, */
static T_sword32 G_hitAtX, G_hitAtY ;

/* what distance the hit occured at, */
static T_sword32 G_hitDistance ;

/* and the slopes of the hit points (if there are more than one, the */
/* arrays are filled. */
static T_sword32 G_hitSlopeXs[MAX_LINE_HITS], G_hitSlopeYs[MAX_LINE_HITS] ;
static T_sword32 G_hitSlopeX, G_hitSlopeY ;
static T_word16 G_hitWho[MAX_LINE_HITS] ;
static T_word16 G_hitType[MAX_LINE_HITS] ;

#define HIT_TYPE_LINE            0
#define HIT_TYPE_OBJECT          1

/* Height of the object that is being check for collisional movement. */
static T_sword16 G_currentHeight = 0 ;


/* As edges/lines are collided with, keep track of the lowest ceiling */
/* and tallest floor and what sector this is associated with. */
static T_sword16 G_shortestCeiling ;
static T_sword16 G_tallestFloor ;
static T_word16  G_floorAbove ;
static T_word16  G_ceilingBelow ;


/* As sectors are found to be under the object that moved, keep track */
/* of the number of sectors and what sectors these are.  This is useful */
/* when a lift goes up and an object is on top of it. */
static T_word16 G_numSurroundingSectors ;
static T_word16 G_surroundingSectors[20] ;


/* In many cases, we don't want to collide with ourself.  This is */
/* the object/object move structure that we to exclude. */
static T_objMoveStruct *G_exceptObjMove = NULL ;


/* Flag to determine if dipping is allowed in the next collision */
/* detection. */
/* WARNING! Not being used? */
static E_Boolean G_allowDip = TRUE ;

/* Standard definition of a wall. */
static T_word16 G_wallDefinition = LINE_IS_IMPASSIBLE ;

/*****************************/
/*    INTERNAL PROTOTYPES    */
/*****************************/

E_Boolean IIsFloorAndCeilingOk(T_word16 lineNum, E_Boolean f_add, T_3dObject *p_obj) ;

T_void IUpdateSectorHeights(T_3dObject *p_obj, T_word16 sector);

E_Boolean MoveToFast(
         T_sword32 *oldX,
         T_sword32 *oldY,
         T_sword32 newX,
         T_sword32 newY,
         T_sword32 distance,
         T_sword32 radius,
         T_sword32 foot,
         T_sword32 head,
         T_sword16 height,
         T_3dObject *p_obj) ;

T_sword16 ICheckLineHitsLine(
              T_sword16 x1,
              T_sword16 y1,
              T_sword16 x2,
              T_sword16 y2,
              T_word16 lineNum) ;

T_sword16 IMoveToXYWithStep(
              T_sword32 *oldX,
              T_sword32 *oldY,
              T_sword32 newx,
              T_sword32 newy,
              T_sword32 radius,
              T_sword32 step,
              T_sword32 foot,
              T_sword32 head,
              T_sword16 height,
              T_3dObject *p_obj) ;

static E_Boolean IIsOnLeftOfLine(
                     T_sword32 pointX,
                     T_sword32 pointY,
                     T_sword32 lineX1,
                     T_sword32 lineY1,
                     T_sword32 lineX2,
                     T_sword32 lineY2) ;

E_Boolean MoveTo(
              T_sword32 oldX,
              T_sword32 oldY,
              T_sword32 newX,
              T_sword32 newY,
              T_sword32 distance,
              T_3dObject *p_obj) ;

T_sword32 Mult32x32AndCompare(
              T_sword32 a,
              T_sword32 b,
              T_sword32 c,
              T_sword32 d) ;

T_sword32 Mult32By32AndDiv32(
              T_sword32 a,
              T_sword32 b,
              T_sword32 c) ;


static E_Boolean IWallTouchInBlock(
                    T_sword16 x1,
                    T_sword16 y1,
                    T_sword16 x2,
                    T_sword16 y2,
                    T_sword32 block,
                    T_word16 maxWalls,
                    T_word16 *p_numWalls,
                    T_word16 *p_walls) ;

static E_Boolean ICanSqueezeThrough(
        T_3dObject *p_obj,
		T_sword16 zPos,
		T_sword16 height,
		T_word16 numSectors,
		T_word16 *p_sectorList);

static E_Boolean ICanSqueezeThroughWithClimb(
        T_3dObject *p_obj,
        T_sword16 *zPos,
        T_sword16 climbHeight,
        T_sword16 height,
        T_word16 numSectors,
        T_word16 *p_sectorList);

/*****************************/
/*    ASSEMBLY DEFINES       */
/*****************************/

#ifndef SERVER_ONLY
#ifndef NO_ASSEMBLY
#pragma aux Mult32x32AndCompare = \
            "       imul ebx" \
            "       xchg eax, esi" \
            "       xchg edx, edi" \
            "       mov ebx, edx" \
            "       imul ebx" \
            "       sub eax, esi" \
            "       sbb edx, edi" \
            "       test edx, edx" \
            "       je done" \
            "       mov eax, edx" \
            "done: " \
            parm [esi] [edi] [eax] [ebx] \
            value [eax] \
            modify [eax edx esi edi];

/* f(a, b, c) = (a * b) / c  */
#pragma aux Mult32By32AndDiv32 = \
            "imul ebx" \
            "idiv ecx" \
            parm [eax] [ebx] [ecx] \
            value [eax] \
            modify [eax edx] ;
#endif
#endif

#ifdef TARGET_NT
/** Windows NT **/

/***************************************************************/
/* Mult32x32AndCompare -- Windows NT on 386+ version.          */
/***************************************************************/

T_sword32 Mult32x32AndCompare(T_sword32 _esi, T_sword32 _edi,
                              T_sword32 _eax, T_sword32 _ebx)
{
   __asm {
      mov esi, _esi
      mov edi, _edi
      mov eax, _eax
      mov ebx, _ebx
      imul ebx
      xchg eax, esi
      xchg edx, edi
      mov ebx, edx
      imul ebx
      sub eax, esi
      sbb edx, edi
      test edx, edx
      je done
      mov eax, edx
     done:
      mov _eax, eax
   }

   return _eax;
}

/***************************************************************/
/* Mult32By32AndDiv32 -- Windows NT on 386+ version            */
/***************************************************************/

T_sword32 Mult32By32AndDiv32 (T_sword32 _eax, T_sword32 _ebx,
                              T_sword32 _ecx)
{
   __asm {
         mov eax, _eax
		 mov ebx, _ebx
		 mov ecx, _ecx
		 imul ebx
		 idiv ecx
		 mov _eax, eax
   }

   return _eax;
}
#endif

#ifdef TARGET_UNIX

/** UNIX; Specifically, GCC on Linux on a 486 **/

/***************************************************************/
/* Mult32x32AndCompare -- Linux/GCC version.                   */
/***************************************************************/

#define Mult32x32AndCompare(a,b,c,d)   \
            ( {  \
               T_sword32 esi, edi, eax, ebx; \
               esi = (a); edi = (b); eax = (c); ebx = (d); \
               asm volatile ( \
                    "  movl %%esi, %1;" \
                    "  movl %%edi, %2;" \
                    "  movl %%eax, %3;" \
                    "  movl %%ebx, %4;" \
                    "  imull %%ebx;" \
                    "  xchgl %%eax, %%esi;" \
                    "  xchgl %%edx, %%edi;" \
                    "  movl %%ebx, %%edx;" \
                    "  imull %%ebx;" \
                    "  subl %%eax, %%esi;" \
                    "  sbbl %%edx, %%edi;" \
                    "  test %%edx, %%edx;" \
                    "  je done;" \
                    "  movl %1, %%edx;" \
                    "done: " \
                           : "=r" (eax) \
                           : "r" (esi), "r" (edi), "0" (eax), "r" (ebx) \
                           : "%esi", "%edi", "%eax", "%ebx", "%edx" ); \
               eax; \
            } )

/***************************************************************/
/* Mult32By32AndDiv32 -- Linux/GCC version.                    */
/***************************************************************/

#define Mult32By32AndDiv32(a,b,c)  \
            ( { \
               T_sword32 eax, ebx, ecx; \
               eax = (a); ebx = (b); ecx = (c); \
               asm volatile ( \
                    "  movl %%eax, %1;" \
                    "  imull %2;" \
                    "  idivl %3;" \
                    "  movl %0, %%eax" \
                           : "=r" (eax) \
                           : "r" (eax), "r" (ebx), "r" (ecx) \
                           : "%eax", "%edx" ); \
               eax; \
            } )

#endif /** TARGET_UNIX **/

/*-------------------------------------------------------------------------*
 * Routine:  IAddSurroundingSector
 *-------------------------------------------------------------------------*/
/**
 *  This routine adds a sector to the surrounding sector list if it
 *  is not already in the sector list.
 *
 *  @param sector -- Sector to add
 *
 *<!-----------------------------------------------------------------------*/
T_void IAddSurroundingSector(T_word16 sector)
{
    T_word16 i ;

#ifndef NDEBUG
if (sector >= G_Num3dSectors)  {
  printf("Illegal sector: %d\n", sector) ;
  fflush(stdout) ;
}
DebugCheck((sector < G_Num3dSectors)) ;
#endif

    if (G_numSurroundingSectors == 10)
        return ;

    for (i=0; i<G_numSurroundingSectors; i++)
        if (sector == G_surroundingSectors[i])
            return ;

    G_surroundingSectors[i] = sector ;
    G_numSurroundingSectors++ ;
}

/*-------------------------------------------------------------------------*
 * Routine:  IIsFloorAndCeilingOk
 *-------------------------------------------------------------------------*/
/**
 *  Given a line/wall on the map, determine if the floor and ceiling
 *  of both sides of the line are in agreement with what is allowed
 *  for going over edges.  In addition, note if we wish to add the
 *  sectors to the surrounding sector list.
 *
 *  @param lineNum -- Wall to check floors and ceilings.
 *  @param f_add -- Flag where TRUE=add to sector list
 *  @param p_obj -- Source object being checked
 *
 *  @return FALSE = colliding with an edge.
 *
 *<!-----------------------------------------------------------------------*/
E_Boolean IIsFloorAndCeilingOk(
              T_word16 lineNum,
              E_Boolean f_add,
              T_3dObject *p_obj)
{
    T_sword16 floorHeight1 = -0x7FFE ;
    T_sword16 ceilingHeight1 = 0x7FFE ;
    T_sword16 floorHeight2 = -0x7FFE ;
    T_sword16 ceilingHeight2 = 0x7FFE ;
    T_word16 sector1, sector2 ;
    T_sword16 side1, side2 ;
    T_sword16 height, climb ;

    if (G_3dLineArray[lineNum].flags & LINE_IS_IMPASSIBLE)
        return FALSE ;

    side1 = G_3dLineArray[lineNum].side[0] ;
    if (side1 != -1)
        sector1 = G_3dSideArray[side1].sector ;
    else
        sector1 = 0xFFFF ;

    side2 = G_3dLineArray[lineNum].side[1] ;
    if (side2 != -1)
        sector2 = G_3dSideArray[side2].sector ;
    else
        sector2 = 0xFFFF ;

    /* If either side of this line is NOT connected to a sector, then */
    /* this is an impassable wall that should NOT be considered OK. */
    if ((sector1 == 0xFFFF) || (sector2 == 0xFFFF))
        return FALSE ;

    /* Find the floor and ceiling heights of both sides of */
    /* a line. */
    if (side1 != -1)  {
        floorHeight1 = MapGetWalkingFloorHeight(&p_obj->objMove, sector1) ;
        ceilingHeight1 = G_3dSectorArray[sector1].ceilingHt ;
        if (G_3dSectorInfoArray[sector1].ceilingLimit < ceilingHeight1)
            ceilingHeight1 = G_3dSectorInfoArray[sector1].ceilingLimit ;
    }

    if (side2 != -1)  {
        floorHeight2 = MapGetWalkingFloorHeight(&p_obj->objMove, sector2) ;
        ceilingHeight2 = G_3dSectorArray[sector2].ceilingHt ;
        if (G_3dSectorInfoArray[sector2].ceilingLimit < ceilingHeight2)
            ceilingHeight2 = G_3dSectorInfoArray[sector2].ceilingLimit ;
    }

    if (f_add == TRUE)  {
        if (side1 != -1)  {
            IAddSurroundingSector(sector1) ;
//            if ((floorHeight1 <= G_currentHeight) &&
//                (G_currentHeight < ceilingHeight1)) {
#ifndef NDEBUG
                if (sector1 >= G_Num3dSectors)  {
                    printf("line error: %d\n", lineNum) ;
                    fflush(stdout) ;
                }
#endif
                IUpdateSectorHeights(p_obj, sector1) ;
//            }
        }

        if (side2 != -1)  {
            IAddSurroundingSector(sector2) ;
//            if ((floorHeight2 <= G_currentHeight) &&
//                (G_currentHeight < ceilingHeight2)) {
#ifndef NDEBUG
                if (sector2 >= G_Num3dSectors)  {
                    printf("line error: %d\n", lineNum) ;
                    fflush(stdout) ;
                }
#endif
                IUpdateSectorHeights(p_obj, sector2) ;
//            }
        }
    }

    /* Take the higher floor. */
    if (floorHeight2 > floorHeight1)
        floorHeight1 = floorHeight2 ;

    /* Take the lower ceiling. */
    if (ceilingHeight2 < ceilingHeight1)
        ceilingHeight1 = ceilingHeight2 ;

    height = ObjectGetHeight(p_obj) ;
    climb = ObjectGetClimbHeight(p_obj) ;

    /* Is the difference between the two too small? */
    if (floorHeight1 + height > ceilingHeight1)
        return FALSE ;

    /* See if stepping up helps. */
    if ((G_currentHeight + climb) < floorHeight1)
        return FALSE ;

    /* Is the floor too high? */
//    if (floorHeight1 > G_currentHeight)
//        return FALSE ;

    /* Is the ceiling too low? */
    if ((G_currentHeight+height) > ceilingHeight1)
        return FALSE ;

    /* Everything checks out OK, must be ok. */
    return TRUE ;
}

#if 0
/*-------------------------------------------------------------------------*
 * Routine:  ICheckSegmentHitBox
 *-------------------------------------------------------------------------*/
/**
 *  ICheckSegmentHitBox determines if a line hits a box and returns
 *  what egdes are being hit.
 *
 *  @param lineNum -- Wall to check floors and ceilings.
 *  @param y1 -- Upper left point of box
 *  @param y2 -- Lower right of box
 *
 *  @return 4 bit encoded word telling what sides
 *      are hitting the line.
 *
 *<!-----------------------------------------------------------------------*/
T_sword16 ICheckSegmentHitBox(
              T_word16 lineNum,
              T_sword16 x1,
              T_sword16 y1,
              T_sword16 x2,
              T_sword16 y2)
{
    T_sword16 segX1, segX2, segY1, segY2 ;
    T_sword32 lesserX, greaterX, lesserY, greaterY ;
    T_sword32 interX, interY ;
    T_sword32 deltaX, deltaY ;
    T_word16 side = 0 ;

    /* Get the coordinates of the line. */

    segX1 = G_3dVertexArray[G_3dLineArray[lineNum].from].x ;
    segY1 = G_3dVertexArray[G_3dLineArray[lineNum].from].y ;
    segX2 = G_3dVertexArray[G_3dLineArray[lineNum].to].x ;
    segY2 = G_3dVertexArray[G_3dLineArray[lineNum].to].y ;

/*
    segX1 = G_linesX1[lineNum] ;
    segY1 = G_linesY1[lineNum] ;
    segX2 = G_linesX2[lineNum] ;
    segY2 = G_linesY2[lineNum] ;
*/
    /* Figure out who is greater than who. */
    if (segX1 < segX2)  {
        lesserX = segX1 ;
        greaterX = segX2 ;
    } else {
        lesserX = segX2 ;
        greaterX = segX1 ;
    }

    if (segY1 < segY2)  {
        lesserY = segY1 ;
        greaterY = segY2 ;
    } else {
        lesserY = segY2 ;
        greaterY = segY1 ;
    }

    /* Check if the boxes touch (draw a box around the line) */
    /* It is assumed that x1 is on the left of x2 and y1 is */
    /* less than y2. */
    if (greaterX < x1)
        return FALSE ;
    if (greaterY < y1)
        return FALSE ;
    if (lesserX > x2)
        return FALSE ;
    if (lesserY > y2)
        return FALSE ;

    /* OK, we must be near each other.  We must now take each side */
    /* and look for a collision point. */

    /* Start by getting some deltas. */
    deltaX = segX2 - segX1 ;
    deltaY = segY2 - segY1 ;

    /* Calculate the left intersection point. */
    if (deltaX != 0)  {
        interY = segY1 + ((deltaY * (x1 - segX1)) / deltaX) ;
        if ((interY >= y1) && (interY <= y2))  {
            side |= 0x8000 ;
            side++ ;
        }
    }

    /* Calculate the right intersection point. */
    if (deltaX != 0)  {
        interY = segY1 + ((deltaY * (x2 - segX1)) / deltaX) ;
        if ((interY >= y1) && (interY <= y2))  {
            side |= 0x4000 ;
            side++ ;
        }
    }

    /* Calculate the top intersection point. */
    if (deltaY != 0)  {
        interX = segX1 + ((deltaX * (y1 - segY1)) / deltaY) ;
        if ((interX >= x1) && (interX <= x2))  {
            side |= 0x2000 ;
            side++ ;
        }
    }

    /* Calculate the bottom intersection point. */
    if (deltaY != 0)  {
        interX = segX1 + ((deltaX * (y2 - segY1)) / deltaY) ;
        if ((interX >= x1) && (interX <= x2))  {
            side |= 0x1000 ;
            side++ ;
        }
    }

    if (side != 0)
        return side ;

    /* None of them intersected, return a FALSE. */
    return FALSE ;
}
#endif

/*-------------------------------------------------------------------------*
 * Routine:  Collide3dCheckSegmentHitBox
 *-------------------------------------------------------------------------*/
/**
 *  Collide3dCheckSegmentHitBox determines quickly if a line hits a box.
 *
 *  @param lineNum -- Wall to check floors and ceilings.
 *  @param x1 -- Left point of box
 *  @param y1 -- Upper point of box
 *  @param x2 -- Right of box
 *  @param y2 -- Lower of box
 *
 *  @return TRUE=collision, else FALSE
 *
 *<!-----------------------------------------------------------------------*/
T_sword16 Collide3dCheckSegmentHitBox(
              T_word16 lineNum,
              T_sword16 x1,
              T_sword16 y1,
              T_sword16 x2,
              T_sword16 y2)
{
    T_sword16 segX1, segX2, segY1, segY2 ;
    T_sword32 lesserX, greaterX, lesserY, greaterY ;
    T_sword32 interX, interY ;
    T_sword32 deltaX, deltaY ;

    /* Get the coordinates of the line. */

    segX1 = G_3dVertexArray[G_3dLineArray[lineNum].from].x ;
    segY1 = G_3dVertexArray[G_3dLineArray[lineNum].from].y ;
    segX2 = G_3dVertexArray[G_3dLineArray[lineNum].to].x ;
    segY2 = G_3dVertexArray[G_3dLineArray[lineNum].to].y ;

    /* Figure out who is greater than who. */
    if (segX1 < segX2)  {
        lesserX = segX1 ;
        greaterX = segX2 ;
    } else {
        lesserX = segX2 ;
        greaterX = segX1 ;
    }

    if (segY1 < segY2)  {
        lesserY = segY1 ;
        greaterY = segY2 ;
    } else {
        lesserY = segY2 ;
        greaterY = segY1 ;
    }

    /* Check if the boxes touch (draw a box around the line) */
    /* It is assumed that x1 is on the left of x2 and y1 is */
    /* less than y2. */
    if (greaterX < x1)
        return FALSE ;
    if (greaterY < y1)
        return FALSE ;
    if (lesserX > x2)
        return FALSE ;
    if (lesserY > y2)
        return FALSE ;

    /* OK, we must be near each other.  We must now take each side */
    /* and look for a collision point. */

    /* Start by getting some deltas. */
    deltaX = segX2 - segX1 ;
    deltaY = segY2 - segY1 ;

    /* If a "flat box" (horizontal or vertical line), drop out */
    /* and hit. */
    if ((deltaX == 0) || (deltaY == 0))
        return TRUE ;

    /* Calculate the left intersection point. */
        interY = segY1 + ((deltaY * (x1 - segX1)) / deltaX) ;
        if ((interY >= y1) && (interY <= y2))  {
            return TRUE ;
        }

    /* Calculate the right intersection point. */
    interY = segY1 + ((deltaY * (x2 - segX1)) / deltaX) ;
    if ((interY >= y1) && (interY <= y2))  {
        return TRUE ;
    }

    /* Calculate the top intersection point. */
    interX = segX1 + ((deltaX * (y1 - segY1)) / deltaY) ;
    if ((interX >= x1) && (interX <= x2))  {
        return TRUE ;
    }

    /* Calculate the bottom intersection point. */
    interX = segX1 + ((deltaX * (y2 - segY1)) / deltaY) ;
    if ((interX >= x1) && (interX <= x2))  {
        return TRUE ;
    }

    /* None of them intersected, return a FALSE. */
    return FALSE ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ProjectXYOntoLine
 *-------------------------------------------------------------------------*/
/**
 *  ProjectXYOntoLine does the calculation necessary to do a linear
 *  projection of a vector going through the origin (the point) along
 *  a given line through the origin (the slope).  The new point is returned
 *  by reference.
 *
 *  NOTE: 
 *  Large values can cause overflow.  Always be careful.
 *
 *  @param pointX -- X Point to project
 *  @param pointY -- Y Point to project
 *  @param slopeX -- X Slope to project along
 *  @param slopeY -- Y Slope to project along
 *
 *<!-----------------------------------------------------------------------*/
T_void ProjectXYOntoLine(
           T_sword32 *pointX,
           T_sword32 *pointY,
           T_sword32 slopeX,
           T_sword32 slopeY)
{
    /* Assumes from point is the origin. */
    T_sword32 x, y ;
    T_sword32 topCalc, bottomCalc ;
    T_sword32 aslopeX, aslopeY ;

    if (slopeX == 0)  {
        *pointX = 0 ;
    }
    if (slopeY == 0)  {
        *pointY = 0 ;
    }

    if ((slopeX==0) || (slopeY==0))
        return ;

    if (slopeX < 0)
        aslopeX = -slopeX ;
    else
        aslopeX = slopeX ;

    if (slopeY < 0)
        aslopeY = -slopeY ;
    else
        aslopeY = slopeY ;

    /* Check if a 45 degree slope. */
    if (aslopeX == aslopeY)  {
        if (slopeX < 0)
            slopeX = -1 ;
        else if (slopeX > 0)
            slopeX = 1 ;
        else
            slopeX = 0 ;

        if (slopeY < 0)
            slopeY = -1 ;
        else if (slopeY > 0)
            slopeY = 1 ;
        else
            slopeY = 0 ;
    }

    /* Get the 32 bit version of these 16 bit values. */
    x = *pointX ;
    y = *pointY ;

    /* Find the proportional fraction along the line we are */
    /* projecting onto. */
    topCalc = (slopeX * x) + (slopeY * y) ;
    bottomCalc = (slopeX * slopeX) + (slopeY * slopeY) ;

    /* Make sure we are not dividing by zero. */
    if (bottomCalc == 0)  {
        /* If we are dividing by zero, we have an effect projection */
        /* of zero. */
        *pointX = 0 ;
        *pointY = 0 ;
    } else {
        /* Calculate the real projection. */
        *pointX = Mult32By32AndDiv32(topCalc, slopeX, bottomCalc) ;
        *pointY = Mult32By32AndDiv32(topCalc, slopeY, bottomCalc) ;
    }
}

#if 0
/*-------------------------------------------------------------------------*
 * Routine:  CheckVertexHit
 *-------------------------------------------------------------------------*/
/**
 *  CheckVertexHit is a routine to determine if the given box collides
 *  with any of the vertices of the lines on the map.  The old location
 *  is given to better determine the angle of approach and what the rebound
 *  slope should be.
 *
 *  NOTE: 
 *  This routine is not currently being used and might be removed soon.
 *
 *  @param oldY -- Where box has moved from
 *  @param y1 -- Upper left of box
 *  @param y2 -- Lower right of box
 *
 *  @return TRUE = vertex was hit.
 *
 *<!-----------------------------------------------------------------------*/
E_Boolean CheckVertexHit(
              T_sword32 oldX,
              T_sword32 oldY,
              T_sword32 x1,
              T_sword32 y1,
              T_sword32 x2,
              T_sword32 y2)
{
    T_word16 i ;
    T_sword32 x, y ;
    T_sword32 mx, my ;
    T_sword32 dx, dy ;
    T_sword32 prevX, prevY ;
    T_sword32 distX, distY ;

    G_numHits = 0xFFFF ;

    mx = (x1 + x2) >> 1 ;
    my = (y1 + y2) >> 1 ;
    dx = mx - oldX ;
    dy = my - oldY ;

    if ((dx==0) && (dy==0))
        return FALSE ;

    for (i=0; i<G_Num3dLines; i++)  {
        if (IIsFloorAndCeilingOk(i, FALSE) == FALSE)  {
            x = ((T_sword32)G_3dVertexArray[G_3dLineArray[i].from].x) << 16 ;
            y = ((T_sword32)G_3dVertexArray[G_3dLineArray[i].from].y) << 16 ;

            if ((x > x1) && (x < x2) && (y > y1) && (y < y2))  {
                prevX = x + dx ;
                prevY = y + dy ;
                if (((prevX < x1) && (x > x1)) ||
                    ((prevX < x2) && (x > x2)) ||
                    ((x < x1) && (prevX > x1)) ||
                    ((x < x2) && (prevX > x2)))  {
                    G_hitSlopeX = 0 ;
                    G_hitSlopeY = 1 ;
                }
                if (((prevY < y1) && (y > y1)) ||
                    ((prevY < y2) && (y > y2)) ||
                    ((y < y1) && (prevY > y1)) ||
                    ((y < y2) && (prevY > y2)))  {
                    if (G_hitSlopeY == 1)  {
                        distX = easyabs(x1-prevX) ;
                        if (easyabs(x2-prevX) < distX)
                            distX = easyabs(x2-prevX) ;
                        distY = easyabs(y1-prevY) ;
                        if (easyabs(y2-prevY) < distY)
                            distY = easyabs(y2-prevY) ;

                        if (distX < distY)  {
                            G_hitSlopeX = 0 ;
                            G_hitSlopeY = 1 ;
                        } else {
                            G_hitSlopeX = 1 ;
                            G_hitSlopeY = 0 ;
                        }
                    } else {
                        G_hitSlopeX = 1 ;
                        G_hitSlopeY = 0 ;
                    }
                }

                IIsFloorAndCeilingOk(i, TRUE) ;
                return TRUE ;
            }

            x = ((T_sword32)G_3dVertexArray[G_3dLineArray[i].to].x) << 16 ;
            y = ((T_sword32)G_3dVertexArray[G_3dLineArray[i].to].y) << 16 ;

            if ((x > x1) && (x < x2) && (y > y1) && (y < y2))  {
                prevX = x + dx ;
                prevY = y + dy ;
                if (((prevX < x1) && (x > x1)) ||
                    ((prevX < x2) && (x > x2)) ||
                    ((x < x1) && (prevX > x1)) ||
                    ((x < x2) && (prevX > x2)))  {
                    G_hitSlopeX = 0 ;
                    G_hitSlopeY = 1 ;
                }
                if (((prevY < y1) && (y > y1)) ||
                    ((prevY < y2) && (y > y2)) ||
                    ((y < y1) && (prevY > y1)) ||
                    ((y < y2) && (prevY > y2)))  {
                    if (G_hitSlopeY == 1)  {
                        distX = easyabs(x1-prevX) ;
                        if (easyabs(x2-prevX) < distX)
                            distX = easyabs(x2-prevX) ;
                        distY = easyabs(y1-prevY) ;
                        if (easyabs(y2-prevY) < distY)
                            distY = easyabs(y2-prevY) ;

                        if (distX < distY)  {
                            G_hitSlopeX = 0 ;
                            G_hitSlopeY = 1 ;
                        } else {
                            G_hitSlopeX = 1 ;
                            G_hitSlopeY = 0 ;
                        }
                    } else {
                        G_hitSlopeX = 1 ;
                        G_hitSlopeY = 0 ;
                    }
                }
                IIsFloorAndCeilingOk(i, TRUE) ;
                return TRUE ;
            }
        }
    }

    return FALSE ;
}
#endif

/*-------------------------------------------------------------------------*
 * Routine:  IGetBlock
 *-------------------------------------------------------------------------*/
/**
 *  IGetBlock is used to determine what list of lines to used based
 *  on a coordinate on the map.  These list of lines are used to determine
 *  what walls are near a location.  It speeds up collision detection very
 *  much.  The returned value is not the index of the list, but an index
 *  to that list exactly (from the start of all lists).
 *
 *  @param y -- Point to get Block for
 *
 *  @return Index to list from beginning, or -1
 *      if point is out of bounds.
 *
 *<!-----------------------------------------------------------------------*/
static T_sword32 IGetBlock(T_sword16 x, T_sword16 y)
{
    T_sword16 row, column ;
    T_sword32 index ;

    /* First, find the block map block that this point is located within. */
    column = (x - G_3dBlockMapHeader->xOrigin) >> 7 ;

    if ((column < 0) || (column > G_3dBlockMapHeader->columns))
        /* Out of bounds, return a bad one. */
        return -1 ;

    row = (y - G_3dBlockMapHeader->yOrigin) >> 7 ;

    if ((row < 0) || (row > G_3dBlockMapHeader->rows))
        /* Out of bounds, return a bad one. */
        return -1 ;

    /* Inbounds, find the index. */
    index = (row * G_3dBlockMapHeader->columns) + column ;

    /* Now translate the index into a position in the list of lines. */
    index = 1+G_3dBlockMapHeader->blockIndexes[index] ;

    return index ;
}

/*-------------------------------------------------------------------------*
 * Routine:  IUpdateSectorHeights
 *-------------------------------------------------------------------------*/
/**
 *  IUpdateSectorHeights is used to update the group of variables that
 *  tell what are the heightest and lowest sectors in a square and what
 *  their values are.
 *
 *  @param p_obj -- Object viewing the sector heights
 *  @param sector -- sector to update
 *
 *<!-----------------------------------------------------------------------*/
T_void IUpdateSectorHeights(T_3dObject *p_obj, T_word16 sector)
{
    T_sword16 floor, ceiling, limit ;

    floor = MapGetWalkingFloorHeight(&p_obj->objMove, sector) ;
//    floor = G_3dSectorArray[sector].floorHt ;
//    if ((G_allowDip) &&
//        (G_3dSectorArray[sector].trigger & SECTOR_DIP_FLAG))
//        floor -= VIEW_WATER_DIP_LEVEL ;

    ceiling = G_3dSectorArray[sector].ceilingHt ;
    limit = G_3dSectorInfoArray[sector].ceilingLimit ;
    if (limit < ceiling)
        ceiling = limit ;

    if (floor > G_tallestFloor)  {
        G_tallestFloor = floor ;
        G_floorAbove = sector ;
    }
    if (ceiling < G_shortestCeiling)  {
        G_shortestCeiling = ceiling ;
        G_ceilingBelow = sector ;
    }
}

/*-------------------------------------------------------------------------*
 * Routine:  ILineHitInBlock
 *-------------------------------------------------------------------------*/
/**
 *  ILineHitInBlock checks all the lines/walls in a block map to determine
 *  if there is a collision.  At the same time it upkeeps a list of sectors
 *  this square is over and what are the highest and lowest sector of
 *  that square.
 *
 *  @param lastX -- Old X location for angular information
 *  @param lastY -- Old Y location for angular information
 *  @param x1 -- Left corner of square
 *  @param y1 -- Upper corner of square
 *  @param x2 -- Right corner of square
 *  @param y2 -- Lower corner of square
 *  @param index -- Index to list of walls to consider
 *  @param radius -- Radius of the given box
 *  @param p_obj -- Source object being checked
 *
 *<!-----------------------------------------------------------------------*/
T_void ILineHitInBlock(
              T_sword16 lastX,
              T_sword16 lastY,
              T_sword16 x1,
              T_sword16 y1,
              T_sword16 x2,
              T_sword16 y2,
              T_sword32 index,
              T_sword16 radius,
              T_3dObject *p_obj)
{
    T_word16 i ;
    T_sword32 segX1, segX2, segY1, segY2 ;
    T_sword32 dx, dy ;
    T_sword32 swap ;

    if (index == -1)
        return ;

    while ((i=G_3dBlockMapArray[index]) != 0xFFFF /* -1 */)  {
        if (G_numHits == MAX_LINE_HITS)
            break ;
        if (Collide3dCheckSegmentHitBox(
               i,
               x1,
               y1,
               x2,
               y2) != 0)  {
            if ((G_3dLineArray[i].flags & 0x01) ||
                (IIsFloorAndCeilingOk(i, TRUE, p_obj) == FALSE))  {
                segX1 = G_3dVertexArray[G_3dLineArray[i].from].x ;
                segY1 = G_3dVertexArray[G_3dLineArray[i].from].y ;
                segX2 = G_3dVertexArray[G_3dLineArray[i].to].x ;
                segY2 = G_3dVertexArray[G_3dLineArray[i].to].y ;

//                if ((G_3dLineArray[i].flags & LINE_IS_TWO_SIDED) ||
//                    (Collide3dPointOnRight(segX1, segY1, segX2, segY2, lastX, lastY)!=0))  {
                    dx = segX2 - segX1 ;
                    dy = segY2 - segY1 ;

                    if (dx == 0)  {
                        /* This is a vertical bar. */
                        /* Make point 1 be on the bottom and 2 on top, if */
                        /* not already. */
                        if (dy < 0)  {
                            swap = segX1 ;
                            segX1 = segX2 ;
                            segX2 = swap ;

                            swap = segY1 ;
                            segY1 = segY2 ;
                            segY2 = swap ;
                        }
                        if ((lastY >= (segY1-radius)) && (lastY <= (segY2+radius)))  {
                            dx = 0 ;
                            dy = 1 ;
                        } else {
                            dx = 1 ;
                            dy = 0 ;
                        }
                    } else if (dy == 0)  {
                        /* The line is a horizontal bar along the x. */
                        /* Put point 1 on left of point 2. */
                        if (dx < 0)  {
                            swap = segX1 ;
                            segX1 = segX2 ;
                            segX2 = swap ;
                        }
                        if ((lastX >= (segX1-radius)) && (lastX <= (segX2+radius)))  {
                            dx = 1 ;
                            dy = 0 ;
                        } else {
                            dx = 0 ;
                            dy = 1 ;
                        }
                    } else if (dy < 0)  {

                        /* TEMP */
                        dx = segX2 - segX1 ;
                        dy = segY2 - segY1 ;
                    } else /* dy > 0 */  {
                        /* TEMP */
                        dx = segX2 - segX1 ;
                        dy = segY2 - segY1 ;
                    }

                    G_hitSlopeXs[G_numHits] = dx ;
                    G_hitSlopeYs[G_numHits] = dy ;
                    G_hitType[G_numHits] = HIT_TYPE_LINE ;
                    G_hitWho[G_numHits] = i ;

                    G_numHits++ ;
//                }
            }
        }
        index++ ;
    }
}

/*-------------------------------------------------------------------------*
 * Routine:  LineHit
 *-------------------------------------------------------------------------*/
/**
 *  LineHit is used to determine if any line collides with the given box.
 *  It also works to determine the sectors, their heights, etc. in the area.
 *
 *  NOTE: 
 *  Due to the way this routine works (find the four corners) and the
 *  size of the map blocks, do NOT call this routine with a box bigger than
 *  256x256.
 *
 *  @param lastX -- Old X location for angular information
 *  @param lastY -- Old Y location for angular information
 *  @param x1 -- Left corner of square
 *  @param y1 -- Upper corner of square
 *  @param x2 -- Right corner of square
 *  @param y2 -- Lower corner of square
 *  @param radius -- Radius of the given box
 *  @param p_obj -- Source object
 *
 *  @return TRUE=collision, else FALSE
 *
 *<!-----------------------------------------------------------------------*/
E_Boolean LineHit(
              T_sword16 lastX,
              T_sword16 lastY,
              T_sword16 x1,
              T_sword16 y1,
              T_sword16 x2,
              T_sword16 y2,
              T_sword16 radius,
              T_3dObject *p_obj)
{
    T_sword32 block1, block2, block3, block4 ;

    /* Reset the global data necessary for processing wall collisions. */
    G_numHits = 0 ;
    G_numSurroundingSectors = 0 ;
    G_shortestCeiling = 0x7FFE ;
    G_tallestFloor = -0x7FFE ;

    /* Determine the blocks at the four corners. */
    block1 = IGetBlock(x1, y1) ;
    block2 = IGetBlock(x1, y2) ;
    block3 = IGetBlock(x2, y2) ;
    block4 = IGetBlock(x2, y1) ;

    /* Check each corner for lines that are hit, but don't */
    /* repeat any blocks that we have already checked. */
    if (block1 != -1)
        ILineHitInBlock(lastX, lastY, x1, y1, x2, y2, block1, radius, p_obj) ;
    if ((block2 != -1) && (block2 != block1))
        ILineHitInBlock(lastX, lastY, x1, y1, x2, y2, block2, radius, p_obj) ;
    if ((block3 != -1) && (block3 != block1) && (block3 != block2))
        ILineHitInBlock(lastX, lastY, x1, y1, x2, y2, block3, radius, p_obj) ;
    if ((block4 != -1) &&
        (block4 != block1) &&
        (block4 != block2) &&
        (block4 != block3))
        ILineHitInBlock(lastX, lastY, x1, y1, x2, y2, block4, radius, p_obj) ;

    /* Did we hit anything? */
    if (G_numHits != 0)  {
        G_hitSlopeX = G_hitSlopeXs[0] ;
        G_hitSlopeY = G_hitSlopeYs[0] ;
        return TRUE ;
    }

    return FALSE ;
}

#if 0
/*-------------------------------------------------------------------------*
 * Routine:  View3dObjectHit
 *-------------------------------------------------------------------------*/
/**
 *  View3dObjectHit is used to determine if there are any objects in the
 *  given x, y, and radius (box).  In addition the slope is calculated.
 *
 *  @param y -- Location of point to check
 *  @param radius -- Radius around point to check
 *
 *  @return TRUE=object found, else FALSE
 *
 *<!-----------------------------------------------------------------------*/
E_Boolean View3dObjectHit(T_sword16 x, T_sword16 y, T_word16 radius)
{
    T_sword32 distance ;
    T_3dObject *p_obj ;
    T_sword32 dx, dy ;

    p_obj = G_First3dObject ;
    while (p_obj != NULL)  {
        if ((&(p_obj->objMove)) != G_exceptObjMove)  {
            if ((ObjectGetAttributes(p_obj) & OBJECT_ATTR_PASSABLE) == 0)  {
                distance = CalculateEstimateDistance(
                               x,
                               y,
                               ObjectGetX16(p_obj),
                               ObjectGetY16(p_obj)) ;
                if (((distance-radius)-ObjectGetRadius(p_obj)) < 0)  {
                    if (G_numHits < MAX_LINE_HITS)  {
                        dx = ObjectGetY16(p_obj) - y ;
                        if (dx < 0)
                            dx = -dx ;
                        dy = ObjectGetX16(p_obj) - x ;
                        if (dy < 0)
                            dy = -dy ;

                        if (dx < dy)  {
                            G_hitSlopeXs[G_numHits] = 0 ;
                            G_hitSlopeYs[G_numHits] = 256 ;
                        } else {
                            G_hitSlopeXs[G_numHits] = 256 ;
                            G_hitSlopeYs[G_numHits] = 0 ;
                        }
                        G_hitWho[G_numHits] = ObjectGetServerId(p_obj) ;
                        G_hitType[G_numHits] = HIT_TYPE_OBJECT ;  /* object */
                        if (G_numHits == 0)  {
                            G_hitSlopeX = G_hitSlopeXs[0] ;
                            G_hitSlopeY = G_hitSlopeYs[0] ;
                        }

                        G_numHits++ ;
                    }
                    return TRUE ;
                }
            }
        }
        p_obj = p_obj->nextObj ;
    }

    return FALSE ;
}
#endif

/*-------------------------------------------------------------------------*
 * Routine:  View3dObjectHitFast
 *-------------------------------------------------------------------------*/
/**
 *  View3dObjectHitFast is used to determine if an object will collide
 *  with another object or volume.
 *
 *  @param x -- Center X location to check
 *  @param y -- Center Y location to check
 *  @param radius -- Radius around point to check
 *  @param lastX -- Last X location for angular info.
 *  @param lastY -- Last Y location for angular info.
 *  @param zBottom -- Bottom of the object
 *  @param zTop -- Top of the object
 *  @param height -- Height of full object
 *  @param p_movingObject -- Object being moved
 *
 *  @return TRUE=object found, else FALSE
 *
 *<!-----------------------------------------------------------------------*/
E_Boolean View3dObjectHitFast(
              T_sword16 x,
              T_sword16 y,
              T_word16 radius,
              T_sword16 lastX,
              T_sword16 lastY,
              T_sword16 zBottom,
              T_sword16 zTop,
              T_sword16 height,
              T_3dObject *p_movingObject)
{
    T_word16 halfwidth ;
    T_3dObject *p_obj ;
    T_sword16 objX, objY ;
    T_sword16 dx, dy ;
    T_sword16 objBottom, objTop ;
    T_sword16 ceiling ;
    T_word16 ceilingSector ;
    T_doubleLinkListElement element ;
    T_sword16 hashX, hashY ;
    T_sword16 startHashX, startHashY ;
    T_word16 group ;
    T_word16 groupWidth ;

    DebugRoutine("View3dObjectHitFast") ;
//if (ObjectIsCreature(p_movingObject))  {
// if (ObjectGetServerId(p_movingObject) != 0)  {
//  printf("VOHF: %d %s\n", ObjectGetServerId(p_movingObject), DebugGetCallerName()) ;
/*
  printf("params: x:%d y:%d r:%d lX:%d lY:%d zBot:%d zTop:%d h:%d\n",
   x,
   y,
   radius,
   lastX,
   lastY,
   zBottom,
   zTop,
   height) ;
*/
// }
//}
//printf("%d) zBottom = %d, zTop = %d, c=%s (%d)\n", ObjectGetServerId(p_movingObject), zBottom, zTop, DebugGetCallerName(), ObjectGetServerId(PlayerGetObject())) ;
//fflush(stdout) ;
    DebugCheck(zBottom <= zTop) ;

    if (!ObjectIsFullyPassable(p_movingObject))  {
        groupWidth = 2+(ObjectGetRadius(p_movingObject) >> 5) ;
        startHashX = ((ObjectGetX16(p_movingObject) -
                      G_3dBlockMapHeader->xOrigin) >> 6) ;
        startHashY = ((ObjectGetY16(p_movingObject) -
                      G_3dBlockMapHeader->yOrigin) >> 6) ;
        for (hashY=-groupWidth; hashY<=groupWidth; hashY++)  {
            /* Don't do ones that are out of bounds. */
            if ((startHashY + hashY) < 0)
                continue ;
            if ((startHashY + hashY) >= G_objCollisionNumY)
                continue ;
            for (hashX=-groupWidth; hashX<=groupWidth; hashX++)  {
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

        /* moved from here. */
                    /* Determine "square" distance to be in */
                    halfwidth = ObjectGetRadius(p_obj) + radius ;
                    objX = ObjectGetX16(p_obj) ;
                    objY = ObjectGetY16(p_obj) ;

                    if ((x <= objX+halfwidth) &&
                        (x >= objX-halfwidth) &&
                        (y <= objY+halfwidth) &&
                        (y >= objY-halfwidth))  {
        /*
        printf("id: %d, x: %d, y: %d, objX: %d, objY: %d, halfwidth: %d\n",
            ObjectGetServerId(p_obj),
            x,
            y,
            objX,
            objY,
            halfwidth) ;
        */
        /* moved */
                    if (p_obj == p_movingObject)  {
        //puts("moving obj") ;
                        continue ;
                    }

                    if ((&(p_obj->objMove)) == G_exceptObjMove)  {
        //puts("except object") ;
                        continue ;
                    }

                    if (ObjectGetAttributes(p_obj) & OBJECT_ATTR_PASSABLE)  {
        //puts("passable") ;
                        continue ;
                    }
        /* moved */


        //if (ObjectIsCreature(p_movingObject))
        // printf("  3dobjhit %d (%d)\n", ObjectGetServerId(p_obj), ObjectGetType(p_obj)) ;
                        objBottom = ObjectGetZ16(p_obj) ;
                        objTop = objBottom + ObjectGetHeight(p_obj) - 1;
        //printf("objBottom: %d  objTop: %d\n", objBottom, objTop) ;
                        ceilingSector = ObjectGetAreaCeilingSector(p_obj) ;
                        if (ceilingSector > G_Num3dSectors)
                            ceilingSector = View3dFindSectorNum(x, y) ;
                        ceiling = MapGetCeilingHeight(ceilingSector) ;
        //printf("ceilingSector %d, ceiling %d\n", ceilingSector, ceiling) ;

                        if (objBottom >= zTop)  {
        //if (ObjectIsCreature(p_movingObject))
        //    printf("  objBottom > zTop\n") ;
                            continue ;
                        }

                        /* If we are over the top, continue.  Unless, we are */
                        /* going to go through the roof. */
                        if (objTop < zBottom)  {
                            if (!p_movingObject)  {
        //if (ObjectIsCreature(p_movingObject))
        //    printf("  !p_movingObject\n") ;
                                continue ;
                             }

                            if ((objTop + height) < (ObjectGetHighestPoint(p_movingObject)>>16))  {
        //if (ObjectIsCreature(p_movingObject))
        //    printf("  o+h\n") ;
                                continue ;
                            }
                        }

                        /* Hit! */
        //printf("G_numHits = %d\n", G_numHits) ;
                        if (G_numHits < MAX_LINE_HITS)  {
        /*
        if ((ObjectGetServerId(p_movingObject) / 100) != 91)
          if ((ObjectGetServerId(p_movingObject) / 100) != 90)
            printf("View3dObjectHitFast: %d hits %d\n", ObjectGetServerId(p_movingObject), ObjectGetServerId(p_obj)) ;
        */
        //    printf("  hit!\n") ;
        //ObjectPrint(stdout, p_movingObject) ;
        //ObjectPrint(stdout, p_obj) ;
                            dy = lastY - objY ;
                            dx = lastX - objX ;
        //if (ObjectIsCreature(p_movingObject))
        // printf("dx: %d, dy: %d\n", dx, dy) ;
                            if (((dx > dy) && (dx < -dy)) ||
                                ((dx > -dy) && (dx < dy)))  {
                                G_hitSlopeYs[G_numHits] = 0 ;
                                G_hitSlopeXs[G_numHits] = 1 ;
                            } else {
                                G_hitSlopeYs[G_numHits] = 1 ;
                                G_hitSlopeXs[G_numHits] = 0 ;
                            }
                            G_hitWho[G_numHits] = ObjectGetServerId(p_obj) ;
                            G_hitType[G_numHits] = HIT_TYPE_OBJECT ;  /* object */

                            if (G_numHits == 0)  {
                                G_hitSlopeX = G_hitSlopeXs[0] ;
                                G_hitSlopeY = G_hitSlopeYs[0] ;
                            }

#ifndef NDEBUG
                            if ((ObjectGetServerId(p_movingObject)/100) != 91)  {
                                if (ObjectGetServerId(p_movingObject) != 0)  {
                                    SyncMemAdd("Collision between %d and %d\n",
                                        ObjectGetServerId(p_movingObject),
                                        ObjectGetServerId(p_obj),
                                        0) ;
                                }
                            }
#endif
#if 0
printf("Collision between objects %d and %d\n", ObjectGetServerId(p_movingObject), ObjectGetServerId(p_obj)) ;
printf("  x=%04X, y=%04X, zBot=%04X, zTop=%04X, height=%d, lastX=%04X, lastY=%04X\n", x, y, zBottom, zTop, height, lastX, lastY) ;
printf("  mov x: %08lX, y: %08lX, z: %08lX, h:%d\n", ObjectGetX(p_movingObject), ObjectGetY(p_movingObject), ObjectGetZ(p_movingObject), ObjectGetHeight(p_movingObject)) ;
printf("  obj x: %08lX, y: %08lX, z: %08lX, h:%d\n", ObjectGetX(p_obj), ObjectGetY(p_obj), ObjectGetZ(p_obj), ObjectGetHeight(p_obj)) ;
#endif
                            G_numHits++ ;
                            DebugEnd() ;
                            return TRUE ;
                        } else {
        #ifndef NDEBUG
                            printf("View3dObjectHitFast: Overflow: %d (%p)\n",
                                ObjectGetServerId(p_movingObject),
                                p_movingObject) ;
        #endif
                            DebugCheck(FALSE) ;
                        }
                    }
                }
            }
        }
    }
    DebugEnd() ;
    return FALSE ;
}

/*-------------------------------------------------------------------------*
 * Routine:  View3dMoveToXY
 *-------------------------------------------------------------------------*/
/**
 *  View3dMoveTo is used to move the player from the current point
 *  to a new point.
 *
 *  @param oldX -- Start X location and place to put final
 *      ending location.
 *  @param oldY -- Start Y location and place to put final
 *      ending location.
 *  @param newx -- New X location to move to
 *  @param newy -- New Y location to move to
 *  @param radius -- Radius of collision detection.
 *  @param foot -- Foot height of object
 *  @param head -- Head height of object
 *  @param height -- Current height.
 *  @param p_obj -- Object moving
 *
 *<!-----------------------------------------------------------------------*/
T_sword16 View3dMoveToXY(
              T_sword32 *oldX,
              T_sword32 *oldY,
              T_sword32 newx,
              T_sword32 newy,
              T_sword32 radius,
              T_sword32 foot,
              T_sword32 head,
              T_sword16 height,
              T_3dObject *p_obj)
{
    T_sword16 status ;
    T_sword16 step ;

    DebugRoutine("View3dMoveToXY") ;

    G_currentHeight = foot>>16 ;

    step = 1+CalculateDistance(
               (*oldX) >> 16,
               (*oldY) >> 16,
               newx >> 16,
               newy >> 16) ;

//printf("foot=%d, head=%d, newx=%d, newy=%d, %d\n", foot>>16, head>>16, newx>>16, newy>>16, ObjectGetServerId(p_obj)) ;
    status = IMoveToXYWithStep(
               oldX,
               oldY,
               newx,
               newy,
               radius,
               step,
               foot,
               head,
               height,
               p_obj) ;

    DebugEnd() ;

    return status ;
}

/*-------------------------------------------------------------------------*
 * Routine:  View3dMoveToXYFast
 *-------------------------------------------------------------------------*/
/**
 *  View3dMoveToXYFast is used to move from a point to another point
 *  using the fast version of the algorithms.
 *
 *  @param oldX -- Start X location and place to put final
 *      ending location.
 *  @param oldY -- Start Y location and place to put final
 *      ending location.
 *  @param newx -- New X location to move to
 *  @param newy -- New Y location to move to
 *  @param foot -- Foot height of object
 *  @param head -- Head height of object
 *  @param radius -- Radius of collision detection.
 *  @param height -- Current height.
 *  @param p_obj -- Object moving
 *
 *<!-----------------------------------------------------------------------*/
T_sword16 View3dMoveToXYFast(
              T_sword32 *oldX,
              T_sword32 *oldY,
              T_sword32 newx,
              T_sword32 newy,
              T_sword32 radius,
              T_sword32 foot,
              T_sword32 head,
              T_sword16 height,
              T_3dObject *p_obj)
{
    T_sword16 step ;
    T_word16 sector ;
    E_Boolean hit ;

    sector = View3dFindSectorNum(
                 (T_sword16)(newx>>16),
                 (T_sword16)(newy>>16)) ;
    if (sector == 0xFFFF)
        return TRUE ;

    G_currentHeight = foot>>16 ;

    step = 1+CalculateDistance(
               (*oldX) >> 16,
               (*oldY) >> 16,
               newx >> 16,
               newy >> 16) ;

    G_numSurroundingSectors = 0 ;

    hit = MoveToFast(oldX, oldY, newx, newy, step, radius, foot, head, height, p_obj) ;

    return hit ;
}

/*-------------------------------------------------------------------------*
 * Routine:  View3dMoveTo
 *-------------------------------------------------------------------------*/
/**
 *  View3dMoveTo is used to move the player from the current point
 *  to a new point.
 *
 *  @param oldX -- X Start location and place to put final
 *      ending location.
 *  @param oldY -- Y Start location and place to put final
 *      ending location.
 *  @param angle -- What angle to move to
 *  @param step -- Amount to step along direction
 *  @param radius -- Radius of collision detection.
 *  @param foot -- Foot height of object moving
 *  @param head -- Head height of object moving
 *  @param height -- Current height.
 *  @param p_obj -- Pointer to object moving
 *
 *<!-----------------------------------------------------------------------*/
T_sword16 View3dMoveTo(
         T_sword32 *oldX,
         T_sword32 *oldY,
         T_word16 angle,
         T_sword32 step,
         T_sword32 radius,
         T_sword32 foot,
         T_sword32 head,
         T_sword16 height,
         T_3dObject *p_obj)
{
    T_sword32 newX, newY ;

    G_currentHeight = foot>>16 ;

    newX = *oldX + step * MathCosineLookup(angle) ;
    newY = *oldY + step * MathSineLookup(angle) ;

    step = easyabs(step) ;

    return IMoveToXYWithStep(
               oldX,
               oldY,
               newX,
               newY,
               radius,
               (T_sword16)step,
               foot,
               head,
               height,
               p_obj) ;
}

E_Boolean CheckLineCanStepThrough(T_word16 i, T_3dObject *p_obj)
{
    E_Boolean canStep = FALSE ;
    T_3dLine *p_line ;
    T_3dSector *p_sector1, *p_sector2 ;
    T_sword16 floor1, ceiling1, floor2, ceiling2 ;
    T_sword16 z ;
    T_sword16 top ;
    T_sword16 climb ;

    DebugRoutine("CheckLineCanStepThrough") ;
    DebugCheck(p_obj != NULL) ;
    DebugCheck(i < G_Num3dLines) ;

    p_line = G_3dLineArray + i ;

    /* Only 2 sided lines need consideration that are not impassible*/
    if ((p_line->flags & LINE_IS_TWO_SIDED) &&
        (!(p_line->flags & LINE_IS_IMPASSIBLE)))  {
        /* Get the floor and ceiling heights of the two sides */
        p_sector1 = G_3dSectorArray + G_3dSideArray[p_line->side[0]].sector ;
        floor1 = p_sector1->floorHt ;
        ceiling1 = p_sector1->ceilingHt ;
        p_sector2 = G_3dSectorArray + G_3dSideArray[p_line->side[1]].sector ;
        floor2 = p_sector2->floorHt ;
        ceiling2 = p_sector2->ceilingHt ;

//printf("    (%d, %d) - (%d, %d)\n", floor1, ceiling1, floor2, ceiling2) ;
        /* Determine the higher of the 2 floors. */
        if (floor2 > floor1)
            floor1 = floor2 ;

        /* Determine the lower of the 2 ceilings. */
        if (ceiling2 < ceiling1)
            ceiling1 = ceiling2 ;

        /* Get the top and bottom of the object in question. */
        z = ObjectGetZ16(p_obj) ;
        top = z + ObjectGetHeight(p_obj) ;
//printf("    z = %d, top = %d\n", z, top) ;
//printf("    f:%d, c:%d\n", floor1, ceiling1) ;
        /* If we can squeeze through, alright! */
        if ((z >= floor1) && (top < ceiling1))  {
            canStep = TRUE ;
//puts("  can step up without climb") ;
        } else {
            if (z < floor1)  {
                /* If not, try stepping up and squeezing through. */
                climb = ObjectGetClimbHeight(p_obj) ;
                if ((z + climb) > floor1)
                    z = floor1 ;
                top = z + ObjectGetHeight(p_obj) ;
//printf("    zstep = %d, topstep = %d\n", z, top) ;

                if ((z >= floor1) && (top < ceiling1))  {
                    canStep = TRUE ;
//puts("  can step up with climb") ;
                } else {
//puts("  cannot step up with climb") ;
                }
            } else {
//puts("  ceiling too low") ;
            }
        }
    } else {
//puts("  impassible") ;
    }

    DebugEnd() ;

    return canStep ;
}

extern T_byte8 IOnRightOfLine(T_sword16 x, T_sword16 y, T_word16 line) ;

/*-------------------------------------------------------------------------*
 * Routine:  IMoveToXYWithStep
 *-------------------------------------------------------------------------*/
/**
 *  IMoveToXYWithStep moves the player to given new location and must
 *  also be given the amount of distance is being traveled.
 *
 *  NOTE: 
 *  G_currentHeight MUST be set in order for this routine to work
 *  correctly.
 *
 *  @param oldX -- X Start location and place to put final
 *      ending location.
 *  @param oldY -- Y Start location and place to put final
 *      ending location.
 *  @param newX -- End X location to move to
 *  @param newY -- End Y location to move to
 *  @param radius -- Radius of collision detection.
 *  @param step -- Amount to step along direction
 *  @param foot -- Foot level
 *  @param head -- Head level
 *  @param height -- Height of this object
 *  @param p_obj -- Object doing the movement
 *
 *<!-----------------------------------------------------------------------*/
T_sword16 IMoveToXYWithStep(
              T_sword32 *oldX,
              T_sword32 *oldY,
              T_sword32 newX,
              T_sword32 newY,
              T_sword32 radius,
              T_sword32 step,
              T_sword32 foot,
              T_sword32 head,
              T_sword16 height,
              T_3dObject *p_obj)
{
    T_sword16 code = 0 ;
    T_sword32 offX, offY ;
    T_word16 i ;
    T_word16 j ;
    T_word16 count = 0 ;
    T_word16 angle, angle2 ;
    T_sword32 mag ;
    T_sword32 velX, velY ;
    T_sword32 leftX, rightX, topY, bottomY ;
    T_word16 vertexNum ;
    T_sword16 vertexX, vertexY ;
    T_word16 otherI, otherJ ;
    T_word16 angleI, angleJ, angleF, angleDistI, angleDistJ ;
    T_sword32 lastX, lastY ;
    T_sword16 origX, origY ;
    E_Boolean hitStepThrough[MAX_LINE_HITS] ;
    E_Boolean anyNonStep ;

    origX = (*oldX>>16) ;
    origY = (*oldY>>16) ;

    G_hitDistance = 0 ;
    G_hitSlopeX = G_hitSlopeY = 0 ;

//    G_numSurroundingSectors = 0 ;

//printf("xy: f: %d h: %d\n", foot>>16, head>>16) ;
//    while (/* (count < 3) && */ (MoveTo(*oldX, *oldY, newX, newY, step, radius, foot, head, height, p_obj) == TRUE))  {
    while (/* (count < 3) && */ (MoveTo(*oldX, *oldY, newX, newY, step, p_obj) == TRUE))  {
//printf("--xy: f: %d h: %d\n", foot>>16, head>>16) ;
//printf("***************** count = %d\n", count) ;
        /* Compute the radial box around the origin. */
        leftX = rightX = (newX)>>16 ;
        leftX -= radius ;
        rightX += radius ;
        topY = bottomY = (newY)>>16 ;
        topY -= radius ;
        bottomY += radius ;
        lastX = *oldX ;
        lastY = *oldY ;

//puts("PREP:") ;
        /* Determine all the lines that are TRULY blocking the */
        /* object. */
        anyNonStep = FALSE ;
        for (i=0; i<G_numHits; i++)  {
            if (G_hitType[i] == HIT_TYPE_LINE)  {
//printf("  line %d check\n", G_hitWho[i]) ;
                if ((hitStepThrough[i] = CheckLineCanStepThrough(G_hitWho[i], p_obj)) == FALSE)
                    anyNonStep = TRUE ;
            } else {
                anyNonStep = TRUE ;
            }
        }

        /* If none of the lines are really blocking, then we are done. */
#if 0
        if (anyNonStep == FALSE)  {
//puts("quick leave") ;
            *oldX = newX ;
            *oldY = newY ;
//            *oldX = G_hitAtX ;
//            *oldY = G_hitAtY ;
            return code ;
        }
#endif

        /* Mark this thing as colliding with something */
        *oldX = G_hitAtX ;
        *oldY = G_hitAtY ;
        offX = (newX - G_hitAtX) ;
        offY = (newY - G_hitAtY) ;
        code |= 1 ;

#if 0
        /* Lower the scale of the slopes to ensure that */
        /* the calculations don't blow up. */
        for (i=0; i<G_numHits; i++)  {
            while ((G_hitSlopeXs[i] > 128) || (G_hitSlopeYs[i] > 128))  {
                G_hitSlopeXs[i] >>= 1 ;
                G_hitSlopeYs[i] >>= 1 ;
            }
        }
#endif

        for (i=0; i<G_numHits; i++)  {
            j = G_numHits ;
//printf("i: %d, type: %d %d (%d %d)\n", i, G_hitType[i], G_hitWho[i], G_hitSlopeXs[i], G_hitSlopeYs[i]) ;
            /* Only consider lines ('cause they have endpoints) */
            if (G_hitType[i] == HIT_TYPE_LINE)  {
                /* Skip lines we can jump through */
                if (hitStepThrough[i] == TRUE)  {
//printf("   line %d can step through\n", G_hitWho[i]) ;
                    continue ;
                } else {
//printf("   line %d can't step through\n", G_hitWho[i]) ;
                }

                /* See if an endpoint is in the box. */
                vertexNum = G_3dLineArray[G_hitWho[i]].from ;
//printf("  vertex num from %d\n", vertexNum) ;
                vertexX = G_3dVertexArray[vertexNum].x ;
                vertexY = G_3dVertexArray[vertexNum].y ;
                if ((vertexX < leftX) ||
                    (vertexX > rightX) ||
                    (vertexY < topY) ||
                    (vertexY > bottomY))  {
                    /* try the other end. */
                    vertexNum = G_3dLineArray[G_hitWho[i]].to ;
//printf("  vertex num to %d\n", vertexNum) ;
                    vertexX = G_3dVertexArray[vertexNum].x ;
                    vertexY = G_3dVertexArray[vertexNum].y ;
                    if ((vertexX < leftX) ||
                        (vertexX > rightX) ||
                        (vertexY < topY) ||
                        (vertexY > bottomY))  {
                        vertexNum = 0xFFFF ;
                    } else {
                        /* Note who is on the other end. */
                        otherI = G_3dLineArray[G_hitWho[i]].from ;
                    }
                } else {
                    /* this is the correct end, note the other end. */
                    otherI = G_3dLineArray[G_hitWho[i]].to ;
                }

                /* Did we find a line with an endpoint? */
                if (vertexNum != 0xFFFF)  {
//printf("  try vertex %d\n", vertexNum) ;
                    /* Check this line with all the other lines */
                    /* and see if there are matching end points. */
                    for (j=0; j<G_numHits; j++)  {
                         if (i == j)
                             continue ;

                         if (hitStepThrough[j] == TRUE)  {
//printf("   2) line %d can step through\n", G_hitWho[j]) ;
                             continue ;
                         }

                        /* Only consider other lines. */
                        if (G_hitType[j] == HIT_TYPE_LINE)  {
                            if (G_hitWho[j] == G_hitWho[i])  {
                                /* No comparison, same lines! */
//printf("  same!\n") ;
                                continue ;
                            }
                            if ((G_3dLineArray[G_hitWho[j]].from == vertexNum) ||
                                (G_3dLineArray[G_hitWho[j]].to == vertexNum))  {
//printf("  match j = %d\n", j) ;
                                /* Matching end point!  Do we do this line */
                                /* (i'th line) or the other line (jth line)? */

                                /* Who is on the other end of this line (j'th) ? */
                                if (G_3dLineArray[G_hitWho[j]].from == vertexNum)  {
                                    otherJ = G_3dLineArray[G_hitWho[j]].to ;
                                } else {
                                    otherJ = G_3dLineArray[G_hitWho[j]].from ;
                                }

//printf("    otherI = %d, otherJ = %d\n", otherI, otherJ) ;

                                /* Compute the angles from this vertex */
                                /* to the other ends. */
                                angleI = MathArcTangent(
                                             G_3dVertexArray[otherI].x - vertexX,
                                             G_3dVertexArray[otherI].y - vertexY) ;
                                angleJ = MathArcTangent(
                                             G_3dVertexArray[otherJ].x - vertexX,
                                             G_3dVertexArray[otherJ].y - vertexY) ;
                                angleF = MathArcTangent(
                                             (origX - vertexX),
                                             (origY - vertexY)) ;
//                                angleF = MathArcTangent(
//                                             ((lastX)>>16) - vertexX,
//                                             ((lastY)>>16) - vertexY) ;

                                angleDistI = angleF - angleI ;
                                if (angleDistI > INT_ANGLE_180)
                                    angleDistI = -angleDistI ;

                                angleDistJ = angleF - angleJ ;
                                if (angleDistJ > INT_ANGLE_180)
                                    angleDistJ = -angleDistJ ;

//printf("    angleI = %04X, angleJ = %04X, angleF = %04X\n", angleI, angleJ, angleF) ;
//printf("    distI = %04X, distJ = %04X\n", angleDistI, angleDistJ) ;
                                if (angleDistI < angleDistJ)  {
                                    /* i'th line is closer than j'th line */
                                    /* Ignore j */
//printf("    keep i\n") ;
                                } else if (angleDistI == angleDistJ) {
                                    if (i < j)  {
                                    } else {
                                        break ;
                                    }
                                } else {
                                    /* j'th line is closer than i'th line */
                                    /* Take this j, not the i line */
//printf("    keep j\n") ;
                                    break ;
                                }
                            } else {
                                /* This line did not have a matching */
                                /* end point. Ignore this line (j'th line) */
                            }
                        }
                    }
                }
            } else {
                /* Not a line, so go ahead and do this. */
                j = G_numHits ;
            }

//printf("  j = %d %d\n", j, G_numHits) ;
            if (j == G_numHits)  {
            offX >>= 10 ;
            offY >>= 10 ;
//printf("XY Hit %d:  %ld %ld, pre: %ld, %ld", i, G_hitSlopeXs[i], G_hitSlopeYs[i], offX, offY) ;
            ProjectXYOntoLine(&offX, &offY, G_hitSlopeXs[i], G_hitSlopeYs[i]) ;
//printf("  post: %ld, %ld\n", offX, offY) ;
            offX <<= 10 ;
            offY <<= 10 ;

            if (count > 4)  {
                angle = INT_ANGLE_270 + MathArcTangent(
                                           G_hitSlopeXs[i],
                                           G_hitSlopeYs[i]) ;
                if ((G_hitType[i] == HIT_TYPE_LINE) && (IOnRightOfLine(lastX>>16, lastY>>16, G_hitWho[i])))
                    angle += INT_ANGLE_180 ;

                offX += (MathCosineLookup(angle))*2 ;
                offY += (MathSineLookup(angle))*2 ;
            }

            if (ObjectGetAttributes(p_obj) & OBJECT_ATTR_SLIDE_ONLY)  {
                /* Affect the velocity too. */
                p_obj->objMove.XV>>=12 ;
                p_obj->objMove.YV>>=12 ;
//printf("   Vel %d:  %ld %ld, pre: %ld, %ld", i, G_hitSlopeXs[i], G_hitSlopeYs[i], p_obj->objMove.XV, p_obj->objMove.YV) ;
                ProjectXYOntoLine(
                    &p_obj->objMove.XV,
                    &p_obj->objMove.YV,
                    G_hitSlopeXs[i],
                    G_hitSlopeYs[i]) ;
//printf("  post: %ld, %ld\n", p_obj->objMove.XV, p_obj->objMove.YV) ;
                p_obj->objMove.XV<<=12 ;
                p_obj->objMove.YV<<=12 ;
            } else {
//puts("reflect") ;
                /* Reflect off the surface. */
                velX = p_obj->objMove.XV ;
                velY = p_obj->objMove.YV ;

                angle = INT_ANGLE_90 + MathArcTangent32(
                                           G_hitSlopeXs[i],
                                           G_hitSlopeYs[i]) ;
                angle2 = MathArcTangent32(velX, velY) ;
                mag = (CalculateDistance(0, 0, velX>>8, velY>>8))<<8 ;
                angle2 = INT_ANGLE_180 + angle + (angle - angle2) ;
                velX = Mult32By32AndDiv32(
                           mag,
                           MathCosineLookup(angle2),
                           0x10000) ;
                velY = Mult32By32AndDiv32(
                           mag,
                           MathSineLookup(angle2),
                           0x10000) ;
                p_obj->objMove.YV = velY ;
                p_obj->objMove.XV = velX ;
            }
            }
        }
        newX = G_hitAtX + offX ;
        newY = G_hitAtY + offY ;

        if ((newX == G_hitAtX) && (newY == G_hitAtY))  {
            return code ;
        }

//        G_numSurroundingSectors = 0 ;
        G_hitSlopeX = G_hitSlopeY = 0 ;

        step -= G_hitDistance ;
        if (step < 0)
            step = 0 ;

        count++ ;
#if 0
        if (count == 3)  {
            if (G_hitSlopeXs[i] >= G_hitSlopeYs[i])  {
                G_hitSlopeXs[i] = 1 ;
                G_hitSlopeYs[i] = 0 ;
            } else {
                G_hitSlopeXs[i] = 0 ;
                G_hitSlopeYs[i] = 1 ;
            }
        }
#endif

        if (count == 10)  {
//            *oldX = lastX ;
//            *oldY = lastY ;
            return code ;
        }
    }
    *oldX = newX ;
    *oldY = newY ;

    return code ;
}

/*-------------------------------------------------------------------------*
 * Routine:  View3dGetFloorAndCeilingHeight
 *-------------------------------------------------------------------------*/
/**
 *  View3dGetFloorAndCeilingHeight is used to get the highest and lowest
 *  floor and ceiling of the last collisional move.
 *
 *  @param floor -- Returned floor height
 *  @param ceiling -- Returned ceiling height
 *
 *<!-----------------------------------------------------------------------*/
T_void View3dGetFloorAndCeilingHeight(T_sword16 *floor, T_sword16 *ceiling)
{
    DebugRoutine("View3dGetFloorAndCeilingHeight") ;

    *floor = G_tallestFloor ;
    *ceiling = G_shortestCeiling ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  View3dGetFloorAbove
 *-------------------------------------------------------------------------*/
/**
 *  View3dGetFloorAbove returns the highest floor that the collisional
 *  move moved over.
 *
 *  @param above -- Returned floor
 *
 *<!-----------------------------------------------------------------------*/
T_void View3dGetFloorAbove(T_word16 *above)
{
    DebugRoutine("View3dGetFloorAbove") ;
    DebugCheck(above != NULL) ;

    *above = G_floorAbove ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  View3dGetCeilingBelow
 *-------------------------------------------------------------------------*/
/**
 *  View3dGetCeilingBelow returns the lowest ceiilng that the collisional
 *  move moved over.
 *
 *  @param below -- Returned ceiling
 *
 *<!-----------------------------------------------------------------------*/
T_void View3dGetCeilingBelow(T_word16 *below)
{
    DebugRoutine("View3dGetCeilngBelow") ;
    DebugCheck(below != NULL) ;

    *below = G_ceilingBelow ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  View3dSetExceptObject
 *-------------------------------------------------------------------------*/
/**
 *  View3dSetExceptObject declares   what object is to be consider non-
 *  collidable
 *
 *  @param exceptId -- ID of object to exclude
 *
 *<!-----------------------------------------------------------------------*/
T_void View3dSetExceptObject(T_word16 exceptId)
{
    T_3dObject *p_obj ;

    DebugRoutine("View3dSetExceptObject") ;

    p_obj = ObjectFind(exceptId) ;
    DebugCheck(p_obj != NULL) ;

    G_exceptObjMove = &(p_obj->objMove) ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  View3dSetExceptObjectByPtr
 *-------------------------------------------------------------------------*/
/**
 *  View3dSetExceptObjectByPtr declares what object is to be consider non-
 *  collidable
 *
 *  @param p_objMove -- Direct pointer to object's move struc
 *      to exclude.
 *
 *<!-----------------------------------------------------------------------*/
T_void View3dSetExceptObjectByPtr(T_objMoveStruct *p_objMove)
{
    DebugRoutine("View3dSetExceptObjectByPtr") ;

    G_exceptObjMove = p_objMove ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  View3dGetSurroundingSectors
 *-------------------------------------------------------------------------*/
/**
 *  This routine returns a pointer to a list of sectors and the number
 *  sectors in that list.
 *
 *  NOTE: 
 *  The data at the returned pointer is NOT to be modified, just copied.
 *
 *  @param numSectors -- Returned number of sectors in list.
 *
 *  @return Pointer to list of sectors
 *
 *<!-----------------------------------------------------------------------*/
T_word16 *View3dGetSurroundingSectors(T_word16 *numSectors)
{
    DebugRoutine("View3dGetSurroundingSectors") ;

    *numSectors = G_numSurroundingSectors ;

    DebugEnd() ;

    return G_surroundingSectors ;
}

/*-------------------------------------------------------------------------*
 * Routine:  View3dMoveToFast
 *-------------------------------------------------------------------------*/
/**
 *  View3dMoveToFast is the same as View3dMoveTo but does not try to
 *  slide and thus is a faster version.
 *
 *<!-----------------------------------------------------------------------*/
T_sword16 View3dMoveToFast(
         T_sword32 *oldX,
         T_sword32 *oldY,
         T_word16 angle,
         T_sword32 step,
         T_sword32 radius,
         T_sword32 foot,
         T_sword32 head,
         T_sword32 height,
         T_3dObject *p_obj)
{
    T_sword16 code = 0 ;
    T_sword32 newX, newY ;

    G_currentHeight = foot>>16 ;

    newX = *oldX + step * MathCosineLookup(angle) ;
    newY = *oldY + step * MathSineLookup(angle) ;

    step = easyabs(step) ;
    G_numSurroundingSectors = 0 ;

    code = MoveToFast(
               oldX,
               oldY,
               newX,
               newY,
               step,
               radius,
               foot,
               head,
               (T_sword16)height,
               p_obj) ;

    return code ;
}

/*-------------------------------------------------------------------------*
 * Routine:  MoveToFast
 *-------------------------------------------------------------------------*/
/**
 *  MoveToFast takes a given object and tries to move it to another
 *  location.  If a collision is detected, a flag is returned and the
 *  global variables are filled with additional information.
 *
 *  @param oldX -- X Start point, and returned final point
 *  @param oldY -- Y Start point, and returned final point
 *  @param newX -- X Target end-point
 *  @param newY -- Y Target end-point
 *  @param distance -- How far along this is (status)
 *  @param radius -- Size of object
 *  @param foot -- Foot height (foot + climbHeight)
 *  @param head -- top of head
 *  @param height -- Height of object (fully)
 *  @param p_movingObject -- Object being moved
 *
 *  @return TRUE=collision
 *
 *<!-----------------------------------------------------------------------*/
E_Boolean MoveToFast(
         T_sword32 *oldX,
         T_sword32 *oldY,
         T_sword32 newX,
         T_sword32 newY,
         T_sword32 distance,
         T_sword32 radius,
         T_sword32 foot,
         T_sword32 head,
         T_sword16 height,
         T_3dObject *p_movingObject)
{
    T_sword32 left, right, top, bottom ;
    T_word16 surroundingSectors[20] ;
    T_word16 numSurroundingSectors ;

T_sword32 ooldX, ooldY ;
T_sword32 ooldX2, ooldY2 ;
T_sword32 midX, midY ;

    ooldX2 = ooldX = *oldX ;
    ooldY2 = ooldY = *oldY ;

    G_shortestCeiling = 0x7FFE ;
    G_tallestFloor = -0x7FFE ;

    /* Store previous hit condition. */
    memcpy(
        surroundingSectors,
        G_surroundingSectors,
        sizeof(G_surroundingSectors)) ;
    numSurroundingSectors = G_numSurroundingSectors ;
    G_numSurroundingSectors = 0 ;

//    while (distance > (1+(radius>>1)))  {
//    while (distance > 3)  {
    while (distance >= radius)  {
        midX = (ooldX+newX)>>1 ;
        midY = (ooldY+newY)>>1 ;
        if (MoveToFast(
               &ooldX,
               &ooldY,
               midX,
               midY,
               (distance>>1),
               radius,
               foot,
               head,
               height,
               p_movingObject) == TRUE)  {
            /* I've hit something.  Record where this occured. */
//            *oldX = ooldX ;
//            *oldY = ooldY ;
            /* We hit something, this isn't good for our end position. */
            /* Restore the last surrounding sectors. */
            memcpy(
                G_surroundingSectors,
                surroundingSectors,
                sizeof(G_surroundingSectors)) ;
            G_numSurroundingSectors = numSurroundingSectors ;

            return TRUE ;
        }
        distance -= (distance>>1) ;
        *oldX = ooldX = midX ;
        *oldY = ooldY = midY ;
    }

    left = (newX>>16) - radius ;
    right = (newX>>16) + radius ;
    top = (newY>>16) - radius ;
    bottom = (newY>>16) + radius ;

    if (LineHit(
          (T_sword16)((*oldX)>>16),
          (T_sword16)((*oldY)>>16),
          left,
          top,
          right,
          bottom,
          (T_sword16)radius,
          p_movingObject) == TRUE)  {
        /* We hit something, this isn't good for our end position. */
        /* Restore the last surrounding sectors. */
        memcpy(
            G_surroundingSectors,
            surroundingSectors,
            sizeof(G_surroundingSectors)) ;
        G_numSurroundingSectors = numSurroundingSectors ;
        return TRUE ;
    }

    if (View3dObjectHitFast(
            ((T_sword16)(newX>>16)),
            ((T_sword16)(newY>>16)),
            (T_sword16)radius,
            (T_sword16)((*oldX)>>16),
            (T_sword16)((*oldY)>>16),
            (T_sword16)(foot>>16),
            (T_sword16)(head>>16),
            height,
            p_movingObject) == TRUE)  {
        /* We hit something, this isn't good for our end position. */
        /* Restore the last surrounding sectors. */
        memcpy(
            G_surroundingSectors,
            surroundingSectors,
            sizeof(G_surroundingSectors)) ;
        G_numSurroundingSectors = numSurroundingSectors ;
        return TRUE ;
    }

    *oldX = newX ;
    *oldY = newY ;

    return FALSE ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ICheckLineHitsLine
 *-------------------------------------------------------------------------*/
/**
 *  ICheckLineHitsLine takes in the coordinates of a line and determines
 *  if it hits the given line number on the map.  If they do hit, the
 *  distance to that line is returned.
 *
 *  @param x1 -- X First point of line to check
 *  @param y1 -- X First point of line to check
 *  @param x2 -- Y Second point of line to check
 *  @param y2 -- Y Second point of line to check
 *  @param lineNum -- Line to check collision with
 *
 *  @return Distance along line of where hit occurs
 *      or -1 if no hit.
 *
 *<!-----------------------------------------------------------------------*/
T_sword16 ICheckLineHitsLine(
              T_sword16 x1,
              T_sword16 y1,
              T_sword16 x2,
              T_sword16 y2,
              T_word16 lineNum)
{
    T_sword16 lx1, lx2, ly1, ly2 ;
    T_sword16 left, right, top, bottom ;
    T_sword32 ox1, ox2, oy1, oy2 ;
    T_sword32 rx1, rx2, ry1, ry2 ;
    T_sword16 t ;
    T_sword32 x ;

    if (x1 < x2)  {
        left = x1 ;
        right = x2 ;
    } else  {
        left = x2 ;
        right = x1 ;
    }

    if (y1 < y2)  {
        top = y1 ;
        bottom = y2 ;
    } else {
        top = y2 ;
        bottom = y1 ;
    }

    lx1 = G_3dVertexArray[G_3dLineArray[lineNum].from].x ;
    ly1 = G_3dVertexArray[G_3dLineArray[lineNum].from].y ;
    lx2 = G_3dVertexArray[G_3dLineArray[lineNum].to].x ;
    ly2 = G_3dVertexArray[G_3dLineArray[lineNum].to].y ;

    if (((lx1 < left) && (lx2 < left)) ||
        ((lx1 > right) && (lx2 > right)) ||
        ((ly1 < top) && (ly2 < top)) ||
        ((ly1 > bottom) && (ly2 > bottom)))  {
        return -1 ;
    }

    /* Now we have to do the real check by finding an intersection. */
    ox1 = lx1 - x1 ;
    oy1 = ly1 - y1 ;
    ox2 = lx2 - x1 ;
    oy2 = ly2 - y1 ;

    rx1 = (ox1 * G_sinLineAngle + oy1 * G_cosLineAngle)>>16 ;
    ry1 = (ox1 * G_cosLineAngle - oy1 * G_sinLineAngle)>>16 ;
    rx2 = (ox2 * G_sinLineAngle + oy2 * G_cosLineAngle)>>16 ;
    ry2 = (ox2 * G_cosLineAngle - oy2 * G_sinLineAngle)>>16 ;

    /* Swap Y's to make ry1 less than ry2.  For what we are doing, */
    /* it doesn't matter if the line is mirror along the y axis. */
    if (ry1 > ry2)  {
        t = ry1 ;
        ry1 = ry2 ;
        ry2 = t ;
    }

    /* Does the line intersect the X axis? */
    if ((ry1 > 0) || (ry2 < 0))
        /* It does not straddle the axis--it is outside. */
        return -1 ;

    /* Where on the x axis does it intersect? */
    x = rx1 - (((rx2-rx1)*ry1) / (ry2-ry1)) ;  //!!! DIVIDE ZERO ERROR HERE

    return x ;
}

/*-------------------------------------------------------------------------*
 * Routine:  View3dFindLineHits
 *-------------------------------------------------------------------------*/
/**
 *  View3dFindLineHits takes the given line coordinates and array and
 *  determines a list of lines that intersect.  Instead of returning
 *  intersection coordinates, distances to that line is returned.
 *
 *  @param x1 -- First X point of line to check
 *  @param y1 -- First Y point of line to check
 *  @param x2 -- Second X point of line to check
 *  @param y2 -- Second Y point of line to check
 *  @param maxHits -- Maximum number of array points
 *  @param lines -- List of lines and distances found.
 *      The array alternates between line
 *      number and then distance.
 *
 *  @return Number of lines hit.
 *
 *<!-----------------------------------------------------------------------*/
T_word16 View3dFindLineHits(
           T_sword16 x1,
           T_sword16 y1,
           T_sword16 x2,
           T_sword16 y2,
           T_word16 maxHits,
           T_sword16 *lines)
{
    T_word16 i ;
    T_word16 count = 0 ;
    T_word16 theta ;
    T_sword16 dx, dy ;
    T_sword16 dist ;

    /* Get the angle of the line so that we can use it in our */
    /* calculations for line interestion. */
    dy = y2-y1 ;
    dx = x2-x1 ;
    theta = MathArcTangent(dy, dx) ;
    G_cosLineAngle = MathCosineLookup(theta) ;
    G_sinLineAngle = MathSineLookup(theta) ;
//    G_numSurroundingSectors = 0 ;

    /* Check all lines to see if they intersect with the line */
    /* in question. */
    for (i=0; (i<G_Num3dLines) && (count < maxHits); i++)  {
        if ((dist = ICheckLineHitsLine(
               x1,
               y1,
               x2,
               y2,
               i)) >= 0)  {
            *(lines++) = i ;
            *(lines++) = dist ;
            count++ ;
        }
    }

    return count ;
}

/*-------------------------------------------------------------------------*
 * Routine:  Mult32By32AndDiv32
 *-------------------------------------------------------------------------*/
/**
 *  Mult32By32AndDiv32 is more of a macro to compute the following -
 *  f(a, b, c) = (a * b) / c
 *  Since all the values are 32 bit, the first multiply can result in
 *  a 64 bit number.  If assembly is turned on, this 64 bit number is
 *  can be quickly divided by c since the internal registers hold this
 *  value correctly.  However, if assembly is turned off, all the values
 *  are shifted right by 16 bits, but the answer is roughly the same.
 *  Unfortunately, this can cause problems in accuracy and divide by zero
 *  errors.
 *
 *  NOTE: 
 *  See above.  Calling routine must be sure that c is not zero.
 *
 *  @param a -- Value to multiply
 *  @param b -- Value to multiply
 *  @param c -- Value to divide
 *
 *  @return (a * b) / c
 *
 *<!-----------------------------------------------------------------------*/
#ifdef NO_ASSEMBLY
T_sword32 Mult32By32AndDiv32(
              T_sword32 a,
              T_sword32 b,
              T_sword32 c)
{
	// It's slow, but it works
	double aa, bb, cc;
	double answer;

	aa = a;
	bb = b;
	cc = c;
	answer = (aa*bb)/cc;

	return (T_sword32)answer;
}
#endif

/*-------------------------------------------------------------------------*
 * Routine:  IIsOnLeftOfLine
 *-------------------------------------------------------------------------*/
/**
 *  IIsOnLeftOfLine tests to see if a point is on the left side
 *  of a line (left is defined as you look from the start/first point
 *  and look at the end/second point) or if the point is on the line.
 *
 *  @param pointY -- Point to test if on left or equal
 *  @param lineY1 -- Start/First point
 *  @param lineY2 -- End/Second point
 *
 *  @return TRUE=on side of line, else FALSE
 *
 *<!-----------------------------------------------------------------------*/
static E_Boolean IIsOnLeftOfLine(
                     T_sword32 pointX,
                     T_sword32 pointY,
                     T_sword32 lineX1,
                     T_sword32 lineY1,
                     T_sword32 lineX2,
                     T_sword32 lineY2)
{
    T_sword32 px, py ;
    T_sword32 dx, dy ;

    /* Translate to put one end of line at origin (now line is */
    /* really a slope. */
    px = pointX - lineX1 ;
    py = pointY - lineY1 ;
    dx = lineX2 - lineX1 ;
    dy = lineY2 - lineY1 ;

    /* Multiply and subtract */
    if (Mult32x32AndCompare(px, dy, py, dx) > 0)
        return TRUE ;
    return FALSE ;

}

#ifdef NO_ASSEMBLY
/*-------------------------------------------------------------------------*
 * Routine:  Mult32x32AndCompare
 *-------------------------------------------------------------------------*/
/**
 *  Mult32x32AndCompare is a specialize routine that does an estimate
 *  of the following fuction -
 *  f(a,b,c,d) = (c*d) - (a*b)
 *  The reason it is an estimate is that you can only depend on getting
 *  three values -
 *  0 if the two sides are equal.
 *  POSITIVE if (c*d) is greater than (a*b)
 *  NEGATIVE if (c*d) is less than (a*b)
 *
 *  NOTE: 
 *  The C routine is a 32 bit approximization of the assembly version
 *  which is true to 64 bits.  Downgrading may cause more 0's returned
 *  than expected.
 *
 *  @param a,b,c,d -- Input values
 *
 *  @return 0, POSITIVE, or NEGATIVE
 *
 *<!-----------------------------------------------------------------------*/
T_sword32 Mult32x32AndCompare(
              T_sword32 a,
              T_sword32 b,
              T_sword32 c,
              T_sword32 d)
{
    /* return (c*d) - (a*b) */

    /* Bring down the accuracy to avoid overflow. */
    while ((a & 0xFFFF0000) || (c & 0xFFFF0000))  {
        a >>= 4 ;
        c >>= 4 ;
    }
    /* Bring down the accuracy to avoid overflow. */
    while ((b & 0xFFFF0000) || (d & 0xFFFF0000))  {
        b >>= 4 ;
        d >>= 4 ;
    }

    return ((c*d)-(a*b)) ;
}
#endif

/*-------------------------------------------------------------------------*
 * Routine:  MoveTo
 *-------------------------------------------------------------------------*/
/**
 *  MoveTo takes a given object and tries to move it to another
 *  location.  If a collision is detected, a flag is returned and the
 *  global variables are filled with additional information.
 *
 *  @param oldX -- Start X point
 *  @param oldY -- Start Y point
 *  @param newX -- Target X end-point
 *  @param newY -- Target Y end-point
 *  @param distance -- How far along this is (status)
 *  @param p_obj -- Object being moved
 *
 *  @return TRUE=collision
 *
 *<!-----------------------------------------------------------------------*/
E_Boolean MoveTo(
         T_sword32 oldX,
         T_sword32 oldY,
         T_sword32 newX,
         T_sword32 newY,
         T_sword32 distance,
         T_3dObject *p_obj)
{
    T_sword32 halfX, halfY ;
    T_sword32 step ;
    T_sword32 hereX, hereY ;
    T_sword32 radius, foot, head ;
    T_sword32 height ;
    T_sword16 z ;
    E_Boolean hadHit = FALSE ;

    radius = ObjectGetRadius(p_obj) ;
    foot = ObjectGetZ(p_obj) ;
    height = ObjectGetHeight(p_obj) ;
    head = foot + (height<<16) ;
//    foot += (ObjectGetClimbHeight(p_obj) << 16) ;

    if (distance < radius)  {
//    if (distance <= 2)  {
        /* Short enough segment.  Check for intersections here. */
        hereX = newX ;
        hereY = newY ;
        G_hitAtX = oldX ;
        G_hitAtY = oldY ;
        G_hitDistance += distance ;

/*
        if (CheckVertexHit(
                oldX,
                oldY,
                hereX-(radius<<16),
                hereY-(radius<<16),
                hereX+(radius<<16),
                hereY+(radius<<16)) == TRUE)  {
            return TRUE ;
        }
*/

        if (LineHit(
                ((T_sword16)(oldX>>16)),
                ((T_sword16)(oldY>>16)),
                ((T_sword16)((hereX>>16)-radius)),
                ((T_sword16)((hereY>>16)-radius)),
                ((T_sword16)((hereX>>16)+radius)),
                ((T_sword16)((hereY>>16)+radius)),
                (T_sword16)radius,
                p_obj) == TRUE)  {
            return TRUE ;
        }
        if (View3dObjectHitFast(
                (T_sword16)(hereX>>16),
                (T_sword16)(hereY>>16),
                (T_sword16)radius,
                (T_sword16)(oldX>>16),
                (T_sword16)(oldY>>16),
                (T_sword16)(foot>>16),
                (T_sword16)(head>>16),
                (T_sword16)height,
                p_obj) == TRUE)  {
            return TRUE ;
        }
        z = ObjectGetZ16(p_obj) ;
        if (ICanSqueezeThroughWithClimb(
                p_obj,
                &z,
                ObjectGetClimbHeight(p_obj),
                ObjectGetHeight(p_obj),
                G_numSurroundingSectors,
                G_surroundingSectors) == FALSE)  {
//puts("MoveTo:  Can't squeeze") ;
            return TRUE ;
        }
    } else {
        /* Find the midpoint. */
        halfX = (oldX + newX)>>1 ;
        halfY = (oldY + newY)>>1 ;

        /* Halve the distance. */
        step = distance >> 1 ;

        /* Check first half of line. */
        if (MoveTo(oldX, oldY, halfX, halfY, step, p_obj) == TRUE)
            return TRUE ;

        /* Check second half of line. */
        if (MoveTo(halfX, halfY, newX, newY, distance-step, p_obj) == TRUE)
            return TRUE ;
    }

    return FALSE ;
}

/*-------------------------------------------------------------------------*
 * Routine:  Collide3dGetWallsInBox
 *-------------------------------------------------------------------------*/
/**
 *  Collide3dGetLinesInBox takes the given square (coordinate and square
 *  radius) and determines all walls that collide with this box.
 *
 *  @param x -- X Center of box
 *  @param y -- Y Center of box
 *  @param radius -- Radius of box
 *  @param maxWalls -- Max walls allowed to find
 *  @param p_walls -- Where to put the walls
 *
 *  @return Number walls/lines found
 *
 *<!-----------------------------------------------------------------------*/
T_word16 Collide3dGetWallsInBox(
             T_sword32 x,
             T_sword32 y,
             T_word16 radius,
             T_word16 maxWalls,
             T_word16 *p_walls)
{
    T_sword32 x1, y1, x2, y2 ;
    T_sword32 radius32 ;
    T_word16 numLines = 0 ;
    T_sword32 block1, block2, block3, block4 ;

    DebugRoutine("Collide3dGetWallsInBox") ;
    DebugCheck(radius <= 128) ;
    DebugCheck(p_walls != NULL) ;

    radius32 = radius<<16 ;

    x1 = x-radius32 ;
    x1 >>= 16 ;
    y1 = y-radius32 ;
    y1 >>= 16 ;
    x2 = x+radius32 ;
    x2 >>= 16 ;
    y2 = y+radius32 ;
    y2 >>= 16 ;

    /* Determine the blocks at the four corners. */
    block1 = IGetBlock((T_sword16)x1, (T_sword16)y1) ;
    block2 = IGetBlock((T_sword16)x1, (T_sword16)y2) ;
    block3 = IGetBlock((T_sword16)x2, (T_sword16)y2) ;
    block4 = IGetBlock((T_sword16)x2, (T_sword16)y1) ;
    /* Check each corner for lines that are hit, but don't */
    /* repeat any blocks that we have already checked. */
    if (block1 != -1)
        IWallTouchInBlock(
            (T_sword16)x1,
            (T_sword16)y1,
            (T_sword16)x2,
            (T_sword16)y2,
            block1,
            maxWalls,
            &numLines,
            p_walls) ;

    if ((block2 != block1) && (block2 != -1))
        IWallTouchInBlock(
            (T_sword16)x1,
            (T_sword16)y1,
            (T_sword16)x2,
            (T_sword16)y2,
            block2,
            maxWalls,
            &numLines,
            p_walls) ;

    if ((block3 != block1) && (block3 != block2) && (block3 != -1))
        IWallTouchInBlock(
            (T_sword16)x1,
            (T_sword16)y1,
            (T_sword16)x2,
            (T_sword16)y2,
            block3,
            maxWalls,
            &numLines,
            p_walls) ;

    if ((block4 != block1) && (block4 != block2) && (block4 != block3) && (block4 != -1))
        IWallTouchInBlock(
            (T_sword16)x1,
            (T_sword16)y1,
            (T_sword16)x2,
            (T_sword16)y2,
            block4,
            maxWalls,
            &numLines,
            p_walls) ;

    DebugEnd() ;

    return numLines ;
}

/*-------------------------------------------------------------------------*
 * Routine:  IWallTouchInBlock
 *-------------------------------------------------------------------------*/
/**
 *  IWallTouchInBlock checks the given block and determines if there
 *  are any walls in that block that touches the given region.  If there
 *  is, that wall is added to the wall list (unless already there).
 *
 *  @param y1 -- Upper left corner of box
 *  @param y2 -- Lower right corner of box
 *  @param block -- Block in to check walls in
 *  @param maxWalls -- Max walls allowed to find
 *  @param p_numWalls -- Walls already found (increment as find)
 *  @param p_walls -- Where to put the walls
 *
 *  @return TRUE if ran over max walls.
 *
 *<!-----------------------------------------------------------------------*/
static E_Boolean IWallTouchInBlock(
                     T_sword16 x1,
                     T_sword16 y1,
                     T_sword16 x2,
                     T_sword16 y2,
                     T_sword32 blockIndex,
                     T_word16 maxWalls,
                     T_word16 *p_numWalls,
                     T_word16 *p_walls)
{
    T_word16 i, j ;
    E_Boolean overflowOccured = FALSE ;
    E_Boolean foundWall ;

    DebugRoutine("IWallTouchInBlock") ;
    DebugCheck(p_walls != NULL) ;
    DebugCheck(p_numWalls != NULL) ;

    /* Go through the list of walls for this block until we */
    /* reach the 0xFFFF end marker. */
    while ((i=G_3dBlockMapArray[blockIndex]) != 0xFFFF)  {
        DebugCheck(i < G_Num3dLines) ;
        if (i < G_Num3dLines)  {
            /* For each wall, check to see if we hit the box. */
            if (Collide3dCheckSegmentHitBox(
                    i,
                    x1,
                    y1,
                    x2,
                    y2) != 0)  {
                /* Go through the list of already colliding walls */
                /* and make sure this line is not already in the list. */
                foundWall = FALSE ;
                for (j=0; j<*p_numWalls; j++)  {
                    if (i==p_walls[j])  {
                        foundWall = TRUE ;
                        break ;
                    }
                }

                /* If we never found the wall, we need to add it to the */
                /* list.  If we did find the wall, just ignore it and */
                /* continue. */
                if (!foundWall)  {
                    /* Make sure we don't go over the limit. */
                    if (*p_numWalls == maxWalls)  {
                        DebugCheck(FALSE) ;
                        overflowOccured = TRUE ;
                        break ;
                    }

                    /* Limit is not a problem.  Just add. */
                    p_walls[(*p_numWalls)++] = i ;
                }
            }
        }
        blockIndex++ ;
    }

    DebugEnd() ;

    return overflowOccured ;
}

/*-------------------------------------------------------------------------*
 * Routine:  Collide3dGetSectorsInBox
 *-------------------------------------------------------------------------*/
/**
 *  Collide3dGetSectorsInBox determines all the sectors that are in a
 *  square radius box.  In addition, it returns a boolean telling if it
 *  found the box touching a void area.
 *
 *  @param y -- Center of box
 *  @param radius -- Radius of box
 *  @param maxSectors -- Max sectors allowed to be found
 *  @param p_sectors -- Where to put the sectors
 *  @param p_numSectors -- Pointer to store num sectors found
 *
 *  @return Found void area
 *
 *<!-----------------------------------------------------------------------*/
#define COLLIDE_3D_MAX_LINES_FOR_SECTORS 100
E_Boolean Collide3dGetSectorsInBox(
              T_sword32 x,
              T_sword32 y,
              T_word16 radius,
              T_word16 maxSectors,
              T_word16 *p_sectors,
              T_word16 *p_numSectors)
{
    T_word16 numLines ;
    T_word16 lines[COLLIDE_3D_MAX_LINES_FOR_SECTORS] ;
    E_Boolean foundVoid=FALSE ;
    T_word16 sector ;
    T_word16 i, j ;
    T_word16 side ;
    T_word16 line ;
    T_word16 sideNum ;
    E_Boolean foundSector ;

    DebugRoutine("Collide3dGetSectorsInBox") ;
    DebugCheck(radius <= 128) ;
    DebugCheck(p_sectors != NULL) ;
    DebugCheck(p_numSectors != NULL) ;

    /* Start by finding the sector that we are over. */
    sector = View3dFindSectorNum(
                 (T_sword16)(x>>16),
                 (T_sword16)(y>>16)) ;

    /* If this is void, make a note of it. */
    if (sector == 0xFFFF)  {
        foundVoid = TRUE ;
        *p_numSectors = 0 ;
    } else {
        /* Otherwise, record the sector. */
        *p_numSectors = 1 ;
        DebugCheck(sector < G_Num3dSectors) ;
        p_sectors[0] = sector ;
    }

    /* Found all the lines that go through the box. */
    numLines = Collide3dGetWallsInBox(
                   x,
                   y,
                   radius,
                   COLLIDE_3D_MAX_LINES_FOR_SECTORS,
                   lines) ;
    /* Go through this list of lines and find all the sectors. */
    for (i=0; i<numLines; i++)  {
        line = lines[i] ;
        DebugCheck(line < G_Num3dLines) ;
        if (line < G_Num3dLines)  {
            /* Check to see if this line is impassable. */
            if (G_3dLineArray[line].flags & G_wallDefinition)  {
                /* Hit an impassable wall, pretend it is */
                /* void on one side. */
                foundVoid = TRUE ;
            }
            /* Do both sides of a line. */
            for (side=0; side<2; side++)  {
                /* Check to see if the side exists. */
                if ((sideNum = G_3dLineArray[line].side[side]) != 0xFFFF)  {
                    /* Side exists.  Add the sector to the list. */
                    DebugCheck(sideNum < G_Num3dSides) ;
                    if (sideNum < G_Num3dSides)  {
                        sector = G_3dSideArray[sideNum].sector ;
                        /* Is this a void area? */
                        if (sector == 0xFFFF)  {
                            /* Yes, the side's sector is void. */
                            /* Just note that we found a void */
                            /* and continue. */
                            foundVoid = TRUE ;
                        } else {
                            /* Nope, no void here. */
                            DebugCheck(sector < G_Num3dSectors) ;
                            if (sector < G_Num3dSectors)  {
                                /* Try adding this to the list, */
                                /* but make sure we */
                                /* don't duplicate it. */
                                foundSector = FALSE ;
                                for (j=0; j<*p_numSectors; j++)  {
                                    if (p_sectors[j] == sector)  {
                                        foundSector = TRUE ;
                                        break ;
                                    }
                                }

                                /* Did we find the sector */
                                /* in the list? */
                                if (!foundSector)  {
                                    /* No.  Then add it. */
                                    /* Are we over the limit? */
                                    if (*p_numSectors < maxSectors)  {
                                        /* Not over the limit. */
                                        p_sectors[(*p_numSectors)++] = sector ;
                                    } else {
                                        /* Over the limit.  */
                                        /* If debug version, crash. */
                                        DebugCheck(FALSE) ;
                                        /* If not debug, stop processing */
                                        /* by skipping to the end. */
                                        i = numLines ;
                                    }
                                }
                            }
                        }
                    }
                } else {
                    /* Side does not exist, thus we touch a void. */
                    foundVoid = TRUE ;
                }
            }
        }
    }

    DebugEnd() ;

    return foundVoid ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ICanSqueezeThrough
 *-------------------------------------------------------------------------*/
/**
 *  ICanSqueezeThrough checks to see if the given parameters will fit
 *  in the given list of sectors (typically returned via
 *  Collide3dGetSectorsInBox).
 *
 *  @param p_obj -- Object being moved
 *  @param zPos -- Z height of check
 *  @param climbHeight -- Height to attempt to stepping up to
 *  @param height -- Vertical height required
 *  @param numSectors -- Number of sectors in sector list
 *  @param p_sectorList -- List of sectors to check
 *
 *  @return TRUE=can fit, else FALSE
 *
 *<!-----------------------------------------------------------------------*/
static E_Boolean ICanSqueezeThrough(
        T_3dObject *p_obj,
		T_sword16 zPos,
		T_sword16 height,
		T_word16 numSectors,
		T_word16 *p_sectorList)
{
    E_Boolean canSqueeze = TRUE ;
    T_word16 i ;
    T_word16 sector ;
    T_sword16 floor, ceiling ;
    T_sword16 floor2, ceiling2 ;
    T_sword16 zTop ;

    DebugRoutine("ICanSqueezeThrough") ;
    DebugCheck(p_sectorList != NULL) ;

    /* Precalculate the top of the head. */
    zTop = zPos + height ;

#if 0
    /* Go through the list of sectors and determine if we can fit. */
    for (i=0; i<numSectors; i++)  {
        sector = p_sectorList[i] ;
        DebugCheck(sector < G_Num3dSectors) ;
//        floor = G_3dSectorArray[sector].floorHt ;
//        if ((G_allowDip) &&
//            (G_3dSectorArray[sector].trigger & SECTOR_DIP_FLAG))
//            floor -= VIEW_WATER_DIP_LEVEL ;
        floor = MapGetWalkingFloorHeight(sector) ;
        ceiling = G_3dSectorArray[sector].ceilingHt ;

        /* See if the ceiling is too low. */
        if (ceiling < zTop)  {
//printf("CanSqueeze: ceiling < zTop\n") ;
            canSqueeze = FALSE ;
            break ;
        }

        /* See if the floor is too high. */
        if (floor > zPos)  {
//printf("CanSqueeze: floor < zPos\n") ;
            canSqueeze = FALSE ;
            break ;
        }
    }
#endif
    floor = -32767 ;
    ceiling = 32767 ;
    for (i=0; i<numSectors; i++)  {
        sector = p_sectorList[i] ;
        floor2 = MapGetWalkingFloorHeight(&p_obj->objMove, sector) ;
        if (floor2 > floor)
            floor = floor2 ;
        ceiling2 = G_3dSectorArray[sector].ceilingHt ;
//printf("ceiling for %d is %d\n", sector, ceiling2) ;
        if (ceiling2 < ceiling)
            ceiling = ceiling2 ;
    }

//printf("CanSqueeze: floor=%d, ceiling=%d, zPos=%d, zTop=%d\n", floor, ceiling, zPos, zTop) ;
    /* See if the ceiling is too low. */
    if (ceiling < zTop)  {
        canSqueeze = FALSE ;
    } else {
        /* See if the floor is too high. */
        if (floor > zPos)  {
            canSqueeze = FALSE ;
        }
    }

    DebugEnd() ;

//printf("CanSqueeze = %d\n", canSqueeze) ;
    return canSqueeze ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ICanSqueezeThroughWithClimb
 *-------------------------------------------------------------------------*/
/**
 *  ICanSqueezeThrough checks to see if the given parameters will fit
 *  in the given list of sectors (typically returned via
 *  Collide3dGetSectorsInBox).
 *
 *  @param p_obj -- ObjMove being moved
 *  @param zPos -- Z height of check
 *  @param climbHeight -- Height to attempt to stepping up to
 *  @param height -- Vertical height required
 *  @param numSectors -- Number of sectors in sector list
 *  @param p_sectorList -- List of sectors to check
 *
 *  @return TRUE=can fit, else FALSE
 *
 *<!-----------------------------------------------------------------------*/
static E_Boolean ICanSqueezeThroughWithClimb(
        T_3dObject *p_obj,
		T_sword16 *zPos,
		T_sword16 climbHeight,
		T_sword16 height,
		T_word16 numSectors,
		T_word16 *p_sectorList)
{
    E_Boolean canSqueeze = TRUE ;
    T_word16 i ;
    T_word16 sector ;
    T_sword16 floor, ceiling ;
    T_sword16 floor2, ceiling2 ;

    DebugRoutine("ICanSqueezeThroughWithClimb") ;

#if 0
    /* Find how high we have to step in order to be here. */
    biggestStep = 0 ;
    for (i=0; i<numSectors; i++)  {
        sector = p_sectorList[i] ;
        DebugCheck(sector < G_Num3dSectors) ;
//        floor = G_3dSectorArray[sector].floorHt ;
//        if ((G_allowDip) &&
//            (G_3dSectorArray[sector].trigger & SECTOR_DIP_FLAG))
//            floor -= VIEW_WATER_DIP_LEVEL ;
        floor = MapGetWalkingFloorHeight(sector) ;
        step = floor - *zPos ;
        if (step > biggestStep)
            biggestStep = step ;
    }

    /* Can we make the step? */
    if (biggestStep <= climbHeight)  {
        /* Yes.  Step up to this new height. */
        *zPos += biggestStep ;

        /* Precalculate the top of the head. */
        zTop = *zPos + height ;

        /* Go through the list of sectors and determine if we can fit */
        /* under the ceilings with the new height we have. */
        for (i=0; i<numSectors; i++)  {
            sector = p_sectorList[i] ;
            DebugCheck(sector < G_Num3dSectors) ;
            ceiling = G_3dSectorArray[sector].ceilingHt ;

            /* See if the ceiling is too low. */
            if (ceiling < zTop)  {
//printf("CanSqueezeWithClimb: ceiling < zTop\n") ;
                canSqueeze = FALSE ;
                break ;
            }
        }
    } else {
//printf("CanSqueezeWithClimb: biggestStep > climbHeight\n") ;
        canSqueeze = FALSE ;
    }
#endif
    /* Find the lowest ceiling and highest floor. */
    floor = -32767 ;
    ceiling = 32767 ;
    for (i=0; i<numSectors; i++)  {
        sector = p_sectorList[i] ;
        floor2 = MapGetWalkingFloorHeight(&p_obj->objMove, sector) ;
        if (floor2 > floor)
            floor = floor2 ;
        ceiling2 = G_3dSectorArray[sector].ceilingHt ;
        if (ceiling2 < ceiling)
            ceiling = ceiling2 ;
    }

//printf("CanSqueezeClimb: floor=%d, ceiling=%d, zPos=%d, height=%d, climb=%d\n", floor, ceiling, *zPos, height, climbHeight) ;
    /* See if there is general room to squeeze. */
    if ((floor + height >= ceiling) || (*zPos + height >= ceiling))  {
        canSqueeze = FALSE ;
    } else {
        /* See if the step is too high. */
        if (*zPos + climbHeight >= floor)  {
            /* Step up if can. */
            if (*zPos < floor)
                *zPos = floor ;
        } else {
            /* Cannot step up, must block. */
            canSqueeze = FALSE ;
        }
    }

    DebugEnd() ;

//printf("CanSqueezeClimb = %d\n", canSqueeze) ;
    return canSqueeze ;
}

/*-------------------------------------------------------------------------*
 * Routine:  Collide3dMoveToXYZ
 *-------------------------------------------------------------------------*/
/**
 *  Collide3dMoveToXYZ takes an object and moves to the given XYZ.
 *
 *  @param p_obj -- Object to move
 *  @param newX -- X Target end-point
 *  @param newY -- Y Target end-point
 *  @param newZ -- Z Target end-point
 *
 *  @return TRUE=collision
 *
 *<!-----------------------------------------------------------------------*/
E_Boolean Collide3dMoveToXYZ(
              T_3dObject *p_obj,
              T_sword32 newX,
              T_sword32 newY,
              T_sword32 newZ)
{
    T_sword32 oldX, oldY, oldZ ;
    T_sword32 halfX, halfY, halfZ ;
    T_word16 distance ;
    E_Boolean collided ;
    T_sword16 newZ16 ;
    T_sword16 radius ;
    static T_word16 depth = 0 ;

    DebugRoutine("Collide3dMoveToXYZ") ;

    /* Make sure this routine doesn't run too deep. */
    if (depth == 8)  {
        DebugEnd() ;

        return TRUE ;
    }

    depth++ ;

//if (ObjectIsCreature(p_obj))
// printf("CMTXYZ: aim to %08lX %08lX %08lX\n", newX, newY, newZ) ;

    /* First, note that we have not collided with anything. */
    collided = FALSE ;

    /* Zero out the count of hits. */
    G_numHits = 0 ;

    /* Where are we moving from? */
    oldX = ObjectGetX(p_obj) ;
    oldY = ObjectGetY(p_obj) ;
    oldZ = ObjectGetZ(p_obj) ;

    /* How far do we need to go? */
    distance = CalculateDistance(
                   oldX >> 16,
                   oldY >> 16,
                   newX >> 16,
                   newY >> 16) ;

    /* Make sure we don't jump around forever. */
    if (distance > 200)
        distance = 200 ;

    radius = ObjectGetRadius(p_obj) ;
    if (radius < 5)
        radius = 5 ;

    /* While the distance is too great, split it up. */
    while ((distance > radius) && (!collided))  {
        /* Where is half way? */
        halfX = (oldX + newX) >> 1 ;
        halfY = (oldY + newY) >> 1 ;
        halfZ = (oldZ + newZ) >> 1 ;
        /* Try moving to that position. */
        collided = Collide3dMoveToXYZ(
                       p_obj,
                       halfX,
                       halfY,
                       halfZ) ;

        if (!collided)  {
            /* Where are we moving from? */
            oldX = ObjectGetX(p_obj) ;
            oldY = ObjectGetY(p_obj) ;
            oldZ = ObjectGetZ(p_obj) ;

            /* How far do we need to go? */
            distance = CalculateDistance(
                           oldX >> 16,
                           oldY >> 16,
                           newX >> 16,
                           newY >> 16) ;
        } else {
            break ;
        }
    }

    /* Make sure we have not collided yet. */
    if (!collided)  {
        /* OK, test for a collision at the new location we */
        /* are moving to. */

        /* What are the list of sectors of where we are trying */
        /* to go. */
        collided = Collide3dGetSectorsInBox(
                       newX,
                       newY,
                       ObjectGetRadius(p_obj),
                       20,
                       G_surroundingSectors,
                       &G_numSurroundingSectors) ;
//if (collided)
// if (ObjectIsCreature(p_obj))
//   printf("Collide3dMoveToXYZ by sectors in box at %08lX %08lX for %d\n", newX, newY, ObjectGetServerId(p_obj)) ;
        /* Did we collide here? */
        if (!collided)  {
            /* Nope.  Still good. */

            /* Can we fit in the new area? */
//puts("\n\ncheck") ;  fflush(stdout) ;
            if (!ICanSqueezeThrough(
                    p_obj,
                    (T_sword16)(newZ >> 16),
                    ObjectGetHeight(p_obj),
                    G_numSurroundingSectors,
                    G_surroundingSectors))  {
//printf("Tight one\n") ;  fflush(stdout) ;
#if 0
                /* Can't squeeze into that new location! */
                collided = TRUE ;
#else
                newZ16 = newZ >> 16 ;
                if (!ICanSqueezeThroughWithClimb(
                    p_obj,
                    &newZ16,
                    (T_word16)(8+ObjectGetClimbHeight(p_obj)),
                    (T_word16)(ObjectGetHeight(p_obj)),
                    (T_word16)G_numSurroundingSectors,
                    G_surroundingSectors))  {

//puts("Tighter") ; fflush(stdout) ;
                    /* Can't squeeze into that new location! */
                    collided = TRUE ;
/*
if (ObjectIsCreature(p_obj)) {
 printf("Squeeze collide for %d\n", ObjectGetServerId(p_obj)) ;
 printf("  squeeze parms: %04X %d %d %d\n", newZ16, 8+ObjectGetClimbHeight(p_obj), ObjectGetHeight(p_obj), G_numSurroundingSectors) ;
}
*/
                } else {
                    newZ = newZ16 << 16;
//puts("ok") ; fflush(stdout) ;
                }
#endif
            }
            if (!collided)  {
                /* How about objects? */
                G_numHits = 0 ;
                collided = View3dObjectHitFast(
                               (T_sword16)(newX>>16),
                               (T_sword16)(newY>>16),
                               ObjectGetRadius(p_obj),
                               (T_sword16)(ObjectGetX16(p_obj)),
                               (T_sword16)(ObjectGetY16(p_obj)),
                               newZ>>16,
                               (newZ>>16) + ObjectGetHeight(p_obj),
//                               (T_sword16)(ObjectGetZ16(p_obj)),
//                               (T_sword16)(ObjectGetZ16(p_obj) + ObjectGetHeight(p_obj)),
                               ObjectGetHeight(p_obj),
                               p_obj) ;

                /* Did we collide? */
                if (!collided)  {
                    /* Nope.  Then move to that location. */
                    ObjectSetX(p_obj, newX) ;
                    ObjectSetY(p_obj, newY) ;
                    ObjectSetZ(p_obj, newZ) ;
                } else {
//if (ObjectIsCreature(p_obj))
//  printf("Collide3dMoveToXYZ at %08lX %08lX for %d\n", newX, newY, ObjectGetServerId(p_obj)) ;
                }
            }
        }
    }

    depth-- ;

    DebugEnd() ;

    return collided ;
}

/* LES 04/12/96 */
T_sword16 Collide3dCheckSegmentHitsSegment(
              T_sword16 x1,
              T_sword16 y1,
              T_sword16 x2,
              T_sword16 y2,
              T_word16 lineNum)
{
    T_sword16 lx1, lx2, ly1, ly2 ;
    T_sword16 left, right, top, bottom ;
    T_byte8 a, b ;

    if (x1 < x2)  {
        left = x1 ;
        right = x2 ;
    } else  {
        left = x2 ;
        right = x1 ;
    }

    if (y1 < y2)  {
        top = y1 ;
        bottom = y2 ;
    } else {
        top = y2 ;
        bottom = y1 ;
    }

    lx1 = G_3dVertexArray[G_3dLineArray[lineNum].from].x ;
    ly1 = G_3dVertexArray[G_3dLineArray[lineNum].from].y ;
    lx2 = G_3dVertexArray[G_3dLineArray[lineNum].to].x ;
    ly2 = G_3dVertexArray[G_3dLineArray[lineNum].to].y ;

    if (((lx1 < left) && (lx2 < left)) ||
        ((lx1 > right) && (lx2 > right)) ||
        ((ly1 < top) && (ly2 < top)) ||
        ((ly1 > bottom) && (ly2 > bottom)))  {
        return -1 ;
    }

    /* Now we have to do the real check by finding an intersection. */
    a = Collide3dPointOnRight(
            lx1,
            ly1,
            lx2,
            ly2,
            x1,
            y1) ;
    b = Collide3dPointOnRight(
            lx1,
            ly1,
            lx2,
            ly2,
            x2,
            y2) ;
    if (a == b)
        return -1 ;

    a = Collide3dPointOnRight(
            x1,
            y1,
            x2,
            y2,
            lx1,
            ly1) ;
    b = Collide3dPointOnRight(
            x1,
            y1,
            x2,
            y2,
            lx2,
            ly2) ;
    if (a == b)
        return -1 ;

    /* Must be intersecting. */
    return 1 ;
}

/* LES 04/12/96 */
T_byte8 Collide3dPointOnRight(
            T_sword32 lineX1,
            T_sword32 lineY1,
            T_sword32 lineX2,
            T_sword32 lineY2,
            T_sword32 pointX,
            T_sword32 pointY)
{
    if (((lineX1 - lineX2) * (pointY - lineY2)) >=
        ((lineY1 - lineY2) * (pointX - lineX2)))
        return 1 ;
    return 0 ;
}

/* LES 04/12/96 */
E_Boolean Collide3dCheckLineOfSight(
              T_sword16 sightStartX,
              T_sword16 sightStartY,
              T_sword16 sightEndX,
              T_sword16 sightEndY)
{
    T_word16 i ;
    T_sword16 ceiling1, ceiling2 ;
    T_sword16 floor1, floor2 ;

    /* Determine if there is something in the way. */
    for (i=0; i<G_Num3dLines; i++)  {
        /* Is the line passable or impassible? */
        if (G_3dLineArray[i].flags & LINE_IS_IMPASSIBLE)  {
            /* impassible. */
            if (Collide3dCheckSegmentHitsSegment(
                    sightStartX,
                    sightStartY,
                    sightEndX,
                    sightEndY,
                    i) != -1)
                return TRUE ;
        } else {
            /* passible. */
            /* Find the lower ceiling in the two sides. */
            ceiling1 = G_3dSectorArray[G_3dSideArray[G_3dLineArray[i].side[0]].sector].ceilingHt ;
            ceiling2 = G_3dSectorArray[G_3dSideArray[G_3dLineArray[i].side[1]].sector].ceilingHt ;
            if (ceiling2 < ceiling1)
                ceiling1 = ceiling2 ;

            /* Find the highest floor on the two sides. */
            floor1 = G_3dSectorArray[G_3dSideArray[G_3dLineArray[i].side[0]].sector].floorHt ;
            floor2 = G_3dSectorArray[G_3dSideArray[G_3dLineArray[i].side[1]].sector].floorHt ;
            if (floor2 > floor1)
                floor1 = floor2 ;

            /* If the floor touches the ceiling or shears, */
            /* consider this wall impassible. */
            if (floor1 >= ceiling1)  {
                /* Is either floor or ceiling blocking the view. */
                if (Collide3dCheckSegmentHitsSegment(
                        sightStartX,
                        sightStartY,
                        sightEndX,
                        sightEndY,
                        i) != -1)
                    return TRUE ;
            }
        }
    }

    return FALSE ;
}

/* LES 04/23/96 -- Get a list of walls that intersect the given */
/* line.  Returns the number of walls found. */
T_word16 Collide3dFindWallList(
             T_sword16 x1,
             T_sword16 y1,
             T_sword16 x2,
             T_sword16 y2,
             T_sword16 z,
             T_word16 maxWalls,
             T_wallListItem *p_list,
             T_byte8 wallTypes)
{
    T_word16 i, j ;
    T_sword16 floor, floor2, ceiling, ceiling2 ;
    T_word16 side ;
    T_word16 count = 0 ;
    T_word16 sector ;

    DebugRoutine("Collide3dFindWallList") ;

    /* Go through the list of lines. */
    for (i=0; (i<G_Num3dLines) && (count < maxWalls); i++)  {
        /* First, determine if there is a basic collision */
        /* of the given segment with the given line. */
        if (Collide3dCheckSegmentHitsSegment(x1, y1, x2, y2, i) != -1)   {
            /* There is a collision going on here. */
            /* But we need to check based on height what type */
            /* and if the collision is really occuring. */

            floor = -0x7FFE ;
            ceiling = 0x7FFE ;
            for (j=0; j<2; j++)  {
                side = G_3dLineArray[i].side[j] ;
                if (side != 0xFFFF)  {
                    sector = G_3dSideArray[side].sector ;
                    if (sector != 0xFFFF)  {
                        floor2 = G_3dSectorArray[sector].floorHt ;
                        ceiling2 = G_3dSectorArray[sector].ceilingHt ;
                        if (floor2 > floor)
                            floor = floor2 ;
                        if (ceiling2 < ceiling)
                            ceiling = ceiling2 ;
                    }
                }
            }

            /* Is this supposed to be an upper, lower, or middle? */
            if (z < floor)  {
                /* Must be a lower section. */
                /* Don't do lower sections unless asked. */
                if (wallTypes & WALL_LIST_ITEM_LOWER)  {
                    p_list[count].lineNumber = i ;
                    p_list[count].itemType = WALL_LIST_ITEM_LOWER ;
                    count++ ;
                }
            } else if (z > ceiling) {
                /* Must be an upper section. */
                /* Don't do upper sections unless asked. */
                if (wallTypes & WALL_LIST_ITEM_UPPER)  {
                    p_list[count].lineNumber = i ;
                    p_list[count].itemType = WALL_LIST_ITEM_UPPER ;
                    count++ ;
                }
            } else {
                /* Must be a main section. */
                /* Only collides if this wall is impassible */
                if (wallTypes & WALL_LIST_ITEM_MAIN)  {
                    if (G_3dLineArray[i].flags & G_wallDefinition)  {
                        p_list[count].lineNumber = i ;
                        p_list[count].itemType = WALL_LIST_ITEM_MAIN ;
                        count++ ;
                    }
                }
            }
        }
    }

    DebugEnd() ;

    return count ;
}

/*-------------------------------------------------------------------------*
 * Routine:  Collide3dSetWallDefinition is used to tell what flags
 *-------------------------------------------------------------------------*/
/**
 *  in a line are actually used for determine a collision.
 *  Currently, only 2 types of definitions are used.
 *  A) LINE_IS_IMPASSIBLE    <== used for the player and
 *  normal objects (including missiles)
 *  B) LINE_IS_IMPASSIBLE | LINE_IS_CREATURE_IMPASSIBLE  <== used
 *  for normal creatures
 *
 *<!-----------------------------------------------------------------------*/
T_void Collide3dSetWallDefinition(T_word16 lineFlags)
{
    DebugRoutine("Collide3dSetWallDefinition") ;

    DebugCheck(
       (lineFlags == LINE_IS_IMPASSIBLE) ||
       (lineFlags == (LINE_IS_IMPASSIBLE | LINE_IS_CREATURE_IMPASSIBLE))) ;

    G_wallDefinition = lineFlags ;

    DebugEnd() ;
}
// Templates:
//Collide3dSetWallDefinition(LINE_IS_IMPASSIBLE) ;
//Collide3dSetWallDefinition(
//    LINE_IS_IMPASSIBLE |
//    LINE_IS_CREATURE_IMPASSIBLE) ;
//

E_Boolean Collide3dObjectToObjectCheckLineOfSight(
              T_3dObject *p_from,
              T_3dObject *p_to)
{
    T_word32 index ;
    E_Boolean isBlocked ;
    static int powers[8] = { 1, 2, 4, 8, 0x10, 0x20, 0x40, 0x80 } ;

    DebugRoutine("Collide3dObjectToObjectCheckLineOfSight") ;

    index = ObjectGetCenterSector(p_from) * G_Num3dSectors +
            ObjectGetCenterSector(p_to) ;
    if (G_3dReject[index>>3] & powers[index&7])
        isBlocked = TRUE ;
    else
        isBlocked = Collide3dCheckLineOfSight(
                        ObjectGetX16(p_from),
                        ObjectGetY16(p_from),
                        ObjectGetX16(p_to),
                        ObjectGetY16(p_to)) ;

    DebugEnd() ;

    return isBlocked ;
}

T_void Collide3dUpdateLineOfSightLast(
           T_lineOfSightLast *p_lastSight,
           T_3dObject *p_target)
{
    if (p_target != NULL)  {
        p_lastSight->x = ObjectGetX16(p_target) ;
        p_lastSight->y = ObjectGetY16(p_target) ;
        p_lastSight->sector = ObjectGetCenterSector(p_target) ;
    } else {
        p_lastSight->x =
        p_lastSight->y = 0x7FFF ;
        p_lastSight->sector = 0 ;
    }
}

E_Boolean Collide3dObjectToXYCheckLineOfSight(
              T_3dObject *p_from,
              T_lineOfSightLast *p_lastSight,
              T_sword16 x,
              T_sword16 y)
{
    T_word32 index ;
    E_Boolean isBlocked ;
    static int powers[8] = { 1, 2, 4, 8, 0x10, 0x20, 0x40, 0x80 } ;

    DebugRoutine("Collide3dObjectToObjectCheckLineOfSight") ;

    if ((p_lastSight->x != x) || (p_lastSight->y != y))  {
        p_lastSight->x = x ;
        p_lastSight->y = y ;
        p_lastSight->sector = View3dFindSectorNum(x, y) ;
    }

    index = ObjectGetCenterSector(p_from) * G_Num3dSectors +
            p_lastSight->sector ;
    if (G_3dReject[index>>3] & powers[index&7])
        isBlocked = TRUE ;
    else
        isBlocked = Collide3dCheckLineOfSight(
                        ObjectGetX16(p_from),
                        ObjectGetY16(p_from),
                        x,
                        y) ;

    DebugEnd() ;

    return isBlocked ;
}

E_Boolean Collide3dCheckLineOfSightWithZ(
              T_sword16 sightStartX,
              T_sword16 sightStartY,
              T_sword16 sightEndX,
              T_sword16 sightEndY,
              T_sword16 sightZ)
{
    T_word16 i ;
    T_sword16 ceiling1, ceiling2 ;
    T_sword16 floor1, floor2 ;

    /* Determine if there is something in the way. */
    for (i=0; i<G_Num3dLines; i++)  {
        /* Is the line passable or impassible? */
        if (G_3dLineArray[i].flags & LINE_IS_IMPASSIBLE)  {
            /* impassible. */
            if (Collide3dCheckSegmentHitsSegment(
                    sightStartX,
                    sightStartY,
                    sightEndX,
                    sightEndY,
                    i) != -1)
                return TRUE ;
        } else {
            /* passible. */
            /* Find the lower ceiling in the two sides. */
            ceiling1 = G_3dSectorArray[G_3dSideArray[G_3dLineArray[i].side[0]].sector].ceilingHt ;
            ceiling2 = G_3dSectorArray[G_3dSideArray[G_3dLineArray[i].side[1]].sector].ceilingHt ;
            if (ceiling2 < ceiling1)
                ceiling1 = ceiling2 ;

            /* Find the highest floor on the two sides. */
            floor1 = G_3dSectorArray[G_3dSideArray[G_3dLineArray[i].side[0]].sector].floorHt ;
            floor2 = G_3dSectorArray[G_3dSideArray[G_3dLineArray[i].side[1]].sector].floorHt ;
            if (floor2 > floor1)
                floor1 = floor2 ;

            /* If the floor touches the ceiling or shears, */
            /* consider this wall impassible. */
            if ((floor1 >= ceiling1) || (sightZ >= ceiling1) || (sightZ <= floor1))  {
                /* Is either floor or ceiling blocking the view. */
                if (Collide3dCheckSegmentHitsSegment(
                        sightStartX,
                        sightStartY,
                        sightEndX,
                        sightEndY,
                        i) != -1)
                    return TRUE ;
            }
        }
    }

    return FALSE ;
}

E_Boolean Collide3dObjectToXYCheckLineOfSightWithZ(
              T_3dObject *p_from,
              T_sword16 x,
              T_sword16 y,
              T_sword16 z)
{
    T_word32 index ;
    E_Boolean isBlocked ;
    static int powers[8] = { 1, 2, 4, 8, 0x10, 0x20, 0x40, 0x80 } ;
    T_word16 sector ;

    DebugRoutine("Collide3dObjectToObjectCheckLineOfSight") ;

    sector = View3dFindSectorNum(x, y) ;

    index = ObjectGetCenterSector(p_from) * G_Num3dSectors + sector ;
    if (G_3dReject[index>>3] & powers[index&7])
        isBlocked = TRUE ;
    else
        isBlocked = Collide3dCheckLineOfSightWithZ(
                        ObjectGetX16(p_from),
                        ObjectGetY16(p_from),
                        x,
                        y,
                        z) ;

    DebugEnd() ;

    return isBlocked ;
}

/** @} */
/*-------------------------------------------------------------------------*
 * End of File:  3D_COLLI.C
 *-------------------------------------------------------------------------*/
