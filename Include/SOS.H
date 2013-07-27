/****************************************************************************

   File              : sos.h

   Programmer(s)     : Don Fowler, Nick Skrepetos

   Date              :

   Purpose           : Master include file for the SOS library 

   Last Updated      :

****************************************************************************
               Copyright(c) 1992,1995 Human Machine Interfaces 
                            All Rights Reserved
****************************************************************************/

#ifndef  _SOS_DEFINED   
#define  _SOS_DEFINED   

#ifdef PHARLAP
#include <pharlap.h>
#endif

// include the defs file
#include "sosdefs.h"

// macro for creating a left/right volume value for the mixer
#define MK_VOLUME( l, r ) ( ( short )r << 16 | ( short )l )

// define for an active state 
#define  _ACTIVE           0x8000

// pack the structures on a byte boundary
#pragma pack( 1 ) 

// structure for the VDS system
typedef struct _tag_sos_evds_struct
{
	unsigned       region_size;
	unsigned       offset;
	unsigned       segment;
	unsigned short	number_available;
	unsigned short number_used;
	unsigned       page0;

} _SOS_EVDS_STRUCT;

// structure for the VDS system
typedef struct _tag_sos_vds_struct 
{
	unsigned       region_size;
	unsigned       offset;
	unsigned short segment;
	unsigned short	ID;
	unsigned       physical;

} _SOS_VDS_STRUCT;

// restore the structure packing back to the default command line packing
#pragma pack() 

// pack all of the structure on DWORD boundaries
#pragma pack(4)

// define for the maximum length of a path 
#define  _SOS_MAX_DIR         128 

// local equates
#define  _MIXING_BSIZE        0x4000

// mixer channels default
#define  _SOS_MIXER_CHANNELS  32

// sample flags
#define  _SACTIVE             0x8000
#define  _SPROCESSED          0x4000
#define  _SDONE               0x2000
#define  _SASR_LOOP           0x1000

// sample formats
#define  _PCM_UNSIGNED        0x8000
#define  _PCM_SWAP_CHANNELS   0x4000

// driver format
#define  _DRV_UNSIGNED        0x8000
#define  _DRV_SWAP_CHANNELS   0x4000

// pcm defines
#define  _PCM_16_BIT       16
#define  _PCM_8_BIT        8
#define  _PCM_MONO         1
#define  _PCM_STEREO       2

// channel equates
#define  _PAN_LEFT         0x00000000
#define  _PAN_CENTER       0x00008000
#define  _PAN_RIGHT        0x0000ffff

// volume max
#define  _VOLUME_MAX       0x7fff7fff

// new sample type
typedef  struct   _tag_sos_sample
{
   PSTR  pSample;                      // pointer to sample data
   PSTR  pSampleCurrent;               // pointer to current location
   PSTR  pSampleLoop;                  // loop point if any

   DWORD wLength;                      // length of sample
   DWORD wLoopLength;                  // length of sample loop
   DWORD wLoopEndLength;               // length of loop end
   DWORD wLoopStage;                   // loop stage: Attack, Sustain, Release

   DWORD wID;                          // sample ID
   DWORD wFlags;                       // sample flags
   DWORD wPriority;                    // priority of sample
   DWORD hSample;                      // handle to sample

   DWORD wVolume;                      // volume
   DWORD wLoopCount;                   // loop count

   DWORD wRate;                        // sample rate
   DWORD wBitsPerSample;               // sample size 8/16
   DWORD wChannels;                    // channels per sample 1/2, mono/stereo
   DWORD wFormat;                      // sample format: signed, unsigned, etc..

   DWORD wPanPosition;                 // pan position 0x8000 == center
   DWORD wPanSpeed;                    // speed of auto pan
   DWORD wPanStart;                    // start of pan 
   DWORD wPanEnd;                      // end of pan
   DWORD wPanMode;                     // mode: ONCE, PING-PONG, LOOP

   DWORD wTotalBytesProcessed;         // total bytes actually played

   VOID  ( cdecl * pfnSampleProcessed )( struct _tag_sos_sample * );  // sample complete callback
   VOID  ( cdecl * pfnSampleDone )( struct _tag_sos_sample * );       // sample complete callback
   VOID  ( cdecl * pfnSampleLoop )( struct _tag_sos_sample * );       // sample loop call back

   DWORD wSystem  [ 16 ];              // system data
   DWORD wUser    [ 16 ];              // user data

   struct _tag_sos_sample *  pLink;         // link to next buffer/sample
   struct _tag_sos_sample *  pNext;         // next sample in chain

} _SOS_SAMPLE;
typedef _SOS_SAMPLE * PSOSSAMPLE;

