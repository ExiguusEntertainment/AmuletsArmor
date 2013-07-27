/****************************************************************************/
/*    FILE:  filetran.c                                                     */
/****************************************************************************/
#include "CMDQUEUE.H"
#include "FILE.H"
#include "FILES.H"
#include "FILETRAN.H"
#include "MEMORY.H"

/** Types with which I keep track of and assemble data transfers. **/

typedef enum _E_direction
{
   DATA_DIRECTION_OFF,
   DATA_DIRECTION_OUTGOING,
   DATA_DIRECTION_INCOMING
} E_direction;

/** This is the structure which holds management information for one **/
/** currently-running transfer. **/
typedef struct _T_dataTransfer
{
   E_direction direction;
   T_word16 packetsLeft;
   T_word16 totalSize; /** NOTE: NOT a T_word32; that would allow absurdly **/
                       /** long transfers. **/
   T_word16 totalPackets;
   T_word32 filename;
   T_word32 revlevel;
   T_byte8 *dataArea;
   T_fileTransferCallback callWhenDone;
   T_word16 commPort;
} T_dataTransfer;

/** This structure holds a pending transfer; that is, one which has **/
/** been requested but has not been replied to yet. **/
typedef struct _T_requestedFile
{
   T_word32 filename;
   T_fileTransferCallback callback;
   struct _T_requestedFile *next, *prev;
} T_requestedFile;


/** I can have up to 256 simultaneous data transfers in progress. **/
/** (This is because fileID's are T_byte8s. **/
T_dataTransfer G_transfers[256];

/** I can have any number of requested files in wait; here is a linked **/
/** list of them. **/
T_requestedFile *G_requestedFileList;

/** Internal function prototypes. **/
T_void IDataPacketSendResendRequest (T_byte8 fileID, T_word16 position);

T_void IDataPacketSendPacket (T_byte8 fileID, T_word16 pos);

T_void IDataPacketCloseIncoming (T_byte8 fileID);


/****************************************************************************/
/*  Routine:  DataPacketInit                                                */
/****************************************************************************/
/*  Description:                                                            */
/*    Initializes this library.  Should be called exactly once.             */
/*                                                                          */
/*  Problems:                                                               */
/*    None.                                                                 */
/*                                                                          */
/*  Inputs:                                                                 */
/*    None.                                                                 */
/*                                                                          */
/*  Outputs:                                                                */
/*    None.                                                                 */
/*                                                                          */
/*  Revision History:                                                       */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    AMT  07/10/95  Created                                                */
/****************************************************************************/
T_void DataPacketInit ()
{
   T_word16 i;

   DebugRoutine ("DataPacketInit");

   for (i = 0; i < 256; i ++)
   {
      G_transfers[i].direction = DATA_DIRECTION_OFF;
      G_transfers[i].dataArea = NULL;
   }

   DebugEnd ();
}

