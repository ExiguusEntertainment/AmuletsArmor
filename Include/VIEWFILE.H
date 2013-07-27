/****************************************************************************/
/*    FILE:  VIEWFILE.H                                                     */
/****************************************************************************/

#ifndef _VIEWFILE_H_
#define _VIEWFILE_H_

#include "DBLLINK.H"
#include "OBJMOVE.H"
#include "OBJTYPE.H"
#include "RESOURCE.H"
#include "SCRIPT.H"

#define MAX_OBJECT_SECTORS 20

typedef enum {
    BODY_PART_LOCATION_HEAD,
    BODY_PART_LOCATION_CHEST,
    BODY_PART_LOCATION_LEFT_ARM,
    BODY_PART_LOCATION_RIGHT_ARM,
    BODY_PART_LOCATION_LEGS,
    BODY_PART_LOCATION_WEAPON,
    BODY_PART_LOCATION_SHIELD,
    BODY_PART_LOCATION_UNKNOWN
} T_bodyPartLocation ;

#define MAX_BODY_PARTS     BODY_PART_LOCATION_UNKNOWN

typedef struct  {
    T_byte8 signature[4]     PACK;
    T_word32 numEntries      PACK;
    T_word32 foffset         PACK;
} T_wadHeader ;

typedef struct  {
    T_word32 foffset         PACK;
    T_word32 size            PACK;
    T_byte8 name[8]          PACK;
} T_directoryEntry ;

typedef struct  {
    T_sword16 x              PACK;
    T_sword16 y              PACK;
} T_3dVertex ;

typedef struct  {
    T_sword16 from           PACK;
    T_sword16 to             PACK;
    T_sword16 flags          PACK;
    T_sword16 special        PACK;
    T_sword16 tag            PACK;
    T_sword16 side[2]        PACK;
} T_3dLine ;

typedef struct  {
    T_sword16 tmXoffset      PACK;
    T_sword16 tmYoffset      PACK;
    T_byte8   upperTx[8]     PACK;
    T_byte8   lowerTx[8]     PACK;
    T_byte8   mainTx[8]      PACK;
    T_word16  sector         PACK;
} T_3dSide ;

typedef struct  {
    T_sword16   floorHt      PACK;
    T_sword16   ceilingHt    PACK;
    T_byte8     floorTx[8]   PACK;
    T_byte8     ceilingTx[8] PACK;
    T_sword16   light        PACK;
    T_sword16   type         PACK;
    T_sword16   trigger      PACK;
} T_3dSector ;

typedef T_byte8 T_sectorType ;
#define SECTOR_TYPE_DIRT         1
#define SECTOR_TYPE_ICE          2
#define SECTOR_TYPE_LAVA         4
#define SECTOR_TYPE_WATER        8
#define SECTOR_TYPE_MUD          16
#define SECTOR_TYPE_ACID         32
#define SECTOR_TYPE_UNKNOWN      64

#define NUM_SECTOR_TYPES         6

typedef struct {
    T_sbyte8 xVelAdd ;
    T_sbyte8 yVelAdd ;
    T_sbyte8 zVelAdd ;
    T_word16 gravity ;
    T_byte8 friction ;
    T_sword16 damage ;
    T_byte8 damageType ;
    T_byte8 depth ;
    T_byte8 textureXOffset ;
    T_byte8 textureYOffset ;
    T_word16 enterSound ;
    T_word16 enterSoundRadius ;
    T_byte8 lightAnimationType;
    T_byte8 lightAnimationRate;
    T_byte8 lightAnimationCenter;
    T_byte8 lightAnimationRadius;
    T_word16 lightAnimationState;
    T_sword16 ceilingLimit ;
    T_word16 nextMap ;
    T_word16 nextMapStart ;
    T_sectorType type ;
} T_3dSectorInfo ;

/** Defines for light animation types. **/
#define SECTOR_LIGHT_ANIM_NONE      0
#define SECTOR_LIGHT_ANIM_OSCILLATE 1
#define SECTOR_LIGHT_ANIM_SAWTOOTH  2
#define SECTOR_LIGHT_ANIM_RANDOM    3
#define SECTOR_LIGHT_ANIM_MIMIC     4


