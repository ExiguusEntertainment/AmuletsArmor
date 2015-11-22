
#include "stdafx.h"
#include "BlackBoxUI.h"
#include "BugSlayerUtil.h"
#include "resource.h"
#include "commdlg.h"
#include <winperf.h>
#include <SHELLAPI.H>




LRESULT CALLBACK InitBlackBoxUI_DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK BlackBoxUI_DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK AboutBlackBox_DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK MachineInfo_DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK MachineState_DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);


HGLOBAL GenerateRawReport();

bool OnSendData( HWND hDlg );

bool OnCopyToClipboardData( HWND hDlg );

bool OnSaveDataToFile( HWND hDlg );

#define WM_UpdateInitDlgProgress	WM_USER + 120

static HWND g_initDlg = NULL;


void UpdateInitDlgProgress( int progress )
{
	SendMessage( g_initDlg,WM_UpdateInitDlgProgress, 0, progress ); 
}




static HINSTANCE g_BlackBoxUIHInstance = NULL;

static EXCEPTION_POINTERS* g_BlackBoxUIExPtrs = NULL;

static SYSTEM_INFO g_sysInfo = {0};
static OSVERSIONINFO g_OSVersion = {0};
static MEMORYSTATUS g_sysMemoryStatus = {0};


#define MAX_GLOBAL_PROCESS_COUNT	100
#define SHORT_STRING_LEN			256

static DWORD g_systemProcessList[MAX_GLOBAL_PROCESS_COUNT] = {0};
static DWORD g_systemProcessListCount = 0;
static DWORD g_systemTotalModuleCount = 0;

static char g_saveFilePath[MAX_PATH];
static char* g_errorLogTypes = "Error log files (*.log)\0*.log\0\0";

static char g_systemProcessNameList[MAX_GLOBAL_PROCESS_COUNT][256];

struct ModuleNameInfo {
	char name[256];
	char version[256];
	DWORD index;
};


static ModuleNameInfo* g_processModuleInfoList = NULL;
static HGLOBAL g_procModListHandle = NULL;

//strings;
static char g_sysInfoString[1024];
static char g_memInfoString[1024];
static char g_versionInfoString[1024];

#define MAX_STACK_TRACE	100

static char g_stackTrace[MAX_STACK_TRACE][256];

static int g_stackTraceCount = 0;

static char g_ProcessorTypes[3][50] = { "Intel 386", "Intel 486", "Intel Pentium" };

static char g_windowsOSTypes[6][50] = { "Windows 95", "Windows 98", "Windows ME", "Windows NT", "Windows 2000", "Windows XP" };

static BOOL HasOSInfo = FALSE;

/*
BuildProcessList pulled from
    (c) Copyright 1999, Emmanuel KARTMANN, all rights reserved
	in his BuildProcessList.cpp
*/


#define INITIAL_SIZE        51200
#define EXTEND_SIZE         25600
#define REGKEY_PERF         _T("software\\microsoft\\windows nt\\currentversion\\perflib")
#define REGSUBKEY_COUNTERS  _T("Counters")
#define PROCESS_COUNTER     _T("process")
#define PROCESSID_COUNTER   _T("id process")



void BuildStackTrace()
{
	
	FillMemory( g_stackTrace[0],MAX_STACK_TRACE*256, 0 );

	const TCHAR* traceDump = 
		GetFirstStackTraceString( GSTSO_MODULE | GSTSO_SYMBOL | GSTSO_SRCLINE,
									g_BlackBoxUIExPtrs );
	g_stackTraceCount = 0;

	UpdateInitDlgProgress( 85 );
	int incr = 85;
	while ( NULL != traceDump ) {
		
		lstrcpy( g_stackTrace[g_stackTraceCount], traceDump );		
	
		g_stackTraceCount++;

		UpdateInitDlgProgress( incr );
		incr += 2;
		traceDump = GetNextStackTraceString( GSTSO_MODULE | GSTSO_SYMBOL | GSTSO_SRCLINE,
			g_BlackBoxUIExPtrs );		
	}				
}

