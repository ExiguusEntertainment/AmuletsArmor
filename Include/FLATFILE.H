/****************************************************************************/
/*    FILE:  FLATFILE.H                                                     */
/****************************************************************************/
#ifndef _FLATFILE_H_
#define _FLATFILE_H_

#include "GENERAL.H"

typedef T_void *T_flatFile ;
#define FLATFILE_BAD NULL

typedef T_word32 T_flatFileIndex ;
#define FLATFILE_INDEX_BAD 0

#define FLATFILE_OPTIONAL_HEADER_MAX_SIZE 256

#define FlatFileExist(filename)  (FileExist(filename))

T_flatFile FlatFileOpenOrCreate(
               T_byte8 *filename, 
               T_word32 entrySize) ;

T_void FlatFileClose(T_flatFile flatFile) ;

T_flatFileIndex FlatFileCreateRecord(T_flatFile flatFile) ;

T_void *FlatFileGetRecord(
            T_flatFile flatFile, 
            T_flatFileIndex index) ;

T_void FlatFilePutRecord(
           T_flatFile flatFile, 
           T_flatFileIndex index, 
           T_void *p_record) ;

T_void FlatFileDeleteRecord(
           T_flatFile flatFile,
           T_flatFileIndex index) ;

T_void FlatFileRefresh(T_flatFile flatFile) ;

T_void *FlatFileGetOptionalHeader(T_flatFile flatFile) ;

T_void FlatFileMarkDirty(T_flatFile flatFile) ;

#endif // _FLATFILE_H_

/****************************************************************************/
/*    END OF FILE: FLATFILE.H                                               */
/****************************************************************************/

