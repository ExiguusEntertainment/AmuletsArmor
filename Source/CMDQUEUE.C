/****************************************************************************/
/*    FILE:  CMDQUEUE.C                                                     */
/****************************************************************************/
#include "COMM.H"
#include "CMDQUEUE.H"
#include "GENERAL.H"
#include "MEMORY.H"
#include "TICKER.H"

#ifdef COMPILE_OPTION_CREATE_PACKET_DATA_FILE
FILE *G_packetFile ;
#endif

typedef enum {
    PACKET_COMMAND_TYPE_LOSSLESS,
    PACKET_COMMAND_TYPE_LOSSFUL
} E_packetCommandType ;

typedef T_cmdQActionRoutine *T_comQActionList ;

typedef struct T_cmdQPacketStructTag {
    struct T_cmdQPacketStructTag *prev ;
    struct T_cmdQPacketStructTag *next ;
    T_word32 timeToRetry ;
    T_word32 extraData ;
    T_word16 retryTime ;
    T_cmdQPacketCallback p_callback ;
    T_packetLong packet ;
#ifndef NDEBUG
    T_byte8 tag[4] ;
#endif
} T_cmdQPacketStruct ;

typedef struct {
    T_cmdQPacketStruct *first ;
    T_cmdQPacketStruct *last ;
} T_cmdQStruct ;

static E_Boolean G_init = FALSE ;

static T_word16 G_activePort = 0xFFFF ;

static T_cmdQStruct *G_activeCmdQList = NULL ;

/** All callbacks are now set dynamically. **/
static T_cmdQActionRoutine G_cmdQActionList[PACKET_COMMAND_MAX] = {
    NULL,                  /* ACK */
    NULL,                  /* LOGIN */
    NULL,                  /* 2 RETRANSMIT */
    NULL,                  /* MONSTER_MOVE */
    NULL,                  /* PLAYER_ATTACK */
    NULL,                  /* 5 TOWN_UI_MESSAGE */
    NULL,                  /* 6 PLAYER_ID_SELF */
    NULL,                  /* 7 REQUEST_PLAYER_ID */
    NULL,                  /* 8 GAME_REQUEST_JOIN */
    NULL,                  /* 9 GAME_RESPOND_JOIN */
    NULL,                  /* 10 GAME_START */
    NULL,                  /* FIREBALL_STOP */
    NULL,                  /* MOVE_CREATURE */
    NULL,                  /* SC_DAMAGE */
    NULL,                  /* CREATURE_ATTACK */
    NULL,                  /* CREATURE_HURT */
    NULL,                  /* CREATURE_DEAD */
    NULL,                  /* REVERSE_SECTOR */
    NULL,                  /* SYNC */
    NULL,                  /* PICK_UP */
    NULL,                  /* MESSAGE */
    NULL,                  /* OPEN_DOOR */
    NULL,                  /* CANNED_SAYING */
    NULL,                  /* SC_OBJECT_POSITION */
    NULL,                  /* CS_REQUEST_TAKE */
    NULL,                  /* SC_TAKE_REPLY */
    NULL,                  /* CSC_ADD_OBJECT */
    NULL,                  /* SC_DESTROY_OBJECT */
    NULL,                  /* SC_SPECIAL_EFFECT */
    NULL,                  /* SC_PLACE_START */
    NULL,                  /* CSC_GOTO_PLACE */
    NULL,                  /* CS_GOTO_SUCCEEDED */
    NULL,                  /* RT_REQUEST_FILE */
    NULL,                  /* TR_START_TRANSFER */
    NULL,                  /* TR_DATA_PACKET */
    NULL,                  /* TR_FINAL_PACKET */
    NULL,                  /* RT_RESEND_PLEASE */
    NULL,                  /* RT_TRANSFER_COMPLETE */
    NULL,                  /* RT_TRANSFER_CANCEL */
    NULL,                  /* 40 TR_FILE_NOT_HERE */

    NULL,                  /* 41 CSC_REQUEST_MEMORY_TRANSFER */
    NULL,                  /* 42 CSC_MEMORY_TRANSFER_READY */
    NULL,                  /* 43 CSC_MEMORY_TRANSFER_DATA */

    NULL,                  /* 44 CSC_CHANGE_BODY_PART */
    NULL,                  /* 45 CSC_PING */
    NULL,                  /* 46 SC_WALL_STATE_CHANGE */
    NULL,                  /* 47 SC_SIDE_STATE_CHANGE */
    NULL,                  /* 48 SC_SECTOR_STATE_CHANGE */
    NULL,                  /* 49 SC_GROUP_STATE_CHANGE */

    NULL,                  /* 50 SC_EXPERIENCE */
    NULL,                  /* 51 CS_REQUEST_SERVER_ID */
    NULL,                  /* 52 SC_SERVER_ID */
    NULL,                  /* 53 CS_REQUEST_ENTER */
    NULL,                  /* 54 SC_REQUEST_ENTER_STATUS */
    NULL,                  /* 55 CS_REQUEST_CHAR_LIST */

    NULL,                  /* 56 CS_LOAD_CHARACTER */
    NULL,                  /* 57 SC_LOAD_CHARACTER_STATUS */
    NULL,                  /* 58 CS_CREATE_CHARACTER */
    NULL,                  /* 59 SC_CREATE_CHARACTER_STATUS */
    NULL,                  /* 60 CS_DELETE_CHARACTER */
    NULL,                  /* 61 SC_DELETE_CHARACTER_STATUS */

    NULL,                  /* 62 CS_CHECK_PASSWORD */
    NULL,                  /* 63 SC_CHECK_PASSWORD_STATUS */
    NULL,                  /* 64 CS_CHANGE_PASSWORD */
    NULL,                  /* 65 SC_CHANGE_PASSWORD_STATUS */
    NULL,                  /* 66 CSC_REQUEST_DATA_BLOCK */
    NULL,                  /* 67 CSC_DAMAGE_OBJECT */
    NULL,                  /* 68 SC_REQUEST_PIECEWISE_LIST */
    NULL,                  /* 69 CS_PIECEWISE_LIST */

    NULL,                  /* 70 CSC_STORE_ADD_ITEM */
    NULL,                  /* 71 CSC_STORE_REMOVE_ITEM */
    NULL,                  /* 72 SC_STORE_ADD_RESULT */
    NULL,                  /* 73 SC_STORE_REMOVE_RESULT */
} ;

