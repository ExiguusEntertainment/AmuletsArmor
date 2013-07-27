/****************************************************************************/
/*    FILE:  IRESOURC.H                                                     */
/****************************************************************************/
#ifndef _IRESOURC_H_
#define _IRESOURC_H_

#include "FILE.H"
#include "GENERAL.H"

#define MAX_RESOURCE_FILES 10

#define RESOURCE_ENTRY_TYPE_MEMORY     0x00
#define RESOURCE_ENTRY_TYPE_DISK       0x01
#define RESOURCE_ENTRY_TYPE_DISCARDED  0x02

#ifndef NDEBUG
#define RESOURCE_ENTRY_TYPE_FILE_LOADED 0x80
#endif

#define RESOURCE_ENTRY_TYPE_FILE       0x00
#define RESOURCE_ENTRY_TYPE_DIRECTORY  0x10
#define RESOURCE_ENTRY_TYPE_LINK       0x20
#define RESOURCE_ENTRY_TYPE_APPEND     0x30

#define RESOURCE_ENTRY_TYPE_UNKNOWN    0xC3

#define RESOURCE_ENTRY_TYPE_MASK_WHERE    0x0F
#define RESOURCE_ENTRY_TYPE_MASK_TYPE     0xF0


#define RESOURCE_FILE_UNIQUE_ID (*((T_word32 *)("Res!")))

typedef struct {
    T_byte8 resID[4]            PACK; /* Should contain "ReS"+'\0' id */
    T_byte8 p_resourceName[14]  PACK; /* Case sensitive, 13 characters + '\0' */
    T_word32 fileOffset         PACK;
    T_word32 size               PACK; /* Size in bytes. */
    T_word16 lockCount          PACK; /* 0 = unlocked. */
    T_byte8 resourceType        PACK;
    T_byte8 *p_data             PACK;
    T_resourceFile resourceFile PACK; /* Resource file this is from. */
    T_void *ownerDir            PACK; /* Locked in owner directory (or NULL) */
} T_resourceEntry ;

/* General information stored for each resource directory in the system. */
typedef struct {
    T_file fileHandle           PACK;
    T_word16 numberEntries      PACK;
    T_resourceEntry *p_entries  PACK;
    T_resource ownerRes         PACK;
    T_sword16 nextResource      PACK;
} T_resourceDirInfo ;

/* Header that is placed at the beginning of the file to locate */
/* the resource index. */
typedef struct {
    T_word32 uniqueID           PACK;
    T_word32 indexOffset        PACK;
    T_word32 indexSize          PACK; /* Size of index in bytes. */
    T_word16 numEntries         PACK; /* Number of entries in the index. */
} T_resourceFileHeader ;

#endif // _IRESOURC_H_

/****************************************************************************/
/*    End of FILE:  IRESOURC.H                                              */
/****************************************************************************/