/****************************************************************************/
/*  Routine:  DataPacketStartIncoming                                       */
/****************************************************************************/
/*  Description:                                                            */
/*    Called when a genericDataTransferPacket is recieved to initialize     */
/*  an incoming file transfer.                                              */
/*                                                                          */
/*  Problems:                                                               */
/*    None.                                                                 */
/*                                                                          */
/*  Inputs:                                                                 */
/*    T_packetEitherShortOrLong   -- Pointer to packet.                     */
/*              *p_packet                                                   */
/*                                                                          */
/*  Outputs:                                                                */
/*    None.                                                                 */
/*                                                                          */
/*  Revision History:                                                       */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    AMT  06/30/95  Created                                                */
/****************************************************************************/
T_void DataPacketStartIncoming (T_packetEitherShortOrLong *p_packet)
{
   T_word16 i;
   T_genericDataStartTransferPacket *p_transferPacket;
   T_dataTransfer *p_transfer;
   T_requestedFile *p_request;

   DebugRoutine ("DataPacketInit");

   /** Get a quick pointer. **/
   p_transferPacket = (T_genericDataStartTransferPacket *)(p_packet->data);

   /** First, I need to make sure that there is not already a transfer **/
   /** with the same fileID in progress.  **/
   DebugCheck (G_transfers[p_transferPacket->fileID].direction 
               == DATA_DIRECTION_OFF);

   /** And another pointer. **/
   p_transfer = &(G_transfers[p_transferPacket->fileID]);

   /** Now I just need to fill in the transfer. **/
   p_transfer->direction = DATA_DIRECTION_INCOMING;

   p_transfer->commPort = CmdQGetActivePortNum ();

   p_transfer->totalSize = p_transferPacket->size;
   p_transfer->totalPackets = p_transfer->packetsLeft =
         p_transferPacket->size / GENERIC_DATA_PACKET_CHUNK_SIZE + 1;
/*   p_transfer->callWhenDone = callback;
*/
   p_transfer->filename = p_transferPacket->name;

   /** Hmm.. is there a request waiting which this should fill? **/
   for (p_request = G_requestedFileList; p_request != NULL;
        p_request = p_request->next)
   {
      if (p_request->filename = p_transfer->filename)
      {
         /** Request matches.  Copy over the callback and remove from **/
         /** list. **/
         p_transfer->callWhenDone = p_request->callback;

         if ((p_request->next != NULL) && (p_request->prev != NULL))
            p_request->prev->next = p_request->next;
         else if (p_request->next != NULL)
            G_requestedFileList = p_request->next;

         MemFree (p_request);
      }
   }


   /** Let me allocate the memory, and initialize it so that I can later **/
   /** tell if packets have been loaded in. **/
   p_transfer->dataArea = MemAlloc (p_transferPacket->size);

   for (i = 0; i < p_transfer->packetsLeft; i++)
   {
      strcpy (p_transfer->dataArea + i * GENERIC_DATA_PACKET_CHUNK_SIZE,
              "EmPtY");
   }

   DebugEnd ();
}

/****************************************************************************/
/*  Routine:  DataPacketReceived                                            */
/****************************************************************************/
/*  Description:                                                            */
/*    Called when a genericDataPacket is received.  This routine copies     */
/*  the data into the transfer area.  It then checks to see if the transfer */
/*  is finished.  If so, it calls the callback function.                    */
/*                                                                          */
/*  Problems:                                                               */
/*    None.                                                                 */
/*                                                                          */
/*  Inputs:                                                                 */
/*    T_packetEitherShortOrLong   -- Pointer to the data packet.            */
/*                *p_receivedPacket                                         */
/*                                                                          */
/*  Outputs:                                                                */
/*    None.                                                                 */
/*                                                                          */
/*  Revision History:                                                       */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    AMT  06/30/95  Created                                                */
/****************************************************************************/
T_void DataPacketReceived (T_packetEitherShortOrLong *p_receivedPacket)
{
   T_genericDataPacket *p_packet;
   T_dataTransfer *p_transfer;

   DebugRoutine ("DataPacketReceived");

   /** Grab a quick pointer or two. **/
   p_packet = (T_genericDataPacket *)(p_receivedPacket->data);
   p_transfer = &(G_transfers[p_packet->header.fileID]);

   /** Is this a valid transfer in progress? **/
   DebugCheck (p_transfer->direction != DATA_DIRECTION_OFF);

   /** !!! If this is the first data packet, get the revlevel. **/
   if (p_packet->header.position == 0)
   {
      p_transfer->revlevel = p_packet->header.revlevel;
   }
   
   /** Is this a normal packet, or the final packet? **/
   if (p_packet->header.size == 0)
   {
      /** Normal packet. **/
      memcpy (p_transfer->dataArea +
              p_packet->header.position * GENERIC_DATA_PACKET_CHUNK_SIZE,
              p_packet->data, GENERIC_DATA_PACKET_CHUNK_SIZE);
   }
   else
   {
      /** Final packet. **/
      memcpy (p_transfer->dataArea + 
              p_packet->header.position * GENERIC_DATA_PACKET_CHUNK_SIZE,
              p_packet->data, p_packet->header.size);
   }

   /** Note that one more packet was received. **/
   p_transfer->packetsLeft --;

printf ("Packet received (file %d, pos %d, packetsleft %d)\n", p_transfer->filename,
       p_packet->header.position, p_transfer->packetsLeft);

   /** Am I finished with this one? **/
   if (p_transfer->packetsLeft == 0)
   {
      /** Close the incoming transfer. **/
      IDataPacketCloseIncoming (p_packet->header.fileID);
   }

   DebugEnd ();
}

