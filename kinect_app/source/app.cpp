#include "app.h"

namespace kinect_app
{

App::App(app_flags::Mask settings)
	: m_settings(settings)
	, m_workingThread(NULL)
	, m_sensor(m_engine)
{
}

App::~App()
{
	::CloseHandle(m_workingThread);
}

void App::Init()
try
{
	m_engine.StartNetworkService();
}
catch (unsigned short)
{
	// @todo:
}

void App::Deinit()
{
}

void App::Start()
{
	unsigned threadId;

	if (m_workingThread = (HANDLE)_beginthreadex(NULL, 0, &App::ThreadStub, this, 0, &threadId))
	{
		m_workingThreadStarted.Wait();
	}
}

void App::Stop()
{
	if (m_workingThread)
	{
		m_sensor.Shutdown();

		::WaitForSingleObject(m_workingThread, 15000);
	}
}

void App::Run()
{
	this->RunInternal();
}

unsigned __stdcall App::ThreadStub(void* arg)
{
	(reinterpret_cast<App*>(arg))->RunInternal();
	return 0;
}

void App::RunInternal()
try
{
	using namespace kinect_app;

	m_workingThreadStarted.Set();
		
	if (!!(m_settings & app_flags::Skeleton))
	{
		m_sensor.EnableSkeletonTracking();
	}
		
	if (!!(m_settings & app_flags::Color))
	{
		m_sensor.OpenImageColorStream();
	}
		
	if (!!(m_settings & app_flags::Depth))
	{
		m_sensor.OpenImageDepthStream();
	}

	NuiSensor::event_mask kind;

	while (m_sensor.WaitEvents(kind))
	{
		m_sensor.DispatchEvents();
	}
}
catch (unsigned short /*code*/)
{
	// @todo:
}


}
