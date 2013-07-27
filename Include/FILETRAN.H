/****************************************************************************/
/*    FILE:  filetran.h                                                     */
/****************************************************************************/
#ifndef __FILETRAN_H
#define __FILETRAN_H

#include "GENERAL.H"
#include "PACKET.H"

/** Status type for callbacks. **/
typedef enum _E_fileTransferStatus
{
   FILE_TRANSFER_DONE,
   FILE_TRANSFER_REQUESTED_FILE_NOT_HERE,
   FILE_TRANSFER_UNKNOWN
} E_fileTransferStatus;

/** Callback type for requested transfers. **/
typedef T_void (*T_fileTransferCallback)(T_word32 filename,
                                          E_fileTransferStatus status);

T_void DataPacketInit(void);

T_void DataPacketStartIncoming(T_packetEitherShortOrLong *p_packet);

T_void DataPacketReceived(T_packetEitherShortOrLong *p_receivedPacket);

T_void DataPacketSend(T_word32 name,
                       T_word32 revlevel,
                       T_word16 size,
                       T_byte8 *data);

T_void DataPacketTransferComplete(T_packetEitherShortOrLong *p_packet);

T_void DataPacketReceiveFileRequest(T_packetEitherShortOrLong *p_packet);

T_void DataPacketRequestFile(T_word32 name,
                              T_word32 revlevel,
                              T_fileTransferCallback callback);

T_void DataPacketFileNotHere(T_packetEitherShortOrLong *p_packet);

#endif

/****************************************************************************/
/*    END OF FILE:  filetran.h                                              */
/****************************************************************************/