/****************************************************************************/
/*  Routine:  IDataPacketCloseIncoming                                      */
/****************************************************************************/
/*  Description:                                                            */
/*    Internal -- Closes an incoming transfer.  This involves checking the  */
/*  checksum, saving the file, responding to the sender with a              */
/*  TRANSFER_COMPLETE, and freeing the transfer's data structures.          */
/*                                                                          */
/*  Problems:                                                               */
/*    Currently, there is no file checksum.                                 */
/*                                                                          */
/*  Inputs:                                                                 */
/*    T_byte8 fileID              -- Active transfer to close.              */
/*                                                                          */
/*  Outputs:                                                                */
/*    None.                                                                 */
/*                                                                          */
/*  Revision History:                                                       */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    AMT  08/30/95  Created                                                */
/****************************************************************************/
T_void IDataPacketCloseIncoming (T_byte8 fileID)
{
   T_packetShort packet;
   T_fileTransferCompletePacket *p_packet;
   T_file file;
   T_dataTransfer *p_transfer;

   DebugRoutine ("IDataPacketCloseIncoming");

   /** Get a quick pointer to the transfer struct. **/
   p_transfer = &(G_transfers[fileID]);

   /** Save the file. **/
   file = FilesCreateFileWithRevlevel (p_transfer->filename,
                                       p_transfer->revlevel);
   FileWrite (file, p_transfer->dataArea, p_transfer->totalSize);
   FileClose (file);

   /** Now we need to send a completion packet. **/
   p_packet = (T_fileTransferCompletePacket *)(packet.data);

   p_packet->command = PACKET_COMMANDRT_TRANSFER_COMPLETE;
   p_packet->fileID = fileID;

   CmdQSetActivePortNum (p_transfer->commPort);

   CmdQSendShortPacket (&packet, 140, 0, NULL);

   /** Call the callback if present. **/
   if (p_transfer->callWhenDone != NULL)
      (p_transfer->callWhenDone) (p_transfer->filename, FILE_TRANSFER_DONE);
 
   /** Done with the data area... **/
   MemFree (p_transfer->dataArea);

   /** and no longer receiving. **/
   p_transfer->direction = DATA_DIRECTION_OFF;

   DebugEnd ();
}

/****************************************************************************/
/*  Routine:  DataPacketRequestFile                                         */
/****************************************************************************/
/*  Description:                                                            */
/*    Requests a data file from the soon-to-be transmitter.  Sends a request*/
/*  packet, and saves the request data on the requestedFiles list so that   */
/*  when the file is downloaded, a callback can be called.                  */
/*    NOTE!  File requests go out the currently-active CmdQ port!!!         */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*  Inputs:                                                                 */
/*    T_word32 name               -- "Filename" of the file.                */
/*    T_word32 revlevel           -- Revision level to demand.              */
/*    T_fileTranCallback callback -- Function to call when file is done     */
/*                                                                          */
/*  Outputs:                                                                */
/*    None.                                                                 */
/*                                                                          */
/*  Revision History:                                                       */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    AMT  08/30/95  Created                                                */
/****************************************************************************/
T_void DataPacketRequestFile (T_word32 name,
                              T_word32 revlevel,
                              T_fileTransferCallback callback)
{
   T_packetShort packet;
   T_requestFileFromServerPacket *p_packet;
   T_requestedFile *newentry;

   DebugRoutine ("DataPacketRequestFile");

   /** Quick pointer time. **/
   p_packet = (T_requestFileFromServerPacket *)(packet.data);

   p_packet->command = PACKET_COMMANDRT_REQUEST_FILE;
   p_packet->name = name;
   p_packet->revlevel = revlevel;

   /** Send the packet. **/
   CmdQSendShortPacket (&packet, 140, 0, NULL);

   /** Now make a note of it. **/
   newentry = MemAlloc (sizeof (T_requestedFile));
   newentry->filename = name;
   newentry->callback = callback;

   newentry->prev = NULL;
   newentry->next = G_requestedFileList;

   if (G_requestedFileList != NULL)
      G_requestedFileList->prev = newentry;

   G_requestedFileList = newentry;

   DebugEnd ();
}

