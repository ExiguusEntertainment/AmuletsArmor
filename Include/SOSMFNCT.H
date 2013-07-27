/****************************************************************************

   File              : sosmfnct.h

   Programmer(s)     : Don Fowler

   Date              : 5/5/95

   Purpose           : Function prototypes for the MIDI system

   Last Updated      : 5/5/95

****************************************************************************
               Copyright(c) 1992,1995 Human Machine Interfaces 
                            All Rights Reserved
****************************************************************************/

#ifndef  _SOS_MIDI_FUNCTIONS_DEFINED   
#define  _SOS_MIDI_FUNCTIONS_DEFINED   

BOOL     sosMIDIBranchSong             ( PSONG, W32 );
BOOL     sosMIDIBranchTrack            ( PTRACK, W32 );
W32     sosMIDIBranchToSongID         ( HANDLE, BYTE );
W32     sosMIDIBranchToTrackID        ( HANDLE, W32, BYTE );
W32     sosMIDIRegisterLoopFunction   ( HANDLE, W32 (*)( HANDLE, BYTE, BYTE ) );
W32     sosMIDIRegisterBranchFunction ( HANDLE, W32 (*)( HANDLE, BYTE, BYTE ) );
W32     sosMIDIRegisterTriggerFunction( HANDLE, W32 (*)( HANDLE, BYTE, BYTE ) );
W32     sosMIDISetDriverVolume        ( HANDLE, W32 );
BOOL     sosMIDIAcquireChannel         ( PTRACK );
VOID     sosMIDIReleaseChannel         ( PTRACK );
VOID     sosMIDISendChannelInfo        ( PTRACK );
VOID     sosMIDIUpdateChannel          ( PTRACK );
W32     sosMIDIResetChannelStealing   ( HANDLE );
W32     sosMIDIGetDeltaTime           ( PSTR, DWORD * );
BOOL     sosMIDIProcessEvent           ( PSONG, PTRACK, W32, W32, W32,
                                         W32, BYTE );
BOOL     sosMIDIProcessSongEvents      ( PSONG );
W32     sosMIDIInitDriver             ( PSOSMIDIDRIVER, HANDLE * );
W32     sosMIDIUnInitDriver           ( HANDLE, BOOL );
W32     sosMIDIResetDriver            ( HANDLE );
BOOL     sosMIDILockLibrary            ( VOID );
BOOL     sosMIDIUnLockLibrary          ( VOID );
BOOL     sosMIDIMapTrack               ( PTRACK );
W32     sosMIDIMuteSong               ( HANDLE );
W32     sosMIDIUnMuteSong             ( HANDLE );
W32     sosMIDIPauseSong              ( HANDLE, W32 );
W32     sosMIDIResumeSong             ( HANDLE );
W32     sosMIDIStartSong              ( HANDLE );
W32     sosMIDIStopSong               ( HANDLE );
BOOL     sosMIDISongDone               ( HANDLE );
W32     sosMIDISendMIDIData           ( HANDLE, LPSTR, DWORD );
W32     sosMIDIInitSong               ( PSOSMIDISONG, HANDLE * );
W32     sosMIDIUnInitSong             ( HANDLE );
W32     sosMIDIResetSong              ( HANDLE );
VOID     sosMIDIProcessSong            ( VOID );
VOID     sosMIDIAddSong                ( PSONG );
W32     sosMIDIInitSystem             ( PSTR, W32 );
W32     sosMIDIUnInitSystem           ( VOID );
W32     sosMIDISongAlterTempo         ( HANDLE, W32 );
BOOL     sosMIDIInitTrack              ( PSONG, PTRACK );
VOID     sosMIDIStealTrackListInsert   ( PTRACK );
PTRACK   sosMIDIStealTrackListRetreive ( VOID );
PTRACK   sosMIDIStealTrackListRemove   ( PTRACK );
VOID     sosMIDIAddTrack               ( PSONG, PTRACK );
W32     sosMIDISetSongVolume          ( HANDLE, W32 );
W32     sosMIDIFadeSong               ( HANDLE, W32, W32, W32, W32, W32 );
W32     sosMIDISetMasterVolume        ( W32 );
W32     sosMIDISetInsData             ( HANDLE, LPSTR, W32 );
W32     sosMIDISetMT32InsData         ( HANDLE, LPSTR, W32 );
VOID     sosMIDIMT32Timer              ( VOID );
W32     sosMIDILoadDriver             ( W32, HANDLE, LPSTR *, LPSTR *, PSTR,
                                         PSTR, DWORD * );
W32     sosMIDIUnLoadDriver           ( HANDLE );
W32     sosMIDISendMIDIEvent          ( HANDLE, W32, W32, W32, W32 );
W32     sosMIDIInitDependencyList     ( PSONG, PSTR * );

