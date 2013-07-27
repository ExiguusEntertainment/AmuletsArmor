/****************************************************************************/
/*    FILE:  MEMTRANS.C                                                     */
/****************************************************************************/
/* Memory transfer module -- used for transfering a block of memory from    */
/* one machine to another machine through CMDQUEUE/ACPORT                   */
/****************************************************************************/
#include "COMPRESS.H"
#include "CMDQUEUE.H"
#include "MEMORY.H"
#include "MEMTRANS.h"
#include "PROMPT.H"

/* DEBUGGING!!! */
//#undef DebugRoutine
//#define DebugRoutine(name) { puts(name) ; DebugAddRoutine(name, __FILE__, __LINE__) ; }

#define MEMORY_TRANSFER_TAG      (*((T_word32 *)"MeTr"))
#define MEMORY_TRANSFER_DEAD_TAG (*((T_word32 *)"DmtR"))

#define MEMORY_TRANSFER_FLAG_SEND       0x00  /* 0--- ---- */
#define MEMORY_TRANSFER_FLAG_RECEIVE    0x80  /* 1--- ---- */
#define MEMORY_TRANSFER_FLAG_MASK_OUT   0x7F  /* -111 1111 */

typedef struct _T_memoryTransferInfo {
    T_byte8 *p_data ;                  /* Where the block is. */
    T_word32 size ;                    /* Size of block to send. */
    T_word32 extraData ;               /* Extra info for callback and */
                                       /* receiver. */
    T_byte8 *p_originalData ;                  /* Where the uncompressed block is. */
    T_word32 originalSize ;                    /* Size of uncompressed block to send. */
                                       /* receiver. */
    T_byte8 transferID ;
    T_memoryTransferCompleteCallback p_callback ;   /* Callback when done. */

    T_word32 bytesSent ;               /* How many bytes to send. */
    T_word32 tag ;                     /* tag to identify this as a */
                                       /* memory transfer */
    struct _T_memoryTransferInfo *p_prev ;   /* Double link list. */
    struct _T_memoryTransferInfo *p_next ;
} T_memoryTransferInfo ;

static T_memoryTransferInfo *G_transferList = NULL ;
static T_byte8 G_transferID = 0 ;
static T_memoryTransferCompleteCallback G_transferReceivedCallback = NULL ;

/* Internal prototypes: */
static T_memoryTransferInfo *IFindMemoryTransfer(T_byte8 transerID) ;
static T_void ISendNextMemoryTransfer(T_memoryTransferInfo *p_transfer) ;
static T_void IDestroyMemoryTransfer(T_memoryTransferInfo *p_transfer) ;
static T_void IBlockTransferred(
                   T_word32 extraData,
                   T_packetEitherShortOrLong *p_packet) ;


/****************************************************************************/
/* Process description:                                                     */
/****************************************************************************/
/*                                                                          */
/*    To transfer a block of memory, several actions must occur in order    */
/*                                                                          */
/* 1) Sender tells receiver that it is going to receive a block.            */
/*    Block size, extra data, and transfer ID is sent.                      */
/*                                                                          */
/* 2) Receiver gets the request and prepares a block of memory of the given */
/*    size and stores away the extraData.  It responds by saying that the   */
/*    block has been created and returns the transfer ID.                   */
/*    In addition, another id is sent to the sender that the receiver       */
/*    will use in all future transactions.  (The first id is now no longer  */
/*    used).                                                                */
/*                                                                          */
/* 3) Sender receives the packet and stores the receiver's transfer ID.     */
/*                                                                          */
/* 4) A block of data is then prepared containing the recevier's transfer   */
/*    id.  A callback routine will be called when the block is acknowledged.*/
/*    (It is assumed that lossless communications is being done).           */
/*    Block is sent along with position in data block.                      */
/*                                                                          */
/* 5) If the block is sent correctly (acked and all that), the callback     */
/*    routine checks to see if another block needs to be sent.  If so,      */
/*    go to step 4.  Repeat until done.                                     */
/*                                                                          */
/* 6) If the whole block has been sent, call the memory transfer complete   */
/*    callback routine.  All transient information is now deleted.          */
/*                                                                          */
/****************************************************************************/