/****************************************************************************/
/*  Routine:  DataPacketFileNotHere                                         */
/****************************************************************************/
/*  Description:                                                            */
/*    Called when a file-not-here packet has been received.  Cancels the    */
/*  transfer.                                                               */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*  Inputs:                                                                 */
/*    T_packetEitherShortOrLong * -- Received packet.                       */
/*                         p_packet                                         */
/*  Outputs:                                                                */
/*    None.                                                                 */
/*                                                                          */
/*  Revision History:                                                       */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    AMT  09/05/95  Created                                                */
/****************************************************************************/
T_void DataPacketFileNotHere (T_packetEitherShortOrLong *p_packet)
{
   T_requestedFile *p_request;
   T_word32 filename;

   DebugRoutine ("DataPacketReceiveFileNotHere");

   /** Get the filename of the turned-down file **/
   filename = ((T_fileNotHerePacket *)(p_packet->data))->filename;

printf ("File request for file %d failed!\n", filename);

   /** Locate the file's request entry. **/
   for (p_request = G_requestedFileList; p_request != NULL;
        p_request = p_request->next)
   {
      if (p_request->filename = filename)
      {
         /** Request matches.  Call the callback and remove from **/
         /** list. **/
         if (p_request->callback != NULL)
            p_request->callback (p_request->filename,
                                 FILE_TRANSFER_REQUESTED_FILE_NOT_HERE);

         if ((p_request->next != NULL) && (p_request->prev != NULL))
            p_request->prev->next = p_request->next;
         else if (p_request->next != NULL)
            G_requestedFileList = p_request->next;

         MemFree (p_request);
      }
   }


   DebugEnd ();
}

/****************************************************************************/
/*  Routine:  DataPacketReceiveFileRequest                                  */
/****************************************************************************/
/*  Description:                                                            */
/*    Called when a file-request packet has been sent.  Loads the file,     */
/*  and calls DataPacketSend() to transmit it.                              */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*  Inputs:                                                                 */
/*    T_packetEitherShortOrLong * -- Request packet.                        */
/*                         p_packet                                         */
/*  Outputs:                                                                */
/*    None.                                                                 */
/*                                                                          */
/*  Revision History:                                                       */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    AMT  08/30/95  Created                                                */
/****************************************************************************/
T_void DataPacketReceiveFileRequest (T_packetEitherShortOrLong *p_packet)
{
   T_requestFileFromServerPacket *p_requestPacket;
   T_void *data;
   T_word32 size;
   T_packetShort packet;
   T_word32 revlevel;

   DebugRoutine ("DataPacketReceiveFileRequest");

   /** Get a quick pointer **/
   p_requestPacket = (T_requestFileFromServerPacket *)(p_packet->data);

   /** Send the given file. **/
   if ((data = FilesLoadFile (p_requestPacket->name,
                              p_requestPacket->revlevel,
                              &revlevel,
                              &size)) != NULL)
   {
      /** Note that the CmdQActivePortNum is still the sender of this **/
      /** packet. **/
      DataPacketSend (p_requestPacket->name, revlevel, (T_word16)size, data);
   }
   else
   {
      printf ("Warning! File %d, rev %d requested, but denied!\n",
              p_requestPacket->name, p_requestPacket->revlevel);

      /** Create a "file not here" packet. **/
      ((T_fileNotHerePacket *)(packet.data))->command =
         PACKET_COMMANDTR_FILE_NOT_HERE;
      ((T_fileNotHerePacket *)(packet.data))->filename =
         p_requestPacket->name;

      CmdQSendShortPacket (&packet, 140, 0, NULL);
   }

   DebugEnd ();
}

