/****************************************************************************
 *
 *  File              : sosez.c
 *  Date Created      : 2/27/95
 *  Description       : easy interface to help get started
 *
 *  Programmer(s)     : Nick Skrepetos
 *  Last Modification : 2/27/95 - 7:59:16 PM
 *  Additional Notes  :
 *
 *****************************************************************************
 *            Copyright (c) 1994-5,  HMI, Inc.  All Rights Reserved          *
 ****************************************************************************/

#include "GENERAL.H"

#ifdef USE_SOS_LIBRARY
#include <stdlib.h>
#include <stdio.h>
#include <dos.h>
#include <fcntl.h>
#if defined(DOS32)
#include <bios.h>
#endif
#include <io.h>
#include <malloc.h>
#include <conio.h>
#include <ctype.h>
#include <string.h>
#include "sos.h"
#include "sosm.h"
#include "sosez.h"
#include "profile.h"

/***************************************************************************

 Notes:

 This file 'sosez.c' is intended to help get the first time user
 up and running in just a few minutes.  The sample program allows
 you to play digital samples and MIDI songs with ease.  Although
 the 'sosEZ' functions to not use every capability of the system,
 it should ease the integration process.

 The user must run the 'SETUP.EXE' program provided with the SOS
 to set up the file 'hmiset.cfg' before running any program containing
 this module.

 ****************************************************************************/

// local data for digital driver
_SOS_DIGI_DRIVER sDIGIDriver;               // digital driver structure

// local data for MIDI driver
_SOS_MIDI_DRIVER sMIDIDriver;               // midi driver structure

// handles for sample, song, midi and digital drivers
W32 wDIGIDeviceID;                            // digital device ID
HANDLE hDIGIDriver;                              // handle to digital driver
HANDLE hDIGITimer;                               // handle to digital mixer
W32 wMIDIDeviceID;                            // midi device ID
HANDLE hMIDIDriver;                              // handle to MIDI driver
HANDLE hMIDISong;                                // handle to MIDI song
PSTR pSong;                                    // pointer to MIDI song
PSTR pMelodicPatch;                            // melodic FM instruments
PSTR pDrumPatch;                               // drum FM instruments
HANDLE G_timerHandle;

VOID TickerInc(VOID);

// W32  sosEZInitSystem( W32 wDDeviceID, W32 wMDeviceID )
W32 cdecl sosEZInitSystem(W32 wDDeviceID, W32 wMDeviceID) {
	// set up the digital driver
	sDIGIDriver.wDriverRate = _SOSEZ_SAMPLE_RATE;
	sDIGIDriver.wDMABufferSize = _SOSEZ_DMA_BUFFER;

	// initialize the timer system
	sosTIMERInitSystem(_TIMER_DOS_RATE, _SOS_DEBUG_NORMAL);

	sosTIMERRegisterEvent(70, TickerInc, &G_timerHandle);

	// initialize the digital and midi systems
	sosDIGIInitSystem(_NULL, _SOS_DEBUG_NORMAL);
	sosMIDIInitSystem(_NULL, _SOS_DEBUG_NORMAL);

	// check to see if the midi device is to be initialized
	if (wMDeviceID != -1) {
		// initialize MIDI
		sMIDIDriver.wID = wMDeviceID;
		if (sosMIDIInitDriver(&sMIDIDriver, &hMIDIDriver)) {
			// uninitialize digital system
			sosDIGIUnInitDriver(hDIGIDriver, _TRUE, _TRUE);

			// uninitialize digital system
			sosDIGIUnInitSystem();

			// uninitialize system
			sosMIDIUnInitSystem();

			// return
			return (_SOSEZ_MIDI_INIT);
		}
	}

	// initialize the digital driver
	if (wDDeviceID != -1) {
		// initialize digital
		sDIGIDriver.wID = wDDeviceID;
		if (sosDIGIInitDriver(&sDIGIDriver, &hDIGIDriver)) {
			// uninitialize digital system
			sosDIGIUnInitSystem();

			// return
			return (_SOSEZ_DIGI_INIT);
		}
	}

	// register digital timer event (mixer)
	if (wDDeviceID != -1)
		sosTIMERRegisterEvent(_SOSEZ_TIMER_RATE, sDIGIDriver.pfnMixFunction,
				&hDIGITimer);

	// check driver type, if it is an OPL2/3 driver, we
	// need to load the patch files for the driver.
	if (wMDeviceID == _MIDI_FM
			|| wMDeviceID == _MIDI_OPL3 && wMDeviceID != -1) {
		// attempt to load melodic patch
		if ((pMelodicPatch = sosEZLoadPatch("melodic.bnk")) == _NULL) {
			// uninitialize system
			sosEZUnInitSystem();

			// return error
			return (_SOSEZ_PATCH_MELODIC);
		}

		// attempt to load drum patch
		if ((pDrumPatch = sosEZLoadPatch("drum.bnk")) == _NULL) {
			// uninitialize system
			sosEZUnInitSystem();

			// return error
			return (_SOSEZ_PATCH_DRUM);
		}

		// set instrument data
		if ((sosMIDISetInsData(hMIDIDriver, pMelodicPatch, 1))) {
			// uninitialize system
			sosEZUnInitSystem();

			// return error
			return (_SOSEZ_PATCH_INIT);
		}

		// set instrument data
		if ((sosMIDISetInsData(hMIDIDriver, pDrumPatch, 1))) {
			// uninitialize system
			sosEZUnInitSystem();

			// return error
			return (_SOSEZ_PATCH_INIT);
		}
	}

	// return success
	return (_SOSEZ_NO_ERROR);
}

