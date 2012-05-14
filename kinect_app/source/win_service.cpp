#include "win_service.h"
#include "event.h"

#include <tchar.h>
#include <strsafe.h>

#define SVCNAME TEXT("KinectAppSvc")
#define SVC_ERROR 1001

namespace
{
class WinService
{
public:
	WinService();

	void AttachHostedApp(std::auto_ptr<kinect_app::ServiceApp> app);

	void Install();

	void Uninstall();

	void __stdcall Main(DWORD dwArgc, LPTSTR *lpszArgv);

	void __stdcall CtrlHandler(DWORD dwCtrl);

	void ReportEvent(LPTSTR szFunction) ;

private:
	void Init(DWORD dwArgc, LPTSTR *lpszArgv);

	void ReportSvcStatus(DWORD dwCurrentState,
						 DWORD dwWin32ExitCode,
						 DWORD dwWaitHint);

private:
	SERVICE_STATUS m_svcStatus; 
	SERVICE_STATUS_HANDLE m_svcStatusHandle; 
	kinect_app::Event m_svcStopEvent;
	std::auto_ptr<kinect_app::ServiceApp> m_app;
	DWORD dwCheckPoint;
};

WinService ServiceInstance;


void __stdcall SvcMain(DWORD dwArgc, LPTSTR *lpszArgv)
{
	ServiceInstance.Main(dwArgc, lpszArgv);
}

void __stdcall SvcCtrlHandler(DWORD dwCtrl)
{
	ServiceInstance.CtrlHandler(dwCtrl);
}


WinService::WinService(): dwCheckPoint(1)
{
}

void WinService::AttachHostedApp(std::auto_ptr<kinect_app::ServiceApp> app)
{
	m_app = app;
}

void WinService::Install()
{
	SC_HANDLE schSCManager;
	SC_HANDLE schService;
	TCHAR szPath[MAX_PATH];

	if (!GetModuleFileName(NULL, szPath, MAX_PATH))
	{
		printf("Cannot install service (%d)\n", GetLastError());
		return;
	}

	// Get a handle to the SCM database. 
 
	schSCManager = OpenSCManager( 
		NULL,                    // local computer
		NULL,                    // ServicesActive database 
		SC_MANAGER_ALL_ACCESS);  // full access rights 
 
	if (NULL == schSCManager) 
	{
		printf("OpenSCManager failed (%d)\n", GetLastError());
		return;
	}

	// Create the service

	schService = CreateService( 
		schSCManager,              // SCM database 
		SVCNAME,                   // name of service 
		SVCNAME,                   // service name to display 
		SERVICE_ALL_ACCESS,        // desired access 
		SERVICE_WIN32_OWN_PROCESS, // service type 
		SERVICE_DEMAND_START,      // start type 
		SERVICE_ERROR_NORMAL,      // error control type 
		szPath,                    // path to service's binary 
		NULL,                      // no load ordering group 
		NULL,                      // no tag identifier 
		NULL,                      // no dependencies 
		NULL,                      // LocalSystem account 
		NULL);                     // no password 
 
	if (schService == NULL) 
	{
		printf("CreateService failed (%d)\n", GetLastError()); 
		CloseServiceHandle(schSCManager);
		return;
	}
	else
	{
		printf("Service installed successfully\n");
	}

	CloseServiceHandle(schService); 
	CloseServiceHandle(schSCManager);
}

void WinService::Uninstall()
{
    SC_HANDLE schSCManager;
    SC_HANDLE schService;
    SERVICE_STATUS ssStatus; 

    schSCManager = OpenSCManager( 
        NULL,                    // local computer
        NULL,                    // ServicesActive database 
        SC_MANAGER_ALL_ACCESS);  // full access rights 
 
    if (NULL == schSCManager) 
    {
        printf("OpenSCManager failed (%d)\n", GetLastError());
        return;
    }

    schService = OpenService(schSCManager, SVCNAME, DELETE);
    if (schService == NULL)
    { 
        printf("OpenService failed (%d)\n", GetLastError()); 
        CloseServiceHandle(schSCManager);
        return;
    }

    if (!DeleteService(schService)) 
    {
        printf("DeleteService failed (%d)\n", GetLastError()); 
    }
    else
	{
		printf("Service deleted successfully\n"); 
	}
 
    CloseServiceHandle(schService); 
    CloseServiceHandle(schSCManager);
}

void __stdcall WinService::Main(DWORD dwArgc, LPTSTR *lpszArgv)
{
	m_svcStatusHandle = RegisterServiceCtrlHandler(SVCNAME, SvcCtrlHandler);
	
	if(!m_svcStatusHandle)
	{ 
		this->ReportEvent(TEXT("RegisterServiceCtrlHandler")); 
		return; 
	}

	m_svcStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS; 
	m_svcStatus.dwServiceSpecificExitCode = 0;

	ReportSvcStatus(SERVICE_START_PENDING, NO_ERROR, 3000);
	
	this->Init(dwArgc, lpszArgv);
}

void WinService::Init(DWORD dwArgc, LPTSTR *lpszArgv)
try
{
	m_app->Init();

	m_app->Start();

	ReportSvcStatus(SERVICE_RUNNING, NO_ERROR, 0);

	m_svcStopEvent.Wait();

	m_app->Deinit();

	ReportSvcStatus(SERVICE_STOPPED, NO_ERROR, 0);
}
catch (...)
{
	ReportSvcStatus(SERVICE_STOPPED, NO_ERROR /*wtf?*/, 0);
}

void WinService::ReportSvcStatus( DWORD dwCurrentState,
						DWORD dwWin32ExitCode,
						DWORD dwWaitHint)
{
	// Fill in the SERVICE_STATUS structure.

	m_svcStatus.dwCurrentState = dwCurrentState;
	m_svcStatus.dwWin32ExitCode = dwWin32ExitCode;
	m_svcStatus.dwWaitHint = dwWaitHint;

	if (dwCurrentState == SERVICE_START_PENDING)
	{
		m_svcStatus.dwControlsAccepted = 0;
	}
	else
	{
		m_svcStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
	}

	if ((dwCurrentState == SERVICE_RUNNING) || (dwCurrentState == SERVICE_STOPPED))
	{
		m_svcStatus.dwCheckPoint = 0;
	}
	else
	{
		m_svcStatus.dwCheckPoint = dwCheckPoint++;
	}

	// Report the status of the service to the SCM.
	SetServiceStatus(m_svcStatusHandle, &m_svcStatus);
}

void __stdcall WinService::CtrlHandler(DWORD dwCtrl)
{
	switch (dwCtrl) 
	{
	case SERVICE_CONTROL_STOP: 
		{
			this->ReportSvcStatus(SERVICE_STOP_PENDING, NO_ERROR, 0);

			m_svcStopEvent.Set();

			this->ReportSvcStatus(m_svcStatus.dwCurrentState, NO_ERROR, 0);
         
			return;
		}
 
	case SERVICE_CONTROL_INTERROGATE: 
		break; 
 
	default: 
		break;
	}

}

void WinService::ReportEvent(LPTSTR szFunction) 
{ 
	HANDLE hEventSource;
	LPCTSTR lpszStrings[2];
	TCHAR Buffer[80];

	hEventSource = RegisterEventSource(NULL, SVCNAME);

	if (NULL != hEventSource)
	{
		StringCchPrintf(Buffer, 80, TEXT("%s failed with %d"), szFunction, GetLastError());

		lpszStrings[0] = SVCNAME;
		lpszStrings[1] = Buffer;

		::ReportEvent(hEventSource,        // event log handle
					EVENTLOG_ERROR_TYPE, // event type
					0,                   // event category
					SVC_ERROR,           // event identifier
					NULL,                // no security identifier
					2,                   // size of lpszStrings array
					0,                   // no binary data
					lpszStrings,         // array of strings
					NULL);               // no binary data

		DeregisterEventSource(hEventSource);
	}
}

}

namespace kinect_app
{

void InstallService()
{
	ServiceInstance.Install();
}

void UninstallService()
{
	ServiceInstance.Uninstall();
}

void StartService(std::auto_ptr<ServiceApp> app)
{
	SERVICE_TABLE_ENTRY DispatchTable[] = 
	{ 
		{ SVCNAME, (LPSERVICE_MAIN_FUNCTION) SvcMain }, 
		{ NULL, NULL } 
	}; 
 
	ServiceInstance.AttachHostedApp(app);

	if (!StartServiceCtrlDispatcher( DispatchTable )) 
	{ 
		ServiceInstance.ReportEvent(TEXT("StartServiceCtrlDispatcher")); 
	} 
}

}