/****************************************************************************/
/*  Routine:  DataPacketSend                                                */
/****************************************************************************/
/*  Description:                                                            */
/*    This routine, unlike the read facilities, is not asynchronous.  To    */
/*  send a block of data, just call this routine.  It breaks the data up    */
/*  and handles all the protocol and transmission issues until the data is  */
/*  sent.                                                                   */
/*    NOTE!!!  This sends the data out the currently active port!           */
/*                                                                          */
/*  Problems:                                                               */
/*    The fact that this routine blocks until the data is all sent, rather  */
/*  than doing something neat and asynchronous, _may_ turn out to be a      */
/*  problem, but I doubt it.                                                */
/*                                                                          */
/*  Inputs:                                                                 */
/*    T_word32 name               -- "Filename" of the file .               */
/*    T_word32 revlevel           -- Revision level to post.                */
/*    T_word16 size               -- Size of the data to transmit.          */
/*    T_byte8 *data               -- Data to transmit.                      */
/*    T_dataCallback callback     -- Function to call when data is done.    */
/*                                                                          */
/*  Outputs:                                                                */
/*    None.                                                                 */
/*                                                                          */
/*  Revision History:                                                       */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    AMT  06/30/95  Created                                                */
/****************************************************************************/
T_void DataPacketSend (T_word32 name,
                       T_word32 revlevel,
                       T_word16 size,
                       T_byte8 *data)
{
   T_word16 fileID, i;
   T_packetShort packet;

   DebugRoutine ("DataPacketSend");

   /** First, locate the first available fileID. **/
   for (fileID = 0; (fileID < 256) && 
        (G_transfers[fileID].direction != DATA_DIRECTION_OFF); fileID ++);

   /** Make sure I found one. **/
   DebugCheck (fileID < 256);

   /** Set up the transfer record. **/
   G_transfers[fileID].filename = name;
   G_transfers[fileID].revlevel = revlevel;
   G_transfers[fileID].totalSize = size;
   G_transfers[fileID].totalPackets =
                                size / GENERIC_DATA_PACKET_CHUNK_SIZE + 1;
   G_transfers[fileID].direction = DATA_DIRECTION_OUTGOING;
   G_transfers[fileID].dataArea = MemAlloc (size);

   /** Copy the data over. **/
   memcpy (G_transfers[fileID].dataArea, data, size);

   /** Create a data transfer start packet.  **/
   ((T_genericDataStartTransferPacket *)(packet.data))->command =
                                            PACKET_COMMANDTR_START_TRANSFER;
   ((T_genericDataStartTransferPacket *)(packet.data))->size = size;
   ((T_genericDataStartTransferPacket *)(packet.data))->fileID = (T_byte8)fileID;
   ((T_genericDataStartTransferPacket *)(packet.data))->name = name;

   /** And send it. **/
puts ("Sending start packet.");
fflush (stdout);
fflush (stdout);
   CmdQSendShortPacket (&packet, 140, 0, NULL);

   /** After that, send the data itself. **/
   for (i = 0; i < G_transfers[fileID].totalPackets; i++)
   {
printf ("Sending data packet %d\n", i);
fflush (stdout);
fflush (stdout);
      IDataPacketSendPacket ((T_byte8)fileID, (T_word16)i);
   }

   /** And now I just wait for a TransferComplete event. **/

   DebugEnd ();
}

