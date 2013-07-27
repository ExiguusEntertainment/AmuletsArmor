/****************************************************************************/
/*    FILE:  TEXT.H                                                       */
/****************************************************************************/

#ifndef _TEXT_H_
#define _TEXT_H_

#include "GENERAL.H"
#include "GRAPHIC.H"
#include "RESOURCE.H"

#define MAX_TEXTS 50

typedef T_void *T_textID;

typedef struct
{
	T_graphicID 	p_graphicID;
	T_byte8			*data;
	T_byte8			fcolor;
	T_byte8			bcolor;
	T_resource 		font ;
} T_textStruct;


T_textID TextCreate (T_word16 lx,
							T_word16 ly,
							const T_byte8  *string);

T_void TextDelete (T_textID textID);
T_void TextCleanUp (T_void);

T_void TextSetFont (T_textID textID, T_byte8 *fontname);
T_void TextSetColor (T_textID textID, T_byte8 fcolor, T_byte8 bcolor);
T_void TextSetText (T_textID textID, const T_byte8 *string);
T_void TextMoveText (T_textID textID, T_word16 x, T_word16 y);

T_void TextDrawCallBack (T_graphicID graphicID, T_word16 index);

T_graphicID TextGetGraphicID (T_textID textID);
T_void TextCleanString (T_byte8 *stringToClean);

#endif