/****************************************************************************/
/*  Routine:  MemoryTransfer                                                */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    MemoryTransfer starts the self going process of sending to the        */
/*  current cmdqueue/port the given block of memory.  It also makes sure    */
/*  the responding application is ready to receive the data.                */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    Must be called with the current port activated that this data will    */
/*  go out of.                                                              */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_void *p_data              -- Pointer to the data block              */
/*                                                                          */
/*    T_word32 size               -- Size of block to send                  */
/*                                                                          */
/*    T_memoryTransfarCompleteCallback -- callback routine, NULL if none    */
/*                                                                          */
/*    T_word32 extraData          -- Extra data to send to other side and   */
/*                                   callback                               */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    MemAlloc                                                              */
/*    CmdQSendShortPacket/CmdQSendShortPacket                             */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  10/26/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void MemoryTransfer(
           T_void *p_data,
           T_word32 size,
           T_memoryTransferCompleteCallback p_callback,
           T_word32 extraData)
{
    T_packetLong packet ;
    T_requestMemoryTransferPacket *p_requestMemoryTransfer ;
    T_byte8 *p_compressedBlock ;
    T_word32 compressedSize ;

    DebugRoutine("MemoryTransfer") ;
    DebugCheck(p_data != NULL) ;
    DebugCheck(size != 0) ;

    p_compressedBlock =
        CompressBlock(
            p_data,
            size,
            &compressedSize) ;
printf("Compressed from %ld to %ld\n", size, compressedSize) ;

    DebugCheck(p_compressedBlock != NULL) ;

#ifndef SERVER_ONLY
    if (CmdQGetActivePortNum() == 0)  {
        PromptStatusBarInit("Uploading data", compressedSize);
    }
#endif
//printf("MemoryTransfer called by %s\n", DebugGetCallerName()) ;  fflush(stdout) ;
    /* Send out a request to start transfer packet. */
    p_requestMemoryTransfer =
        (T_requestMemoryTransferPacket *)packet.data ;
    p_requestMemoryTransfer->command =
        PACKET_COMMANDCSC_REQUEST_MEMORY_TRANSFER ;
    p_requestMemoryTransfer->size = compressedSize ;
    p_requestMemoryTransfer->extraData = extraData ;
    p_requestMemoryTransfer->callback = (T_word32)p_callback ;
    p_requestMemoryTransfer->pointer = (T_word32)p_compressedBlock ;
    p_requestMemoryTransfer->originalSize = size ;
    p_requestMemoryTransfer->originalPointer = (T_word32)p_data ;
    packet.header.packetLength = sizeof(T_requestMemoryTransferPacket) ;

#ifdef TARGET_NT
    CmdQSendPacket((T_packetEitherShortOrLong *)&packet, 140, 0, NULL);
#else
    CmdQSendPacket ((T_packetEitherShortOrLong *)&packet, 140, 0, NULL);
#endif

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  MemoryTransferReceiveTransferReadyPacket                      */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    When a MEMORY_TRANSFER_READY packet is received, this routine is      */
/*  called to continue the sending process.                                 */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_packetEitherShortOrLong *p_packet -- Received ready packet          */
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
/*    LES  10/26/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void MemoryTransferReceiveTransferReadyPacket(
            T_packetEitherShortOrLong *p_packet)
{
    T_readyForMemoryTransferPacket *p_ready ;
    T_memoryTransferInfo *p_transfer ;

    DebugRoutine("MemoryTransferReceiveTransferReadyPacket") ;
    DebugCheck(p_packet != NULL) ;

    p_ready = (T_readyForMemoryTransferPacket *)p_packet->data ;
    DebugCheck(p_ready->command == PACKET_COMMANDCSC_MEMORY_TRANSFER_READY) ;

//    p_transfer = IFindMemoryTransfer(p_ready->transferID) ;
    p_transfer = MemAlloc(sizeof(T_memoryTransferInfo)) ;
    DebugCheck(p_transfer != NULL) ;

    if (p_transfer)  {
        /* Link in the transfer. */
        p_transfer->p_prev = NULL ;
        p_transfer->p_next = G_transferList ;
        G_transferList = p_transfer ;

        /* Fill in the structure. */
        p_transfer->p_originalData =
            ((T_byte8 *)(p_ready->request.originalPointer)) ;
        p_transfer->originalSize = p_ready->request.originalSize ;

        p_transfer->p_data = ((T_byte8 *)(p_ready->request.pointer)) ;
        p_transfer->size = p_ready->request.size ;
        p_transfer->extraData = p_ready->request.extraData ;
        p_transfer->transferID = p_ready->transferID ;
        p_transfer->p_callback =
            (T_memoryTransferCompleteCallback)p_ready->request.callback ;
        p_transfer->bytesSent = 0 ;
        p_transfer->tag = MEMORY_TRANSFER_TAG ;

        ISendNextMemoryTransfer(p_transfer) ;
    }

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  IFindMemoryTransfer                * INTERNAL *               */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    IFindMemoryTransfer searches through the list of memory transfers     */
/*  to find one with a matching transferID.                                 */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_byte8 transferID          -- Transfer ID of searched memory transfer*/
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_memoryTransferINfo *      -- Found memory transfer or NULL..        */
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
/*    LES  10/26/95  Created                                                */
/*                                                                          */
/****************************************************************************/

