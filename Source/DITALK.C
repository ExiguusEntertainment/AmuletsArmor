/****************************************************************************/
/*    FILE:  DITALK.C                                                       */
/****************************************************************************/
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

#define DIRECT_TALK_MAX_SIZE_BUFFER      256

/* Range of allowable vectors. */
#define VALID_LOW_VECTOR               0x60
#define VALID_HIGH_VECTOR              0x67

#define DIRECT_TALK_TAG                0xAABBDDCC
#define DIRECT_TALK_DEAD_TAG           0x99665544

/* Enumerate the different type of commands that can be sent back */
/* and forth. */
/* Commands are in reference to the DOS32 side (not the REAL side), */
/* therefore, SEND means that the DOS32 program is sending data */
/* to the REAL side. */
typedef T_byte8 E_directTalkCommand ;
#define DIRECT_TALK_COMMAND_SEND         0
#define DIRECT_TALK_COMMAND_RECV         1
#define DIRECT_TALK_COMMAND_CONNECT      2
#define DIRECT_TALK_COMMAND_DISCONNECT   3
#define DIRECT_TALK_COMMAND_UNKNOWN      4

typedef struct {
    T_word32 tag ;                     /* Identifier to ensure correct tag. */
    T_byte8 vector ;                   /* Vector to communicate. */
    T_directTalkHandle handle ;
    E_directTalkCommand command ;
    E_directTalkLineStatus lineStatus ;
    T_byte8 buffer[DIRECT_TALK_MAX_SIZE_BUFFER] ;
    T_byte8 bufferLength ;             /* Size of data in buffer. */
    E_Boolean bufferFilled ;           /* TRUE if anything in buffer. */
    T_byte8 uniqueAddress[6] ;            /* Unique address to this comlink. */
    E_directTalkServiceType serviceType ;
    T_byte8 destinationAddress[6] ;       /* Destination */
} T_directTalkStruct ;

/* Callback routines to handle receiving and requests to send. */
static T_directTalkReceiveCallback G_receiveCallback = NULL ;
static T_directTalkSendCallback G_sendCallback = NULL ;
static T_directTalkConnectCallback G_connectCallback = NULL ;
static T_directTalkDisconnectCallback G_disconnectCallback = NULL ;

/* Location of direct talk struct. */
static T_directTalkStruct *G_talk ;

#ifndef COMPILE_OPTION_DIRECT_TALK_IS_DOS32
static void interrupt (*G_oldVector)(__CPPARGS) = NULL ;
static E_Boolean G_interruptInstalled = FALSE ;
static void interrupt IDirectTalkISR(T_void) ;
static void IDirectTalkUndoISR(T_void) ;
#endif

static T_directTalkUniqueAddress G_blankAddress ;

#ifndef ALLOW_DOS_DITALK
#define DITALK_MAX_PACKETS  10
typedef struct {
    void *p_data ;
    T_word16 size ;
} T_ditalkRecording ;
static T_ditalkRecording G_packets[DITALK_MAX_PACKETS] ;
static T_word16 G_numPackets = 0 ;
static int G_ipxEnabled = 0;
#endif


