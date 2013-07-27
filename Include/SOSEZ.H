/****************************************************************************
*
*  File              : sosez.h
*  Date Created      : 2/27/95
*  Description       : 
*
*  Programmer(s)     : Nick Skrepetos
*  Last Modification : 2/27/95 - 9:0:32 PM
*  Additional Notes  :
*
*****************************************************************************
*            Copyright (c) 1994-5,  HMI, Inc.  All Rights Reserved          *
****************************************************************************/

#ifndef  _SOSEZ_DEFINED
#define  _SOSEZ_DEFINED

// indicate not to use driver
#define  _SOSEZ_NO_DRIVER     -1

// rate to register the digital mixer
#define  _SOSEZ_TIMER_RATE    200
// size of the system DMA buffer
#define  _SOSEZ_DMA_BUFFER    0x1000

// sample rate to run the driver
#define  _SOSEZ_SAMPLE_RATE   22050

// sosEZ error codes
enum
   {
      _SOSEZ_NO_ERROR,
      _SOSEZ_DIGI_INIT,
      _SOSEZ_MIDI_INIT,
      _SOSEZ_PATCH_MELODIC,
      _SOSEZ_PATCH_DRUM,
      _SOSEZ_PATCH_INIT

   };

#endif

// function prototypes for sosEZ
W32  cdecl sosEZInitSystem               ( W32 wDDeviceID, W32 wMDeviceID );
W32  cdecl sosEZUnInitSystem             ( VOID );
BOOL  cdecl sosEZGetConfig                ( PSTR szName );
_SOS_SAMPLE * cdecl sosEZLoadSample ( PSTR szName );
PSTR  cdecl sosEZLoadPatch                ( PSTR szName );
W32  cdecl sosEZLoadSong                 ( PSTR szName );


// pack the structure on a byte alignment
#pragma pack( 1 )

// .WAV file header structure
typedef struct _tag_wavheader  
{
   BYTE szRIFF[ 4 ];
   DWORD dwFormatLength;
   BYTE szWAVE[ 4 ];
   BYTE szFMT[ 4 ];
   DWORD dwWaveFormatLength;
   short wFormatTag;
   short wChannels;
   DWORD dwSamplesPerSec;
   DWORD dwAvgBytesPerSec;
   short wBlockAlign;
   short wBitsPerSample;
   BYTE szDATA[ 4 ];
   DWORD dwDataLength;

} _WAVHEADER;

#pragma pack()

