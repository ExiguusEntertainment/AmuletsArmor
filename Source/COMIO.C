/****************************************************************************/
/*    FILE:  COMIO.C                                                        */
/****************************************************************************/

#include "standard.h"

/* Definition of a port's buffer for either receiveing or writing buffering. */
typedef struct {
    T_word16        numBytes ;    /* Number of bytes current in buffer. */
    T_word16        sendPos ;    /* Position to send new bytes. */
    T_word16        receivePos ;     /* Position to receive bytes. */
    T_COMPort       owner ;       /* What port owns this buffer. */
    T_byte8         *p_data ;     /* Pointer to data to use as buffer. */
} T_PortBuffer ;


/* Structure to define what one communications port is and it's relationship
   to other structures. */
typedef struct {
    T_word16        ioAddress ;   /* I/O Address for this port */
    T_byte8         ioInterrupt ; /* Interrupt level of port. */
    E_COMPortType   typePort ;    /* Type of modem/port */
    T_word16        flags ;       /* Flags attached to this port */
    T_word16        nextPort ;    /* Link to next port with same interrupt */
                                  /* Value of 0xFFFF means no next port */
    T_PortBuffer    receiveBuffer ;  /* Receive buffer */
    T_PortBuffer    sendBuffer;  /* Send buffer */
} T_PortInfo ;

/* Define a easy to use pointer to interrupt type. */
#if defined(WATCOM)
typedef void (__interrupt __far *TP_Interrupt)() ;
#elif defined(WIN32)
typedef void (void *TP_Interrupt)(void) ;
#endif

#define COM_PORT_IN_USE        0x8000
#define COM_PORT_FLAGS_ENABLED 0x4000

#define COM_END_LIST           0xFFFF

/* 8250 Registers Offsets: */
#define COM_REG_TRANSMIT            0
#define COM_REG_RECEIVE             0
#define COM_REG_BAUD_DIV_LOW_BYTE   0
#define COM_REG_INTERRUPT_ENABLE    1
#define COM_REG_BAUD_DIV_HIGH_BYTE  1
#define COM_REG_INTERRUPT_ID        2
#define COM_REG_LINE_CONTROL        3
#define COM_REG_MODEM_CONTROL       4
#define COM_REG_LINE_STATUS         5
#define COM_REG_MODEM_STATUS        6

/* Programmable Intterupt Controller (PIC) Mask Register */
#define PIC_MASK                    0x21

#define PIC_CONTROL_REG             0x20
#define END_OF_INTERRUPT            0x20

/* Internal prototypes: */
T_void ActivateInterrupt(T_byte8 ioInterrupt, T_word16 ioAddress) ;
T_void DeactivateInterrupt(T_byte8 ioInterrupt, T_word16 ioAddress) ;
T_void HandleInterrupt(T_byte8 ioInterrupt) ;
#if defined(WATCOM)
void __interrupt __far COMIO_Interrupt0() ;
void __interrupt __far COMIO_Interrupt1() ;
void __interrupt __far COMIO_Interrupt2() ;
void __interrupt __far COMIO_Interrupt3() ;
void __interrupt __far COMIO_Interrupt4() ;
void __interrupt __far COMIO_Interrupt5() ;
void __interrupt __far COMIO_Interrupt6() ;
void __interrupt __far COMIO_Interrupt7() ;
#endif

/* List of where interrupts start, a value of 0xFFFF represents none. */
static T_word16 G_InterruptPortStart[COM_NUMBER_INTERRUPTS] ;

/* Number of items on the interrupt list. */
static T_word16 G_InterruptListCount[COM_NUMBER_INTERRUPTS] ;

/* Number of items on the interrupt list that is enabled. */
static T_word16 G_InterruptNumEnabled[COM_NUMBER_INTERRUPTS] ;

/* Old Interrupt locations */
static TP_Interrupt G_Interrupts[COM_NUMBER_INTERRUPTS] ;

/* Array of ports that we can use. */
static T_PortInfo G_Port[COM_MAX_PORTS] ;

/* First free port. */
static T_COMPort G_FirstPort = 0 ;

/* Flag to tell if we are initialized or not. */
static E_Boolean F_Initialized = FALSE ;

#if defined(WATCOM)
/* Here is a reference list of the interrupt functions: */
static TP_Interrupt G_ListInterrupts[8] = {
    COMIO_Interrupt0,
    COMIO_Interrupt1,
    COMIO_Interrupt2,
    COMIO_Interrupt3,
    COMIO_Interrupt4,
    COMIO_Interrupt5,
    COMIO_Interrupt6,
    COMIO_Interrupt7
} ;
#else
#endif