typedef struct  {
    T_sword16 from           PACK;
    T_sword16 to             PACK;
    T_word16 angle           PACK;
    T_sword16 line           PACK;
    T_sword16 lineSide       PACK;
    T_sword16 lineOffset     PACK;
} T_3dSegment ;

typedef struct  {
    T_sword16 numSegs        PACK;
    T_sword16 firstSeg       PACK;
} T_3dSegmentSector ;

typedef struct  {
    T_sword16 x              PACK;
    T_sword16 y              PACK;
    T_sword16 dx             PACK;
    T_sword16 dy             PACK;
    T_sword16 ry2            PACK;
    T_sword16 ry1            PACK;
    T_sword16 rx1            PACK;
    T_sword16 rx2            PACK;
    T_sword16 ly2            PACK;
    T_sword16 ly1            PACK;
    T_sword16 lx1            PACK;
    T_sword16 lx2            PACK;
    T_word16 right           PACK;
    T_word16 left            PACK;
} T_3dNode ;

typedef struct  {
    T_sword16 x              PACK;
    T_sword16 y              PACK;
    T_word16 angle           PACK;
    T_sword16 objectType     PACK;
    T_sword16 attributes     PACK;
} T_3dObjectInFile ;

/*
typedef struct  {
    T_sword16 height ;
    T_sword16 light ;
    T_word16 picWidth ;
    T_word16 picHeight ;
    T_byte8 *p_picture ;
    T_word16 sector ;
    T_word16 nextFreeObject ;
    T_word16 accessoryData ;
    T_word16 numSectors ;
    T_word16 sectors[MAX_OBJECT_SECTORS] ;
    T_orientation orientation ;
} T_3dObjectInfo ;
*/

#define OBJECT_TYPE_BASIC_MASK       0x0FFF
#define OBJECT_TYPE_COLOR_MASK       0xF000
#define OBJECT_TYPE_COLOR_OFFSET     12

typedef struct T_3dObject_ {
    T_objMoveStruct objMove ;
    T_sword16 objectType ;
    T_objTypeInstance p_objType ;
    T_sword16 attributes ;
    T_word16 accessoryData ;
    T_byte8 *p_picture ;
    T_resource picResource ;
    T_orientation orientation ;
    T_word16 objServerId ;
    T_word32 objUniqueId ;
    T_word16 health ;
    T_void *extraData ;             /* Any extra data that needs to */
                                    /* be in addition to the object */
    T_word16 numPackets ;           /* Number of update packets sent */
                                    /* for this object. */
    T_word32 scaleX, scaleY ;
    T_script script ;
    struct T_3dObject_ *p_chainedObjects ;
                                    /* A chained object or list of chained */
                                    /* objects if piecewise. */
    struct T_3dObject_ *nextObj ;
    struct T_3dObject_ *prevObj ;

    struct T_3dObject_ *p_hash ;    /* Hash table link. */

    T_byte8 tag[4] ;
    E_Boolean inWorld ;
    T_sword16 illumination ;          /* Amount object lights up sectors. */
    T_word32 ownerID ;          /* Owner of this object
                                         (who produced it). 0=none */
    T_word16 lastSectorSteppedOn ;      /* 0xFFFF is initial condition. */

    /* Keep track of our location in the object collision list. */
    T_doubleLinkListElement elementInObjCollisionList ;

    /* Keep track of the obj collision list number we are on. */
    T_word16 objCollisionGroup ;
} T_3dObject ;

typedef struct  {
    T_sword16 xOrigin        PACK;
    T_sword16 yOrigin        PACK;
    T_sword16 columns        PACK;
    T_sword16 rows           PACK;

/** The GCC parser will not allow empty array decls. **/
#ifdef __GNUC__
    T_word16 blockIndexes[1] PACK;
#else
    T_word16 blockIndexes[];
#endif
} T_3dBlockMapHeader ;

typedef T_sword16 T_3dBlockMap ;

typedef struct {
    T_sword16 number ;
    T_3dObject *p_obj ;
} T_bodyPart ;

#endif

/****************************************************************************/
/*    END OF FILE:  VIEWFILE.H                                              */
/****************************************************************************/
