/****************************************************************************/
/*    FILE:  CLI_RECV.C             Client Receive Packet Module            */
/****************************************************************************/
#include "CLI_RECV.H"
#include "CLI_SEND.H"
#include "CLIENT.H"
#include "CMDQUEUE.H"
#include "CSYNCPCK.H"
#include "GENERAL.H"
#include "GUILDUI.H"
#include "MAP.H"
#include "MEMORY.H"
#include "MEMTRANS.H"
#include "MESSAGE.H"
#include "OBJECT.H"
#include "PEOPHERE.H"
#include "PLAYER.H"
#include "SMCCHOOS.H"
#include "SOUND.H"
#include "SOUNDS.H"
#include "STATS.H"
#include "TOWNUI.H"

/* Internal prototypes: */
static T_void IProcessUpdateBlockPacket(
                  T_packetEitherShortOrLong *p_packet,
                  T_word32 extraData) ;

static T_void IClientDataBlockSentNowFree(
                  T_void *p_data,
                  T_word32 size,
                  T_word32 extraData) ;
static T_void IClientDataBlockSentNowDontFree(
                  T_void *p_data,
                  T_word32 size,
                  T_word32 extraData) ;

/****************************************************************************/
/*  Routine:  ClientReceivePlaceStartPacket                                 */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ClientReceiveLoginPacket tells the client that the server has just    */
/*  allowed the client to login.                                            */
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
/*  Calls:                                                                  */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/20/95  Created                                                */
/*    AMT  07/12/95  Modified so it works when we aren't on the same machine*/
/*                                                                          */
/****************************************************************************/