void BuildModuleList()
{
    DWORD i;
	UpdateInitDlgProgress( 50 );

	g_systemTotalModuleCount = 0;
	DWORD totMemtoAlloc = 0;
	for (i=0; i<g_systemProcessListCount;i++ ) {
		HANDLE hProc = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, 
									FALSE,
									g_systemProcessList[i] );


		if ( NULL != hProc ) {
			HMODULE pLoadedModules[4096];
			DWORD nLoadedBytes = 0;
			if (EnumProcessModules(hProc,
									pLoadedModules,
									(DWORD)sizeof(pLoadedModules),
									&nLoadedBytes)) {
				
				totMemtoAlloc += (nLoadedBytes/sizeof(HMODULE));
				
			}
			
		}

		CloseHandle( hProc );
	}

	g_procModListHandle = GlobalAlloc( GHND, totMemtoAlloc * sizeof(ModuleNameInfo) );
	g_processModuleInfoList = (ModuleNameInfo*)GlobalLock( g_procModListHandle ); 

	FillMemory( g_processModuleInfoList, totMemtoAlloc * sizeof(ModuleNameInfo), 0  );

	ModuleNameInfo* tmpModuleListPtr = g_processModuleInfoList;

	double div = 0.0;
	double incr = 35.0;
	if ( g_systemProcessListCount ) {
		div = 50.0 / (double)totMemtoAlloc;
	}

	for ( i=0; i<g_systemProcessListCount;i++ ) {
		
		HANDLE hProc = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, 
									FALSE,
									g_systemProcessList[i] );

		char tmp[256];
		wsprintf( tmp, "OpenProcess() returned: %p for PID:%d",hProc,g_systemProcessList[i]);
		//MessageBox( NULL, tmp, "", MB_OK );

		if ( NULL != hProc ) {
			HMODULE pLoadedModules[4096];
			DWORD nLoadedBytes = 0;
			if (EnumProcessModules(hProc,
									pLoadedModules,
									(DWORD)sizeof(pLoadedModules),
									&nLoadedBytes)) {				
				
				int nNumberOfModules = nLoadedBytes/sizeof(HMODULE);
				for (int j=0; j<nNumberOfModules; j++) {
					// Fetch module file name
					char pFileName[MAX_PATH];
					incr += div;

					if (GetModuleFileNameEx(hProc, pLoadedModules[j], 
											pFileName, MAX_PATH ) > 0) {

						tmpModuleListPtr->index = i;
						lstrcpy( tmpModuleListPtr->name, pFileName );						

						tmpModuleListPtr ++;
						
						g_systemTotalModuleCount ++;
					}
					UpdateInitDlgProgress( (int)incr );
				}				
			}
			
		}

		CloseHandle( hProc );
	}
}


