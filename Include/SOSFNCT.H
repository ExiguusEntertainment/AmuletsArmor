/****************************************************************************

   File              : sosfnct.h

   Programmer(s)     : Don Fowler, Nick Skrepetos
   Date              :

   Purpose           : 

   Last Updated      :

****************************************************************************
               Copyright(c) 1993,1994 Human Machine Interfaces 
                            All Rights Reserved
****************************************************************************/

#ifndef  _SOS_FUNCTIONS
#define  _SOS_FUNCTIONS

#pragma pack(4)

VOID sosSetMemAllocFunction      ( PSTR ( * )( DWORD ) );
// NEW
VOID sosSetMemFreeFunction       ( VOID ( * )( PSTR, W32 ) );
// END
W32 sosDIGILockLibrary          (  VOID  );
W32 sosDIGIUnLockLibrary        (  VOID  );
W32 sosDIGIInitSystem           (  PSTR, W32  );
W32 sosDIGIUnInitSystem         (  VOID  );
W32 sosDIGIInitDriver           (  PSOSDIGIDRIVER, HANDLE * );
W32 sosDIGIUnInitDriver         (  HANDLE, BOOL, BOOL  );
W32 sosDIGILoadDriver           (  W32, HANDLE, LPSTR *, LPSTR *, PSTR, PSTR, W32 * );
W32 sosDIGIUnLoadDriver         (  HANDLE  );
W32 sosDIGIGetDeviceCaps        (  HANDLE, PSOSCAPABILITIES  );
PSTR sosDIGIAllocDMABuffer       (  W32, DWORD *, W32 *, LPSTR * );
W32 sosDIGIStopSample           (  HANDLE, HANDLE  );
W32 sosDIGIStartSample          (  HANDLE, PSOSSAMPLE  );
W32 sosDIGIDetectInit           (  PSTR  );
W32 sosDIGIDetectUnInit         (  VOID  );
W32 sosDIGIDetectFindHardware   (  W32, PSOSDIGIDRIVER  );
W32 sosDIGIDetectFindFirst      (  PSOSDIGIDRIVER  );
W32 sosDIGIDetectFindNext       (  PSOSDIGIDRIVER  );
W32 sosDIGIDetectGetSettings    (  PSOSDIGIDRIVER  );
W32 sosDIGIDetectGetCaps        (  W32, PSOSDIGIDRIVER  );
W32 sosDIGIDetectVerifySettings (  PSOSDIGIDRIVER  );
PSTR sosGetErrorString           (  W32  );
W32 sosTIMERRegisterEvent       ( W32, VOID ( * )( VOID ),
                                   HANDLE * );
W32 sosTIMERInitSystem          (  W32, W32  );
W32 sosTIMERUnInitSystem        (  W32  );
W32 sosTIMERSetRate             (  HANDLE  );  
W32 sosTIMERRemoveEvent         (  HANDLE );
W32 sosTIMERAlterEventRate      (  HANDLE, W32  );
W32 sosTIMERGetEventRate        (  HANDLE  );
VOID sosTIMEROldHandler          (  VOID  );
VOID sosTIMERHandler             (  VOID  );
PSTR sosAlloc                    ( W32 );
// NEW
VOID sosFree                     ( PSTR, W32 );
// END
VOID sosDIGIMixFunction0         ( VOID );
VOID sosDIGIMixFunction1         ( VOID );
VOID sosDIGIMixFunction2         ( VOID );
VOID sosDIGIMixFunction3         ( VOID );
VOID sosDIGIMixFunction4         ( VOID );
W32 sosDIGIStopAllSamples       ( HANDLE );

W32 sosDIGIGetDMAPosition       ( HANDLE );
W32 sosDIGISetSampleVolume      ( HANDLE, HANDLE, W32 );
W32 sosDIGIGetSampleVolume      ( HANDLE, HANDLE );
W32 sosDIGIGetBytesProcessed    ( HANDLE, HANDLE );
W32 sosDIGIGetLoopCount         ( HANDLE, HANDLE );
W32 sosDIGISetPanLocation       ( HANDLE, HANDLE, W32 );
W32 sosDIGIGetPanLocation       ( HANDLE, HANDLE );
W32 sosDIGISetSampleRate        ( HANDLE, HANDLE, W32 );
W32 sosDIGIGetSampleRate        ( HANDLE, HANDLE );
W32 sosDIGIGetPanSpeed          ( HANDLE, HANDLE );
W32 sosDIGIGetSampleID          ( HANDLE, HANDLE );
W32 sosDIGIGetSampleHandle      ( HANDLE, HANDLE );
W32 sosDIGISamplesPlaying       ( HANDLE );
BOOL sosDIGISampleDone           ( HANDLE, HANDLE );

// memory locking functions
VOID sosModule1Start             ( VOID );
VOID sosModule1End               ( VOID );
VOID sosModule2Start             ( VOID );
VOID sosModule2End               ( VOID );
VOID sosModule3Start             ( VOID );
VOID sosModule3End               ( VOID );
VOID sosModule4Start             ( VOID );
VOID sosModule4End               ( VOID );
VOID sosModule5Start             ( VOID );
VOID sosModule5End               ( VOID );
VOID sosModule6Start             ( VOID );
VOID sosModule6End               ( VOID );
VOID sosModule7Start             ( VOID );
VOID sosModule7End               ( VOID );
VOID sosModule8Start             ( VOID );
VOID sosModule8End               ( VOID );
VOID sosModule9Start             ( VOID );
VOID sosModule9End               ( VOID );
VOID sosModule10Start            ( VOID );
VOID sosModule10End              ( VOID );
VOID sosModule11Start            ( VOID );
VOID sosModule11End              ( VOID );

