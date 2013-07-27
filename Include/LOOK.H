/****************************************************************************/
/*    FILE:  LOOK.H                                                         */
/****************************************************************************/
#ifndef _LOOK_H_
#define _LOOK_H_

#include "GENERAL.H"
#include "VIEWFILE.H"

typedef struct
{
    T_byte8 name[32];
    T_byte8 description[256];
    T_byte8 picturename[64];
} T_lookDataStruct;

T_void LookInit               (T_void);
T_void LookUpdateCreatureInfo (T_3dObject *p_obj);
T_void LookRequestPlayerInfo  (T_3dObject *p_obj);
T_void LookUpdatePlayerInfo   (T_lookDataStruct *p_lookData);

#endif
