/****************************************************************************

   File              : sosm.h

   Programmer(s)     : Don Fowler

   Date              : 5/5/95

   Purpose           : Include Files For MIDI System

   Last Updated      : 5/5/95

****************************************************************************
               Copyright(c) 1992,1995 Human Machine Interfaces 
                            All Rights Reserved
****************************************************************************/

#ifndef  _SOS_MIDI_DEFINED   
#define  _SOS_MIDI_DEFINED   

#include < stdlib.h >
#include "sosdefs.h"
#include "sos.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _DEBUG
// macro to display debug information 
#define  _DBG( pString, x1, x2, x3, x4 )  {  printf( pString, x1, x2, x3, x4 );  }
#else
// macro to display debug information 
#define  _DBG( pString, x1, x2, x3, x4 )  { }
#endif

// define to flag a track in the steal list
#define  _MIDI_TRACK_IN_STEAL_LIST  0x0020       

// define for the lowest priority
#define  _MIDI_STEAL_LOW_PRIORITY   0x0009
    
// defines for the loadable driver ids
#define  _MIDI_SOUND_MASTER_II      0xa000
#define  _MIDI_MPU_401              0xa001
#define  _MIDI_FM                   0xa002
#define  _MIDI_OPL2                 0xa002
#define  _MIDI_CALLBACK             0xa003
#define  _MIDI_MT_32                0xa004
#define  _MIDI_DIGI                 0xa005  
#define  _MIDI_INTERNAL_SPEAKER     0xa006
#define  _MIDI_WAVE_TABLE_SYNTH     0xa007  
#define  _MIDI_AWE32                0xa008  
#define  _MIDI_OPL3                 0xa009  
#define  _MIDI_GUS                  0xa00a  

// define for the name of the MIDI driver file
#define  _MIDI_DRIVER_FILE          "HMIMDRV.386"

// maximum number of instruments that can be set in the 
// digital driver
#define  _MIDI_DIGI_MAX_INS         128
#define  _MIDI_MAX_INS         128

// call indexes for the loadable drivers
enum
{ 
   _DRV_MIDI_GET_CAPS,
   _DRV_MIDI_GET_CALL_TABLE,
   _DRV_MIDI_SPECIAL1
};

// define for the channel used by drums
#define  _MIDI_DRUM_CHANNEL            0x09

// define for identifying a drum instrument in the MIDIDIGI system 
#define  _SOS_MIDI_DRUM_INS   ( short )0x8000

// define for the total number of devices that are available
#define  _MIDI_DEFINED_DRIVERS            11

// define for the number of functions each MIDI driver can have
#define  _MIDI_DRV_FUNCTIONS              30

// flags for pausing a song
#define  _MUTE                            0x0001

// flag to indicate that an element has been initialized
#define  _INITIALIZED                     0x1000

// flag to indicate that the pointers have been normalized
#define  _NORMALIZED                      0x4000

// flag to indicate that a driver has been loaded
#define  _LOADED                          0x0100

// define for the chunk types
#define  _MIDI_SONG_CHUNK                 "HMI-MIDISONG061595"
#define  _MIDI_TRACK_CHUNK                "HMI-MIDITRACK"

// define for the mask to use to mask an event off of the channel/event
#define  _MIDI_EVENT_MASK                 0xf0

// define for the maximum and initial volume for the system
#define  _MIDI_MAX_VOLUME                 0x7f
                                              
// bit mask to OR a branch ID with to convert to a global branch ID
#define  _MIDI_GLOBAL_BRANCH_ID_BIT_MASK  0x80

// maximum number of songs that the system can play at any time
#define  _MIDI_MAX_SONGS                  0x20

// define for the maximum size of a chunk ID string
#define  _MIDI_MAX_CHUNK_ID_SIZE          0x20

// define for the maximum number of drivers that the MIDI system can use
#define  _MIDI_MAX_DRIVERS                0x08

// defines for the volume fading system
#define  _MIDI_FADE_IN                    0x01  
#define  _MIDI_FADE_OUT                   0x02  
#define  _MIDI_FADE_OUT_STOP              0x04  
#define  _MIDI_VOLUME_FADE                0x08

// define for the track/song flags to indicate that the process is paused
#define  _PAUSED                          0x20
  
// flag for a track that is playing, but not sending data out to the 
// driver
#define  _SUPRESSED                       0x10

// flag for a track to indicate that it has been mapped into the dependency
// list
#define  _MAPPED                          0x80

