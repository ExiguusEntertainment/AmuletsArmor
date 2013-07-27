/****************************************************************************/
/*    FILE:  CLI_SEND.C             Client Send Packet Module               */
/****************************************************************************/
#include "3D_COLLI.H"
#include "CMDQUEUE.H"
#include "CLI_RECV.H"
#include "CLI_SEND.H"
#include "CLIENT.H"
#include "CSYNCPCK.H"
#include "DITALK.H"
#include "EQUIP.H"
#include "GENERAL.H"
#include "MAP.H"
#include "MESSAGE.H"
#include "OBJECT.H"
#include "PACKET.H"
#include "PEOPHERE.H"
#include "PLAYER.H"
#include "SMCCHOOS.H"
#include "STATS.H"

/****************************************************************************/
/*  Routine:  ClientSendMessage                                             */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ClientSendMessage sends the message that the user just typed in across*/
/*  the network and to everyone else in range.                              */
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
/*    CmdQSendLongPacket                                                    */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  04/18/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ClientSendMessage(T_byte8 *message)
{
    T_packetLong packet ;
    T_messagePacket *p_msg ;
    char buffer[100] ;

    DebugRoutine("ClientSendMessage") ;

    /* Get a quick pointer. */
    p_msg = (T_messagePacket *)packet.data ;

    /* Put the message in the packet. */
    p_msg->command = PACKET_COMMAND_MESSAGE ;
    p_msg->player = ClientGetLoginId() ;
    p_msg->groupID = ClientSyncGetGameGroupID() ;
//    strcpy(p_msg->message, message) ;
    sprintf(buffer, "%s: %s", StatsGetName(), message) ;
    sprintf(p_msg->message, "%-50.50s", buffer) ;

    /* Send the whole packet. */
    CmdQSendLongPacket(&packet, 280, 0, NULL) ;

    /* Send it back to ourselves so we have a running log. */
    ClientReceiveMessagePacket((T_packetEitherShortOrLong *)&packet) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ClientSendRequestServerIDPacket                               */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ClientSendRequestServerIDPacket sends to the server a packet          */
/*  requesting for the server's unique id.                                  */
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
/*    CmdQSendShortPacket                                                   */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  02/29/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ClientSendRequestServerIDPacket(T_void)
{
    T_packetLong packet ;
    T_requestServerIDPacket *p_requestServerID ;

    DebugRoutine("ClientSendRequestServerIDPacket") ;

    /* Get a quick pointer to the true action data. */
    p_requestServerID = (T_requestServerIDPacket *)packet.data ;

    p_requestServerID->command = PACKET_COMMANDCS_REQUEST_SERVER_ID ;
    packet.header.packetLength = sizeof(T_requestServerIDPacket) ;

    /* Send the packet. */
    CmdQSetActivePortNum(0) ;
    CmdQSendPacket((T_packetEitherShortOrLong *)&packet, 280, 0, NULL) ;

    DebugEnd() ;
}


/****************************************************************************/
/*  Routine:  ClientSendRequestEnterPacket                                  */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ClientSendRequestEnterPacket sends a request to server for            */
/*  entering the server's first screen.                                     */
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
/*    CmdQSendShortPacket                                                   */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  03/01/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ClientSendRequestEnterPacket(T_void)
{
    T_packetLong packet ;
    T_requestServerIDPacket *p_requestServerID ;

    DebugRoutine("ClientSendRequestEnterPacket") ;

    /* Get a quick pointer to the true action data. */
    p_requestServerID = (T_requestServerIDPacket *)packet.data ;

    p_requestServerID->command = PACKET_COMMANDCS_REQUEST_ENTER ;
    packet.header.packetLength = sizeof(T_requestEnterPacket) ;

    /* Send the packet. */
    CmdQSetActivePortNum(0) ;
    CmdQSendPacket((T_packetEitherShortOrLong *)&packet, 280, 0, NULL) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ClientSendRequestCharacterListing                             */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ClientSendRequestCharacterListing sends out a request char listing    */
/*  packet to the server.                                                   */
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
/*    CmdQSendShortPacket                                                   */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  03/01/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ClientSendRequestCharacterListing(T_void)
{
    T_packetLong packet ;
    T_requestCharacterListPacket *p_request ;
    T_statsSavedCharArray *p_charArray ;

    DebugRoutine("ClientSendRequestCharacterListing") ;

    /* Get a quick pointer to the true action data. */
    p_request = (T_requestCharacterListPacket *)packet.data ;

    if (ClientIsServerBased())  {
        p_request->command = PACKET_COMMANDCS_REQUEST_CHAR_LIST ;

        packet.header.packetLength = sizeof(T_requestCharacterListPacket) ;

        /* Send the packet. */
        CmdQSetActivePortNum(0) ;
        CmdQSendPacket((T_packetEitherShortOrLong *)&packet, 280, 0, NULL) ;
    } else {
        /* Get the saved character listing. */
        p_charArray = StatsGetSavedCharacterList() ;

        /* Make this listing active. */
        StatsSetSavedCharacterList(p_charArray) ;

        /* Note that we are done "entering" the character list. */
        SMCChooseSetFlag(
            SMCCHOOSE_FLAG_ENTER_COMPLETE,
            TRUE) ;
    }

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ClientSendLoadCharacter                                       */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ClientSendLoadCharacter tells the server to load the given character  */
/*  and to start a transfer if necessary.                                   */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_byte8 slot                -- Slot of character to load              */
/*                                                                          */
/*    T_word32 checksum           -- Checksum to check against              */
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
/*    LES  03/06/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ClientSendLoadCharacter(T_byte8 slot, T_word32 checksum)
{
    T_packetLong packet ;
    T_loadCharacterPacket *p_load ;

    DebugRoutine("ClientSendLoadCharacter") ;

    if (ClientIsServerBased())  {
        /* Get a quick pointer to the true action data. */
        p_load = (T_loadCharacterPacket *)packet.data ;

        p_load->command = PACKET_COMMANDCS_LOAD_CHARACTER ;
        p_load->slot = slot ;
        p_load->checksum = checksum ;
        packet.header.packetLength = sizeof(T_loadCharacterPacket) ;

        /* Send the packet. */
        CmdQSetActivePortNum(0) ;

        CmdQSendPacket((T_packetEitherShortOrLong *)&packet, 280, 0, NULL) ;
    } else {
        /* Always make it OK to load the character. */
        ClientSetLoadCharacterStatus(LOAD_CHARACTER_STATUS_CORRECT) ;
    }

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ClientSendCreateCharacter                                     */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ClientSendCreateCharacter sends a packet to tell the server to create */
/*  a character with the given checksum.  A transfer of the character is    */
/*  then started.                                                           */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_byte8 slot                -- Slot of character password to create   */
/*                                                                          */
/*    T_word32 checksum           -- Checksum to use.                       */
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
/*    LES  03/07/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ClientSendCreateCharacter(
           T_byte8 slot,
           T_word32 checksum,
           T_byte8 *p_password)
{
    T_packetLong packet ;
    T_createCharacterPacket *p_create ;

    DebugRoutine("ClientSendCreateCharacter") ;

    if (ClientIsServerBased())  {
        /* Get a quick pointer to the true action data. */
        p_create = (T_createCharacterPacket *)packet.data ;

        p_create->command = PACKET_COMMANDCS_CREATE_CHARACTER ;
        p_create->slot = slot ;
        p_create->checksum = checksum ;
        strncpy(p_create->password, p_password, MAX_SIZE_PASSWORD) ;
        packet.header.packetLength = sizeof(T_createCharacterPacket) ;

        /* Send the packet. */
        CmdQSetActivePortNum(0) ;
        CmdQSendPacket((T_packetEitherShortOrLong *)&packet, 280, 0, NULL) ;
    } else {
        /* Always make it ok to create a character. */
        ClientSetCreateCharacterStatus(CREATE_CHARACTER_STATUS_OK) ;
    }

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ClientSendDeleteCharacter                                     */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ClientSendDeleteCharacter sends a packet to tell the server to delete */
/*  the given character slot.                                               */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_byte8 slot                -- Slot of character password to delete   */
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
/*    LES  03/07/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ClientSendDeleteCharacter(T_byte8 slot)
{
    T_packetLong packet ;
    T_deleteCharacterPacket *p_delete ;

    DebugRoutine("ClientSendDeleteCharacter") ;

    if (ClientIsServerBased())  {
        /* Get a quick pointer to the true action data. */
        p_delete = (T_deleteCharacterPacket *)packet.data ;

        p_delete->command = PACKET_COMMANDCS_DELETE_CHARACTER ;
        p_delete->slot = slot ;
        packet.header.packetLength = sizeof(T_deleteCharacterPacket) ;

        /* Send the packet. */
        CmdQSetActivePortNum(0) ;
        CmdQSendPacket((T_packetEitherShortOrLong *)&packet, 280, 0, NULL) ;
    } else {
        /* It is always OK to delete a character. */
        ClientSetDeleteCharacterStatus(DELETE_CHARACTER_STATUS_OK) ;
    }
    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ClientSendCheckPassword                                       */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ClientSendCheckPassword asks the server to check the given password   */
