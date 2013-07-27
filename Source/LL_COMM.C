#include "COMM.H"

#if defined(DOS32)

#define ON 1
#define OFF 0
#define TRUE 1
#define FALSE 0
#define PIC 0x20
#define PICM 0x21
#define EOI 0x20

#define THR 0
#define RBR 0
#define BAUDL 0
#define BAUDH 1
#define IER 1
#define IIR 2
#define FCR 2
#define LCR 3
#define MCR 4
#define LSR 5
#define MSR 6
#define UART 0x3
#define MODE 0xC

#define FIFO                       2
#define FIFO_ENABLE                0x01
#define FCR_TRIGGER_RX_LEVEL_1     0x00
#define FCR_TRIGGER_RX_LEVEL_4     0x40
#define FCR_TRIGGER_RX_LEVEL_8     0x80
#define FCR_TRIGGER_RX_LEVEL_14    0xC0
#define LSR_DATA_READY             0x01

typedef T_void (*T_function)() ;

static T_void ISR_Receive(T_void) ;
static T_void ISR_Transmit(T_void) ;
static T_void ISR_MStat(T_void) ;
static T_void ISR_LStat(T_void) ;

typedef struct {
    T_word16 Base ;
    T_byte8 IRQ ;
    T_byte8 Flags ;
} T_PortID ;


static T_PortID PortTable ;
static T_word32 PortCode = 0 ;
static T_word16 PortInUse = 0 ;
static T_word16 Port = 0 ;
static T_word16 Registers[7] ;
static T_byte8 Vector ;
static T_byte8 DisableIRQ ;
static T_byte8 EnableIRQ ;
static T_word16 ISR_OldS ;
static T_word32 ISR_OldO ;

typedef enum {
    UART_TYPE_8250,
    UART_TYPE_16550,
    UART_TYPE_UNKNOWN
} E_uartType ;

static E_uartType G_uartType = UART_TYPE_8250 ;
static T_byte8 G_primed = 0 ;

#define SendSize (4096-1)
static T_byte8 SendBuffer[SendSize+2] ;
static T_word16 SendHead = 0 ;
static T_word16 SendTail = 0 ;

#define RecvSize (4096-1)
static T_byte8 RecvBuffer[RecvSize+2] ;
static T_word16 RecvHead = 0 ;
static T_word16 RecvTail = 0 ;


int ioGetMode(int adr)
{
    return 0 ;
}

int ioSetMode(int adr, int i)
{
    return 0 ;
}

static T_void __interrupt __far ISR_PM(T_void)
{
    T_byte8 b ;
    T_word16 pos ;
    T_byte8 reg ;
    T_byte8 count ;

*((char far *)0xA0000000L) = 31 ;
    for (;;) {
        reg = inp(Registers[IIR]) ;
        if (reg & 1)
            break ;

        reg >>= 1 ;
        reg &= 3 ;

        switch(reg)  {
            case 0:     /* Modem status changed. */
                inp(Registers[MSR]) ;
                break ;
            case 1:     /* Transfer */
                if (G_uartType == UART_TYPE_16550)
                    count = 16 ;
                else
                    count = 1 ;

                while (count)  {
                    count-- ;
                    pos = SendTail ;
                    if (pos != SendHead)  {
                        b = SendBuffer[SendTail++] ;
                        SendTail &= SendSize ;
                        outp(Registers[THR], b) ;
                    } else {
                        G_primed = 0 ;
                        break ;
                    }
                }
                break ;
            case 2:     /* Received char */
                do {
                    pos = RecvHead ;
                    RecvBuffer[pos++] = inp(Registers[RBR]) ;
                    pos &= RecvSize ;
                    if (pos != RecvTail)
                        RecvHead = pos ;
                } while ((G_uartType == UART_TYPE_16550) &&
                         (inp(Registers[LSR]) & LSR_DATA_READY)) ;
                break ;
            case 3:     /* Line status changed. */
                inp(Registers[LSR]) ;
                break ;
        }
    }
*((char far *)0xA0000000L) = 0 ;

    outp(PIC, EOI) ;
}

static T_void (__interrupt __far *IOldComInterrupt)();

static void IioClosePortAtExit(void) ;

