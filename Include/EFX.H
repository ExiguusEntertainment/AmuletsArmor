/****************************************************************************/
/*    FILE:  EFX.H                                                            */
/****************************************************************************/
#ifndef _EFX_H_
#define _EFX_H_

#include "DBLLINK.H"

typedef T_doubleLinkListElement T_efxID;
typedef T_void (*T_efxHandler)(T_efxID efxID) ;

#define OBJECT_TYPE_TELEPORT           4091
#define OBJECT_TYPE_WALL_HIT           4085

#define OBJECT_TYPE_POWER_EXPLOSION    82
#define OBJECT_TYPE_POISON_EXPLOSION   83
#define OBJECT_TYPE_ACID_EXPLOSION     86
#define OBJECT_TYPE_ELECTRIC_EXPLOSION 87
#define OBJECT_TYPE_MAGIC_EXPLOSION    97
#define OBJECT_TYPE_FIRE_EXPLOSION     98

#define OBJECT_TYPE_BLOOD_SHRAPNEL_1   4084
#define OBJECT_TYPE_BLOOD_SHRAPNEL_2   92
#define OBJECT_TYPE_BLOOD_SHRAPNEL_3   93
#define OBJECT_TYPE_BONE_SHRAPNEL      94
#define OBJECT_TYPE_MANA_SHRAPNEL      91
#define OBJECT_TYPE_POISON_SHRAPNEL    96
#define OBJECT_TYPE_ELECTRIC_SHRAPNEL  90
#define OBJECT_TYPE_ACID_SHRAPNEL      95
#define OBJECT_TYPE_FLAME_SHRAPNEL     85
#define OBJECT_TYPE_FIRE_SHRAPNEL      84

typedef enum
{
    EFX_WALL_HIT,
    EFX_TELEPORT,
    EFX_FIRE_EXPLOSION,
    EFX_ACID_EXPLOSION,
    EFX_POISON_EXPLOSION,
    EFX_ELECTRIC_EXPLOSION,
    EFX_BLOOD_EXPLOSION,
    EFX_POWER_EXPLOSION,
    EFX_MAGIC_EXPLOSION,
    EFX_COMBO_EXPLOSION,     //use bits 0-5 (extraData) for explosion type
    EFX_FIRE_SHRAPNEL,
    EFX_FLAME_SHRAPNEL,
    EFX_ACID_SHRAPNEL,
    EFX_POISON_SHRAPNEL,
    EFX_ELECTRIC_SHRAPNEL,
    EFX_BLOOD_SHRAPNEL,
    EFX_MAGIC_SHRAPNEL,
    EFX_BONE_SHRAPNEL,
    EFX_COMBO_SHRAPNEL,      //use bits 0-7 (extraData) for explosion type
    EFX_UNKNOWN
} E_efxType;


typedef struct
{
    T_doubleLinkList        objectList;
    T_word32                Xorigin;
    T_word32                Yorigin;
    T_word32                Zorigin;
    T_word16                numberOf;
    T_word32                reserved;
    T_word16                duration;
    T_efxHandler            updateCallback;
    T_doubleLinkListElement myID;
} T_efxStruct;


T_void EfxInit (T_void);
T_void EfxFinish (T_void);
T_void EfxUpdate (T_void);

T_efxID EfxCreate (E_efxType type,        // type of effect
                   T_word32  Xorigin,     // x,y,z location of effect
                   T_word32  Yorigin,
                   T_word32  Zorigin,
                   T_word16  numberOf,    // number of objects to create
                   E_Boolean transparent, // should objects be transparent?
                   T_word32  extraData);  // xtra data for some efx

T_void EfxDestroy (T_efxID efxID);

#endif
