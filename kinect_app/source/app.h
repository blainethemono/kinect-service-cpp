#ifndef __kinect_app_app_h__
#define __kinect_app_app_h__

#include "win_service.h"
#include "app.h"
#include "sensor.h"
#include "logic.h"
#include "event.h"

#include <memory>
#include <Windows.h>
#include <process.h>


namespace kinect_app
{

namespace app_flags
{
	enum
	{
		Skeleton = 1,
		Depth    = 2,
		Color    = 4
	};

	typedef size_t Mask;

	const size_t Default = Skeleton | Depth;
}

class App : public kinect_app::ServiceApp
{
public:
	App(app_flags::Mask settings);
	~App();

	virtual void Init();
	virtual void Deinit();
	virtual void Start();
	virtual void Stop();
	virtual void Run();

private:
	static unsigned __stdcall ThreadStub(void* arg);
	void RunInternal();

private:
	app_flags::Mask m_settings;
	Event m_stopEvent;
	HANDLE m_workingThread;
	Event m_workingThreadStarted;
	kinect_app::Engine m_engine;
	kinect_app::NuiSensor m_sensor;
};


}


#endif //__kinect_app_app_h__