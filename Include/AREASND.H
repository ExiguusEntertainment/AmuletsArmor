/****************************************************************************/
/*    FILE:  AREASND.H                                                      */
/****************************************************************************/

#ifndef _AREASND_H_
#define _AREASND_H_

#include "GENERAL.H"

typedef T_void *T_areaSoundGroup ;
#define AREA_SOUND_GROUP_BAD NULL

typedef void *T_areaSound ;

typedef enum {
    AREA_SOUND_TYPE_ONCE,
    AREA_SOUND_TYPE_LOOP,
    AREA_SOUND_TYPE_UNKNOWN
} E_areaSoundType ;

typedef void (*T_areaSoundFinishCallback)(T_areaSound sound,
                                          T_word32 data) ;

#define AREA_SOUND_MAX_VOLUME 255
#define AREA_SOUND_BAD        NULL

T_void AreaSoundInitialize(T_void) ;

T_void AreaSoundFinish(T_void) ;

T_void AreaSoundLoad(T_word32 mapNumber) ;

T_areaSound AreaSoundCreate(
                T_sword16 x,
                T_sword16 y,
                T_word16 radius,
                T_word16 volume,
                E_areaSoundType type,
                T_word16 length,
                T_areaSound p_groupLeader,
                T_areaSoundFinishCallback p_callback,
                T_word32 data,
                T_word16 soundNum) ;

T_void AreaSoundDestroy(T_areaSound areaSound) ;

T_void AreaSoundUpdate(T_sword16 listenX, T_sword16 listenY) ;

T_void AreaSoundMove(T_areaSound areaSound, T_sword16 newX, T_sword16 newY) ;

T_areaSound AreaSoundFindGroupLeader(T_word16 groupID) ;

T_void AreaSoundSetGroupId(T_areaSound areaSound, T_word16 groupID) ;

#ifndef NDEBUG
T_void AreaSoundCheck(T_void) ;
#else
#define AreaSoundCheck()  ((T_void)0)
#endif

#endif

/****************************************************************************/
/*    END OF FILE:  AREASND.H                                               */
/****************************************************************************/
