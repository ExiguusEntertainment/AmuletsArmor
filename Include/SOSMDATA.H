/****************************************************************************

   File              : sosmdata.h

   Programmer(s)     : Don Fowler

   Date              : 5/5/95

   Purpose           : Global data external definitions for the MIDI system

   Last Updated      : 5/5/95

****************************************************************************
               Copyright(c) 1992,1995 Human Machine Interfaces 
                            All Rights Reserved
****************************************************************************/

#ifndef  _SOS_MIDI_DATA_DEFINED   
#define  _SOS_MIDI_DATA_DEFINED   

#include "sosm.h"

extern _MIDI_SYSTEM  _sMIDISystem;
extern _MIDI_DRIVER  _sMIDIDriver[];
extern W32 ( cdecl far * _lpMIDICBCKFunctions[] )( LPSTR, W32, W32 );
extern W32 ( cdecl far * _lpMIDIDIGIFunctions[] )( LPSTR, W32, W32 );
extern W32 ( cdecl far * _lpMIDIWAVEFunctions[] )( LPSTR, W32, W32 );
extern W32 _wMIDIDriverTotalChannels[];
extern W32 _wMIDIDriverChannel[ _MIDI_DEFINED_DRIVERS ][ _MIDI_MAX_CHANNELS ];
extern VOID ( cdecl far * _lpMIDICBCKFunction )( LPSTR, W32, W32 );

// data for the MIDIDIGI system
extern _MIDI_DIGI_CHANNEL _sMIDIDIGIChannel[];
extern _MIDI_DIGI_QUEUE_ELEMENT _sMIDIDIGIQueue[][ _MAX_VOICES ];
extern W32 _wMIDIDIGISampleQueueHead[];
extern W32 _wMIDIDIGISampleQueueTail[];
extern W32 _wMIDIDIGIMaxSamples[];
extern W32 _wMIDIDIGIUsedSamples[];
extern _SOS_SAMPLE * _sMIDIDIGIStartSample[][ _MIDI_DIGI_MAX_INS ];
extern LPMIDIDIGIINSFILEHEADER _lpMIDIDIGIInsFileHeader[];
extern W32 _wMIDIDIGIDriverHandle[];
extern W32 _wMIDIDIGIDriverInitialized[];
extern W32 _wMIDIDIGITimerEventHandle[];
extern W32 _wMIDIDIGIDriverInitMIDI[];
extern W32 _wMIDIDIGIVelocitySensing[];
extern BYTE _szMIDIDIGIInsFileID[];
extern W32 _wSOSDIGIMIDIHandle[];

extern W32 _wCData1Start;
extern W32 _wCData1End;

#endif