void BuildProcessList()
{
	HGLOBAL						bufHandle = NULL;
	bool						 bErrorOccured=false;
	DWORD                        rc = 0;
    HKEY                         hKeyNames = NULL;
    DWORD                        dwType = 0;
    DWORD                        dwSize = 0;
    LPBYTE                       buf = NULL;
    char                        subKey[1024];
    LANGID                       lid;
    LPSTR                        p = NULL;
    LPSTR                        p2 = NULL;
    PPERF_DATA_BLOCK             pPerf = NULL;
    PPERF_OBJECT_TYPE            pObj = NULL;
    PPERF_INSTANCE_DEFINITION    pInst = NULL;
    PPERF_COUNTER_BLOCK          pCounter = NULL;
	PPERF_COUNTER_DEFINITION     pCounterDef = NULL;
    DWORD                        i = 0;
    DWORD                        dwProcessIdTitle = 0; 
    DWORD                        dwProcessIdCounter = 0; 
    TCHAR                        processName[MAX_PATH];
    DWORD                        dwLimit = 256;
	DWORD dwNumTasks = 0;
	double div = 0.0;
	double incr = 10;

	bool continueLoop = true;

    lid = MAKELANGID(LANG_ENGLISH, SUBLANG_NEUTRAL);
    wsprintf( subKey, "%s\\%03x", REGKEY_PERF, lid );	

    rc = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                       subKey,
                       0,
                       KEY_READ,
                       &hKeyNames
                     );
    if (rc != ERROR_SUCCESS)
	{
		bErrorOccured=true;
        goto exit;//ick, ick, ick

    }

	UpdateInitDlgProgress( 2 );

    //
    // get the buffer size for the counter names
    //
    rc = RegQueryValueEx(hKeyNames,
                          REGSUBKEY_COUNTERS,
                          NULL,
                          &dwType,
                          NULL,
                          &dwSize
                        );

    if (rc != ERROR_SUCCESS)
	{
		bErrorOccured=true;
//		m_strLastError=_T("Could not open counter registry key");
        goto exit;
    }
	//
    // allocate the counter names buffer
    //
	bufHandle = GlobalAlloc( GHND, dwSize );
    buf = (LPBYTE) GlobalLock( bufHandle );
    if (buf == NULL)
	{
		bErrorOccured=true;
//		m_strLastError=_T("Out of Memory");
        goto exit;
    }
    FillMemory(buf, dwSize, 0 );

    //
    // read the counter names from the registry
    //
    rc = RegQueryValueEx( hKeyNames,
                          REGSUBKEY_COUNTERS,
                          NULL,
                          &dwType,
                          buf,
                          &dwSize
                        );

    if (rc != ERROR_SUCCESS) 
	{
		bErrorOccured=true;
//		m_strLastError=_T("Could Not Read the counter Names");
        goto exit;
    }

    //
    // now loop thru the counter names looking for the "Process" counters:
    // the buffer contains multiple null terminated strings and then
    // finally null terminated at the end.  the strings are in pairs of
    // counter number and counter name.
    //

    p =(LPSTR) buf;
    while (*p) 	{
        if (p > (LPSTR)buf) {
            for( p2=p-2; _istdigit(*p2); p2--)
						;
        }
        if (stricmp(p, PROCESS_COUNTER) == 0)	{
            // look backwards for the counter number
            for(p2=p-2; _istdigit(*p2); p2--) 
						;
            lstrcpy(subKey, p2+1);			
        } 
		else {
			if (stricmp(p, PROCESSID_COUNTER) == 0) {
				// 
				// look backwards for the counter number
				//
				for( p2=p-2; BlackBox::isdigit(*p2); p2--) 
					; 
				dwProcessIdTitle = BlackBox::atol( p2+1 );				
			}
		}
        //
		// next string
		// 
        p += (_tcslen(p) + 1);
    }

	UpdateInitDlgProgress( 10 );

    // free the counter names buffer
	GlobalUnlock( bufHandle );
	GlobalFree( bufHandle );


    // allocate the initial buffer for the performance data

    dwSize = INITIAL_SIZE;

	bufHandle = GlobalAlloc( GHND, dwSize );


    buf = (LPBYTE)GlobalLock( bufHandle );
    if (buf == NULL) {
		bErrorOccured=true;
//		m_strLastError=_T("Out of Memory");
        goto exit;
    }
    FillMemory(buf, dwSize, 0);
	
    while (continueLoop) {

        rc = RegQueryValueEx( HKEY_PERFORMANCE_DATA,
                              subKey,
                              NULL,
                              &dwType,
                              buf,
                              &dwSize );

        pPerf = (PPERF_DATA_BLOCK) buf;

        // check for success and valid perf data block signature

        if ((rc == ERROR_SUCCESS) &&
            (dwSize > 0) &&
            (pPerf)->Signature[0] == (WCHAR)'P' &&
            (pPerf)->Signature[1] == (WCHAR)'E' &&
            (pPerf)->Signature[2] == (WCHAR)'R' &&
            (pPerf)->Signature[3] == (WCHAR)'F' )
		{
            break;
        }

        // if buffer is not big enough, reallocate and try again

        if (rc == ERROR_MORE_DATA){
			
            dwSize += EXTEND_SIZE;
			
			GlobalUnlock( bufHandle );
			
            bufHandle = GlobalReAlloc( bufHandle, dwSize, GMEM_ZEROINIT );

			buf = (LPBYTE)GlobalLock( bufHandle );
        }
        else {
			bErrorOccured=true;
//			m_strLastError=_T("Could Not Obtain Performance Data");
			goto exit;
        }
    }

    // set the perf_object_type pointer

    pObj = (PPERF_OBJECT_TYPE) ((DWORD)pPerf + pPerf->HeaderLength);

    // 
    // loop thru the performance counter definition records looking 
    // for the process id counter and then save its offset 
    // 
	
    pCounterDef = (PPERF_COUNTER_DEFINITION) ((DWORD)pObj + pObj->HeaderLength); 
	
    for (i=0; i<(DWORD)pObj->NumCounters; i++) { 
        if (pCounterDef->CounterNameTitleIndex == dwProcessIdTitle) { 			
			
            dwProcessIdCounter = pCounterDef->CounterOffset; 
            break; 
        } 
		pCounterDef = ((PPERF_COUNTER_DEFINITION)((PBYTE)pCounterDef + pCounterDef->ByteLength));
        //pCounterDef++; 
    } 
	

    dwNumTasks = min( dwLimit, (DWORD)pObj->NumInstances );
    pInst = (PPERF_INSTANCE_DEFINITION) ((DWORD)pObj + pObj->DefinitionLength);
	pCounter = (PPERF_COUNTER_BLOCK) ((DWORD)pInst + pInst->ByteLength);

    // loop thru the performance instance data extracting each process name	

	
	if ( dwNumTasks ) {
		div = 25.0/(double)dwNumTasks;
	}
    for (i=1; i<dwNumTasks; i++) {
       incr += div;
		//
        // pointer to the process name
        //
        p = (LPSTR) ((DWORD)pInst + pInst->NameOffset);
		
		

        //
        // convert it to ascii
        //
        rc = WideCharToMultiByte( CP_ACP,
                                  0,
                                  (LPCWSTR)p,
                                  -1,
                                  processName,
                                  sizeof(processName),
                                  NULL,
                                  NULL );

		if (rc)	{
   				//m_strArray.Add(szProcessName);
				//TRACE1("%s\t", szProcessName);
			//MessageBox( NULL, processName, "", MB_OK );
		}

        // get the process id
        pCounter = (PPERF_COUNTER_BLOCK) ((DWORD)pInst + pInst->ByteLength);
        DWORD nProcessId = *((LPDWORD)((DWORD)pCounter + dwProcessIdCounter));
		

		if ( nProcessId != 0 ) {			
			lstrcpy( &g_systemProcessNameList[g_systemProcessListCount][0], processName );
			g_systemProcessList[g_systemProcessListCount] = nProcessId;

			char tmp[256];
			wsprintf( tmp, "Proc name: %s, PID: %d", processName, nProcessId );
			//MessageBox( NULL, tmp, "", MB_OK );

			g_systemProcessListCount ++;
		}		
		UpdateInitDlgProgress( (int)incr  );
		// next process
        pInst = (PPERF_INSTANCE_DEFINITION) ((DWORD)pCounter + pCounter->ByteLength);
        

		char tmp[256];
		wsprintf( tmp, "pCounter is %p, pInst: %p, ", pCounter, pInst );
		//MessageBox( NULL, tmp, "", MB_OK );

    }

