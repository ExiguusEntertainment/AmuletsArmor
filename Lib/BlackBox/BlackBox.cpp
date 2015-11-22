// BlackBox.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "BlackBox.h"
#include "Internal.h"
#include "BugSlayerUtil.h"
#include "BlackBoxUI.h"

static HINSTANCE g_hInst = NULL ;

LONG __stdcall BlackBoxCrashHandlerFilter ( EXCEPTION_POINTERS * pExPtrs );

const char* g_errorLable = 
					"A crash has been detected by BlackBox\n\n"\
					"To help the development process, this program will try "\
					"and gather as much information about the crash, and the state "\
					"of your machine at the time of the crash. This data will can then "\
					"be either copied to the clipboard or saved to a filed as plain text.\n"\
					"You can then either email to ddiego@users.sourceforge.net or submit "\
					"a new bug (http://sourceforge.net/tracker/?func=add&group_id=6796&atid=106796)"\
					" and paste the information into the Detailed Description field";


const char* g_mailToAddress = "mailto:ddiego@users.sourceforge.net";

const char* g_submitBugURL = "http://sourceforge.net/tracker/?func=add&group_id=6796&atid=106796";

BOOL APIENTRY DllMain ( HINSTANCE hInst       ,
                      DWORD     dwReason    ,
                      LPVOID    )
{
    BOOL bRet = TRUE ;	

    switch ( dwReason )   {

	    case DLL_PROCESS_ATTACH : {

			// Save off the DLL hInstance.
			g_hInst = hInst ;
			// I don't need the thread notifications.
			DisableThreadLibraryCalls ( g_hInst ) ;

#ifdef _DEBUG
		bRet = InternalMemStressInitialize ( ) ;
#endif
			
		if ( SetCrashHandlerFilter( BlackBoxCrashHandlerFilter ) ) {
			OutputDebugString( "SetCrashHandlerFilter succeeded\n" );			
		}
		else {
			OutputDebugString( "SetCrashHandlerFilter failed\n" );
		}

		}
		break ;		
		
		case DLL_PROCESS_DETACH : {
		
#ifdef _DEBUG
		bRet = InternalMemStressShutdown ( ) ;
#endif	
		}
		break ;

		default : {

		}
		break ;
    }
    return ( bRet ) ;
}



LONG __stdcall BlackBoxCrashHandlerFilter ( EXCEPTION_POINTERS * pExPtrs )
{
	ShowBlackBoxUI( pExPtrs, g_hInst );
	
	return EXCEPTION_EXECUTE_HANDLER;
}