// error definition for sound operating system  
#define  _SOS_ERR          -1

// number of drivers allowed to be open at one time
#define  _SOS_MAX_DRIVERS  5

// flag types for driver
#define  _FLAGS_SIGNED              0x8000
  
// define for no slots available
#define  _ERR_NO_SLOTS ( W32 )-1

// enumeration for the driver functions
enum
{
   _DETDRV_EXISTS,
   _DETDRV_GET_SETTINGS,
   _DETDRV_GET_CAPABILITIES
};

// size of the temporary driver buffer
#define  _DRIVER_BUFFER_SIZE 4096
 
// error codes for the system
enum
{
   _ERR_NO_ERROR,
   _ERR_DRIVER_NOT_LOADED,
   _ERR_INVALID_POINTER,
   _ERR_DETECT_INITIALIZED,
   _ERR_FAIL_ON_FILE_OPEN,
   _ERR_MEMORY_FAIL,
   _ERR_INVALID_DRIVER_ID,
   _ERR_NO_DRIVER_FOUND,
   _ERR_DETECTION_FAILURE,
   _ERR_DRIVER_LOADED,
   _ERR_INVALID_HANDLE,
   _ERR_NO_HANDLES,
   _ERR_PAUSED,   
   _ERR_NOT_PAUSED,
   _ERR_INVALID_DATA,
   _ERR_DRV_FILE_FAIL,
   _ERR_INVALID_PORT,
   _ERR_INVALID_IRQ,
   _ERR_INVALID_DMA,
   _ERR_INVALID_DMA_IRQ,
   _ERR_STREAM_PLAYING,      
   _ERR_STREAM_EMPTY,        
   _ERR_STREAM_PAUSED,       
   _ERR_STREAM_NOT_PAUSED, 
   _ERR_INITIALIZED,
   _ERR_NOT_INITIALIZED,
   _ERR_NO_TRACKS
};   

// maximum number of timer events that can be registered 
#define  _TIMER_MAX_EVENTS    0x10  

// flags for the debugging system
#define  _SOS_DEBUG_NORMAL       0x0000
#define  _SOS_DEBUG_NO_TIMER     0x0001
#define  _SOS_TIMER_DPMI         0x0002

// define for types of DOS extenders
#define  _SOS_RATIONAL           0x8000
#define  _SOS_FLASHTECK          0x4000

// define for special 18.2 callback rate to dos
#define  _TIMER_DOS_RATE   0xff00