exit:
	GlobalUnlock( bufHandle );
	GlobalFree( bufHandle );

    RegCloseKey(hKeyNames);
    RegCloseKey(HKEY_PERFORMANCE_DATA);
}






int ShowBlackBoxUI( EXCEPTION_POINTERS * pExPtrs, HINSTANCE hInstance )
{

	g_initDlg = CreateDialog(hInstance, (LPCTSTR)IDD_INIT_DLG, 
								NULL, (DLGPROC)InitBlackBoxUI_DlgProc);

	//SetWindowPos( g_initDlg, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE );  

	g_BlackBoxUIExPtrs = pExPtrs;	
	g_BlackBoxUIHInstance = hInstance;

	g_sysInfoString[0] = 0;
	g_memInfoString[0] = 0;
	g_versionInfoString[0] = 0;
	g_systemProcessListCount = 0;

	//GetOS info now
	ZeroMemory( &g_sysInfo, sizeof(g_sysInfo) );
	GetSystemInfo( &g_sysInfo );	
	
	
	ZeroMemory( &g_OSVersion, sizeof(g_OSVersion) );
	g_OSVersion.dwOSVersionInfoSize = sizeof(g_OSVersion);

	HasOSInfo = GetVersionEx( &g_OSVersion );
	

	ZeroMemory( &g_sysMemoryStatus, sizeof(MEMORYSTATUS) );
	g_sysMemoryStatus.dwLength = sizeof(MEMORYSTATUS);
	GlobalMemoryStatus ( &g_sysMemoryStatus );

	int processorIndex = 0;
	if ( g_sysInfo.dwProcessorType == PROCESSOR_INTEL_386 ) {
		processorIndex = 0;
	}
	else if ( g_sysInfo.dwProcessorType == PROCESSOR_INTEL_486 ) {
		processorIndex = 1;
	}
	else if ( g_sysInfo.dwProcessorType == PROCESSOR_INTEL_PENTIUM ) {
		processorIndex = 2;
	}

	wsprintf( g_sysInfoString, "Number of Processors: %d\nProcessor Type: %s", 
				g_sysInfo.dwNumberOfProcessors,
				g_ProcessorTypes[processorIndex] );


	int osIndex = 0;
	if ( g_OSVersion.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS ) {
		if ( g_OSVersion.dwMinorVersion == 0 ) {
			osIndex = 0;
		}
		else if ( g_OSVersion.dwMinorVersion > 0 ) {
			osIndex = 1;
		}
	}
	else if ( g_OSVersion.dwPlatformId == VER_PLATFORM_WIN32_NT ) {
		if ( g_OSVersion.dwMajorVersion == 4 ) {
			osIndex = 3;
		}
		else if ( g_OSVersion.dwMajorVersion > 4 ) {
			osIndex = 4;
		}
	}
	wsprintf( g_versionInfoString, "Windows OS: %s\nVersion %d.%04d\nBuild Number %d\n%s\n",
				g_windowsOSTypes[osIndex], 
				g_OSVersion.dwMajorVersion, 
				g_OSVersion.dwMinorVersion,
				g_OSVersion.dwBuildNumber,
				g_OSVersion.szCSDVersion );




	wsprintf( g_memInfoString, 
		"Current Memory Load:\t\t%d%\nTotal Physical Memory:\t\t%d bytes\nAvailable Physical Memory:\t\t%d bytes"\
		"\nTotal Virtual Memory:\t\t%d bytes\nAvailable Virtual Memory:\t\t%d bytes",
		g_sysMemoryStatus.dwMemoryLoad,
		g_sysMemoryStatus.dwTotalPhys,
		g_sysMemoryStatus.dwAvailPhys,
		g_sysMemoryStatus.dwTotalVirtual,
		g_sysMemoryStatus.dwAvailVirtual  );


	if ( g_OSVersion.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS ) {
		g_systemProcessListCount = 1;
		g_systemProcessList[0] = GetCurrentProcessId();

	}
	else if ( g_OSVersion.dwPlatformId == VER_PLATFORM_WIN32_NT ) {
		/*
		if ( ! EnumProcesses( g_systemProcessList, 512, &g_systemProcessListCount ) ) {
			g_systemProcessListCount = 0;
		}
		else {
			//try and get the module listings
		}
		*/		

		BuildProcessList();
		BuildModuleList();
	}		

	BuildStackTrace();

	DialogBox( hInstance, (LPCTSTR)IDD_BLACKBOX_ERR_DLG, NULL, (DLGPROC)BlackBoxUI_DlgProc);

	GlobalUnlock( g_procModListHandle );
	GlobalFree( g_procModListHandle );

	return TRUE;
}




