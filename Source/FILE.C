/****************************************************************************/
/*    FILE:  FILE.C                                                         */
/****************************************************************************/
#include <fcntl.h>
#include <sys\stat.h>
#include <sys\types.h>
#include <io.h>
#include "FILE.H"
#include "MEMORY.H"
#include "SOUND.H"

#define MAX_FILES 20

/* Number of files currently open: */
static T_word16 G_numberOpenFiles = 0 ;

/****************************************************************************/
/*  Routine:  FileOpen                                                      */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    Open a file for reading, writing, appending, etc.  All files are      */
/*  created unless you request to read.  A file handle is returned for      */
/*  all future accesses.  Note that a maximum of MAX_FILES is allowed to    */
/*  be opened at a time.                                                    */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    Obviously I can't check to see if someone does something stupid to    */
/*  a file they shouldn't be touching, but there is always the possibility. */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_byte8 *p_filename         -- pointer to the string that holds       */
/*                                   the real filename.  Note that we       */
/*                                   don't have any particular format       */
/*                                   in mind.  A path name can be included. */
/*                                                                          */
/*    E_fileMode                  -- Different read/write modes.  See .H    */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_file                      -- file handle for all future accesses.   */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    open                                                                  */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  11/16/94  Created                                                */
/*                                                                          */
/****************************************************************************/

T_file FileOpen(T_byte8 *p_filename, E_fileMode mode)
{
    T_file file ;
    static T_word32 fileOpenModes[4] = {
         O_RDONLY|O_BINARY,
         O_WRONLY|O_CREAT|O_BINARY,
         O_RDWR|O_APPEND|O_CREAT|O_BINARY,
         O_RDWR|O_CREAT|O_BINARY
    } ;

    DebugRoutine("FileOpen") ;
    DebugCheck(p_filename != NULL) ;
    DebugCheck(mode < FILE_MODE_UNKNOWN) ;
    DebugCheck(G_numberOpenFiles < MAX_FILES) ;

    file = open(p_filename, fileOpenModes[mode], S_IREAD|S_IWRITE) ;
    if (file != FILE_BAD)
        G_numberOpenFiles++ ;

    DebugEnd() ;

    return file ;
}

/****************************************************************************/
/*  Routine:  FileClose                                                     */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    Close a previously opened file.  Nothing really special here.         */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_file file                 -- file to close.                         */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    close                                                                 */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  11/16/94  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void FileClose(T_file file)
{
    DebugRoutine("FileClose") ;
    DebugCheck(file != FILE_BAD) ;

    close(file) ;

    /* Decrement the number of open files. */
    G_numberOpenFiles-- ;

    DebugEnd();
}

/****************************************************************************/
/*  Routine:  FileSeek                                                      */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    Perhaps one of the most useful file routines is the file seek         */
/*  function.  Just provide the file to seek into and you will be           */
/*  position at the point you requested.                                    */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    Doesn't check to see if you stayed inside the file bounds.  This is   */
/*  not really a problem for writing, but can be a big problem for reading. */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_file file                 -- File to seek into                      */
/*                                                                          */
/*    T_word32 position           -- position to seek from the beginning.   */
/*                                   A position of 0 is the very first      */
/*                                   byte.                                  */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    Nothing.                                                              */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    lseek                                                                 */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  11/16/94  Created                                                */
/*                                                                          */
/****************************************************************************/

/* All seeks are from the beginning of the file. */
T_void FileSeek(T_file file, T_word32 position)
{
    DebugRoutine("FileSeek") ;
    DebugCheck(file != FILE_BAD) ;

    lseek(file, position, SEEK_SET) ;

    DebugEnd() ;
}

/****************************************************************************/
/*  Routine:  FileRead                                                      */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    FileRead is used to retrieve bytes from a file from the current       */
/*  file position.                                                          */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    There is no way to check if the buffer pointer that is passed has     */
/*  enough room for the data is about to be read and may overwrite a bunch  */
/*  of stuff that is valuable (including the OS).                           */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_file file                 -- handle of file to read from.           */
/*                                                                          */
/*    T_byte8 *p_buffer           -- Pointer to buffer to read bytes into.  */
/*                                                                          */
/*    T_sword32 size              -- number of bytes to read.               */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_sword32                   -- number of bytes read, or -1 for error. */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    read                                                                  */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  11/16/94  Created                                                */
/*                                                                          */
/****************************************************************************/

T_sword32 FileRead(T_file file, T_void *p_buffer, T_word32 size)
{
    T_sword32 result ;

    DebugRoutine("FileRead") ;
    DebugCheck(file != FILE_BAD) ;
//    DebugCheck(size > 0) ;
    DebugCheck(p_buffer != NULL) ;

    SoundUpdateOften() ;
    result = read(file, p_buffer, size) ;
    SoundUpdateOften() ;

    DebugEnd() ;

    return result ;
}

/****************************************************************************/
/*  Routine:  FileWrite                                                     */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    Use FileWrite to store bytes at the current file position.  When the  */
/*  writing is done, the current file position will be at the next byte     */
/*  after all of the writing.                                               */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    This routine doesn't check to see if we have a file handle that is    */
/*  for writing.  You could get some weird errors if this happens.          */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_file file                 -- handle of file to write to.            */
/*                                                                          */
/*    T_byte8 *p_buffer           -- Pointer to buffer to write bytes from. */
/*                                                                          */
/*    T_sword32 size              -- number of bytes to write.              */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_sword32                   -- number of bytes written, or else -1.   */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    write                                                                 */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  11/16/94  Created                                                */
/*                                                                          */
/****************************************************************************/