// devices that can be loaded
#define  _SOUND_BLASTER_8_MONO      0xe000
#define  _SOUND_BLASTER_8_ST        0xe001
#define  _SBPRO_8_ST                _SOUND_BLASTER_8_ST
#define  _SBPRO_8_MONO              0xe00f
#define  _SOUND_MASTER_II_8_MONO    0xe002
#define  _MV_PAS_8_MONO             0xe003
#define  _MV_PAS_16_MONO            0xe004
#define  _MV_PAS_8_ST               0xe005
#define  _MV_PAS_16_ST              0xe006
#define  _ADLIB_GOLD_8_ST           0xe007
#define  _ADLIB_GOLD_16_ST          0xe008
#define  _ADLIB_GOLD_8_MONO         0xe009
#define  _ADLIB_GOLD_16_MONO        0xe00a
#define  _MICROSOFT_8_MONO          0xe00b
#define  _MICROSOFT_8_ST            0xe00c
#define  _MICROSOFT_16_MONO         0xe00d
#define  _MICROSOFT_16_ST           0xe00e
#define  _SOUND_SOURCE_8_MONO_PC    0xe010
#define  _SOUND_SOURCE_8_MONO_TANDY 0xe011
#define  _GENERAL_PORT_8_MONO       0xe012
#define  _GENERAL_PORT_8_MONO_R     0xe013
#define  _SIERRA_8_MONO             0xe014
#define  _SB16_8_MONO               0xe015
#define  _SB16_8_ST                 0xe016
#define  _SB16_16_MONO              0xe017
#define  _SB16_16_ST                0xe018
#define  _ESS_AUDIODRIVE_8_MONO     0xe019
#define  _ESS_AUDIODRIVE_8_ST       0xe01a
#define  _ESS_AUDIODRIVE_16_MONO    0xe01b
#define  _ESS_AUDIODRIVE_16_ST      0xe01c
#define  _SOUNDSCAPE_8_MONO         0xe01d
#define  _SOUNDSCAPE_8_ST           0xe01e
#define  _SOUNDSCAPE_16_MONO        0xe01f
#define  _SOUNDSCAPE_16_ST          0xe020
#define  _RAP10_8_MONO              0xe021
#define  _RAP10_16_MONO             0xe022
#define  _GUS_8_MONO                0xe023
#define  _GUS_8_ST                  0xe024
#define  _GUS_16_MONO               0xe025
#define  _GUS_16_ST                 0xe026
#define  _GUS_MAX_8_MONO            0xe027
#define  _GUS_MAX_8_ST              0xe028
#define  _GUS_MAX_16_MONO           0xe029
#define  _GUS_MAX_16_ST             0xe02a
#define  _WAVEJAMMER_8_MONO         0xe02b
#define  _WAVEJAMMER_8_ST           0xe02c
#define  _WAVEJAMMER_16_MONO        0xe02d
#define  _WAVEJAMMER_16_ST          0xe02e
#define  _TEMPOCS_8_MONO            0xe02f
#define  _TEMPOCS_8_ST              0xe030
#define  _TEMPOCS_16_MONO           0xe031
#define  _TEMPOCS_16_ST             0xe032
#define  _WAVEJAMMERCD_8_MONO       0xe033
#define  _WAVEJAMMERCD_8_ST         0xe034
#define  _WAVEJAMMERCD_16_MONO      0xe035
#define  _WAVEJAMMERCD_16_ST        0xe036
#define  _TB_MULTISOUND_8_MONO      0xe037 
#define  _TB_MULTISOUND_8_ST        0xe038 
#define  _TB_MULTISOUND_16_MONO     0xe039 
#define  _TB_MULTISOUND_16_ST       0xe03a 
#define  _SOUND_BLASTER_8_MONO_R    0xe050
#define  _MICROSOFT_8_MONO_R        0xe051
#define  _SOUND_MASTER_II_8_MONO_R  0xe052
#define  _ADLIB_GOLD_8_MONO_R       0xe053
#define  _MV_PAS_8_MONO_R           0xe054
#define  _RAP10_8_MONO_R            0xe058
#define  _RAP10_16_MONO_R           0xe059
#define  _SB16_8_MONO_R             0xe05a
#define  _SB16_8_ST_R               0xe05b
#define  _SB16_16_MONO_R            0xe05c
#define  _SB16_16_ST_R              0xe05d
#define  _MV_PAS_16_MONO_R          0xe060
#define  _SOUNDSCAPE_8_MONO_R       0xe061
#define  _SOUNDSCAPE_8_ST_R         0xe062
#define  _SOUNDSCAPE_16_MONO_R      0xe063
#define  _SOUNDSCAPE_16_ST_R        0xe064
#define  _ESS_AUDIODRIVE_8_MONO_R   0xe065
#define  _ESS_AUDIODRIVE_8_ST_R     0xe066
#define  _ESS_AUDIODRIVE_16_MONO_R  0xe067
#define  _ESS_AUDIODRIVE_16_ST_R    0xe068
#define  _SPEECH_THING_8_MONO       0xe090
#define  _YAMAHA_8_MONO             0xe106
#define  _INT_SPEAKER_8_MONO        0xe107