#ifdef PHARLAP
VOID  sosFreeVDSPage          ( unsigned short, unsigned short, DWORD );
W32  sosAllocVDSPage         ( unsigned short *, unsigned short *, DWORD * );
#else
W32  sosAllocVDSPage            ( LPSTR *, W32 *, DWORD * );
VOID  sosFreeVDSPage             ( W32, W32, LONG );
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef PHARLAP
extern   int   cdecl sosRealFree          ( int );
extern   BOOL  cdecl  _sos_read( W32, LPSTR, W32, W32 * );
extern   int   cdecl sosRealAlloc( int, int *, int * );
extern   void  cdecl sosDRVFarMemCopy( LPSTR, LPSTR, W32 );
extern   int   cdecl sosGetCS( VOID );
extern   int   cdecl sosGetES( VOID );
#else
extern   int   cdecl sosRealAlloc         ( int, int *, int * );
extern   int   cdecl sosRealFree          ( int );
#endif

// sos driver functions
extern   W32  cdecl sosDRVLockMemory     ( DWORD, DWORD );
extern   W32  cdecl sosDRVUnLockMemory   ( DWORD, DWORD );
extern   void  cdecl sosDRVGetCapsInfo    ( LPSTR, LPSTR, _SOS_CAPABILITIES * );
extern   void  cdecl sosDetDRVGetCapsInfo ( LPSTR, LPSTR, _SOS_CAPABILITIES * );
extern   void  cdecl sosDRVGetCapsPtr     ( LPSTR, LPSTR, _SOS_CAPABILITIES * );
extern   void  cdecl sosDRVInit           ( LPSTR, LPSTR, int, int, int, int, int, int );  
extern   void  cdecl sosDRVStart          ( LPSTR, LPSTR, int, int );
extern   void  cdecl sosDRVSetRate        ( LPSTR, LPSTR, int );
extern   void  cdecl sosDRVSetAction      ( LPSTR, LPSTR );
extern   void  cdecl sosDRVStop           ( LPSTR, LPSTR );
extern   void  cdecl sosDRVUnInit         ( LPSTR, LPSTR );
extern   void  cdecl sosFillSampleStructs ( PSTR, LPSTR );
extern   W32  cdecl sosDetDRVExist       ( LPSTR, LPSTR );
extern   W32  cdecl sosDetDRVGetSettings ( LPSTR, LPSTR );
extern   W32  cdecl sosDetDRVVerifySettings( LPSTR, W32, W32, W32, LPSTR );
extern   W32  cdecl sosDIGIInitForWindows( W32 );
extern   W32  cdecl sosDIGIUnInitForWindows( W32 );
extern   LPSTR cdecl sosAllocateFarMem      ( W32, PSTR, DWORD * );
extern   LPSTR cdecl sosCreateAliasCS       ( LPSTR );
extern   VOID  cdecl sosFreeSelector        ( LPSTR, DWORD );
extern   LPSTR cdecl sosMAKEDOSPtr          ( PSTR );
extern   VOID  cdecl sosDetDRVSetEnvString  ( DWORD, PSTR );
extern   PSTR  cdecl sosDetDRVGetEnvString  ( DWORD );
extern   VOID  cdecl sosDetDRVEnvStringInit ( LPSTR, LPSTR );
extern   LPSTR cdecl sosDRVSetupCallFunctions( LPSTR, LPSTR );
extern   W32  cdecl sosDRVGetFreeMemory     ( VOID );
extern   W32  cdecl sosDRVAllocVDSStruct    ( W32, W32 *, W32 * );
extern   W32  cdecl sosDRVFreeVDSStruct     ( W32, W32 );
extern   W32  cdecl sosDRVIsWindowsActive   ( VOID );
extern   W32  cdecl sosDRVVDSGetBuffer    ( W32 );
extern   W32  cdecl sosDRVVDSFreeBuffer   ( W32 );
extern   W32  cdecl getDS( VOID );
extern   W32  cdecl sosDRVMakeDMASelector   ( W32 );  
extern   W32  cdecl sosDRVFreeDMASelector   ( W32 );  

extern   void  cdecl hmiDigitalMixer( PSOSDIGIDRIVER );

extern   void  cdecl sosTIMERDRVInit( int wRate, void ( * )( void ) );
extern   void  cdecl sosTIMERDRVUnInit( void );
extern   void  cdecl sosTIMERDRVHandler( void );
extern   void  cdecl sosTIMERDRVFHandler( void );
extern   void  cdecl sosTIMERDRVEnable( void );
extern   void  cdecl sosTIMERDRVDisable( void );
extern   void  cdecl sosTIMERDRVCallOld( void );
extern   void  cdecl sosTIMERDRVSetRate( W32 );    
extern   void  cdecl sosDIGITimer_Start( void );
extern   void  cdecl sosDIGITimer_End( void );
extern   void  cdecl sosDIGIDrv_Start( void );
extern   void  cdecl sosDIGIDrv_End( void );

extern   void  cdecl sosModule12Start( void );
extern   void  cdecl sosModule12End( void );
extern   void  cdecl sosModule13Start( void );
extern   void  cdecl sosModule13End( void );
extern   void  cdecl sosModule14Start( void );
extern   void  cdecl sosModule14End( void );
extern   void  cdecl sosModule15Start( void );
extern   void  cdecl sosModule15End( void );
extern   void  cdecl sosModule16Start( void );
extern   void  cdecl sosModule16End( void );

#ifdef __cplusplus
}
#endif 

#pragma pack()

#endif