int ioOpenPort(T_word32 base, T_word32 irq)
{
    T_byte8 mask ;
    T_word16 i ;
    T_byte8 type ;

//printf("Checking everything: ") ;
    if ((PortInUse == ON) || (irq == 0) || (irq >= 8))  {
//        printf("NOT OK\n") ;
//        if (PortInUse == ON)
//            printf("PORT in use\n") ;
//        else
//            printf("bad irq\n") ;
        return 0 ;
    }
//printf("OK\n") ;
    PortInUse = ON ;
    Port = (T_word16)base ;
    Vector = irq+8 ;

    mask = 1<<irq ;
    DisableIRQ = mask ;
    EnableIRQ = (~mask) ;

    for (i=0; i<=6; i++)
        Registers[i] = base+i ;

    ioClearWrite(0) ;
    ioClearRead(0) ;

    printf("UART type is ") ;
    outp(Registers[FIFO], FIFO_ENABLE | FCR_TRIGGER_RX_LEVEL_4) ;
    type = inp(Registers[IIR]) ;
    if ((type & 0xF8) == 0xC0)  {
        G_uartType = UART_TYPE_16550 ;
        puts("16650") ;
    } else {
        G_uartType = UART_TYPE_8250 ;
        puts("8250") ;

        /* Make sure that if we miss identified the 16550 that the */
        /* buffering is turned off. */
        outp(Registers[FIFO], 0) ;
    }

    _disable() ;

printf("Installing interrupt\n") ;
    IOldComInterrupt = _dos_getvect((int)Vector);
    _dos_setvect((int)Vector, ISR_PM);

    outp(Registers[MCR], inp(Registers[MCR]) | 0x08) ;
    outp(Registers[LCR], inp(Registers[LCR]) & 0x7F) ;
    outp(Registers[IER], 1) ;
    outp(PICM, inp(PICM) & EnableIRQ) ;

    _enable() ;

    atexit(IioClosePortAtExit) ;

    ioClearWrite(0) ;
    ioClearRead(0) ;
    PortCode = (T_word32)&PortTable ;

    return ((int)PortCode) ;
}

static void IioClosePortAtExit(void)
{
#ifndef NDEBUG
    puts("Closing io comm port") ;
#endif
    ioClosePort(0) ;
}

int ioClosePort(int address)
{
    if (PortInUse != ON)
        return -1 ;

    outp(PICM, (inp(PICM) | DisableIRQ)) ;
    outp(Registers[IER], 0) ;
    outp(Registers[MCR], inp(Registers[MCR]) & 0xF7) ;

//    _disable() ;
    _dos_setvect((int)Vector, IOldComInterrupt);
//    _enable() ;

    PortInUse = OFF ;

    return 0 ;
}


void ioClearRead(int address)
{
    _disable() ;
    RecvHead = RecvTail = 0 ;
    _enable() ;
}

void ioClearWrite(int address)
{
    _disable() ;
    SendHead = SendTail = 0 ;
    _enable() ;
}

T_byte8 ioReadByte(int address)
{
    T_byte8 b ;
    T_word16 pos ;

    if (RecvTail == RecvHead)
        return 0 ;

    pos = RecvTail ;
    b = RecvBuffer[pos++] ;
    pos &= RecvSize ;
    RecvTail = pos ;

//printf("R[%02X]\n", b) ;  fflush(stdout) ;
    return b ;
}

int ioWriteByte(int address, T_byte8 b)
{
    T_word16 pos ;
    T_word16 next ;

    pos = SendHead ;
    next = (pos+1) & SendSize ;
    if (next == SendTail)
        return 0 ;

    SendBuffer[pos] = b ;
    SendHead = next ;

    if (G_primed == 0)  {
        outp(Registers[IER], inp(Registers[IER]) & (~2)) ;
        outp(Registers[IER], inp(Registers[IER]) | 2) ;
        G_primed = 1 ;
    }

    return -1 ;
}

int ioReadStatus(int address)
{
    T_sword16 value ;

    value = RecvHead - RecvTail ;

    if (value < 0)
        value += RecvSize ;

    /* TESTING DTR AND CTS */
/*
    if (value == 0)  {
        outp(Registers[MCR], inp(Registers[MCR]) & 0xFE) ;
    } else {
        outp(Registers[MCR], inp(Registers[MCR]) | 0x01) ;
    }
*/

    return value ;
}

int ioWriteStatus(int address)
{
    T_sword16 value ;

    value = SendHead - SendTail ;

    if (value < 0)
        value += SendSize ;

    return value ;
}

T_void ioSetBaud(int address, int baud)
{
    T_word32 speed ;

    if (baud == 0)
        return ;

    speed = 115200 / baud ;

    _disable() ;

    outp(Registers[LCR], inp(Registers[LCR]) | 0x80) ;

    outp(Registers[BAUDL], speed & 0xFF) ;
    outp(Registers[BAUDH], (speed & 0xFF00)>>8) ;
    outp(Registers[LCR], inp(Registers[LCR]) & 0x7F) ;

    _enable() ;
}

int ioGetBaud(int address)
{
    T_word32 baud ;

    _disable() ;
    outp(Registers[LCR], inp(Registers[LCR]) | 0x80) ;
    baud = ((T_word32)inp(Registers[BAUDH])) << 8;
    baud |= ((T_word32)inp(Registers[BAUDL])) ;
    outp(Registers[LCR], inp(Registers[LCR]) & 0x7F) ;
    _enable() ;

    if (baud != 0)
        baud = 115200 / baud ;

    return baud ;
}

T_void ioSetHandShake(int address, int hand)
{
    outp(Registers[MCR], hand | 0x08) ;
}

int ioGetHandShake(int address)
{
    return inp(Registers[MCR]) ;
}

int ioGetStatus(int address)
{
    T_byte8 b ;

    b = (((T_word32)inp(Registers[MSR])) << 8)  |
        ((T_word32)inp(Registers[LSR])) ;

    return (T_word32)b ;
}

int ioGetControl(int address)
{
    return(inp(Registers[LCR])) ;
}

T_void ioSetControl(int address, int control)
{
    outp(Registers[LCR], control&0x7F) ;
}

#endif