// size of a note on event in the midi stream, the size is from the status
// to the end of the data the next dword is the delta time for the note
// to stay on
#define  _NOTE_ON_SIZE                    0x02

// defines for the midi event
#define  _MIDI_NOTE_OFF                   0x80
#define  _MIDI_NOTE_ON                    0x90
#define  _MIDI_AFTERTOUCH                 0xa0
#define  _MIDI_CONTROL                    0xb0
#define  _MIDI_PROGRAM                    0xc0
#define  _MIDI_CHANNEL_PRESSURE           0xd0
#define  _MIDI_BEND                       0xe0
#define  _MIDI_SYSEX                      0xf0
#define  _MIDI_TRACK_EVENT                0xff
#define  _MIDI_END_OF_TRACK               0x2f
#define  _MIDI_HMI_EVENT                  0xfe

// define for the all notes off controller
#define  _MIDI_ALL_NOTES_OFF_CTRLR        108

// define for the all notes off controller as used by the system
#define _MIDI_ALL_NOTES_OFF_MIDI          123
   
// define for the volume controller
#define  _MIDI_VOLUME_CTRLR               7

// define for the reset controllers controller
#define  _MIDI_RESET_CONTROLLERS_CTRLR    121

// defines for the enable/disable controller restore controllers
#define  _MIDI_ENABLE_CONTROLLER_RESTORE  103
#define  _MIDI_DISABLE_CONTROLLER_RESTORE 104

// defines for the channel steal/release controllers
#define  _MIDI_STEAL_CHANNEL              105

// defines for the channel lock/unlock controllers
#define  _MIDI_LOCK_CHANNEL               106

// defines for the channel lock/unlock controllers
#define  _MIDI_SET_PRIORITY               107

// define for the trigger controller
#define  _MIDI_TRIGGER_CTRLR              119

// defines for the enable/disable all data
#define  _MIDI_ALL_CONTROLLERS            115 

// defines for non controller MIDI data that can/cannot be restored
#define  _MIDI_TURN_ALL_NOTES_OFF         103      
#define  _MIDI_CTRLR_PROGRAM_CHANGE       104
#define  _MIDI_CTRLR_PITCH_BEND           105
#define  _MIDI_CTRLR_AFTERTOUCH           106
#define  _MIDI_CTRLR_CHANNEL_PRESSURE     107
   
// defines for the song flow ids
#define  _MIDI_LOCAL_BRANCH_DATA          0x10
#define  _MIDI_LOCAL_BRANCH               0x11
#define  _MIDI_LOCAL_LOOP_COUNT           0x12
#define  _MIDI_LOCAL_LOOP_END             0x13
#define  _MIDI_GLOBAL_LOOP_COUNT          0x14
#define  _MIDI_GLOBAL_LOOP_END            0x15
#define  _MIDI_GLOBAL_BRANCH              0x16

// defines for the track and patch name ids
#define  _MIDI_SYS_TRACK_NAME             0x01   
#define  _MIDI_SYS_FMMPAT_NAME            0x02   
#define  _MIDI_SYS_FMDPAT_NAME            0x03   
#define  _MIDI_SYS_DIGPAT_NAME            0x04   
#define  _MIDI_SYS_USERPAT_NAME_BASE      0x05   
#define  _MIDI_MAX_USER_PATCHES           0x10  

// maximum size of a patch file name
#define  _MIDI_MAX_PATCH_NAME_SIZE        0x10

// define for the channel structure priority to signal that a channel cannot
// be stolen, it is locked
#define  _MIDI_CHANNEL_LOCKED             0x0400

// maximum number of channels that a driver can use
#define  _MIDI_MAX_CHANNELS               0x10

// largest priority value allowed for a track
#define  _MIDI_MAX_PRIORITY               0x09

// error return for the MIDI DIGI system
#define  _MIDI_DIGI_ERR                   -1

// enumeration for all of the driver functions
enum
{
   _MIDI_DRV_SEND_DATA,
   _MIDI_DRV_INIT,
   _MIDI_DRV_UNINIT,
   _MIDI_DRV_RESET,
   _MIDI_DRV_SET_INST_DATA
};

// the driver files are packed on a 4 byte boundary 
#pragma pack( 4 )

// forward structure declarations to appease the C++ compiler
struct _tag_midi_song;
struct _tag_midi_track;