static E_packetCommandType G_CmdQTypeCommand[PACKET_COMMAND_MAX] = {
    PACKET_COMMAND_TYPE_LOSSFUL,
    PACKET_COMMAND_TYPE_LOSSLESS,
    PACKET_COMMAND_TYPE_LOSSLESS,               /* 2 RETRANSMIT */
    PACKET_COMMAND_TYPE_LOSSFUL, /**/
    PACKET_COMMAND_TYPE_LOSSLESS, /* LESS */     /* PLAYER_ATTACK */
    PACKET_COMMAND_TYPE_LOSSLESS,               /* 5 TOWN_UI_MESSAGE */
    PACKET_COMMAND_TYPE_LOSSLESS,               /* 6 PLAYER_ID_SELF */
    PACKET_COMMAND_TYPE_LOSSLESS,               /* 7 REQUEST_PLAYER_ID */
    PACKET_COMMAND_TYPE_LOSSLESS,               /* 8 GAME_REQUEST_JOIN */
    PACKET_COMMAND_TYPE_LOSSLESS,               /* 9 GAME_RESPOND_JOIN */
    PACKET_COMMAND_TYPE_LOSSLESS,               /* 10 GAME START */
    PACKET_COMMAND_TYPE_LOSSLESS,
    PACKET_COMMAND_TYPE_LOSSFUL, /**/
    PACKET_COMMAND_TYPE_LOSSFUL, /* LESS */     /* SC_DAMAGE */
    PACKET_COMMAND_TYPE_LOSSLESS,
    PACKET_COMMAND_TYPE_LOSSLESS,
    PACKET_COMMAND_TYPE_LOSSLESS,
    PACKET_COMMAND_TYPE_LOSSLESS,
    PACKET_COMMAND_TYPE_LOSSFUL,                /* 18 SYNC */
    PACKET_COMMAND_TYPE_LOSSLESS,
    PACKET_COMMAND_TYPE_LOSSLESS,
    PACKET_COMMAND_TYPE_LOSSLESS,
    PACKET_COMMAND_TYPE_LOSSLESS,
    PACKET_COMMAND_TYPE_LOSSFUL,
    PACKET_COMMAND_TYPE_LOSSLESS,
    PACKET_COMMAND_TYPE_LOSSLESS,
    PACKET_COMMAND_TYPE_LOSSLESS,
    PACKET_COMMAND_TYPE_LOSSLESS,
    PACKET_COMMAND_TYPE_LOSSFUL, /* LESS */     /* SC_SPECIAL_EFFECT */
    PACKET_COMMAND_TYPE_LOSSLESS,               /* SC_PLACE_START */
    PACKET_COMMAND_TYPE_LOSSLESS,               /* CSC_GOTO_PLACE */
    PACKET_COMMAND_TYPE_LOSSLESS,               /* CS_GOTO_SUCCEEDED */
    PACKET_COMMAND_TYPE_LOSSLESS,               /* CSC_PROJECTILE_CREATE */
    PACKET_COMMAND_TYPE_LOSSLESS,               /* RT_REQUEST_FILE */
    PACKET_COMMAND_TYPE_LOSSLESS,               /* TR_START_TRANSFER */
    PACKET_COMMAND_TYPE_LOSSLESS,               /* TR_DATA_PACKET */
    PACKET_COMMAND_TYPE_LOSSLESS,               /* TR_FINAL_PACKET */
    PACKET_COMMAND_TYPE_LOSSLESS,               /* RT_RESEND_PLEASE */
    PACKET_COMMAND_TYPE_LOSSLESS,               /* RT_TRANSFER_COMPLETE */
    PACKET_COMMAND_TYPE_LOSSLESS,               /* RT_TRANSFER_CANCEL */
    PACKET_COMMAND_TYPE_LOSSLESS,               /* TR_FILE_NOT_HERE */

    PACKET_COMMAND_TYPE_LOSSLESS,               /* 41 CSC_REQUEST_MEMORY_TRANSFER */
    PACKET_COMMAND_TYPE_LOSSLESS,               /* 42 CSC_MEMORY_TRANSFER_READY */
    PACKET_COMMAND_TYPE_LOSSLESS,               /* 43 CSC_MEMORY_TRANSFER_DATA */

    PACKET_COMMAND_TYPE_LOSSLESS,               /* 44 CSC_CHANGE_BODY_PART */
    PACKET_COMMAND_TYPE_LOSSLESS,               /* 45 CSC_PING */
    PACKET_COMMAND_TYPE_LOSSLESS,               /* 46 SC_WALL_STATE_CHANGE */
    PACKET_COMMAND_TYPE_LOSSLESS,               /* 47 SC_SIDE_STATE_CHANGE */
    PACKET_COMMAND_TYPE_LOSSLESS,               /* 48 SC_SECTOR_STATE_CHANGE */
    PACKET_COMMAND_TYPE_LOSSLESS,               /* 49 SC_GROUP_STATE_CHANGE */

    PACKET_COMMAND_TYPE_LOSSLESS,               /* 50 SC_EXPERIENCE */
    PACKET_COMMAND_TYPE_LOSSLESS,               /* 51 CS_REQUEST_SERVER_ID */
    PACKET_COMMAND_TYPE_LOSSLESS,               /* 52 SC_SERVER_ID */
    PACKET_COMMAND_TYPE_LOSSLESS,               /* 53 CS_REQUEST_ENTER */
    PACKET_COMMAND_TYPE_LOSSLESS,               /* 54 SC_REQUEST_ENTER_STATUS */
    PACKET_COMMAND_TYPE_LOSSLESS,               /* 55 CS_REQUEST_CHAR_LIST */

    PACKET_COMMAND_TYPE_LOSSLESS,               /* 56 CS_LOAD_CHARACTER */
    PACKET_COMMAND_TYPE_LOSSLESS,               /* 57 SC_LOAD_CHARACTER_STATUS */
    PACKET_COMMAND_TYPE_LOSSLESS,               /* 58 CS_CREATE_CHARACTER */
    PACKET_COMMAND_TYPE_LOSSLESS,               /* 59 SC_CREATE_CHARACTER_STATUS */
    PACKET_COMMAND_TYPE_LOSSLESS,               /* 60 CS_DELETE_CHARACTER */
    PACKET_COMMAND_TYPE_LOSSLESS,               /* 61 SC_DELETE_CHARACTER_STATUS */

    PACKET_COMMAND_TYPE_LOSSLESS,               /* 62 CS_CHECK_PASSWORD */
    PACKET_COMMAND_TYPE_LOSSLESS,               /* 63 SC_CHECK_PASSWORD_STATUS */
    PACKET_COMMAND_TYPE_LOSSLESS,               /* 64 CS_CHANGE_PASSWORD */
    PACKET_COMMAND_TYPE_LOSSLESS,               /* 65 SC_CHANGE_PASSWORD_STATUS */
    PACKET_COMMAND_TYPE_LOSSLESS,               /* 66 CSC_REQUEST_DATA_BLOCK */
    PACKET_COMMAND_TYPE_LOSSLESS,               /* 67 CSC_DAMAGE_OBJECT */
    PACKET_COMMAND_TYPE_LOSSLESS,               /* 68 SC_REQUEST_PIECEWISE_LIST */
    PACKET_COMMAND_TYPE_LOSSLESS,               /* 69 CS_PIECEWISE_LIST */

    PACKET_COMMAND_TYPE_LOSSLESS,               /* 70 CSC_STORE_ADD_ITEM */
    PACKET_COMMAND_TYPE_LOSSLESS,               /* 71 CSC_STORE_REMOVE_ITEM */
    PACKET_COMMAND_TYPE_LOSSLESS,               /* 72 SC_STORE_ADD_RESULT */
    PACKET_COMMAND_TYPE_LOSSLESS,               /* 73 SC_STORE_REMOVE_RESULT */
} ;