/*  against the given character slot's password.                            */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_byte8 slot                -- Slot of character password to check    */
/*                                                                          */
/*    T_byte8 password            -- Password to check                      */
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
/*    LES  03/08/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ClientSendCheckPassword(
           T_byte8 slot,
           T_byte8 password[MAX_SIZE_PASSWORD])
{
    T_packetLong packet ;
    T_checkPasswordPacket *p_check ;

    DebugRoutine("ClientSendCheckPassword") ;

    if (ClientIsServerBased())  {
        /* Get a quick pointer to the true action data. */
        p_check = (T_checkPasswordPacket *)packet.data ;

        /* Fill the packet. */
        p_check->command = PACKET_COMMANDCS_CHECK_PASSWORD ;
        p_check->slot = slot ;
        memcpy(p_check->password, password, MAX_SIZE_PASSWORD) ;

        /* Squeeze down the size of the packet to only what we need. */
        packet.header.packetLength = sizeof(T_checkPasswordPacket) ;

        /* Send the packet. */
        CmdQSetActivePortNum(0) ;
        CmdQSendPacket((T_packetEitherShortOrLong *)&packet, 280, 0, NULL) ;
    } else {
        /* Password is always ok for now. */
        ClientSetCheckPasswordStatus(CHECK_PASSWORD_STATUS_OK) ;
    }

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ClientSendChangePassword                                      */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ClientSendChangePassword tells the server to change the password      */
/*  of the given character slot.                                            */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_byte8 slot                -- Slot of character password to change   */
/*                                                                          */
/*    T_byte8 password            -- Old password                           */
/*                                                                          */
/*    T_byte8 newPassword         -- New password                           */
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
/*    LES  03/08/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ClientSendChangePassword(
           T_byte8 slot,
           T_byte8 password[MAX_SIZE_PASSWORD],
           T_byte8 newPassword[MAX_SIZE_PASSWORD])
{
    T_packetLong packet ;
    T_changePasswordPacket *p_change ;

    DebugRoutine("ClientSendChangePassword") ;

    if (ClientIsServerBased())  {
        /* Get a quick pointer to the true action data. */
        p_change = (T_changePasswordPacket *)packet.data ;

        /* Fill the packet. */
        p_change->command = PACKET_COMMANDCS_CHANGE_PASSWORD ;
        p_change->slot = slot ;
        memcpy(p_change->password, password, MAX_SIZE_PASSWORD) ;
        memcpy(p_change->newPassword, newPassword, MAX_SIZE_PASSWORD) ;

        /* Squeeze down the size of the packet to only what we need. */
        packet.header.packetLength = sizeof(T_changePasswordPacket) ;

        /* Send the packet. */
        CmdQSetActivePortNum(0) ;
        CmdQSendPacket((T_packetEitherShortOrLong *)&packet, 280, 0, NULL) ;
    } else {
        ClientSetChangePasswordStatus(CHANGE_PASSWORD_STATUS_OK) ;
    }

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ClientRequestDataBlock                                        */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ClientRequestDataBlock sends out a packet to ask the server to send   */
/*  it a data block.                                                        */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word16 dataBlockType      -- Type of block to retrieve              */
/*                                                                          */
/*    T_word32 extraData          -- Extra data info                        */
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
/*    LES  03/12/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ClientRequestDataBlock(
           T_word16 dataBlockType,
           T_word32 extraData)
{
    T_packetLong packet ;
    T_requestDataBlockPacket *p_request ;

    DebugRoutine("ClientRequestDataBlock") ;
    DebugCheck(dataBlockType < MEMORY_BLOCK_TYPE_UNKNOWN) ;

    /* Get a quick pointer to the true action data. */
    p_request = (T_requestDataBlockPacket *)packet.data ;

    /* Fill the packet. */
    p_request->command = PACKET_COMMANDCSC_REQUEST_DATA_BLOCK ;
    p_request->dataBlockType = dataBlockType ;
    p_request->extraData = extraData ;

    /* Squeeze down the size of the packet to only what we need. */
    packet.header.packetLength = sizeof(T_requestDataBlockPacket) ;

    /* Send the packet. */
    CmdQSetActivePortNum(0) ;
    CmdQSendPacket((T_packetEitherShortOrLong *)&packet, 280, 0, NULL) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ClientRequestTake                                             */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ClientRequestTake sends the proper packet to the server to request    */