T_directTalkHandle DirectTalkInit(
           T_directTalkReceiveCallback p_callRecv,
           T_directTalkSendCallback p_callSend,
           T_directTalkConnectCallback p_callConnect,
           T_directTalkDisconnectCallback p_callDisconnect,
           T_directTalkHandle handle)
{
#ifdef ALLOW_DOS_DITALK
    T_directTalkStruct *p_talk ;
    T_directTalkHandle newHandle = DIRECT_TALK_HANDLE_BAD ;
    T_word16 vector ;
    T_byte8 **p_vector ;

    DebugRoutine("DirectTalkInit") ;

    /* Record the send and receive callbacks. */
    G_receiveCallback = p_callRecv ;
    G_sendCallback = p_callSend ;
    G_connectCallback = p_callConnect ;
    G_disconnectCallback = p_callDisconnect ;

    memset(&G_blankAddress, 0, sizeof(G_blankAddress)) ;

#ifdef COMPILE_OPTION_DIRECT_TALK_IS_DOS32
    DebugCheck(p_callRecv != NULL) ;
    DebugCheck(p_callSend == NULL) ;
    DebugCheck(p_callConnect == NULL) ;
    DebugCheck(p_callDisconnect == NULL) ;

    if ((handle == DIRECT_TALK_HANDLE_BAD) ||
        (((T_word32)handle) >= 0x100000))  {
        puts("DirectTalk Failure: BAD TALK HANDLE.  Is a driver running?") ;
        exit(1) ;
    }

    /* Get access to the structure from the given handle. */
    p_talk = (T_directTalkStruct *)handle ;

    /* Store the pointer for future use. */
    G_talk = p_talk ;

    if (p_talk->tag != DIRECT_TALK_TAG)  {
        puts("DirectTalk Failure: BAD TALK HANDLE.  Is a driver running?") ;
        exit(1) ;
    }

    /* Make sure we are talking about the same handle. */
    if (p_talk->handle != handle)  {
        puts("DirectTalk Failure: INCONSISTENT HANDLES.") ;
        exit(1) ;
    }

    if ((p_talk->vector < VALID_LOW_VECTOR) ||
        (p_talk->vector > VALID_HIGH_VECTOR))  {
        puts("DirectTalk Failure: BAD VECTOR NUMBER.") ;
        exit(1) ;
    }

    /* Init is successful. */
    newHandle = handle ;
#else
    DebugCheck(p_callRecv != NULL) ;
    DebugCheck(p_callSend != NULL) ;
    DebugCheck(p_callConnect != NULL) ;
    DebugCheck(p_callDisconnect != NULL) ;

    /* Allocate memory for the talk structure. */
    p_talk = G_talk = MemAlloc(sizeof(T_directTalkStruct)) ;
    if (p_talk == NULL)  {
        puts("DirectTalk Failure: NOT ENOUGH MEMORY.") ;
        exit(1) ;
    }

    /* Clear the talk structure. */
    memset(G_talk, 0, sizeof(T_directTalkStruct)) ;

    /* Mark the talk structure as valid. */
    p_talk->tag = DIRECT_TALK_TAG ;

    /* Convert the address into an appropriate handle. */
    handle = (((T_word32)(FP_SEG((T_void far *)p_talk)))<<4) +
                 ((T_word32)(FP_OFF((T_void far *)p_talk))) ;

    /* Store the handle for validation later. */
    p_talk->handle = handle ;

    /* Now, look for a vector that we can use. */
    for (vector=VALID_LOW_VECTOR; vector <= VALID_HIGH_VECTOR; vector++)  {
        p_vector = (T_byte8 **)(vector * 4) ;
        if ((!(*p_vector)) || (**p_vector == 0xCF))  {
            /* Found a vector I can use. */
            break ;
        }
    }

    /* Check if the vector is now good. */
    if (vector > VALID_HIGH_VECTOR)  {
        puts("DirectTalk Failure: CANNOT FIND USABLE VECTOR") ;
        exit(1) ;
    }

    /* Vector is good.  Store it. */
    p_talk->vector = vector ;

    /* Install vector. */
    printf("Installing on vector 0x%02X\n", vector) ;
	G_oldVector = getvect(vector);
	setvect(vector, IDirectTalkISR);
    G_interruptInstalled = TRUE ;

    atexit(IDirectTalkUndoISR) ;

    /* Record the new handle we are to use. */
    newHandle = handle ;
#endif

    DebugEnd() ;

    return newHandle ;
#else  /* not ALLOW_DOS_DITALK */
    /* Record the send and receive callbacks. */
    G_receiveCallback = p_callRecv ;
    G_sendCallback = p_callSend ;
    G_connectCallback = p_callConnect ;
    G_disconnectCallback = p_callDisconnect ;
    G_numPackets = 0 ;
    if (handle == 1) {
        // IPX is enabled
        G_ipxEnabled = 1;
    } else {
        G_ipxEnabled = 0;
    }

    /* Just return any ol' handle, just not 0 */
    return (T_directTalkHandle)1 ;
#endif
}

T_void DirectTalkFinish(T_directTalkHandle handle)
{
#ifdef ALLOW_DOS_DITALK
    DebugRoutine("DirectTalkFinish") ;
#ifdef COMPILE_OPTION_DIRECT_TALK_IS_DOS32
    DebugCheck(handle == G_talk->handle) ;
#endif
    /* Mark the structure no longer valid (aka closed out) */
    G_talk->tag = DIRECT_TALK_DEAD_TAG ;

#ifndef COMPILE_OPTION_DIRECT_TALK_IS_DOS32
    /* Stop talking responding to the other process. */
    IDirectTalkUndoISR() ;

    /* Let go of the talk structure. */
    G_talk->tag = DIRECT_TALK_DEAD_TAG ;
    MemFree(G_talk) ;
#endif

    DebugEnd() ;
#else
    T_word16 i ;

    for (i=0; i<G_numPackets; i++)
        MemFree(G_packets[i].p_data) ;

    G_numPackets = 0 ;
#endif
}

