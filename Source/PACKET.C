/*-------------------------------------------------------------------------*
 * File:  PACKET.C
 *-------------------------------------------------------------------------*/
/**
 * The low level "packets" in the networking are sent and received here.
 * They are classified into two sizes:  Short or Long.  This old packet
 * layer interfaces to the raw serial communications.
 * This code is DEPRECATED!
 *
 * @addtogroup PACKET
 * @brief Packet Communications
 * @see http://www.amuletsandarmor.com/AALicense.txt
 * @{
 *
 *<!-----------------------------------------------------------------------*/
#include "standard.h"

/* Keep track of the number of the current packet.  Although this is mainly */
/* for debugging, we can use it to make sure packets are sent correctly.   */
/* If nothing else, we'll make sure everything is in order. */
static T_word32 G_packetID = 1 ;

static T_word16 IPacketComputeChecksum(T_packetEitherShortOrLong *p_packet) ;

/*-------------------------------------------------------------------------*
 * Routine:  PacketSendShort
 *-------------------------------------------------------------------------*/
/**
 *  PacketSendShort is called to small (about 16 byte) packets out the
 *  currently active communications port.
 *
 *  @param p_shortPacket -- Packet with data entry filled.  The
 *      rest of the fields will be filled out.
 *
 *  @return 0 if packet sent, -1 if not sent
 *
 *<!-----------------------------------------------------------------------*/
T_sword16 PacketSendShort(T_packetShort *p_shortPacket)
{
    T_sword16 code = -1 ;

    DebugRoutine("PacketSendShort") ;
    DebugCheck(p_shortPacket != NULL) ;

    /* Store the header information in the packet. */
    p_shortPacket->header.prefix = PACKET_PREFIX ;

    /* Make this a short packet. */
    p_shortPacket->header.packetLength = SHORT_PACKET_LENGTH ;

    /* Put the id for this packet in the packet. */
    p_shortPacket->header.id = G_packetID++ ;

    /* Compute the checksum for this packet. */
    p_shortPacket->header.checksum =
        IPacketComputeChecksum((T_packetEitherShortOrLong *)p_shortPacket) ;

    /* Actually send the data out the port. */
    CommSendData((T_byte8 *)p_shortPacket, sizeof(T_packetShort)) ;
    /* Note that the packet was sent. */
    code = 0 ;

    DebugEnd() ;

    return code ;
}

/*-------------------------------------------------------------------------*
 * Routine:  PacketSendLong
 *-------------------------------------------------------------------------*/
/**
 *  PacketSendLong sends a long packet of 80 characters out the
 *  currently active communications port.
 *
 *  @param p_longPacket -- Packet with data entry filled.  The
 *      rest of the fields will be filled out.
 *
 *  @return 0 if packet sent, -1 if not sent
 *
 *<!-----------------------------------------------------------------------*/
T_sword16 PacketSendLong(T_packetLong *p_longPacket)
{
    T_sword16 code = -1 ;

    DebugRoutine("PacketSendLong") ;
    DebugCheck(p_longPacket != NULL) ;

    /* Store the header information in the packet. */
    p_longPacket->header.prefix = PACKET_PREFIX ;

    /* Make this a long packet. */
    p_longPacket->header.packetLength = LONG_PACKET_LENGTH ;

    /* Put the id for this packet in the packet. */
    p_longPacket->header.id = G_packetID++ ;

    /* Compute the checksum for this packet. */
    p_longPacket->header.checksum =
        IPacketComputeChecksum((T_packetEitherShortOrLong *)p_longPacket) ;

    /* See if there is room to send the packet. */
    /* Actually send the data out the port. */
    CommSendData((T_byte8 *)p_longPacket, sizeof(T_packetLong)) ;

    /* Note that the packet was sent. */
    code = 0 ;

    DebugEnd() ;

    return code ;
}

/*-------------------------------------------------------------------------*
 * Routine:  PacketSendAnyLength
 *-------------------------------------------------------------------------*/
/**
 *  PacketSendAnyLength sends a packet of any size up to a long packet
 *  size out the active communications port.
 *
 *  @param p_anyPacket -- packet to send.
 *
 *  @return 0 if packet sent, -1 if not sent
 *
 *<!-----------------------------------------------------------------------*/