/*  permission to take an object.  The object is actually 'taken' if the    */
/*  server responds with permission with a SC_TAKE_REPLY packet.  See       */
/*  ClientReceiveTakeReply().                                               */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*    T_3dObject *p_obj              -- Object to request taking.           */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    ClientGetLoginId                                                        */
/*    CmdQSendShortPacket                                                   */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    AMT  07/12/95  Created                                                */
/*                                                                          */
/****************************************************************************/
T_void ClientRequestTake(T_3dObject *p_obj, E_Boolean autoStore)
{
    T_word16 dist;
    T_sword16 z;
    T_word16 num;
    T_wallListItem wallList[20];

    DebugRoutine("ClientRequestTake");

    /* See if the object exists and is grabable. */
    if ((p_obj != NULL )&&
    (ObjectGetAttributes (p_obj) & OBJECT_ATTR_GRABABLE)){
    /* Object is not grabable. */
    /* Is the object close enough? */
    if ((dist = CalculateDistance(ObjectGetX16(p_obj),
                            ObjectGetY16(p_obj),
                            PlayerGetX16(),
                            PlayerGetY16())) < PLAYER_GRAB_DISTANCE) {
        /* Find a mutual z height. */
        z = ObjectGetZ16(p_obj);
        if (z < PlayerGetZ16())
        z += ObjectGetHeight(p_obj);

        /* Make sure there is nothing blocking the hit */
        num = Collide3dFindWallList(
                PlayerGetX16(),
                PlayerGetY16(),
                ObjectGetX16(p_obj),
                ObjectGetY16(p_obj),
                z,
                20,
                wallList,
                WALL_LIST_ITEM_MAIN);
        if (num == 0) {
            /* No walls are in the way. */

            /* Object is close enough.  Attempt to take it. */
            ClientSyncSendActionPickUpItem(ObjectGetServerId(p_obj), autoStore);
        } else {
            MessageAdd("Cannot grab through wall.");
        }
    } else {
        /* Object is too far. */
        MessageAdd ("Object too far away to take.");
    }
} else {
    /* Not grabbable, tell the player. */
    MessageAdd("Object can not be taken.");
}
    DebugEnd ();
}