// instrument file header structure
typedef struct
{
   // file ID
   BYTE  szID[ 32 ];

   // file version
   W32  wVersion;

   // instruments in file
   W32  wInstruments;

   // list of pointers to start sample structures
   _SOS_SAMPLE * pStartSample[ _MIDI_DIGI_MAX_INS ];

   DWORD temp1;
   DWORD temp2;

} _MIDI_WAVE_FILE_HEADER;
typedef _MIDI_WAVE_FILE_HEADER * PMIDIWAVEFILEHEADER;
typedef _MIDI_WAVE_FILE_HEADER far * LPMIDIWAVEFILEHEADER;

// structure for the hmi instrument file header
typedef struct _tag_midi_digi_ins_file_header
{
   // file id type
   BYTE szFileID[ 32 ];

   // file version
   W32 wFileVersion;

   // size of the file
   DWORD dwFileSize;

} _MIDI_DIGI_INS_FILE_HEADER;
typedef _MIDI_DIGI_INS_FILE_HEADER * PMIDIDIGIINSFILEHEADER;
typedef _MIDI_DIGI_INS_FILE_HEADER far * LPMIDIDIGIINSFILEHEADER;
 

// file header for the driver
typedef struct _tag_midi_file_header
{
   // name ID
   BYTE  szName[ 32 ];

   // number of drivers in the file
   W32  wDrivers;

   // offset of first driver
   DWORD dwOffset;

   // size of the file
   DWORD dwFileSize;

} _MIDI_FILE_HEADER;
typedef _MIDI_FILE_HEADER * PMIDIFILEHEADER;
typedef _MIDI_FILE_HEADER far * LPMIDIFILEHEADER;

// driver header structure
typedef struct _tag_midi_driver_header
{
   // name ID
   BYTE  szName[ 32 ];

   // offset of next driver
   DWORD dwNext;

   // size of current driver
   W32  wSize;

   // id for the current device
   W32  wID;

   // id for the extender type
   W32  wExtenderType;

} _MIDI_DRIVER_HEADER;
typedef _MIDI_DRIVER_HEADER * PMIDIDRIVERHEADER;
typedef _MIDI_DRIVER_HEADER far * LPMIDIDRIVERHEADER;

#pragma pack()

// pack the note structure for BYTE alignment to save memory
#pragma pack( 1 )
 
// structure for digital driver queue element
typedef struct _tag_midi_digi_queue_element
{
   // handle for the sample
   W32  wSampleHandle;

   // id for the sample
   W32  wSampleID;

   // velocity for the sample
   W32  wVelocity;

   // channel for the sample
   W32  wChannel;

} _MIDI_DIGI_QUEUE_ELEMENT;
typedef _MIDI_DIGI_QUEUE_ELEMENT * PMIDIDIGIQUEUEELEMENT;
typedef _MIDI_DIGI_QUEUE_ELEMENT far * LPMIDIDIGIQUEUEELEMENT;

// structure for digital drums to use to store midi information
typedef struct _tag_midi_digi_channel
{
   // current volume
   W32  wVolume;

   // current pan position
   W32  wPanPosition;

   // reserved
   DWORD dwReserved;
   
} _MIDI_DIGI_CHANNEL;
typedef _MIDI_DIGI_CHANNEL * PMIDIDIGICHANNEL;
typedef _MIDI_DIGI_CHANNEL far * LPMIDIDIGICHANNEL;

// device hardware information
typedef struct _tag_sos_midi_hardware
{
   // port to be used
   W32  wPort;

   // IRQ for the board
   W32  wIRQ;

   // extra parameter
   W32  wParam;

} _SOS_MIDI_HARDWARE;
typedef _SOS_MIDI_HARDWARE * PSOSMIDIHARDWARE;
typedef _SOS_MIDI_HARDWARE far * LPSOSMIDIHARDWARE;

// structure for initializing a digital driver 
typedef struct _tag_sos_midi_digi_driver 
{
   // ID for the digital driver
   W32  wDriverID;

   // timer rate to use
   W32  wTimerRate;

   // timer callback rate to use
   W32  wTimerCallbackRate;

   // max voices for the driver to use
   W32  wMaxVoices;

   // velocity sensing flag
   W32  wVelocitySensing;

   // init driver info
   _SOS_DIGI_DRIVER * pDIGIDriverInfo;

} _SOS_MIDI_DIGI_DRIVER;
typedef _SOS_MIDI_DIGI_DRIVER * PSOSMIDIDIGIDRIVER;
typedef _SOS_MIDI_DIGI_DRIVER far * LPSOSMIDIDIGIDRIVER;