LRESULT CALLBACK BlackBoxUI_DlgProc(HWND hBlackBoxUIDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	LRESULT result = FALSE;

	switch (message)
	{
		
		case WM_CTLCOLORSTATIC: {
			//HWND label = ::GetDlgItem( hBlackBoxUIDlg, IDC_INTRO );

			//HDC hdcStatic = (HDC) wParam;   // handle to display context 
			HWND hwndStatic = (HWND) lParam;
			
			if ( (GetDlgCtrlID(hwndStatic) == IDC_INTRO) || 
					(GetDlgCtrlID(hwndStatic) == IDC_INTRO2) || 
					(GetDlgCtrlID(hwndStatic) == IDC_BUG_LBL) ) {
				HBRUSH br = CreateSolidBrush( RGB(255,255,255) );
				result = (LRESULT)br;
			}			
		}
		break;

		case WM_INITDIALOG: {
			if ( g_BlackBoxUIExPtrs == NULL ) {
				result = FALSE;
			}
			else {
				HICON icon = (HICON)LoadImage( g_BlackBoxUIHInstance, 
											MAKEINTRESOURCE(IDI_BUG),
											IMAGE_ICON,
											0, 0, 
											LR_DEFAULTSIZE );

				HWND label = ::GetDlgItem( hBlackBoxUIDlg, IDC_INTRO );
				SetWindowText( label, g_errorLable );

				SendMessage( hBlackBoxUIDlg, WM_SETICON, ICON_BIG, (LPARAM)icon );

				HWND stackTraceHWnd = ::GetDlgItem( hBlackBoxUIDlg, IDC_STACKTRACE );
				HWND exceptionLabelHWnd = ::GetDlgItem( hBlackBoxUIDlg, IDC_EXCEPTION );

				SetWindowText( exceptionLabelHWnd, GetFaultReason( g_BlackBoxUIExPtrs ) );

				HWND registersLabelHWnd = ::GetDlgItem( hBlackBoxUIDlg, IDC_REGISTER );

				SetWindowText( registersLabelHWnd, GetRegisterString( g_BlackBoxUIExPtrs ) );
				

				int incr = 85;

				for ( int i=0;i<g_stackTraceCount;i++){
					SendMessage( stackTraceHWnd,  LB_ADDSTRING, 0, (LPARAM)g_stackTrace[i] );

					UpdateInitDlgProgress( incr  );
					incr += 2;
				}			
				
				
				DestroyWindow( g_initDlg );
			}
			result = TRUE;
		}
		break;

		case WM_COMMAND: {
			switch ( LOWORD(wParam) ) {
				case IDOK : case IDCANCEL : {
					EndDialog( hBlackBoxUIDlg, LOWORD(wParam) );
					return TRUE;
				}
				break;

				case IDC_ABOUT : {
					DialogBox( g_BlackBoxUIHInstance, 
								(LPCTSTR)IDD_ABOUT_BLACKBOX, 
								hBlackBoxUIDlg, 
								(DLGPROC)AboutBlackBox_DlgProc);
				}
				break;

				case IDC_MACHINE_INFO : {
					DialogBox( g_BlackBoxUIHInstance, 
								(LPCTSTR)IDD_MACHINE_INFO_DLG, 
								hBlackBoxUIDlg, 
								(DLGPROC)MachineInfo_DlgProc);
				}
				break;

				case IDC_MACHINE_STATE : {
					DialogBox( g_BlackBoxUIHInstance, 
								(LPCTSTR)IDD_MACHINESTATE_DLG, 
								hBlackBoxUIDlg, 
								(DLGPROC)MachineState_DlgProc);
				}
				break;

				case IDC_SEND : {
					SetCursor( LoadCursor(NULL, IDC_WAIT) );
					if ( OnSendData( hBlackBoxUIDlg ) ) {
						MessageBox( hBlackBoxUIDlg, "Successfully sent Crash Information", "Bug Trapper", MB_OK | MB_ICONINFORMATION );
					}
					else {
						MessageBox( hBlackBoxUIDlg, "Failed to send Crash Information.\nPlease save the information to a file\nand then send this to jim.crafton@ftid.com", "Bug Trapper", MB_OK | MB_ICONERROR );
						HWND saveToFileBtn = ::GetDlgItem( hBlackBoxUIDlg, IDC_SAVE_TO_FILE );

						::SetActiveWindow( saveToFileBtn );
						SetFocus( saveToFileBtn );						
					}
					SetCursor( LoadCursor(NULL, IDC_ARROW) );
				}
				break;
				
				case IDC_MAILTO : {
					SetCursor( LoadCursor(NULL, IDC_WAIT) );
					
					::ShellExecute( NULL, NULL, g_mailToAddress, NULL, NULL, SW_NORMAL );

					SetCursor( LoadCursor(NULL, IDC_ARROW) );
				}
				break;

				case IDC_SUBMIT_NEW_VCF_BUG : {
					SetCursor( LoadCursor(NULL, IDC_WAIT) );

					::ShellExecute( NULL, NULL, g_submitBugURL, NULL, NULL, SW_NORMAL );

					SetCursor( LoadCursor(NULL, IDC_ARROW) );
				}
				break;
				
				case IDC_COPY_TO_CLIPBOARD : {
					SetCursor( LoadCursor(NULL, IDC_WAIT) );

					OnCopyToClipboardData( hBlackBoxUIDlg );

					SetCursor( LoadCursor(NULL, IDC_ARROW) );
				}
				break;

				case IDC_SAVE_TO_FILE : {
					SetCursor( LoadCursor(NULL, IDC_WAIT) );

					OnSaveDataToFile( hBlackBoxUIDlg );

					SetCursor( LoadCursor(NULL, IDC_ARROW) );
				}
				break;
			}			
		}
		break;
	}

    return result;
}