T_sword16 PacketSendAnyLength(T_packetEitherShortOrLong *p_anyPacket)
{
    T_sword16 code = -1 ;

    DebugRoutine("PacketSendAnyLength") ;
    DebugCheck(p_anyPacket != NULL) ;
    DebugCheck(p_anyPacket->header.packetLength <= LONG_PACKET_LENGTH) ;

    /* Store the header information in the packet. */
    p_anyPacket->header.prefix = PACKET_PREFIX ;

    /* Make this a long packet. */
//    p_anyPacket->header.packetLength = LONG_PACKET_LENGTH ;

    /* Put the id for this packet in the packet. */
    p_anyPacket->header.id = G_packetID++ ;

    /* Compute the checksum for this packet. */
    p_anyPacket->header.checksum = IPacketComputeChecksum(p_anyPacket) ;

    /* See if there is room to send the packet. */
    /* Actually send the data out the port. */
    CommSendData(
        (T_byte8 *)p_anyPacket,
        (T_word16)(sizeof(T_packetHeader) + p_anyPacket->header.packetLength)) ;

    /* Note that the packet was sent. */
    code = 0 ;

    DebugEnd() ;

    return code ;
}

/*-------------------------------------------------------------------------*
 * Routine:  PacketSend
 *-------------------------------------------------------------------------*/
/**
 *  PacketSend sends either a short or a long packet out the current
 *  communications port.
 *
 *  @param p_packet -- packet to send.
 *
 *<!-----------------------------------------------------------------------*/
T_sword16 PacketSend(T_packetEitherShortOrLong *p_packet)
{
    T_sword16 status ;

    DebugRoutine("PacketSend") ;

    switch(p_packet->header.packetLength)  {
        case SHORT_PACKET_LENGTH:
            status = PacketSendShort((T_packetShort *)p_packet) ;
            break ;
        case LONG_PACKET_LENGTH:
            status = PacketSendLong((T_packetLong *)p_packet) ;
            break ;
        default:
            DebugCheck(p_packet->header.packetLength <= LONG_PACKET_LENGTH) ;
            status = PacketSendAnyLength(p_packet) ;
            break ;
    }

    DebugEnd() ;

    return status ;
}

/*-------------------------------------------------------------------------*
 * Routine:  PacketSetId
 *-------------------------------------------------------------------------*/
/**
 *  PacketSetId sets the packet Id.  THIS SHOULD ONLY BE CALLED FROM
 *  WITHIN CMDQUEUE.
 *
 *  @param p_packet -- Pointer to packet whose ID must be
 *      p_packet        changed.
 *  @param packetID -- New ID to assign to the packet.
 *
 *<!-----------------------------------------------------------------------*/
T_void PacketSetId (T_packetEitherShortOrLong *p_packet, T_word32 packetID)
{
    p_packet->header.id = packetID;
}


/*-------------------------------------------------------------------------*
 * Routine:  PacketGet
 *-------------------------------------------------------------------------*/
/**
 *  PacketGet is the routine called to retrieve a packet from the
 *  currently active communications port.  If no packet is found, a -1
 *  is returned.  If a packet is found, a 0 is returned.
 *
 *  @param p_packet -- Packet location to receive data.
 *      Note that you must have room allocated
 *      for a long packet in case either a
 *      long or a short packet is received.
 *      You will want to check the packet type
 *      if you do receive data.
 *
 *  @return Resultant flag.  A -1 means no
 *      packet was received.  A 0 means a
 *      packet was found.
 *
 *<!-----------------------------------------------------------------------*/