// call indexes for the loadable drivers
enum
{ 
   _DRV_INIT,
   _DRV_UNINIT,
   _DRV_SETRATE,
   _DRV_SETACTION,
   _DRV_START,
   _DRV_STOP,
   _DRV_PAUSE,
   _DRV_RESUME,
   _DRV_CAPABILITIES,
   _DRV_PLAY_FOREGROUND,
   _DRV_GET_FILL_INFO, 
   _DRV_GET_CALL_FUNCTIONS,
   _DRV_SET_CALL_FUNCTIONS
};

// maximum number of available voice
#define  _MAX_VOICES    32

// defines for the wParam flags 
#define  _SINGLE_SAMPLE 0x01

#define  _SOS_DCAPS_AUTO_REINIT     0x01
#define  _SOS_DCAPS_MPU_401         0x02
#define  _SOS_DCAPS_OPL2            0x04
#define  _SOS_DCAPS_OPL3            0x08
#define  _SOS_DCAPS_OPL4            0x10
#define  _SOS_DCAPS_WAVETABLE       0x20
#define  _SOS_DCAPS_DL_SAMPLES      0x40
#define  _SOS_DCAPS_FIFO_DEVICE     0x80
#define  _SOS_DCAPS_ENV_NEEDED      0x100
#define  _SOS_DCAPS_PSEUDO_DMA1     0x200
#define  _SOS_DCAPS_SIGNED_DATA     0x8000

// structure definition for the capabilities
typedef struct _tagCAPABILITIES
{
   BYTE  szDeviceName[ 32 ];  // device name
   W32  wDeviceVersion;      // device version
   W32  wBitsPerSample;      // bits per sound sample
   W32  wChannels;           // stereo/mono sound card
   W32  wMinRate;            // minimum rate
   W32  wMaxRate;            // maximum rate
   W32  wMixerOnBoard;       // board contains mixer
   W32  wMixerFlags;         // mixer capabilities
   W32  wFlags;              // miscellaneous flags   
   short far * lpPortList;         // list of usable ports
   short far * lpDMAList;          // list of usable dma channels
   short far * lpIRQList;          // list of usable irq channels
   short far * lpRateList;         // list of usable rates, -1 if any in min to max
   W32  fBackground;         // foreground or background driver
   W32  wID;                 // ID for the device
   W32  wTimerID;            // ID for the timer
   
} _SOS_CAPABILITIES; 
typedef _SOS_CAPABILITIES * PSOSCAPABILITIES;

// device hardware information
typedef struct
{
   // port to be used
   W32  wPort;

   // irq to use
   W32  wIRQ;

   // dma channel to se
   W32  wDMA; 
  
   // extra parameter
   W32  wParam;

} _SOS_HARDWARE;
typedef _SOS_HARDWARE * PSOSHARDWARE;

// flags for the _SOS_DIGI_DRIVER.wFlags
#define _SOS_DRV_INITIALIZED           0x0001
#define _SOS_DRV_LOADED                0x0002
#define _SOS_DRV_DMA_BUFFER_ALLOCATED  0x0004

