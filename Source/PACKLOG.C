/****************************************************************************/
/*    FILE:  PACKLOG.C                                                      */
/****************************************************************************/
#include "FILE.H"
#include "HASH32.H"
#include "MEMORY.H"
#include "PACKLOG.H"
#include "SYNCTIME.H"

#define PACKET_LOG_TAG         (*((T_word32 *)"PLTg"))
#define PACKET_LOG_DEAD_TAG    (*((T_word32 *)"DplG"))

#define PACKET_LOG_PACKET_TAG       (*((T_word32 *)"PlPt"))
#define PACKET_LOG_PACKET_DEAD_TAG  (*((T_word32 *)"DpLp"))

#define PACKET_LOG_UPDATE_BLOCK_HEADER  "PbHd"

typedef struct {
    T_word32 tag ;
    T_word32 time ;                        /* Time of packet. */
    T_hash32Item item ;                    /* Handle to this hash item. */
    T_doubleLinkListElement element ;      /* Element in the order list. */
    T_byte8 data[] ;                       /* Packet Data follows. */
} T_packetLogPacketHeaderStruct ;

typedef struct {
    T_word32 tag ;                         /* Tag to identify this struct. */
    T_word16 numPackets ;                  /* Number type of packets. */
    T_packetLogParam *p_parameters ;       /* Array of parameters, one */
                                           /* per type of packet. */
    T_hash32 *p_hashTables ;               /* Hash table per packet type. */
    T_doubleLinkList subLogs ;             /* List of sub-logs created */
                                           /* from splitting. */
    T_doubleLinkList orderedLog ;          /* List of the packets in order */
                                           /* that they are created. */
    T_doubleLinkListElement child ;        /* Child element if part */
                                           /* of a master log. */
} T_packetLogStruct ;

/* Internal prototypes: */
static T_void IPacketLogLoadPacket(
                  T_packetEitherShortOrLong *p_packet,
                  T_word32 extraData) ;
static T_void ITestParse(
                  T_packetEitherShortOrLong *p_packet,
                  T_word32 extraData) ;

/****************************************************************************/
/*  Routine:  PacketLogCreate                                               */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    PacketLogCreate starts up a new packet log in memory.  Just pass      */
/*  to it the packet parameters that set up the rules on what to do.        */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    The passed in parameters are NOT copied, therefore, they cannot       */
/*  be removed or changed in memory until the log is destroyed.             */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word16 numPackets         -- Num packets types (num items in array) */
/*                                                                          */
/*    T_packetLogParam *params    -- Parameter array                        */
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
/*    DoubleLinkListCreate                                                  */
/*    Hash32Create                                                          */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/25/96  Created                                                */
/*    LES  01/26/96  Added the ordered list                                 */
/*                                                                          */
/****************************************************************************/

T_packetLog PacketLogCreate(
                T_word16 numPackets,
                T_packetLogParam *params)
{
    T_word16 i ;
    T_packetLogStruct *p_log = NULL ;
    DebugRoutine("PacketLogCreate");
    DebugCheck(numPackets != 0) ;

    if (numPackets != 0)  {
        p_log = MemAlloc(sizeof(T_packetLogStruct)) ;
        DebugCheck(p_log != NULL) ;
        if (p_log)  {
            /* Initialize the data for the log structure. */
            p_log->numPackets = numPackets ;
            p_log->p_parameters = params ;

            /* Set up a sub-list type. */
            p_log->subLogs = DoubleLinkListCreate() ;
            DebugCheck(p_log->subLogs != DOUBLE_LINK_LIST_BAD) ;

            /* Set up the order log. */
            p_log->orderedLog = DoubleLinkListCreate() ;
            DebugCheck(p_log->orderedLog != DOUBLE_LINK_LIST_BAD) ;

            /* Set up the hash tables for those items that */
            /* need hash tables. */
            p_log->p_hashTables = MemAlloc(sizeof(T_hash32) * numPackets) ;
            DebugCheck(p_log->p_hashTables) ;
            if (p_log->p_hashTables)  {
                /* Initialize each hash table.  Only those packet types */
                /* that we are NOT ignoring get a hash table. */
                for (i=0; i<numPackets; i++)  {
                    if (p_log->p_parameters[i].type ==
                        PACKET_LOG_PARAM_TYPE_IGNORE)
                        p_log->p_hashTables[i] = HASH32_BAD ;
                    else
                        p_log->p_hashTables[i] = Hash32Create(256) ;
                }
            }

            /* This is a master list by default. */
            p_log->child = DOUBLE_LINK_LIST_ELEMENT_BAD ;

            /* Mark this packet log as good. */
            p_log->tag = PACKET_LOG_TAG ;
        }
    }

    DebugEnd() ;

    return ((T_packetLog)p_log) ;
}