// W32  sosEZUnInitSystem( VOID )
W32 cdecl sosEZUnInitSystem(VOID) {
	// release timer event
	sosTIMERRemoveEvent(hDIGITimer);

	// uninitialize digital driver
	if (wDIGIDeviceID != -1)
		sosDIGIUnInitDriver(hDIGIDriver, _TRUE, _TRUE);

	// uninitialize midi driver
	if (wMIDIDeviceID != -1)
		sosMIDIUnInitDriver(hMIDIDriver, _TRUE);

	// uninitialize midi system
	sosMIDIUnInitSystem();

	// uninitialze digital system
	sosDIGIUnInitSystem();

	// uninitialize timer system
	sosTIMERUnInitSystem(0);

	// return success
	return (_TRUE);
}

// BOOL  sosEZGetConfig( PSTR szName )
BOOL cdecl sosEZGetConfig(PSTR szName) {
	_INI_INSTANCE sInstance;
	BOOL wError;

	// reset digital and MIDI driver structures
	memset(&sDIGIDriver, 0, sizeof(_SOS_DIGI_DRIVER));
	memset(&sMIDIDriver, 0, sizeof(_SOS_MIDI_DRIVER));

	// open .ini file
	if (!hmiINIOpen(&sInstance, szName))
		return (_FALSE);

	// locate section for digital settings
	if (!hmiINILocateSection(&sInstance, "DIGITAL")) {
		// close file
		hmiINIClose(&sInstance);

		// return error
		return (_FALSE);
	}

	// fetch device ID, Port, DMA, IRQ
	wError = hmiINIGetItemDecimal(&sInstance, "DeviceID", &wDIGIDeviceID);
	wError = hmiINIGetItemDecimal(&sInstance, "DevicePort",
			&sDIGIDriver.sHardware.wPort);
	wError = hmiINIGetItemDecimal(&sInstance, "DeviceDMA",
			&sDIGIDriver.sHardware.wDMA);
	wError = hmiINIGetItemDecimal(&sInstance, "DeviceIRQ",
			&sDIGIDriver.sHardware.wIRQ);

	// error
	if (!wError) {
		// close file
		hmiINIClose(&sInstance);

		// return error
		return (_FALSE);
	}

	// locate section for MIDI settings
	if (!hmiINILocateSection(&sInstance, "MIDI")) {
		// close file
		hmiINIClose(&sInstance);

		// return error
		return (_FALSE);
	}

	// fetch device ID, Port, DMA, IRQ
	wError = hmiINIGetItemDecimal(&sInstance, "DeviceID", &wMIDIDeviceID);
	wError = hmiINIGetItemDecimal(&sInstance, "DevicePort",
			&sMIDIDriver.sHardware.wPort);

	// error
	if (!wError) {
		// close file
		hmiINIClose(&sInstance);

		// return error
		return (_FALSE);
	}

	// close file
	hmiINIClose(&sInstance);

	/* If either device is turned off, don't do anything. */
	if ((wMIDIDeviceID == 0xFFFFFFFF) || (wDIGIDeviceID == 0xFFFFFFFF)) {
		return (_FALSE);
	}
	// return success
	return (_TRUE);
}