// chunck header for each chunk type in a HMI file   
typedef struct _tag_hmi_header
{
   // chunk identifier string 
   BYTE  szHMIChunk[ _MIDI_MAX_CHUNK_ID_SIZE ];

   // offset of next chunk 
   DWORD dwNext;

} _HMI_HEADER;

// structure for initializing a driver
typedef struct _tag_sos_midi_driver
{  
   // ID for the driver
   W32  wID;

   // type of driver to use if using a digital driver
   W32        wDIGIID;

   // pointer to driver memory
   VOID far * lpDriverMemoryDS;
   VOID far * lpDriverMemoryCS;

   // pointer to digital driver initialization information
   PSOSMIDIDIGIDRIVER pDIGIInit;

   // miscellaneous W32 parameter for driver
   W32  wParam;

   // miscellaneous DWORD parameter for driver
   DWORD dwParam;

   // hardware structure
   _SOS_MIDI_HARDWARE sHardware;

} _SOS_MIDI_DRIVER;
typedef _SOS_MIDI_DRIVER * PSOSMIDIDRIVER;
typedef _SOS_MIDI_DRIVER far * LPSOSMIDIDRIVER;

// structure for a note in the note list
typedef struct _tag_midi_note
{
   // song that the note is associated to 
   struct _tag_midi_song * pSong;

   // track that the note is associated to
   struct _tag_midi_track * pTrack;

   // channel that the note was started on
   BYTE  bDriverAndChannel;

   // note number
   BYTE  bNote;

   // note delta time
   DWORD dwDelta;

   // pointer to the next note in the list, if the pointer is NULL
   // then the list is terminated
   struct _tag_midi_note * pNext;

} _MIDI_NOTE;
typedef _MIDI_NOTE * PNOTE;
typedef _MIDI_NOTE far * LPNOTE;

// structure for initializing a song
typedef struct _tag_sos_midi_song
{  
   // pointer to song memory
   PSTR pSong;

   // pointer to callback function for pertinent song information
   VOID ( * pfnSongCallback )( W32 );

   // temporary spacing data
   PSTR pTemp1;
   PSTR pTemp2;
   DWORD dwTemp1[ 4 ];

} _SOS_MIDI_SONG;
typedef _SOS_MIDI_SONG * PSOSMIDISONG;
typedef _SOS_MIDI_SONG far * LPSOSMIDISONG;

// structure for a drivers channel
typedef struct _tag_midi_channel
{
   // flags 
   // _ACTIVE
   // _MIDI_CHANNEL_LOCKED
   W32  wFlags;

   // number of tracks using the channel
   W32  wTracks;

   // channel number that the channel structure is relative to
   W32  wChannel;

   // current song that owns the channel, -1 if no owner
   struct _tag_midi_song * pOwnerSong;

   // current priority of owner channel, -1 if no owner
   W32  wOwnerPriority;

   // volume for the channel from the track
   W32  wVolume;

   // volume for the driver 
   W32  wDriverVolume;

   // final volume used to scale channel and drum volumes
   W32  wVolumeScalar;

   // pointer to dependency list for owner tracks
   struct _tag_midi_track * * pOwnerDependency;

} _MIDI_CHANNEL;
typedef _MIDI_CHANNEL * PCHANNEL;
typedef _MIDI_CHANNEL far * LPCHANNEL;

// structure for a driver
typedef struct _tag_midi_driver
{
   // driver flags
   W32  wFlags;

   // driver ID 
   W32  wID;

   // pointer to channel information for the driver
   PCHANNEL pChannel;

   // pointer to the function to send data to the driver
   W32 ( cdecl far * lpfnDataFunction )( LPSTR, W32, W32 );

   // array of pointers to function for the system to call a driver with
   W32 ( cdecl far * lpfnFunction[ _MIDI_DRV_FUNCTIONS ] )(
      LPSTR, W32, W32 );

   // pointer to the driver CS and DS
   LPSTR lpDS;
   LPSTR lpCS;

   // handle for the memory allocated for the driver
   HANDLE hMemory;

   // size of the driver
   W32  wSize;

   // linear address of driver
   DWORD dwLinear;

   // handle for the gravis driver timer event
   HANDLE hEvent;

   // header for the driver file used to load the driver
   _MIDI_FILE_HEADER sFileHeader;

   // header for the driver 
   _MIDI_DRIVER_HEADER sDriverHeader;

} _MIDI_DRIVER;
typedef _MIDI_DRIVER * PDRIVER;
typedef _MIDI_DRIVER far * LPDRIVER;