// structure for a driver
typedef struct _tag_sos_driver
{  
   // flags for the driver
   // _SOS_DRV_INITIALIZED
   // _SOS_DRV_LOADED
   // _SOS_DRV_DMA_BUFFER_ALLOCATED
   W32 wFlags;

   // rate dma is moving in samples per second
   DWORD wDriverRate;

   // number of channels that the driver is using(1,2)
   DWORD wDriverChannels;

   // number of bits per sample(8,16)
   DWORD wDriverBitsPerSample;

   // driver format
   DWORD wDriverFormat;

   // number of channels that the mixer is using
   DWORD wMixerChannels;

   // dma count register
   DWORD wDMACountRegister;

   // current position of the dma controller 
   DWORD wDMAPosition;

   // last position that the dma controller was at
   DWORD wDMALastPosition;

   // amount moved on last dma tick 
   DWORD wDMADistance;

   // position to transfer data to
   PSTR  pXFERPosition;

   // amount to jump ahead of the dma controller each time a sample
   // is started
   DWORD wXFERJumpAhead;

   // pointer to the sample list
   _SOS_SAMPLE * pSampleList;

   // pointer to the PSEUDO-DMA callback functions
   VOID ( far * pfnPseudoDMAFunction )( VOID );

   // pointer to the dma buffer
   PSTR  pDMABuffer;

   // pointer to the end of the dma buffer
   PSTR  pDMABufferEnd;

   // size of the dma buffer
   DWORD wDMABufferSize;

   // pointer to the mixing buffer
   PSTR  pMixingBuffer;

   // pointer to the end of the mixing buffer
   PSTR  pMixingBufferEnd;

   // size of the mixing buffer
   DWORD wMixingBufferSize;

   // number of active channels for the mixer 
   DWORD wActiveChannels;

   // pointer to the sample list
   _SOS_SAMPLE * pSamples;

   // hardware information 
   _SOS_HARDWARE sHardware;

   // capabilities for the driver
   _SOS_CAPABILITIES sCaps;

   // pointers to the driver
   LPSTR lpDriverDS;
   LPSTR lpDriverCS;

   // size of the driver
   W32 wSize;

   // linear address of the driver
   DWORD dwLinear;

   // physical address of the dma buffer
   DWORD dwDMAPhysical;

   // far pointer to the dma buffer 
   LPSTR lpDMABuffer;

   // memory handle for the driver
   W32 hMemory;

   // real mode segment of dma buffer
   W32 wDMARealSeg;

   // ID for the driver
   W32 wID;

   // pointer to mix function
   VOID ( * pfnMixFunction )( VOID );

} _SOS_DIGI_DRIVER;
typedef _SOS_DIGI_DRIVER * PSOSDIGIDRIVER;

// file header structure
typedef struct
{
   // name ID
   BYTE  szName[ 32 ];

   // number of drivers in the file
   W32  wDrivers;

   // offset of first driver
   W32  lOffset;

   // size of the file
   W32  lFileSize;

} _SOS_DRV_FILEHEADER;
typedef _SOS_DRV_FILEHEADER * PSOSDRVFILEHEADER;

// driver header structure
typedef struct
{
   // name ID
   BYTE  szName[ 32 ];

   // offset of next driver
   W32  lNextDriver;

   // size of current driver
   W32  wSize;

   // id for the current device
   W32  wDeviceID;

   // id for the type of DOS extender
   W32  wExtenderType;

} _SOS_DRV_DRIVERHEADER;
typedef _SOS_DRV_DRIVERHEADER * PSOSDRVDRIVERHEADER;

// flags for the _SOS_SYSTEM.wFlags
#define  _SOS_SYSTEM_INITIALIZED    0x0001

