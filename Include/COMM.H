/****************************************************************************/
/*    FILE:  COMM.H                                                         */
/****************************************************************************/
#ifndef _COMM_H_
#define _COMM_H_

#include "GENERAL.H"

#define MAX_COMM_PORTS 64

typedef enum {
    BAUD_RATE_2400,
    BAUD_RATE_9600,
    BAUD_RATE_19200,
    BAUD_RATE_57600,
    BAUD_RATE_UNKNOWN
} E_commBaudRate ;

typedef enum {
   COMM_TYPE_NULL_MODEM,
   COMM_TYPE_STANDARD_MODEM,
   COMM_TYPE_IRQ_MODEM,
   COMM_TYPE_SELF,
   COMM_TYPE_SELF_CLIENT,
   COMM_TYPE_UNKNOWN
} E_commType ;

#define COMM_PORT_BAD NULL

#define COMM_SUB_TYPE_DIRECT_MASTER      0
#define COMM_SUB_TYPE_DIRECT_SLAVE       1
#define COMM_SUB_TYPE_DIAL               2
#define COMM_SUB_TYPE_ANSWER             3
#define COMM_SUB_TYPE_UNKNOWN            4

typedef T_void *T_commPort ;

T_commPort CommOpenPort(
               E_commType type,
               T_word16 addressOrComNum,
               T_word16 irq,
               E_commBaudRate baud) ;

T_void CommClosePort(T_commPort port) ;

T_void CommSetActivePort(T_commPort port) ;

T_word16 CommScanByte(T_void) ;

T_word16 CommReadByte(T_void) ;

T_void CommSendByte(T_byte8) ;

T_word16 CommScanData(T_byte8 *p_data, T_word16 numberBytes) ;

T_word16 CommReadData(T_byte8 *p_data, T_word16 numberBytes) ;

T_void CommSendData(T_byte8 *p_data, T_word16 numberBytes) ;

T_word16 CommGetReadBufferLength(T_void) ;

T_word16 CommGetSendBufferLength(T_void) ;

T_void CommRewindScan(T_word16 numChar) ;

T_word16 CommReadConfigFile(T_byte8 *p_mode, T_byte8 *p_otherData) ;

T_void CommCloseAll(T_void) ;

T_void CommSetActivePortN(T_word16 numOfPort) ;

T_word16 CommGetNumberPorts(T_void) ;

E_commBaudRate CommGetBaudRate(T_void) ;

T_word32 CommConvertBaudTo32(E_commBaudRate baud) ;

E_commType CommGetPortType(void) ;

T_void CommClearPort(T_void) ;

E_Boolean CommCheckClientAndServerExist(T_void) ;

#ifndef SERVER_ONLY
E_Boolean CommIsServer(T_void) ;
#else
#define CommIsServer()    TRUE           // If SERVER_ONLY, always is server
#endif

T_word16 CommGetLinkSubType(T_void) ;

#endif

/****************************************************************************/
/*    END OF FILE:  COMM.H                                                  */
/****************************************************************************/
