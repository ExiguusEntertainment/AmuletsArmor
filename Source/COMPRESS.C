/****************************************************************************/
/*    FILE:  COMPRESS.H                                                       */
/****************************************************************************/
#include "COMPRESS.H"
#include "GENERAL.H"
#include "MEMORY.H"

#define COMPRESS_HEADER_TAG           (*((T_word32 *)"CmpH"))

#define UNCOMPRESS_HEADER_TAG         (*((T_word32 *)"uNcH"))

#define COMPRESS_HEADER_DEAD_TAG         (*((T_word32 *)"DcMh"))


#define COMPRESS_NON_REPEAT_FLAG        0x8000
#define COMPRESS_RUN_LENGTH_END_MARK        0xFFFF

/* 8 bytes are added to the front of the compressed block for verification. */
typedef struct {
    T_word32 tag ;
    T_word32 fullsize ;
    T_byte8 data[] ;
} T_compressHeader ;

/****************************************************************************/
/*  Routine:  CompressBlock                                                 */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    Compress a block of data using run-length compression.  In some cases,*/
/*  no compression will occur, but this routine will gaurantee at least     */
/*  equal size (with +8 bytes).                                             */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_void *p_block             -- Block to compress                      */
/*                                                                          */
/*    T_word32 size               -- Size of block to compress              */
/*                                                                          */
/*    T_word32 *p_newSize         -- Pointer to new size                    */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_void *                    -- Newly compressed block.                */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  06/25/96  Created                                                */
/*                                                                          */
/****************************************************************************/

#define MODE_COMPRESS_START          0
#define MODE_COMPRESS_START_2        1
#define MODE_COMPRESS_SAME_RUN       2
#define MODE_COMPRESS_DIFF_RUN       3

T_void *CompressBlock(
            T_void *p_block,
            T_word32 size,
            T_word32 *p_newSize)
{
    T_byte8 *p_work ;
    T_byte8 *p_output ;
    T_byte8 *p_pos ;
    T_word32 count = 0 ;
    T_word32 outSize = 0 ;

    T_byte8 mode = MODE_COMPRESS_START ;
    T_byte8 lastByte = 0 ;
    T_byte8 c ;
    T_word16 repeatLen ;
    T_word16 tooBig = 32000 ;
    T_word16 endMark = COMPRESS_RUN_LENGTH_END_MARK ;
    T_word32 maxSize ;
    E_Boolean didCompress = TRUE ;

    T_byte8 *p_nonRepeatStart ;
    T_word16 nonRepeatCount ;

    DebugRoutine("CompressBlock") ;

    p_work = MemAlloc(sizeof(T_compressHeader) + size) ;
    p_output = p_work ;
    maxSize = sizeof(T_compressHeader) + size ;
    DebugCheck(p_work != NULL) ;
    if (p_work)  {
        p_pos = (T_byte8 *)p_block ;
        count = size ;

        /* Skip the header until later. */
        p_output += 8 ;
        outSize += 8 ;

        p_nonRepeatStart = p_pos ;
        nonRepeatCount = 0 ;

        do {
            /* Count a run of characters (usually one character) */
            lastByte = *(p_pos++) ;
            count-- ;
            repeatLen = 1 ;
            while (count)  {
                c = *p_pos ;
                if (c != lastByte)
                    break ;
                lastByte = c ;
                p_pos++ ;
                count-- ;
                repeatLen++ ;

                /* Stop if the length risks being too long. */
                if (repeatLen >= 32000)
                    break ;
            }

            if ((repeatLen > 3) || (count == 0))  {
                /* We found a repeating group of more than 3. */
                /* First save out the non-repeating part that was */
                /* before this, then save out the repeating part. */

                /* Save out non-repeating part. */
                if (nonRepeatCount != 0)  {
                    outSize += 2 + nonRepeatCount ;
                    if (outSize > maxSize)  {
                        didCompress = FALSE ;
                        break ;
                    }

                    /* Store the number of non-repeats. */
                    nonRepeatCount |= COMPRESS_NON_REPEAT_FLAG ;
                    memcpy(p_output, &nonRepeatCount, sizeof(nonRepeatCount)) ;
                    nonRepeatCount &= (~COMPRESS_NON_REPEAT_FLAG) ;

                    p_output+=2 ;
                    /* Store the non-repeats. */
                    memcpy(p_output, p_nonRepeatStart, nonRepeatCount) ;

                    p_output += nonRepeatCount ;
                }

                /* Write out the repeat info. */
                if (repeatLen != 0)  {
                    outSize += sizeof(repeatLen) ;
                    outSize += sizeof(lastByte) ;
                    if (outSize > maxSize)  {
                        didCompress = FALSE ;
                        break ;
                    }
                    memcpy(p_output, &repeatLen, sizeof(repeatLen)) ;
                    p_output += sizeof(repeatLen) ;
                    memcpy(p_output, &lastByte, sizeof(lastByte)) ;
                    p_output += sizeof(lastByte) ;
                }

                /* Start the non-repeat area again. */
                nonRepeatCount = 0 ;
                p_nonRepeatStart = p_pos ;
            } else {
                /* Must not be repeating.  Add in the size. */
                nonRepeatCount += repeatLen ;

                /* If the repeat len is too big, we need to output. */
                if (nonRepeatCount > 32000)  {
                    outSize += sizeof(tooBig) + tooBig ;
                    if (outSize > maxSize)  {
                        didCompress = FALSE ;
                        break ;
                    }
                    /* Store the number of non-repeats. */
                    tooBig |= COMPRESS_NON_REPEAT_FLAG ;
                    memcpy(p_output, &tooBig, sizeof(tooBig)) ;
                    tooBig &= (~COMPRESS_NON_REPEAT_FLAG) ;

                    p_output+=sizeof(tooBig) ;

                    /* Store the non-repeats. */
                    memcpy(p_output, p_nonRepeatStart, tooBig) ;

                    p_nonRepeatStart += tooBig ;
                    nonRepeatCount -= tooBig ;
                    p_output += tooBig ;
                }
            }
        } while (count > 0)  ;

        outSize += sizeof(endMark) ;
        if (outSize > maxSize)  {
            didCompress = FALSE ;
        } else {
            /* Output the run-length end mark. */
            memcpy(p_output, &endMark, sizeof(endMark)) ;
            p_output += sizeof(endMark) ;
        }

        *p_newSize = outSize ;

        /* Create and store the header. */
        ((T_compressHeader *)p_work)->fullsize = size ;
        ((T_compressHeader *)p_work)->tag = COMPRESS_HEADER_TAG ;

        /* Did actually compress any? */
        if (didCompress == FALSE)  {
            /* No, it is either the same size or bigger. */
            /* Just change the header over to say non-compressed and */
            /* copy over the non-compressed data. */
            ((T_compressHeader *)p_work)->tag = UNCOMPRESS_HEADER_TAG ;
            memcpy(
                p_work+sizeof(T_compressHeader),
                p_block,
                size) ;
            *p_newSize = size + sizeof(T_compressHeader) ;
        }
    } else {
        *p_newSize = 0 ;
    }

    DebugEnd() ;

    return p_work ;
}