/****************************************************************************/
/*  Routine:  IDataPacketSendPacket                                         */
/****************************************************************************/
/*  Description:                                                            */
/*    Called to send a specific segment of the data out the port.           */
/*                                                                          */
/*  Problems:                                                               */
/*    None.                                                                 */
/*                                                                          */
/*  Inputs:                                                                 */
/*    T_byte8 fileID              -- ID of file from which to send packet.  */
/*    T_word16 pos                -- Position of the packet in the file.    */
/*                                                                          */
/*  Outputs:                                                                */
/*    None.                                                                 */
/*                                                                          */
/*  Revision History:                                                       */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    AMT  07/09/95  Created                                                */
/****************************************************************************/
T_void IDataPacketSendPacket (T_byte8 fileID, T_word16 pos)
{
   T_packetLong packet;
   T_word16 len;
   T_genericDataPacket *p_dataPacket;

   DebugRoutine ("DataPacketResendPacket");

   /** Get a quick pointer. **/
   p_dataPacket = ((T_genericDataPacket *)(packet.data));

   p_dataPacket->header.command = PACKET_COMMANDTR_DATA_PACKET;
   p_dataPacket->header.position = pos;
   p_dataPacket->header.fileID = fileID;

   /** Am I the first packet? **/
   if (pos == 0)
   {
      /** Yes.  Send the revlevel. **/
      p_dataPacket->header.revlevel = G_transfers[fileID].revlevel;
   }

   /** Am I the last packet? **/
   if (pos < G_transfers[fileID].totalPackets - 1)
   {
      /** No.  Just send a chunk. **/
      p_dataPacket->header.size = 0;
      memcpy (p_dataPacket->data,
         G_transfers[fileID].dataArea + pos * GENERIC_DATA_PACKET_CHUNK_SIZE,
         GENERIC_DATA_PACKET_CHUNK_SIZE);
   }
   else
   {
      /* Yes. Calculate how many bytes I will contain. **/
      len = G_transfers[fileID].totalSize % GENERIC_DATA_PACKET_CHUNK_SIZE;

      p_dataPacket->header.size = (T_byte8)len;
      memcpy (p_dataPacket->data,
        G_transfers[fileID].dataArea + pos * GENERIC_DATA_PACKET_CHUNK_SIZE,
        len);
   }

printf("Sending packet... (num=%d, size=%d)\n", p_dataPacket->header.position,
              p_dataPacket->header.size);
   CmdQSendLongPacket (&packet, 140, 0, NULL);

   DebugEnd ();
}

/****************************************************************************/
/*  Routine:  DataPacketTransferComplete                                    */
/****************************************************************************/
/*  Description:                                                            */
/*    The receiver of a file sends a TRANSFER_COMPLETE packet when it has   */
/*  gotten everything.  This routine responds to such a packet.             */
/*                                                                          */
/*  Problems:                                                               */
/*    None.                                                                 */
/*                                                                          */
/*  Inputs:                                                                 */
/*    T_packetEitherShortOrLong * -- TRANSFER_COMPLETE packet received.     */
/*                     p_packet                                             */
/*  Outputs:                                                                */
/*    ...                         -- ...                                    */
/*                                                                          */
/*  Revision History:                                                       */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    AMT    /  /    Created                                                */
/****************************************************************************/
T_void DataPacketTransferComplete (T_packetEitherShortOrLong *p_packet)
{
   T_byte8 fileID;

   DebugRoutine ("DataPacketTransferComplete");

   fileID = ((T_fileTransferCompletePacket *)(p_packet->data))->fileID;

   /** This should not be called on a 'finished' transfer. **/
   DebugCheck (G_transfers[fileID].direction != DATA_DIRECTION_OFF);

   G_transfers[fileID].direction = DATA_DIRECTION_OFF;
   MemFree (G_transfers[fileID].dataArea);

   DebugEnd ();
}

/****************************************************************************/
/*  Routine:  IDataPacketSendResendRequest                                  */
/****************************************************************************/
/*  Description:                                                            */
/*    Sends a RESEND_PLEASE packet to request the resending of a lost packet*/
/*                                                                          */
/*  Problems:                                                               */
/*    None.                                                                 */
/*                                                                          */
/*  Inputs:                                                                 */
/*    T_byte8 fileID              -- Which currently in-progress file?      */
/*    T_word16 position           -- Ordinal number of requested packet     */
/*                                                                          */
/*  Outputs:                                                                */
/*    None.                                                                 */
/*                                                                          */
/*  Revision History:                                                       */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    AMT  07/10/95  Created                                                */
/****************************************************************************/
T_void IDataPacketSendResendRequest (T_byte8 fileID, T_word16 position)
{
   T_packetShort packet;

   DebugRoutine ("IDataPacketSendResendRequest");

   ((T_resendPleasePacket *)(packet.data))->command =
                                            PACKET_COMMANDRT_RESEND_PLEASE;
   ((T_resendPleasePacket *)(packet.data))->fileID = fileID;
   ((T_resendPleasePacket *)(packet.data))->position = position;

   CmdQSendShortPacket (&packet, 140, 0, NULL);

   DebugEnd ();
}

/****************************************************************************/
/*    END OF FILE:  filetran.c                                              */
/****************************************************************************/