LRESULT CALLBACK AboutBlackBox_DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM )
{
	LRESULT result = FALSE;

	switch (message)
	{
		case WM_INITDIALOG: {
			
			result = TRUE;
		}
		break;

		case WM_COMMAND: {
			if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) 
			{
				EndDialog( hDlg, LOWORD(wParam) );
				return TRUE;
			}
		}
		break;
	}

    return result;
}

LRESULT CALLBACK MachineState_DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM )
{	
	LRESULT result = FALSE;

	switch (message)
	{
		case WM_INITDIALOG: {			

			HWND processList = ::GetDlgItem( hDlg, IDC_PROCESS_LIST );
			for (DWORD i=0; i<g_systemProcessListCount;i++ ) {				
				char tmp[1024];
				wsprintf( tmp, "%s, PID: %d", 
					g_systemProcessNameList[i],
					g_systemProcessList[i] );				
				
				SendMessage( processList, LB_ADDSTRING, 0, (LPARAM)tmp );
			}
			result = TRUE;
		}
		break;

		case WM_COMMAND: {
			if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) 
			{
				EndDialog( hDlg, LOWORD(wParam) );
				return TRUE;
			}
			else {
				switch ( HIWORD(wParam) ) {
					case LBN_SELCHANGE : {
						if ( LOWORD(wParam) == IDC_PROCESS_LIST ) {
							HWND processList = ::GetDlgItem( hDlg, IDC_PROCESS_LIST );
							DWORD index = SendMessage( processList, LB_GETCURSEL, 0, 0 );

							//char tmp[256];
							//wsprintf( tmp, "index: %d", index );
							//MessageBox( hDlg, tmp, "", MB_OK );

							HWND moduleList = ::GetDlgItem( hDlg, IDC_PROCESS_MODULE_LIST );
							SendMessage( moduleList, LB_RESETCONTENT, 0,0 );
							bool doneLoadingModuleInfo = false;
							for (DWORD j=0;j<g_systemTotalModuleCount;j++){
								if ( g_processModuleInfoList[j].index == index ) {

									SendMessage( moduleList, LB_ADDSTRING, 0, 
												(LPARAM)g_processModuleInfoList[j].name );
									//doneLoadingModuleInfo = true;
								}
								else if (doneLoadingModuleInfo) {
									break;
								}
							}

						}	
					}
					break;
				}
			}
		}
		break;
	}

    return result;
}

LRESULT CALLBACK MachineInfo_DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM )
{
	LRESULT result = FALSE;

	switch (message)
	{
		case WM_INITDIALOG: {
			HWND CPULabel = GetDlgItem( hDlg, IDC_CPU_LABEL );
			SetWindowText( CPULabel, g_sysInfoString );

			HWND OSLabel = GetDlgItem( hDlg, IDC_OS_LABEL );
			SetWindowText( OSLabel, g_versionInfoString );

			HWND MemLabel = GetDlgItem( hDlg, IDC_MEM_LABEL );
			SetWindowText( MemLabel, g_memInfoString );

			result = TRUE;
		}
		break;

		case WM_COMMAND: {
			if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) 
			{
				EndDialog( hDlg, LOWORD(wParam) );
				return TRUE;
			}
		}
		break;
	}

    return result;
}	

