/****************************************************************************/
/*    FILE:  COMM.C                                                         */
/****************************************************************************/

#include "standard.h"
#include "ll_comm.h"

#define MAX_SIZE_LOOK_AHEAD 200

static T_word16 G_subtype = COMM_SUB_TYPE_UNKNOWN;

static E_Boolean G_clientAndServerExist = FALSE ;

/* Flag that tells if we are a server. */
static E_Boolean G_isServer = FALSE ;

/* Structure to contain all the information about a port that we need. */
typedef struct {
    E_commType type ;
    T_word16 comNum ;
    T_word16 address ;
    T_word16 irq ;
    E_commBaudRate baud ;
    COMM irqComPort ;

    T_sword16 lookAheadStart ;
    T_sword16 lookAheadEnd ;
    T_sword16 lookAheadCount ;
    T_byte8 lookAheadBuffer[MAX_SIZE_LOOK_AHEAD] ;
} T_commPortStruct ;

/* Integer lookup for the corresponding baud enumeration type. */
static T_word32 baudLookUp[] = {
    2400,
    9600,
    19200,
    57600
} ;

/* Table lookup for the numbers necessary to send to bios for the */
/* different baud rates. */
static T_byte8 baudForBios[] = {
    5,
    7,
    8,
    10
} ;

static T_commPort G_ports[MAX_COMM_PORTS] ;
static T_word16 G_numPorts = 0 ;

#define SELF_COMM_BUFFER_SIZE 1024

typedef struct {
    T_byte8 readBuffer[SELF_COMM_BUFFER_SIZE] ;
    T_word16 count ;
    T_word16 readIn ;
    T_word16 readOut ;
} T_selfCommInfo ;

static T_selfCommInfo *G_selfComm = NULL ;
static T_selfCommInfo *G_selfClientComm = NULL ;

/* Pointer to the currently active port. */
T_commPortStruct *G_currentPort = COMM_PORT_BAD ;

/* Declare pointers to the functions that do the actual read and write. */
T_word16 (*LowLevelCommReadByte)(T_void) ;
T_void (*LowLevelCommSendByte)(T_byte8 byte);
T_word16 (*LowLevelCommGetReadBufferLength)(T_void);
T_word16 (*LowLevelCommGetWriteBufferLength)(T_void);

/* Internal prototypes: */
static T_word16 ModemReadByte(T_void) ;
static T_void ModemWriteByte(T_byte8) ;
static T_word16 ModemGetReadBufferLength(T_void) ;
static T_word16 ModemGetWriteBufferLength(T_void) ;

static T_word16 IrqReadByte(T_void) ;
static T_void IrqWriteByte(T_byte8) ;
static T_word16 IrqGetReadBufferLength(T_void) ;
static T_word16 IrqGetWriteBufferLength(T_void) ;

static T_word16 SelfReadByte(T_void) ;
static T_void SelfWriteByte(T_byte8) ;
static T_word16 SelfGetReadBufferLength(T_void) ;
static T_word16 SelfGetWriteBufferLength(T_void) ;

static T_word16 SelfClientReadByte(T_void) ;
static T_void SelfClientWriteByte(T_byte8) ;
static T_word16 SelfClientGetReadBufferLength(T_void) ;
static T_word16 SelfClientGetWriteBufferLength(T_void) ;

/****************************************************************************/
/*  Routine:  CommOpenPort                                                  */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    CommOpenPort creates a port and initializes it to the given           */
/*  parameters.  Pass in the type of connection (null modem, standard       */
/*  modem, or irq based modem), COM number or base address, baud rate,      */
/*  and irq (if an irq is needed).                                          */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    E_commType type             -- type of connection being used          */
/*                                                                          */
/*    T_word16 addressOrComNum    -- base communication address if irq      */
/*                                   modem, or COM port number if           */
/*                                   null or standard modem (NOTE: 1 = COM1)*/
/*                                                                          */
/*    T_word16 irq                -- Interrupt # for irq modem.  Ignored    */
/*                                   for other modem types (just pass 0)    */
/*                                                                          */
/*    E_commBaudRate              -- Communications rate                    */
/*                                                                          */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_commPort                  -- Handle to comm port or COMM_PORT_BAD   */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    MemAlloc                                                              */
/*    int386                                                                */
/*    ioOpenPort                                                            */
/*    ioSetBaud                                                             */
/*    ioSetHandshake                                                        */
/*    ioSetControl                                                          */
/*    ioSetMode                                                             */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  12/30/94  Created                                                */
/*    LES  01/19/94  Added Self and Self Client comm port types.            */
/*                                                                          */
/****************************************************************************/