// structure for all of the system information
typedef struct _tag_sos_system
{
   // flags for the system
   // _SOS_SYSTEM_INITIALIZED
   W32 wFlags;
    
   // path to the drivers
   BYTE szDriverPath[ _SOS_MAX_DIR ];

   // temporary path to the drivers
   BYTE szTempDriverPath[ _SOS_MAX_DIR ];

   // pointers to all of the driver structures
   PSOSDIGIDRIVER pDriver[ _SOS_MAX_DRIVERS ];
 
   // VDS information
   _SOS_VDS_STRUCT sVDS;

   // structures for loading a driver
   _SOS_DRV_FILEHEADER sFileHeader;
   _SOS_DRV_DRIVERHEADER sDriverHeader;

   // pointers to the memory allocation functions
   PSTR ( * pMemAllocFunction )( DWORD );
// NEW
   VOID ( * pMemFreeFunction )( PSTR, W32 );
// END

} _SOS_SYSTEM;
typedef _SOS_SYSTEM * PSOSSYSTEM;

// flags for the _SOS_DET_SYSTEM.wFlags
#define  _SOS_DET_SYSTEM_INITIALIZED    0x0001

// structure for all of the detection system information
typedef struct _tag_sos_det_system
{
   // flags for the system
   // _SOS_DET_SYSTEM_INITIALIZED
   W32 wFlags;
    
   // path to the drivers
   BYTE szDriverPath[ _SOS_MAX_DIR ];

   // temporary path to the drivers
   BYTE szTempDriverPath[ _SOS_MAX_DIR ];

   // structures for loading a driver
   _SOS_DRV_FILEHEADER sFileHeader;
   _SOS_DRV_DRIVERHEADER sDriverHeader;

   // capabilities for the driver
   _SOS_CAPABILITIES sCaps;
 
   // pointer to the detection device caps 
   PSOSCAPABILITIES pCaps;

   // pointer to the temporary detection system driver buffer data segment 
   LPSTR lpBufferDS;

   // pointer to the temporary detection system driver buffer data segment 
   LPSTR lpBufferCS;

   // handle for the detection driver file 
   W32 hFile;

   // indexes into the detection driver file
   DWORD dwDriverIndex;
   W32 wDriverIndexCur;

   // memory handle for the detection system temporary buffer
   W32 hMemory;

   // linear address of the detection system temporary buffer
   DWORD dwLinear;

} _SOS_DET_SYSTEM;
typedef _SOS_DET_SYSTEM * PSOSDETSYSTEM;

// flags for the _SOS_TIMER_SYSTEM.wFlags
#define  _SOS_TIMER_INITIALIZED     0x0001
#define  _SOS_TIMER_USED            0x0002
#define  _SOS_TIMER_ACTIVE          0x0004

// structure for the timer system
typedef struct _tag_sos_timer_system
{
   // flags for the system 
   // _SOS_TIMER_INTIALIZED
   // _SOS_TIMER_USED
   // _SOS_TIMER_ACTIVE
   W32 wFlags;

   // last value written to the timer chip
   W32 wChipDivisor;

   // event list pointers
   VOID ( * pfnEvent[ _TIMER_MAX_EVENTS ] )( VOID );

   // rate for each event
   W32 wEventRate[ _TIMER_MAX_EVENTS ];

   // fractional value to add to the current summation each timer tick
   DWORD dwAdditiveFraction[ _TIMER_MAX_EVENTS ];

   // current summation for each event
   DWORD dwCurrentSummation[ _TIMER_MAX_EVENTS ];

   // handle of the song that is relative to the current events
   W32  wMIDIEventSongHandle[ _TIMER_MAX_EVENTS ];

   // current song handle
   W32  wMIDIActiveSongHandle;

#ifdef PHARLAP
   // pharlap timer system variables
   RMC_BLK sRealRegs;
   REALPTR pPrevRealPtr;
   FARPTR pPrevProtPtr;
   FARPTR pCurProtPtr;
#endif

} _SOS_TIMER_SYSTEM;
typedef _SOS_TIMER_SYSTEM * PSOSTIMERSYSTEM;

// restore the structure packing back to the default command line packing
#pragma pack()

#include "sosdata.h"
#include "sosfnct.h"

#endif
