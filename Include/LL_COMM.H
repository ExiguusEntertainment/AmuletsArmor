/*
** LL_COMM (R) Asynchronous Communication Routines
** Copyright (C) 1994 by James P. Ketrenos
**
** VERSION	:	1.2
** DATE 	:	7-4-94
** CODED BY :   James P. Ketrenos [ketrenoj@cs.pdx.edu]
**      aka :   Lord Logics (for those who care)
**
** Special thanks to all of the contributors to the Serial.FAQ
**
*
*	NOTE:  As of this release, these routines only allow for ONE port to be
*	opened at a time.  Hopefully, if all goes well, the next version will
*	allow for up to 32 different ports to be opened.  I have set up these
*	routines to be compatible with the future format, so using the next
*	version shouldn't require much, if any, change to working code.
*
*	Also, you probably have noticed that the assembly listings for this code
*	have not been included.  If you are interested in the assembly code,
*	please read the file LL_COMM.NFO, as it explains why it is not here, and
*	where/how you can acquire it.
*
**
*/

#ifndef _LL_COMM_H_
#define _LL_COMM_H_
#ifdef	__cplusplus
#define CEXT	extern "C"
#else
#define CEXT	extern
#endif

typedef int 	COMM;
typedef char	*PACKET;

#ifndef TRUE
#define TRUE	1
#define FALSE	0
#endif

/* Initialization Routines	****/
CEXT    int ioOpenPort(T_word32 base, T_word32 irq) ;
CEXT	int 	ioClosePort(COMM Port);

/* Buffer Routines			****/
CEXT	void	ioClearWrite(COMM);
CEXT	void	ioClearRead(COMM);
CEXT	int 	ioReadStatus(COMM);
CEXT	int 	ioWriteStatus(COMM);

/* I/O Routines 			****/
CEXT    T_byte8 ioReadByte(int address) ;
CEXT    int ioWriteByte(int address, T_byte8 b) ;
CEXT	int 	ioReadPacket(COMM, PACKET);
CEXT	int 	ioWritePacket(COMM, PACKET);

/* Mode Routines			****/
CEXT	int 	ioGetMode(COMM);
CEXT	int 	ioSetMode(COMM, int);

/* Port Setup Routines		****/
CEXT	int 	ioGetBaud(COMM);
CEXT    void    ioSetBaud(COMM, int);
CEXT	int 	ioGetHandShake(COMM);
CEXT	void	ioSetHandShake(COMM, int);
CEXT	int 	ioGetStatus(COMM);
CEXT	int 	ioGetControl(COMM);
CEXT	void	ioSetControl(COMM, int);
CEXT	int 	ioGetFIFO(COMM);
CEXT	int 	ioSetFIFO(COMM, int);

/*
** ISR I/O Modes
*/
/********************************........xx..	*/
#define BYTE_MODE		0x00	/* 00000000b	*/
#define PACKET_MODE 	0x01	/* 00000001b	*/
#define VARIABLE_MODE	0x02	/* 00000010b	*/

/*
** Line Control Register
*/
/********************************........xx..	*/
#define CLEAR_BITS		0xFC	/* 11111100b	*/
#define BITS_5			0x00	/* 00000000b	*/
#define BITS_6			0x01	/* 00000001b	*/
#define BITS_7			0x02	/* 00000010b	*/
#define BITS_8			0x03	/* 00000011b	*/
/********************************.......x...	*/
#define CLEAR_STOP		0xFB	/* 11111011b	*/
#define STOP_1			0x00	/* 00000000b	*/
#define STOP_2			0x04	/* 00000100b (Doesn't work w/ BITS_5) */
/*********************************...xxx....	*/
#define CLEAR_PARITY	0xE7	/* 11000111b	*/
#define NO_PARITY		0x00	/* 00000000b	*/
#define ODD_PARITY		0x08	/* 00001000b	*/
#define EVEN_PARITY 	0x18	/* 00011000b	*/
#define MARK_PARITY 	0x28	/* 00101000b	*/
#define SPACE_PARITY	0x38	/* 00111000b	*/
/********************************...x.......	*/
#define CLEAR_MODE		0xBF	/* 10111111b	*/
#define NORMAL			0x00	/* 00000000b	*/
#define BREAK			0x40	/* 01000000b	*/

/*
** HandShaking
*/
#define	DTR				0x01	// 00000001b
#define	RTS				0x02	// 00000010b
#define LOOPBACK        0x10    // 00010000b

/*
** Modem/Line Status Register
*/
#define	D_CTS			0x0100	// 00000001 00000000b
#define	D_DSR			0x0200	// 00000010 00000000b
#define	D_RI			0x0400	// 00000100 00000000b
#define	D_DCD			0x0800	// 00001000 00000000b
#define	CTS				0x1000	// 00010000 00000000b
#define	DSR				0x2000	// 00100000 00000000b
#define	RI				0x4000	// 01000000 00000000b
#define	DCD				0x8000	// 10000000 00000000b
#define RBF 			0x0001	// 00000000 00000001b
#define OVR_ERROR		0x0002	// 00000000 00000010b
#define	PAR_ERROR		0x0004	// 00000000 00000100b
#define FRM_ERROR		0x0008	// 00000000 00001000b
#define	BRK_ERROR		0x0010	// 00000000 00010000b
#define	THREMPTY		0x0020	// 00000000 00100000b
#define TEMPTY			0x0040	// 00000000 01000000b
#define FFO_ERROR		0x0080	// 00000000 10000000b


/* WATCOM Specific Parameter Passing Setup	*/
/*
#pragma aux  ioOpenPort 	parm	[EDX] [ECX]
#pragma aux  ioClosePort	parm	[EAX]

#pragma aux  ioClearRead	parm	[EAX]
#pragma aux  ioClearWrite	parm	[EAX]
#pragma aux  ioReadStatus	parm	[EAX]
#pragma aux  ioWriteStatus	parm	[EAX]

#pragma aux  ioReadByte 	parm	[EAX]
#pragma aux  ioWriteByte	parm	[EAX] [ECX]
#pragma aux  ioReadPacket	parm	[EAX]
#pragma aux  ioWritePacket	parm	[EAX] [ESI]

#pragma aux  ioGetBaud		parm	[EAX]
#pragma aux  ioSetBaud		parm	[EAX] [EBX]

#pragma aux  ioGetHandShake parm	[EAX]
#pragma aux  ioSetHandShake parm	[ECX] [EAX]

#pragma aux  ioGetStatus	parm	[EAX]

#pragma aux  ioGetControl	parm	[EAX]
#pragma aux  ioSetControl	parm	[ECX] [EAX]

#pragma aux  ioGetFIFO		parm	[EAX]
#pragma aux  ioSetFIFO		parm	[EAX] [ECX]

#pragma aux  ioGetMode		parm	[EAX]
#pragma aux  ioSetMode		parm	[EAX] [ECX]

*/

#endif