T_commPort CommOpenPort(
               E_commType type,
               T_word16 addressOrComNum,
               T_word16 irq,
               E_commBaudRate baud)
{
    union REGS r ;

    T_commPortStruct *newPort = COMM_PORT_BAD ;

    DebugRoutine("CommOpenPort") ;
    DebugCheck(type < COMM_TYPE_UNKNOWN) ;
    DebugCheck(baud < BAUD_RATE_UNKNOWN) ;
    DebugCheck(addressOrComNum != 0) ;

    /* Allocate memory for this port. */
    newPort = MemAlloc(sizeof(T_commPortStruct)) ;

    /* Make sure we got that memory or else return with an error. */
    DebugCheck(newPort != NULL) ;
    if (newPort != NULL)  {
        /* Store the data we were given. */
        newPort->type = type ;
        newPort->comNum = addressOrComNum-1 ;
        newPort->address = addressOrComNum ;
        newPort->irq = irq ;
        newPort->baud = baud ;

        /* Initialize look ahead buffer info. */
        newPort->lookAheadStart = 0 ;
        newPort->lookAheadEnd = 0 ;
        newPort->lookAheadCount = 0 ;

        /* Now do the actual initialization based on the type of port */
        /* we are declaring. */
        switch(type)  {
            case COMM_TYPE_NULL_MODEM:
            case COMM_TYPE_STANDARD_MODEM:
                /* Use the new comport number. */

                /* Function #4 */
                r.h.ah = 0x04 ;

                /* no break. */
                r.h.al = 0 ;

                /* no parity. */
                r.h.bh = 0 ;

                /* one stop bit. */
                r.h.bl = 0 ;

                /* 8 bits per byte. */
                r.h.ch = 3 ;

                /* and the baudrate. */
                r.h.cl = baudForBios[baud] ;

                /* Initialize the port. */
#ifdef __386__
                r.x.edx = newPort->comNum ;
                int386(0x14, &r, &r) ;
#else
                r.x.dx = newPort->comNum ;
                int86(0x14, &r, &r) ;
#endif

                DebugCheck(r.h.ah != 0xFF) ;

                break ;
            case COMM_TYPE_IRQ_MODEM:
                newPort->irqComPort = ioOpenPort(addressOrComNum, irq) ;
                ioSetBaud(newPort->irqComPort, baudLookUp[baud]) ;
//                ioSetHandShake(newPort->irqComPort, 0);
                ioSetHandShake(newPort->irqComPort, DTR | RTS);
                ioSetControl(newPort->irqComPort, BITS_8 | STOP_1 | NO_PARITY);
                ioSetMode(newPort->irqComPort, BYTE_MODE);
                break ;
            case COMM_TYPE_SELF:
                DebugCheck(G_selfComm == NULL) ;
                if (G_selfComm == NULL)  {
                    G_selfComm = MemAlloc(sizeof(T_selfCommInfo)) ;
                    memset(G_selfComm, 0, sizeof(T_selfCommInfo)) ;
                }
                break ;
            case COMM_TYPE_SELF_CLIENT:
                DebugCheck(G_selfClientComm == NULL) ;
                if (G_selfClientComm == NULL)  {
                    G_selfClientComm = MemAlloc(sizeof(T_selfCommInfo)) ;
                    memset(G_selfClientComm, 0, sizeof(T_selfCommInfo)) ;
                }
                break ;
            default:
                /* If we got here, something very wrong happened. */
                DebugCheck(FALSE) ;
                break ;
        }
    }

    DebugEnd() ;

    return newPort ;
}

/****************************************************************************/
/*  Routine:  CommClosePort                                                 */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    CommClosePort finalizes the port's existence by removing it from      */
/*  memory and removing any interrupts tied to that port.  It does not      */
/*  worry about hanging up the phone or sending last messages -- this       */
/*  is up to the calling routines.                                          */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_commPort port             -- Port to close                          */
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
/*    ioClosePort                                                           */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/02/94  Created                                                */
/*    LES  01/19/94  Added Self and Self Client comm port types.            */
/*                                                                          */
/****************************************************************************/

