/****************************************************************************/
/*    FILE:  COMPRESS.H                                                       */
/****************************************************************************/
#ifndef _COMPRESS_H_
#define _COMPRESS_H_

#include "GENERAL.H"

T_void *CompressBlock(
            T_void *p_block,
            T_word32 size,
            T_word32 *p_newSize) ;

T_void *UncompressBlock(
            T_void *p_block,
            T_word32 size,
            T_word32 *p_newSize) ;

#endif

/****************************************************************************/
/*    END OF FILE:  COMPRESS.H                                                */
/****************************************************************************/
