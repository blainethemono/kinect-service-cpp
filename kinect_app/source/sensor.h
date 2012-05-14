#ifndef __kinect_app_sensor_h__
#define __kinect_app_sensor_h__

#include "event.h"

#include <Windows.h>
#include <NuiApi.h>

namespace kinect_app
{

struct SensorCallbacks;

class NuiSensor
{
public:
	enum
	{
		Timeout,
		Depth,
		Color,
		Skeleton
	};
	typedef size_t event_mask;

	NuiSensor(SensorCallbacks& callbacks);

	~NuiSensor();

	// enables skeleton tracking
	void EnableSkeletonTracking();

	// enables image stream
	void OpenImageColorStream();

	// enables depth stream
	void OpenImageDepthStream();

	// waits for events including stop event
	bool WaitEvents(event_mask& kind);
	
	// dispatches event synchronously
	bool DispatchEvents();

	// cancels kinect events loop
	void Shutdown();

private:
	// dispatches depth alert
	void DispatchDepthAlert();
	
	// dispatches color alert
	void DispatchColorAlert();

	// dispatches skeleton alert
	void DispatchSkeletonAlert();

private:
	NuiSensor(const NuiSensor&);
	NuiSensor& operator = (const NuiSensor&);

private:
	SensorCallbacks& m_callbacks;

	INuiSensor* m_sensor;
	
	Event m_stopEvent;

	Event m_skeletonTrackingEvent;

	Event m_nextColorFrameEvent;
	HANDLE m_streamHandle;

	Event m_nextDepthFrameEvent;
	HANDLE m_depthStreamHandle;

	HANDLE m_events[4];
	DWORD  m_index;
};
}

#endif //__kinect_app_sensor_h__
