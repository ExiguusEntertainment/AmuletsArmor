/****************************************************************************

   File              : sosdefs.h

   Programmer(s)     : Don Fowler, Nick Skrepetos
   Date              :

   Purpose           : 

   Last Updated      :

****************************************************************************
               Copyright(c) 1993,1994 Human Machine Interfaces 
                            All Rights Reserved
****************************************************************************/


#ifndef  _SOSDEFS_DEFINED
#define  _SOSDEFS_DEFINED

#undef   _TRUE
#undef   _FALSE
#undef   TRUE
#undef   FALSE
#undef   _NULL
#ifndef _TRUE
enum
      {
         _FALSE,
         _TRUE
      };
#endif

#define  _NULL  0

#ifndef  VOID
#define  VOID           void
#endif
typedef  int            BOOL;
typedef  unsigned int   UINT;
typedef  unsigned char  BYTE;
typedef  unsigned short WORD;
typedef  unsigned int   W32;
typedef  unsigned short W16;
#ifndef  LONG
typedef  signed long    LONG;
#endif
typedef  unsigned long  DWORD;

typedef  BYTE  *        PBYTE;
typedef  char near *    PSTR;
typedef  WORD  *        PWORD;
typedef  unsigned short  * PSHORT;
typedef  LONG  *        PLONG;
typedef  VOID  *        PVOID;

typedef  BYTE  far   *  LPBYTE;
typedef  BYTE  far   *  LPSTR;
typedef  WORD  far   *  LPWORD;
typedef  W32   far   *  LPW32;
typedef  LONG  far   *  LPLONG;
typedef  VOID  far   *  LPVOID;

typedef  BYTE  huge  *  HPBYTE;
typedef  BYTE  huge  *  HPSTR;
typedef  LONG  huge  *  HPLONG;
typedef  VOID  huge  *  HPVOID;

typedef  unsigned       HANDLE;

#endif

