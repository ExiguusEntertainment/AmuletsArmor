/****************************************************************************
 * FILE:  DOSDTALK.C
 ****************************************************************************/
#include "DITALK.H"
#include "..\DITALKP.H"
#include "MEMORY.H"

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

/*--------------------------------------------------------------------------*
 * Routine: DirectTalkInit
 *--------------------------------------------------------------------------*/
/**
 * @brief     DirectTalk initialization routine.
 *
 * @param p_callRecv -- Routine to call each time a packet is received.
 * @param p_callSend -- Routine to call each time a packet has been sent or 0.
 * @param p_callConnect -- Routine called when a new connection is made or 0.
 * @param p_callDisconnect -- Routine called when a connection is dropped or 0.
 * @param handle -- Handle to direct talk system
 *
 * @return The handle to the direct talk system.
 */
/*--------------------------------------------------------------------------*/
T_directTalkHandle DirectTalkInit(
           T_directTalkReceiveCallback p_callRecv,
           T_directTalkSendCallback p_callSend,
           T_directTalkConnectCallback p_callConnect,
           T_directTalkDisconnectCallback p_callDisconnect,
           T_directTalkHandle handle)
{
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
    // OLD OLD 16-bit DOS version
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
}

/*--------------------------------------------------------------------------*
 * Routine: DirectTalkFinish
 *--------------------------------------------------------------------------*/
/**
 * @brief Close the DirectTalk system.
 *
 * @param handle Returned direct talk handle created by DirectTalkInit().
 */
/*--------------------------------------------------------------------------*/
T_void DirectTalkFinish(T_directTalkHandle handle)
{
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
}

/*--------------------------------------------------------------------------*
 * Routine: DirectTalkSendData
 *--------------------------------------------------------------------------*/
/**
 * @brief Send a block of data in a packet to the direct talk connection.
 *
 * @param p_data Pointer to a data block.
 * @param size Size of data block (in bytes) to send.
 */
/*--------------------------------------------------------------------------*/
#ifdef COMPILE_OPTION_DIRECT_TALK_IS_DOS32
T_void DirectTalkSendData(T_void *p_data, T_byte8 size)
{
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
}

/*--------------------------------------------------------------------------*
 * Routine: DirectTalkPollData
 *--------------------------------------------------------------------------*/
/**
 * @brief Quickly check the direct talk connection for updates.  This will
 *          call any callbacks that were registered at the DirectTalkInit()
 *          configuration.
 */
/*--------------------------------------------------------------------------*/
T_void DirectTalkPollData(T_void)
{
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
}

/*--------------------------------------------------------------------------*
 * Routine: DirectTalkConnect
 *--------------------------------------------------------------------------*/
/**
 * @brief Connect to a given address.  Sends the DIRECT_TALK_COMMAND_CONNECT
 *          to the driver.
 *          NOTE: Is this used anymore?
 *
 * @param p_address String of address to connect to.
 */
/*--------------------------------------------------------------------------*/
T_void DirectTalkConnect(T_byte8 *p_address)
{
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
}

/*--------------------------------------------------------------------------*
 * Routine: DirectTalkDisconnect
 *--------------------------------------------------------------------------*/
/**
 * @brief Disconnects from the current direct talk connection (if any).
 *          NOTE: Is this used anymore?
 *
 */
/*--------------------------------------------------------------------------*/
T_void DirectTalkDisconnect(T_void)
{
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
}
#endif

#ifndef COMPILE_OPTION_DIRECT_TALK_IS_DOS32
/*--------------------------------------------------------------------------*
 * Routine: IDirectTalkISR
 *--------------------------------------------------------------------------*/
/**
 * @brief   Interrupt ISR for handling Direct Talk events.  Handles sent,
 *          received, connected, and disconnected events.
 *          NOTE: Any routines called from this routine is inside of
 *          an interrupt!  Probably why it was never used anymore.
 */
/*--------------------------------------------------------------------------*/
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

/*--------------------------------------------------------------------------*
 * Routine: IDirectTalkUndoISR
 *--------------------------------------------------------------------------*/
/**
 * @brief   Removes the interrupt ISR in the system.
 */
/*--------------------------------------------------------------------------*/
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

/*--------------------------------------------------------------------------*
 * Routine: DirectTalkGetLineStatus
 *--------------------------------------------------------------------------*/
/**
 * @brief   Returns the current line status (connection status).
 *
 * @return Line/Connection status.
 */
/*--------------------------------------------------------------------------*/
E_directTalkLineStatus DirectTalkGetLineStatus(T_void)
{
    return G_talk->lineStatus ;
}

/*--------------------------------------------------------------------------*
 * Routine: DirectTalkSetLineStatus
 *--------------------------------------------------------------------------*/
/**
 * @brief   Sets the current line status.
 *          NOTE: Why would we use this?  It is NOT used!
 *
 * @return Line/Connection status.
 */