W32     sosMIDIGetSongNotesOn         ( HANDLE );
W32     sosMIDIGetSongLocation        ( HANDLE, W32 *, W32 *, W32 * );
W32     sosMIDIGetTrackLocation       ( HANDLE, W32, W32 *, W32 *, W32 * );
W32     sosMIDICalculateLocation      ( PSONG, DWORD, W32 *, W32 *, W32 * );

W32 cdecl far sosCBCKSendData         ( LPSTR, W32, W32 );
W32 cdecl far sosCBCKInit             ( LPSTR, W32, W32 );
W32 cdecl far sosCBCKUnInit           ( LPSTR, W32, W32 );
W32 cdecl far sosCBCKReset            ( LPSTR, W32, W32 );
W32 cdecl far sosCBCKSetInstrumentData( LPSTR, W32, W32 );

W32 cdecl far digiSendData             (  LPSTR, W32, W32  );
W32 cdecl far digiInit                 (  LPSTR, W32, W32  );
W32 cdecl far digiUnInit               (  LPSTR, W32, W32  );
W32 cdecl far digiReset                (  LPSTR, W32, W32  );
W32 cdecl far digiSetInstrumentData    (  LPSTR, W32, W32  );

W32 cdecl far waveSendData             (  LPSTR, W32, W32  );
W32 cdecl far waveInit                 (  LPSTR, W32, W32  );
W32 cdecl far waveUnInit               (  LPSTR, W32, W32  );
W32 cdecl far waveReset                (  LPSTR, W32, W32  );
W32 cdecl far waveSetInstrumentData    (  LPSTR, W32, W32  );
VOID cdecl far waveSampleCallback       (  W32, W32, W32  );

VOID        digiQueueInit           (  W32, W32  );
VOID        digiQueueUnInit         (  W32  );
W32        digiQueueAddItem        (  W32, W32, W32, W32, W32 );
W32        digiQueueGetItem        (  W32, W32  );
W32        digiQueueGetItemWAVE    (  W32  );
W32        digiQueueDeleteItem     (  W32, W32  );
W32        digiQueueDeleteItemWAVE (  W32, W32  );
W32        digiQueueDeleteItemMIDI (  W32, W32, W32  );
W32        digiQueueFindItemMIDI   (  W32, W32, W32  );

// function prototypes
VOID cdecl digiSampleCallback(  W32, W32, W32  );

// function prototypes for memory locking
VOID sosMIDICode1Start( VOID );
VOID sosMIDICode2Start( VOID );
VOID sosMIDICode3Start( VOID );
VOID sosMIDICode4Start( VOID );
VOID sosMIDICode5Start( VOID );
VOID sosMIDICode6Start( VOID );
VOID sosMIDICode7Start( VOID );
VOID sosMIDICode8Start( VOID );
VOID sosMIDICode9Start( VOID );
VOID sosMIDICode10Start( VOID );
VOID sosMIDICode11Start( VOID );
VOID sosMIDICode12Start( VOID );
VOID sosMIDICode13Start( VOID );
VOID sosMIDICode14Start( VOID );
VOID sosMIDICode15Start( VOID );
VOID sosMIDICode16Start( VOID );
VOID sosMIDICode17Start( VOID );
VOID sosMIDICode18Start( VOID );
VOID sosMIDICode19Start( VOID );
VOID sosMIDICode20Start( VOID );
VOID sosMIDICode21Start( VOID );
VOID sosMIDICode22Start( VOID );
VOID sosMIDICode1End( VOID );
VOID sosMIDICode2End( VOID );
VOID sosMIDICode3End( VOID );
VOID sosMIDICode4End( VOID );
VOID sosMIDICode5End( VOID );
VOID sosMIDICode6End( VOID );
VOID sosMIDICode7End( VOID );
VOID sosMIDICode8End( VOID );
VOID sosMIDICode9End( VOID );
VOID sosMIDICode10End( VOID );
VOID sosMIDICode11End( VOID );
VOID sosMIDICode12End( VOID );
VOID sosMIDICode13End( VOID );
VOID sosMIDICode14End( VOID );
VOID sosMIDICode15End( VOID );
VOID sosMIDICode16End( VOID );
VOID sosMIDICode17End( VOID );
VOID sosMIDICode18End( VOID );
VOID sosMIDICode19End( VOID );
VOID sosMIDICode20End( VOID );
VOID sosMIDICode21End( VOID );
VOID sosMIDICode22End( VOID );

#ifdef __cplusplus
extern "C" {
#endif

extern   void  cdecl sosMIDIDRVGetFuncsPtr( LPSTR, LPSTR, LPSTR );
extern   LPSTR cdecl sosMIDIDRVSpecialFunction( LPSTR, LPSTR, W32 );
extern   W32 cdecl xgetES( void );
extern   VOID cdecl sosMIDICode23Start( VOID );
extern   VOID cdecl sosMIDICode23End( VOID );

#ifdef __cplusplus
}
#endif 

#endif