// structure for a logical channel used by a track
typedef struct _tag_midi_track_channel
{
   // flags 
   // _ACTIVE
   W32  wFlags;

   // last data sent for each of the following events
   BYTE  bAftertouch;

   // last data sent for each of the following controllers
   BYTE  bController[ 128 ];  

} _MIDI_TRACK_CHANNEL;
typedef _MIDI_TRACK_CHANNEL * PTRACKCHANNEL;
typedef _MIDI_TRACK_CHANNEL far * LPTRACKCHANNEL;

// structure for a tracks channel stealing information
typedef struct _tag_midi_steal_info
{
   // track priority
   short  wPriority;

   // original channel for the track
   short  wRawChannel;
  
   // remapped channel
   short  wRemapChannel;

} _MIDI_STEAL_INFO;
typedef _MIDI_STEAL_INFO * PSTEALINFO;
typedef _MIDI_STEAL_INFO far * LPSTEALINFO;

// structure for a tracks branching information
typedef struct _tag_midi_branch_info
{
   // pointer to the branch list for the track
   PSTR * pBranchList;

   // array of flags to signal which MIDI data to restore on a branch
   DWORD    dwCtrlFlags[ 4 ];

} _MIDI_BRANCH_INFO;
typedef _MIDI_BRANCH_INFO * PBRANCHINFO;
typedef _MIDI_BRANCH_INFO far * LPBRANCHINFO;

// structure for a track in a song
typedef struct _tag_midi_track
{
   // chunk string for the track "HMI-MIDITRACK"
   BYTE  szHMIChunk[ _MIDI_MAX_CHUNK_ID_SIZE ];

   // flags 
   // _ACTIVE
   // _SUPRESSED
   // _INITIALIZED
   short  wFlags;

   // bit array for melodic instruments that are used
   DWORD dwInsUsed[ 4 ];

   // bit array for drum instruments that are used
   DWORD dwDrumInsUsed[ 4 ];

   // current running status for the track
   BYTE bRunningStatus;

   // pointer to the owner song
   struct _tag_midi_song * pSong;

   // track number in song, this is required to reinsert a track that has
   // finished and is being restarted into the track list in the correct
   // order
   short wTrackIndex;

   // original track number in song, this is required for the application
   // to reference the tracks based on the original song
   short wOriginalTrackIndex; 

   // pointer to the track name data space
   PSTR pTrackNames;

   // global position in track, in delta ticks
   DWORD dwTotalTicks;
 
   // delta time until the next event on the track
   DWORD dwDelta;

   // initialization pointer to the track data
   PSTR  pDataInit;

   // pointer to the track data
   PSTR  pData;

   // driver for the track to send data out
   short hDriver;

   // number of events on track
   short wEvents;

   // branch information structure
   _MIDI_BRANCH_INFO sBranch;

   // channel stealing information
   _MIDI_STEAL_INFO  sSteal;
  
   // pointer to the channel information for the track, this is a pointer
   // so that tracks that share the same channel can access the same 
   // channel information structure, this structure is in the song
   PSTR pTrackChannel;

   // pointer to the channel information for the track, this structure is
   // allocated when the driver that is associated with the channel is
   // initialized
   PCHANNEL pDriverChannel;

   // pointer to the track/channel dependancy list. the list is a pointer
   // for each track to a list of tracks that use the same channel and
   struct _tag_midi_track * * pTrackDependencyList;

   // pointer to the first track in the dependency list
   struct _tag_midi_track * pDependentTrack;

   // pointer to the next and previous track in the list, if the pointer
   // to the next is NULL then the list is terminated, if the pointer
   // to the first is NULL then this is the first track in the list
   struct _tag_midi_track * pNext;
   struct _tag_midi_track * pPrev;

   // pointer to the next track in the tracks that need a channel
   struct _tag_midi_track * pStealNext;

} _MIDI_TRACK;
typedef _MIDI_TRACK * PTRACK;
typedef _MIDI_TRACK far * LPTRACK;  

// structure for a volume fade event
typedef struct _tag_midi_volume
{
   // flags
   // _VOLUME_FADE
   short  wFlags;

   // current volume for the song
   short  wVolume;

   // current volume for the song
   DWORD dwFadeVolume;

   // start volume for the song during a fade
   short  wStartVolume;

   // current fractional amount to adjust volume
   DWORD dwFraction;

   // direction to fade song
   // _MIDI_FADE_OUT_STOP
   // _MIDI_FADE_OUT
   // _MIDI_FADE_IN
   short  wDirection;

   // number of ticks to fade the song over
   short  wTicks;

   // number of ticks to skip between sending out data
   short  wSkip;

   // current copy of ticks to skip between sending data out 
   short  wSkipCurrent;

   // start and end volumes for the song fade
   short  wStart;
   short  wEnd;

} _MIDI_VOLUME;