// PSTR  sosEZLoadSample( PSTR szName )
_SOS_SAMPLE * cdecl sosEZLoadSample( PSTR szName )
   {
      W32  hFile;
      W32  wSize;
      PSTR  pData;
      _SOS_SAMPLE *  pSample;
      _WAVHEADER * pWaveHeader;

      // attempt to open file
      if ( ( hFile = open( szName, O_RDONLY | O_BINARY ) ) == -1 )
         return( _NULL );

      // seek to the end of the file to determine 
      // the file length;
      wSize =  lseek( hFile, 0, SEEK_END );

      // seek back to the start of the file
      lseek( hFile, 0, SEEK_SET );

      // allocate memory for the file
      if ( ( pData = (PSTR)malloc( wSize + sizeof( _SOS_SAMPLE  ) ) ) == _NULL )
      {
         // close file
         close( hFile );

         // return error
         return( _NULL );
      }

      // read in file
      if ( read( hFile, pData + sizeof( _SOS_SAMPLE ), wSize ) != wSize )
      {
         // close file
         close( hFile );

         // free memory
         free( pData );

         // return error
         return( _NULL );
      }

      // close file
      close( hFile );

      // reset start sample structure
      memset( pData, 0, sizeof( _SOS_SAMPLE ) );

      // set pointer to sample
      pSample  =  ( _SOS_SAMPLE * )pData;

      // check to see if this is a .WAV file, if so then grab the settings
      // out of the file
      if( !strncmp( pData + sizeof( _SOS_SAMPLE ), "RIFF", 0x04 ) )
      {
         // setup a pointer to the wave header
         pWaveHeader = ( _WAVHEADER * )( pData + sizeof( _SOS_SAMPLE ) );

         // set size of the sample and pointer to the sample
         pSample->pSample        =  ( PSTR )pData + sizeof( _SOS_SAMPLE ) + 
            sizeof( _WAVHEADER );
         pSample->wLength        = pWaveHeader->dwDataLength - sizeof( _WAVHEADER );
         pSample->wBitsPerSample = pWaveHeader->wBitsPerSample;
         pSample->wChannels      = pWaveHeader->wChannels;
         if( pWaveHeader->wBitsPerSample == 0x08 )
            pSample->wFormat    = _PCM_UNSIGNED;
         else
            pSample->wFormat    = 0x00;
         pSample->wRate          = pWaveHeader->dwSamplesPerSec;
      }
      else
      {
         // set size of the sample and pointer to the sample
         pSample->pSample    =  ( PSTR )pData + sizeof( _SOS_SAMPLE );
         pSample->wLength   =  ( DWORD )wSize;
         pSample->wBitsPerSample = 0x10;
         pSample->wChannels = 0x01;
//         pSample->wFormat = _PCM_UNSIGNED;
         pSample->wRate = 22050;
      }

      // set the pan and volume
      pSample->wPanPosition = _PAN_CENTER;
      pSample->wVolume = MK_VOLUME( 0x7fff, 0x7fff);

// return pointer to the sample
return( pSample );
}

// W32  sosEZLoadSong( PSTR szName )
W32 cdecl sosEZLoadSong(PSTR szName) {
W32 hFile;
W32 hSong;
PSTR pData;
W32 wSize;
W32 wIndex;
_SOS_MIDI_SONG * sSong;

// attempt to open file
if ((hFile = open(szName, O_RDONLY | O_BINARY)) == -1)
	return ((W32) -1);

// seek to the end of the file to determine
// the file length;
wSize = lseek(hFile, 0, SEEK_END);

// seek back to the start of the file
lseek(hFile, 0, SEEK_SET);

// allocate memory for the file, init song structure and track map
if ((pData = (PSTR) malloc(wSize + sizeof(_SOS_MIDI_SONG))) == _NULL) {
	// close file
	close(hFile);

	// return error
	return ((W32) -1);
}

// read in file
if (read(hFile, pData + sizeof(_SOS_MIDI_SONG), wSize) != wSize) {
	// close file
	close(hFile);

	// free memory
	free(pData);

	// return error
	return ((W32) -1);
}

// close file
close(hFile);

// set up pointers to track mapping and song
sSong = (_SOS_MIDI_SONG *) pData;

// reset song structure
memset(sSong, 0, sizeof(_SOS_MIDI_SONG));

// set up song structure
sSong->pSong = (PSTR)(pData + sizeof(_SOS_MIDI_SONG));

// initialize song
if (sosMIDIInitSong(sSong, &hSong)) {
	// free song memory
	free(pData);

	// return error
	return ((W32) -1);
}

// save pointer to song memory area to free
// later.
pSong = pData;

// return song handle
return (hSong);
}

// PSTR  cdecl sosEZLoadPatch( PSTR szName )
PSTR cdecl sosEZLoadPatch(PSTR szName) {
PSTR pData;
W32 wSize;
W32 hFile;

// attempt to open file
if ((hFile = open(szName, O_RDONLY | O_BINARY)) == -1)
	return (_NULL);

// seek to the end of the file to determine
// the file length;
wSize = lseek(hFile, 0, SEEK_END);

// seek back to the start of the file
lseek(hFile, 0, SEEK_SET);

// allocate memory for the file, init song structure and track map
if ((pData = (PSTR) malloc(wSize)) == _NULL) {
	// close file
	close(hFile);

	// return error
	return (_NULL);
}

// read in file
if (read(hFile, pData, wSize) != wSize) {
	// close file
	close(hFile);

	// free memory
	free(pData);

	// return error
	return (_NULL);
}

// close file
close(hFile);

// return pointer
return (pData);
}

#endif // USE_SOS_LIBRARY