T_sword32 FileWrite(T_file file, T_void *p_buffer, T_word32 size)
{
    T_sword32 result ;

    DebugRoutine("FileWrite") ;
    DebugCheck(file != FILE_BAD) ;
    DebugCheck(size > 0) ;
    DebugCheck(p_buffer != NULL) ;

    result = write(file, p_buffer, size) ;

    DebugEnd() ;

    return result ;
}

/****************************************************************************/
/*  Routine:  FileLoad                                                      */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    FileLoad allocates and reads in a file in one swipe so that the       */
/*  calling routine can just use the file like a memory allocation.         */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_byte8 *p_filename         -- File to load                           */
/*                                                                          */
/*    T_word32 *p_size            -- Indirect reference to the size of the  */
/*                                   file.                                  */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    FileOpen                                                              */
/*    FileRead                                                              */
/*    FileGetSize                                                           */
/*    FileClose                                                             */
/*    MemAlloc                                                              */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  12/23/94  Created                                                */
/*                                                                          */
/****************************************************************************/

T_void *FileLoad(T_byte8 *p_filename, T_word32 *p_size)
{
    T_byte8 *p_data ;
    T_file file ;

    DebugRoutine("FileLoad") ;
    DebugCheck(p_filename != NULL) ;
    DebugCheck(p_size != NULL) ;

    /* See how big the file is so we know how much memory to allocate. */
    *p_size = FileGetSize(p_filename) ;
#ifdef COMPILE_OPTION_FILE_OUTPUT
printf("!A 1 file_%s\n", p_filename) ;
printf("!A 1 file_r_%s\n", DebugGetCallerName()) ;
#endif
    if (*p_size)  {
        /* Allocate the memory for the file. */
        p_data = MemAlloc(*p_size) ;

        DebugCheck(p_data != NULL) ;

        /* Make sure we got the memory. */
        if (p_data != NULL)  {
            /* If memory was allocated, read in the file into this memory. */
            file = FileOpen(p_filename, FILE_MODE_READ) ;
            FileRead(file, p_data, *p_size) ;
            FileClose(file) ;
        } else {
            /* If memory was not allocated, return with a zero length. */
            *p_size = 0 ;
        }
    } else {
        *p_size = 0 ;
        p_data = NULL ;
    }

    DebugEnd() ;

    /* Return the pointer to the data. */
    return p_data ;
}

/****************************************************************************/
/*  Routine:  FileGetSize                                                   */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    FileGetSize looks at a given file name and returns the size of that   */
/*  file.                                                                   */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    NOTE:  This is the WATCOM C/C++ v10.0 specific version.               */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_byte8 *p_filename         -- File to get size of                    */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    T_word32                    -- Size of file                           */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    _dos_findfirst                                                        */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  12/23/94  Created                                                */
/*                                                                          */
/****************************************************************************/

T_word32 FileGetSize(T_byte8 *p_filename)
{
    T_word32 size ;
#if defined(WIN32)
    FILE *fp;

    DebugRoutine("FileGetSize");
    fp = fopen(p_filename, "rb");
    size = filelength(fileno(fp));
    fclose(fp);
    DebugEnd() ;
#else
    struct find_t fileinfo ;

    DebugRoutine("FileGetSize") ;

    /* Get information about the file. */
    if (_dos_findfirst(p_filename, _A_NORMAL, &fileinfo) == 0)  {
        /* If we found the file, return the file size. */
        size = fileinfo.size ;
    } else {
        /* If we didn't find the file, return a zero. */
        size = 0 ;
    }

    DebugEnd() ;
#endif

    return size ;
}

/****************************************************************************/
/*  Routine:  FileExist                                                     */
/****************************************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*    FileExist checks to see if a file exists and returns TRUE if it does. */
/*                                                                          */
/*                                                                          */
/*  Problems:                                                               */
/*                                                                          */
/*    None.                                                                 */
/*                                                                          */
/*                                                                          */
/*  Inputs:                                                                 */
/*                                                                          */
/*    T_byte8 *p_filename         -- File to check size of                  */
/*                                                                          */
/*                                                                          */
/*  Outputs:                                                                */
/*                                                                          */
/*    E_Boolean                   -- TRUE=file exists, else FALSE           */
/*                                                                          */
/*                                                                          */
/*  Calls:                                                                  */
/*                                                                          */
/*    FileOpen                                                              */
/*    FileClose                                                             */
/*                                                                          */
/*                                                                          */
/*  Revision History:                                                       */
/*                                                                          */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    LES  01/14/96  Created                                                */
/*                                                                          */
/****************************************************************************/

E_Boolean FileExist(T_byte8 *p_filename)
{
    E_Boolean fileFound = FALSE ;
    T_file file ;

    DebugRoutine("FileExist") ;
    DebugCheck(p_filename != NULL) ;

    file = FileOpen(p_filename, FILE_MODE_READ) ;
    if (file != FILE_BAD)  {
        fileFound = TRUE ;
        FileClose(file) ;
    }

    DebugEnd() ;

    return fileFound ;
}

/****************************************************************************/
/*    END OF FILE:  FILE.C                                                  */
/****************************************************************************/

