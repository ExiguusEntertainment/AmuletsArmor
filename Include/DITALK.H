/****************************************************************************/
/*    FILE:  DITALK.H                                                       */
/****************************************************************************/
/* The Direct Talk module is used to have a real mode application talk      */
/* to a dos32 application.  Handshaking and all is handled through these    */
/* routines.                                                                */
/****************************************************************************/
#ifndef _DITALK_H_
#define _DITALK_H_

#include "GENERAL.H"

typedef T_void (*T_directTalkReceiveCallback)(
                    T_void *p_data,
                    T_word16 size) ;

typedef T_void (*T_directTalkSendCallback)(
                    T_void *p_data,
                    T_byte8 *size,
                    E_Boolean *anyData) ;

typedef T_void (*T_directTalkConnectCallback)(T_void *p_data) ;

typedef T_void (*T_directTalkDisconnectCallback)(T_void) ;

typedef T_word32 T_directTalkHandle ;
#define DIRECT_TALK_HANDLE_BAD        0

typedef T_byte8 E_directTalkLineStatus ;
#define DIRECT_TALK_LINE_STATUS_CONNECTING     0
#define DIRECT_TALK_LINE_STATUS_TIMED_OUT      1
#define DIRECT_TALK_LINE_STATUS_ABORTED        2
#define DIRECT_TALK_LINE_STATUS_CONNECTED      3
#define DIRECT_TALK_LINE_STATUS_DISCONNECTING  4
#define DIRECT_TALK_LINE_STATUS_DIALING        5
#define DIRECT_TALK_LINE_STATUS_INITIALIZED    6
#define DIRECT_TALK_LINE_STATUS_BUSY           7
#define DIRECT_TALK_LINE_STATUS_UNKNOWN        8

typedef T_byte8 E_directTalkServiceType ;
#define DIRECT_TALK_SELF_SERVER                0
#define DIRECT_TALK_SERIAL_TWO_PLAYER          1
#define DIRECT_TALK_MODEM_TWO_PLAYER           2
#define DIRECT_TALK_IPX                        3
#define DIRECT_TALK_DRAGON_MODEM               4
#define DIRECT_TALK_UNKNOWN                    5

typedef struct {
    T_byte8 address[6] ;
} T_directTalkUniqueAddress ;

/* Start talking. */
T_directTalkHandle DirectTalkInit(
           T_directTalkReceiveCallback p_callRecv,
           T_directTalkSendCallback p_callSend,
           T_directTalkConnectCallback p_callConnect,
           T_directTalkDisconnectCallback p_callDisconnect,
           T_directTalkHandle handle) ;

/* Close out all the talking. */
T_void DirectTalkFinish(T_directTalkHandle handle) ;

#ifdef COMPILE_OPTION_DIRECT_TALK_IS_DOS32
/* Routine to send data out the talk (only used by DOS32 side). */
T_void DirectTalkSendData(T_void *p_data, T_byte8 size) ;

/* Routine to request data be received (only used by DOS32 side). */
T_void DirectTalkPollData(T_void) ;

T_void DirectTalkConnect(T_byte8 *p_address) ;

T_void DirectTalkDisconnect(T_void) ;

#endif

E_directTalkLineStatus DirectTalkGetLineStatus(T_void) ;

T_void DirectTalkSetLineStatus(E_directTalkLineStatus status) ;

T_void DirectTalkGetUniqueAddress(T_directTalkUniqueAddress *p_unique) ;

T_void DirectTalkSetUniqueAddress(T_directTalkUniqueAddress *p_unique) ;

T_directTalkUniqueAddress *DirectTalkGetNullBlankUniqueAddress(T_void) ;

T_void DirectTalkPrintAddress(FILE *fp, T_directTalkUniqueAddress *p_addr) ;

E_directTalkServiceType DirectTalkGetServiceType(T_void) ;

T_void DirectTalkSetServiceType(E_directTalkServiceType serviceType) ;

T_void DirectTalkSetDestination(T_directTalkUniqueAddress *p_dest) ;

T_void DirectTalkSetDestinationAll(T_void) ;

T_byte8 *DirectTalkGetDestination(T_void) ;

T_byte8 DirectTalkIsBroadcastAddress(T_directTalkUniqueAddress *p_dest);

#endif // _DITALK_H_

/****************************************************************************/
/*    END OF FILE:  DITALK.H                                                */
/****************************************************************************/
