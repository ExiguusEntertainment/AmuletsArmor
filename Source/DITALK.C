/*-------------------------------------------------------------------------*
 * File:  DITALK.C
 *-------------------------------------------------------------------------*/
/**
 * The "Direct Talk" system is just the fancy name for the networking
 * communications.  It was attempted to be setup so it worked the same
 * between modem, null modem/serial, and IPX communications.
 *
 * @addtogroup DITALK
 * @brief Direct Talk System
 * @see http://www.amuletsandarmor.com/AALicense.txt
 * @{
 *
 *<!-----------------------------------------------------------------------*/
#include "DITALK.H"
#include "MEMORY.H"

#ifdef DOS32
#define ALLOW_DOS_DITALK
#endif

#ifdef WIN32
#include "Win32\ipx_client.h"
#endif

// DirectTalk is the name give to the API between A&A and a generic
// system of passing packets to a network group.  The name came from
// Windows original DirectTalk name (but it never was Windows DirectX
// DirectTalk).
//
// The basic idea is there is a T_directTalkStruct that represents the
// current state of the direct talk interface.  In DOS, there
// was a vector that was called passing in the T_directTalkStruct.
// All data in/out was passed through this singular structure.
// Because the DOS drivers were programmed to support this structure,
// all the code is in this one place.
//
// Additionally, the DOS version had a hook for incoming data via
// another vector connected to IDirectTalkISR().  DirectTalkInit
// configures all the links to the vector on who to call on each event
// (connect, disconnect, sent, received).
//
// Digging even deeper, we see we have these assertions:
//
//    DebugCheck(p_callRecv != NULL) ;
//    DebugCheck(p_callSend == NULL) ;
//    DebugCheck(p_callConnect == NULL) ;
//    DebugCheck(p_callDisconnect == NULL) ;
//
// Which means, quite plainly that only the receive callback is used
// and most of the events are not even used.  Admittedly, this implies
// a weakness, but considering that we are not planning on modems
// any more, then we might as well just drop it all.
//
// Now that we've added a Windows version, we want to add a layer/option
// on top of this that works very similarly.  For the Windows version,
// however, we are going to have to poll for incoming packets/events.
// This is done through the routine DirectTalkPollData().

#include "DITALKP.H"

// Use one of the .C files
#ifdef WIN32
    #if WIN_IPX
        #include "WIN32\WINDTALK.C"
    #else
        #include "Generic\NODTALK.C"
    #endif
#else
    #include "DOS\DOSDTALK.C"
#endif

/** @} */
/*-------------------------------------------------------------------------*
 * End of File:  DITALK.C
 *-------------------------------------------------------------------------*/
