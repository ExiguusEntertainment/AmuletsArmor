/****************************************************************************/
/*    FILE:  SOUND.H                                                        */
/****************************************************************************/
#ifndef _SOUND_H_
#define _SOUND_H_

#include "GENERAL.H"

#define VOLUME_LEVEL_100 255
#define VOLUME_LEVEL_90  230
#define VOLUME_LEVEL_80  204
#define VOLUME_LEVEL_75  192
#define VOLUME_LEVEL_70  179
#define VOLUME_LEVEL_60  153
#define VOLUME_LEVEL_50  128
#define VOLUME_LEVEL_40  102
#define VOLUME_LEVEL_30  77
#define VOLUME_LEVEL_25  64
#define VOLUME_LEVEL_20  51
#define VOLUME_LEVEL_10  26
#define VOLUME_LEVEL_0   0

#define SOUND_BAD -1
typedef T_void (*T_soundDoneCallback)(T_void *p_data) ;

T_void SoundInitialize(T_void) ;

T_void SoundFinish(T_void) ;

T_void SoundSetBackgroundMusic(const char *filename) ;

T_sword16 SoundPlayByName(const char *filename, T_word16 volume) ;

T_sword16 SoundPlayByNumber(T_word16 num, T_word16 volume) ;

T_sword16 SoundPlayByNumberWithCallback(
              T_word16 num,
              T_word16 volume,
              T_soundDoneCallback callback,
              T_void *p_data) ;

T_sword16 SoundPlayLoopByNumberWithCallback(
              T_word16 num,
              T_word16 volume,
              T_soundDoneCallback callback,
              T_void *p_data) ;

E_Boolean SoundIsOn(T_void) ;

T_void SoundSetBackgroundVolume(T_byte8 volume) ;

T_byte8 SoundGetBackgroundVolume(T_void) ;

T_sword16 SoundPlayLoopByNumber(T_word16 soundNum, T_word16 volume) ;

T_word16 SoundGetEffectsVolume(T_void) ;

T_void SoundSetEffectsVolume(T_word16 vol) ;

T_void SoundUpdate(T_void) ;

T_void SoundStopAllSounds(T_void) ;

E_Boolean SoundIsDone(T_word16 bufferId) ;

T_void SoundSetVolume(T_word16 bufferId, T_word16 volume) ;

T_void SoundSetStereoPanLocation(T_word16 bufferId, T_word16 panLocation) ;

T_void SoundStop(T_word16 bufferId) ;

E_Boolean SoundGetAllowFreqShift(T_void) ;

T_void SoundSetAllowFreqShift(E_Boolean newAllow) ;

T_void SoundUpdateOften(T_void) ;

T_void SoundStopBackgroundMusic(T_void) ;

T_sword16 SoundPlayByNameWithDetails(
              T_byte8 *filename,
              T_word16 volume,
              T_word16 frequency,
              T_word16 bits,
              E_Boolean isStereo) ;

T_sword16 SoundPlayByNumberWithDetails(
             T_word16 num,
             T_word16 volume,
             T_word16 frequency,
             T_word16 bits,
             E_Boolean isStereo) ;

#endif

/****************************************************************************/
/*    END OF FILE:  SOUND.H                                                 */
/****************************************************************************/