/****************************************************************************/
/*  Routine:  ClientRequestRetransmit                                       */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ClientRequestRetransmit sends out a request to get back in sync       */
/*  with a particular player.                                               */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_byte8 player                 -- Player being request to retrans     */
/*                                                                          */
/*    T_byte8 transmitStart          -- Packet to start retransmiting from  */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  06/17/96  Created                                                */
/*    LES  08/21/96  Added group ID                                         */
/*                                                                          */
/****************************************************************************/

T_void ClientRequestRetransmit(
           T_byte8 player,
           T_byte8 transmitStart,
           T_gameGroupID groupID,
           T_word16 destination)
{
    T_packetShort packet ;
    T_retransmitPacket *p_request ;

    DebugRoutine("ClientRequestRetransmit") ;

    /* Fill out the request. */
    p_request = (T_retransmitPacket *)(packet.data) ;
    p_request->command = PACKET_COMMAND_RETRANSMIT ;
    p_request->fromPlayer = ObjectGetServerId(PlayerGetObject()) - 9000 ;
    p_request->toPlayer = player ;
    p_request->transmitStart = transmitStart ;
    p_request->groupID = groupID ;

    /* Send out the request. */
//    CmdQSendShortPacket(&packet, 140, 0, NULL) ;
    DirectTalkSetDestination(PeopleHereGetUniqueAddr(destination)) ;
    PacketSendShort(&packet) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ClientSendGotoSucceeded                                       */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ClientSendGotoSucceeded tells the server that the location has been   */
/*  reached and is now just waiting for the "OK" (PlaceStartPacket).        */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word16 placeNumber           -- Place player went to                */
/*                                                                          */
/*    T_byte8 startLocation          -- Sub-place player went to            */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  07/23/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ClientSendGotoSucceeded(
           T_word16 placeNumber,
           T_word16 startLocation)
{
    T_packetShort packet ;
    T_gotoSucceededPacket *p_succeed ;

    DebugRoutine("ClientSendGotoSucceeded") ;

    /* If we are not playing by ourselves, tell the others that */
    /* we made it to the new place. */
    if (ClientGetConnectionType() != CLIENT_CONNECTION_TYPE_SINGLE)  {
        /** Prepare and send a GOTO_SUCCEEDED packet. **/
        p_succeed = (T_gotoSucceededPacket *)packet.data;

        p_succeed->command = PACKET_COMMANDCS_GOTO_SUCCEEDED ;
        p_succeed->placeNumber = placeNumber;
        p_succeed->startLocation = startLocation ;

        CmdQSendShortPacket(&packet, 600, 0, NULL) ;
    }

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ClientSendTownUIAddMessage                                    */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ClientSendTownUIAddMessage tells everyone in the pub/town to add a    */
/*  message.                                                                */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_byte8 *p_name                -- Name of player                      */
/*                                                                          */
/*    T_byte8 startLocation          -- Sub-place player went to            */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  08/19/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ClientSendTownUIAddMessage(T_byte8 *p_msg)
{
    T_townUIMessagePacket *p_msgPacket ;
    T_packetLong packet ;

    DebugRoutine("ClientSendTownUIAddMessage") ;

    p_msgPacket = (T_townUIMessagePacket *)(packet.data) ;
    memset(p_msgPacket, 0, sizeof(T_townUIMessagePacket)) ;
    p_msgPacket->command = PACKET_COMMAND_TOWN_UI_MESSAGE ;
    strncpy(p_msgPacket->name, StatsGetName(), 29) ;
    strncpy(p_msgPacket->msg, p_msg, 39) ;
    CmdQSendLongPacket(&packet, 600, 0, NULL) ;

    ClientReceiveTownUIMessagePacket((T_packetEitherShortOrLong *)&packet) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ClientSendPlayerIDSelf                                        */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ClientSendPlayerIDSelf is used for several things.  Mainly it is used */
/*  to identify a player's location at particular times.                    */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  08/19/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ClientSendPlayerIDSelf(T_void)
{
    T_packetLong packet ;
    T_playerIDSelfPacket *p_self ;

    DebugRoutine("ClientSendPlayerIDSelf") ;

    p_self = (T_playerIDSelfPacket *)(packet.data) ;
    PeopleHereGetPlayerIDSelfStruct(&p_self->id) ;
    p_self->command = PACKET_COMMAND_PLAYER_ID_SELF ;

    CmdQSendLongPacket(&packet, 600, 0, NULL) ;

    ClientReceivePlayerIDSelf((T_packetEitherShortOrLong *)&packet) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ClientSendRequestPlayerID                                     */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ClientSendRequestPlayerID asks that the given player tell what        */
/*  location and state the player is in.                                    */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  08/20/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ClientSendRequestPlayerID(T_void)
{
    T_packetShort packet ;

    DebugRoutine("ClientSendRequestPlayerID") ;

    /* Send a short packet with this simple command to get a player id. */
    packet.data[0] = PACKET_COMMAND_REQUEST_PLAYER_ID ;
    CmdQSendShortPacket(&packet, 140, 0, NULL) ;

    ClientReceiveRequestPlayerIDPacket((T_packetEitherShortOrLong *)&packet) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ClientSendRespondToJoinPacket                                 */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    This routine is used by a creator of a game to respond to a players   */
/*  request to enter an already existing game.                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  08/21/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ClientSendRespondToJoinPacket(
           T_directTalkUniqueAddress uniqueAddress,
           T_gameGroupID groupID,
           T_word16 adventure,
           E_respondJoin response)
{
    T_packetLong packet ;
    T_gameRespondJoinPacket *p_respond ;

    DebugRoutine("ClientSendRespondToJoinPacket") ;

    /* Make a packet and fill it out. */
    p_respond = (T_gameRespondJoinPacket *)packet.data ;
    p_respond->command = PACKET_COMMAND_GAME_RESPOND_JOIN ;
    p_respond->uniqueAddress = uniqueAddress ;
    p_respond->groupID = groupID ;
    p_respond->adventure = adventure ;
    p_respond->response = response ;

//printf("Send respond to join %d\n", response) ;
    CmdQSendLongPacket(&packet, 140, 0, NULL) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ClientSendRequestJoin                                         */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    This routine is used by a player wanting to join a game already       */
/*  created.                                                                */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  08/21/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ClientSendRequestJoin(
            T_word16 map,
            T_gameGroupID instance)
{
    T_packetLong packet ;
    T_gameRequestJoinPacket *p_request ;

    DebugRoutine("ClientSendRequestJoin") ;

    /* Fill a packet to request joining a game. */
    p_request = (T_gameRequestJoinPacket *)packet.data ;
    p_request->command = PACKET_COMMAND_GAME_REQUEST_JOIN ;
    DirectTalkGetUniqueAddress(&p_request->uniqueAddress) ;
    p_request->groupID = instance ;
    p_request->adventure = map ;

//printf("Send request to join %d ", map) ;
//DirectTalkPrintAddress(stdout, &instance) ;
//puts(".") ;

    CmdQSendLongPacket(&packet, 140, 0, NULL) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ClientSendGameStartPacket                                     */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    This routine is used by a player starting up a game that has already  */
/*  been constructed.                                                       */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  08/22/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ClientSendGameStartPacket(
           T_gameGroupID groupID,
           T_word16 adventure,
           T_byte8 numPlayers,
           T_directTalkUniqueAddress *players,
           T_word16 firstLevel)
{
    T_packetLong packet ;
    T_gameStartPacket *p_start ;

    DebugRoutine("ClientSendRequestJoin") ;

    p_start = (T_gameStartPacket *)(packet.data) ;

    p_start->command = PACKET_COMMAND_GAME_START ;
    p_start->groupID = groupID ;
    p_start->adventure = adventure ;
    p_start->numPlayers = numPlayers ;
    p_start->players[0] = players[0] ;
    p_start->players[1] = players[1] ;
    p_start->players[2] = players[2] ;
    p_start->players[3] = players[3] ;
    p_start->timeOfDay = MapGetDayOffset() ;
    if (firstLevel == 0)
        firstLevel = adventure ;
    p_start->firstLevel = firstLevel ;

//printf("Send request to start %d %d\n", groupID, adventure) ;
    CmdQSendLongPacket(&packet, 140, 0, NULL) ;

    ClientReceiveGameStartPacket((T_packetEitherShortOrLong *)&packet) ;

    DebugEnd() ;
}

/****************************************************************************/
/*    END OF FILE: CLI_SEND.C       Client Send Packet Module               */
/****************************************************************************/