#ifdef COMPILE_OPTION_DIRECT_TALK_IS_DOS32
T_void DirectTalkSendData(T_void *p_data, T_byte8 size)
{
#ifdef ALLOW_DOS_DITALK
    union REGS regs ;

    DebugRoutine("DirectTalkSendData") ;

    /* DOS32 has requested to send data to the REAL driver. */
    /* Copy over the request and call the driver. */
    G_talk->command = DIRECT_TALK_COMMAND_SEND ;
    memcpy(G_talk->buffer, p_data, size) ;
    G_talk->bufferLength = size ;
    G_talk->bufferFilled = TRUE ;

    /* Do the actual 32-bit interrupt call. */
    int386(
        G_talk->vector,
        &regs,
        &regs) ;

    DebugEnd() ;
#else /* not ALLOW_DOS_DITALK */
    if (G_ipxEnabled) {
        // Send the data
        IPXSendPacket((const char *)p_data, size);
    } else {
        T_void *p_storeddata ;

        if (G_numPackets < DITALK_MAX_PACKETS)  {
            p_storeddata = (void *)MemAlloc((T_word32)size) ;
            memcpy(p_storeddata, p_data, size) ;
            G_packets[G_numPackets].p_data = p_storeddata ;
            G_packets[G_numPackets].size = size ;
            G_numPackets++ ;
        }
    }
#endif
}

T_void DirectTalkPollData(T_void)
{
#ifdef ALLOW_DOS_DITALK
    union REGS regs ;

    DebugRoutine("DirectTalkPollData") ;

    /* Request that any data that needs to be sent to us is done now. */
    G_talk->command = DIRECT_TALK_COMMAND_RECV ;
    G_talk->bufferFilled = FALSE ;

    /* Do the actual 32-bit interrupt call. */
    int386(
        G_talk->vector,
        &regs,
        &regs) ;

    if (G_talk->bufferFilled)  {
        /* Tell the DOS32 program that it just received data. */
        G_receiveCallback(
            G_talk->buffer,
            G_talk->bufferLength) ;
    }

    DebugEnd() ;
#else /* not ALLOW_DOS_DITALK */
    char buffer[2048];
    unsigned int length;

    if (G_ipxEnabled) {
        if (IPXClientPoll(buffer, &length)) {
            G_receiveCallback(buffer, length);
        }
    } else {
        if (G_numPackets)  {
            G_receiveCallback(
                G_packets[0].p_data,
                G_packets[0].size) ;
            if (G_numPackets > 1)
                memmove(
                    &G_packets[0], 
                    &G_packets[1], 
                    (G_numPackets-1)*sizeof(G_packets[0])) ;
            G_numPackets-- ;
        }
    }

#endif
}

T_void DirectTalkConnect(T_byte8 *p_address)
{
#ifdef ALLOW_DOS_DITALK
    union REGS regs ;

    DebugRoutine("DirectTalkConnect") ;

    /* Copy over the dial info. */
    strcpy(G_talk->buffer, p_address) ;

    /* Request that the driver connects to the server (on a hardward level) */
    G_talk->command = DIRECT_TALK_COMMAND_CONNECT ;
    G_talk->bufferFilled = FALSE ;

    /* Do the actual 32-bit interrupt call. */
    int386(
        G_talk->vector,
        &regs,
        &regs) ;

    DebugEnd() ;
#else /* not ALLOW_DOS_DITALK */
#endif
}

T_void DirectTalkDisconnect(T_void)
{
#ifdef ALLOW_DOS_DITALK
    union REGS regs ;

    DebugRoutine("DirectTalkDisconnect") ;

    /* Request that the driver disconnects from */
    /* the server (on a hardward level) */
    G_talk->command = DIRECT_TALK_COMMAND_DISCONNECT ;
    G_talk->bufferFilled = FALSE ;

    /* Do the actual 32-bit interrupt call. */
    int386(
        G_talk->vector,
        &regs,
        &regs) ;

    DebugEnd() ;
#else /* not ALLOW_DOS_DITALK */
#endif
}

#endif

#ifdef ALLOW_DOS_DITALK
#ifndef COMPILE_OPTION_DIRECT_TALK_IS_DOS32
static void interrupt IDirectTalkISR(T_void)
{
    /* Just received a request to do something. */
    switch(G_talk->command)  {
        case DIRECT_TALK_COMMAND_SEND:
            /* We just received data (SEND = send from DOS32), therefore */
            /* we need to pass this on to the next layer. */
            G_receiveCallback(G_talk->buffer, G_talk->bufferLength) ;
            break ;
        case DIRECT_TALK_COMMAND_RECV:
            /* We just were told to send data.  Let's get some data. */
            G_sendCallback(
                G_talk->buffer,
                &G_talk->bufferLength,
                &G_talk->bufferFilled) ;
            break ;
        case DIRECT_TALK_COMMAND_CONNECT:
            G_connectCallback(G_talk->buffer) ;
            break ;
        case DIRECT_TALK_COMMAND_DISCONNECT:
            G_disconnectCallback() ;
            break ;
        default:
            /* Could stop and output an error, but it is best */
            /* to ignore the request. */
            G_talk->bufferFilled = FALSE ;
            break ;
    }
}