T_void *UncompressBlock(
            T_void *p_block,
            T_word32 size,
            T_word32 *p_newSize)
{
    T_byte8 *p_work = NULL ;
    T_byte8 *p_pos ;
    T_byte8 *p_from ;
    T_compressHeader *p_header ;
    T_word16 key ;
    T_word32 outSize = 0 ;
    T_byte8 c ;

    DebugRoutine("UncompressBlock") ;
    DebugCheck(p_block != NULL) ;

    if (p_block != NULL)  {
        p_header = ((T_compressHeader *)p_block) ;
        if (p_header->tag == COMPRESS_HEADER_TAG)  {
            /* This is block is compressed with run length compression. */
            /* Allocate memory and uncompress the block. */
            p_work = MemAlloc(p_header->fullsize) ;
            DebugCheck(p_work != NULL) ;
            if (p_work)  {
                p_pos = p_work ;
                p_from = p_header->data ;

                while (outSize < p_header->fullsize)  {
                    key = *((T_word16 *)p_from) ;
                    p_from += sizeof(T_word16) ;

                    /* Compressed or uncompressed run? */
                    if (key & COMPRESS_NON_REPEAT_FLAG)  {
                        key &= (~COMPRESS_NON_REPEAT_FLAG) ;
                        /* Not compressed, raw data follows. */
                        outSize += key ;
                        DebugCheck(outSize <= p_header->fullsize) ;
                        memcpy(p_pos, p_from, key) ;
                        p_pos += key ;
                        p_from += key ;
                    } else {
                        /* Data stream is compressed as repeats. */
                        c = *p_from ;
                        p_from++ ;
                        outSize += key ;
                        DebugCheck(outSize <= p_header->fullsize) ;
                        memset(p_pos, c, key) ;
                        p_pos += key ;
                    }
                }
            }
        } else if (p_header->tag == UNCOMPRESS_HEADER_TAG)  {
            /* This is the simple case.  Compression could not be */
            /* done because the compressed size was bigger.  Therefore, */
            /* the data was just stored in a raw format that */
            /* we copy out. */
            p_work = MemAlloc(p_header->fullsize) ;
            DebugCheck(p_work != NULL) ;
            if (p_work)  {
                /* Copy over the raw data. */
                memcpy(p_work, p_header->data, p_header->fullsize) ;
                outSize = p_header->fullsize ;
            }
        } else {
            /* Unknown type of un/compressed block. */
            DebugCheck(FALSE) ;
        }
    }

    *p_newSize = outSize ;

    DebugEnd() ;

    return p_work ;
}


/****************************************************************************/
/*    END OF FILE:  COMPRESS.H                                                */
/****************************************************************************/
