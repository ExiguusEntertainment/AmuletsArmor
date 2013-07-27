/****************************************************************************/
/*    FILE:  SCHEDULE.C                                                     */
/****************************************************************************/

#include "standard.h"

#define TEST_PACKET_LEN LONG_PACKET_LENGTH

T_packetLong packet ;
T_word16 packetIndex = 0 ;

T_void CommTestOld(T_void)
{
    T_byte8 character = 0 ;
    T_byte8 receive ;

    DebugRoutine("CommTest") ;

    CommReadConfigFile() ;
    DebugCheck(numPorts != 0) ;

    KeyboardBufferOn() ;
    CommSetActivePort(ports[0]) ;
    GrSetCursorPosition(10, 100) ;
    GrDrawCharacter('>', COLOR_GREEN) ;
    do {
        if (character == '~')  {
            CommSendData("0123456789abcdefghijklmnopqrstuvwxyz", 36) ;
        } else if (character != 0)  {
            GrDrawCharacter(character, COLOR_BLUE) ;
            CommSendByte(character) ;
        }
        while (CommGetReadBufferLength() > 0)  {
            receive = CommReadByte() ;
//            puts(".") ;
            GrDrawCharacter(receive, COLOR_YELLOW) ;
        }
    } while ((character = KeyboardBufferGet()) != '\`') ;

    DebugEnd() ;
}

T_void SendViaPacket(T_byte8 c)
{
    DebugRoutine("SendViaPacket") ;

    packet.data[packetIndex++] = c ;

    if (packetIndex == TEST_PACKET_LEN)  {
        packetIndex = 0 ;
        PacketSendLong(&packet) ;
    }

    DebugEnd() ;
}

T_void SendViaPacketData(T_byte8 *str, T_word16 len)
{
    T_word16 i ;

    DebugRoutine("SendViaPacketData") ;

    for (i=0; i<len; i++)
        SendViaPacket(*(str++)) ;

    DebugEnd() ;
}

T_void CommTest(T_void)
{
    T_byte8 character = 0 ;
    T_byte8 receive ;
    T_word16 i ;
    T_packetLong longPacket ;

    DebugRoutine("CommTest") ;

    CommReadConfigFile() ;
    DebugCheck(numPorts != 0) ;

    KeyboardBufferOn() ;
    CommSetActivePort(ports[0]) ;
    GrSetCursorPosition(10, 100) ;
    GrDrawCharacter('>', COLOR_GREEN) ;
    do {
        if (character == '~')  {
            SendViaPacketData("0123456789abcdefghijklmnopqrstuvwxyz", 36) ;
        } else if (character != 0)  {
            GrDrawCharacter(character, COLOR_BLUE) ;
            SendViaPacket(character) ;
        }
        if (PacketGet(&longPacket)!=-1)  {
            for (i=0; i<longPacket.header.packetLength; i++)  {
                GrDrawCharacter(longPacket.data[i], COLOR_YELLOW) ;
            }
        }
    } while ((character = KeyboardBufferGet()) != '\`') ;

    DebugEnd() ;
}

/****************************************************************************/
/*    END OF FILE:  SCHEDULE.C                                              */
/****************************************************************************/