static void IDirectTalkUndoISR(T_void)
{
    /* Restore if there is an old vector. */
    if (G_interruptInstalled)  {
        printf("Uninstall vector 0x%02X\n", G_talk->vector) ;
        setvect(G_talk->vector, G_oldVector) ;
        G_oldVector = NULL ;
        G_interruptInstalled = FALSE ;
    }
}

#endif
#endif /* ALLOW_DOS_DITALK */

E_directTalkLineStatus DirectTalkGetLineStatus(T_void)
{
#ifdef ALLOW_DOS_DITALK
    return G_talk->lineStatus ;
#else
    return DIRECT_TALK_LINE_STATUS_CONNECTED ;
#endif
}

T_void DirectTalkSetLineStatus(E_directTalkLineStatus status)
{
#ifdef ALLOW_DOS_DITALK
printf("line status: %d\n", status) ;
    DebugCheck(status < DIRECT_TALK_LINE_STATUS_UNKNOWN) ;
    G_talk->lineStatus = status ;
#endif
}

/* Get the unique address associated with this talking connection */
/* In reality, this is the IPX number of the hardware. */
/* NOTE:  unique addresses are only unique to the computer, not */
/*        based on each instantiation of direct talk. */
T_void DirectTalkGetUniqueAddress(T_directTalkUniqueAddress *p_unique)
{
#ifdef ALLOW_DOS_DITALK
    T_word16 i ;

    for (i=0; i<sizeof(*p_unique); i++)
        p_unique->address[i] = G_talk->uniqueAddress[i] ;
#else
    /* Return all ones (can't use all zeros, that's a no address) */
    if (G_ipxEnabled) {
        IPXGetUniqueAddress(p_unique->address);
    } else {
        memset(p_unique->address, 1, sizeof(p_unique->address)) ;
    }
#endif
}

T_void DirectTalkSetUniqueAddress(T_directTalkUniqueAddress *p_unique)
{
#ifdef ALLOW_DOS_DITALK
    T_word16 i ;

    for (i=0; i<sizeof(*p_unique); i++)
        G_talk->uniqueAddress[i] = p_unique->address[i] ;
#endif
}

T_directTalkUniqueAddress *DirectTalkGetNullBlankUniqueAddress(T_void)
{
    return &G_blankAddress ;
}

T_void DirectTalkPrintAddress(FILE *fp, T_directTalkUniqueAddress *p_addr)
{
    fprintf(fp, "%02X:%02X:%02X:%02X:%02X:%02X",
        p_addr->address[0],
        p_addr->address[1],
        p_addr->address[2],
        p_addr->address[3],
        p_addr->address[4],
        p_addr->address[5]) ;
}

E_directTalkServiceType DirectTalkGetServiceType(T_void)
{
#ifdef ALLOW_DOS_DITALK
    return G_talk->serviceType ;
#else
    /* No real server out there -- just make a self server */
    if (G_ipxEnabled)
        return DIRECT_TALK_IPX;
    else
        return DIRECT_TALK_SELF_SERVER ;
#endif
}

T_void DirectTalkSetServiceType(E_directTalkServiceType serviceType)
{
#ifdef ALLOW_DOS_DITALK
    G_talk->serviceType = serviceType ;
#endif
}

T_void DirectTalkSetDestination(T_directTalkUniqueAddress *p_dest)
{
#ifdef ALLOW_DOS_DITALK
    /* Destination address */
    if (G_talk)
        memcpy(G_talk->destinationAddress, p_dest, 6) ;
#else
    if (G_ipxEnabled)
        IPXSetDestinationAddress(p_dest->address);
#endif
}

T_void DirectTalkSetDestinationAll(T_void)
{
#ifdef ALLOW_DOS_DITALK
    /* Broadcast message mode */
    if (G_talk)
        memset(G_talk->destinationAddress, 0xFF, 6) ;
#else
    if (G_ipxEnabled)
        IPXSetDestinationAddressAll();
#endif
}

T_byte8 *DirectTalkGetDestination(T_void)
{
#ifdef ALLOW_DOS_DITALK
    return G_talk->destinationAddress ;
#else
    if (G_ipxEnabled)
        return IPXGetDestinationAddress();
    else
        return (T_byte8 *)DirectTalkGetNullBlankUniqueAddress() ;
#endif
}

/****************************************************************************/
/*    END OF FILE:  DITALK.C                                                */
/****************************************************************************/
