/****************************************************************************/
/*    FILE:  files.c                                                        */
/****************************************************************************/
#include "CMDQUEUE.H"
#include "FILES.H"
#include "FILETRAN.H"
#include "SCHEDULE.H"

typedef struct
{
   T_word32 filenumber;
   T_word32 revlevel;
} T_fileInfo;

E_Boolean G_receivedFileFlag = FALSE;

/** Internal function prototypes. **/
T_void IFilesReceivedFile (T_word32 filename, E_fileTransferStatus status);

/****************************************************************************/
/*  Routine:  IFilesReceivedFile                                            */
/****************************************************************************/
/*  Description:                                                            */
/*    Internal: Callback used to notify me of a change in file transfer     */
/*  status (completed or aborted)                                           */
/*                                                                          */
/*  Problems:                                                               */
/*    None.                                                                 */
/*                                                                          */
/*  Inputs:                                                                 */
/*    T_word32 filename           -- Filename of file that was requested.   */
/*    E_fileTransferStatus status -- Reason for callback.                   */
/*                                                                          */
/*  Outputs:                                                                */
/*    None.                                                                 */
/*                                                                          */
/*  Revision History:                                                       */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    AMT  08/31/95  Created                                                */
/****************************************************************************/
T_void IFilesReceivedFile (T_word32 filename, E_fileTransferStatus status)
{
   DebugRoutine ("IFilesReceivedFile");

   G_receivedFileFlag = TRUE;

   DebugEnd ();
}

/****************************************************************************/
/*  Routine:  FilesObtainFile                                               */
/****************************************************************************/
/*  Description:                                                            */
/*    Takes a filename, checks to see if it exist on this system (with      */
/*  the given revlevel), and if it doesn't, downloads it from the server.   */
/*                                                                          */
/*  Problems:                                                               */
/*    None.                                                                 */
/*                                                                          */
/*  Inputs:                                                                 */
/*    T_word32 filename           -- Internal "filename"                    */
/*    T_word32 revlevel           -- Revision level to require.             */
/*                                                                          */
/*  Outputs:                                                                */
/*    T_void *                    -- Pointer to loaded file.                */
/*                                                                          */
/*  Revision History:                                                       */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    AMT  08/30/95  Created                                                */
/****************************************************************************/
T_void FilesObtainFile (T_word32 filename, T_word32 revlevel)
{
   DebugRoutine ("FilesObtainFile");

printf ("FilesObtainFile: %d, %d\n", filename, revlevel);

   if (FilesGetRevlevel (filename) < revlevel)
   {
/** The client can always download from the server... **/
#ifndef SERVER_ONLY
puts ("Need to download");

      /** Need to download! **/
      G_receivedFileFlag = FALSE;

      DataPacketRequestFile (filename, revlevel, IFilesReceivedFile);

      while (G_receivedFileFlag == FALSE)
      {
         /** While waiting for the file, make sure the packets are **/
         /** being received... **/
         CmdQUpdateAllSends ();
         ScheduleUpdateEvents ();
         CmdQUpdateAllReceives ();
      }

#else
/** ... but the server can't do anything. **/
      printf ("Error!  Attempted to obtain file #%d rev %d, does not exist.\n",
                filename, revlevel);
      DebugCheck (FALSE);
      exit (0);

#endif /** SERVER_ONLY **/
   }

   DebugEnd ();
}

/****************************************************************************/
/*  Routine:  FilesLoadFile                                                 */
/****************************************************************************/
/*  Description:                                                            */
/*    Like FileLoad, but takes a T_word32 FILES-style name rather than an   */
/*  actual filename.                                                        */
/*                                                                          */
/*  Problems:                                                               */
/*    None.                                                                 */
/*                                                                          */
/*  Inputs:                                                                 */
/*    T_word32 filename           -- Internal "filename"                    */
/*    T_word32 revlevel           -- Revision level to require.             */
/*    T_word32 *actual_revlevel   -- Place to put the actual revlevel.      */
/*    T_word32 *size              -- Place to put the size of the file      */
/*                                                                          */
/*  Outputs:                                                                */
/*    T_void *                    -- Pointer to loaded file.                */
/*                                                                          */
/*  Revision History:                                                       */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    AMT  08/30/95  Created                                                */
/****************************************************************************/
T_void *FilesLoadFile (T_word32 filename, T_word32 revlevel,
                       T_word32 *actual_revlevel, T_word32 *size)
{
   T_void *data;
   char realname[256];

   DebugRoutine ("FilesLoadFile");

   /** Ensure that it exists, and meets the revlevel. **/
   if ((*actual_revlevel = FilesGetRevlevel (filename)) < revlevel)
   {
      /** Doesn't exist, or fails to meet the revlevel. **/
      data = NULL;
      *size = 0;
   }
   else
   {
      /** Exists. Load the data file. **/
      /** Construct the real filename. **/
#ifdef TARGET_UNIX
      sprintf (realname, "_Files/f%07d.d", filename);
#else
      sprintf (realname, "_Files\\f%07d.d", filename);
#endif /** TARGET_UNIX **/

      data = FileLoad (realname, size);
   }

   DebugEnd ();

   return data;
}