T_sword16 PacketGet(T_packetLong *p_packet)
{
    T_sword16 returnCode = -1 ;
    T_word16 bufferLength ;
    T_byte8 prefix ;
    T_byte8 packetlen[2] ;
    T_word16 checksum ;
    E_Boolean tryAgain ;

    DebugRoutine("PacketGet") ;
    DebugCheck(p_packet != NULL) ;

    /* Determine how much data is waiting in the incoming buffer. */
    bufferLength = CommGetReadBufferLength() ;

    /* Check to see if there is at least buffer a packet header. */
    /* If there is not, we don't even need to bother with trying to */
    /* read in a packet -- there isn't enough data. */
    if (bufferLength >= sizeof(T_packetHeader))  {
        /* Otherwise, we need to first skip all the bad characters until */
        /* we find an appropriate prefix.  We'll scan first before we */
        /* actually read the data. */
//printf("--- Looking for prefix\n") ;
        do {
            tryAgain = FALSE ;
            /* Otherwise, we need to first skip all the bad characters until */
            /* we find an appropriate prefix.  We'll scan first before we */
            /* actually read the data. */
//printf("--- Looking for prefix\n") ;
            do {
                CommScanData(&prefix, sizeof(T_byte8)) ;
//printf("prefix: %02X\n", prefix) ;
                /* Check to see if we have a correct prefix. */
                if (prefix != PACKET_PREFIX)  {
                    /* Extra character in the stream, remove it. */
                    prefix = CommReadByte() ;
//printf("Extra char: %02X, '%c'\n", prefix, prefix) ;
                    /* One less in the buffer. */
                    bufferLength-- ;
                }
                /* Loop until we find a packet prefix or we are shorter */
                /* than a packet header. */
            } while ((prefix != PACKET_PREFIX) &&
                     (bufferLength >= sizeof(T_packetHeader))) ;

            /* Once again, check the length of the buffer. */
            if (bufferLength >= sizeof(T_packetHeader))  {
                /* We must of gotten a correct packet prefix. */
                /* Scan in the first two characters. */
                CommScanData(packetlen, 2) ;
//printf("packetlen %02X %02X\n", packetlen[0], packetlen[1]) ;
                /* The first byte is the prefix, the second is the packet's */
                /* length.  See if we have enough bytes for the size. */
                /* Check to see if there is enough data to fill */
                /* a the packet. */
                if (bufferLength >= (sizeof(T_packetHeader)+packetlen[1]))  {
                    /* Yes, there is enough data in the buffer. */
                    /* Read in the data.  */
                    if (packetlen[1] > LONG_PACKET_LENGTH)  {
                        /* Hmmm ... too big to be a valid packet. */
                        /* skip the header and cause us to ignore this */
                        /* packet. */
                        CommReadByte() ;
                        tryAgain = TRUE ;
                    } else {
                        DebugCheck(((T_word16)packetlen[1]) <= LONG_PACKET_LENGTH) ;
                        CommReadData(
                            (T_byte8 *)p_packet,
                            (T_word16)(sizeof(T_packetHeader)+((T_word16)packetlen[1]))) ;
                        /* Before we are done, we must compute the checksum */
                        /* and make sure the data is valid. */
                        checksum = IPacketComputeChecksum(
                                       (T_packetEitherShortOrLong *)p_packet) ;
                        /* Compare the checksums.  If they are the same, we */
                        /* have a valid packet.  If not, we discard the packet */
                        /* and pretend that we never got the packet. */
                        if (checksum == p_packet->header.checksum)  {
                             returnCode = 0 ;
                        } else {
//puts("Bad checksum") ;
                             returnCode = -1 ;
                        }
                    }
                }
            }
        } while (tryAgain) ;
    }

    DebugEnd() ;


/*
if (returnCode == 0)  {
  printf("Recv: %d %d %d %d\n",
    p_packet->header.id,
    p_packet->header.packetLength,
    p_packet->header.checksum,
    p_packet->data[0]) ;
}
*/

    return returnCode ;
}
/*-------------------------------------------------------------------------*
 * Routine:  IPacketComputeChecksum
 *-------------------------------------------------------------------------*/
/**
 *  IPacketComputeChecksum calculates a 16 bit checksum for the either
 *  short or long packet passed and returns that value.
 *
 *  @param packet -- Packet to compute checksum
 *
 *  @return Calculated checksum
 *
 *<!-----------------------------------------------------------------------*/
static T_word16 IPacketComputeChecksum(T_packetEitherShortOrLong *packet)
{
    T_word16 checksum ;
    T_word16 i ;

    DebugRoutine("IPacketComputeChecksum") ;
    DebugCheck(packet != NULL) ;

    /* Start out the checksum to equal the id of the block. */
    checksum = packet->header.id ;

    /* Add in the packet type. */
    checksum += packet->header.packetLength ;

    /* Loop the length of the data. */
    for (i=0; i<packet->header.packetLength; i++)  {
        /* If i is odd, then add.  Otherwise, do an exclusive-or to mix */
        /* up the bits. */
        if (i&1)
            checksum += packet->data[i] ;
        else
            checksum ^= packet->data[i] ;
    }

    DebugEnd() ;

    return checksum ;
}

/** @} */
/*-------------------------------------------------------------------------*
 * End of File:  PACKET.C
 *-------------------------------------------------------------------------*/