/****************************************************************************/
/*  Routine:  COMIO_Initialize                                              */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*      Initialize all the important global variables used by the           */
/*    COMIO package.                                                        */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*      I don't think there are any.                                        */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*      None.                                                               */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*      Nothing.                                                            */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*      Nothing.                                                            */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    me   dd/mm/yy  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void COMIO_Initialize(T_void)
{
    T_word16 i ;

    DebugRoutine("COMIO_Initialize") ;
    DebugCheck(F_Initialized==FALSE) ;

    /* Set up the free list which happens to be all the ports. */
    for (i=0; i<COM_MAX_PORTS; i++)
        G_Port[i].nextPort = i+1 ;

    /* Make the last item point to no one. */
    G_Port[COM_MAX_PORTS-1].nextPort = COM_END_LIST ;

    /* First free is setup to first item in array. */
    G_FirstPort = 0 ;

    /* Note that we are now initialized and we should not have to
       do it again. */
    F_Initialized = TRUE ;

    /* Initialize the interrupt variables. */
    for (i=0; i<COM_NUMBER_INTERRUPTS; i++)  {
        /* None of the interrupts are active. */
        G_InterruptNumEnabled[i] = 0 ;

        /* The lists contain nothing in them. */
        G_InterruptListCount[i] = 0 ;

        /* The first item on each list is nothing. */
        G_InterruptPortStart[i] = COM_END_LIST ;
    }
    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  COMIO_Open                                                    */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    COMIO_Open is the routine that opens up a communication port on the   */
/*  PC computer given the PC I/O address (e.g. 0x03E8) and the interrupt    */
/*  tied to that port (e.g. interrupt level 3).  A communications port      */
/*  identifier used by the rest of these routines is returned.              */
/*    Also, the returned port is in disable mode and needs a call to        */
/*  COMIO_EnablePort to active it.                                          */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    COMIO_Open does not check if the given address and interrupt is       */
/*  valid (except for the allowable ranges).  Should a invalid numbers      */
/*  be used, no error may be returned.                                      */
/*    Also, COMIO_Open does not currently check to see if it is has already */
/*  created a port with the same address and interrupt.                     */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_word16 io_address    -- I/O address for port to allocate            */
/*                                                                          */
/*    T_byte8  io_interrupt  -- Interrupt level for port                    */
/*                                                                          */
/*    Assumptions:                                                          */
/*        io_address is in the range of 0x200 - 0x400                       */
/*        io_interrupt is in the range of 0-7                               */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_COMPort              -- Returns a com port identifier               */
/*                                                                          */
/*    Assumptions:                                                          */
/*        T_COMPort is a value of 0-(COM_MAX_PORTS-1)                       */
/*        Also, the initial port has not been enabled.                      */
/*                                                                          */
/*  Calls:                                                                  */
/*    COMIO_ClearReceiveBuffer                                                 */
/*    COMIO_ClearSendBuffer                                                */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  11/12/94  Created                                                */
/*                                                                          */
/****************************************************************************/

T_COMPort COMIO_Open(T_word16 io_address, T_byte8 io_interrupt)
{
    T_COMPort port ;
    T_PortInfo *p_port ;

    /* COMIO_Open needs to do the following actions:
       1. Allocate a port or return with an error.
       2. Allocate a pair of buffers for the port and initialize.
       3. Initialize the port (as disabled)
       4. Place port on interrupt list.
       5. Return the newly created port. */

    DebugRoutine("COMIO_Open") ;
    DebugCheck(F_Initialized) ;
    DebugCheck((io_address >= 0x200) && (io_address <= 0x400)) ;
    DebugCheck(io_interrupt < COM_NUMBER_INTERRUPTS) ;


    /* 1. Allocate a port by taking it off the free list of ports. */
    port = G_FirstPort ;
    p_port = &G_Port[port] ;
    G_FirstPort = p_port->nextPort ;

    /* 2. Allocate a pair of buffers for the port. */
    DebugCheck(p_port->receiveBuffer.p_data == NULL) ;
    DebugCheck(p_port->sendBuffer.p_data == NULL) ;

    /* 2.1 Allocate the buffers. */
    p_port->receiveBuffer.p_data = MemAlloc(COM_BUFFER_SIZE) ;
    p_port->sendBuffer.p_data = MemAlloc(COM_BUFFER_SIZE) ;

    /* 2.2 Check if we allocated the buffers. */
    /* -- For now, produce a fatal error if a buffer cannot be created. */
    DebugCheck(p_port->receiveBuffer.p_data != NULL) ;
    DebugCheck(p_port->sendBuffer.p_data != NULL) ;

    /* 2.3 Declare the owning port. */
    p_port->receiveBuffer.owner =
        p_port->sendBuffer.owner = port ;

    /* 3. Initialize the port data (not the port itself) */
    p_port->ioAddress = io_address ;
    p_port->ioInterrupt = io_interrupt ;
    p_port->typePort = COM_PORT_TYPE_8250 ;
    p_port->flags = COM_PORT_IN_USE ;

    /* 3.1 Clear the buffers. */
    COMIO_ClearReceiveBuffer(port) ;
    COMIO_ClearSendBuffer(port) ;

    /* 4. Place port on the interrupt list. */
    /* 4.1 Turn off interrupts. */
    _disable() ;

    /* 4.2 Change the interrupt list and increment the count by one. */
    p_port->nextPort = G_InterruptPortStart[io_interrupt] ;
    G_InterruptPortStart[io_interrupt] = port ;
    G_InterruptListCount[io_interrupt]++ ;

    /* 4.3 turn interrupts back on. */
    _enable() ;

    DebugCheck(port < COM_MAX_PORTS) ;
    DebugEnd() ;

    return port ;
}

/****************************************************************************/
/*  Routine:  COMIO_Close                                                   */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*      COMIO_Close does the shutting down of a port.  It will make sure    */
/*   that the interrupt list is updated and buffers in memory are removed.  */
/*   You should make a call to COMIO_DisablePort before you call this       */
/*   routine.                                                               */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*      This routine assumes you are done with the port, but it does not    */
/*    actually stop the interrupts from occuring.                           */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*      T_COMPort port            -- Port needing to be closed.             */
/*                                                                          */
/*      Assumptions:                                                        */
/*          The port must be in use to be closed (as well as valid).        */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*      None.                                                               */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*      Nothing.                                                            */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  11/12/94  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void COMIO_Close(T_COMPort port)
{
    T_PortInfo *p_port ;
    T_COMPort search ;
    T_COMPort last = COM_END_LIST;

    DebugRoutine("COMIO_Close") ;
    DebugCheck(port < COM_MAX_PORTS) ;
    DebugCheck((G_Port[port].flags & COM_PORT_IN_USE) != 0) ;

    /* All that has to be done is
       1. Remove the port from the interrupt list,
       2. Deallocate the memory used by the buffers, and
       3. Put the free port on the free list. */

    /* 1. Remove the port from the interrupt list. */
    p_port = &G_Port[port] ;
    search = G_InterruptPortStart[p_port->ioInterrupt] ;
    while ((search != COM_END_LIST) && (search != port))  {
        last = search ;
        search = G_Port[search].nextPort ;
    }

    /* There should be a port found, if there isn't, produce
       a fatal error. */
    DebugCheck(search != COM_END_LIST) ;

    _disable() ;
    G_Port[last].nextPort = G_Port[port].nextPort ;
    _enable() ;

    /* 2. Deallocate the buffer memory. */
    MemFree(G_Port[port].receiveBuffer.p_data) ;
    MemFree(G_Port[port].sendBuffer.p_data) ;

    /* 3. Put the free port on the free list. */
    G_Port[port].nextPort = G_FirstPort ;
    G_FirstPort = port ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  COMIO_SetBaudRate                                             */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    Before a port can be used effectively, it needs the correct baud      */
/*  rate to be set.  COMIO_SetBaudRate sets the baud to one of the          */
/*  specified enumerated baud rates.                                        */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_COMPort port              -- Valid port to change baud rate         */
/*                                                                          */
/*    E_COMBaudRate baud          -- Baud rate to go to.                    */
/*                                                                          */
/*    Assumptions:                                                          */
/*        baud ranges from 300 to 38400 (any bigger and we can't handle it) */
/*        port is a valid port                                              */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    inp                                                                   */
/*    outp                                                                  */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  11/12/94  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void COMIO_SetBaudRate(T_COMPort port, T_COMBaudRate baud)
{
    T_word16 divider ;
    T_byte8 lowByte ;
    T_byte8 highByte ;
    T_word16 ioAddress ;

    DebugRoutine("COMIO_SetBaudRate") ;
    DebugCheck(port < COM_MAX_PORTS) ;
    DebugCheck((baud >= (T_COMBaudRate)300) &&
               (baud <= (T_COMBaudRate)38400)) ;

    /* Get the ioAddress of the port. */
    ioAddress = G_Port[port].ioAddress ;

    /* Calculate the clock divider */
    divider = ((T_word32)115200) / baud ;

    /* Get the high and low byte of the resulting 16 bit value. */
    lowByte = divider & 0xFF ;
    highByte = (divider >> 8) & 0xFF ;

    /* Knock off any interrupts we might be getting. */
    _disable() ;

    /* Set the bit that let's us access the baud divisor registers. */
    outp(ioAddress+COM_REG_LINE_CONTROL,
        inp(ioAddress+COM_REG_LINE_CONTROL) | 0x80) ;

printf("Old low: %02X\n", inp(ioAddress+COM_REG_BAUD_DIV_LOW_BYTE)) ;
printf("Old high: %02X\n", inp(ioAddress+COM_REG_BAUD_DIV_HIGH_BYTE)) ;
    /* Set the baud rate */
    outp(ioAddress+COM_REG_BAUD_DIV_LOW_BYTE, lowByte) ;
    outp(ioAddress+COM_REG_BAUD_DIV_HIGH_BYTE, highByte) ;
printf("New low: %02X\n", inp(ioAddress+COM_REG_BAUD_DIV_LOW_BYTE)) ;
printf("New high: %02X\n", inp(ioAddress+COM_REG_BAUD_DIV_HIGH_BYTE)) ;

    /* Reset the bit that let's us access the baud divisor registers. */
    outp(ioAddress+COM_REG_LINE_CONTROL,
        inp(ioAddress+COM_REG_LINE_CONTROL) & 0x7F) ;

    /* Turn back on the interrupts. */
    _enable() ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  COMIO_SetTypePort                                            */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    Set the type of modem that we will be using.  Currently only a        */
/*  couple types are used.  The default modem is the 8250.  This routine    */
/*  should only be called before the modem is enabled.                      */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_COMPort port              -- Port to change type of                 */
/*                                                                          */
/*    E_COMPortType type          -- Type to change port to                 */
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
/*    LES  11/14/94  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void COMIO_SetTypePort(T_COMPort port, E_COMPortType type)
{
    DebugRoutine("COMIO_SetTypePort") ;
    DebugCheck(port < COM_MAX_PORTS) ;
    DebugCheck(type < COM_PORT_TYPE_UNKNOWN) ;

    /* Store the type.  In this software, anybody who needs to know the
       type will check the type as they are processing. */
    G_Port[port].typePort = type ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  COMIO_SetControl                                              */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    COMIO_SetControl sets the bit length, stop bits, and parity of the    */
/*  port given.                                                             */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    This had not be called while the port is active.  Unpredictable       */
/*  results will occur.                                                     */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*                                                                          */
/*    T_COMPort port              -- Port to change control                 */
/*                                                                          */
/*    E_COMBitLength bit_length   -- Number of bits per transfer            */
/*                                                                          */
/*    E_COMStopBit                -- Number of stop bits at end             */
/*                                                                          */
/*    E_COMParity                 -- Type of parity checking (if any)       */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    Nothing.                                                              */
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
/*    LES  11/14/94  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void COMIO_SetControl(
               T_COMPort port,
               E_COMBitLength bit_length,
               E_COMStopBit stop_bit,
               E_COMParity parity)
{
    static f_bit_length[] = {
               0x00,   /* ------00   COM_BIT_LENGTH_5 */
               0x01,   /* ------01   COM_BIT_LENGTH_6 */
               0x02,   /* ------10   COM_BIT_LENGTH_7 */
               0x03    /* ------11   COM_BIT_LENGTH_8 */
    } ;
    static f_stop_bit[] = {
               0x00,   /* -----0--   COM_STOP_BIT_1         */
               0x04    /* -----1--   COM_STOP_BIT_1_POINT_5 */
    } ;
    static f_parity[] = {
               0x00,   /* --000---   COM_PARITY_NONE        */
               0x20,   /* --100---   COM_PARITY_ODD         */
               0x30,   /* --110---   COM_PARITY_EVEN        */
               0x28,   /* --101---   COM_PARITY_MARK        */
               0x38,   /* --111---   COM_PARITY_SPACE       */
    } ;
    T_byte8 control ;
    T_word16 ioAddress ;

    DebugRoutine("COMIO_SetControl") ;
    DebugCheck(port < COM_MAX_PORTS) ;
    DebugCheck(bit_length < COM_BIT_LENGTH_UNKNOWN) ;
    DebugCheck(stop_bit < COM_STOP_BIT_UNKNOWN) ;
    DebugCheck(parity < COM_PARITY_UNKNOWN) ;

    /* Find this port's line control register address. */
    ioAddress = G_Port[port].ioAddress + COM_REG_LINE_CONTROL ;

    /* Read in the old control and mask out all except the break enable
       and alternative use of baud/data ports. */
    control = inp(ioAddress) & 0x40 ;

    /* Output the changed control by or'ing all our flag values
       to form a single control byte. */
    outp(ioAddress,
         control |
         f_bit_length[bit_length] |
         f_stop_bit[stop_bit] |
         f_parity[parity]) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  COMIO_CheckCarrier                                            */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    You can check for if a modem is connected by checking the carrier     */
/*  signal of the modem.  This routine will return TRUE if the given        */
/*  port has a carrier signal, or else it will return FALSE.                */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_COMPort port              -- Valid communications port.             */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    E_Boolean                   -- TRUE = Carrier found                   */
/*                                   FALSE = Carrier not found              */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    Nobody.                                                               */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  11/14/94  Created                                                */
/*                                                                          */
/****************************************************************************/

E_Boolean COMIO_CheckCarrier(T_COMPort port)
{
    E_Boolean status ;
    T_word16 ioAddress ;
    T_byte8 statusReg ;

    DebugRoutine("COMIO_CheckCarrier") ;
    DebugCheck(port < COM_MAX_PORTS) ;

    /* Get the io address to the Modem Status Registers (MSR) */
    ioAddress = G_Port[port].ioAddress + COM_REG_MODEM_STATUS ;

    /* Read in that status. */
    statusReg = inp(ioAddress) ;

    /* Look at the Data carrier detect (DCD) bit. */
    if ((statusReg & 0x80)==0x00)  {
        status = FALSE ;
    } else {
        status = TRUE ;
    }

    DebugCheck(status < BOOLEAN_UNKNOWN) ;
    DebugEnd() ;

    /* Return the status. */
    return status ;
}

/****************************************************************************/
/*  Routine:  COMIO_EnablePort                                              */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*      Turn on a port that has already been opened (and perhaps previously */
/*    was disabled).  Enabling a port allows it to send and receive data.   */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    Hmmm ... let me think about that one.                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_COMPort port              -- Port to enable                         */
/*                                                                          */
/*    Assumptions:                                                          */
/*        Would it be too much to assume that the port is valid?  Probably  */
/*        so.                                                               */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    ActivateInterrupt                                                     */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  11/12/94  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void COMIO_EnablePort(T_COMPort port)
{
    T_word16 ioInterrupt ;

    DebugRoutine("EnablePort") ;
    DebugCheck(port < COM_MAX_PORTS) ;
    DebugCheck((G_Port[port].flags & COM_PORT_FLAGS_ENABLED) == 0) ;

    /* Make the port enabled. */
    G_Port[port].flags |= COM_PORT_FLAGS_ENABLED ;

    /* Get the IO Interrupt level of the port. */
    ioInterrupt = G_Port[port].ioInterrupt ;

    /* Increment the number of enabled ports for this interrupt, but
       check if it was a zero.  */
    if ((G_InterruptNumEnabled[ioInterrupt]++) == 0)
        /* If it *was* zero, then we need to turn on the interrupt. */
        ActivateInterrupt(ioInterrupt, G_Port[port].ioAddress) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  COMIO_DisablePort                                             */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*      Turn on a port that has already been opened (and perhaps previously */
/*    was disabled).  Enabling a port allows it to send and receive data.   */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    I'm not really sure if there are any problems.                        */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_COMPort port              -- Port to enable                         */
/*                                                                          */
/*    Assumptions:                                                          */
/*        The port I'm getting should be a valid number.                    */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    DeactivateInterrupt                                                   */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  11/12/94  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void COMIO_DisablePort(T_COMPort port)
{
    T_word16 ioInterrupt ;

    DebugRoutine("DisablePort") ;
    DebugCheck(port < COM_MAX_PORTS) ;
    DebugCheck((G_Port[port].flags & COM_PORT_FLAGS_ENABLED) != 0) ;

    /* Make the port disabled. */
    G_Port[port].flags &= (~COM_PORT_FLAGS_ENABLED) ;

    /* Get the IO Interrupt level of the port. */
    ioInterrupt = G_Port[port].ioInterrupt ;

    /* Decrement the number of enabled ports for this interrupt, then
       check if it is a zero.  */
    if ((--(G_InterruptNumEnabled[ioInterrupt])) == 0)
        /* If it is zero, then we need to turn off the interrupt. */
        DeactivateInterrupt(ioInterrupt, G_Port[port].ioAddress) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  COMIO_GetEnableStatus                                         */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    Should you forget, you may wish to sometimes find out if a port       */
/*  is active or inactive.  A call to this routine will return a TRUE       */
/*  if the port is active or a FALSE if it is inactive.                     */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_COMPort port              -- Port to check enable status            */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    E_Boolean                   -- TRUE = port is enabled                 */
/*                                   FALSE = port is disabled               */
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
/*    LES  11/12/94  Created                                                */
/*                                                                          */
/****************************************************************************/

E_Boolean COMIO_GetEnableStatus(T_COMPort port)
{
    E_Boolean result ;

    DebugRoutine("DisablePort") ;
    DebugCheck(port < COM_MAX_PORTS) ;

    if ((G_Port[port].flags & COM_PORT_FLAGS_ENABLED) != 0)  {
        result = TRUE ;
    } else {
        result = FALSE ;
    }

    DebugEnd() ;

    return result ;
}


/****************************************************************************/
/*  Routine:  COMIO_ReceiveByte                                             */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    When you need one byte from a port, call this routine.  It will       */
/*  check to see if there is a byte in the buffer, and if there is, will    */
/*  return it.  If there is not, it will return with COM_ERROR_EMPTY_BUFFER.*/
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    Note that this routine does not sit and wait for a byte to appear     */
/*  in the read buffer.  Therefore, the calling routine must always take    */
/*  the responsibility of considering a time out condition.                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_COMPort port              -- Port to read from receive buffer.      */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_word16                    -- if 0-255, byte read                    */
/*                                -- otherwise, COM_ERROR_EMPTY_BUFFER      */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    No one.                                                               */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  11/14/94  Created                                                */
/*                                                                          */
/****************************************************************************/

T_word16 COMIO_ReceiveByte(T_COMPort port)
{
    T_PortBuffer *p_buffer ;
    T_word16 value ;

    DebugRoutine("COMIO_ReceiveByte") ;
    DebugCheck(port < COM_MAX_PORTS) ;

    /* Get a pointer to the port's buffer for easy referencing. */
    p_buffer = &G_Port[port].receiveBuffer ;

    /* Check if the buffer is empty. */
    if (p_buffer->numBytes == 0)  {
        /* If the buffer is empty, return a empty buffer error. */
        value = COM_ERROR_EMPTY_BUFFER ;
    } else {
        /* If the buffer is not empty,
           turn off interrupts for a second (so we can change the number
           of bytes without interruption. */
        _disable() ;

        /* decrement the number of available bytes, */
        p_buffer->numBytes-- ;

        /* Turn interrupts back on. */
        _enable() ;

        /* Get the byte from the buffer, */
        value = p_buffer->p_data[p_buffer->receivePos++] ;

        /* And update the buffer position. */
        if (p_buffer->receivePos == COM_BUFFER_SIZE)
            p_buffer->receivePos = 0 ;
    }
    DebugCheck((value < 256) || (value == COM_ERROR_EMPTY_BUFFER)) ;
    DebugEnd() ;

    return value ;
}

/****************************************************************************/
/*  Routine:  COMIO_GetReceiveCount                                         */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    To help determine if you want to read from the receive buffer,        */
/*  you can use this function to tell you how many bytes are there.  If     */
/*  it is zero, then the buffer is empty.                                   */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.  Just make sure we keep those _disable and _enable lines.       */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_COMPort port              -- Port to check receive buffer           */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_word16                    -- Number of bytes in buffer              */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    _disable                                                              */
/*    _enable                                                               */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  11/12/94  Created                                                */
/*                                                                          */
/****************************************************************************/

T_word16 COMIO_GetReceiveCount(T_COMPort port)
{
    T_word16 count ;

    DebugRoutine("COMIO_GetReceiveCount") ;
    DebugCheck(port < COM_MAX_PORTS) ;

    /* Look up the number of bytes in the receive buffer when nothing
       is affecting it. */
    _disable() ;
    count = G_Port[port].receiveBuffer.numBytes ;
    _enable() ;

    DebugCheck(count <= COM_BUFFER_SIZE) ;
    DebugEnd() ;

    return count ;
}

/****************************************************************************/
/*  Routine:                                                                */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*                                                                          */
/*                                                                          */
/*                                                                          */
/*                                                                          */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*                                                                          */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*                                                                          */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    me   dd/mm/yy  Created                                                */
/*                                                                          */
/****************************************************************************/

T_word16 COMIO_ReceiveData(T_COMPort port, T_word16 count, T_byte8 *buffer) ;

/****************************************************************************/
/*  Routine:  COMIO_SendByte                                                */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    COMIO_SendByte is the basic routine to send one byte over the com     */
/*  port that you give.  If the buffer is full, you will get an error       */
/*  of COM_ERROR_FULL_BUFFER.                                               */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    If the buffer is full when you try to send it, the routine does       */
/*  not wait until it is empty.  It is up to the calling routine to consider*/
/*  this case.                                                              */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_COMPort port              -- Port to send byte out through          */
/*                                                                          */
/*    T_byte8 data                -- character to send                      */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_word16                    -- Returns either COM_ERROR_NO_ERROR      */
/*                                   or COM_ERROR_EMPTY_BUFFER              */
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
/*    LES  11/14/94  Created                                                */
/*                                                                          */
/****************************************************************************/

T_word16 COMIO_SendByte(T_COMPort port, T_byte8 data)
{
    T_PortBuffer *p_buffer ;
    T_word16 value ;
    T_word16 ioAddress ;

    DebugRoutine("COMIO_SendByte") ;
    DebugCheck(port < COM_MAX_PORTS) ;

    /* Get a pointer to the port's buffer for easy referencing. */
    p_buffer = &G_Port[port].sendBuffer ;

    /* Check if the buffer is full. */
    if (p_buffer->numBytes == COM_BUFFER_SIZE)  {
        /* If the buffer is full, return a full buffer error. */
        value = COM_ERROR_FULL_BUFFER ;
    } else {
        /* If the buffer is not full, */
        /* Add the byte to the buffer, */
        p_buffer->p_data[p_buffer->sendPos++] = data ;

        /* And update the buffer position. */
        if (p_buffer->sendPos == COM_BUFFER_SIZE)
            p_buffer->sendPos = 0 ;

        /* Finally, incrment the number of available bytes (while interrupts
           are turned off. */
        _disable() ;
        p_buffer->numBytes++ ;
        _enable() ;

        /* Now we need to turn on the interrupt to send the byte (if
           it's not already on.) */
        /* Get the address of the interrupt enable register. */
        ioAddress = G_Port[port].ioAddress + COM_REG_INTERRUPT_ENABLE ;

        /* Flip on the transmit hold bit. */
        outp(ioAddress, inp(ioAddress)|0x02) ;

        /* Return saying we had no errors. */
        value = COM_ERROR_NO_ERROR ;
    }
    DebugCheck((value == COM_ERROR_NO_ERROR) ||
               (value == COM_ERROR_FULL_BUFFER)) ;
    DebugEnd() ;

    return value ;
}

/****************************************************************************/
/*  Routine:  COMIO_GetSendCount                                            */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    GetSendCount    is mainly to see how full the input  buffer is        */
/*  for a single port.  Use this routine to determine how many bytes are    */
/*  still needing to be sent out by the port.                               */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.  Just make sure we keep those _disable and _enable lines.       */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_COMPort port              -- Port to check send buffer              */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_word16                    -- Number of bytes in buffer              */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    _disable                                                              */
/*    _enable                                                               */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  11/12/94  Created                                                */
/*                                                                          */
/****************************************************************************/

T_word16 COMIO_GetSendCount(T_COMPort port)
{
    T_word16 count ;

    DebugRoutine("COMIO_GetSendCount") ;
    DebugCheck(port < COM_MAX_PORTS) ;

    /* Look up the number of bytes in the receive buffer when nothing
       is affecting it. */
    _disable() ;
    count = G_Port[port].sendBuffer.numBytes ;
    _enable() ;

    DebugCheck(count <= COM_BUFFER_SIZE) ;
    DebugEnd() ;

    return count ;
}

/****************************************************************************/
/*  Routine:                                                                */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*                                                                          */
/*                                                                          */
/*                                                                          */
/*                                                                          */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*                                                                          */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*                                                                          */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    me   dd/mm/yy  Created                                                */
/*                                                                          */
/****************************************************************************/

T_word16 COMIO_SendData(T_COMPort port, T_word16 count, T_byte8 *buffer) ;

/****************************************************************************/
/*  Routine:  COMIO_ClearReceiveBuffer                                         */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*      COMIO_ClearReceiveBuffer initializes the read buffer of a port to be   */
/*    that of an empty buffer.  It does not actually zero out the buffer,   */
/*    but all indexes are zeroed.                                           */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*      Using this routine should only be done when the port is not         */
/*    enabled.  If it is, no telling what will happen.  I've included       */
/*    a DebugCheck to make sure this is not the case.                       */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*      T_COMPort port            -- Port to have its receive buffer zeroed.   */
/*                                                                          */
/*      Assumptions:                                                        */
/*          The port must be in use and disabled.                           */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*      Nothing.                                                            */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*      Nothing.                                                            */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  11/12/94  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void COMIO_ClearReceiveBuffer(T_COMPort port)
{
    T_PortBuffer *p_buffer ;

    DebugRoutine("COMIO_ClearReceiveBuffer") ;
    DebugCheck(port < COM_MAX_PORTS) ;
    DebugCheck((G_Port[port].flags & COM_PORT_IN_USE) != 0) ;
    DebugCheck((G_Port[port].flags & COM_PORT_FLAGS_ENABLED) == 0) ;
    p_buffer = &G_Port[port].receiveBuffer ;

    p_buffer->numBytes = 0 ;
    p_buffer->receivePos = 0 ;
    p_buffer->sendPos = 0 ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  COMIO_ClearSendBuffer                                        */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*      COMIO_ClearSendBuffer initializes the send buffer of a port to be */
/*    that of an empty buffer.  It does not actually zero out the buffer,   */
/*    but all indexes are zeroed.                                           */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*      Using this routine should only be done when the port is not         */
/*    enabled.  If it is, no telling what will happen.  I've included       */
/*    a DebugCheck to make sure this is not the case.                       */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*      T_COMPort port            -- Port to have its receive buffer zeroed.   */
/*                                                                          */
/*      Assumptions:                                                        */
/*          The port must be in use and disabled.                           */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*      Nothing.                                                            */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*      Nothing.                                                            */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  11/12/94  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void COMIO_ClearSendBuffer(T_COMPort port)
{
    T_PortBuffer *p_buffer ;

    DebugRoutine("COMIO_ClearSendBuffer") ;
    DebugCheck(port < COM_MAX_PORTS) ;
    DebugCheck((G_Port[port].flags & COM_PORT_IN_USE) != 0) ;
    DebugCheck((G_Port[port].flags & COM_PORT_FLAGS_ENABLED) == 0) ;
    p_buffer = &G_Port[port].sendBuffer ;

    p_buffer->numBytes = 0 ;
    p_buffer->receivePos = 0 ;
    p_buffer->sendPos = 0 ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  COMIO_Interrupt0 - COMIO_Interrupt7                           */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*      COMIO_InterruptXXX are the 8 interrupts for the different interrupt */
/*    levels that can be received by the PC.  I don't think commenting each */
/*    interrupt would be useful since they all do the same thing.           */
/*      When active, each interrupt calls the interrupt handler with        */
/*    the number of the interrupt level.  When done, it returns by          */
/*    chaining to the standard interrupt handler.                           */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    If several modems/devices are on the same interrupt level, the        */
/*    handler is going to get called and this might slow down operations    */
/*    more than we would like.  However, it will take specialized hardware  */
/*    to do it differently.                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*      None.                                                               */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*      Nothing.                                                            */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*      HandleInterrupt                                                     */
/*      _chain_intr                                                         */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  11/12/94  Created                                                */
/*                                                                          */
/****************************************************************************/

#if defined(WATCOM)
T_void __interrupt __far COMIO_Interrupt0()
{
    HandleInterrupt(0) ;
    _chain_intr(G_Interrupts[0]) ;
}

T_void __interrupt __far COMIO_Interrupt1()
{
    HandleInterrupt(1) ;
    _chain_intr(G_Interrupts[1]) ;
}

T_void __interrupt __far COMIO_Interrupt2()
{
    HandleInterrupt(2) ;
    _chain_intr(G_Interrupts[2]) ;
}

T_void __interrupt __far COMIO_Interrupt3()
{
    HandleInterrupt(3) ;
    _chain_intr(G_Interrupts[3]) ;
}

T_void __interrupt __far COMIO_Interrupt4()
{
    HandleInterrupt(4) ;
    _chain_intr(G_Interrupts[4]) ;
}

T_void __interrupt __far COMIO_Interrupt5()
{
    HandleInterrupt(5) ;
    _chain_intr(G_Interrupts[5]) ;
}

T_void __interrupt __far COMIO_Interrupt6()
{
    HandleInterrupt(6) ;
    _chain_intr(G_Interrupts[6]) ;
}

T_void __interrupt __far COMIO_Interrupt7()
{
    HandleInterrupt(7) ;
    _chain_intr(G_Interrupts[7]) ;
}
#endif

/****************************************************************************/
/*  Routine:  HandleInterrupt                                               */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*      HandleInterrupt is the main routine that does ALL of the byte       */
/*    transactions for input and output of the ports.  When called, it      */
/*    is given an interrupt level that was called.  Using this interrupt    */
/*    level, HandleInterrupt will go through a list of devices and          */
/*    check if those devices are interrupting.  It will do whatever         */
/*    service(s) are being asked for in order of the list.                  */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    If two devices ask for an interrupt simultaneously, the first         */
/*    interrupt will do both, the second interrupt will do neither.         */
/*    But this is not really a fallacy since the alternative is to have     */
/*    two interrupts with one device (out of the two) being served.         */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_byte8 ioInterrupt         -- Level of interrupt to handle           */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    ???                                                                   */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  11/12/94  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void HandleInterrupt(T_byte8 ioInterrupt)
{
    T_COMPort port ;
    T_PortInfo *p_port ;
    T_word16 ioAddress ;
    T_byte8 id, oldid ;
    T_byte8 data ;
    T_PortBuffer *p_buffer ;

/* !!! Debugging/Testing only */
///extern T_word32 hit_count ;
///hit_count++ ;

    /* Get the beginning of the list of ports attached to this interrupt. */
    port = G_InterruptPortStart[ioInterrupt] ;
    /* Loop until we reach the end. */
    while (port != COM_END_LIST)  {
        /* Get a quick pointer to this port. */
        p_port = &G_Port[port] ;

        /* First check if the port is enabled. */
        if ((p_port->flags & COM_PORT_FLAGS_ENABLED) != 0)  {
            /* If it is, we had best get the address. */
            ioAddress = p_port->ioAddress ;

            do {
                /* See if this port has an interrupt on it. */
                id = (oldid = inp(ioAddress+COM_REG_INTERRUPT_ID)) & 0x06 ;
/*
hit_count = id ;
hit_count = inp(ioAddress+COM_REG_LINE_STATUS) ;
hit_count = inp(ioAddress+COM_REG_LINE_STATUS) ;
hit_count = inp(ioAddress+COM_REG_LINE_STATUS) ;
*/
/*
hit_count = inp(ioAddress+COM_REG_LINE_STATUS) ;
*/
                switch(id)  {
                    case 0x02:    /* Data sent out has gone. (Transmit done) */
                        /* Get the pointer to the send (transmit) buffer. */
                        p_buffer = &p_port->sendBuffer ;

                        /* Check if buffer is empty. */
                        if (p_buffer->numBytes == 0)  {
                            /* If the buffer is empty, stop interrupting me! */
                            data = inp(ioAddress+COM_REG_INTERRUPT_ENABLE) ;
                            outp(ioAddress+COM_REG_INTERRUPT_ENABLE, data & 0xFD) ;
                        } else {
                            /* Go ahead and say we have one less byte
                               in case another weird interrupt comes in. */
                            p_buffer->numBytes-- ;

                            /* If not empty, we need to send another byte. */
                            data = p_buffer->p_data[p_buffer->receivePos++] ;

                            /* Check for roll over in the receive position. */
                            if (p_buffer->receivePos == COM_BUFFER_SIZE)
                                p_buffer->receivePos = 0 ;

                            /* Send the byte on its merry way. */
                            outp(ioAddress+COM_REG_TRANSMIT, data) ;
                        }
                        break ;
                    case 0x04:    /* Data is incoming. (Receiving) */
                        /* Get the pointer to the read (receive) buffer. */
                        p_buffer = &p_port->receiveBuffer ;

                        /* If the buffer is full, don't handle the interrupt. */
                        if (p_buffer->numBytes != COM_BUFFER_SIZE)  {
                            /* Get the byte from the port. */
                            data = inp(ioAddress+COM_REG_RECEIVE) ;

                            /* Store in the buffer. */
                            p_buffer->p_data[p_buffer->sendPos++] = data ;

                            /* Check if we rolled over the buffer. */
                            if (p_buffer->sendPos == COM_BUFFER_SIZE)
                                p_buffer->sendPos = 0 ;

                            /* Note that we have another byte available. */
                            p_buffer->numBytes++ ;
                        }
                        break ;
                    default:
                        /* Put edge in interrupt register. */
/*
                        data = inp(ioAddress+COM_REG_INTERRUPT_ENABLE) ;
                        outp(ioAddress+COM_REG_INTERRUPT_ENABLE, 0) ;
                        outp(ioAddress+COM_REG_INTERRUPT_ENABLE, data) ;
*/
                        break ;
                }
                /* Loop while there is another interrupt pending. */
/*
} while (1==0) ;
*/
            } while (/* ((id==0x02) || (id == 0x04)) && */
                     ((oldid /* inp(ioAddress+COM_REG_INTERRUPT_ID)*/ &0x01) != 0)) ;


            /* We're done with this port, move to the next one. */
            port = p_port->nextPort ;
        }
    }
    outp(PIC_CONTROL_REG, END_OF_INTERRUPT) ;
}

/****************************************************************************/
/*  Routine:  ActivateInterrupt                                             */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*      ActivateInterrupt is used to turn on an interrupt that was          */
/*    originally inactive.  This routine MUST NOT be called when an         */
/*    interrupt handler is already active.                                  */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None really.                                                          */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_byte8 ioInterrupt         -- Level of interrupt to activate         */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    _dos_getvect                                                          */
/*    _dos_setvect                                                          */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  11/12/94  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void ActivateInterrupt(T_byte8 ioInterrupt, T_word16 ioAddress)
{
    T_byte8 data ;

    DebugRoutine("ActivateInterrupt") ;

    /* Make sure the interrupt is not already installed. */
    DebugCheck(_dos_getvect(ioInterrupt+8) != G_ListInterrupts[ioInterrupt]) ;
    DebugCheck(ioInterrupt < COM_NUMBER_INTERRUPTS) ;

/* !!! */
printf("Activating interrupt %d\n", ioInterrupt) ;
fflush(stdout) ;

    /* First, store the old interrupt. */
    G_Interrupts[ioInterrupt] = _dos_getvect(ioInterrupt+8) ;

printf("Old: %d is %p\n", ioInterrupt, G_Interrupts[ioInterrupt]) ;
printf("Proc: %p\n", COMIO_Interrupt3) ;
    /* Second, declare the new interrupt. */
    _disable() ;
    _dos_setvect(ioInterrupt+8, G_ListInterrupts[ioInterrupt]) ;

printf("New: %p\n", G_ListInterrupts[ioInterrupt]) ;

    /* Reset the bit that let's us access the normal modem registers. */
    outp(ioAddress+COM_REG_LINE_CONTROL,
        inp(ioAddress+COM_REG_LINE_CONTROL) & 0x7F) ;

    /* Turn on the modem interrupt flag and the DTR/RTS lines
       (and make sure loop back testing is turned off.) */
    data = inp(ioAddress+COM_REG_MODEM_CONTROL) ;
printf("Modem con: %02X\n", data) ;
    outp(ioAddress+COM_REG_MODEM_CONTROL, ((data & 0xEF) | 0x0B)) ;
printf("new Modem con: %02X\n", data=inp(ioAddress+COM_REG_MODEM_CONTROL)) ;

    /* Make sure we are accessing the right second group of registers. */
printf("Line con: %02X\n", data=inp(ioAddress+COM_REG_LINE_CONTROL)) ;
    outp(ioAddress+COM_REG_LINE_CONTROL,
        inp(ioAddress+COM_REG_LINE_CONTROL) & 0x7F) ;
printf("new Line con: %02X\n", data=inp(ioAddress+COM_REG_LINE_CONTROL)) ;

printf("PIC: %02X\n", data=inp(PIC_MASK)) ;
    /* Make sure the PIC interrupt mask register is off (0=enabled). */
    outp(PIC_MASK, data=inp(PIC_MASK) & (~(1 << ioInterrupt))) ;
printf("new PIC: %02X\n", inp(PIC_MASK)) ;

    /* Turn on receive interrupts. */
printf("IER: %02X\n", data=inp(ioAddress+COM_REG_INTERRUPT_ENABLE)) ;
    outp(ioAddress+COM_REG_INTERRUPT_ENABLE, 0x01 /* 0x01 */) ;
printf("new IER: %02X\n", data=inp(ioAddress+COM_REG_INTERRUPT_ENABLE)) ;

    _enable() ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  DeactivateInterrupt                                           */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*      DeactivateInterrupt is used to turn off an interrupt that was       */
/*    originally active.  This routine MUST NOT be called when an           */
/*    interrupt handler is already inactive.                                */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*      None really                                                         */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_byte8 ioInterrupt         -- Level of interrupt to deactivate       */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    _dos_getvect                                                          */
/*    _dos_setvect                                                          */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  11/12/94  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void DeactivateInterrupt(T_byte8 ioInterrupt, T_word16 ioAddress)
{
    T_byte8 data ;

    DebugRoutine("DeactivateInterrupt") ;

    /* Make sure the interrupt we are deactivating is in place. */
    DebugCheck(_dos_getvect(ioInterrupt+8) == G_ListInterrupts[ioInterrupt]) ;
    DebugCheck(ioInterrupt < COM_NUMBER_INTERRUPTS) ;

/* !!! */
printf("Deactivating interrupt %d\n", ioInterrupt) ;

    /* Turn off the interrupts for a second. */
    _disable() ;

    /* Turn off the modem interrupt flag */
    outp(ioInterrupt+COM_REG_MODEM_CONTROL,
        inp(ioInterrupt+COM_REG_MODEM_CONTROL) & 0xF7) ;

    /* Restore the old interrupt. */
    _dos_setvect(ioInterrupt+8, G_Interrupts[ioInterrupt]) ;

    /* Turn off all interrupts from this port. */
/*
    outp(ioInterrupt+COM_REG_INTERRUPT_ENABLE, 0) ;
*/

    /* Turn off all interrupts for this port. */
printf("IER: %02X\n", data=inp(ioAddress+COM_REG_INTERRUPT_ENABLE)) ;
    outp(ioAddress+COM_REG_INTERRUPT_ENABLE, 0x00 /* 0x01 */) ;
printf("new IER: %02X\n", data=inp(ioAddress+COM_REG_INTERRUPT_ENABLE)) ;

printf("PIC: %02X\n", data=inp(PIC_MASK)) ;
    /* Make sure the PIC interrupt mask register is off (0=enabled). */
    outp(PIC_MASK, data=inp(PIC_MASK) | (1 << ioInterrupt)) ;
printf("new PIC: %02X\n", inp(PIC_MASK)) ;

    /* Turn off the modem interrupt flag */
    data = inp(ioAddress+COM_REG_MODEM_CONTROL) ;
printf("Modem con: %02X\n", data) ;
    outp(ioAddress+COM_REG_MODEM_CONTROL, 0) ;
printf("new Modem con: %02X\n", data=inp(ioAddress+COM_REG_MODEM_CONTROL)) ;

    _enable() ;

    DebugEnd() ;
}

/****************************************************************************/
/*    END OF FILE:  COMIO.C                                                 */
/****************************************************************************/

/* Just an idea: */
T_void COMIO_ForceSendByte(T_COMPort port, T_byte8 data)
{
    DebugRoutine("COMIO_ForceSendByte") ;
    DebugCheck(port < COM_MAX_PORTS) ;

    /* Send the byte on its merry way. */
    outp(G_Port[port].ioAddress+COM_REG_TRANSMIT, data) ;

    DebugEnd() ;
}