/****************************************************************************/
/*  Routine:  FilesOpenFile                                                 */
/****************************************************************************/
/*  Description:                                                            */
/*    Like FileOpen, but takes a T_word32 FILES-style name rather than an   */
/*  actual filename.                                                        */
/*                                                                          */
/*  Problems:                                                               */
/*    None.                                                                 */
/*                                                                          */
/*  Inputs:                                                                 */
/*    T_word32 filename           -- Internal "filename"                    */
/*    T_word32 revlevel           -- Revision level to require.             */
/*    E_fileMode mode             -- Mode to open the file with.            */
/*                                                                          */
/*  Outputs:                                                                */
/*    T_file                      -- Opened file.                           */
/*                                                                          */
/*  Revision History:                                                       */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    AMT  08/29/95  Created                                                */
/****************************************************************************/
T_file FilesOpenFile (T_word32 filename, T_word32 revlevel, E_fileMode mode)
{
   T_file result;
   char realname[256];
   T_word32 revision;

   DebugRoutine ("FilesOpenFile");
   /** No writing! **/
   DebugCheck (mode != FILE_MODE_WRITE);

   /** Ensure that it exists, and meets the revlevel. **/
   if ((revision = FilesGetRevlevel (filename)) == 0)
   {
      /** Doesn't exist. **/
      result = FILE_BAD;
   }
   else
   {
      /** Now, open the data file. **/

      /** Construct the real filename. **/
#ifdef TARGET_UNIX
      sprintf (realname, "_Files/f%07d.d", filename);
#else
      sprintf (realname, "_Files\\f%07d.d", filename);
#endif /** TARGET_UNIX **/

      if (revision >= revlevel)
         result = FileOpen (realname, mode);
      else
         result = FILE_BAD;
   }

   DebugEnd ();

   return result;
}

/****************************************************************************/
/*  Routine:  FilesCreateFileWithRevlevel                                   */
/****************************************************************************/
/*  Description:                                                            */
/*    Given a filename, creates a new file with the given name and revlevel.*/
/*                                                                          */
/*  Problems:                                                               */
/*    None.                                                                 */
/*                                                                          */
/*  Inputs:                                                                 */
/*    T_word32 filename           -- Internal "filename"                    */
/*    T_word32 revlevel           -- Revision level to set.                 */
/*                                                                          */
/*  Outputs:                                                                */
/*    T_file                      -- File descriptor for writing.           */
/*                                                                          */
/*  Revision History:                                                       */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    AMT  08/29/95  Created                                                */
/****************************************************************************/
T_file FilesCreateFileWithRevlevel (T_word32 filename, T_word32 revlevel)
{
   T_file file, result;
   char realname[256];
   T_fileInfo info;

   DebugRoutine ("FilesCreateFile");

   /** Construct the real filename. **/
#ifdef TARGET_UNIX
   sprintf (realname, "_Files/f%07d", filename);
#else
   sprintf (realname, "_Files\\f%07d", filename);
#endif /** TARGET_UNIX **/

   /** Fill in a new info struct. **/
   info.filenumber = filename;
   info.revlevel = revlevel;

   /** Create the new info (.i) file. **/
printf ("Creating new info file '%s'\n", realname);

   strcat (realname, ".i");
   file = FileOpen (realname, FILE_MODE_WRITE);
   DebugCheck (file != FILE_BAD);

   /** Put the info in the info file. **/
   FileWrite (file, &info, sizeof (T_fileInfo));

   FileClose (file);

   /** Now, open the data file. **/
   strcpy (strchr (realname, '.'), ".d");
   result = FileOpen (realname, FILE_MODE_WRITE);

   DebugEnd ();

   return result;

}

/****************************************************************************/
/*  Routine:  FilesCreateNewFile                                            */
/****************************************************************************/
/*  Description:                                                            */
/*    Given a filename, creates a new file with the given name, and revlevel*/
/*  of 1.                                                                   */
/*                                                                          */
/*  Problems:                                                               */
/*    None.                                                                 */
/*                                                                          */
/*  Inputs:                                                                 */
/*    T_word32 filename           -- Internal "filename"                    */
/*                                                                          */
/*  Outputs:                                                                */
/*    T_file                      -- File descriptor for writing.           */
/*                                                                          */
/*  Revision History:                                                       */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    AMT  08/29/95  Created                                                */
/****************************************************************************/
T_file FilesCreateNewFile (T_word32 filename)
{
   T_file file;

   DebugRoutine ("FilesCreateNewFile");

   file = FilesCreateFileWithRevlevel (filename, 1);

   DebugEnd ();

   return file;
}

/****************************************************************************/
/*  Routine:  FilesGetRevlevel                                              */
/****************************************************************************/
/*  Description:                                                            */
/*    Given a filename, checks that file's existence and revlevel.          */
/*                                                                          */
/*  Problems:                                                               */
/*    None.                                                                 */
/*                                                                          */
/*  Inputs:                                                                 */
/*    T_word32 filename           -- Internal "filename"                    */
/*                                                                          */
/*  Outputs:                                                                */
/*    T_word32                    -- Revlevel (or 0 for DNE)                */
/*                                                                          */
/*  Revision History:                                                       */
/*    Who  Date:     Comments:                                              */
/*    ---  --------  ---------                                              */
/*    AMT  08/29/95  Created                                                */
/****************************************************************************/
T_word32 FilesGetRevlevel (T_word32 filename)
{
   T_file file;
   T_byte8 realname[255];
   T_fileInfo info;
   T_word32 result;

   DebugRoutine ("FilesGetRevlevel");

   /** Construct the real filename. **/
#ifdef TARGET_UNIX
   sprintf (realname, "_Files/f%07d.i", filename);
#else
   sprintf (realname, "_Files\\f%07d.i", filename);
#endif /** TARGET_UNIX **/

   /** Read the info file. **/
   file = FileOpen (realname, FILE_MODE_READ);

   if (file == FILE_BAD)
   {
      result = 0;
   } 
   else
   {
      FileRead (file, &info, sizeof (T_fileInfo));
      FileClose (file);

      result = info.revlevel;
   }

   DebugEnd ();

   return result;

}

/****************************************************************************/
/*    END OF FILE:  files.c                                                 */
/****************************************************************************/