static T_memoryTransferInfo *IFindMemoryTransfer(T_byte8 transferID)
{
    T_memoryTransferInfo *p_transfer ;

    DebugRoutine("IFindMemoryTransfer") ;

    /* Go through the linked list and try to find the */
    /* making transferID */
    p_transfer = G_transferList ;
    while (p_transfer)  {
        if (p_transfer->transferID == transferID)
            break ;
        p_transfer = p_transfer->p_next ;
    }

    DebugEnd() ;

    return p_transfer ;
}

/****************************************************************************/
/*  Routine:  ISendNextMemoryTransfer            * INTERNAL *               */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ISendNextMemoryTransfer checks to see if the transfer is complete     */
/*  or if there is more to send.  If complete, the callback routine is      */
/*  called and the memory transfer structure is destroyed.   If not done,   */
/*  a new long packet is created when the next block of information and     */
/*  sent out the port.                                                      */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_memoryTransferInfo *p_transfer -- Transfer to send next block.      */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    p_transfer->p_callback (indirectly)                                   */
/*    IDestroyMemoryTransfer                                                */
/*    memcpy                                                                */
/*    CmdQSendLongPacket/CmdQSendLongPacket                               */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  10/26/95  Created                                                */
/*                                                                          */
/****************************************************************************/

static T_void ISendNextMemoryTransfer(T_memoryTransferInfo *p_transfer)
{
    T_word32 sizeBlock ;
    T_packetLong packet ;
    T_transferMemoryPacket *p_transferMemoryPacket ;

    DebugRoutine("ISendNextMemoryTransfer") ;
    DebugCheck(p_transfer != NULL) ;
    DebugCheck(p_transfer->tag == MEMORY_TRANSFER_TAG) ;

    /* Are we done or is there more? */
    if (p_transfer->bytesSent >= p_transfer->size)  {
        /* We have completed! */
        /* Make a call to that callback routine. */
        if (p_transfer->p_callback)
            p_transfer->p_callback(
                 p_transfer->p_originalData,
                 p_transfer->originalSize,
                 p_transfer->extraData) ;

        /* Free the compressed block version of the data. */
        MemFree(p_transfer->p_data) ;

        /* Now get rid of this memory transfer item. */
        IDestroyMemoryTransfer(p_transfer) ;
#ifndef SERVER_ONLY
        PromptStatusBarClose() ;
#endif
    } else {
        /* Not done yet. */
        /* Determine size of next block. */
        sizeBlock = p_transfer->size - p_transfer->bytesSent ;
        if (sizeBlock > TRANSFER_MEMORY_BLOCK_LENGTH)
            sizeBlock = TRANSFER_MEMORY_BLOCK_LENGTH ;

        /* Prepare the long packet to be sent. */
        p_transferMemoryPacket =
            (T_transferMemoryPacket *)packet.data ;

        p_transferMemoryPacket->command =
            PACKET_COMMANDCSC_MEMORY_TRANSFER_DATA ;
        p_transferMemoryPacket->transferID =
            p_transfer->transferID &
                MEMORY_TRANSFER_FLAG_MASK_OUT ;
        p_transferMemoryPacket->position = p_transfer->bytesSent ;
        memcpy(
            p_transferMemoryPacket->data,
            p_transfer->p_data + p_transfer->bytesSent,
            sizeBlock) ;

#ifndef SERVER_ONLY
        PromptStatusBarUpdate(p_transfer->bytesSent) ;
#endif

#ifdef TARGET_NT
        CmdQSendLongPacket(
            &packet,
            350,
            ((T_word32)p_transfer),
            IBlockTransferred) ;
#else
        CmdQSendLongPacket(
            &packet,
            350,
            ((T_word32)p_transfer),
            IBlockTransferred) ;
#endif

        p_transfer->bytesSent += sizeBlock ;
    }

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  IDestroyMemoryTransfer             * INTERNAL *               */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    IDestroyMemoryTransfer removes the memory transfer object from the    */
/*  list and destroys it from memory.                                       */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_memoryTransferInfo *p_transfer -- Transfer to send next block.      */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    MemFree                                                               */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  10/26/95  Created                                                */
/*                                                                          */
/****************************************************************************/

static T_void IDestroyMemoryTransfer(T_memoryTransferInfo *p_transfer)
{
    DebugRoutine("IDestroyMemoryTransfer") ;
    DebugCheck(p_transfer != NULL) ;
    DebugCheck(p_transfer->tag == MEMORY_TRANSFER_TAG) ;

    /* Remove the transfer memory info structure from the list. */
    if (p_transfer->p_prev)  {
        p_transfer->p_prev->p_next = p_transfer->p_next ;
    } else {
        G_transferList = p_transfer->p_next ;        
    }

    if (p_transfer->p_next)
        p_transfer->p_next->p_prev = p_transfer->p_prev ;

    /* Now we can delete the structure. */
    p_transfer->tag = MEMORY_TRANSFER_DEAD_TAG ;
    MemFree(p_transfer) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  IBlockTransferred                  * INTERNAL *               */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    IBlockTransferred is the acknowledgement callback that is invoked     */
/*  after a transfer memory block has been sent.   When that happens, this  */
/*  routine immediately tries to send another block (if there is one).      */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word32 extraData          -- Data disguised as T_memoryTransferInfo */
/*                                                                          */
/*    T_packetEitherShortOrLong *p_packet -- packet sent (not used here)    */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    ISendNextMemoryTransfer                                               */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  10/26/95  Created                                                */
/*                                                                          */
/****************************************************************************/

static T_void IBlockTransferred(
                   T_word32 extraData,
                   T_packetEitherShortOrLong *p_packet)
{
    DebugRoutine("IBlockTransferred") ;
    DebugCheck(p_packet != NULL) ;

    /* Finished with the last block, try another. */
    ISendNextMemoryTransfer((T_memoryTransferInfo *)extraData) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  MemoryTransferReceiveRequestTransfer                          */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    MemoryTransferReceiveRequestTransfer is called when a request to      */
/*  do a memory transfer is received.  This routine prepares for the        */
/*  transfer and sends back a ready packet.                                 */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_packetEitherShortOrLong *p_packet -- request transfer packet        */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    MemAlloc                                                              */
/*    CmdQSendShortPacket/CmdQSendShortPacket                             */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  10/26/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void MemoryTransferReceiveRequestTransfer(
            T_packetEitherShortOrLong *p_packet)
{
    T_packetLong packet ;
    T_requestMemoryTransferPacket *p_request ;
    T_memoryTransferInfo *p_memTransfer ;
    T_byte8 *p_data ;
    T_readyForMemoryTransferPacket *p_ready ;

    DebugRoutine("MemoryTransferReceiveRequestTransfer") ;
    DebugCheck(p_packet != NULL) ;

    p_request = (T_requestMemoryTransferPacket *)p_packet->data ;

    /* Create a structure to note that we are going */
    /* to do a transfer, allocate the memory, and then say that */
    /* we are ready. */
    p_memTransfer = MemAlloc(sizeof(T_memoryTransferInfo)) ;
    p_data = MemAlloc(p_request->size) ;
/*
#ifndef NDEBUG
if (!p_data)
    printf("Request size: %ld\n", p_request->size) ;
#endif
*/
    DebugCheck(p_memTransfer != NULL) ;
    DebugCheck(p_data != NULL) ;

    if ((p_memTransfer) && (p_data))  {
        /* Put into the linked list. */
        p_memTransfer->p_prev = NULL ;
        p_memTransfer->p_next = G_transferList ;
        G_transferList = p_memTransfer ;

        /* Initialize the data structure. */
        p_memTransfer->p_data = p_data ;
        p_memTransfer->size = p_request->size ;
#ifndef SERVER_ONLY
        if (CmdQGetActivePortNum() == 0)  {
            PromptStatusBarInit("Transferring data", p_request->size);
        }
#endif
        p_memTransfer->extraData = p_request->extraData ;
        p_memTransfer->transferID = G_transferID++ ;
        p_memTransfer->transferID |= MEMORY_TRANSFER_FLAG_RECEIVE ;

        p_memTransfer->p_callback = NULL ;
        p_memTransfer->bytesSent = 0 ;
        p_memTransfer->tag = MEMORY_TRANSFER_TAG ;

        /* Send out a request to start transfer packet. */
        p_ready = (T_readyForMemoryTransferPacket *)packet.data ;
        p_ready->transferID = p_memTransfer->transferID &
                                  MEMORY_TRANSFER_FLAG_MASK_OUT ;
        p_ready->request = *p_request ;
//printf("Starting transfer id %d\n", p_memTransfer->transferID) ;
        p_ready->command = PACKET_COMMANDCSC_MEMORY_TRANSFER_READY ;
        packet.header.packetLength = sizeof(T_readyForMemoryTransferPacket) ;

#ifdef TARGET_NT
        CmdQSendPacket((T_packetEitherShortOrLong *)&packet, 140, 0, NULL);
#else
        CmdQSendPacket((T_packetEitherShortOrLong *)&packet, 140, 0, NULL);
#endif
    }

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  MemoryTransferReceiveData                                     */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    MemoryTransferReceiveData is called each time a data packet for a     */
/*  memory transfer is received.  This routine appropriate stores each      */
/*  received block and then notifies the system when a whole block has      */
/*  arrived.                                                                */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_packetEitherShortOrLong *p_packet -- data    transfer packet        */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    IFindMemoryTransfer                                                   */
/*    memcpy                                                                */
/*    G_transferReceivedCallback (indirectly)                               */
/*    IDestroyMemoryTransfer                                                */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  10/26/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void MemoryTransferReceiveData(
            T_packetEitherShortOrLong *p_packet)
{
    T_transferMemoryPacket *p_transfer ;
    T_memoryTransferInfo *p_info ;
    T_word16 sizeLeft ;
    T_word32 uncompressedSize ;
    T_byte8 *p_uncompressedData ;

    DebugRoutine("MemoryTransferReceiveData") ;
    DebugCheck(p_packet != NULL) ;
    DebugCheck(p_packet->header.packetLength == LONG_PACKET_LENGTH) ;

    /* Another packet, store it in our data. */
    p_transfer = (T_transferMemoryPacket *)p_packet->data ;

    /* Find what transfer this block is related to. */
//printf("Searching for transfer %d\n", p_transfer->receiveTransferID) ;
    p_info = IFindMemoryTransfer((T_byte8)(p_transfer->transferID |
                                           MEMORY_TRANSFER_FLAG_RECEIVE)) ;
//    DebugCheck(p_info != NULL) ;
    if (p_info)  {
printf("bytes sent: %ld,  position = %ld,  size=%ld\n", p_info->bytesSent, p_transfer->position, p_info->size) ;  fflush(stdout) ;
         /* Is this block in the correct next position? */
         if (p_info->bytesSent == p_transfer->position)  {
             sizeLeft = p_info->size - p_transfer->position ;
             if (sizeLeft > TRANSFER_MEMORY_BLOCK_LENGTH)
                 sizeLeft = TRANSFER_MEMORY_BLOCK_LENGTH ;
             /* Yes, it is. */
             memcpy(
                 p_info->p_data + p_transfer->position,
                 p_transfer->data,
                 sizeLeft) ;

             p_info->bytesSent += sizeLeft ;

#ifndef SERVER_ONLY
             if (CmdQGetActivePortNum() == 0)  {
                 PromptStatusBarUpdate(p_info->bytesSent);
             }
#endif
             if (p_info->bytesSent == p_info->size)  {
                 /* Transfer complete, call the appropriate */
                 /* callback routine. */
                 DebugCheck(G_transferReceivedCallback != NULL) ;
puts("Transfer complete") ;  fflush(stdout) ;
                 if (G_transferReceivedCallback)  {
                     p_uncompressedData =
                         UncompressBlock(
                             p_info->p_data,
                             p_info->size,
                             &uncompressedSize) ;
printf("Uncompressed from %ld to %ld\n", p_info->size, uncompressedSize) ;

                     G_transferReceivedCallback(
                         p_uncompressedData,
                         uncompressedSize,
                         p_info->extraData) ;
                 }

                 /* Free the compressed block of data. */
                 MemFree(p_info->p_data) ;

                 /* Don't need the transfer info block anymore. */
                 /* destroy it. */
                 IDestroyMemoryTransfer(p_info) ;
#ifndef SERVER_ONLY
                 if (CmdQGetActivePortNum() == 0)  {
                     PromptStatusBarClose() ;
                 }
#endif
             }
         } else {
             /* No, its not. */
             puts("Block on transfer out of order!") ;
//             DebugCheck(FALSE) ;
         }
    }

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  MemoryTransferSetCallback                                     */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    MemoryTransferSetCallback sets the callback routine that is invoked   */
/*  when a whole memory block has been received.                            */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_memoryTransferCompleteCallback *p_callback -- Routine to call       */
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
/*    LES  10/27/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void MemoryTransferSetCallback(
           T_memoryTransferCompleteCallback p_callback)
{
    DebugRoutine("MemoryTransferSetCallback") ;
    DebugCheck(p_callback != NULL) ;

    G_transferReceivedCallback = p_callback ;

    DebugEnd() ;
}

/****************************************************************************/
/*    END OF FILE: MEMTRANS.C                                               */
/****************************************************************************/

