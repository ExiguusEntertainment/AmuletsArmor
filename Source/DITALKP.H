#ifndef _DITALK_PRIVATE_H_
#define _DITALK_PRIVATE_H_

#define DIRECT_TALK_MAX_SIZE_BUFFER      256

/* Range of allowable vectors. */
#define VALID_LOW_VECTOR               0x60
#define VALID_HIGH_VECTOR              0x67

#define DIRECT_TALK_TAG                0xAABBDDCC
#define DIRECT_TALK_DEAD_TAG           0x99665544

/* Enumerate the different type of commands that can be sent back */
/* and forth. */
/* Commands are in reference to the DOS32 side (not the REAL side), */
/* therefore, SEND means that the DOS32 program is sending data */
/* to the REAL side. */
typedef T_byte8 E_directTalkCommand ;
#define DIRECT_TALK_COMMAND_SEND         0
#define DIRECT_TALK_COMMAND_RECV         1
#define DIRECT_TALK_COMMAND_CONNECT      2
#define DIRECT_TALK_COMMAND_DISCONNECT   3
#define DIRECT_TALK_COMMAND_UNKNOWN      4

typedef struct {
    T_word32 tag ;                     /* Identifier to ensure correct tag. */
    T_byte8 vector ;                   /* Vector to communicate. */
    T_directTalkHandle handle ;
    E_directTalkCommand command ;
    E_directTalkLineStatus lineStatus ;
    T_byte8 buffer[DIRECT_TALK_MAX_SIZE_BUFFER] ;
    T_byte8 bufferLength ;             /* Size of data in buffer. */
    E_Boolean bufferFilled ;           /* TRUE if anything in buffer. */
    T_byte8 uniqueAddress[6] ;            /* Unique address to this comlink. */
    E_directTalkServiceType serviceType ;
    T_byte8 destinationAddress[6] ;       /* Destination */
} T_directTalkStruct ;

//extern T_directTalkReceiveCallback G_receiveCallback;
//extern T_directTalkSendCallback G_sendCallback;
//extern T_directTalkConnectCallback G_connectCallback;
//extern T_directTalkDisconnectCallback G_disconnectCallback;

#endif // _DITALK_PRIVATE_H_