T_void ClientReceivePlaceStartPacket(T_packetEitherShortOrLong *p_packet)
{
    T_placeStartPacket *p_start ;
    extern E_Boolean G_serverActive ;

    DebugRoutine("ClientReceivePlaceStartPacket") ;

    DebugCheck (ClientIsInit() == TRUE);

    p_start = (T_placeStartPacket *)p_packet->data ;
    ClientStartPlayer(p_start->objectId, p_start->loginId) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ClientReceivePlayerLogoffPacket                               */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ClientReceivePlayerLogoffPacket tells the client that one of the      */
/*  players have left this group.  Remove all the associated information.   */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_packetEitherShortOrLong *p_packet -- logoff packet                  */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    memmove                                                               */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/25/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ClientReceivePlayerLogoffPacket(T_packetEitherShortOrLong *p_packet)
{
    T_logoffPacket *p_logoff ;
    T_3dObject *p_obj ;

    DebugRoutine("ClientReceivePlayerLogoffPacket") ;

    if ((ClientIsAttemptingLogout() == FALSE) &&
        (ClientIsActive() == TRUE))  {
        p_logoff = (T_logoffPacket *)p_packet->data ;

        /* Find the other player's object. */
        p_obj = ObjectFind(p_logoff->objectId) ;

        /* If we found it, remove and destroy it. */
        if (p_obj != NULL)  {
            ObjectRemove(p_obj) ;
            ObjectDestroy(p_obj) ;
        }
    }

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ClientReceiveMessagePacket                                    */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ClientReceiveMessagePacket is called when someone sends a message     */
/*  about someone saying something.                                         */
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
/*  Calls:                                                                  */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  04/18/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ClientReceiveMessagePacket(T_packetEitherShortOrLong *p_packet)
{
    T_messagePacket *p_msg ;
    T_gameGroupID groupID ;

    DebugRoutine("ClientReceiveMessagePacket") ;

    if ((ClientIsAttemptingLogout() == FALSE) &&
        (ClientIsActive() == TRUE))  {
        /* Get a quick pointer. */
        p_msg = (T_messagePacket *)(p_packet->data) ;

        groupID = ClientSyncGetGameGroupID() ;
        if (CompareGameGroupIDs(p_msg->groupID, groupID))
        {
            MessageAdd(p_msg->message) ;
            SoundDing() ;
        }
    }

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ClientReceiveGotoPlacePacket                                  */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ClientReceiveGotoPlacePacket is the routine that stops the current    */
/*  map and goes to another.                                                */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  07/21/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ClientReceiveGotoPlacePacket(T_packetEitherShortOrLong *p_gotoPacket)
{
    T_gotoPlacePacket *p_packet ;

    DebugRoutine("ClientReceiveGotoPlacePacket") ;
puts("ClientReceiveGotoPlacePacket");

    if (ClientIsAttemptingLogout() == FALSE)  {
        /* Get a quick pointer. */
        p_packet = (T_gotoPlacePacket *)p_gotoPacket->data ;

        ClientForceGotoPlace(p_packet->placeNumber, p_packet->startLocation) ;
    }

    DebugEnd() ;
}

/* TESTING */
T_void ClientReceivedMemoryBlock
                   (T_void *p_data,
                    T_word32 size,
                    T_word32 extraData)
{
    FILE *fp ;
    E_Boolean clientActivity ;
    T_characterBlock *p_charBlock ;

    DebugRoutine("ClientReceivedMemoryBlock") ;


#ifndef NDEBUG
//fprintf(stderr, "Received block of memory %p %ld %08lX\n",
//    p_data,
//    size,
//    extraData) ;

fp = fopen("testfile.", "wb") ;
fwrite(p_data, size, 1, fp) ;
fclose(fp) ;
#endif

//printf("Received memory block %d\n", extraData) ;
//puts("ClientReceiveMemoryBlock: Start") ;
    /* Based on what type of block we have: */
    switch (extraData&0xFFFF)  {
        case CLIENT_MEMORY_BLOCK_UPDATE_BLOCK:
            /* Fake the client into thinking it is actively playing the game. */
            clientActivity = ClientIsActive() ;
            ClientSetActive() ;
/*
            ServerPacketLogParseUpdateBlock(
                p_data,
                size,
                IProcessUpdateBlockPacket,
                0) ;
*/
            if (clientActivity)
                ClientSetActive() ;
            else
                ClientSetInactive() ;
            break ;
        case CLIENT_MEMORY_BLOCK_CHARACTER_LISTING:
            StatsSetSavedCharacterList((T_statsSavedCharArray *)p_data) ;

            /* TESTING ! */
            SMCChooseSetFlag(
                SMCCHOOSE_FLAG_ENTER_COMPLETE,
                TRUE) ;
            break ;
        case MEMORY_BLOCK_CHARACTER_DATA:
            /* Received a downloaded character. */
            p_charBlock = (T_characterBlock *)p_data ;
            StatsReceiveCharacterData(
                p_charBlock->charData,
                p_charBlock->size) ;
puts("Download char complete") ;  fflush(stdout) ;
            SMCChooseSetFlag(
                SMCCHOOSE_FLAG_DOWNLOAD_COMPLETE,
                TRUE) ;
            break ;
        default:
#ifndef NDEBUG
//            printf("ClientReceiveMemoryBlock: %d\n", extraData&0xFFFF) ;
#endif
            DebugCheck(FALSE) ;
            break ;
    }
//puts("ClientReceiveMemoryBlock: End") ;
//fflush(stdout);

//    IProcessLevelChangeData(p_data, size) ;
    MemFree(p_data) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  IProcessUpdateBlockPacket               * INTERNAL *          */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    IProcessUpdateBlockPacket processes a parsed out packet from the      */
/*  level update block.                                                     */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_packetEitherShortOrLong *p_packet -- packet to process              */
/*                                                                          */
/*    T_word32 extraData                  -- Extra data to send             */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    CmdQForcedReceive                                                     */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/27/96  Created                                                */
/*                                                                          */
/****************************************************************************/

static T_void IProcessUpdateBlockPacket(
                  T_packetEitherShortOrLong *p_packet,
                  T_word32 extraData)
{
    DebugRoutine("IProcessUpdateBlockPacket") ;

//printf("IProcessUpdateBlockPacket: %d\n", p_packet->data[0]) ;
//fflush(stdout) ;
    /* I just need to pretend that this is a packet coming */
    /* into the system. */
    CmdQForcedReceive(p_packet) ;

    /* extraData is currently not used. */

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ClientReceiveServerdIDPacket                                  */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ClientReceiveServerIDPacket receives a server id packet that tells    */
/*  what the unique server identifier is.                                   */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_packetEitherShortOrLong *p_packet -- server id packet               */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    ClientSetCurrentServerID                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  02/29/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ClientReceiveServerIDPacket(T_packetEitherShortOrLong *p_packet)
{
    T_serverIDPacket *p_serverID ;

    DebugRoutine("ClientReceiveServerIDPacket") ;

    /* get a quick pointer. */
    p_serverID = (T_serverIDPacket *)(p_packet->data) ;

    ClientSetCurrentServerID(p_serverID->serverID) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ClientReceiveRequestEnterStatusPacket                         */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ClientReceiveRequestEnterStatusPacket is called when the server is    */
/*  responding to a ClientRequestEnterStatusPacket.                         */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_packetEitherShortOrLong *p_packet -- request enter status packet    */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    ClientSetServerEnterStatus                                            */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  03/01/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ClientReceiveRequestEnterStatusPacket(
           T_packetEitherShortOrLong *p_packet)
{
    T_requestEnterStatusPacket *p_status ;

    DebugRoutine("ClientReceiveReqeustEnterStatusPacket") ;

    /* get a quick pointer. */
    p_status = (T_requestEnterStatusPacket *)(p_packet->data) ;

    /* Change the enter status. */
    ClientSetServerEnterStatus(p_status->status) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ClientReceiveLoadCharStatusPacket                             */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ClientReceiveLoadCharStatusPacket is called when the server is ready  */
/*  to tell if the character needs to be downloaded or not.                 */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_packetEitherShortOrLong *p_packet -- load character status packet   */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    ClientSetServerEnterStatus                                            */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  03/01/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ClientReceiveLoadCharStatusPacket(
           T_packetEitherShortOrLong *p_packet)
{
    T_loadCharacterStatusPacket *p_status ;

    DebugRoutine("ClientReceiveLoadCharStatusPacket") ;

    /* get a quick pointer. */
    p_status = (T_loadCharacterStatusPacket *)(p_packet->data) ;

    ClientSetLoadCharacterStatus(p_status->status) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ClientReceiveCreateCharStatusPacket                           */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ClientReceiveCreateCharStatusPacket is called when the server is ready*/
/*  to tell if the character needs to be downloaded or not.                 */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_packetEitherShortOrLong *p_packet -- load character status packet   */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    ClientSetServerEnterStatus                                            */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  03/07/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ClientReceiveCreateCharStatusPacket(
           T_packetEitherShortOrLong *p_packet)
{
    T_createCharStatusPacket *p_status ;

    DebugRoutine("ClientReceiveCreateCharStatusPacket") ;

    /* get a quick pointer. */
    p_status = (T_createCharStatusPacket *)(p_packet->data) ;

    ClientSetCreateCharacterStatus(p_status->status) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ClientReceiveDeleteCharStatusPacket                           */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ClientReceiveDeleteCharStatusPacket is called when the server is ready*/
/*  to tell if the character needs to be downloaded or not.                 */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_packetEitherShortOrLong *p_packet -- load character status packet   */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    ClientSetServerDeleteCharacterStatus                                  */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  03/07/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ClientReceiveDeleteCharStatusPacket(
           T_packetEitherShortOrLong *p_packet)
{
    T_deleteCharStatusPacket *p_status ;

    DebugRoutine("ClientReceiveDeleteCharStatusPacket") ;
//puts("ClientReceiveDeleteCharStatusPacket") ;

    /* get a quick pointer. */
    p_status = (T_deleteCharStatusPacket *)(p_packet->data) ;

    ClientSetDeleteCharacterStatus(p_status->status) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ClientReceiveCheckPasswordStatusPacket                        */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ClientReceiveCheckPasswordStatusPacket is received when the client    */
/*  previously made a request to check a password.  This is the result.     */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_packetEitherShortOrLong *p_packet -- load character status packet   */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    ClientSetCheckPasswordStatus                                          */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  03/08/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ClientReceiveCheckPasswordStatusPacket(
           T_packetEitherShortOrLong *p_packet)
{
    T_checkPasswordStatusPacket *p_status ;

    DebugRoutine("ClientReceiveCheckPasswordStatusPacket") ;

    /* get a quick pointer. */
    p_status = (T_checkPasswordStatusPacket *)(p_packet->data) ;

    ClientSetCheckPasswordStatus(p_status->status) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ClientReceiveChangePasswordStatusPacket                       */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ClientReceiveChangePasswordStatusPacket is received when the client   */
/*  previously made a request to change a password.  This is the result.    */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_packetEitherShortOrLong *p_packet -- load character status packet   */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    ClientSetChangePasswordStatus                                         */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  03/08/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ClientReceiveChangePasswordStatusPacket(
           T_packetEitherShortOrLong *p_packet)
{
    T_changePasswordStatusPacket *p_status ;

    DebugRoutine("ClientReceiveChangePasswordStatusPacket") ;

    /* get a quick pointer. */
    p_status = (T_changePasswordStatusPacket *)(p_packet->data) ;

    ClientSetChangePasswordStatus(p_status->status) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ClientReceiveRequestDataBlockPacket                           */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*      ClientReceiveRequestDataBlockPacket is called when the server is    */
/*    requesting a large data block from the client. In this packet there   */
/*    is what the data is and any extra necessary data.                     */
/*                                                                          */
/*  Problems:                                                               */
/*    None.                                                                 */
/*                                                                          */
/*  Inputs:                                                                 */
/*    T_packetEitherShortOrLong *p_packet -- reqeuest data block   packet   */
/*                                                                          */
/*  Outputs:                                                                */
/*    None.                                                                 */
/*                                                                          */
/*  Revision History:                                                       */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  03/12/96  Created                                                */
/*                                                                          */
/****************************************************************************/

static E_Boolean G_doingTransfer = FALSE ;

T_void ClientReceiveRequestDataBlockPacket(
           T_packetEitherShortOrLong *p_packet)
{
    T_word32 size ;
    T_word32 extraData ;
    T_requestDataBlockPacket *p_request ;
    E_Boolean doMemFree ;
    T_void *p_data ;

    DebugRoutine("ClientReceiveRequestDataBlockPacket") ;

    if (G_doingTransfer == FALSE)  {
        /* get a quick pointer. */
        p_request = (T_requestDataBlockPacket *)(p_packet->data) ;
        extraData = p_request->extraData ;

        /* This routine does not handle the actual data fetching, */
        /* Let a more suitable routine do that. */
        p_data = ClientFetchDataBlock(
                     p_request->dataBlockType,
                     p_request->extraData,
                     &size,
                     &doMemFree) ;

        /* Start the transfer if not null. */
        if (p_data != NULL)  {
            G_doingTransfer = TRUE ;
            if (doMemFree)
                MemoryTransfer(
                    p_data,
                    size,
                    IClientDataBlockSentNowFree,
                    p_request->dataBlockType) ;
            else
                MemoryTransfer(
                    p_data,
                    size,
                    IClientDataBlockSentNowDontFree,
                    p_request->dataBlockType) ;
        }
    }

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  IClientDataBlockSentNowFree                                   */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*      Called after a memory transfer, it frees the memory block just      */
/*    sent.                                                                 */
/*                                                                          */
/*  Problems:                                                               */
/*    None.                                                                 */
/*                                                                          */
/*  Inputs:                                                                 */
/*    T_void *p_data              -- data block sent.                       */
/*    T_word32 size               -- Size of block (not used)               */
/*    T_word32 extraData          -- acc. data (not used)                   */
/*                                                                          */
/*  Outputs:                                                                */
/*    None.                                                                 */
/*                                                                          */
/*  Revision History:                                                       */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  03/12/96  Created                                                */
/*                                                                          */
/****************************************************************************/

static T_void IClientDataBlockSentNowFree(
                  T_void *p_data,
                  T_word32 size,
                  T_word32 extraData)
{
    DebugRoutine("IClientDataBlockSentNowFree") ;

    G_doingTransfer = FALSE ;
    MemFree(p_data) ;

    DebugEnd() ;
}

static T_void IClientDataBlockSentNowDontFree(
                  T_void *p_data,
                  T_word32 size,
                  T_word32 extraData)
{
    DebugRoutine("IClientDataBlockSentNowDontFree") ;

    G_doingTransfer = FALSE ;

    DebugEnd() ;
}

/* LES: 03/26/96 */
/* If we receive a request for the list of parts that make up */
/* this player, immediately send back a long packet with */
/* the 7 (or so) parts that make up the piecewise player. */
T_void ClientReceiveRequestPiecewiseListPacket(
           T_packetEitherShortOrLong *p_packet)
{
    T_packetLong packet ;
    T_piecewiseListPacket *p_list ;
    T_word16 i ;

    DebugRoutine("ClientReceiveRequestPiecewiseListPacket") ;

    p_list = (T_piecewiseListPacket *)(packet.data) ;
    p_list->command = PACKET_COMMANDCS_PIECEWISE_LIST ;

    for (i=0; i<MAX_BODY_PARTS; i++)  {
        p_list->parts[i] = PlayerGetBodyPart(i) ;
//printf("Send part %d is %d\n", i, p_list->parts[i]) ;
//fflush(stdout) ;
    }

    packet.header.packetLength = sizeof(T_piecewiseListPacket) ;

    CmdQSendPacket(
        (T_packetEitherShortOrLong *)&packet,
        140,
        0,
        NULL) ;

    DebugEnd() ;
}

/* LES: 06/12/96 */
T_void ClientReceiveSyncPacket(
           T_packetEitherShortOrLong *p_packet)
{
    T_syncPacket *p_sync ;
    T_syncronizePacket *p_syncro ;
    T_gameGroupID groupID ;

    DebugRoutine("ClientReceiveSyncPacket") ;

    if (ClientIsActive())  {
        p_sync = (T_syncPacket *)(p_packet->data) ;

        /* See if this is the correct group. */
        groupID = ClientSyncGetGameGroupID() ;
//printf("Us--- %02X:%02X:%02X:%02X:%02X:%02X\n", groupID.address[0],groupID.address[1],groupID.address[2],groupID.address[3],groupID.address[4],groupID.address[5]) ;
//printf("Them- %02X:%02X:%02X:%02X:%02X:%02X\n", p_sync->groupID.address[0],p_sync->groupID.address[1],p_sync->groupID.address[2],p_sync->groupID.address[3],p_sync->groupID.address[4],p_sync->groupID.address[5]) ;
        if (!(CompareGameGroupIDs(p_sync->groupID, *DirectTalkGetNullBlankUniqueAddress())))  {
            if (CompareGameGroupIDs(p_sync->groupID, groupID))  {
                p_syncro = (T_syncronizePacket *)(p_sync->syncData) ;

//puts("process") ;
                ClientSyncPacketProcess(p_syncro) ;
            }
        }
    }

    DebugEnd() ;
}

/****************************************************************************/
T_void ClientReceiveTownUIMessagePacket(
           T_packetEitherShortOrLong *p_packet)
{
    T_townUIMessagePacket *p_msg ;

    DebugRoutine("ClientReceiveTownUIMessagePacket") ;

//puts("ClientReceiveTownUIMessagePacket") ;  fflush(stdout) ;
    /* Ignore unless we have the town ui screen up and running. */
    if (TownUIIsOpen() == TRUE)  {
        p_msg = (T_townUIMessagePacket *)(p_packet->data) ;

//printf("UI Msg: '%s' -- '%s'\n", p_msg->name, p_msg->msg) ;  fflush(stdout) ;
        /* Add the message. */
        TownUIAddMessage(p_msg->name, p_msg->msg);
    }

    DebugEnd() ;
}

/****************************************************************************/
T_void ClientReceivePlayerIDSelf(
           T_packetEitherShortOrLong *p_packet)
{
    T_playerIDSelfPacket *p_self ;

    DebugRoutine("ClientReceivePlayerIDSelf") ;

    p_self = (T_playerIDSelfPacket *)(p_packet->data) ;

    /* Update somebody. */
    PeopleHereUpdatePlayer(&p_self->id) ;

    DebugEnd() ;
}

/****************************************************************************/
T_void ClientReceiveRequestPlayerIDPacket(T_packetEitherShortOrLong *p_packet)
{
    DebugRoutine("ClientReceiveRequestPlayerIDPacket") ;

    /* Respond by sending a player id. */
    ClientSendPlayerIDSelf() ;

    DebugEnd() ;
}

/****************************************************************************/
T_void ClientReceiveGameRequestJoinPacket(
           T_packetEitherShortOrLong *p_packet)
{
    T_gameRequestJoinPacket *p_request ;

    DebugRoutine("ClientReceiveGameRequestJoinPacket") ;

    /* Get a quick pointer. */
    p_request = (T_gameRequestJoinPacket *)(p_packet->data) ;

//printf("Receive request join %d %d\n", p_request->groupID, p_request->adventure) ;  fflush(stdout) ;
    /* Just pass on the request to another routine (better suited). */
    PeopleHereRequestJoin(
        p_request->uniqueAddress,
        p_request->groupID,
        p_request->adventure) ;

    DebugEnd() ;
}

/****************************************************************************/
T_void ClientReceiveGameRespondJoinPacket(
           T_packetEitherShortOrLong *p_packet)
{
    T_gameRespondJoinPacket *p_request ;

    DebugRoutine("ClientReceiveGameRespondJoinPacket") ;

    /* Only process this packet if we are in the guild and are trying */
    /* to join a game. */
//printf("Receive respond join ... \n") ;  fflush(stdout) ;
    if (PeopleHereGetOurState() == PLAYER_ID_STATE_JOINING_GAME)  {
        p_request = (T_gameRespondJoinPacket *)(p_packet->data) ;

        PeopleHereRespondToJoin(
            p_request->uniqueAddress,
            p_request->groupID,
            p_request->adventure,
            p_request->response) ;
    } else {
//puts("... nope") ;  fflush(stdout) ;
    }

    DebugEnd() ;
}

/****************************************************************************/
T_void ClientReceiveGameStartPacket(
           T_packetEitherShortOrLong *p_packet)
{
    T_gameStartPacket *p_start ;
    T_word16 i ;
    T_directTalkUniqueAddress ourAddress ;
    T_gameGroupID groupID ;

    DebugRoutine("ClientReceiveGameStartPacket") ;

//puts("ClientReceiveGameStartPacket") ;
    p_start = (T_gameStartPacket *)(p_packet->data) ;

    /* Set up the time offset. */
    MapSetDayOffset(p_start->timeOfDay) ;

    /* Only consider the packet if we are in the same group. */
//printf("-- %d %d\n", ClientSyncGetGameGroupID(), p_start->groupID) ;
    groupID = ClientSyncGetGameGroupID() ;
    if (CompareGameGroupIDs(groupID, p_start->groupID))  {
        DirectTalkGetUniqueAddress(&ourAddress) ;

        /* See if we are in the list. */
        for (i=0; i<p_start->numPlayers; i++)  {
            if (memcmp(&ourAddress, &p_start->players[i], sizeof(ourAddress)) == 0)
                break ;
        }

        /* Did we find a match? */
        if (i != p_start->numPlayers)  {
//puts("Matched") ;
            /* Yes, we are on the list in the ith position. */
            /* Set up the game setup and go. */
            ClientSyncSetNumberPlayers(p_start->numPlayers) ;
            ClientSetLoginId(i) ;

            /* Set up the next jump */
            ClientSetNextPlace(p_start->firstLevel, 0) ;
            ClientSetAdventureNumber(p_start->adventure) ;
            ClientSyncInitPlayersHere() ;

            /* Clear all the messages */
            MessageClear() ;

            /* Copy all the player address over to the peophere module. */
            for (i=0; i<p_start->numPlayers; i++)
                PeopleHereSetUniqueAddr(i, (&p_start->players[i])) ;
        } else {
//puts("Not Matched") ;
            /* Make sure we are still in the guild and if so drop us. */
            if ((PeopleHereGetOurLocation() == PLAYER_ID_LOCATION_GUILD) &&
                (PeopleHereGetOurState() == PLAYER_ID_STATE_JOINING_GAME))  {
                /* Cancel that game we were joining, it left without us. */
                GuildUICancelJoinGame(NULL);

                /* Nope, we got excluded. */
                MessageAdd("Game started without you.") ;
            }
        }
    }

    DebugEnd() ;
}

/****************************************************************************/
/*    END OF FILE: CLI_RECV.C       Client Receive Packet Module            */
/****************************************************************************/