/****************************************************************************/
/*  Routine:  PacketLogDestroy                                              */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    PacketLogDestroy destroys everything in a given packet log INCLUDING  */
/*  all the data attached to the packet log.                                */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    Nothing will remain with the packet log since it does all the memory  */
/*  allocation.                                                             */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_packetLog log             -- Packet log to destroy                  */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    DoubleLinkListGetFirst                                                */
/*    DoubleLinkListElementGetData                                          */
/*    PacketLogDestroy                                                      */
/*    DoubleLinkListRemoveElement                                           */
/*    DoubleLinkListDestroy                                                 */
/*    Hash32GetFirstItem                                                    */
/*    Hash32ItemGetData                                                     */
/*    MemFree                                                               */
/*    Hash32Remove                                                          */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/25/96  Created                                                */
/*    LES  01/26/96  Added the ordered list                                 */
/*                                                                          */
/****************************************************************************/

T_void PacketLogDestroy(T_packetLog log)
{
    T_word16 i ;
    T_packetLogStruct *p_log ;
    T_doubleLinkListElement element ;
    T_packetLog subLog ;
    T_hash32Item packetItem ;
    T_hash32 hash ;
    T_void *p_data ;

    DebugRoutine("PacketLogDestroy");

    DebugCheck(log != PACKET_LOG_BAD) ;
    if (log != PACKET_LOG_BAD)  {
        /* Get a quick pointer to the packet log. */
        p_log = (T_packetLogStruct *)log ;

        DebugCheck(p_log->tag == PACKET_LOG_TAG) ;
        if (p_log->tag == PACKET_LOG_TAG)  {
            /* First of all things, go through the list of */
            /* sub packet logs and destroy each one of those. */
            while ((element = DoubleLinkListGetFirst(p_log->subLogs)) !=
                      DOUBLE_LINK_LIST_ELEMENT_BAD)  {
                subLog = (T_packetLog)DoubleLinkListElementGetData(element) ;

                /* Destroy the sub-log. */
                PacketLogDestroy(subLog) ;
                DoubleLinkListRemoveElement(element) ;
            }
            /* Get rid of the actual double link list. */
            DoubleLinkListDestroy(p_log->subLogs) ;

            /* Go through all the hash tables and destroy all */
            /* the elements. */
            for (i=0; i<p_log->numPackets; i++)  {
                hash = p_log->p_hashTables[i] ;
                if (hash != HASH32_BAD)  {
                    /* Go through the list and remove and destroy */
                    /* all the items. */
                    while ((packetItem = Hash32GetFirstItem(hash)) !=
                             HASH32_ITEM_BAD)  {
                        /* Get the hash item's data. */
                        p_data = Hash32ItemGetData(packetItem) ;
                        PacketLogPacketDestroy((T_packetLogPacket)p_data) ;
#if 0

                        /* Free the data. */
                        MemFree(p_data) ;

                        /* remove it from the hash table. */
                        Hash32Remove(packetItem) ;
#endif
                    }

                    /* Finally, destroy the hash header. */
                    Hash32Destroy(hash) ;
                }
            }

            /* Get rid of the ordered log. */
            /* NOte that I should not have to destroy the items */
            /* since they are shared with the hash tables.  Only */
            /* the actual list needs to be destroyed. */
            DoubleLinkListDestroy(p_log->orderedLog) ;

            /* Remove myself from the parent list, IF I am a child. */
            if (p_log->child != DOUBLE_LINK_LIST_ELEMENT_BAD)
                DoubleLinkListRemoveElement(p_log->child) ;

            /* Destroy the header, and remember to delete the tag. */
            p_log->tag = PACKET_LOG_DEAD_TAG ;
            MemFree(p_log) ;
        }
    }
    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  PacketLogSplit                                                */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    PacketLogSplit makes a sub packet log out of the given log.  This     */
/*  allows packet updates to traverse down into the packet log group.       */
/*  The new packet log is totally empty, but depends on how packets         */
/*  are added by its parent packet.                                         */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_packetLog log             -- Log to split                           */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_packetLog                 -- Created packet log attached to         */
/*                                   log that was split.                    */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    PacketLogCreate                                                       */
/*    DoubleLinkListAddElementAtEnd                                         */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/25/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_packetLog PacketLogSplit(T_packetLog log)
{
    T_packetLogStruct *p_log ;
    T_packetLog subLog = PACKET_LOG_BAD ;
    T_doubleLinkListElement child ;

    DebugRoutine("PacketLogSplit");

    DebugCheck(log != PACKET_LOG_BAD) ;
    if (log != PACKET_LOG_BAD)  {
        /* Get a quick pointer to the packet log. */
        p_log = (T_packetLogStruct *)log ;

        DebugCheck(p_log->tag == PACKET_LOG_TAG) ;
        if (p_log->tag == PACKET_LOG_TAG)  {
            /* Try creating a sub-log with the same parameters. */
            subLog = PacketLogCreate(p_log->numPackets, p_log->p_parameters) ;
            DebugCheck(subLog != PACKET_LOG_BAD) ;

            /* If the sub log was created, add it to */
            /* the list of sub logs for this log. */
            if (subLog != PACKET_LOG_BAD)  {
                child = DoubleLinkListAddElementAtEnd(
                            p_log->subLogs,
                            (T_void *)subLog) ;

                /* Mark the sub-log as a child. */
                ((T_packetLogStruct *)subLog)->child = child ;
            }
        }
    }

    DebugEnd() ;

    return subLog ;
}

/****************************************************************************/
/*  Routine:  PacketLogAddPacket                                            */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    PacketLogAddPacket takes the given packet and analyzes how it         */
/*  will fit into the given packet log.  Note that this also carries the    */
/*  packet down into into any sub-logs attached to the given packet log.    */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_packetLog log             -- Packet log to add packet to.           */
/*                                                                          */
/*    T_packetEitherShortOrLong *p_packet -- Packet to add.                 */
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
/*    memcpy                                                                */
/*    Hash32Find                                                            */
/*    MemFree                                                               */
/*    Hash32Remove                                                          */
/*    Hash32Add                                                             */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/25/96  Created                                                */
/*    LES  01/26/96  Added post process packet callback call                */
/*    LES  01/26/96  Added the call to put packet on ordered list.          */
/*                                                                          */
/****************************************************************************/

T_void PacketLogAddPacket(
           T_packetLog log,
           T_packetEitherShortOrLong *p_packet)
{
    T_packetLogStruct *p_log ;
    T_packetLog subLog = PACKET_LOG_BAD ;
    T_byte8 command ;
    T_word32 key ;
    T_packetLogPacketHeaderStruct *p_packetData ;
    T_packetLogPacketHeaderStruct *p_oldEntry ;
    T_packetLogParam *p_param ;
    T_hash32Item hashPosition ;
    T_doubleLinkListElement element ;

    DebugRoutine("PacketLogAddPacket");

    DebugCheck(log != PACKET_LOG_BAD) ;
    if (log != PACKET_LOG_BAD)  {
        /* Get a quick pointer to the packet log. */
        p_log = (T_packetLogStruct *)log ;

        DebugCheck(p_log->tag == PACKET_LOG_TAG) ;
        if (p_log->tag == PACKET_LOG_TAG)  {
            /* Ok, the log is good.  Let us look at the packet type/command. */
            command = p_packet->data[0] ;
            /* Is this a valid command? */
            DebugCheck(command < p_log->numPackets) ;
            if (command < p_log->numPackets)  {
                /* Look up the parameter structure/instructions */
                /* on what do with this packet type. */
                p_param = p_log->p_parameters+command ;

                /* Ok, what do we do with it? */
                /* Do we ignore it? */
                if (p_param->type != PACKET_LOG_PARAM_TYPE_IGNORE)  {
                    /* Ok, this packet is meaningful.  Let us do */
                    /* something with it. */

                    /* Allocate room for the packet data copy. */
                    p_packetData =
                        MemAlloc(
                            sizeof(T_packetLogPacketHeaderStruct) +
                            p_param->size) ;

                    /* Check if we got the memory. */
                    DebugCheck(p_packetData != NULL) ;
                    if (p_packetData)  {
                        /* Tag the packet. */
                        p_packetData->tag = PACKET_LOG_PACKET_TAG ;

                        /* Move the packet data over. */
                        memcpy(
                            p_packetData->data,
                            p_packet->data,
                            p_param->size) ;

                        /* Okay, fill in the rest of the packet info. */
                        p_packetData->time = SyncTimeGet() ;

                        /* Get the key for this packet. */
                        key = p_param->p_getKeyCallback(p_packet) ;

                        /* Now that we have the key, */
                        /* see if it is already in the hash table. */
                        hashPosition =
                            Hash32Find(
                                p_log->p_hashTables[command],
                                key) ;

                        /* Does it already exist? */
                        if (hashPosition != HASH32_ITEM_BAD)  {
                             /* Yes, it already exists. */
                             /* Get the old entry. */
                             p_oldEntry = (T_packetLogPacketHeaderStruct *)
                                               Hash32ItemGetData(hashPosition) ;
                             PacketLogPacketDestroy(p_oldEntry) ;
#if 0
                             /* Delete both hash entry and data. */
                             MemFree(p_oldEntry) ;
                             Hash32Remove(hashPosition) ;
#endif
                        }

                        /* Now the entry does not exist. */
                        /* Add this one. */
                        hashPosition = Hash32Add(
                                           p_log->p_hashTables[command],
                                           key,
                                           p_packetData) ;

                        /* Record in itself the hash item. */
                        p_packetData->item = hashPosition ;

                        /* Now add the packet to the order list for this */
                        /* log. */
                        p_packetData->element = DoubleLinkListAddElementAtEnd(
                                                    p_log->orderedLog,
                                                    p_packetData) ;

                        /* OK, check if there is a post process routine */
                        /* to call. */
                        if (p_param->p_postProcess)  {
                            p_param->p_postProcess(
                                p_packet,
                                log,
                                (T_packetLogPacket)p_packetData /* self */) ;
                        }
                        /* NOTE:  Don't use p_packetData below this */
                        /* point since it might not exist any more. */
                        p_packetData = NULL ;

                        /* Should we carry the packet addition down */
                        /* to the sub-logs? */
                        if (!(p_param->type & PACKET_LOG_PARAM_TYPE_ONLY_MASTER_LOG))  {
                            /* Yes, do it to each sub-log. */
                            element = DoubleLinkListGetFirst(p_log->subLogs) ;
                            while (element != DOUBLE_LINK_LIST_ELEMENT_BAD)  {
                                subLog = (T_packetLog)
                                    DoubleLinkListElementGetData(element) ;
                                PacketLogAddPacket(subLog, p_packet) ;
                                element = DoubleLinkListElementGetNext(element) ;
                            }
                        }
                    }
                }
            }
        }
    }

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  PacketLogGetUpdateBlock                                       */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    PacketLogGetUpdateBlock goes through all the packets in a packet log  */
/*  and bundles them together into one big block of memory for either       */
/*  transmitting or storage.                                                */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_packetLog log             -- Log to get "bundle"                    */
/*                                                                          */
/*    T_word32 *p_size            -- Returned size of update block          */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_byte8 *                   -- Pointer to created block.              */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    Hash32GetNumberItems                                                  */
/*    MemAlloc                                                              */
/*    Hash32GetFirstItem                                                    */
/*    Hash32ItemGetData                                                     */
/*    Hash32ItemGetNext                                                     */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/25/96  Created                                                */
/*    LES  01/26/96  Use the order list instead of grouping so that actions */
/*                   are correctly parsed in order.                         */
/*                                                                          */
/****************************************************************************/

T_byte8 *PacketLogGetUpdateBlock(T_packetLog log, T_word32 *p_size)
{
    T_word16 i ;
    T_packetLogStruct *p_log ;
    T_word32 blockSize ;
    T_hash32 hashTable ;
    T_packetLogPacketHeaderStruct *p_packetData ;
    T_byte8 *p_block = NULL ;
    T_byte8 *p_pos ;
    T_word16 packetSize ;
    T_doubleLinkListElement element ;

    DebugRoutine("PacketLogGetUpdateBlock");

    DebugCheck(log != PACKET_LOG_BAD) ;
    if (log != PACKET_LOG_BAD)  {
        /* Get a quick pointer to the packet log. */
        p_log = (T_packetLogStruct *)log ;

        DebugCheck(p_log->tag == PACKET_LOG_TAG) ;
        if (p_log->tag == PACKET_LOG_TAG)  {
            /* Ok, log is a valid log ... now let's make that block. */
            /* First we will find the complete size of the block by */
            /* adding up all the size info.  Then we will allocate */
            /* a block with this size.  And finally, we will put */
            /* all the data in the correct format into this block. */

            /* FIRST, find the complete size of the block. */
            blockSize = 4 ;
            /* There is also a four byte header at the begining. */

            /* Look at each hash table. */
            for (i=0; i<p_log->numPackets; i++)  {
                hashTable = p_log->p_hashTables[i] ;
                if (hashTable != HASH32_BAD)  {
                    /* Add in the number of bytes that this */
                    /* hash table has.  It is the multiple of */
                    /* the size of each packet and the number */
                    /* of items in the hash table. */
                    blockSize +=
                        Hash32GetNumberItems(hashTable) *
                        p_log->p_parameters[i].size ;
                }
            }

            if (blockSize > 4)  {
                /* SECOND, Now that we have the block size, store it and create */
                /* a block of the same size. */
                *p_size = blockSize ;
                p_block = MemAlloc(blockSize) ;

                DebugCheck(p_block != NULL) ;
                if (p_block)  {
                    /* THIRD, Fill the block. */

                    /* Again, we will go through the whole list of */
                    /* hash tables and copy out the parts that we */
                    /* need. */

                    /* Set up the pointer that will traverse the block */
                    /* and point at where data will be placed. */
                    p_pos = p_block ;

                    /* Place the header. */
                    strncpy(p_pos, PACKET_LOG_UPDATE_BLOCK_HEADER, 4) ;
                    p_pos += 4;

                    element = DoubleLinkListGetFirst(p_log->orderedLog) ;
                    while (element != DOUBLE_LINK_LIST_ELEMENT_BAD)  {
                        p_packetData = DoubleLinkListElementGetData(element) ;

                        packetSize =
                            p_log->p_parameters[p_packetData->data[0]].size ;

                        /* Copy over only the revelant part. */
                        memcpy(
                            p_pos,
                            p_packetData->data,
                            packetSize) ;

                        /* Move up the position. */
                        p_pos += packetSize ;

                        element = DoubleLinkListElementGetNext(element) ;
                    }

                    DebugCheck((p_block + blockSize) == p_pos) ;
                } else {
                    /* The block has nothing in it, return NULL. */
                    p_block = NULL ;
                    *p_size = 0 ;
                }
                /* OK, should have the block completely filled now. */
                /* It is ready. */
            }
        }
    }

    DebugEnd() ;

    return p_block ;
}

#if 0                /* Backup if I screw up */
T_byte8 *PacketLogGetUpdateBlock(T_packetLog log, T_word32 *p_size)
{
    T_word16 i ;
    T_packetLogStruct *p_log ;
    T_word32 blockSize ;
    T_hash32 hashTable ;
    T_hash32Item item ;
    T_packetLogPacketHeaderStruct *p_packetData ;
    T_byte8 *p_block = NULL ;
    T_byte8 *p_pos ;
    T_word16 packetSize ;

    DebugRoutine("PacketLogGetUpdateBlock");

    DebugCheck(log != PACKET_LOG_BAD) ;
    if (log != PACKET_LOG_BAD)  {
        /* Get a quick pointer to the packet log. */
        p_log = (T_packetLogStruct *)log ;

        DebugCheck(p_log->tag == PACKET_LOG_TAG) ;
        if (p_log->tag == PACKET_LOG_TAG)  {
            /* Ok, log is a valid log ... now let's make that block. */
            /* First we will find the complete size of the block by */
            /* adding up all the size info.  Then we will allocate */
            /* a block with this size.  And finally, we will put */
            /* all the data in the correct format into this block. */

            /* FIRST, find the complete size of the block. */
            blockSize = 4 ;
            /* There is also a four byte header at the begining. */

            /* Look at each hash table. */
            for (i=0; i<p_log->numPackets; i++)  {
                hashTable = p_log->p_hashTables[i] ;
                if (hashTable != HASH32_BAD)  {
                    /* Add in the number of bytes that this */
                    /* hash table has.  It is the multiple of */
                    /* the size of each packet and the number */
                    /* of items in the hash table. */
                    blockSize +=
                        Hash32GetNumberItems(hashTable) *
                        p_log->p_parameters[i].size ;
                }
            }

            if (blockSize > 4)  {
                /* SECOND, Now that we have the block size, store it and create */
                /* a block of the same size. */
                *p_size = blockSize ;
                p_block = MemAlloc(blockSize) ;

                DebugCheck(p_block != NULL) ;
                if (p_block)  {
                    /* THIRD, Fill the block. */

                    /* Again, we will go through the whole list of */
                    /* hash tables and copy out the parts that we */
                    /* need. */

                    /* Set up the pointer that will traverse the block */
                    /* and point at where data will be placed. */
                    p_pos = p_block ;

                    /* Place the header. */
                    strncpy(p_pos, PACKET_LOG_UPDATE_BLOCK_HEADER, 4) ;

                    for (i=0; i<p_log->numPackets; i++)  {
                        hashTable = p_log->p_hashTables[i] ;
                        if (hashTable != HASH32_BAD)  {
                            /* Get a quick value to how big packets */
                            /* are in this hash table. */
                            packetSize = p_log->p_parameters[i].size ;
                            item = Hash32GetFirstItem(hashTable) ;
                            while (item != HASH32_ITEM_BAD)  {
                                /* Get access to the hash item's data. */
                                p_packetData = Hash32ItemGetData(item) ;

                                /* Copy over only the revelant part. */
                                memcpy(
                                    p_pos,
                                    p_packetData->data,
                                    packetSize) ;

                                /* Move up the position. */
                                p_pos += packetSize ;

                                /* Go to the next item in the */
                                /* hash table. */
                                item = Hash32ItemGetNext(item) ;
                            }
                        }
                    }
                } else {
                    /* The block has nothing in it, return NULL. */
                    p_block = NULL ;
                    *p_size = 0 ;
                }
                /* OK, should have the block completely filled now. */
                /* It is ready. */
            }
        }
    }

    DebugEnd() ;

    return p_block ;
}
#endif

/****************************************************************************/
/*  Routine:  PacketLogParseUpdateBlock                                     */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    PacketLogParseUpdateBlock goes through a given block of packet        */
/*  updates (as provided by PacketLogGetUpdateBlock) and parses it out      */
/*  into individual packets.                                                */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    The created packets are not filled with checksum or other header      */
/*  info.  Only the data field of packetShort or packetLong is filled.      */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_byte8 *p_block            -- Block to parse out                     */
/*                                                                          */
/*    T_word32 sizeBlock          -- Size of the block                      */
/*                                                                          */
/*    T_word16 numPackets         -- Number type of packets                 */
/*                                                                          */
/*    T_packetLogParam *params    -- Parameters to tell what size each      */
/*                                   packet is (plus other junk that is     */
/*                                   probably not used in this routine).    */
/*                                                                          */
/*    T_packetLogUpdateCallback p_callback -- Routine to call per packet    */
/*                                            parsed.                       */
/*                                                                          */
/*    T_word32 extraData          -- Extra data to follow the parse callback*/
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    memcpy                                                                */
/*    (p_callback)                                                          */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/25/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void PacketLogParseUpdateBlock(
           T_byte8 *p_block,
           T_word32 sizeBlock,
           T_word16 numPackets,
           T_packetLogParam *params,
           T_packetLogUpdateCallback p_callback,
           T_word32 extraData)
{
    T_packetEitherShortOrLong packet ;
    T_word32 offset ;
    T_byte8 *p_pos ;
    T_byte8 command ;
    T_word16 packetSize ;

    DebugRoutine("PacketLogParseUpdateBlock");

    /* Make sure the header is there on the block. */
    DebugCheck(strncmp(p_block, PACKET_LOG_UPDATE_BLOCK_HEADER, 4) == 0) ;
    if (strncmp(p_block, PACKET_LOG_UPDATE_BLOCK_HEADER, 4) == 0)  {
        /* Start the indexes. */
        offset = 4 ;
        p_pos = p_block+4 ;

        /* Loop while there is still data to process. */
        while (offset < sizeBlock)  {
            /* What type of packet is this? */
            command = *p_pos ;

            /* Is this a valid command? */
            DebugCheck(command < numPackets) ;
            if (command < numPackets)  {
                /* Get the size of the packet. */
                packetSize = params[command].size ;

                /* Make sure this size is legal. */
                DebugCheck(packetSize < LONG_PACKET_LENGTH) ;

                /* Copy over the data. */
                memcpy(packet.data, p_pos, packetSize) ;

                /* Make a call to the callback routine */
                /* to have it do whatever it wants to this */
                /* packet. */
                p_callback(&packet, extraData) ;

                /* Progress to the next packet to parse. */
                p_pos += packetSize ;
                offset += packetSize ;
            } else {
                /* Found a bad packet, don't try continuing. */
                break ;
            }
        }
    }

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  PacketLogSave                                                 */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    PacketLogSave is a quick way to save out the current state of         */
/*  a packet log.                                                           */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    NOTE:  The packet log is saved, but not destroyed.                    */
/*    NOTE:  NONE of the splits/sub logs are saved with this sub-log.       */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_packetLog log             -- Packet log to save                     */
/*                                                                          */
/*    T_byte8 *filename           -- Filename to save log under             */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    FileOpen                                                              */
/*    FileWrite                                                             */
/*    FileClose                                                             */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/25/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void PacketLogSave(T_packetLog log, T_byte8 *filename)
{
    T_byte8 *p_block ;
    T_word32 blockSize ;
    T_file file ;

    DebugRoutine("PacketLogSave");

    p_block = PacketLogGetUpdateBlock(log, &blockSize) ;
    if (p_block)  {
        /* We did get a block, save it. */
        file = FileOpen(filename, FILE_MODE_WRITE) ;
        DebugCheck(file != FILE_BAD) ;
        if (file != FILE_BAD)  {
            FileWrite(file, p_block, blockSize) ;
            FileClose(file) ;
        }
    }

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  PacketLogLoad                                                 */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    PacketLogLoad reads in a previously loaded log file and creates a     */
/*  packet log out of it.  If the file does not exist, this routine still   */
/*  creates the log.                                                        */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_byte8 *filename           -- Filename of log to load                */
/*                                                                          */
/*    T_word16 numPackets         -- Number of packet types                 */
/*                                                                          */
/*    T_packetLogParm *params     -- Parameter array                        */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    PacketLogCreate                                                       */
/*    FileExist                                                             */
/*    FileLoad                                                              */
/*    PacketLogParseUpdateBlock                                             */
/*    IPacketLogLoadPacket                                                  */
/*    MemFree                                                               */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/25/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_packetLog PacketLogLoad(
                T_byte8 *filename,
                T_word16 numPackets,
                T_packetLogParam *params)
{
    T_packetLog log = PACKET_LOG_BAD ;
    T_word32 blockSize ;
    T_byte8 *p_block ;

    DebugRoutine("PacketLogLoad");

    /* Check for a legal list of parameters. */
    DebugCheck(numPackets != 0) ;
    if (numPackets != 0)  {
        /* Regardless of what happens, create a packet log. */
        log = PacketLogCreate(numPackets, params) ;

        DebugCheck(log != PACKET_LOG_BAD) ;
        if (log != PACKET_LOG_BAD)  {
            /* See if the file exists. */
            if (FileExist(filename))  {
                p_block = FileLoad(filename, &blockSize) ;
                DebugCheck(p_block != NULL) ;
                if (p_block)  {
                    /* Parse out the block. */
                    PacketLogParseUpdateBlock(
                        p_block,
                        blockSize,
                        numPackets,
                        params,
                        IPacketLogLoadPacket,
                        (T_word32)log) ;

                    /* Done with it, free it. */
                    MemFree(p_block) ;
                }
            }
        }
    }

    DebugEnd() ;

    return log ;
}

/****************************************************************************/
/*  Routine:  IPacketLogLoadPacket                    * INTERNAL *          */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    IPacketLogLoadPacket loads in a single packet as each call is made    */
/*  from the parsed packets.                                                */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_packetEitherShortOrLong *p_packet -- packet to add                  */
/*                                                                          */
/*    T_word32 extraData          -- Data type                              */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    PacketLogAddPacket                                                    */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/25/96  Created                                                */
/*                                                                          */
/****************************************************************************/

static T_void IPacketLogLoadPacket(
                  T_packetEitherShortOrLong *p_packet,
                  T_word32 extraData)
{
    T_packetLog log ;

    DebugRoutine("IPacketLogLoadPacket") ;

    /* Do something with the parsed block. */
    /* Get the handle to the log item. */
    log = (T_packetLog)extraData ;

    /* Just add this packet like any other. */
    PacketLogAddPacket(log, p_packet) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  PacketLogPacketFind                                           */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    PacketLogPacketFind searches for a packet in a packet log based       */
/*  on the given command/packet type and key.                               */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_packetLog log             -- Log to search                          */
/*                                                                          */
/*    T_word16 command            -- Packet type                            */
/*                                                                          */
/*    T_word32 key                -- Key to find                            */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    Hash32Find                                                            */
/*    Hash32ItemGetData                                                     */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/25/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_packetLogPacket PacketLogPacketFind(
                      T_packetLog log,
                      T_word16 command,
                      T_word32 key)
{
    T_packetLogStruct *p_log ;
    T_packetLogPacket logPacket = PACKET_LOG_PACKET_BAD ;
    T_hash32Item item ;

    DebugRoutine("PacketLogPacketFind");

    DebugCheck(log != PACKET_LOG_BAD) ;
    if (log != PACKET_LOG_BAD)  {
        /* Get a quick pointer to the packet log. */
        p_log = (T_packetLogStruct *)log ;

        DebugCheck(p_log->tag == PACKET_LOG_TAG) ;
        if (p_log->tag == PACKET_LOG_TAG)  {
            /* OK, this is a valid log file. */
            /* Try find the packet. */
            DebugCheck(command < p_log->numPackets) ;
            if (command < p_log->numPackets)  {
                /* Valid command. */
                item = Hash32Find(p_log->p_hashTables[command], key) ;
                if (item != HASH32_ITEM_BAD)  {
                    logPacket = (T_packetLogPacket)Hash32ItemGetData(item) ;
                    DebugCheck(((T_packetLogPacketHeaderStruct *)logPacket)->tag == PACKET_LOG_PACKET_TAG) ;
                }
            }
        }
    }

    DebugEnd() ;

    return logPacket ;
}

/****************************************************************************/
/*  Routine:  PacketLogPacketGetData                                        */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    PacketLogPacketGetData retrieves the data related to a given          */
/*  logPacket.                                                              */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    This routine does not check for a valid logPacket (other than NULL).  */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_packetLogPacket logPacket -- Handle to packet log packet            */
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
/*    LES  01/25/96  Created                                                */
/*                                                                          */
/****************************************************************************/

T_byte8 *PacketLogPacketGetData(T_packetLogPacket logPacket)
{
    T_byte8 *p_data = NULL ;
    DebugRoutine("PacketLogPacketGetData");
    DebugCheck(logPacket != PACKET_LOG_PACKET_BAD) ;

    if (logPacket != PACKET_LOG_PACKET_BAD)  {
        DebugCheck(((T_packetLogPacketHeaderStruct *)logPacket)->tag == PACKET_LOG_PACKET_TAG) ;
        p_data = ((T_packetLogPacketHeaderStruct *)logPacket)->data ;
    }

    DebugEnd() ;

    return p_data ;
}

/****************************************************************************/
/*  Routine:  PacketLogPacketDestroy                                        */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    PacketLogPacketDestroy gets rid of a given packet log packet.         */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    This routine does not check for a valid logPacket (other than NULL).  */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_packetLogPacket logPacket -- Packet log packet to destroy.          */
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
/*    Hash32Remove                                                          */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/25/96  Created                                                */
/*    LES  01/26/96  Added call to remove log packet from ordered list      */
/*                                                                          */
/****************************************************************************/

T_void PacketLogPacketDestroy(T_packetLogPacket logPacket)
{
    T_packetLogPacketHeaderStruct *p_packet ;
    T_hash32Item item ;
    T_doubleLinkListElement element ;

    DebugRoutine("PacketLogPacketDestroy");
    DebugCheck(logPacket != PACKET_LOG_PACKET_BAD) ;

    if (logPacket != PACKET_LOG_PACKET_BAD)  {
        p_packet = (T_packetLogPacketHeaderStruct *)logPacket ;

        DebugCheck(p_packet->tag == PACKET_LOG_PACKET_TAG) ;
        if (p_packet->tag == PACKET_LOG_PACKET_TAG)  {
            /* Get the hash handle. */
            item = p_packet->item ;

            /* Get the link list handle to the ordered list. */
            element = p_packet->element ;

            /* Destroy the hash entry. */
            Hash32Remove(item) ;

            /* Get rid of the linked list element. */
            DoubleLinkListRemoveElement(element) ;

            /* Tag it as freed. */
            p_packet->tag = PACKET_LOG_PACKET_DEAD_TAG ;

            /* Free the associated memory. */
            MemFree(p_packet) ;
        }
    }

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  PacketLogDump                           * DEBUG ONLY *        */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    PacketLogDump dumps all data associated with each packet log.         */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_packetLog log             -- Packet log to dump                     */
/*                                                                          */
/*    FILE *fp                    -- File stream to output to               */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    fprintf                                                               */
/*    Hash32GetNumberItems                                                  */
/*    Hash32GetFirstItem                                                    */
/*    Hash32ItemGetNext                                                     */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/25/96  Created                                                */
/*    LES  01/26/96  Added code to show the ordered list.                   */
/*                                                                          */
/****************************************************************************/

#ifndef NDEBUG
T_void PacketLogDump(T_packetLog log, FILE *fp)
{
    T_packetLogStruct *p_log ;
    T_word16 i ;
    T_hash32 hash ;
    T_hash32Item item ;
    T_packetLogPacketHeaderStruct *p_packet ;
    T_doubleLinkListElement element ;
T_byte8 *p_block ;
T_word32 blockSize ;

    DebugRoutine("PacketLogDump") ;

    fprintf(fp, "--- PacketLogDump: %p\n", log) ;
    if (log)  {
        p_log = (T_packetLogStruct *)log ;
        if (p_log->tag == PACKET_LOG_TAG)  {
            fprintf(fp, "    Parameters table:\n") ;
            fprintf(fp, "        num packets: %d\n", p_log->numPackets) ;
            for (i=0; i<p_log->numPackets; i++)  {
                fprintf(fp, "        0x%02X %3d %p %p\n",
                    p_log->p_parameters[i].type,
                    p_log->p_parameters[i].size,
                    p_log->p_parameters[i].p_postProcess,
                    p_log->p_parameters[i].p_getKeyCallback) ;
            }

            for (i=0; i<p_log->numPackets; i++)  {
                fprintf(fp, "    Packet type %3d: ", i) ;
                hash = p_log->p_hashTables[i] ;

                if (hash)  {
                    fprintf(fp, "%d items\n", Hash32GetNumberItems(hash)) ;
                    item = Hash32GetFirstItem(hash) ;
                    while (item != HASH32_ITEM_BAD)  {
                        p_packet = Hash32ItemGetData(item) ;
                        fprintf(fp, "        %p T%08d %p %2d %d\n",
                            p_packet,
                            p_packet->time,
                            p_packet->item,
                            p_packet->data[0],
                            *((T_word16 *)(p_packet->data+1))) ;
                        DebugCheck(p_packet->tag == PACKET_LOG_PACKET_TAG) ;
                        item = Hash32ItemGetNext(item) ;
                    }
                } else {
                    fprintf(fp, "IGNORED\n") ;
                }
            }

            printf("    Ordered List:  %d\n",
                   DoubleLinkListGetNumberElements(p_log->orderedLog)) ;
            element = DoubleLinkListGetFirst(p_log->orderedLog) ;
            while (element != DOUBLE_LINK_LIST_ELEMENT_BAD)  {
                p_packet = DoubleLinkListElementGetData(element) ;
                fprintf(fp, "        %p T%08d %p %2d %d\n",
                    p_packet,
                    p_packet->time,
                    p_packet->item,
                    p_packet->data[0],
                    *((T_word16 *)(p_packet->data+1))) ;
                DebugCheck(p_packet->tag == PACKET_LOG_PACKET_TAG) ;
                element = DoubleLinkListElementGetNext(element) ;
            }
        } else {
            fprintf(fp, "    PACKET LOG BAD TAG!\n") ;
        }
    } else {
        fprintf(fp, "    NULL PACKET LOG!") ;
    }
    fprintf(fp, "--- PacketLogDump End\n") ;
    fflush(fp) ;

    fprintf(fp, "RAW DUMP: >>>\n") ;
    p_block = PacketLogGetUpdateBlock(log, &blockSize) ;
    if (p_block)  {
        fwrite(p_block, blockSize, 1, fp) ;
        fprintf(fp, "<<< RAW DUMP\n") ;
        fflush(fp) ;
        fprintf(fp, "\n\nParsing block: size: %d\n", blockSize) ;
        fflush(fp) ;
        PacketLogParseUpdateBlock(
            p_block,
            blockSize,
            p_log->numPackets,
            p_log->p_parameters,
            ITestParse,
            (T_word32)fp) ;
        fprintf(fp, "Freeing memory block\n") ;
        fflush(fp) ;
        MemFree(p_block) ;
    } else {
        fprintf(fp, "--- NULL ---\n") ;
    }
    fprintf(fp, "DUMP COMPLETE\n") ;
    fflush(fp) ;

    DebugEnd() ;
}

static T_void ITestParse(
                  T_packetEitherShortOrLong *p_packet,
                  T_word32 extraData)
{
    FILE *fp ;
    DebugRoutine("ITestParse") ;

    fp = (FILE *)extraData ;

    fprintf(fp, "Parse block: %d %d\n", p_packet->data[0], *((T_word16 *)(p_packet->data+1))) ;

    DebugEnd() ;
}

#endif

/****************************************************************************/
/*    END OF FILE:  PACKLOG.C                                               */
/****************************************************************************/
