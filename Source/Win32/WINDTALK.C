/****************************************************************************/
/*    FILE:  DOSDTALK.C                                                     */
/****************************************************************************/
#include "..\INCLUDE\DITALK.H"
#include "..\DITALKP.H"
#include "MEMORY.H"

/* Callback routines to handle receiving and requests to send. */
static T_directTalkReceiveCallback G_receiveCallback = NULL ;
static T_directTalkSendCallback G_sendCallback = NULL ;
static T_directTalkConnectCallback G_connectCallback = NULL ;
static T_directTalkDisconnectCallback G_disconnectCallback = NULL ;

/* Location of direct talk struct. */
//static T_directTalkStruct *G_talk ;
#define DITALK_MAX_PACKETS  10
typedef struct {
    void *p_data ;
    T_word16 size ;
} T_ditalkRecording ;
static T_ditalkRecording G_packets[DITALK_MAX_PACKETS] ;

static T_word16 G_numPackets = 0 ;
static int G_ipxEnabled = 0;
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
 * @param handle -- Direct talk handle
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
    memset(&G_blankAddress, 0, sizeof(G_blankAddress)) ;

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
    T_word16 i ;

    for (i=0; i<G_numPackets; i++)
        MemFree(G_packets[i].p_data) ;

    G_numPackets = 0 ;
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
T_void DirectTalkSendData(T_void *p_data, T_byte8 size)
{
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
}

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
    return DIRECT_TALK_LINE_STATUS_CONNECTED ;
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
    /* Return all ones (can't use all zeros, that's a no address) */
    if (G_ipxEnabled) {
        IPXGetUniqueAddress(p_unique->address);
    } else {
        memset(p_unique->address, 1, sizeof(p_unique->address)) ;
    }
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
//    T_word16 i ;

//    for (i=0; i<sizeof(*p_unique); i++)
//        G_talk->uniqueAddress[i] = p_unique->address[i] ;
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
    /* No real server out there -- just make a self server */
    if (G_ipxEnabled)
        return DIRECT_TALK_IPX;
    else
        return DIRECT_TALK_SELF_SERVER ;
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
    if (G_ipxEnabled)
        IPXSetDestinationAddress(p_dest->address);
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
    if (G_ipxEnabled)
        IPXSetDestinationAddressAll();
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
    if (G_ipxEnabled)
        return IPXGetDestinationAddress();
    else
        return (T_byte8 *)DirectTalkGetNullBlankUniqueAddress() ;
}

/****************************************************************************/
/*    END OF FILE:  DOSDTALK.C                                              */
/****************************************************************************/
