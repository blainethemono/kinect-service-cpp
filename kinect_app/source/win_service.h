#ifndef __kinekt_app_win_service_h__
#define __kinekt_app_win_service_h__

#include <Windows.h>
#include <memory>

namespace kinect_app
{

// acceptable sequences:
// 1. thread1: app->Init(), app->Start(), app->Stop(), app->Deinit(): used in service mode
// 2. thread1: app->Init(), app->Run(), thread2: app->Stop(), thread1: app->Deinit(): used in console mode
struct ServiceApp
{
	// initialize, must be called before start/run
	virtual void Init() = 0;

	// starts thread performing synchronous loop, must return after working thread started
	virtual void Start() = 0;

	// synchronous loop, call if don't need to do anything else in your app
	virtual void Run() = 0;

	// stops synchronous loop, must wait working thread stopped
	virtual void Stop() = 0;

	// deinitialize, must be called after stop
	virtual void Deinit() = 0;

	virtual ~ServiceApp() {}
};

// installs KinectAppSvc
void InstallService();

// uninstalls KinectAppSvc
void UninstallService();

// starts KinectAppSvc
void StartService(std::auto_ptr<ServiceApp> app);

}

#endif //__kinekt_app_win_service_h__