// structure for a song
typedef struct _tag_midi_song
{
   // chunk string for the song "HMI-MIDISONG"
   BYTE  szHMIChunk[ _MIDI_MAX_CHUNK_ID_SIZE ];

   // flags
   // _ACTIVE
   short  wFlags;

   // pointer to names of patch files, the area contains 
   // [count] - byte, count of patch file names
   // [id] - byte, patch file ID
   // [szPatchName] - null terminated string with patch name
   PSTR pPatchNames;

   // array to store the controller indexes, the system can find the index
   // into the local tracks controller array by indexing this array with
   // the controller value. e.g. controller 7 would index this array
   // at index 7 and find the value 1 which would mean that at index
   // one into the local track controller storage structure the value
   // for controller 7 can be saved or retreived
   BYTE bCtrlrIndexes[ 128 ];

   // init song structure
   _SOS_MIDI_SONG sInit;

   // handle for the timer event used to play the song
   short hEvent;

   // handle for the song
   short hSong;
   
   // current location in song, in delta ticks 
   DWORD dwTotalTicks;

   // total number of ticks gone by in the file
   DWORD dwGlobalTicks;

   // ticks per quarter note
   short  wTicksPerQuarterNote;

   // ticks per second to play song at
   short  wPlayRate;

   // pointer to the tempo change list
   PSTR pTempoChangeList;

   // pointer to the time change list
   PSTR pTimeChangeList;

   // pointer to the note list memory, to be allocated when a song is 
   // initialized
   PNOTE pNoteList;

   // total number of notes used in song
   short wTotalNotes;

   // total number of tracks in the song  
   short wTotalTracks;

   // number of active tracks in the song
   short wActiveTracks; 

   // pointer to the list of track pointers
   PTRACK * pTrackList;

   // pointer to the first and last track in the track list, this is the
   // location to go to when a track traverse is started
   PTRACK pTrackFirst;
   PTRACK pTrackLast;

   // pointer to the channel/track dependency pointers
   PSTR pDependencyPtrs;

   // pointer to the channel/track dependency pointers
   PTRACK * pChannelTrackDependencyLists;

   // pointer to the first and last element in the note list, this is the 
   // location to go to when a search for a note is started 
   PNOTE pNoteFirst;
   PNOTE pNoteLast;

   // volume fade structure
   _MIDI_VOLUME sVolume;

   // callback function for the song
   VOID ( * pfnSongCallback )( HANDLE ); 

   // branch callback function
   W32 ( * pfnBranchCallback )( HANDLE, BYTE, BYTE );

   // loop callback function
   W32 ( * pfnLoopCallback )( HANDLE, BYTE, BYTE );

   // trigger callback function
   W32 ( * pfnTriggerCallback )( HANDLE, BYTE, BYTE ); 

   // pointer to the next song in the list, if the pointer is NULL
   // then the list is terminated
   struct _tag_midi_song * pNext;

   // temporary spacing to make structure match windows structure  
   DWORD dwTemp[ 0x10 ];
  
} _MIDI_SONG;
typedef _MIDI_SONG * PSONG;
typedef _MIDI_SONG far * LPSONG;

// structure for the global data for the MIDI system
typedef struct _tag_midi_system
{
   // flags
   W32  wFlags;

   // path to driver file
   BYTE  szPath[ _MAX_DIR ];
   BYTE  szPathTemp[ _MAX_DIR ]; 

   // flag to indicate that a track was branched
   W32  wTrackBranched;

   // pointer to the first element of the tracks to be stolen list
   PTRACK pStealList;

   // list of pointers to songs, this is where the users handle to 
   // a song comes from
   PSONG pSongList[ _MIDI_MAX_SONGS ];

   // pointer to first song in active song list
   PSONG pSongFirst;
 
   // master volume for the system
   W32  wVolume;

   // flag for MT32 instrument data settings, when this flag
   // is set it is OK to send data to the MT32
   W32  wMT32SendData;

} _MIDI_SYSTEM;
typedef _MIDI_SYSTEM * PSYSTEM;
typedef _MIDI_SYSTEM far * LPSYSTEM;
   
#pragma pack()

#ifdef __cplusplus
}
#endif 

#include "sosmfnct.h"
#include "sosmdata.h"

#endif