LRESULT CALLBACK InitBlackBoxUI_DlgProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT result = FALSE;

	switch (message)
	{
		case WM_INITDIALOG: {
			
			HWND progress = GetDlgItem( hDlg, IDC_PROGRESS_LABEL );
			char tmp[256];
			wsprintf( tmp, "Updating .....%d %%", 0 );
			SetWindowText( progress, tmp );
			InvalidateRect( hDlg, NULL, TRUE );

			result = TRUE;
		}
		break;

		case WM_UpdateInitDlgProgress : {
			HWND progress = GetDlgItem( hDlg, IDC_PROGRESS_LABEL );
			char tmp[256];
			wsprintf( tmp, "Updating .....%d %%", (int)lParam );
			SetWindowText( progress, tmp );
			InvalidateRect( progress, NULL, TRUE );
			InvalidateRect( hDlg, NULL, TRUE );
		}
		break;

		case WM_COMMAND: {
			if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) 
			{
				EndDialog( hDlg, LOWORD(wParam) );
				return TRUE;
			}
		}
		break;
	}

    return result;
}

DWORD GetRawReportSize()
{
	DWORD result = 0;
    DWORD j;

	result = lstrlen( g_sysInfoString ) + 1;
	result += lstrlen( g_memInfoString ) + 1;
	result += lstrlen( g_versionInfoString ) + 1;

	for ( int i=0;i<g_stackTraceCount;i++){		
		result += lstrlen( g_stackTrace[i] ) + 1;	
	}


	for (j=0; j<g_systemProcessListCount;j++ ) {				
		char tmp[1024];
		wsprintf( tmp, "%s, PID: %d", 
			g_systemProcessNameList[j],
			g_systemProcessList[j] );					
		result += lstrlen( tmp ) + 1;	
	}

	for (j=0;j<g_systemTotalModuleCount;j++){
		result += lstrlen( g_processModuleInfoList[j].name ) + 1;
	}

	const char* faultReason = GetFaultReason( g_BlackBoxUIExPtrs );
	const char* registerString = GetRegisterString( g_BlackBoxUIExPtrs );

	result += lstrlen( faultReason ) + 1;
	result += lstrlen( registerString ) + 1;

	return result;
}

HGLOBAL GenerateRawReport()
{
	HGLOBAL result = NULL;

	const char* faultReason = GetFaultReason( g_BlackBoxUIExPtrs );
	const char* registerString = GetRegisterString( g_BlackBoxUIExPtrs );	
	
	int reportSize = GetRawReportSize();
	char rptSizeStr[25];
	wsprintf( rptSizeStr, "%d", reportSize );

	char* reportBuf = NULL;

	result = GlobalAlloc( GHND, reportSize + lstrlen(rptSizeStr)+1 );
	if ( NULL != result ) {
		reportBuf = (char*) GlobalLock( result );
	}
	else {		
		return result;
	}


	
	char* tmp = reportBuf;
	if ( NULL == result ) {
		lstrcpy( tmp, rptSizeStr );
		tmp[lstrlen(rptSizeStr)] = '\n';
		tmp += lstrlen(rptSizeStr)+1;
	}

	int index = 0;
	lstrcpy( tmp, g_sysInfoString );
	index = lstrlen( g_sysInfoString );
	tmp[index] = '\n';
	tmp += index+1;	

	lstrcpy( tmp, g_versionInfoString );
	index = lstrlen( g_versionInfoString );
	tmp[index] = '\n';
	tmp += index+1;	
	

	lstrcpy( tmp, g_memInfoString );
	index = lstrlen( g_memInfoString );
	tmp[index] = '\n';
	tmp += index+1;	

	lstrcpy( tmp, registerString );
	index = lstrlen( registerString );
	tmp[index] = '\n';
	tmp += index+1;	

	lstrcpy( tmp, faultReason );
	index = lstrlen( faultReason );
	tmp[index] = '\n';
	tmp += index+1;	
	
	


	for ( int i=0;i<g_stackTraceCount;i++){		
		lstrcpy( tmp, g_stackTrace[i] );
		index = lstrlen( g_stackTrace[i] );
		tmp[index] = '\n';
		tmp += index+1;
	}

	for ( DWORD j=0; j<g_systemProcessListCount;j++ ) {				
		char processID[1024];
		wsprintf( processID, "%s, PID: %d", 
			g_systemProcessNameList[j],
			g_systemProcessList[j] );					
		
		lstrcpy( tmp, processID );
		index = lstrlen( processID );
		
		tmp[index] = '\n';
		tmp += index+1;
		
		bool first = true;
		for (DWORD k=0;k<g_systemTotalModuleCount;k++){
			if ( g_processModuleInfoList[k].index == j ) {
				lstrcpy( tmp, g_processModuleInfoList[k].name );
				index = lstrlen( g_processModuleInfoList[k].name );
				tmp[index] = '\n';
				tmp += index+1;

				first = false;
			}
			else if (!first) {
				break;
			}
		}
	}
	
	if ( NULL != result ) {
		GlobalUnlock( result );
	}

	return result;
}


