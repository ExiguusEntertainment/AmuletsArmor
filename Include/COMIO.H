/****************************************************************************/
/*    FILE:  COMIO.H                                                        */
/****************************************************************************/

#ifndef _COMIO_H_
#define _COMIO_H_

#include "general.h"

/* COMIO Routines: */

typedef T_word16 T_COMPort ;

#define COM_ERROR_MASK         0xc000   /* xxxx000000000000 (bits) */
#define COM_ERROR_DATA_MASK    0x0FFF   /* 0000xxxxxxxxxxxx */

#define COM_ERROR_NO_ERROR     0x0000   /* 0000------------ */
#define COM_ERROR_NO_PORT      0x8000   /* 1000------------ */
#define COM_ERROR_EMPTY_BUFFER 0x9000   /* 1001------------ */
#define COM_ERROR_FULL_BUFFER  0xA000   /* 1010------------ */

#define COM_MAX_PORTS          16
#define COM_NUMBER_INTERRUPTS  8

#define COM_BUFFER_SIZE       1024

typedef T_word32 T_COMBaudRate ;

typedef enum {
    COM_PORT_TYPE_8250,  /* default */
    COM_PORT_TYPE_16550,
    COM_PORT_TYPE_UNKNOWN
} E_COMPortType ;

typedef enum {
    COM_BIT_LENGTH_5,
    COM_BIT_LENGTH_6,
    COM_BIT_LENGTH_7,
    COM_BIT_LENGTH_8,
    COM_BIT_LENGTH_UNKNOWN
} E_COMBitLength ;

typedef enum {
    COM_STOP_BIT_1,
    COM_STOP_BIT_1_POINT_5,
    COM_STOP_BIT_UNKNOWN
} E_COMStopBit ;

typedef enum {
    COM_PARITY_NONE,
    COM_PARITY_ODD,
    COM_PARITY_EVEN,
    COM_PARITY_MARK,
    COM_PARITY_SPACE,
    COM_PARITY_UNKNOWN
} E_COMParity ;

/*-------------------------- Function Prototypes ---------------------------*/
/* Port control functions: */
T_void COMIO_Initialize(T_void) ;

T_COMPort COMIO_Open(T_word16 io_address, T_byte8 io_interrupt) ;

T_void COMIO_Close(T_COMPort port) ;

T_void COMIO_SetBaudRate(T_COMPort port, T_COMBaudRate baud) ;

T_void COMIO_SetTypePort(T_COMPort port, E_COMPortType type) ;

T_void COMIO_SetControl(
               T_COMPort port,
               E_COMBitLength bit_length,
               E_COMStopBit stop_bit,
               E_COMParity parity) ;

E_Boolean COMIO_CheckCarrier(T_COMPort port) ;

T_void COMIO_EnablePort(T_COMPort port) ;

T_void COMIO_DisablePort(T_COMPort port) ;

E_Boolean COMIO_GetEnableStatus(T_COMPort port) ;


/* Port read functions: */
T_word16 COMIO_ReceiveByte(T_COMPort port) ;

T_word16 COMIO_GetReceiveCount(T_COMPort port) ;

T_word16 COMIO_ReceiveData(T_COMPort port, T_word16 count, T_byte8 *buffer) ;


/* Port write functions: */
T_word16 COMIO_SendByte(T_COMPort port, T_byte8) ;

T_void COMIO_ForceSendByte(T_COMPort port, T_byte8) ;

T_word16 COMIO_GetSendCount(T_COMPort port) ;

T_word16 COMIO_SendData(T_COMPort port, T_word16 count, T_byte8 *buffer) ;

/* Utility functions: */
T_void COMIO_ClearReceiveBuffer(T_COMPort port) ;

T_void COMIO_ClearSendBuffer(T_COMPort port) ;
/*--------------------------------------------------------------------------*/

#endif

/****************************************************************************/
/*    END OF FILE:  COMIO.H                                                 */
/****************************************************************************/
