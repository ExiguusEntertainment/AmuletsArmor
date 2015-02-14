/****************************************************************************/
/*    FILE:  RESOURCE.H                                                     */
/****************************************************************************/

#ifndef _RESOURCE_H_
#define _RESOURCE_H_

#include <stdio.h>
#include "GENERAL.H"

#define RESOURCE_BAD NULL
#define RESOURCE_FILE_BAD 0

typedef T_void *T_resourceFile ;

typedef T_void *T_resource ;

T_resourceFile ResourceOpen(const char *p_filename) ;

T_void ResourceClose(T_resourceFile resourceFile) ;

T_resource ResourceFind(T_resourceFile resourceFile, const char *p_resourceName) ;

T_void ResourceUnfind(T_resource res) ;

T_void *ResourceLock(T_resource resource) ;

T_void ResourceUnlock(T_resource resource) ;

T_word32 ResourceGetSize(T_resource resource) ;

T_byte8 *ResourceGetName(T_void *p_data) ;

#ifndef NDEBUG
T_void ResourceDumpIndex(T_resourceFile resourceFile) ;
T_void ResourcePrint(FILE *fp, T_resource res) ;
T_void ResourceCheckByPtr(T_byte8 *p_resData) ;
#else
#define ResourceDumpIndex()
#define ResourcePrint(fp, res)
#define ResourceCheckByPtr(p_resData)
#endif

#endif

/****************************************************************************/
/*    END OF FILE:  RESOURCE.H                                              */
/****************************************************************************/
