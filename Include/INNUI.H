#ifndef _INNUI_H_
#define _INNUI_H_

#include "GENERAL.H"

typedef enum
{
    INN_ROOM_COMMONS,
    INN_ROOM_SMALL,
    INN_ROOM_LARGE,
    INN_ROOM_SUITE,
    INN_ROOM_UNKNOWN
} E_innRoomType;

T_void InnUIStart    (T_word32 formNum);
T_void InnUIUpdate   (T_void);
T_void InnUIEnd      (T_void);

#endif
