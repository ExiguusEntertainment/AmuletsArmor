/*-------------------------------------------------------------------------*
 * File:  SOUND.C
 *-------------------------------------------------------------------------*/
/**
 * Routines for playing sounds and looping them.
 *
 * @addtogroup SOUND
 * @brief Sound System
 * @see http://www.amuletsandarmor.com/AALicense.txt
 * @{
 *
 *<!-----------------------------------------------------------------------*/
#include "CONFIG.H"
#include "FILE.H"
#include "MEMORY.H"
#include "RESOURCE.H"
#include "SOUND.H"

#include "keys.h"         // Include #define's for keyboard commands

#ifdef USE_SOS_LIBRARY
#include "sos.h"
#include "sosm.h"
#include "sosez.h"
#include "profile.h"
#include "sosfnct.h"
#endif

#ifdef USE_SOS_LIBRARY
/* Use the following compile option to output sound lists */
//#ifdef COMPILE_OPTION_SOUND_CHECK_LIST

#define BUFFER_ID_BAD 0xFFFF
//#define COMPILE_OPTION_OUTPUT_BAD_SOUNDS
#define MUSICVOLUME 192
#define MAX_SOUND_CHANNELS 32

#define FALSE 0
#define TRUE 1


/* Background sample being played. */
static      W32  G_backSong = -1 ;

static T_void cdecl ISampleDone(struct _tag_sos_sample *p_sample) ;

/* external variables located in sosez.c */
extern   W32  wDIGIDeviceID;
extern   W32  wMIDIDeviceID;
extern   W32  hDIGIDriver;
extern   W32  hMIDIDriver;

#ifdef COMPILE_OPTION_OUTPUT_BAD_SOUNDS
/* For debugging purposes, output a list of all the bad sounds. */
static FILE *fileBadSounds = NULL ;
#endif

/* Streamed music information */
#define SIZE_SOUND_STREAM_BUFFER  8192
#define NUM_STREAM_BUFFERS        16
static T_byte8 G_musicBuffer[NUM_STREAM_BUFFERS][SIZE_SOUND_STREAM_BUFFER] ;
static T_word32 G_musicBufferLen[NUM_STREAM_BUFFERS] ;
static T_byte8 G_nextMusicBuffer = 0 ;
static T_byte8 G_nextFillBuffer = 0 ;
static T_file G_musicFile = FILE_BAD ;
static W32 G_musicHandle = 0 ;
static _SOS_SAMPLE G_musicSample ;
static T_word32 G_musicLength = 0 ;
static T_word32 G_musicPos = 0 ;
static E_Boolean G_musicNeedsUpdate = FALSE ;
#endif

#ifdef WIN32
#define MAX_SOUND_CHANNELS 32
#define BUFFER_ID_BAD ((T_word16)0xFFFF)
#endif

#define  _PAN_LEFT         0x00000000
#define  _PAN_CENTER       0x00008000
#define  _PAN_RIGHT        0x0000ffff

typedef struct {
        // Actively used buffer?
        E_Boolean inUse;

        // Are we playing?
        E_Boolean isPlaying;

        // Pointer to the raw sample data
        void *data;

        // Sample length
        T_word32 length;

        // Sample position
        T_word32 position;

        // freq divider
        T_word16 frequencyDivider;

        // Current sound level on left and right
        T_byte8 volume;

        // Panning position (0x0000 = all left, 0xFFFF = all right, 0x8000 = centered)
        T_word16 pan;

        // ETrue if 16bit, else 8bit
        E_Boolean is16Bit;
        // Is this a signed or unsigned input sample?
        E_Boolean isUnsigned;

        // Keep playing this in a loop?
        E_Boolean loop;

        // Is this Music (TRUE) or normal sound (FALSE)
        E_Boolean isMusic;

        T_word16 sampleRate;
} T_SDLSoundBuffer;
static T_SDLSoundBuffer G_soundBuffers[MAX_SOUND_CHANNELS];

/* Keep a structure per sound. */
typedef struct {
    T_resource resource ;
#ifdef USE_SOS_LIBRARY
    _SOS_SAMPLE sosSample ;
    W32 sosHandle ;
#else
    T_SDLSoundBuffer *sample;
#endif
    T_word16 volume ;
    T_soundDoneCallback doneCallback ;
    T_void *doneCallbackData ;
    E_Boolean isAllocated ;
    T_word16 next ;
    T_word16 prev ;
    T_void *p_sample ;
    T_word32 size ;
} T_soundBuffer ;

static T_byte8 G_currentSong[20] = "" ;

static T_resourceFile G_soundsFile ;
static E_Boolean G_soundsInit = FALSE ;

/* How many sounds are currently being played? */
T_word16 G_numSoundsPlaying = 0 ;
static T_word16 G_firstFreeBuffer = 0 ;
static T_word16 G_playingList = BUFFER_ID_BAD ;

/* Volume of sound effects. */
static T_word16 G_soundVolume = 192 ;

/* Volume of background music. */
static T_word16 G_musicVolume = 192 ;

/* Flag to tell if we are doing 16 bit sound. */
static E_Boolean G_is16BitSound = FALSE ;

/* Internal prototypes: */
static T_sword16 IAllocateBuffer(T_resource res) ;

static T_void IFreeBuffer(T_word16 bufferId) ;

static T_void ISoundStopAtExit(T_void) ;

static T_void ISoundFadeMusic(T_word32 originalVolume) ;

static T_void ISoundFadeInMusic(T_word32 originalVolume) ;

static T_void ISoundStartStreamIO(char *filename) ;

static T_void ICheckForNextMusicUpdate(T_void) ;

static T_sword16 IAllocateBufferDirect(void *aRawSound, T_word32 aSize);

T_soundBuffer G_soundBufferArray[MAX_SOUND_CHANNELS] ;

static E_Boolean G_allowFreqShift = TRUE ;

static T_void *G_backgroundMusic = 0;
static T_word16 G_backgroundMusicID = BUFFER_ID_BAD;

#ifdef COMPILE_OPTION_SOUND_CHECK_LIST
static T_void ICheckLists(T_void) ;
#else
#define ICheckLists()
#endif


#ifdef USE_SOS_LIBRARY
/*-------------------------------------------------------------------------*
 * Routine:  SoundInitialize
 *-------------------------------------------------------------------------*/
/**
 *  SoundInitialize is called to start up the sound system and prepare
 *  all interrupts and timers necessary for sound effects and background
 *  music.
 *
 *<!-----------------------------------------------------------------------*/