T_void CommClosePort(T_commPort port)
{
    DebugRoutine("CommClosePort") ;
    DebugCheck(port != COMM_PORT_BAD) ;

    /* Determine what type of port we have. */
    switch(((T_commPortStruct *)port)->type)  {
       case COMM_TYPE_NULL_MODEM:
       case COMM_TYPE_STANDARD_MODEM:
           /* For standard ports, we don't do anything. */
           break ;
       case COMM_TYPE_IRQ_MODEM:
           /* For ports on an interrupt, we have to close it out. */
           ioClosePort(((T_commPortStruct *)port)->irqComPort) ;
           break ;
       case COMM_TYPE_SELF:
           MemFree(G_selfComm) ;
           break;
       case COMM_TYPE_SELF_CLIENT:
           MemFree(G_selfClientComm) ;
           break;
       default:
           /* Big boo boo if we got here. */
           DebugCheck(FALSE) ;
           break ;
    }

    /* Make sure that if this is the current port, null it. */
    if (G_currentPort == port)
        G_currentPort = COMM_PORT_BAD ;

    /* Free memory for the structure and it's buffers. */
    MemFree(port) ;
    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  CommSetActivePort                                             */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    CommSetActivePort declares what port is to be used in all the next    */
/*  Comm port routine calls.  This also sets up faster ways to call the     */
/*  comm port routines.                                                     */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_commPort port             -- Port to make the active port           */
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
/*    LES  01/02/94  Created                                                */
/*    LES  01/19/94  Added Self and Self Client comm port types.            */
/*                                                                          */
/****************************************************************************/

T_void CommSetActivePort(T_commPort port)
{
char buffer[80] ;
    DebugRoutine("CommSetActivePort") ;
    DebugCheck(port != COMM_PORT_BAD) ;

    /* Declare the currently active port. */
    G_currentPort = port ;

    /* Depending on what type of modem this is, declare the pointers */
    /* to what is an appropriate read and write byte command. */
    switch(((T_commPortStruct *)port)->type)  {
       case COMM_TYPE_NULL_MODEM:
       case COMM_TYPE_STANDARD_MODEM:
           LowLevelCommReadByte = ModemReadByte ;
           LowLevelCommSendByte = ModemWriteByte ;
           LowLevelCommGetReadBufferLength = ModemGetReadBufferLength ;
           LowLevelCommGetWriteBufferLength = ModemGetWriteBufferLength ;
           break ;
       case COMM_TYPE_IRQ_MODEM:
           LowLevelCommReadByte = IrqReadByte ;
           LowLevelCommSendByte = IrqWriteByte ;
           LowLevelCommGetReadBufferLength = IrqGetReadBufferLength ;
           LowLevelCommGetWriteBufferLength = IrqGetWriteBufferLength ;
           break ;
       case COMM_TYPE_SELF:
           LowLevelCommReadByte = SelfReadByte ;
           LowLevelCommSendByte = SelfWriteByte ;
           LowLevelCommGetReadBufferLength = SelfGetReadBufferLength ;
           LowLevelCommGetWriteBufferLength = SelfGetWriteBufferLength ;
           break ;
       case COMM_TYPE_SELF_CLIENT:
           LowLevelCommReadByte = SelfClientReadByte ;
           LowLevelCommSendByte = SelfClientWriteByte ;
           LowLevelCommGetReadBufferLength = SelfClientGetReadBufferLength ;
           LowLevelCommGetWriteBufferLength = SelfClientGetWriteBufferLength ;
           break ;
       default:
           DebugCheck(FALSE) ;
           break ;
    }

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  CommScanByte                                                  */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    CommScanByte looks ahead into the buffer and returns characters as    */
/*  they are found.  If no more characters are found, a zero is returned.   */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    No more than MAX_SIZE_LOOK_AHEAD characters can be scanned ahead.     */
/*  If there are, this problem bombs since there is no more space           */
/*  to store the look ahead characters.                                     */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_word16                    -- Character found (or 0xFFFF if none)    */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    LowLevelCommReadByte                                                  */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/02/94  Created                                                */
/*                                                                          */
/****************************************************************************/

T_word16 CommScanByte(T_void)
{
    T_word16 c ;
    T_word16 i ;

    DebugRoutine("CommScanByte") ;
    DebugCheck(G_currentPort != COMM_PORT_BAD) ;

    /* See if we have anything already in our look ahead buffer. */
    if (G_currentPort->lookAheadCount == 0)  {
        /* There is nothing in the look ahead buffer. */
        /* We need to look ahead and get a byte. */
        c = LowLevelCommReadByte() ;

        /* Did we get a character? */
        if (c != 0xFFFF)  {
            /* Yes, we did. */
            /* Add it to the look ahead buffer. */

            /* Make sure we have enough room for the look ahead. */
            DebugCheck(G_currentPort->lookAheadCount < MAX_SIZE_LOOK_AHEAD) ;

            /* See where the next character goes. */
            i = G_currentPort->lookAheadEnd ;

            /* Store the character and move up the tail. */
            G_currentPort->lookAheadBuffer[i++] = c ;

            /* If the tail is past the end, roll it over. */
            if (i == MAX_SIZE_LOOK_AHEAD)
                i = 0 ;

            /* Store the tail position. */
            G_currentPort->lookAheadEnd = i ;

            /* Increment the count of look ahead characters. */
            G_currentPort->lookAheadCount++ ;
        }
    } else {
        /* Yes, there is already a byte in the look ahead.  Get it. */
        c = G_currentPort->lookAheadBuffer[G_currentPort->lookAheadStart] ;
    }

    DebugEnd() ;

    return c ;
}

/****************************************************************************/
/*  Routine:  CommReadByte                                                  */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    CommReadByte reads in one character in from the appropriate low       */
/*  level driver.  However, if characters are in the look ahead buffer,     */
/*  they are taken out of there first.                                      */
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
/*    T_word16                    -- Character found or 0xFFFF if none.     */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    LowLevelCommReadByte                                                  */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/02/94  Created                                                */
/*                                                                          */
/****************************************************************************/

T_word16 CommReadByte(T_void)
{
    T_word16 c ;
    T_word16 i ;

    DebugRoutine("CommReadByte") ;
    DebugCheck(G_currentPort != COMM_PORT_BAD) ;

    /* Do we have any characters in the look ahead buffer? */
    if (G_currentPort->lookAheadCount != 0)  {
        /* Yes, we do.  Remove one of those characters. */
        /* Start by getting ahold of the start of the buffer */
        i = G_currentPort->lookAheadStart ;

        /* Get the character in the buffer and move up the head. */
        c = G_currentPort->lookAheadBuffer[i++] ;

        /* See if the head has wrapped around. */
        if (i == MAX_SIZE_LOOK_AHEAD)
            i = 0 ;

        /* Store the head position. */
        G_currentPort->lookAheadStart = i ;

        /* Decrement the count of characters in the look ahead buffer. */
        G_currentPort->lookAheadCount-- ;
    } else {
        /* Get a low level character from the port. */
        c = LowLevelCommReadByte() ;
    }
    DebugEnd() ;

    return c ;
}

/****************************************************************************/
/*  Routine:  CommSendByte                                                  */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    CommSendByte puts a character on the output buffer for sending.       */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    If the output buffer is full, the character sent is discarded.        */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_byte8 byte                -- Data to send                           */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    LowLevelCommSendByte                                                  */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/02/94  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void CommSendByte(T_byte8 byte)
{
    DebugRoutine("CommSendByte") ;
    DebugCheck(G_currentPort != COMM_PORT_BAD) ;

    LowLevelCommSendByte(byte) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  CommScanData                                                  */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    CommScanData scans a number of bytes ahead into the read stream       */
/*  and stores the bytes into the given pointer.                            */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    No more than MAX_SIZE_LOOK_AHEAD characters can be scanned ahead.     */
/*  If there are, this problem bombs since there is no more space           */
/*  to store the look ahead characters.                                     */
/*    Make sure to only call this routine if there is already that many     */
/*  characters in the receive buffer.                                       */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_byte8 *p_data             -- Pointer to where to store data         */
/*                                                                          */
/*    T_word16 numberBytes        -- Number of bytes to scan                */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_word16                    -- Number of bytes actually scanned.      */
/*                                   Sometimes this is less than            */
/*                                   numberBytes when there are no more     */
/*                                   characters in the input buffer.        */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    CommScanByte                                                          */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/02/94  Created                                                */
/*                                                                          */
/****************************************************************************/

T_word16 CommScanData(T_byte8 *p_data, T_word16 numberBytes)
{
    T_word16 i ;
    T_word16 c ;
    T_word16 pos ;

    DebugRoutine("CommScanData") ;
    DebugCheck(G_currentPort != COMM_PORT_BAD) ;
    DebugCheck(numberBytes < MAX_SIZE_LOOK_AHEAD) ;
    DebugCheck(p_data != NULL) ;

    /* Let's see if we have already scanned enough characters. */
    while (numberBytes > G_currentPort->lookAheadCount)  {
        /* Scan in a character and store at the end. */
        c = LowLevelCommReadByte() ;

        /* Did we get a character? */
        if (c != 0xFFFF)  {
            /* Yes, we did. */
            /* Add it to the look ahead buffer. */
            /* Make sure we have enough room for the look ahead. */
            DebugCheck(G_currentPort->lookAheadCount < MAX_SIZE_LOOK_AHEAD) ;

            /* See where the next character goes. */
            i = G_currentPort->lookAheadEnd ;

            /* Store the character and move up the tail. */
            G_currentPort->lookAheadBuffer[i++] = c ;

            /* If the tail is past the end, roll it over. */
            if (i == MAX_SIZE_LOOK_AHEAD)
                i = 0 ;

            /* Store the tail position. */
            G_currentPort->lookAheadEnd = i ;

            /* Increment the count of look ahead characters. */
            G_currentPort->lookAheadCount++ ;
        } else {
            /* no, we did not.  Break out. */
            break ;
        }
    }

    /* Copy the group of characters starting at the start of the look */
    /* ahead buffer. */
    pos = G_currentPort->lookAheadStart ;

    for (i=0; i<numberBytes; i++, p_data++)  {
        *p_data = G_currentPort->lookAheadBuffer[pos] ;
        pos++ ;
        if (pos == MAX_SIZE_LOOK_AHEAD)
            pos = 0 ;
    }

    DebugEnd() ;

    /* Return the number of scanned bytes. */
    return i ;
}

/****************************************************************************/
/*  Routine:  CommReadData                                                  */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    CommReadData is used to read n number of bytes from the input stream  */
/*  (taking from the look ahead buffer as needed).                          */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    Make sure to only call this routine if there is already that many     */
/*  characters in the receive buffer.                                       */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_byte8 *p_data             -- Pointer to where to store data         */
/*                                                                          */
/*    T_word16 numberBytes        -- Number of bytes to read                */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_word16                    -- Number of bytes actually read.         */
/*                                   Sometimes this is less than            */
/*                                   numberBytes when there are no more     */
/*                                   characters in the input buffer.        */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    CommReadByte                                                          */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/02/94  Created                                                */
/*                                                                          */
/****************************************************************************/

T_word16 CommReadData(T_byte8 *p_data, T_word16 numberBytes)
{
    T_word16 i ;
    T_word16 c ;

    DebugRoutine("CommReadData") ;
    DebugCheck(G_currentPort != COMM_PORT_BAD) ;
    DebugCheck(p_data != NULL) ;

    /* Try to read in that many number of bytes. */
    for (i=0; i<numberBytes; i++)  {
        c = CommReadByte() ;

        /* Stop if we didn't get a character. */
        if (c == 0xFFFF)
            break ;

        /* Store the data and move forward. */
        *(p_data++) = (T_byte8)c ;
    }

    DebugEnd() ;

    /* Return the number of read bytes. */
    return i ;
}

/****************************************************************************/
/*  Routine:  CommSendData                                                  */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    CommSendData sends n number of bytes out the output port.             */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_byte8 *p_data             -- Pointer to where to get data           */
/*                                                                          */
/*    T_word16 numberBytes        -- Number of bytes to send                */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    CommSendByte                                                          */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/02/94  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void CommSendData(T_byte8 *p_data, T_word16 numberBytes)
{
    T_word16 i ;

    DebugRoutine("CommSendData") ;
    DebugCheck(p_data != NULL) ;
    DebugCheck(G_currentPort != COMM_PORT_BAD) ;

//DebugCheck (CommGetSendBufferLength () < 1071);

    /* Go through the data one by one and send out the characters. */
    for (i=0; i<numberBytes; i++)
        CommSendByte(*(p_data++)) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  CommGetReadBufferLength                                       */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    CommGetReadBufferLength returns the number of characters in the       */
/*  read buffer.                                                            */
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
/*    T_word16                    -- Number of waiting characters.          */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    LowLevelCommGetReadBufferLength                                       */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/02/94  Created                                                */
/*                                                                          */
/****************************************************************************/

T_word16 CommGetReadBufferLength(T_void)
{
    T_word16 length = 0 ;

    DebugRoutine("CommGetReadBufferLength") ;
    DebugCheck(G_currentPort != COMM_PORT_BAD) ;

    length = LowLevelCommGetReadBufferLength() +
             G_currentPort->lookAheadCount ;

    DebugCheck(length < 8000) ;
    DebugEnd() ;

    return length ;
}

/****************************************************************************/
/*  Routine:  CommGetSendBufferLength                                       */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    CommGetSendBufferLength returns the number of USED locations (number  */
/*  of characters that can be sent) in the output buffer.                   */
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
/*    T_word16                    -- Number of free character spots.        */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    LowLevelCommGetWriteBufferLength                                      */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/02/94  Created                                                */
/*                                                                          */
/****************************************************************************/

T_word16 CommGetSendBufferLength(T_void)
{
    T_word16 length = 0 ;

    DebugRoutine("CommGetSendBufferLength") ;
    DebugCheck(G_currentPort != COMM_PORT_BAD) ;

    length = LowLevelCommGetWriteBufferLength() ;

//    DebugCheck(length < 8000) ;
    DebugEnd() ;

    return length ;
}

/****************************************************************************/
/*  Routine:  ModemReadByte                                                 */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ModemReadByte reads a character from the BIOS buffered receive        */
/*  buffers.                                                                */
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
/*    T_word16                    -- Character read or 0xFFFF for none.     */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    int386                                                                */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/02/94  Created                                                */
/*                                                                          */
/****************************************************************************/

static T_word16 ModemReadByte(T_void)
{
    union REGS r ;
    T_word16 c ;

//    DebugRoutine("ModemReadByte") ;

    /* Set up for the com port. */

    /* Prepare for a read command. */
    r.h.ah = 2 ;

    /* Execute. */
#ifdef __386__
    r.x.edx = G_currentPort->comNum ;
    int386(0x14, &r, &r) ;
#else
    r.x.dx = G_currentPort->comNum ;
    int86(0x14, &r, &r) ;
#endif

    /* See if we timed out. */
    if ((r.h.ah & 0x80) != 0)  {
         /* A nothing character. */
         c = 0xFFFF ;
    } else {
         /* The character we received. */
         c = r.h.al ;
    }

//    DebugEnd() ;

    /* Return our character or 0xFFFF */
    return c ;
}

/****************************************************************************/
/*  Routine:  ModemWriteByte                                                */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*     ModemWriteByte sends a character to the output buffer using          */
/*  BIOS calls.                                                             */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_byte8 c                   -- character to send                      */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    int386                                                                */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/02/94  Created                                                */
/*                                                                          */
/****************************************************************************/

static T_void ModemWriteByte(T_byte8 c)
{
    union REGS r ;

//    DebugRoutine("ModemWriteByte") ;

    /* Set up for the com port. */

    /* Prepare for a write command. */
    r.h.ah = 1 ;

    /* Tell what character to send. */
    r.h.al = c ;

    /* Execute and send it. */
#ifdef __386__
    r.x.edx = G_currentPort->comNum ;
    int386(0x14, &r, &r) ;
#else
    r.x.dx = G_currentPort->comNum ;
    int86(0x14, &r, &r) ;
#endif

//    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  ModemGetReadBufferLength                                      */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ModemGetReadBufferLength tells how many characters are in the         */
/*  read buffer.                                                            */
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
/*    T_word16                    -- Number of characters that can be read. */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    int386                                                                */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/02/94  Created                                                */
/*                                                                          */
/****************************************************************************/

static T_word16 ModemGetReadBufferLength(T_void)
{
    union REGS r ;

//    DebugRoutine("ModemGetReadBufferLength") ;

    /* Set up for the com port. */

    /* Prepare for a get buffer len command. */
    r.h.ah = 0x0A ;

    /* Execute. */
#ifdef __386__
    r.x.edx = G_currentPort->comNum ;
    int386(0x14, &r, &r) ;
#else
    r.x.dx = G_currentPort->comNum ;
    int86(0x14, &r, &r) ;
#endif

//    DebugEnd() ;

    /* Return the length */
#ifdef __386__
    return r.x.eax ;
#else
    return r.x.ax ;
#endif
}

/****************************************************************************/
/*  Routine:  ModemGetWriteBufferLength                                     */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    ModemGetWriteBufferLength returns the number of bytes that can        */
/*  be sent out the port using the BIOS commands.                           */
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
/*    T_word16                    -- Number of characters that can be sent. */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    int386                                                                */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/02/94  Created                                                */
/*                                                                          */
/****************************************************************************/

static T_word16 ModemGetWriteBufferLength(T_void)
{
    union REGS r ;
    T_word16 sizeBuffer ;

//    DebugRoutine("ModemGetWriteBufferLength") ;

    /* Set up for the com port. */

    /* Prepare for a get transmit buffer size command. */
    r.h.ah = 0x1B ;
    r.h.al = 0 ;

    /* Execute. */
#ifdef __386__
    r.x.edx = G_currentPort->comNum ;
    int386(0x14, &r, &r) ;
#else
    r.x.dx = G_currentPort->comNum ;
    int86(0x14, &r, &r) ;
#endif

    /* Get the size of the buffer. */
#ifdef __386__
    sizeBuffer = r.x.ebx ;
#else
    sizeBuffer = r.x.bx ;
#endif


    /* Prepare for a get buffer write len command. */
    r.h.ah = 0x12 ;

    /* Execute. */
#ifdef __386__
    r.x.edx = G_currentPort->comNum ;
    int386(0x14, &r, &r) ;
#else
    r.x.dx = G_currentPort->comNum ;
    int86(0x14, &r, &r) ;
#endif

//    DebugEnd() ;

    /* Return the free space */
#ifdef __386__
    return sizeBuffer - r.x.eax ;
#else
    return sizeBuffer - r.x.ax ;
#endif
}

/****************************************************************************/
/*  Routine:  IrqReadByte                                                   */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    IrqReadByte uses the LLCOM routines to read a byte from the irq       */
/*  buffer that it has.                                                     */
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
/*    T_word16                    -- Character or 0xFFFF for none.          */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    ioReadByte                                                            */
/*    ioReadStatus                                                          */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/02/94  Created                                                */
/*                                                                          */
/****************************************************************************/

static T_word16 IrqReadByte(T_void)
{
    T_word16 c ;

//    DebugRoutine("IrqReadByte") ;
    /* See if we have a character read. */
    if (ioReadStatus(G_currentPort->irqComPort))  {
        /* Yes, a character is in the input buffer .. get it. */
        c = ioReadByte(G_currentPort->irqComPort) ;
    } else {
        /* No input character.  Return nothing. */
        c = 0xFFFF ;
    }

//    DebugEnd() ;
    return c ;
}

/****************************************************************************/
/*  Routine:  IrqWriteByte                                                  */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    IrqWriteByte sends out a character out the irq buffer and irq write.  */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_byte8 c                   -- Character to send                      */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    ioWriteByte                                                           */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/02/94  Created                                                */
/*                                                                          */
/****************************************************************************/

static T_void IrqWriteByte(T_byte8 c)
{
//    DebugRoutine("IrqWriteByte") ;
    /* We'll make this one simple, just write it out. */
    ioWriteByte(G_currentPort->irqComPort, c) ;
//    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  IrqGetReadBufferLength                                        */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    IrqGetReadBufferLength determines how many characters are waiting     */
/*  in the irq read buffer.                                                 */
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
/*    T_word16                    -- Number of characters waiting.          */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    ioReadStatus                                                          */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/02/94  Created                                                */
/*                                                                          */
/****************************************************************************/

static T_word16 IrqGetReadBufferLength(T_void)
{
    T_word16 len ;

//    DebugRoutine("IrqGetReadBufferLength") ;

    len = ioReadStatus(G_currentPort->irqComPort) ;

//    DebugEnd() ;

    return len ;
}

/****************************************************************************/
/*  Routine:  IrqGetWriteBufferLength                                       */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    IrqGetWriteBufferLength determines how many more characters can       */
/*  be sent to the output irq buffer before being full.                     */
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
/*    T_word16                    -- Number of characters until full.       */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    ioWriteStatus                                                         */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/02/94  Created                                                */
/*                                                                          */
/****************************************************************************/

static T_word16 IrqGetWriteBufferLength(T_void)
{
    T_word16 len ;

//    DebugRoutine("IrqGetWriteBufferLength") ;

    /* NOTE!!! 4095 is hardcoded to the size of the write buffer. */
//    len = 4095 - ioWriteStatus(G_currentPort->irqComPort) ;
    len = ioWriteStatus(G_currentPort->irqComPort) ;

//    DebugEnd() ;

    return len ;
}

/****************************************************************************/
/*  Routine:  CommRewindScan                                                */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    CommRewindScan moves the current position of the scanned data back    */
/*  the given number of bytes.  This, in effect, allows the system          */
/*  to go to previous characters that were already scanned from the         */
/*  data stream (since they have not been "read" they are still in the      */
/*  scan buffer).                                                           */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    Make sure that you don't rewind more than you have scanned MINUS      */
/*  what you have read.  If you do, this will bomb.                         */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word16 numChar            -- Number of characters to rewind         */
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
/*    LES  01/03/94  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void CommRewindScan(T_word16 numChar)
{
    DebugRoutine("CommRewindScan") ;
    DebugCheck(G_currentPort != COMM_PORT_BAD) ;
    DebugCheck(G_currentPort->lookAheadCount >= numChar) ;

    /* Move the start position of the scan back numChar's. */
    /* If it rolls under, roll it around to the end of the buffer. */
    if ((G_currentPort->lookAheadStart -= numChar) < 0)
        G_currentPort->lookAheadStart += MAX_SIZE_LOOK_AHEAD ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  CommReadConfigFile                                            */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    CommReadConfigFile reads in the "PORT.CFG" file and opens all the     */
/*  ports counting the number of actually opened ports.                     */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    No real errors are detected by this routine and ports that could      */
/*  not be opened are ignored.                                              */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_word16                    -- Number of ports opened                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    CommOpenPort                                                          */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/03/94  Created                                                */
/*    LES  01/19/94  Added Self and Self Client comm port types.            */
/*                                                                          */
/****************************************************************************/

T_word16 CommReadConfigFile(T_byte8 *p_mode, T_byte8 *p_otherData)
{
    E_commType type ;
    static T_word32 baudTable[] = {
        2400,
        9600,
        19200,
        57600
    } ;
    T_iniFile iniFile ;
    T_byte8 *p_value ;
    T_word32 baud ;
    T_word16 ioaddr ;
    T_word16 irq ;
    T_word32 realBaud ;
    T_word16 comPort ;

    DebugRoutine("CommReadConfigFile") ;
    DebugCheck(G_numPorts == 0) ;

    memset(G_ports, 0, sizeof(G_ports)) ;
    iniFile = INIFileOpen("config.ini") ;
    if (iniFile == INIFILE_BAD)  {
        puts("Cannot open CONFIG.INI file!") ;
        puts("Aborting serial driver ... be sure to run correct SETUP.") ;
        exit(1) ;
    }

    p_value = INIFileGet(iniFile, "serial", "comport") ;
    sscanf(p_value+3, "%u", &comPort) ;
    p_value = INIFileGet(iniFile, "serial", "baud") ;
    sscanf(p_value, "%lu", &baud) ;
    p_value = INIFileGet(iniFile, "serial", "ioaddr") ;
    sscanf(p_value+2, "%x", &ioaddr) ;
    p_value = INIFileGet(iniFile, "serial", "irq") ;
    sscanf(p_value, "%u", &irq) ;
    p_value = INIFileGet(iniFile, "serial", "phonenum") ;
    if (p_value)
        strcpy(p_otherData, p_value) ;
    else
        p_otherData[0] = '\0' ;

    if (stricmp(p_mode, "directmaster") == 0)  {
        type = COMM_TYPE_IRQ_MODEM ;
        G_subtype = COMM_SUB_TYPE_DIRECT_MASTER ;
    } else if (stricmp(p_mode, "dial") == 0)  {
        type = COMM_TYPE_IRQ_MODEM ;
        G_subtype = COMM_SUB_TYPE_DIAL ;
    } else if (stricmp(p_mode, "answer") == 0)  {
        type = COMM_TYPE_IRQ_MODEM ;
        G_subtype = COMM_SUB_TYPE_ANSWER ;
    } else if (stricmp(p_mode, "single") == 0)  {
        type = COMM_TYPE_SELF_CLIENT ;
    } else if (stricmp(p_mode, "directslave") == 0)  {
        type = COMM_TYPE_IRQ_MODEM ;
        G_subtype = COMM_SUB_TYPE_DIRECT_SLAVE ;
    } else {
        puts("Warning!  Unknown type of serial connection.") ;
        puts("Defaulting to single player mode.") ;
        type = COMM_TYPE_SELF ;
    }

    for (realBaud=0; realBaud<BAUD_RATE_UNKNOWN; realBaud++)
        if (baud <= baudTable[realBaud])
            break ;
    G_ports[G_numPorts] = CommOpenPort(type, ioaddr, irq, realBaud) ;
    if (G_ports[G_numPorts] != COMM_PORT_BAD)
        G_numPorts++ ;
    if (type == COMM_TYPE_SELF_CLIENT)  {
        /* Open up the server for the client */
        G_ports[G_numPorts] = CommOpenPort(COMM_TYPE_SELF, ioaddr, irq, realBaud) ;
        if (G_ports[G_numPorts] != COMM_PORT_BAD)
            G_numPorts++ ;
        G_clientAndServerExist = TRUE ;
    }

    INIFileClose("config.bak", iniFile) ;
#if 0
    fp = fopen("port.cfg", "r") ;

    G_isServer = FALSE ;
    fgets(buffer, 80, fp) ;
    while (!feof(fp))  {
        switch(buffer[0])  {
            case 'M':           /* Standard modem: M <com port> <baud> */
                sscanf(buffer+1, "%d%d", &comPort, &baud) ;
                type = COMM_TYPE_STANDARD_MODEM ;
                break ;
            case 'N':           /* Null modem: N <com port> <baud> */
                sscanf(buffer+1, "%d%d", &comPort, &baud) ;
                type = COMM_TYPE_NULL_MODEM ;
                break ;
            case 'I':           /* IRQ modem: I <base adr> <irq> <baud> */
                sscanf(buffer+1, "%d%d%ld", &comPort, &irq, &baud) ;
                type = COMM_TYPE_IRQ_MODEM ;
                break ;
            case 'S':
                type = COMM_TYPE_SELF ;
                comPort = 100 ;
                baud = 57600 ;
                G_isServer = TRUE ;
                break ;
            case 'C':
                type = COMM_TYPE_SELF_CLIENT ;
                comPort = 100 ;
                baud = 57600 ;
                G_clientAndServerExist = TRUE ;
                break ;
            default:
                type = COMM_TYPE_UNKNOWN ;
                break ;
        }

        if (type != COMM_TYPE_UNKNOWN)  {
            for (realBaud=0; realBaud<BAUD_RATE_UNKNOWN; realBaud++)
                if (baud <= baudTable[realBaud])
                    break ;
            G_ports[G_numPorts] = CommOpenPort(type, comPort, irq, realBaud) ;
            if (G_ports[G_numPorts] != COMM_PORT_BAD)
                G_numPorts++ ;
        }

        fgets(buffer, 80, fp) ;
    }

    fclose(fp) ;
#endif

    DebugEnd() ;

    return G_numPorts ;
}

/****************************************************************************/
/*  Routine:  CommCloseAll                                                  */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    CommCloseAll closes all the ports opened by CommReadConfigFile.       */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    You should not close any of the ports that were opened by CommRead... */
/*  except by calling this routine.                                         */
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
/*    CommClosePort                                                         */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/03/94  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void CommCloseAll(T_void)
{
    T_word16 i ;

    DebugRoutine("CommCloseAll") ;

    /* Sequentially go through all the ports and close them out. */
    for (i=0; i<G_numPorts; i++)
        CommClosePort(G_ports[i]) ;

    /* Note that no more ports are opened. */
    G_numPorts = 0 ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  CommSetActivePortN                                            */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    CommSetActivePortN sets the active port to be one of the already      */
/*  opened ports by CommReadConfigFile.                                     */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    Make sure you pass in a numOfPort that is less than the number of     */
/*  ports open (the first is 0, ..., number of ports-1).                    */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word16 numOfPort                                                    */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    CommSetActivePort                                                     */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/03/94  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void CommSetActivePortN(T_word16 numOfPort)
{
    DebugRoutine("CommSetActivePortN") ;
//if (numOfPort >= G_numPorts)  {
//    printf("numOfPort = %d, G_numPorts = %d\n", numOfPort, G_numPorts) ; fflush(stdout) ;
//}
    DebugCheck(numOfPort < G_numPorts) ;

    CommSetActivePort(G_ports[numOfPort]) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  CommGetNumberPorts                                            */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    CommGetNumberPorts tells the number of ports previously opened by     */
/*  CommReadConfigFile.  This routine is usually used with                  */
/*  CommSetActivePortN.                                                     */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    You should of called CommReadConfigFile before calling this routine.  */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_word16                    -- Number of opened ports.                */
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
/*    LES  01/03/94  Created                                                */
/*                                                                          */
/****************************************************************************/

T_word16 CommGetNumberPorts(T_void)
{
    T_word16 num ;

    DebugRoutine("CommGetNumberPorts") ;

    num = G_numPorts ;

    DebugEnd() ;

    return num ;
}

/****************************************************************************/
/*  Routine:  SelfReadByte                                                  */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    SelfReadByte is used by the server to read data from the buffer that  */
/*  receives information from the SelfClientWriteByte calls.                */
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
/*    T_word16                    -- Character or 0xFFFF for none.          */
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
/*    LES  01/19/94  Created                                                */
/*                                                                          */
/****************************************************************************/

static T_word16 SelfReadByte(T_void)
{
    T_word16 c ;

//    DebugRoutine("SelfReadByte") ;
    /* See if we have a character to read. */
    if (G_selfComm->count != 0)  {
        /* Yes, a character is in the input buffer .. get it. */
        c = G_selfComm->readBuffer[G_selfComm->readOut++] ;
        G_selfComm->readOut &= SELF_COMM_BUFFER_SIZE-1 ;
        G_selfComm->count-- ;
    } else {
        /* No input character.  Return nothing. */
        c = 0xFFFF ;
    }

//    DebugEnd() ;
    return c ;
}

/****************************************************************************/
/*  Routine:  SelfWriteByte                                                 */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    SelfWriteByte sends a byte out to the current self client.            */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_byte8 c                   -- Character to send                      */
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
/*    LES  01/19/94  Created                                                */
/*                                                                          */
/****************************************************************************/

static T_void SelfWriteByte(T_byte8 c)
{

//    DebugRoutine("SelfWriteByte") ;
    /* We'll make this one simple, just write it out. */
//    DebugCheck(G_selfClientComm != NULL) ;

    if (G_selfClientComm->count != SELF_COMM_BUFFER_SIZE)  {
        G_selfClientComm->readBuffer[G_selfClientComm->readIn++] = c ;
        G_selfClientComm->readIn &= SELF_COMM_BUFFER_SIZE-1 ;
        G_selfClientComm->count++ ;
    }
//    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  SelfGetReadBufferLength                                       */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    SelfGetReadBufferLength determines how many characters are waiting    */
/*  in the self read buffer.                                                */
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
/*    T_word16                    -- Number of characters waiting.          */
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
/*    LES  01/19/94  Created                                                */
/*                                                                          */
/****************************************************************************/

static T_word16 SelfGetReadBufferLength(T_void)
{
    T_word16 len ;

//    DebugRoutine("SelfGetReadBufferLength") ;

    len = G_selfComm->count ;

//    DebugEnd() ;

    return len ;
}

/****************************************************************************/
/*  Routine:  SelfGetWriteBufferLength                                      */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    SelfGetWriteBufferLength determines how many more characters can      */
/*  be sent to the output self buffer before being full.                    */
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
/*    T_word16                    -- Number of characters until full.       */
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
/*    LES  01/19/94  Created                                                */
/*                                                                          */
/****************************************************************************/

static T_word16 SelfGetWriteBufferLength(T_void)
{
    T_word16 len ;

//    DebugRoutine("SelfGetWriteBufferLength") ;

//    len = SELF_COMM_BUFFER_SIZE - G_selfClientComm->count ;
    len = G_selfClientComm->count ;

//    DebugEnd() ;

    return len ;
}

/****************************************************************************/
/*  Routine:  SelfClientReadByte                                            */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    SelfClientReadByte is used by the server to read data from the buffer */
/*  that receives information from the SelfClientWriteByte calls.           */
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
/*    T_word16                    -- Character or 0xFFFF for none.          */
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
/*    LES  01/19/94  Created                                                */
/*                                                                          */
/****************************************************************************/

static T_word16 SelfClientReadByte(T_void)
{
    T_word16 c ;

//    DebugRoutine("SelfClientReadByte") ;
    /* See if we have a character to read. */
    if (G_selfClientComm->count != 0)  {
        /* Yes, a character is in the input buffer .. get it. */
        c = G_selfClientComm->readBuffer[G_selfClientComm->readOut++] ;
        G_selfClientComm->readOut &= SELF_COMM_BUFFER_SIZE-1 ;
        G_selfClientComm->count-- ;
    } else {
        /* No input character.  Return nothing. */
        c = 0xFFFF ;
    }

//    DebugEnd() ;
    return c ;
}

/****************************************************************************/
/*  Routine:  SelfClientWriteByte                                           */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    SelfClientWriteByte sends a byte out to the current self server.      */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_byte8 c                   -- Character to send                      */
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
/*    LES  01/19/94  Created                                                */
/*                                                                          */
/****************************************************************************/

static T_void SelfClientWriteByte(T_byte8 c)
{

//    DebugRoutine("SelfClientWriteByte") ;
    /* We'll make this one simple, just write it out. */
//    DebugCheck(G_selfClientComm != NULL) ;

    if (G_selfComm->count != SELF_COMM_BUFFER_SIZE)  {
        G_selfComm->readBuffer[G_selfComm->readIn++] = c ;
        G_selfComm->readIn &= SELF_COMM_BUFFER_SIZE-1 ;
        G_selfComm->count++ ;
    }
//    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  SelfClientGetReadBufferLength                                 */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    SelfClientGetReadBufferLength determines how many characters are      */
/*  waiting in the self read buffer.                                        */
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
/*    T_word16                    -- Number of characters waiting.          */
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
/*    LES  01/19/94  Created                                                */
/*                                                                          */
/****************************************************************************/

static T_word16 SelfClientGetReadBufferLength(T_void)
{
    T_word16 len ;

//    DebugRoutine("SelfClientGetReadBufferLength") ;

    len = G_selfClientComm->count ;

//    DebugEnd() ;

    return len ;
}

/****************************************************************************/
/*  Routine:  SelfClientGetWriteBufferLength                                */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    SelfClientGetWriteBufferLength determines how many more characters can*/
/*  be sent to the output self buffer before being full.                    */
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
/*    T_word16                    -- Number of characters until full.       */
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
/*    LES  01/19/94  Created                                                */
/*                                                                          */
/****************************************************************************/

static T_word16 SelfClientGetWriteBufferLength(T_void)
{
    T_word16 len ;

//    DebugRoutine("SelfGetWriteBufferLength") ;

//    len = SELF_COMM_BUFFER_SIZE - G_selfComm->count ;
    len = G_selfComm->count ;

//    DebugEnd() ;

    return len ;
}

/****************************************************************************/
/*  Routine:  CommGetBaudRate                                               */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    CommGetBaudRate tells what the active port's baud rate is.            */
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
/*    E_commBaudRate              -- Enumerated value of baud rate.         */
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
/*                                                                          */
/****************************************************************************/

E_commBaudRate CommGetBaudRate(T_void)
{
    E_commBaudRate baud ;

    DebugRoutine("CommGetBaudRate") ;
    DebugCheck(G_currentPort != COMM_PORT_BAD) ;

    baud = G_currentPort->baud ;

    DebugCheck(baud < BAUD_RATE_UNKNOWN) ;
    DebugEnd() ;

    return baud ;
}

/****************************************************************************/
/*  Routine:  CommConvertBaudTo32                                           */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    CommConvertBaudTo32 converts given enumerated baud type to a 32 bit   */
/*  integer.                                                                */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    E_commBaudRate baud         -- Rate of baud as enumeration            */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_word32                    -- Rate of baud as 32 bit integer         */
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
/*                                                                          */
/****************************************************************************/

T_word32 CommConvertBaudTo32(E_commBaudRate baud)
{
    T_word32 baud32 ;

    DebugRoutine("CommConvertBaudTo32") ;
    DebugCheck(baud < BAUD_RATE_UNKNOWN) ;

    baud32 = baudLookUp[baud] ;

    DebugEnd() ;

    return baud32 ;
}

/****************************************************************************/
/*  Routine:  CommGetPortType                                               */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    Tells what type of port is attached (modem, null modem, server, ...)  */
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
/*    E_commType                  -- Comm type of active port.              */
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
/*    LES  01/25/95  Created                                                */
/*                                                                          */
/****************************************************************************/

E_commType CommGetPortType()
{
    E_commType type ;

    DebugRoutine("ComGetPortType") ;
    DebugCheck(G_currentPort != COMM_PORT_BAD) ;

    type = G_currentPort->type ;

    DebugCheck(type < COMM_TYPE_UNKNOWN) ;
    DebugEnd() ;

    return type ;
}

/****************************************************************************/
/*  Routine:  CommClearPort                                                 */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    Clears out the incoming data.                                         */
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
/*    Nothing                                                               */
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
/*    LES  01/25/95  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void CommClearPort()
{
    DebugRoutine("CommClearPort") ;

    while(CommGetReadBufferLength())  {
        CommReadByte() ;
    }

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  CommCheckClientAndServerExist                                 */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    This routine checks to see if this is a machine that has both a       */
/*  client and a server on the same machine/process.                        */
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
/*    E_Boolean                                                             */
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
/*    LES  01/25/95  Created                                                */
/*                                                                          */
/****************************************************************************/

E_Boolean CommCheckClientAndServerExist(T_void)
{
    E_Boolean status ;
    DebugRoutine("CommCheckClientAndServerExist") ;

    status = G_clientAndServerExist ;

    DebugCheck(status < BOOLEAN_UNKNOWN) ;
    DebugEnd() ;

    return status ;
}

#ifndef SERVER_ONLY
/* LES: 05/13/96 -- Added routine to tell if there is a server on */
/* this communications network computer. */
E_Boolean CommIsServer(T_void)
{
    return G_isServer ;
}
#endif

T_word16 CommGetLinkSubType(T_void)
{
    return G_subtype ;
}

/****************************************************************************/
/*    END OF FILE:  COMM.C                                                  */
/****************************************************************************/