/*--------------------------------------------------------------------------*/
T_void DirectTalkSetLineStatus(E_directTalkLineStatus status)
{
    printf("line status: %d\n", status) ;
        DebugCheck(status < DIRECT_TALK_LINE_STATUS_UNKNOWN) ;
        G_talk->lineStatus = status ;
}

/*--------------------------------------------------------------------------*
 * Routine: DirectTalkGetUniqueAddress
 *--------------------------------------------------------------------------*/
/**
 * @brief   Get the unique address associated with this talking connection.
 *          In reality, this is the IPX number of the hardware.
 *          NOTE:  unique addresses are only unique to the computer, not
 *              based on each instantiation of direct talk.
 *
 * @param p_unique Pointer to place to store the unique address.
 */
/*--------------------------------------------------------------------------*/
T_void DirectTalkGetUniqueAddress(T_directTalkUniqueAddress *p_unique)
{
    T_word16 i ;

    for (i=0; i<sizeof(*p_unique); i++)
        p_unique->address[i] = G_talk->uniqueAddress[i] ;
}

/*--------------------------------------------------------------------------*
 * Routine: DirectTalkSetUniqueAddress
 *--------------------------------------------------------------------------*/
/**
 * @brief   Override the unique address!
 *          NOTE: Not used!
 *
 * @param p_unique Pointer to place of unique address to use.
 */
/*--------------------------------------------------------------------------*/
T_void DirectTalkSetUniqueAddress(T_directTalkUniqueAddress *p_unique)
{
    T_word16 i ;

    for (i=0; i<sizeof(*p_unique); i++)
        G_talk->uniqueAddress[i] = p_unique->address[i] ;
}

/*--------------------------------------------------------------------------*
 * Routine: DirectTalkGetNullBlankUniqueAddress
 *--------------------------------------------------------------------------*/
/**
 * @brief   Return a pointer to a standard blank/unknown unique address.
 *          This is used to compare addresses to see if they are assigned
 *          to a computer or not.
 *
 * @return Unique address of a blank (usually all zero's).
 */
/*--------------------------------------------------------------------------*/
T_directTalkUniqueAddress *DirectTalkGetNullBlankUniqueAddress(T_void)
{
    return &G_blankAddress ;
}

/*--------------------------------------------------------------------------*
 * Routine: DirectTalkPrintAddress
 *--------------------------------------------------------------------------*/
/**
 * @brief   Print a unique address (6 bytes) to the given file.
 *
 * @param fp File or output to write address into
 * @param p_addr Pointer to unique address to display.
 */
/*--------------------------------------------------------------------------*/
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

/*--------------------------------------------------------------------------*
 * Routine: DirectTalkGetServiceType
 *--------------------------------------------------------------------------*/
/**
 * @brief   Get the type of service being used.  This a single enumerated
 *          type that explains what connection type is being used should
 *          special steps be required.
 *
 * @return Type of service offered by this direct talk connection.
 */
/*--------------------------------------------------------------------------*/
E_directTalkServiceType DirectTalkGetServiceType(T_void)
{
    return G_talk->serviceType ;
}

/*--------------------------------------------------------------------------*
 * Routine: DirectTalkSetServiceType
 *--------------------------------------------------------------------------*/
/**
 * @brief   Set the service type being used.
 *          NOTE: Not used!
 *
 * @return Type of service offered by this direct talk connection.
 */
/*--------------------------------------------------------------------------*/
T_void DirectTalkSetServiceType(E_directTalkServiceType serviceType)
{
    G_talk->serviceType = serviceType ;
}

/*--------------------------------------------------------------------------*
 * Routine: DirectTalkSetDestination
 *--------------------------------------------------------------------------*/
/**
 * @brief   Set the destination of the next sent data.
 *
 * @param p_dest -- Unique address of the next packet.
 */
/*--------------------------------------------------------------------------*/
T_void DirectTalkSetDestination(T_directTalkUniqueAddress *p_dest)
{
    /* Destination address */
    if (G_talk)
        memcpy(G_talk->destinationAddress, p_dest, 6) ;
}

/*--------------------------------------------------------------------------*
 * Routine: DirectTalkSetDestinationAll
 *--------------------------------------------------------------------------*/
/**
 * @brief   Declare the next packet to be sent to ALL members on this
 *          direct talk network.  Used to broadcast information to all
 *          members.
 */
/*--------------------------------------------------------------------------*/
T_void DirectTalkSetDestinationAll(T_void)
{
    /* Broadcast message mode */
    if (G_talk)
        memset(G_talk->destinationAddress, 0xFF, 6) ;
}

/*--------------------------------------------------------------------------*
 * Routine: DirectTalkGetDestination
 *--------------------------------------------------------------------------*/
/**
 * @brief   Get the currently declared destination address.
 *          NOTE: Not used!
 */
/*--------------------------------------------------------------------------*/
T_byte8 *DirectTalkGetDestination(T_void)
{
    return G_talk->destinationAddress ;
}

/****************************************************************************
 * END OF FILE:  DOSDTALK.C
 ****************************************************************************/