T_void SoundInitialize(T_void)
{
    T_word16 i ;
    T_word16 report = 1 ;
    DebugRoutine("SoundInitialize") ;

    if (G_soundsInit == FALSE)  {
        /* Clear out the buffers. */
        memset(G_soundBufferArray, 0, sizeof(G_soundBufferArray)) ;

        /* Link the buffers together into the free buffer. */
        for (i=0; i<MAX_SOUND_CHANNELS; i++)  {
            G_soundBufferArray[i].next = i+1 ;
            G_soundBufferArray[i].prev = i-1 ;
        }
        /* Fix the end points. */
        G_soundBufferArray[0].prev = BUFFER_ID_BAD ;
        G_soundBufferArray[MAX_SOUND_CHANNELS-1].next = BUFFER_ID_BAD ;
        G_firstFreeBuffer = 0 ;
        G_playingList = BUFFER_ID_BAD ;
        ICheckLists() ;

        /* Choose the resource file based on 16 or 8 bit sound. */
        if (G_is16BitSound)  {
            G_soundsFile = ResourceOpen("sounds16.res") ;
        } else {
            G_soundsFile = ResourceOpen("sounds.res") ;
        }

        /* retrieve configuration information from .cfg file */
        if (sosEZGetConfig("hmiset.cfg"))  {
            /* Turn on the sound system. */
            if (!sosEZInitSystem(wDIGIDeviceID, wMIDIDeviceID))
                report = 0 ;
        }
    }

    /* Note if sound is on or off and take appropriate actions. */
    if (report == 0)  {
//        puts("Sound system is ON") ;
	    G_soundsInit = TRUE ;

        /* put in a stopper to keep things from crashing */
        atexit(ISoundStopAtExit) ;
    } else {
        /* No sound system, turn off everything. */
//        puts("Sound system is OFF") ;
        G_soundsInit = FALSE ;
        ResourceClose(G_soundsFile) ;
        G_soundsFile = RESOURCE_FILE_BAD;
    }

    fflush(stdout) ;

#ifdef COMPILE_OPTION_OUTPUT_BAD_SOUNDS
    fileBadSounds = fopen("badsound.lst", "a") ;
    fprintf(fileBadSounds, "------------------------------- SESSION\n");
#endif

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  SoundFinish
 *-------------------------------------------------------------------------*/
/**
 *  SoundFinish is called before exiting the program.  It turns off all
 *  the interrupts and timers that the sound system uses.
 *
 *<!-----------------------------------------------------------------------*/
T_void SoundFinish(T_void)
{
    DebugRoutine("SoundFinish") ;

    if (G_soundsInit == TRUE)  {
        /* Stop all music */
        SoundStopBackgroundMusic() ;

        /* Stop all the sounds being played. */
        SoundStopAllSounds() ;

        /* Make sure our interrupts are still good before */
        /* we uninstall ours. */
        DebugCheckVectorTable() ;

        /* Turn off the sound system. */
        sosEZUnInitSystem();

        /* Save this change in the vector table. */
        DebugSaveVectorTable() ;

        /* Note that our sound system is disabled. */
        G_soundsInit = FALSE ;

        /* Close the resource file. */
	    ResourceClose(G_soundsFile) ;
        G_soundsFile = RESOURCE_FILE_BAD;

#ifdef COMPILE_OPTION_OUTPUT_BAD_SOUNDS
        /* Close out the file to tell we got bad sounds. */
        fclose(fileBadSounds) ;
#endif

    }

//delay(3000) ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  SoundSetBackgroundMusic
 *-------------------------------------------------------------------------*/
/**
 *  SoundSetBackgroundMusic loads and plays a background song.
 *
 *  NOTE: 
 *  At this point, I don't know what happens when you call this routine
 *  while a song is already in progress.
 *
 *  @param filename -- Name of background song to play.
 *
 *<!-----------------------------------------------------------------------*/
T_void SoundSetBackgroundMusic(const char *filename)
{
    W32 newSong;
    char realFilename[80] ;
	DebugRoutine("SoundSetBackgroundMusic") ;

    if (SoundIsOn())  {
        /* See if we are already playing this one. */
        if (strcmp(filename, G_currentSong) != 0)  {
            /* Record as this song. */
            strcpy(G_currentSong, filename) ;

            /* Stop whatever sound is currently playing. */
            SoundStopBackgroundMusic() ;

            if (G_musicVolume != 0)  {
                switch (ConfigGetMusicType()) {
                    case MUSIC_TYPE_MIDI:
                        sprintf(realFilename, "%s.HMI", filename) ;
                        if ((newSong = sosEZLoadSong(realFilename)) != -1)  {
                            G_backSong = newSong ;

                            /* start playing the song. */
                            sosMIDIStartSong(G_backSong);
                        } else {
#                           ifndef NDEBUG
                            printf("Could not play song %s\n", filename) ;
#                           endif
                        }

    //                    SoundSetBackgroundVolume(G_musicVolume) ;
                        break;

                    case MUSIC_TYPE_STREAM_IO:
                        ISoundStartStreamIO((T_byte8 *)filename) ;
    //                    SoundSetBackgroundVolume(G_musicVolume) ;
                        break ;

                    case MUSIC_TYPE_NONE:
                    default:
                        break ;
                }
            }
        }
    }

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  SoundPlayByName
 *-------------------------------------------------------------------------*/
/**
 *  SoundPlayByName looks up a sound in the sound file, if not loaded--
 *  loads it in, and plays that sound.  Finally, the sound is unlocked
 *  for possible removal.
 *
 *  @param filename -- Name of sound to play
 *
 *  @return Channel number of sound, or -1 if none.
 *
 *<!-----------------------------------------------------------------------*/
T_sword16 SoundPlayByName(const char *filename, T_word16 volume)
{
    T_resource sound ;
    T_word32 newVolume ;
    _SOS_SAMPLE *p_sample ;
    W32 hSample;
    T_word16 bufferId = BUFFER_ID_BAD ;
    T_soundBuffer *p_buffer ;

    DebugRoutine("PlaySoundByName") ;

    if (G_soundsInit == TRUE)  {
        sound = ResourceFind(G_soundsFile, (T_byte8 *)filename) ;
        if (sound != RESOURCE_BAD)  {
            bufferId = IAllocateBuffer(sound) ;
            if (bufferId != BUFFER_ID_BAD)  {
                DebugCheck(bufferId < MAX_SOUND_CHANNELS) ;
                DebugCheck(bufferId >= 0) ;
                p_buffer= G_soundBufferArray + bufferId ;
                p_sample = &p_buffer->sosSample ;
                DebugCheck(p_sample != NULL) ;

                memset(p_sample, 0, sizeof(_SOS_SAMPLE)) ;
                p_sample->pSample = p_buffer->p_sample ;
                p_sample->wLength = p_buffer->size ;

                if (G_is16BitSound)  {
                    p_sample->wBitsPerSample = 0x10 ;
                    p_sample->wChannels = 1 ;
                    if (G_allowFreqShift)
                        p_sample->wRate = 22050 + (rand() & 2047) - 1000 ;
                    else
                        p_sample->wRate = 22050 ;
//                        p_sample->wFormat = _PCM_UNSIGNED ;
                    p_sample->wFormat = 0 ;
                } else {
                    p_sample->wBitsPerSample = 0x08 ;
                    p_sample->wChannels = 1 ;
                    if (G_allowFreqShift)
                        p_sample->wRate = 11000 + (rand() & 1023) - 500 ;
                    else
                        p_sample->wRate = 11000 ;

                    p_sample->wFormat = _PCM_UNSIGNED ;
                }

                p_sample->wPanPosition = _PAN_CENTER ;
                newVolume = volume ;
                newVolume *= G_soundVolume ;
                newVolume >>= 1 ;
                p_sample->wVolume = MK_VOLUME(newVolume, newVolume) ;

                hSample = sosDIGIStartSample(hDIGIDriver, p_sample) ;

                /* Did we get a sound to play? */
                if (hSample != _ERR_NO_SLOTS)  {
                    /* The sound is being played.  */
                    /* Note what resource and sound */
                    /* this is. */
                    DebugCheck(hSample < MAX_SOUND_CHANNELS) ;
                    p_buffer->sosHandle = hSample ;
                    p_buffer->volume = volume ;
                    p_buffer->doneCallback = NULL ;
                } else {
                    /* No sound.  Let the buffer go. */
                    IFreeBuffer(bufferId) ;
                    bufferId = BUFFER_ID_BAD ;
                }
            }
        } else {
#ifdef COMPILE_OPTION_OUTPUT_BAD_SOUNDS
            fprintf(fileBadSounds, "sound '%s' not found\n", filename) ;  fflush(fileBadSounds) ;
            MessagePrintf("sound '%s' not found\n", filename) ;  fflush(fileBadSounds) ;
#endif
        }

    }

    DebugEnd() ;

    return bufferId ;
}

/*-------------------------------------------------------------------------*
 * Routine:  SoundPlayLoopByNumber
 *-------------------------------------------------------------------------*/
/**
 *  SoundPlayLoopByNumber works just like SoundPlayByNumber except that
 *  it loops the sound being played until it is stopped.
 *
 *  @param soundNum -- Number of sound to play
 *  @param volume -- Volume level to play sound
 *
 *  @return Channel number of sound, or -1 if none.
 *
 *<!-----------------------------------------------------------------------*/
T_sword16 SoundPlayLoopByNumber(T_word16 soundNum, T_word16 volume)
{
    T_resource sound ;
    T_word32 newVolume ;
    _SOS_SAMPLE *p_sample ;
    W32 hSample;
    T_word16 bufferId = BUFFER_ID_BAD ;
    T_soundBuffer *p_buffer ;
    T_byte8 filename[20] ;

    DebugRoutine("SoundPlayLoopByNumber") ;
    if (G_soundsInit == TRUE)  {
	    sprintf(filename, "snd#%d", soundNum) ;
        sound = ResourceFind(G_soundsFile, filename) ;
        if (sound != RESOURCE_BAD)  {
            bufferId = IAllocateBuffer(sound) ;
            if (bufferId != BUFFER_ID_BAD)  {
                DebugCheck(bufferId < MAX_SOUND_CHANNELS) ;
                DebugCheck(bufferId >= 0) ;
                p_buffer= G_soundBufferArray + bufferId ;
                p_sample = &p_buffer->sosSample ;
                DebugCheck(p_sample != NULL) ;

                memset(p_sample, 0, sizeof(_SOS_SAMPLE)) ;
                p_sample->pSample = p_buffer->p_sample ;
                p_sample->wLength = p_buffer->size ;

                if (G_is16BitSound)  {
                    p_sample->wBitsPerSample = 0x10 ;
                    p_sample->wChannels = 1 ;
                    if (G_allowFreqShift)
                        p_sample->wRate = 22050 + (rand() & 2047) - 1000 ;
                    else
                        p_sample->wRate = 22050 ;
//                        p_sample->wFormat = _PCM_UNSIGNED ;
                    p_sample->wFormat = 0 ;
                } else {
                    p_sample->wBitsPerSample = 0x08 ;
                    p_sample->wChannels = 1 ;
                    if (G_allowFreqShift)
                        p_sample->wRate = 11000 + (rand() & 1023) - 500 ;
                    else
                        p_sample->wRate = 11000 ;
                    p_sample->wFormat = _PCM_UNSIGNED ;
                }

                p_sample->wPanPosition = _PAN_CENTER ;
                newVolume = volume ;
                newVolume *= G_soundVolume ;
                newVolume >>= 1 ;
                p_sample->wVolume = MK_VOLUME(newVolume, newVolume) ;

                /* Make the sound loop. */
                p_sample->wLoopLength = p_buffer->size ;
                p_sample->wLoopEndLength = 0 ;
                p_sample->wLoopStage = 0 ;
                p_sample->wLoopCount = 0x7FFFFFFF ;

                /* Start playing the sample. */
                hSample = sosDIGIStartSample(hDIGIDriver, p_sample) ;

                /* Did we get a sound to play? */
                if (hSample != _ERR_NO_SLOTS)  {
                    /* The sound is being played.  */
                    /* Note what resource and sound */
                    /* this is. */
                    DebugCheck(hSample < MAX_SOUND_CHANNELS) ;
                    p_buffer->sosHandle = hSample ;
                    p_buffer->volume = volume ;
                    p_buffer->doneCallback = NULL ;
                } else {
                    /* No sound.  Let the buffer go. */
                    IFreeBuffer(bufferId) ;
                    bufferId = BUFFER_ID_BAD ;
                }
            }
        } else {
#ifdef COMPILE_OPTION_OUTPUT_BAD_SOUNDS
            fprintf(fileBadSounds, "sound '%s' not found\n", filename) ;  fflush(fileBadSounds) ;
            MessagePrintf("sound '%s' not found\n", filename) ;  fflush(fileBadSounds) ;
#endif
        }
    }

    DebugEnd() ;

    return bufferId ;
}

/*-------------------------------------------------------------------------*
 * Routine:  SoundPlayByNumber
 *-------------------------------------------------------------------------*/
/**
 *  SoundPlayByNumber looks up the sound to be played by its number.
 *  In actuallity, it looks for the name snd#??? where ??? is the number
 *  to be played.  Once found, the digital sound is played.
 *
 *  @param num -- #0-65535 of the sound to play
 *  @param volume -- Volume level to play sound
 *
 *  @return Channel number of sound, or -1 if none.
 *
 *<!-----------------------------------------------------------------------*/
T_sword16 SoundPlayByNumber(T_word16 num, T_word16 volume)
{
    char buffer[20] ;
    T_word16 bufferId = BUFFER_ID_BAD ;

    DebugRoutine("PlaySoundByNumber") ;

    /* Don't play any sound unless the sound system was initialized. */
    if (G_soundsInit == TRUE)  {
	    sprintf(buffer, "snd#%d", (T_word32)num) ;
	    bufferId = SoundPlayByName(buffer, volume) ;
    }

    DebugEnd() ;

    return bufferId ;
}


/*-------------------------------------------------------------------------*
 * Routine:  SoundSetBackgroundVolume
 *-------------------------------------------------------------------------*/
/**
 *  SoundSetBackgroundVolume changes the volume that the current or
 *  soon to be played background music is at.
 *
 *  @param volume -- Volume level (0-255)
 *
 *<!-----------------------------------------------------------------------*/
T_void SoundSetBackgroundVolume(T_byte8 volume)
{
    T_word32 vol ;
    T_byte8 oldVol ;
    char filename[80] ;

    DebugRoutine("SoundSetBackgroundVolume") ;

    oldVol = G_musicVolume ;
    if (G_soundsInit)  {
        /* Just turn off the music */
        if (volume == 0)  {
            G_musicVolume = 0 ;
            SoundStopBackgroundMusic() ;
        } else {
            oldVol = G_musicVolume ;

            G_musicVolume = volume ;
            vol = volume ;
            vol <<= 7 ;

            /* Music was off ... start it up */
            if ((oldVol == 0) && (volume != 0))  {
                strcpy(filename, G_currentSong) ;
                G_currentSong[0] = '\0' ;
                SoundSetBackgroundMusic(filename) ;
            }

            sosMIDISetMasterVolume(volume) ;

            if (G_musicFile != FILE_BAD)  {
                sosDIGISetSampleVolume(
                    hDIGIDriver,
                    G_musicHandle,
                    MK_VOLUME(vol, vol)) ;
            }
        }
    }


    DebugEnd() ;
}

T_byte8 SoundGetBackgroundVolume(T_void)
{
    return G_musicVolume ;
}

/*-------------------------------------------------------------------------*
 * Routine:  SoundSetEffectsVolume
 *-------------------------------------------------------------------------*/
/**
 *  SoundSetEffectsVolume sets the volume of the sound effects.
 *
 *  @param volume -- Level of sound
 *
 *<!-----------------------------------------------------------------------*/
T_void SoundSetEffectsVolume(T_word16 volume)
{
    T_word16 i ;
    T_word16 vol ;
    T_word16 bufferId ;
    T_soundBuffer *p_buffer ;

    DebugRoutine("SoundSetEffectsVolume") ;
    DebugCheck(volume < 256) ;

    G_soundVolume = volume ;

    /* Go through the list of all playing sounds and modify */
    /* their volume. */
    bufferId = G_playingList ;
    while (bufferId != BUFFER_ID_BAD)  {
        DebugCheck(bufferId >= 0) ;
        DebugCheck(bufferId < MAX_SOUND_CHANNELS) ;
        p_buffer = G_soundBufferArray + bufferId ;

        vol = (p_buffer->volume * volume) >> 1 ;
        sosDIGISetSampleVolume(
            hDIGIDriver,
            p_buffer->sosHandle,
            vol) ;
        p_buffer->volume = vol>>7 ;

        bufferId = p_buffer->next ;
    }

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ISoundStopAtExit
 *-------------------------------------------------------------------------*/
/**
 *  ISoundStopAtExit is a callback routine for the atexit command that
 *  turns off the sound system.
 *
 *<!-----------------------------------------------------------------------*/
T_void ISoundStopAtExit(T_void)
{
#ifndef NDEBUG
//    puts("Restoring sound system.") ;
#endif
    SoundFinish() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  SoundUpdate
 *-------------------------------------------------------------------------*/
/**
 *  SoundUpdate should be called as often as possible to ensure the
 *  resource manager locks and unlocks all the data tied to the
 *  sound effects.
 *
 *<!-----------------------------------------------------------------------*/
T_void SoundUpdate(T_void)
{
    T_word16 bufferId ;
    T_soundBuffer *p_buffer ;
    T_sword16 nextId ;

    DebugRoutine("SoundUpdate") ;

//MessagePrintf("num sounds %d\n", G_numSoundsPlaying) ;
    if (SoundIsOn())  {
        /* Check for a completed music score. */
        if (G_backSong != -1)  {
            /* Repeat the song if it is finished. */
            if (sosMIDISongDone(G_backSong))  {
                /* start song */
                sosMIDIStartSong(G_backSong);
            }
        }

        /* Free all buffers that are complete. */
        bufferId = G_playingList ;
        while (bufferId != BUFFER_ID_BAD)  {
            DebugCheck(bufferId >= 0) ;
            DebugCheck(bufferId < MAX_SOUND_CHANNELS) ;
            p_buffer = G_soundBufferArray + bufferId ;
            nextId = p_buffer->next ;
            /* See if any sounds are done. */
            if (sosDIGISampleDone(hDIGIDriver, p_buffer->sosHandle))  {
                /* Do the callback first, then free. */
                if (p_buffer->doneCallback != NULL)  {
                    /* call the done callback reporting the */
                    /* sound complete. */
                    p_buffer->doneCallback(p_buffer->doneCallbackData) ;
                    p_buffer->doneCallback = NULL ;
                }
                /* If so, close out the buffer. */
                IFreeBuffer(bufferId) ;
            }
            bufferId = nextId ;
        }
    }

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  SoundIsDone
 *-------------------------------------------------------------------------*/
/**
 *  SoundIsDone checks to see if the given sound is turned off.
 *
 *  @param bufferId -- Channel/handle of previous sound.
 *
 *<!-----------------------------------------------------------------------*/
E_Boolean SoundIsDone(T_word16 bufferId)
{
    E_Boolean isDone ;
    T_soundBuffer *p_buffer ;

    DebugRoutine("SoundIsDone") ;
    DebugCheck(bufferId != BUFFER_ID_BAD) ;
    DebugCheck(bufferId >= 0) ;
    DebugCheck(bufferId < MAX_SOUND_CHANNELS) ;

    /* Get a buffer entry. */
    p_buffer = G_soundBufferArray + bufferId ;
    DebugCheck(p_buffer->isAllocated == TRUE) ;

    if (sosDIGISampleDone(hDIGIDriver, p_buffer->sosHandle))  {
        isDone = FALSE ;
    } else {
        isDone = TRUE ;
    }

    DebugEnd() ;

    return isDone ;
}

/*-------------------------------------------------------------------------*
 * Routine:  SoundSetVolume
 *-------------------------------------------------------------------------*/
/**
 *  SoundSetVolume changes the volume of a current sound effect.
 *
 *  @param bufferId -- Channel/handle of previous sound.
 *  @param volume -- Volume of 0-255 to change to.
 *
 *<!-----------------------------------------------------------------------*/
T_void SoundSetVolume(T_word16 bufferId, T_word16 volume)
{
    T_soundBuffer *p_buffer ;

    DebugRoutine("SoundSetVolume") ;

    if (SoundIsOn())  {
        DebugCheck(bufferId != BUFFER_ID_BAD) ;
        DebugCheck(bufferId >= 0) ;
        DebugCheck(bufferId < MAX_SOUND_CHANNELS) ;

        /* Get a buffer entry. */
        p_buffer = G_soundBufferArray + bufferId ;
        DebugCheck(p_buffer->isAllocated == TRUE) ;

        sosDIGISetSampleVolume(
            hDIGIDriver,
            p_buffer->sosHandle,
            (volume<<8)) ;
    }

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  SoundSetStereoPanLocation
 *-------------------------------------------------------------------------*/
/**
 *  SoundSetStereoPanLocation changes the stereo panning on a given
 *  sound effect.
 *
 *  @param bufferId -- Channel/handle of previous sound.
 *  @param panLocation -- Pan is 0 if all on left, 0x8000 if
 *      centered, and 0xFFFF if all on right.
 *
 *<!-----------------------------------------------------------------------*/
T_void SoundSetStereoPanLocation(T_word16 bufferId, T_word16 panLocation)
{
    T_soundBuffer *p_buffer ;

    DebugRoutine("SoundSetStereoPanLocation") ;

    if (SoundIsOn())  {
        DebugCheck(bufferId != BUFFER_ID_BAD) ;
        DebugCheck(bufferId >= 0) ;
        DebugCheck(bufferId < MAX_SOUND_CHANNELS) ;

        /* Get a buffer entry. */
        p_buffer = G_soundBufferArray + bufferId ;
        DebugCheck(p_buffer->isAllocated == TRUE) ;

        sosDIGISetPanLocation(
            hDIGIDriver,
            p_buffer->sosHandle,
            panLocation) ;
    }

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  SoundStop
 *-------------------------------------------------------------------------*/
/**
 *  SoundStop is called to force a sound to stop (particularly a looping
 *  sound)>
 *
 *  @param channel -- Channel no longer being played.
 *
 *<!-----------------------------------------------------------------------*/
T_void SoundStop(T_word16 bufferId)
{
    T_soundBuffer *p_buffer ;

    DebugRoutine("SoundStop") ;

    DebugCheck(bufferId != BUFFER_ID_BAD) ;
    DebugCheck(bufferId >= 0) ;
    DebugCheck(bufferId < MAX_SOUND_CHANNELS) ;

    if (SoundIsOn())  {
        p_buffer = G_soundBufferArray + bufferId ;
        DebugCheck(p_buffer->isAllocated == TRUE) ;

        if (!sosDIGISampleDone(hDIGIDriver, p_buffer->sosHandle))
            sosDIGIStopSample(hDIGIDriver, p_buffer->sosHandle) ;
    }

    DebugEnd() ;
}

T_void SoundUpdateOften(T_void)
{
    static E_Boolean inHere = FALSE ;

    if (inHere != TRUE)  {
        inHere = TRUE ;
        if ((G_musicNeedsUpdate) && (G_musicFile != FILE_BAD))
            ICheckForNextMusicUpdate() ;
        inHere = FALSE ;
    }
}

static T_void ICheckForNextMusicUpdate(T_void)
{
    T_byte8 next ;
    T_byte8 now ;
    T_word16 i ;

    for (i=0; i<NUM_STREAM_BUFFERS; i++)  {
        if (G_musicNeedsUpdate)  {
            G_musicNeedsUpdate = FALSE ;

            next = G_nextFillBuffer ;
            now = G_nextMusicBuffer ;

            /* Repeat at beginning if we have reached the end. */
            if (G_musicPos >= G_musicLength)  {
                G_musicPos = 0 ;
                FileSeek(G_musicFile, 0) ;
            }

    //        if (now == 0xFF)
    //            now = NUM_STREAM_BUFFERS-1 ;

            G_musicBufferLen[next] =
                FileRead(G_musicFile,
                G_musicBuffer[next],
                SIZE_SOUND_STREAM_BUFFER) ;

            next++ ;
            if (next == NUM_STREAM_BUFFERS)
                next = 0 ;

            G_nextFillBuffer = next ;

            /* Move over a block */
            G_musicPos += SIZE_SOUND_STREAM_BUFFER ;

            if (next == now)
                G_musicNeedsUpdate = FALSE ;
            else
                G_musicNeedsUpdate = TRUE ;
        } else {
            break ;
        }
    }
}

VOID cdecl IUpdateMusicBuffer(struct _tag_sos_sample *p_sample)
{
    G_nextMusicBuffer++ ;
    if (G_nextMusicBuffer == NUM_STREAM_BUFFERS)
        G_nextMusicBuffer = 0 ;

    p_sample->pSample = G_musicBuffer[G_nextMusicBuffer] ;
    p_sample->wLength = G_musicBufferLen[G_nextMusicBuffer] ;

    G_musicNeedsUpdate = TRUE ;
}

static T_void ISoundStartStreamIO(char *filename)
{
    W32 newSong;
    T_word16 newVolume ;
    _SOS_SAMPLE *p_sample ;
    char realFilename[80] ;

	DebugRoutine("ISoundStartStreamIO") ;

#if 0
    sprintf(
        realFilename,
        "%c:\\aamusic\\%s.MUS",
        ConfigGetCDROMDrive(),
        filename) ;
#else
    sprintf(
        realFilename,
        "aamusic\\%s.MUS",
        filename) ;
#endif
    if (FileExist(realFilename) == TRUE)  {
        G_musicFile = FileOpen(realFilename, FILE_MODE_READ) ;
        G_musicLength = FileGetSize(realFilename) ;

        G_musicBufferLen[0] = FileRead(G_musicFile, G_musicBuffer[0], SIZE_SOUND_STREAM_BUFFER) ;

        p_sample = &G_musicSample ;

        DebugCheck(p_sample != NULL) ;

        memset(p_sample, 0, sizeof(_SOS_SAMPLE)) ;
        p_sample->pSample = G_musicBuffer[0] ;
        p_sample->wLength = G_musicBufferLen[0] ;
        G_musicPos = G_musicBufferLen[0] ;

        p_sample->wBitsPerSample = 0x10 ;
        p_sample->wChannels = 1 ;
        p_sample->wRate = 22050 ;
        p_sample->wFormat = 0 /* signed */ ;

        p_sample->wPanPosition = _PAN_CENTER ;
        newVolume = 255 ;
        newVolume *= G_musicVolume ;
        newVolume >>= 1 ;
        p_sample->wVolume = MK_VOLUME(newVolume, newVolume) ;
        p_sample->pfnSampleProcessed = IUpdateMusicBuffer ;

        /* Force the next buffer to be filled */
        G_musicNeedsUpdate = TRUE ;
        G_nextMusicBuffer = 0 ;
        G_nextFillBuffer = 1 ;
        ICheckForNextMusicUpdate() ;

        G_musicHandle = sosDIGIStartSample(hDIGIDriver, p_sample) ;
    } else {
#ifndef NDEBUG
        printf("Could not play song %s\n", realFilename) ;
#endif
        G_musicFile = FILE_BAD ;
    }

    DebugEnd() ;
}

T_void SoundStopBackgroundMusic(T_void)
{
    DebugRoutine("SoundStopBackgroundMusic") ;

    /* Stop the original MIDI song (if playing). */
	if (G_backSong != -1)
	{
        /* stop playing the last song. */
        sosMIDIStopSong(G_backSong);
        G_backSong = -1 ;
	}

    /* Stop any digital music */
    if (G_musicFile != FILE_BAD)  {
        sosDIGIStopSample(hDIGIDriver, G_musicHandle) ;
        FileClose(G_musicFile) ;
        G_musicFile = FILE_BAD ;
    }

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  SoundPlayByNameWithDetails
 *-------------------------------------------------------------------------*/
/**
 *
 *<!-----------------------------------------------------------------------*/
T_sword16 SoundPlayByNameWithDetails(
              T_byte8 *filename,
              T_word16 volume,
              T_word16 frequency,
              T_word16 bits,
              E_Boolean isStereo)
{
    T_resource sound ;
    T_word32 newVolume ;
    _SOS_SAMPLE *p_sample ;
    W32 hSample;
    T_word16 bufferId = BUFFER_ID_BAD ;
    T_soundBuffer *p_buffer ;

    DebugRoutine("PlaySoundByNameWithDetails") ;

    if (G_soundsInit == TRUE)  {
        sound = ResourceFind(G_soundsFile, filename) ;
        if (sound != RESOURCE_BAD)  {
            bufferId = IAllocateBuffer(sound) ;
            if (bufferId != BUFFER_ID_BAD)  {
                DebugCheck(bufferId < MAX_SOUND_CHANNELS) ;
                DebugCheck(bufferId >= 0) ;
                p_buffer= G_soundBufferArray + bufferId ;
                p_sample = &p_buffer->sosSample ;
                DebugCheck(p_sample != NULL) ;

                memset(p_sample, 0, sizeof(_SOS_SAMPLE)) ;
                p_sample->pSample = p_buffer->p_sample ;
                p_sample->wLength = p_buffer->size ;

                p_sample->wBitsPerSample = bits ;
                if (isStereo)
                    p_sample->wChannels = 2 ;
                else
                    p_sample->wChannels = 1 ;

                p_sample->wRate = frequency ;
                p_sample->wFormat = _PCM_UNSIGNED ;

                p_sample->wPanPosition = _PAN_CENTER ;
                newVolume = volume ;
                newVolume *= G_soundVolume ;
                newVolume >>= 1 ;
                p_sample->wVolume = MK_VOLUME(newVolume, newVolume) ;

                hSample = sosDIGIStartSample(hDIGIDriver, p_sample) ;

                /* Did we get a sound to play? */
                if (hSample != _ERR_NO_SLOTS)  {
                    /* The sound is being played.  */
                    /* Note what resource and sound */
                    /* this is. */
                    DebugCheck(hSample < MAX_SOUND_CHANNELS) ;
                    p_buffer->sosHandle = hSample ;
                    p_buffer->volume = volume ;
                    p_buffer->doneCallback = NULL ;
                } else {
                    /* No sound.  Let the buffer go. */
                    IFreeBuffer(bufferId) ;
                    bufferId = BUFFER_ID_BAD ;
                }
            }
        } else {
#ifdef COMPILE_OPTION_OUTPUT_BAD_SOUNDS
            fprintf(fileBadSounds, "sound '%s' not found\n", filename) ;  fflush(fileBadSounds) ;
            MessagePrintf("sound '%s' not found\n", filename) ;  fflush(fileBadSounds) ;
#endif
        }

    }

    DebugEnd() ;

    return bufferId ;
}

#else

#define SOUND_STREAM_NUM_SAMPLES      1024
SDL_AudioSpec G_audioSpec;

static void IMixer(void *userdata, Uint8 *stream, int len)
{
    unsigned int i;
    typedef struct {
            T_word16 left;
            T_word16 right;
    } T_stereoSample;
    T_stereoSample *p = (T_stereoSample *)stream;
    T_SDLSoundBuffer *p_buffer;
    T_sword32 left, right;
    T_sword32 v16;
    T_byte8 buffer;

    len /= sizeof(T_stereoSample);
    // len is number of bytes total
    // In this case, 4 bytes per sample, signed word16 on left, then signed word16 on right

    // For now, fill with zeros
    for (i=0; i<(unsigned int)len; i++) {
        // Compute sum of samples
        left = 0;
        right = 0;
        for (buffer=0; buffer<MAX_SOUND_CHANNELS; buffer++) {
            p_buffer = &G_soundBuffers[buffer];
            if (p_buffer->inUse) {
                if (p_buffer->isPlaying) {
                    if (p_buffer->isUnsigned) {
                        if (p_buffer->is16Bit) {
                            v16 = (T_word16)(((T_sword16 *)p_buffer->data)[p_buffer->position]);
                        } else {
                            v16 = (T_word16)((((T_sbyte8 *)p_buffer->data)[p_buffer->position]) << 8);
                        }
                        v16 -= 0x8000;
                    } else {
                        if (p_buffer->is16Bit) {
                            v16 = ((T_sword16 *)p_buffer->data)[p_buffer->position];
                        } else {
                            v16 = (((T_sbyte8 *)p_buffer->data)[p_buffer->position]) << 8;
                        }
                    }
                    v16 *= p_buffer->volume;
                    v16 /= 256;
                    v16 *= (p_buffer->isMusic) ? G_musicVolume : G_soundVolume;
                    v16 /= 256;
                    left += (v16 * (0xFFFF-p_buffer->pan)) / 0x10000;
                    right += (v16 * p_buffer->pan) / 0x10000;
                }
                p_buffer->frequencyDivider += p_buffer->sampleRate;
                if (p_buffer->frequencyDivider >= G_audioSpec.freq) {
                    p_buffer->position++;
                    p_buffer->frequencyDivider -= G_audioSpec.freq;
                }
                if (p_buffer->position >= p_buffer->length) {
                    if (p_buffer->loop) {
                        // Loop back at the start
                        p_buffer->position = 0;
                    } else {
                        // Sound is complete, stop here
                        p_buffer->isPlaying = FALSE;
                    }
                }
            }
        }

        // Clip the audio
        if (left > 32767)
            left = 32767;
        else if (left < -32767)
            left = -32767;
        if (right > 32767)
            right = 32767;
        else if (right < -32767)
            right = -32767;

        p->left = left;
        p->right = right;
        p++;
    }
}


#if 0 // test code
#include <math.h>
#define M_PI        (3.14159265358979323846)
#define M_PI_2      (1.57079632679489661923)

#define SAMPLE2_FREQUENCY       11025
static T_sword16 G_sample16[44100];
static T_sbyte8 G_sample8B[SAMPLE2_FREQUENCY];

void CreateTestSounds(void)
{
    int i;
    T_SDLSoundBuffer *p = &G_soundBuffers[0];

    // 1000 Hz tone
    for (i=0; i<44100; i++) {
        G_sample16[i] = sin(1000*2*M_PI*i/44100.0)*16384.0;
    }
    p->inUse = TRUE;
    p->data = G_sample16;
    p->is16Bit = TRUE;
    p->sampleRate = 44100;
    p->isPlaying = FALSE;
    p->length = 44100;
    p->loop = TRUE;
    p->frequencyDivider = 0;
//    p->pan = 0; // left only
    p->pan = 0x8000; // left and right
//    p->pan = 0x4000; // mostly to the left
//    p->pan = 0xFFFF; // right only
    p->position = 0;
//    p->volume = 255;
    p->volume = 25; // low volume

    p = &G_soundBuffers[1];
    // 500 Hz tone
    for (i=0; i<SAMPLE2_FREQUENCY; i++) {
        G_sample8B[i] = sin(500*2*M_PI*i/SAMPLE2_FREQUENCY)*64.0;
    }
    p->inUse = TRUE;
    p->data = G_sample8B;
    p->is16Bit = FALSE;
    p->isPlaying = FALSE;
    p->length = SAMPLE2_FREQUENCY;
    p->sampleRate = SAMPLE2_FREQUENCY;
    p->loop = TRUE;
    p->frequencyDivider = 0;
//    p->pan = 0; // left only
    p->pan = 0x8000; // left and right
//    p->pan = 0x4000; // mostly to the left
//    p->pan = 0xFFFF; // right only
    p->position = 0;
//    p->volume = 255;
    p->volume = 50; // mid volume
}

void TestSoundsPlay(void)
{
    G_soundBuffers[0].isPlaying = TRUE;
    SDL_Delay(500);
    G_soundBuffers[1].isPlaying = TRUE;
    SDL_Delay(3500);
    G_soundBuffers[0].isPlaying = FALSE;
    SDL_Delay(3500);
    G_soundBuffers[1].isPlaying = FALSE;
}
#endif

T_void SoundInitialize(T_void)
{
    SDL_AudioSpec desired;
    T_word16 i ;
    T_word16 report = 1 ;
    DebugRoutine("SoundInitialize") ;

    if (G_soundsInit == FALSE)  {
        /* Clear out the buffers. */
        memset(G_soundBufferArray, 0, sizeof(G_soundBufferArray)) ;

        // No buffers are active
        for (i=0; i<MAX_SOUND_CHANNELS; i++) {
            G_soundBuffers[i].inUse = FALSE;
            // 1 for 1 in this implementation
            G_soundBufferArray[i].sample = &G_soundBuffers[i];
        }

        /* Link the buffers together into the free buffer. */
        for (i=0; i<MAX_SOUND_CHANNELS; i++)  {
            G_soundBufferArray[i].next = i+1 ;
            G_soundBufferArray[i].prev = i-1 ;
        }
        /* Fix the end points. */
        G_soundBufferArray[0].prev = BUFFER_ID_BAD ;
        G_soundBufferArray[MAX_SOUND_CHANNELS-1].next = BUFFER_ID_BAD ;
        G_firstFreeBuffer = 0 ;
        G_playingList = BUFFER_ID_BAD ;
        ICheckLists() ;

        /* Choose the resource file based on 16 or 8 bit sound. */
        if (G_is16BitSound)  {
            G_soundsFile = ResourceOpen("sounds16.res") ;
        } else {
            G_soundsFile = ResourceOpen("sounds.res") ;
        }

        // 44KHz, let's go high fidelity
        desired.freq = 44100;
        // signed 16bit
        desired.format = AUDIO_S16SYS;
        // Stereo
        desired.channels = 2;
        // Audio sample buffer needs to be so-so large
        desired.samples = 2048;
        // Our callback function
        desired.callback = IMixer;
        desired.userdata = 0;
        // Open the audio device
        if (SDL_OpenAudio(&desired, &G_audioSpec) < 0) {
            printf("Couldn't open audio: %s\n", SDL_GetError());
            exit(-1);
        }

        SDL_PauseAudio(0);

#if 0 // test code
        CreateTestSounds();
        TestSoundsPlay();
#endif
    }

    /* Note if sound is on or off and take appropriate actions. */
//        puts("Sound system is ON") ;
	G_soundsInit = TRUE ;


#ifdef COMPILE_OPTION_OUTPUT_BAD_SOUNDS
    fileBadSounds = fopen("badsound.lst", "a") ;
    fprintf(fileBadSounds, "------------------------------- SESSION\n");
    fflush(stdout) ;
#endif

    DebugEnd() ;

    G_soundsInit = TRUE;
}

T_void SoundFinish(T_void)
{
    G_soundsInit = FALSE;
    SDL_CloseAudio();
}

static T_void IBackgroundMusicDone(void *data)
{
    T_soundBuffer *p_buffer = (T_soundBuffer *)data;

    // Free the music on this buffer
    MemFree(p_buffer->p_sample);
    p_buffer->p_sample = 0;
    MemCheck(8203);
}

T_void SoundSetBackgroundMusic(const char *filename)
{
    T_byte8 realFilename[80] ;
    T_file file;
    T_word32 length;
    T_soundBuffer *p_buffer ;
    T_SDLSoundBuffer *p_sample;

	DebugRoutine("SoundSetBackgroundMusic") ;
    MemCheck(8200);

    if (SoundIsOn())  {
        /* See if we are already playing this one. */
        if (strcmp((const char *)filename, (const char *)G_currentSong) != 0)  {
            MemCheck(8201);
            /* Record as the new song. */
            strcpy((char *)G_currentSong, (const char *)filename) ;

            if (G_backgroundMusic) {
                MemCheck(8202);
                /* Stop whatever sound is currently playing. */
                SoundStopBackgroundMusic();
            }

            // Load the new music (even if we are not going to play it yet)
            sprintf(realFilename, "AAMUSIC\\%s.MUS", filename);
            file = FileOpen(realFilename, FILE_MODE_READ) ;
            if (file != FILE_BAD) {
                length = FileGetSize(realFilename)/2;
                G_backgroundMusic = MemAlloc(length);
                DebugCheck(G_backgroundMusic != 0);
                MemCheck(8204);
                FileRead(file, G_backgroundMusic, length);
                MemCheck(8205);

                G_backgroundMusicID = IAllocateBufferDirect(G_backgroundMusic, length) ;
                if (G_backgroundMusicID != BUFFER_ID_BAD)  {
                    DebugCheck(G_backgroundMusicID < MAX_SOUND_CHANNELS) ;
                    DebugCheck(G_backgroundMusicID >= 0) ;
                    p_buffer = G_soundBufferArray + G_backgroundMusicID ;
                    p_sample = p_buffer->sample ;
                    DebugCheck(p_sample != NULL) ;

                    memset(p_sample, 0, sizeof(*p_sample)) ;
                    p_sample->inUse = TRUE; // we are about to use this one
                    p_sample->data = p_buffer->p_sample;
                    // 16-bit music uses half the samples
                    p_sample->length = p_buffer->size/2;

                    p_sample->is16Bit = TRUE ;
                    p_sample->sampleRate = 22050 ;
                    p_sample->isUnsigned = FALSE;

                    p_sample->pan = _PAN_CENTER ;
                    p_sample->volume = 255;
                    p_sample->loop = TRUE;
                    p_sample->isMusic = TRUE;
                    p_buffer->doneCallback = IBackgroundMusicDone ;
                    p_buffer->doneCallbackData = (void *)p_buffer;

                    // Start playing the music
                    p_sample->isPlaying = TRUE;
                }
                MemCheck(8210);
                FileClose(file);
            }
        } else {
#ifdef COMPILE_OPTION_OUTPUT_BAD_SOUNDS
            fprintf(fileBadSounds, "sound '%s' not found\n", filename) ;  fflush(fileBadSounds) ;
            MessagePrintf("sound '%s' not found\n", filename) ;  fflush(fileBadSounds) ;
#endif
        }
    }

    DebugEnd() ;
}

T_sword16 SoundPlayByName(const char *filename, T_word16 volume)
{
    T_resource sound ;
    T_SDLSoundBuffer *p_sample;
    T_word16 bufferId = BUFFER_ID_BAD ;
    T_soundBuffer *p_buffer ;

    DebugRoutine("PlaySoundByName") ;

    if (G_soundsInit == TRUE)  {
        sound = ResourceFind(G_soundsFile, (T_byte8 *)filename) ;
        if (sound != RESOURCE_BAD)  {
            bufferId = IAllocateBuffer(sound) ;
            if (bufferId != BUFFER_ID_BAD)  {
                DebugCheck(bufferId < MAX_SOUND_CHANNELS) ;
                DebugCheck(bufferId >= 0) ;
                p_buffer = G_soundBufferArray + bufferId ;
                p_sample = p_buffer->sample;
                DebugCheck(p_sample != NULL) ;

                memset(p_sample, 0, sizeof(*p_sample)) ;
                p_sample->inUse = TRUE; // we are about to use this one
                p_sample->data = p_buffer->p_sample;
                p_sample->length = p_buffer->size;

                if (G_is16BitSound)  {
                    // 16-bit has half the samples
                    p_sample->length /= 2;
                    p_sample->is16Bit = TRUE ;
                    if (G_allowFreqShift)
                        p_sample->sampleRate = 22050 + (rand() & 2047) - 1000 ;
                    else
                        p_sample->sampleRate = 22050 ;
                    p_sample->isUnsigned = FALSE;
                } else {
                    p_sample->is16Bit = FALSE ;
                    if (G_allowFreqShift)
                        p_sample->sampleRate = 11000 + (rand() & 1023) - 500 ;
                    else
                        p_sample->sampleRate = 11000 ;

                    p_sample->isUnsigned = TRUE;
                }

                p_sample->pan = _PAN_CENTER ;
                p_sample->volume = (T_byte8)volume;
                p_sample->isMusic = FALSE;
                p_buffer->doneCallback = NULL ;

                // Start playing the sound
                p_sample->isPlaying = TRUE;
            }
        } else {
#ifdef COMPILE_OPTION_OUTPUT_BAD_SOUNDS
            fprintf(fileBadSounds, "sound '%s' not found\n", filename) ;  fflush(fileBadSounds) ;
            MessagePrintf("sound '%s' not found\n", filename) ;  fflush(fileBadSounds) ;
#endif
        }

    }

    DebugEnd() ;

    return bufferId ;
}

T_sword16 SoundPlayByNumber(T_word16 num, T_word16 volume)
{
    char buffer[20] ;
    T_word16 bufferId = BUFFER_ID_BAD ;

    DebugRoutine("PlaySoundByNumber") ;

    /* Don't play any sound unless the sound system was initialized. */
    if (G_soundsInit == TRUE)  {
	    sprintf(buffer, "snd#%d", (T_word32)num) ;
	    bufferId = SoundPlayByName((T_byte8 *)buffer, volume) ;
    }

    DebugEnd() ;

    return bufferId ;
}

T_void SoundSetBackgroundVolume(T_byte8 volume)
{
    T_word16 oldVol ;
    char filename[80] ;

    DebugRoutine("SoundSetBackgroundVolume") ;

    oldVol = G_musicVolume ;
    if (G_soundsInit)  {
        /* Just turn off the music */
        if (volume == 0)  {
            G_musicVolume = 0 ;
            SoundStopBackgroundMusic() ;
        } else {
            G_musicVolume = volume ;

            /* Music was off ... start it up */
            if ((oldVol == 0) && (volume != 0))  {
                strcpy(filename, G_currentSong) ;
                G_currentSong[0] = '\0' ;
                SoundSetBackgroundMusic(filename) ;
            }
        }
    }


    DebugEnd() ;
}

T_byte8 SoundGetBackgroundVolume(T_void)
{
    return (T_byte8)G_musicVolume ;
}

T_sword16 SoundPlayLoopByNumber(T_word16 soundNum, T_word16 volume)
{
    T_resource sound ;
    T_word16 bufferId = BUFFER_ID_BAD ;
    T_soundBuffer *p_buffer ;
    T_SDLSoundBuffer *p_sample;
    char filename[20] ;

    DebugRoutine("SoundPlayLoopByNumber") ;
    if (G_soundsInit == TRUE)  {
	    sprintf(filename, "snd#%d", soundNum) ;
        sound = ResourceFind(G_soundsFile, (T_byte8 *)filename) ;
        if (sound != RESOURCE_BAD)  {
            bufferId = IAllocateBuffer(sound) ;
            if (bufferId != BUFFER_ID_BAD)  {
                DebugCheck(bufferId < MAX_SOUND_CHANNELS) ;
                DebugCheck(bufferId >= 0) ;
                p_buffer= G_soundBufferArray + bufferId ;
                p_sample = p_buffer->sample ;
                DebugCheck(p_sample != NULL) ;

                memset(p_sample, 0, sizeof(*p_sample)) ;
                p_sample->inUse = TRUE; // we are about to use this one
                p_sample->data = p_buffer->p_sample;
                p_sample->length = p_buffer->size;

                if (G_is16BitSound)  {
                    // 16-bit samples have half as many samples
                    p_sample->length /= 2;
                    p_sample->is16Bit = TRUE ;
                    if (G_allowFreqShift)
                        p_sample->sampleRate = 22050 + (rand() & 2047) - 1000 ;
                    else
                        p_sample->sampleRate = 22050 ;
                    p_sample->isUnsigned = FALSE;
                } else {
                    p_sample->is16Bit = FALSE ;
                    if (G_allowFreqShift)
                        p_sample->sampleRate = 11000 + (rand() & 1023) - 500 ;
                    else
                        p_sample->sampleRate = 11000 ;

                    p_sample->isUnsigned = TRUE;
                }

                p_sample->pan = _PAN_CENTER ;
                p_sample->volume = (T_byte8)G_soundVolume;
                p_sample->loop = TRUE;
                p_buffer->doneCallback = NULL ;

                // Start playing the sound
                p_sample->isPlaying = TRUE;
            }
        } else {
#ifdef COMPILE_OPTION_OUTPUT_BAD_SOUNDS
            fprintf(fileBadSounds, "sound '%s' not found\n", filename) ;  fflush(fileBadSounds) ;
            MessagePrintf("sound '%s' not found\n", filename) ;  fflush(fileBadSounds) ;
#endif
        }
    }

    DebugEnd() ;

    return bufferId ;
}

T_void SoundSetEffectsVolume(T_word16 volume)
{
#if 0
    T_word16 i ;
    T_soundBuffer *p_buffer ;
    T_word16 bufferId ;
#endif

    DebugRoutine("SoundSetEffectsVolume") ;
    DebugCheck(volume < 256) ;

    G_soundVolume = volume ;

#if 0
    /* Go through the list of all playing sounds and modify */
    /* their volume. */
    bufferId = G_playingList ;
    while (bufferId != BUFFER_ID_BAD)  {
        DebugCheck(bufferId >= 0) ;
        DebugCheck(bufferId < MAX_SOUND_CHANNELS) ;
        p_buffer = G_soundBufferArray + bufferId ;

        vol = (p_buffer->volume * volume);
        p_buffer->volume = vol>>7 ;
        p_buffer->sample->volume = p_buffer->volume;

        bufferId = p_buffer->next ;
    }
#endif

    DebugEnd() ;
}

T_void SoundUpdate(T_void)
{
    T_word16 bufferId ;
    T_soundBuffer *p_buffer ;
    T_sword16 nextId ;

    DebugRoutine("SoundUpdate") ;

    if (SoundIsOn())  {
        /* Free all buffers that are complete. */
        bufferId = G_playingList ;
        while (bufferId != BUFFER_ID_BAD)  {
            DebugCheck(bufferId >= 0) ;
            DebugCheck(bufferId < MAX_SOUND_CHANNELS) ;
            p_buffer = G_soundBufferArray + bufferId ;
            nextId = p_buffer->next ;

            /* See if any sounds are done. */
            if (!p_buffer->sample->isPlaying)  {
                /* Do the callback first, then free. */
                if (p_buffer->doneCallback != NULL)  {
                    /* call the done callback reporting the */
                    /* sound complete. */
                    p_buffer->doneCallback(p_buffer->doneCallbackData) ;
                    p_buffer->doneCallback = NULL ;
                }
                /* If so, close out the buffer. */
                IFreeBuffer(bufferId) ;
            }
            bufferId = nextId ;
        }
    }

    DebugEnd() ;
}

E_Boolean SoundIsDone(T_word16 bufferId)
{
    E_Boolean isDone ;
    T_soundBuffer *p_buffer ;

    DebugRoutine("SoundIsDone") ;
    DebugCheck(bufferId != BUFFER_ID_BAD) ;
    DebugCheck(bufferId >= 0) ;
    DebugCheck(bufferId < MAX_SOUND_CHANNELS) ;

    /* Get a buffer entry. */
    p_buffer = G_soundBufferArray + bufferId ;
    DebugCheck(p_buffer->isAllocated == TRUE) ;

    if (p_buffer->sample->isPlaying)  {
        isDone = FALSE ;
    } else {
        isDone = TRUE ;
    }

    DebugEnd() ;

    return isDone ;
}

T_void SoundSetVolume(T_word16 bufferId, T_word16 volume)
{
    T_soundBuffer *p_buffer ;

    DebugRoutine("SoundSetVolume") ;

    if (SoundIsOn())  {
        DebugCheck(bufferId != BUFFER_ID_BAD) ;
        DebugCheck(bufferId >= 0) ;
        DebugCheck(bufferId < MAX_SOUND_CHANNELS) ;

        /* Get a buffer entry. */
        p_buffer = G_soundBufferArray + bufferId ;
        DebugCheck(p_buffer->isAllocated == TRUE) ;

        p_buffer->sample->volume = (T_byte8)volume;
    }

    DebugEnd() ;
}

T_void SoundSetStereoPanLocation(T_word16 bufferId, T_word16 panLocation)
{
    T_soundBuffer *p_buffer ;

    DebugRoutine("SoundSetStereoPanLocation") ;

    if (SoundIsOn())  {
        DebugCheck(bufferId != BUFFER_ID_BAD) ;
        DebugCheck(bufferId >= 0) ;
        DebugCheck(bufferId < MAX_SOUND_CHANNELS) ;

        /* Get a buffer entry. */
        p_buffer = G_soundBufferArray + bufferId ;
        DebugCheck(p_buffer->isAllocated == TRUE) ;

        p_buffer->sample->pan = panLocation;
    }

    DebugEnd() ;
}

T_void SoundStop(T_word16 bufferId)
{
    T_soundBuffer *p_buffer ;

    DebugRoutine("SoundStop") ;

    DebugCheck(bufferId != BUFFER_ID_BAD) ;
    DebugCheck(bufferId >= 0) ;
    DebugCheck(bufferId < MAX_SOUND_CHANNELS) ;

    if (SoundIsOn())  {
        p_buffer = G_soundBufferArray + bufferId ;
        DebugCheck(p_buffer->isAllocated == TRUE) ;

        p_buffer->sample->isPlaying = FALSE;
    }

    DebugEnd() ;
}

T_void SoundUpdateOften(T_void)
{
    // TODO: Music feed
}

T_void SoundStopBackgroundMusic(T_void)
{
    DebugRoutine("SoundStopBackgroundMusic");
    if (G_backgroundMusicID != BUFFER_ID_BAD) {
        SoundStop(G_backgroundMusicID);
        G_backgroundMusic = 0;
        G_backgroundMusicID = BUFFER_ID_BAD;
    }
    DebugEnd();
}

T_sword16 SoundPlayByNameWithDetails(
              T_byte8 *filename,
              T_word16 volume,
              T_word16 frequency,
              T_word16 bits,
              E_Boolean isStereo)
{
    T_resource sound ;
    T_SDLSoundBuffer *p_sample;
    T_word16 bufferId = BUFFER_ID_BAD ;
    T_soundBuffer *p_buffer ;
    // TODO: Don't do isStereo!

    DebugRoutine("PlaySoundByNameWithDetails") ;

    if (G_soundsInit == TRUE)  {
        sound = ResourceFind(G_soundsFile, filename) ;
        if (sound != RESOURCE_BAD)  {
            bufferId = IAllocateBuffer(sound) ;
            if (bufferId != BUFFER_ID_BAD)  {
                DebugCheck(bufferId < MAX_SOUND_CHANNELS) ;
                DebugCheck(bufferId >= 0) ;
                p_buffer= G_soundBufferArray + bufferId ;
                p_sample = p_buffer->sample ;
                DebugCheck(p_sample != NULL) ;

                memset(p_sample, 0, sizeof(*p_sample)) ;
                p_sample->inUse = TRUE; // we are about to use this one
                p_sample->data = p_buffer->p_sample;
                p_sample->length = p_buffer->size;

                if (bits == 16)  {
                    // 16-bit samples have half as many samples
                    p_sample->length /= 2;
                    p_sample->is16Bit = TRUE ;
                    p_sample->isUnsigned = FALSE;
                } else {
                    p_sample->is16Bit = FALSE ;
                    p_sample->isUnsigned = TRUE;
                }

                p_sample->sampleRate = frequency ;
                p_sample->pan = _PAN_CENTER ;
                p_sample->volume = (T_byte8)volume;
                p_sample->loop = TRUE;
                p_buffer->doneCallback = NULL ;

                // Start playing the sound
                p_sample->isPlaying = TRUE;
            }
        } else {
#ifdef COMPILE_OPTION_OUTPUT_BAD_SOUNDS
            fprintf(fileBadSounds, "sound '%s' not found\n", filename) ;  fflush(fileBadSounds) ;
            MessagePrintf("sound '%s' not found\n", filename) ;  fflush(fileBadSounds) ;
#endif
        }

    }

    DebugEnd() ;

    return bufferId ;
}

#endif

/*-------------------------------------------------------------------------*
 * Routine:  IAllocateBuffer
 *-------------------------------------------------------------------------*/
/**
 *  IAllocateBuffer quickly finds a free sound buffer and returns it.
 *
 *  @return index to buffer, or -1 for none.
 *
 *<!-----------------------------------------------------------------------*/
static T_sword16 IAllocateBuffer(T_resource res)
{
    T_word16 bufferId ;
    T_soundBuffer *p_buffer ;

    DebugRoutine("IAllocateBuffer") ;
    /* Pull one off the front. */
    bufferId = G_firstFreeBuffer ;

    /* Is there any? */
    if (bufferId != BUFFER_ID_BAD)  {
        DebugCheck(bufferId >= 0) ;
        DebugCheck(bufferId < MAX_SOUND_CHANNELS) ;

        /* Fix up the next free buffer. */
        p_buffer = G_soundBufferArray + bufferId ;
        DebugCheck(p_buffer->isAllocated == FALSE) ;
        G_firstFreeBuffer = p_buffer->next ;
        if (G_firstFreeBuffer != BUFFER_ID_BAD)
            G_soundBufferArray[G_firstFreeBuffer].prev = BUFFER_ID_BAD ;

        /* Put the new item on the play list. */
        p_buffer->next = G_playingList ;
        p_buffer->prev = BUFFER_ID_BAD ;
        if (G_playingList != BUFFER_ID_BAD)
            G_soundBufferArray[G_playingList].prev = bufferId ;
        G_playingList = bufferId ;

        p_buffer->resource = res ;
        p_buffer->p_sample = ResourceLock(res) ;
        p_buffer->size = ResourceGetSize(res) ;

        G_numSoundsPlaying++ ;
        p_buffer->isAllocated = TRUE ;

        ICheckLists() ;
    }

    DebugEnd() ;

    return bufferId ;
}

static T_sword16 IAllocateBufferDirect(void *aRawSound, T_word32 aSize)
{
    T_word16 bufferId ;
    T_soundBuffer *p_buffer ;

    DebugRoutine("IAllocateBufferDirect") ;
    /* Pull one off the front. */
    bufferId = G_firstFreeBuffer ;

    /* Is there any? */
    if (bufferId != BUFFER_ID_BAD)  {
        DebugCheck(bufferId >= 0) ;
        DebugCheck(bufferId < MAX_SOUND_CHANNELS) ;

        /* Fix up the next free buffer. */
        p_buffer = G_soundBufferArray + bufferId ;
        DebugCheck(p_buffer->isAllocated == FALSE) ;
        G_firstFreeBuffer = p_buffer->next ;
        if (G_firstFreeBuffer != BUFFER_ID_BAD)
            G_soundBufferArray[G_firstFreeBuffer].prev = BUFFER_ID_BAD ;

        /* Put the new item on the play list. */
        p_buffer->next = G_playingList ;
        p_buffer->prev = BUFFER_ID_BAD ;
        if (G_playingList != BUFFER_ID_BAD)
            G_soundBufferArray[G_playingList].prev = bufferId ;
        G_playingList = bufferId ;

        p_buffer->resource = RESOURCE_BAD;
        p_buffer->p_sample = aRawSound;
        p_buffer->size = aSize;
        p_buffer->isAllocated = TRUE ;
        G_numSoundsPlaying++ ;

        ICheckLists() ;
    }

    DebugEnd() ;

    return bufferId ;
}

/*-------------------------------------------------------------------------*
 * Routine:  IFreeBuffer
 *-------------------------------------------------------------------------*/
/**
 *  IFreeBuffer quickly frees a previously allocated buffer.
 *
 *  @return index to buffer, or -1 for none.
 *
 *<!-----------------------------------------------------------------------*/
static T_void IFreeBuffer(T_word16 bufferId)
{
    T_soundBuffer *p_buffer ;

    DebugRoutine("IFreeBuffer") ;
    DebugCheck(bufferId >= 0) ;
    DebugCheck(bufferId < MAX_SOUND_CHANNELS) ;
    p_buffer = G_soundBufferArray + bufferId ;
    DebugCheck(p_buffer->isAllocated == TRUE) ;

    if (p_buffer->resource != RESOURCE_BAD) {
        ResourceUnlock(p_buffer->resource) ;
        ResourceUnfind(p_buffer->resource) ;
        p_buffer->resource = RESOURCE_BAD ;
    }
    p_buffer->isAllocated = FALSE ;

    /* Take the buffer off the playing list. */
    if (p_buffer->prev != BUFFER_ID_BAD)  {
        G_soundBufferArray[p_buffer->prev].next = p_buffer->next ;
    } else {
        G_playingList = p_buffer->next ;
    }
    if (p_buffer->next != BUFFER_ID_BAD)  {
        G_soundBufferArray[p_buffer->next].prev = p_buffer->prev ;
    }

    /* Put the buffer on the free list. */
    p_buffer->next = G_firstFreeBuffer ;
    p_buffer->prev = BUFFER_ID_BAD ;
    if (G_firstFreeBuffer != BUFFER_ID_BAD)
        G_soundBufferArray[G_firstFreeBuffer].prev = bufferId ;
    G_firstFreeBuffer = bufferId ;

    G_numSoundsPlaying-- ;

    ICheckLists() ;

    DebugEnd() ;
}

#ifdef COMPILE_OPTION_SOUND_CHECK_LIST
static T_void ICheckLists(T_void)
{
    T_soundBuffer *p_buffer ;
    T_word16 bufferId ;

    DebugRoutine("ICheckLists") ;

    printf("Buffers free:\n") ;
    bufferId = G_firstFreeBuffer ;
    while (bufferId != BUFFER_ID_BAD)  {
        DebugCheck(bufferId != BUFFER_ID_BAD) ;
        DebugCheck(bufferId >= 0) ;
        DebugCheck(bufferId < MAX_SOUND_CHANNELS) ;
        p_buffer = G_soundBufferArray + bufferId ;

        printf("  %d (%d %d %d)\n",
            bufferId,
            p_buffer->isAllocated,
            p_buffer->prev,
            p_buffer->next) ;
        fflush(stdout) ;

        bufferId = p_buffer->next ;
    }

    printf("Buffers allocated:\n") ;
    bufferId = G_playingList ;
    while (bufferId != BUFFER_ID_BAD)  {
        DebugCheck(bufferId != BUFFER_ID_BAD) ;
        DebugCheck(bufferId >= 0) ;
        DebugCheck(bufferId < MAX_SOUND_CHANNELS) ;
        p_buffer = G_soundBufferArray + bufferId ;

        printf("  %d (%d %d %d)\n",
            bufferId,
            p_buffer->isAllocated,
            p_buffer->prev,
            p_buffer->next) ;
        fflush(stdout) ;

        bufferId = p_buffer->next ;
    }

    DebugEnd() ;
}

#endif

/* LES: 04/08/96 */
T_sword16 SoundPlayByNumberWithCallback(
              T_word16 num,
              T_word16 volume,
              T_soundDoneCallback callback,
              T_void *p_data)
{
    T_soundBuffer *p_buffer ;
    T_word16 bufferId ;

    DebugRoutine("SoundPlayByNumberWithCallback") ;

    bufferId = SoundPlayByNumber(num, volume) ;
    if (bufferId != BUFFER_ID_BAD)  {
        DebugCheck(bufferId != BUFFER_ID_BAD) ;
        DebugCheck(bufferId >= 0) ;
        DebugCheck(bufferId < MAX_SOUND_CHANNELS) ;

        p_buffer = G_soundBufferArray + bufferId ;
        DebugCheck(p_buffer->isAllocated == TRUE) ;

        p_buffer->doneCallback = callback ;
        p_buffer->doneCallbackData = p_data ;
    }

    DebugEnd() ;

    return bufferId ;
}

/* LES: 04/08/96 */
T_sword16 SoundPlayLoopByNumberWithCallback(
              T_word16 num,
              T_word16 volume,
              T_soundDoneCallback callback,
              T_void *p_data)
{
    T_soundBuffer *p_buffer ;
    T_word16 bufferId ;

    DebugRoutine("SoundPlayLoopByNumberWithCallback") ;

    bufferId = SoundPlayLoopByNumber(num, volume) ;
    if (bufferId != BUFFER_ID_BAD)  {
        DebugCheck(bufferId != BUFFER_ID_BAD) ;
        DebugCheck(bufferId >= 0) ;
        DebugCheck(bufferId < MAX_SOUND_CHANNELS) ;

        p_buffer = G_soundBufferArray + bufferId ;
        DebugCheck(p_buffer->isAllocated == TRUE) ;

        p_buffer->doneCallback = callback ;
        p_buffer->doneCallbackData = p_data ;
    }

    DebugEnd() ;

    return bufferId ;
}

/*-------------------------------------------------------------------------*
 * Routine:  SoundGetEffectsVolume
 *-------------------------------------------------------------------------*/
/**
 *  SoundGetEffectsVolume gets the volume of the sound effects.
 *
 *  @return Level of sound
 *
 *<!-----------------------------------------------------------------------*/
T_word16 SoundGetEffectsVolume(T_void)
{
    T_word16 volume ;

    DebugRoutine("SoundGetEffectsVolume") ;

    volume = G_soundVolume ;

    DebugCheck(volume < 256) ;
    DebugEnd() ;

    return volume ;
}

/*-------------------------------------------------------------------------*
 * Routine:  SoundStopAllSounds
 *-------------------------------------------------------------------------*/
/**
 *  SoundStopAllSounds turns off all the sounds being played in digital
 *  form.  The music is not affected.
 *
 *<!-----------------------------------------------------------------------*/
T_void SoundStopAllSounds(T_void)
{
    T_word16 bufferId ;
    T_soundBuffer *p_buffer ;

    DebugRoutine("SoundStopAllSounds") ;

    if (SoundIsOn())  {
        /* Free all buffers that are complete. */
        bufferId = G_playingList ;
        while (bufferId != BUFFER_ID_BAD)  {
            DebugCheck(bufferId >= 0) ;
            DebugCheck(bufferId < MAX_SOUND_CHANNELS) ;
            p_buffer = G_soundBufferArray + bufferId ;
#ifdef WIN32
            if (!p_buffer->sample->isMusic)
                SoundStop(bufferId) ;
#endif
#ifdef USE_SOS_LIBRARY
            SoundStop(bufferId) ;
#endif
            bufferId = p_buffer->next ;
        }
    }

    /* Update the resource manager to tell */
    /* that the sounds are now freed. */
    SoundUpdate() ;

    DebugEnd() ;
}

/* LES 07/31/96 Created */
E_Boolean SoundGetAllowFreqShift(T_void)
{
    return G_allowFreqShift ;
}

/* LES 07/31/96 Created */
T_void SoundSetAllowFreqShift(E_Boolean newAllow)
{
    G_allowFreqShift = newAllow ;
}

/*-------------------------------------------------------------------------*
 * Routine:  SoundPlayByNumberWithDetails
 *-------------------------------------------------------------------------*/
/**
 *
 *<!-----------------------------------------------------------------------*/
T_sword16 SoundPlayByNumberWithDetails(
             T_word16 num,
             T_word16 volume,
             T_word16 frequency,
             T_word16 bits,
             E_Boolean isStereo)
{
    char buffer[20] ;
    T_word16 bufferId = BUFFER_ID_BAD ;

    DebugRoutine("PlaySoundByNumberWithDetails") ;

    /* Don't play any sound unless the sound system was initialized. */
    if (G_soundsInit == TRUE)  {
	    sprintf(buffer, "snd#%d", (T_word32)num) ;
	    bufferId = SoundPlayByNameWithDetails(
                       buffer,
                       volume,
                       frequency,
                       bits,
                       isStereo) ;
    }

    DebugEnd() ;

    return bufferId ;
}

/*-------------------------------------------------------------------------*
 * Routine:  SoundIsOn
 *-------------------------------------------------------------------------*/
/**
 *  SoundIsOn checks to see if the sound system is enabled and returns
 *  this fact.
 *
 *  @return TRUE if sound is enabled, FALSE if not
 *
 *<!-----------------------------------------------------------------------*/
E_Boolean SoundIsOn(T_void)
{
    E_Boolean ret ;
    DebugRoutine("SoundIsOn") ;

    ret = G_soundsInit ;

    DebugCheck(ret < BOOLEAN_UNKNOWN) ;
    DebugEnd() ;

    return ret ;
}


/** @} */
/*-------------------------------------------------------------------------*
 * End of File:  SOUND.C
 *-------------------------------------------------------------------------*/