static T_cmdQStruct G_cmdQueues[MAX_COMM_PORTS][PACKET_COMMAND_MAX];

typedef struct {
    T_word32 time ;
    T_word32 throughput ;
    T_byte8 commandPos ;
} T_cmdQueueInfo ;

static T_cmdQueueInfo G_cmdQueueInfo[MAX_COMM_PORTS];

static T_cmdQueueInfo *P_activeQueueInfo = NULL ;

/** CMDQUEUE now controls the packet ID's. **/
static T_word32 G_nextPacketId = 0;

/* Internal prototypes: */
T_void ICmdQUpdateSendForPort(T_void) ;

T_void ICmdQDiscardPacket(T_byte8 commandNum) ;

T_void ICmdQSendPacket(
            T_packetEitherShortOrLong *p_packet,
            T_word16 retryTime,
            T_word32 extraData,
            T_cmdQPacketCallback p_callback) ;

static T_void ICmdQClearPort(T_void) ;

#ifndef NDEBUG
T_word32 G_packetsAlloc = 0 ;
T_word32 G_packetsFree = 0 ;
#endif

/****************************************************************************/
/*  Routine:  CmdQInitialize                                                */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    CmdQInitialize clears out all the variables that are necessary for    */
/*  the CmdQ module.                                                        */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    This routine needs to be called AFTER all ports are opened.           */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/20/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void CmdQInitialize(T_void)
{
    DebugRoutine("CmdQInitialize") ;
    DebugCheck(G_init == FALSE) ;

    /* Note that we are now initialized. */
    G_init = TRUE ;

#ifdef COMPILE_OPTION_CREATE_PACKET_DATA_FILE
    G_packetFile = fopen("packets.dat", "w") ;
#endif

    /* Clear some of those global variables. */
    memset(G_cmdQueues, 0, sizeof(G_cmdQueues)) ;
    memset(G_cmdQueueInfo, 0, sizeof (G_cmdQueueInfo)) ;

#if 0
    /* Let's see how many ports there are. */
    numPorts = CommGetNumberPorts() ;

    /* Go through to each port and set it up. */
    for (port=0; port<numPorts; port++)  {
        /* Set the active port. */
        CommSetActivePortN(port) ;
    }
#endif

    /* Make the active port an illegal port. */
    G_activePort = 0xFFFF ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routines: CmdQRegisterClientCallbacks                                   */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    These functions each take a pointer to an array of function pointers, */
/*  with PACKET_COMMAND_MAX entries.  ..ServerCallbacks sets this pointer   */
/*  as the list of server packet callbacks, and ..ClientCallbacks sets this */
/*  as the list of client packet callbacks.                                 */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    AMT  08/03/95  Created                                                */
/*                                                                          */
/****************************************************************************/
T_void CmdQRegisterClientCallbacks (T_cmdQActionRoutine *callbacks)
{
   T_word16 i;

   DebugRoutine ("CmdQRegisterClientCallbacks");

   for (i=0; i < PACKET_COMMAND_MAX; i++)
      G_cmdQActionList[i] = callbacks[i];

   DebugEnd ();
}

/****************************************************************************/
/*  Routine:  CmdQFinish                                                    */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    CmdQFinish unallocates any memory or structures that are no longer    */
/*  need by the CmdQ module.                                                */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/20/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void CmdQFinish(T_void)
{
    DebugRoutine("CmdQFinish") ;
    DebugCheck(G_init == TRUE) ;

    /* Note that we are now finished. */
    G_init = FALSE ;

#ifdef COMPILE_OPTION_CREATE_PACKET_DATA_FILE
    fclose(G_packetFile) ;
#endif

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  CmdQSetActivePortNum                                          */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    CmdQSetActivePortNum sets up the Cmd Queue module for the given       */
/*  port (and makes the given port the active communications port).         */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word16 num                -- Number of port to make active.         */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/20/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void CmdQSetActivePortNum(T_word16 num)
{
    DebugRoutine("CmdQSetActivePortNum") ;
    DebugCheck(G_init == TRUE) ;

    /* Go ahead and set the active port. */
//    CommSetActivePortN(num) ;
    G_activePort = num ;

    /* Declare which list of command queues we want active (for this port). */
    G_activeCmdQList = &G_cmdQueues[num][0] ;

    /* Set a poiner to that queue's overall information. */
    P_activeQueueInfo = &G_cmdQueueInfo[num] ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  CmdQGetActivePortNum                                          */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    CmdQGetActivePortNum returns the number of the active port being      */
/*  used.                                                                   */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_word16                    -- Number of port.                        */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/20/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_word16 CmdQGetActivePortNum(T_void)
{
    T_word16 portNum ;

    DebugRoutine("CmdQGetActivePortNum") ;
    DebugCheck(G_init == TRUE) ;

    /* Just get it. */
    portNum = G_activePort ;

    DebugEnd() ;

    return portNum ;
}

/****************************************************************************/
/*  Routine:  CmdQSendShortPacket                                           */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    CmdQSendShortPacket prepares a short packet for sending and sends     */
/*  it over to ICmdQSendPacket for processing.                              */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_packetShort *p_packet     -- Packet to send                         */
/*                                                                          */
/*    T_word16 retryTime          -- How long to wait for acknowledgement   */
/*                                   before trying to send again.           */
/*                                                                          */
/*    T_word32 extraData          -- Extra data for the callback routine.   */
/*                                                                          */
/*    T_cmdQPacketCallback *p_callback -- Routine to be called when either  */
/*                                        the packet is lost or sent.       */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/20/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void CmdQSendShortPacket(
            T_packetShort *p_packet,
            T_word16 retryTime,
            T_word32 extraData,
            T_cmdQPacketCallback p_callback)
{
    DebugRoutine("CmdQSendShortPacket") ;
    DebugCheck(p_packet != NULL) ;

    p_packet->header.packetLength = SHORT_PACKET_LENGTH ;
    ICmdQSendPacket(
        (T_packetEitherShortOrLong *)p_packet,
        retryTime,
        extraData,
        p_callback) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  CmdQSendLongPacket                                            */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    CmdQSendLongPacket  prepares a long  packet for sending and sends     */
/*  it over to ICmdQSendPacket for processing.                              */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_packetLong  *p_packet     -- Packet to send                         */
/*                                                                          */
/*    T_word16 retryTime          -- How long to wait for acknowledgement   */
/*                                   before trying to send again.           */
/*                                                                          */
/*    T_word32 extraData          -- Extra data for the callback routine.   */
/*                                                                          */
/*    T_cmdQPacketCallback *p_callback -- Routine to be called when either  */
/*                                        the packet is lost or sent.       */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/20/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void CmdQSendLongPacket(
            T_packetLong *p_packet,
            T_word16 retryTime,
            T_word32 extraData,
            T_cmdQPacketCallback p_callback)
{
    DebugRoutine("CmdQSendLongPacket") ;
    DebugCheck(p_packet != NULL) ;

    p_packet->header.packetLength = LONG_PACKET_LENGTH ;
    ICmdQSendPacket(
        (T_packetEitherShortOrLong *)p_packet,
        retryTime,
        extraData,
        p_callback) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ICmdQSendPacket                    * INTERNAL *               */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ICmdQSendPacket does all the work of placing a packet in one of the   */
/*  active command queues and prepares it for sending out.                  */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_packetEitherShortOrLong *p_packet -- Packet to send                 */
/*                                                                          */
/*    T_word16 retryTime          -- How long to wait for acknowledgement   */
/*                                   before trying to send again.           */
/*                                                                          */
/*    T_word32 extraData          -- Extra data for the callback routine.   */
/*                                                                          */
/*    T_cmdQPacketCallback *p_callback -- Routine to be called when either  */
/*                                        the packet is lost or sent.       */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/20/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ICmdQSendPacket(
            T_packetEitherShortOrLong *p_packet,
            T_word16 retryTime,
            T_word32 extraData,
            T_cmdQPacketCallback p_callback)
{
    T_byte8 command ;
    T_cmdQPacketStruct *p_cmdPacket ;
    DebugRoutine("ICmdQSendPacket") ;

    DebugCheck (G_init == TRUE);

    /* All we have to do is put the packet in the queue's list of packets. */
    /* That's all.  We'll let CmdQUpdateAllSends do the actual processing. */

    /* First, what command are we sending? */
    command = p_packet->data[0] ;
    DebugCheck(command < PACKET_COMMAND_UNKNOWN) ;

    /** Give the packet a unique packet ID **/
    PacketSetId (
        p_packet,
        G_nextPacketId++);

    /* Allocate a structure to hold the information plus more. */
    p_cmdPacket = MemAlloc(sizeof(T_cmdQPacketStruct)) ;

#ifndef NDEBUG
    G_packetsAlloc++ ;
#endif

    /* clear out this packet. */
    memset(p_cmdPacket, 0, sizeof(T_cmdQPacketStruct)) ;
#ifndef NDEBUG
    strcpy(p_cmdPacket->tag, "CmQ") ;
#endif

    /* Copy in the packet data. */
    p_cmdPacket->packet = *((T_packetLong *)p_packet) ;

    /* Now attack this new packet to the appropriate command list. */
    p_cmdPacket->next = G_activeCmdQList[command].first ;

    /* Update the first and last links. */
    if (G_activeCmdQList[command].first != NULL)  {
        G_activeCmdQList[command].first->prev = p_cmdPacket ;
    } else {
        G_activeCmdQList[command].last = p_cmdPacket ;
    }

    /* Declare this as the new first. */
    G_activeCmdQList[command].first = p_cmdPacket ;

    /* Fill in given data. */
    p_cmdPacket->extraData = extraData ;
    p_cmdPacket->p_callback = p_callback ;
    p_cmdPacket->retryTime = retryTime ;

    /* Put in a time of zero to denote first sending. */
    p_cmdPacket->timeToRetry = 0 ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  CmdQUpdateAllSends                                            */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    CmdQUpdateAllSends goes through all the ports and determines what     */
/*  data needs to be sent and what should wait.                             */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/20/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void CmdQUpdateAllSends(T_void)
{
    DebugRoutine("CmdQUpdateAllSends") ;

#if 0
    /* Let's see how many ports there are. */
    numPorts = CommGetNumberPorts() ;

    /* Go through to each port looking for incoming data. */
    for (port=0; port<numPorts; port++)  {
        /* Set the active port (and any additional information). */
        CmdQSetActivePortNum(port) ;

        ICmdQUpdateSendForPort() ;
    }
#else
    CmdQSetActivePortNum(0) ;
    ICmdQUpdateSendForPort() ;
#endif

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  CmdQUpdateAllReceives                                         */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    CmdQUpdateAllReceives goes through all the ports looking for data     */
/*  to take in.  All data that is received is then processed and the        */
/*  appropriate action for the command is called.                           */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/20/95  Created                                                */
/*    AMT  07/15/95  Modified so it doesn't call the callback for the same  */
/*                   packet twice (in case the other end sent it again)     */
/*                                                                          */
/****************************************************************************/

T_void CmdQUpdateAllReceives(T_void)
{
    T_packetLong packet ;
    T_sword16 status ;
    T_byte8 command ;
    T_byte8 ackCommand ;
    T_word32 packetId ;
    T_packetShort ackPacket ;
    E_Boolean packetOkay;
    T_word16 port ;
    T_word16 numPorts ;

    DebugRoutine("CmdQUpdateAllReceives") ;
    INDICATOR_LIGHT(260, INDICATOR_GREEN) ;
    DebugCheck(G_init == TRUE) ;

    /* Let's see how many ports there are. */
    DebugCheckValidStack() ;
//    numPorts = CommGetNumberPorts() ;
    numPorts = 1 ;
    DebugCheckValidStack() ;

    /* Go through to each port looking for incoming data. */
    for (port=0; port<numPorts; port++)  {
        /* Set the active port (and any additional information). */
        DebugCheckValidStack() ;

        /* Hunting for a bug... */
        /* ... . */

        CmdQSetActivePortNum(port) ;
        DebugCheckValidStack() ;

        /* Loop while there are packets to get. */
        do {
            DebugCompare("CmdQUpdateAllReceives") ;
            DebugCheckValidStack() ;
            /* Try getting a packet. */
            status = PacketGet(&packet) ;
            DebugCheckValidStack() ;

            /* Did we get a packet? */
            if (status == 0)  {
                /* Yes, we did.  See what command is being issued. */
                command = packet.data[0] ;

                /* Make sure it is a legal commands.  Unfortunately, */
                /* we'll have to ignore those illegal commands. */
                if (command < PACKET_COMMAND_UNKNOWN)  {
                    /* Is it an ACK packet? */


//printf("R(%d) %2d %ld %ld\n", CmdQGetActivePortNum(), packet.data[0], packet.header.id, TickerGet()) ;  fflush(stdout) ;
#ifdef COMPILE_OPTION_CREATE_PACKET_DATA_FILE
fprintf(G_packetFile, "R(%d) %2d %ld %ld\n", CmdQGetActivePortNum(), packet.data[0], packet.header.id, SyncTimeGet()) ; fflush(G_packetFile) ;
#endif

                    if (command == PACKET_COMMAND_ACK)  {
                        /* Yes, it is an ack.  See what command it is */
                        /* acknowledging. */
                        ackCommand = packet.data[1] ;

                        /* Is that a valid command? */
                        if (ackCommand < PACKET_COMMAND_UNKNOWN)  {
                            INDICATOR_LIGHT(264, INDICATOR_GREEN) ;
                            /* Yes.  But is it a lossless command? */
                            if (G_CmdQTypeCommand[ackCommand] ==
                                PACKET_COMMAND_TYPE_LOSSLESS)  {
                                /* Get the packet id. */
                                packetId = *((T_word32 *)(&(packet.data[2]))) ;
                                /* Is there a list at that point? */
                                if (G_activeCmdQList[ackCommand].last != NULL)  {
                                    /* Yes.  Is the ack for the same packet */
                                    /* waiting? */
                                    if (G_activeCmdQList[ackCommand].last->packet.header.id == packetId)  {
                                        /* Yes. We can now discard it. */
                                        DebugCheckValidStack() ;
                                        ICmdQDiscardPacket(ackCommand) ;
                                        DebugCheckValidStack() ;
                                    }
                                }
                            }
                            INDICATOR_LIGHT(264, INDICATOR_RED) ;
                        }
                    } else {
                        /* No, do the normal action. */

                        /** If this is a lossful packet, always call the **/
                        /** callback routine. **/
                        packetOkay = TRUE;

                        /* Is this a lossless command? */
                        if (G_CmdQTypeCommand[command] ==
                                PACKET_COMMAND_TYPE_LOSSLESS)  {
                            INDICATOR_LIGHT(268, INDICATOR_GREEN) ;
                            /* Yes, it is.  We need to send an ACK that */
                            /* we got it. */
                            /* Make an ack packet with the packet's */
                            /* command and id we received. */
                            ackPacket.data[0] = PACKET_COMMAND_ACK ;
                            ackPacket.data[1] = command ;
                            *((T_word32 *)(&ackPacket.data[2])) =
                                packet.header.id ;
                            /* Send it!  Note that we go through our */
                            /* routines. */
                            INDICATOR_LIGHT(272, INDICATOR_GREEN) ;
                            DebugCheckValidStack() ;
                            CmdQSendShortPacket(
                                &ackPacket,
                                140,  /* Once two seconds is plenty fast */
                                0,  /* No extra data since no callback. */
                                NULL) ;  /* No callback. */
                            INDICATOR_LIGHT(272, INDICATOR_RED) ;
                            DebugCheckValidStack() ;

                            INDICATOR_LIGHT(268, INDICATOR_RED) ;
                        }
                        /* IF the packet is a new packet or a lossful one, */
                        /* we'll go ahead and do the appropriate action */
                        /* on this side. */
//printf("Considering %p [%d]\n", G_cmdQActionList[command], command) ;
                        if ((G_cmdQActionList[command] != NULL) &&
                            (packetOkay == TRUE))  {
                            /* Call the appropriate action item. */
                            INDICATOR_LIGHT(276, INDICATOR_GREEN) ;
//printf("Did go: (%d) %d, %p\n", command, packetOkay, G_cmdQActionList[command]) ; fflush(stdout) ;
                            DebugCheckValidStack() ;
                            G_cmdQActionList[command]
                                   ((T_packetEitherShortOrLong *)&packet) ;
                            DebugCheckValidStack() ;
                            INDICATOR_LIGHT(276, INDICATOR_RED) ;
                            DebugCompare("CmdQUpdateAllReceives") ;
                        } else {
//printf("Didn't go: (%d) %d, %p\n", command, packetOkay, G_cmdQActionList[command]) ; fflush(stdout) ;
                        }
                    }
                }
            }
            DebugCheckValidStack() ;
        } while (status == 0) ;
    }
    DebugEnd() ;

    INDICATOR_LIGHT(260, INDICATOR_RED) ;
}

/****************************************************************************/
/*  Routine:  ICmdQUpdateSendForPort             * INTERNAL *               */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ICmdQUpdateSendForPort updates the sending of all packets for this    */
/*  single port (the currently active port).  Packets that need to be       */
/*  transferred are sent.  Those that don't, don't.  The amount of packets  */
/*  sent depends on the baud rate and the last time this routine was        */
/*  called for this port.                                                   */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/20/95  Created                                                */
/*    LES  05/22/95  Fixed round robin bug.                                 */
/*                   Tried to get speed of modem to be self modifying.      */
/*    AMT  07/15/95  Made it take care of packet ID numbers.                */
/*                                                                          */
/****************************************************************************/

T_void ICmdQUpdateSendForPort(T_void)
{
    T_byte8 currentCmd ;
    T_cmdQPacketStruct *p_packet ;
    T_byte8 packetLength ;
    T_word32 time ;
    T_sword16 status ;
    E_Boolean sentOne ;
    T_word16 bytesused ;
    T_word32 maxOutput ;
    E_Boolean onceFlag = FALSE ;
    E_Boolean sentAny ;

    DebugRoutine("ICmdQUpdateSendForPort") ;

    /* Get the current time. */
    time = TickerGet() ;

    bytesused = 0 ;
    maxOutput = 100 ;
    currentCmd = 0 ;
    if (bytesused < maxOutput)  {
        do {
            sentAny = FALSE ;
            do {
                sentOne = FALSE ;
                /* Try sending the command at currentCmd. */
                /* See if there is anything that needs to be sent. */
                while ((G_activeCmdQList[currentCmd].last != NULL) &&
                       (bytesused < maxOutput))  {
                    DebugCheck(currentCmd < PACKET_COMMAND_UNKNOWN) ;
                    /* Yes, there appears to be a packet at the end of the list. */
                    /* Get a hold on that packet struct. */
                    p_packet = G_activeCmdQList[currentCmd].last ;
#ifndef NDEBUG
                    if (strcmp(p_packet->tag, "CmQ") != 0)  {
                        printf("Bad packet %p\n", p_packet) ;
                        DebugCheck(FALSE) ;
                    }
#endif

                    /* See if it is time to send it. */
                    /** (It's always time to send an ACK.) **/
                    if ((currentCmd == PACKET_COMMAND_ACK) ||
                        (p_packet->timeToRetry < time))  {
                         /* Yes, it is time to send it. */
                         packetLength = p_packet->packet.header.packetLength ;
                         /* Add the count to the output. */
                         bytesused += packetLength+sizeof(T_packetHeader) ;

                         /* Make sure this didn't go over the threshold. */
                         if (bytesused >= maxOutput)
                             break ;

                         /* Let's send that packet. */
                         DirectTalkSetDestinationAll() ;
                         status = PacketSend((T_packetEitherShortOrLong *)
                                                (&p_packet->packet)) ;
                         sentAny = TRUE ;

#ifdef COMPILE_OPTION_CREATE_PACKET_DATA_FILE
fprintf(G_packetFile, "S(%d) cmd=%2d, id=%ld, time=%ld\n", CmdQGetActivePortNum (), p_packet->packet.data[0], p_packet->packet.header.id, SyncTimeGet()) ; fflush(G_packetFile) ;
#endif
                         /* Was the packet actually sent? */
                         if (status == 0)  {
                             /* Packet was sent correctly (as far as we know). */
                             /* Is this a lossful or lossless command queue? */
                             if (G_CmdQTypeCommand[currentCmd] ==
                                    PACKET_COMMAND_TYPE_LOSSFUL)  {
                                 /* Lossful.  Means we can go ahead and */
                                 /* discard this packet. */
                                 ICmdQDiscardPacket(currentCmd) ;
                             } else {
                                 /* Lossless.  Means we must wait for an ACK */
                                 /* packet to confirm that we were sent. */
                                 /* Until then, we can't discard the packet. */
                                 /* But we might have to resend latter. */
                                 /* Set up the retry time. */
                                 p_packet->timeToRetry = time +
                                                          p_packet->retryTime ;
                             }

                             /* Note that we sent the packet (or at least) */
                             /* gave a good try. */
                             sentOne = TRUE ;
                         } else {
                             /* Packet was NOT sent correctly. */
                             /** -- IFF the packet is lossless... **/
                             /* We'll have to retry later.  Change the time, */
                             /* but don't remove it from the linked list. */
                             if (G_CmdQTypeCommand[currentCmd] ==
                                 PACKET_COMMAND_TYPE_LOSSLESS)  {
                                 p_packet->timeToRetry = time +
                                                        p_packet->retryTime ;
                             } else {
                                 /* Waste it!  We can't wait around */
                                 ICmdQDiscardPacket(currentCmd) ;
                             }
                         }
                    } else {
                         /* Don't loop if it is not time to send. */
                         break ;
                    }

		            /** Now, if we aren't checking the ACK queue, we need **/
		            /** to break after the first send.  If we are, we **/
		            /** want to dump everything in the queue. **/
                    if (currentCmd != PACKET_COMMAND_ACK)
                        break ;
                }

                /* Update to the next command. */
                currentCmd++ ;
            } while (currentCmd < PACKET_COMMAND_MAX) ;

        } while ((bytesused < maxOutput) && (sentAny == TRUE)) ;
    }

    P_activeQueueInfo->commandPos = currentCmd ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ICmdQDiscardPacket                                            */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ICmdQDiscardPacket removes a packet from the given command queue in   */
/*  the current command queue list.                                         */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_byte8 commandNum          -- Command in command list to do a        */
/*                                   packet discard on.                     */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/20/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ICmdQDiscardPacket(T_byte8 commandNum)
{
    T_cmdQStruct *p_cmdQ ;
    T_cmdQPacketStruct *p_packet ;

    DebugRoutine("ICmdQDiscardPacket") ;
    DebugCheck(commandNum < PACKET_COMMAND_UNKNOWN) ;

    /* Get a quick pointer to the command's queue. */
    p_cmdQ = &G_activeCmdQList[commandNum] ;

    p_packet = p_cmdQ->last ;
    /* Make sure there is a last entry. */
    DebugCheck(p_packet != NULL) ;

    if (p_packet != NULL)  {
#ifndef NDEBUG
        /* Check for the tag. */
        if (strcmp(p_packet->tag, "CmQ") != 0)  {
            printf("Bad packet %p\n", p_packet) ;
            DebugCheck(FALSE) ;
        }
#endif
        DebugCheck(p_packet->prev != p_packet) ;
        p_cmdQ->last = p_packet->prev ;
        if (p_cmdQ->last == NULL)
            p_cmdQ->first = NULL ;
        else {
            p_cmdQ->last->next = NULL ;
        }

        /* Before we free it, we need to call the callback routine for */
        /* this packet to say, "Hey!  We're done with you." */
        /* Of course, we must check to see if there is a callback routine. */
        if (p_packet->p_callback != NULL)  {
            p_packet->p_callback(
                p_packet->extraData,
                (T_packetEitherShortOrLong *)&p_packet->packet) ;
        }


#ifndef NDEBUG
        /* Mark the packet as gone. */
        strcpy(p_packet->tag, "c_q") ;
#endif
        MemFree(p_packet) ;

#ifndef NDEBUG
        G_packetsFree++ ;
#endif
    }

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  CmdQSendPacket                                                */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    CmdQSendPacket sets up a short or long packet for sending (and does   */
/*  so by calling CmdQSendShortPacket or CmdQSendLongPacket).               */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    The packetLength field in the packet's header field MUST be set       */
/*  to either short or long.                                                */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_packetEitherShortOrLong *p_packet -- Packet to send                 */
/*                                                                          */
/*    T_word16 retryTime          -- How long to wait for acknowledgement   */
/*                                   before trying to send again.           */
/*                                                                          */
/*    T_word32 extraData          -- Extra data for the callback routine.   */
/*                                                                          */
/*    T_cmdQPacketCallback *p_callback -- Routine to be called when either  */
/*                                        the packet is lost or sent.       */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/23/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void CmdQSendPacket(
            T_packetEitherShortOrLong *p_packet,
            T_word16 retryTime,
            T_word32 extraData,
            T_cmdQPacketCallback p_callback)
{
    DebugRoutine("CmdQSendPacket") ;

    /* Looks like we had something to send. */
    ICmdQSendPacket(
        (T_packetEitherShortOrLong *)p_packet,
        retryTime,
        extraData,
        p_callback) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  CmdQClearAllPorts                                             */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    CmdQClearAllPorts goes through each port and clears out any packets   */
/*  that are waiting to be sent or are incoming.  ACK packets are left      */
/*  alone.                                                                  */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    Make sure that this routine is only called when all communications    */
/*  are finalized.                                                          */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None                                                                  */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  07/24/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void CmdQClearAllPorts(T_void)
{
    T_word16 numPorts ;
    T_word16 port ;
    DebugRoutine("CmdQClearAllPorts") ;

    /* Let's see how many ports there are. */
//    numPorts = CommGetNumberPorts() ;
    numPorts = 1 ;

    /* Go through to each port and set it up. */
    for (port=0; port<numPorts; port++)  {
        /* Set the active port. */
        CmdQSetActivePortNum(port) ;

        ICmdQClearPort() ;
    }

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ICmdQClearPort                     * INTERNAL *               */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ICmdQClearPort removes all outgoing packets for the current port      */
/*  and all incoming packets, too.  ACK packets are left alone.             */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    Make sure that this routine is only called when all communications    */
/*  are finalized.                                                          */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None                                                                  */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  07/24/95  Created                                                */
/*                                                                          */
/****************************************************************************/

static T_void ICmdQClearPort(T_void)
{
    T_word16 i ;
    T_cmdQStruct *p_cmdQ ;
    T_cmdQPacketStruct *p_packet ;
    T_packetLong packet ;

    DebugRoutine("ICmdQClearPort") ;

    /* First, read in all the incoming packets (and ignore them). */
    while (PacketGet(&packet) == 0)
        { }

    /* Catch all the outgoing packets. */
    /* Go through all the queues looking for packets to remove. */
    for (i=1; i<PACKET_COMMAND_MAX; i++)  {
        for (;;)  {
            p_cmdQ = &G_activeCmdQList[i] ;
            p_packet = p_cmdQ->last ;
            if (p_packet != NULL)  {
#ifndef NDEBUG
                /* Check for the tag. */
                if (strcmp(p_packet->tag, "CmQ") != 0)  {
                    printf("Bad packet %p\n", p_packet) ;
                    DebugCheck(FALSE) ;
                }
#endif
                /* Found a packet, remove it. */
                p_cmdQ->last = p_packet->prev ;
                if (p_cmdQ->last == NULL)
                    p_cmdQ->first = NULL ;
                else {
                    p_cmdQ->last->next = NULL ;
                }

#ifndef NDEBUG
                /* Mark the packet as gone. */
                strcpy(p_packet->tag, "c_q") ;
                G_packetsFree++ ;
#endif

                /* Delete it. */
                MemFree(p_packet) ;
            } else {
                /* No packet.  Stop looping on this queue. */
                break ;
            }
        }
    }

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  CmdQForcedReceive                                             */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    CmdQForcedReceive makes the command queue act like it just received   */
/*  the packet from the inport.  All processing is the same.                */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    NEVER PASS AN ACK PACKET TO THIS ROUTINE.  It does not properly       */
/*  work with them.                                                         */
/*    Be sure to make a call to CmdQSetActivePortNum or else this routine   */
/*  will be unpredictable.                                                  */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_packetEitherShortOrLong *p_packet -- Packet being forced in         */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None                                                                  */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  10/20/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void CmdQForcedReceive(T_packetEitherShortOrLong *p_packet)
{
    T_word16 command ;

    DebugRoutine("CmdQForcedReceive") ;

    command = p_packet->data[0] ;

    if (G_cmdQActionList[command] != NULL)  {
        /* Call the appropriate action item. */
        command = p_packet->data[0] ;
        G_cmdQActionList[command](p_packet) ;
        DebugCompare("CmdQForcedReceive") ;
    }

    DebugEnd() ;
}

/****************************************************************************/
/*    END OF FILE:  CMDQUEUE.C                                              */
/****************************************************************************/