#define BUG_TRAPPER_SVR_PORT	10666


bool OnSendData( HWND )
{
	bool result = false;


	HGLOBAL hGlobal = GenerateRawReport();
	if ( NULL == hGlobal ) {
		return false;
	}

	char* rawReport = (char*)GlobalLock( hGlobal );

	if ( NULL != rawReport ) {
		
		WORD wVersionRequested = MAKEWORD(1, 1);
		WSADATA wsaData; 
		int err = WSAStartup(wVersionRequested, &wsaData); 
		if (err != 0) {		
			
			WSACleanup();
			GlobalUnlock( hGlobal );
			GlobalFree( hGlobal );
			return false;
		}
		
		if ( LOBYTE( wsaData.wVersion ) != 1 || 
			HIBYTE( wsaData.wVersion ) != 1 ) 	{ 
			WSACleanup();
			GlobalUnlock( hGlobal );
			GlobalFree( hGlobal );
			return false;
		}
		
		SOCKET sock;
		sock = ::socket(AF_INET,SOCK_STREAM,0);
		if ( sock > 0 ) {
			char* hostName = "172.17.17.98";
			struct hostent *hp;
			short port = BUG_TRAPPER_SVR_PORT;
			struct sockaddr_in sockAddr;
			// Initialize the socket address to the server's address. 
			FillMemory( &sockAddr, sizeof(sockAddr), 0 );
			sockAddr.sin_family = AF_INET;
			
			hp = gethostbyname( hostName );    //to get host address 

			if (hp != NULL) {
				CopyMemory( &(sockAddr.sin_addr.s_addr), hp->h_addr, hp->h_length );
				sockAddr.sin_port = htons(port);
				// Connect to the server.
				int connectErr = connect(sock, (sockaddr*)&sockAddr,sizeof(sockAddr));
				if ( connectErr != -1 ) {	
					DWORD size = GetRawReportSize();
					int res = ::send( sock, rawReport, size, 0 );
					if ( -1 != res ) {	
						
						result = true;
					}
					else {
						//int i = WSAGetLastError();						
					}
				}
			}

			closesocket( sock );
			
		}
		WSACleanup();

		GlobalUnlock( hGlobal );		
	}	
	GlobalFree( hGlobal );


	return result;
}

bool OnCopyToClipboardData( HWND hDlg )
{
	bool result = false;

	OpenClipboard( hDlg );

	HGLOBAL hMem = GenerateRawReport();	

	SetClipboardData( CF_TEXT, hMem );

	CloseClipboard();

	return result;
}	

bool OnSaveDataToFile( HWND hDlg )
{
	bool result = false;
	
	HGLOBAL hMem = GenerateRawReport();	
	if ( NULL == hMem ) {
		return false;
	}


	char* rawReport = (char*)GlobalLock( hMem );
	if ( NULL != rawReport ) {
		
		g_saveFilePath[0] = 0;
		SYSTEMTIME tm = {0};
		GetSystemTime( &tm );
		wsprintf( g_saveFilePath, "errorlog-%02d-%02d-%04d(%02d.%02d.%02d).log",
					tm.wDay, tm.wMonth, tm.wYear, tm.wHour, tm.wMinute, tm.wSecond );

		OPENFILENAME ofn = {0};
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hInstance = ::GetModuleHandle( NULL );
		ofn.hwndOwner = hDlg;
		ofn.lpstrFilter = g_errorLogTypes;
		ofn.lpstrFile = g_saveFilePath;
		ofn.nMaxFile = MAX_PATH;
		ofn.Flags = OFN_EXPLORER | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;

		if ( GetSaveFileName( &ofn ) ) {
			
			HANDLE hFile = CreateFile( g_saveFilePath, GENERIC_WRITE, FILE_SHARE_READ, NULL,
										CREATE_ALWAYS, 0, NULL );

			if ( INVALID_HANDLE_VALUE != hFile ) {
				DWORD size = GlobalSize(hMem);
				DWORD written = 0;
				WriteFile( hFile, rawReport, size, &written, NULL );
				CloseHandle( hFile );	
			}
		}		
	}
	GlobalUnlock( hMem );
	GlobalFree( hMem );

	return result;
}