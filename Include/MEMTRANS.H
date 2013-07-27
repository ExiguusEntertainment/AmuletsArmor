/****************************************************************************/
/*    FILE:  MEMTRANS.H                                                     */
/****************************************************************************/
/* Memory transfer module -- used for transfering a block of memory from    */
/* one machine to another machine through CMDQUEUE.                         */
/****************************************************************************/
#ifndef _MEM_TRANS_H_
#define _MEM_TRANS_H_

#include "GENERAL.H"
#include "PACKET.H"

/* Callback routine called when transfer is complete. */
typedef T_void (*T_memoryTransferCompleteCallback)
                   (T_void *p_data, 
                    T_word32 size, 
                    T_word32 extraData) ;

/* Routine to call to do it all automatically. */
T_void MemoryTransfer(
           T_void *p_data,
           T_word32 size,
           T_memoryTransferCompleteCallback p_callback,
           T_word32 extraData) ;

/* Use this routine to have the system call the given callback */
/* when a block is received througth the port. */
T_void MemoryTransferSetCallback(
           T_memoryTransferCompleteCallback p_callback) ;

/* Routines to be called by the CMDQUEUE list. */
T_void MemoryTransferReceiveTransferReadyPacket(
            T_packetEitherShortOrLong *p_packet) ;

T_void MemoryTransferReceiveRequestTransfer(
            T_packetEitherShortOrLong *p_packet) ;

T_void MemoryTransferReceiveData(
            T_packetEitherShortOrLong *p_packet) ;

#endif // _MEM_TRANS_H_

/****************************************************************************/
/*    END OF FILE: MEMTRANS.H                                               */
/****************************************************************************/

